/*
 * VisualizationWidget.cpp - widget for visualization of sound-data
 *
 * Copyright (c) 2005-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include "VisualizationWidget.h"

#include "BufferManager.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "Mixer.h"
#include "Song.h"
#include "ToolTip.h"
#include "gui_templates.h"

#include <QMouseEvent>
#include <QPainter>

#include <cmath>

VisualizationWidget::VisualizationWidget(const QPixmap& _bg,
                                         QWidget*       _parent,
                                         Mode           _mode) :
      Widget(_parent),
      m_mode(_mode), m_active(false), m_frozen(false), m_stabilized(false),
      s_background(_bg)
{
    m_mode = _mode;

    const int w = s_background.width();
    const int h = s_background.height();

    m_pointCount = w;
    m_points     = new QPointF[m_pointCount];

    setFixedSize(w, h);
    //resizeCache(w, h);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    // const fpp_t frames = Engine::mixer()->framesPerPeriod();
    // m_buffer = new sampleFrame[frames];
    // BufferManager::clear( m_buffer, frames );

    m_buffer = BufferManager::acquire();

    ToolTip::add(this, tr("Left-click to enable/disable visualization of "
                          "master-output")
                               + tr(", middle-click to freeze, right-click "
                                    "to stabilize."));

    if(m_mode == Surround)
        setActive(ConfigManager::inst()
                          ->value("ui", "displaywaveform")
                          .toInt());
    else
        setActive(true);
}

VisualizationWidget::~VisualizationWidget()
{
    BufferManager::release(m_buffer);
    // delete[] m_buffer;
    delete[] m_points;
}

void VisualizationWidget::updateSurroundBuffer(
        const surroundSampleFrame* _buffer)
{
    if(m_active && !m_frozen && !Engine::getSong()->isExporting())
    {
        // qWarning("VisualizationWidget::updateAudioBuffer");
        const fpp_t fpp = Engine::mixer()->framesPerPeriod();
        // memcpy( m_buffer, _buffer, sizeof( surroundSampleFrame ) * fpp );
        for(ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ++ch)
            for(fpp_t i = 0; i < fpp; i++)
            {
                m_buffer[i][ch] = _buffer[i][ch];
                // if(_buffer[i][ch] != 0.) qWarning("%f",_buffer[i][ch]);
            }

        invalidateCache();
        // update();
    }
}

void VisualizationWidget::updateStereoBuffer(const sampleFrame* _buffer)
{
    if(m_active && !m_frozen && !Engine::getSong()->isExporting())
    {
        // qWarning("VisualizationWidget::updateAudioBuffer");
        const fpp_t fpp = Engine::mixer()->framesPerPeriod();
        memcpy(m_buffer, _buffer, sizeof(sampleFrame) * fpp);

        invalidateCache();
        // update();
    }
}

void VisualizationWidget::setActive(bool _active)
{
    if(m_active == _active)
        return;

    m_active = _active;
    invalidateCache();

    if(m_active)
    {
        connect(gui->mainWindow(), SIGNAL(periodicUpdate()), this,
                SLOT(update()));
        if(m_mode == Surround)
        {
            connect(Engine::mixer(),
                    SIGNAL(nextAudioBuffer(const surroundSampleFrame*)), this,
                    SLOT(updateSurroundBuffer(const surroundSampleFrame*)));
        }
    }
    else
    {
        disconnect(gui->mainWindow(), SIGNAL(periodicUpdate()), this,
                   SLOT(update()));
        if(m_mode == Surround)
        {
            disconnect(
                    Engine::mixer(),
                    SIGNAL(nextAudioBuffer(const surroundSampleFrame*)), this,
                    SLOT(updateSurroundBuffer(const surroundSampleFrame*)));
        }
        update();
    }
}

void VisualizationWidget::setFrozen(bool _frozen)
{
    if(m_frozen == _frozen)
        return;

    m_frozen = _frozen;
    invalidateCache();
}

void VisualizationWidget::setStabilized(bool _stabilized)
{
    if(m_stabilized == _stabilized)
        return;

    m_stabilized = _stabilized;
    invalidateCache();
}

void VisualizationWidget::drawWidget(QPainter& _p)
{
    _p.drawPixmap(0, 0, s_background);
    _p.setRenderHints(QPainter::Antialiasing, true);

    if(m_active && !Engine::getSong()->isExporting())
    {
        Mixer const* mixer = Engine::mixer();

        float master_output = mixer->masterGain();
        // qWarning("VisualizationWidget::masterGain %f",master_output);
        int         w      = width() - 4;
        const float half_h = -(height() - 11 - 6) / 2.0 * master_output - 1;
        int         x_base = 2;
        const float y_base = (height() - 11) / 2 - 0.5f;

        //		_p.setClipRect( 2, 2, w, height()-4 );

        const fpp_t frames = mixer->framesPerPeriod();
        float       peakLeft;
        float       peakRight;
        mixer->getPeakValues(m_buffer, frames, peakLeft, peakRight);
        const float max_level
                = qMax<float>(peakLeft, peakRight) * master_output;

        // _p.setRenderHint( QPainter::Antialiasing );

        // if(max_level>0.0001f)
        {
            int nbp = qMin<int>(w, frames);
            if(nbp <= 1)
                return;
            if(nbp > m_pointCount)
                nbp = m_pointCount;

            const fpp_t fpp   = Engine::mixer()->framesPerPeriod();
            int         fzero = 0;
            for(fpp_t f = 1; f < fpp - 1; f++)
            {
                float v0 = m_buffer[f][0];
                float v1 = m_buffer[f - 1][0];
                if(v1 <= 0.f && v0 >= 0.f)
                {
                    if(fabsf(v1) < fabsf(v0))
                        fzero = f - 1;
                    else
                        fzero = f;
                    break;
                }
            }

            int izero = (int)floorf(1.f * (nbp - 1) * fzero / (fpp - 1));
            for(ch_cnt_t ch = DEFAULT_CHANNELS - 1; ch >= 0; --ch)
            {
                QColor c;
                if(max_level > 1.f)
                    c = QColor(255,0,0); //QColor(255, 64, 64);
                else if(max_level > 0.9f)
                    c = QColor(255,180,0); //QColor(255, 192, 64);
                else if(ch == 0)
                    c = QColor(0,180,255); //QColor(71, 253, 133);
                else
                    c = QColor(0,255,180); //QColor(71, 133, 253);
                _p.setPen(c);
                // _p.setPen( QPen( _p.pen().color(), 0.7 ) );

                for(int i = 0; i < nbp; i++)
                {
                    const float   tp    = (float)i / (float)nbp;
                    const f_cnt_t frame = frames * tp;
                    const float   xp = x_base + (w - 1.f) / (nbp - 1.f) * i;
                    const float   yp
                            = y_base
                              + half_h * Mixer::clip(m_buffer[frame][ch]);
                    m_points[i] = QPointF(xp, yp);
                }
                if(!m_stabilized || izero == 0)
                {
                    _p.drawPolyline(m_points, nbp);
                }
                else
                {
                    _p.translate(x_base - m_points[izero].x(), 0.f);
                    _p.drawPolyline(m_points + izero, nbp - 1 - izero);
                    _p.translate(-x_base + m_points[izero].x(), 0.f);

                    /*
                    _p.translate(-x_base + m_points[nbp - izero].x(), 0.f);
                    _p.drawPolyline(m_points, izero + 1);
                    _p.translate(x_base - m_points[nbp - izero].x(), 0.f);
                    */
                }
            }

            /*
            const float xd = (float) w / frames;
            // now draw all that stuff
            for( ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ++ch )
            {
                    for( int frame = 0; frame < frames;
            frame+=qMax(1,frames/w) )//++frame )
                    {
                            m_points[frame]=
                                    QPointF(x_base + (float) frame * xd,
                                            y_base + Mixer::clip(
            m_buffer[frame][ch] ) * half_h );
                    }
                    _p.drawPolyline( m_points, frames );
            }
            */
        }
        // else draw an horizontal line (TODO)
    }
    else
    {
        _p.setPen(QColor(192, 192, 192));
        _p.setFont(pointSize<7>(_p.font()));
        _p.drawText(6, height() - 5, tr("Click to enable"));
    }
}

/*
void VisualizationWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    if(paintCache(p))
        return;

    QPainter& pc = beginCache();
    drawWidget(pc);
    endCache();

    paintCache(p);
}
*/

void VisualizationWidget::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::LeftButton)
    {
        setActive(!m_active);
        if(m_active)
            m_frozen = false;
    }
    if(_me->button() == Qt::MiddleButton)
    {
        if(m_active)
            setFrozen(!m_frozen);
    }
    if(_me->button() == Qt::RightButton)
    {
        if(m_active)
            setStabilized(!m_stabilized);
    }
}
