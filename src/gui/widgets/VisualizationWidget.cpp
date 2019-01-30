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

#include "Backtrace.h"
#include "BufferManager.h"
#include "Configuration.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "Mixer.h"
#include "PaintManager.h"
#include "Song.h"
#include "ToolTip.h"
#include "gui_templates.h"
#include "lmms_constants.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

#include <cmath>

VisualizationWidget::VisualizationWidget(const QPixmap& _bg,
                                         QWidget*       _parent,
                                         Ring*          _ring,
                                         ChannelMode    _channelMode,
                                         DisplayMode    _displayMode) :
      VisualizationWidget(_bg.width(),
                          _bg.height(),
                          _parent,
                          _ring,
                          _channelMode,
                          _displayMode)
{
    m_background = _bg;
    // m_background.fill(Qt::black);
}

VisualizationWidget::VisualizationWidget(int         _width,
                                         int         _height,
                                         QWidget*    _parent,
                                         Ring*       _ring,
                                         ChannelMode _channelMode,
                                         DisplayMode _displayMode) :
      Widget(_parent),
      m_channelMode(_channelMode), m_displayMode(Off),  // m_frozen(false),
      m_stabilized(false), m_zoomX(1.f), m_zoomY(1.f), m_ring(_ring)
{
    m_pointCount = _width;
    m_points     = new QPointF[m_pointCount];

    setFixedSize(_width, _height);
    // resizeCache(w, h);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    // const fpp_t frames = Engine::mixer()->framesPerPeriod();
    // m_buffer = new sampleFrame[frames];
    // BufferManager::clear( m_buffer, frames );

    // m_len    = qMax<int>(1024, Engine::mixer()->framesPerPeriod());
    // m_buffer = MM_ALLOC(sampleFrame, m_len);  // BufferManager::acquire();
    // memset(m_buffer, 0, sizeof(sampleFrame) * m_len);

    ToolTip::add(this, tr("Left-click to display, middle-click to freeze, "
                          "right-click to stabilize."));

    if(!CONFIG_GET_BOOL("ui.displaywaveform"))
        setDisplayMode(Off);
    else
        setDisplayMode(_displayMode);
}

VisualizationWidget::~VisualizationWidget()
{
    // BufferManager::release(m_buffer);
    // MM_FREE(m_buffer);  // delete[] m_buffer;
    delete[] m_points;
}

/*
void VisualizationWidget::updateSurroundBuffer(
        const surroundSampleFrame* _buffer)
{
    if(m_displayMode != Off && !m_frozen && !Engine::getSong()->isExporting())
    {
        // qWarning("VisualizationWidget::updateAudioBuffer");
        const Mixer* mixer = Engine::mixer();
        const fpp_t  fpp   = mixer->framesPerPeriod();
        m_masterOutput     = mixer->masterGain();

        mixer->getPeakValues(_buffer, fpp, m_peakLeft, m_peakRight);
        m_maxLevel = qMax<float>(m_peakLeft, m_peakRight) * m_masterOutput;

        if(sizeof(sampleFrame) == sizeof(surroundSampleFrame))
            m_ring.write(static_cast<const sampleFrame*>(_buffer), fpp);
        else
            for(fpp_t i = 0; i < fpp; i++)
                m_ring.write(static_cast<const sampleFrame&>(_buffer[i]));
        invalidateCache();
    }
}

void VisualizationWidget::updateStereoBuffer(const sampleFrame* _buffer)
{
    if(m_displayMode != Off && !m_frozen && !Engine::getSong()->isExporting())
    {
        // qWarning("VisualizationWidget::updateAudioBuffer");
        const Mixer* mixer = Engine::mixer();
        const fpp_t  fpp   = mixer->framesPerPeriod();
        m_masterOutput     = mixer->masterGain();

        mixer->getPeakValues(_buffer, fpp, m_peakLeft, m_peakRight);
        m_maxLevel = qMax<float>(m_peakLeft, m_peakRight) * m_masterOutput;

        m_ring.write(_buffer, fpp);
        invalidateCache();
    }
}
*/

void VisualizationWidget::applyChannelMode(float& _v0, float& _v1) const
{
    switch(m_channelMode)
    {
        case Left:
            _v1 = 0.f;
            break;
        case Right:
            _v0 = 0.f;
            break;
        case Stereo:
            break;
        case Joint:
            _v1 -= _v0;
            break;
        case Average:
        {
            float a = (_v0 + _v1) / 2.f;
            _v1     = (_v1 - _v0) / 2.f;
            _v0     = a;
        }
        break;
        case Difference:
        {
            float a = (_v0 + _v1) / 2.f;
            _v0     = (_v0 - a) / 2.f;
            _v1     = (_v1 - a) / 2.f;
        }
        break;
        case MinMax:
        {
            float a = qMin(_v0, _v1);
            float b = qMax(_v0, _v1);
            _v0     = a;
            _v1     = b;
        }
        case RhoTan:
            // todo
            break;
    }
}

void VisualizationWidget::setDisplayMode(DisplayMode _mode)
{
    if(m_displayMode == _mode)
        return;

    m_displayMode = _mode;
    // invalidateCache();

    if(m_displayMode != Off)
    {
        connect(gui->mainWindow(), SIGNAL(periodicUpdate()), this,
                SLOT(update()), Qt::UniqueConnection);
        /*
        if(m_inputMode == Surround)
        {
            connect(Engine::mixer(),
                    SIGNAL(nextAudioBuffer(const surroundSampleFrame*)), this,
                    SLOT(updateSurroundBuffer(const surroundSampleFrame*)));
        }
        */
    }
    else
    {
        disconnect(gui->mainWindow(), SIGNAL(periodicUpdate()), this,
                   SLOT(update()));
        /*
        if(m_inputMode == Surround)
        {
            disconnect(
                    Engine::mixer(),
                    SIGNAL(nextAudioBuffer(const surroundSampleFrame*)), this,
                    SLOT(updateSurroundBuffer(const surroundSampleFrame*)));
        }
        */
    }

    update();
}

void VisualizationWidget::setChannelMode(ChannelMode _mode)
{
    if(m_channelMode == _mode)
        return;

    m_channelMode = _mode;
    update();  // invalidateCache();
}

void VisualizationWidget::setFrozen(bool _frozen)
{
    if(m_ring->isFrozen() == _frozen)
        return;

    m_ring->setFrozen(_frozen);
    update();  // invalidateCache();
}

void VisualizationWidget::setStabilized(bool _stabilized)
{
    if(m_stabilized == _stabilized)
        return;

    m_stabilized = _stabilized;
    update();  // invalidateCache();
}

void VisualizationWidget::drawWidget(QPainter& _p)
{
    _p.setRenderHints(QPainter::Antialiasing, false);

    if(m_background.isNull())
        _p.fillRect(0, 0, width(), height(), Qt::black);
    else
        _p.drawPixmap(0, 0, m_background);

    _p.setRenderHints(QPainter::Antialiasing, true);

    {
        _p.setPen(Qt::white);  // Color(192, 192, 192));
        _p.setFont(pointSizeF(font(), 7.f));
        //_p.drawText(6, height() - 5, tr("Click to enable"));
        QString s("%1%2 %3%4 X%5 Y%6");
        s = s.arg(m_displayMode)
                    .arg(m_channelMode)
                    .arg(m_ring->isFrozen() ? "F" : "-")
                    .arg(m_stabilized ? "S" : "-")
                    .arg(m_zoomX, 0, 'f', 0)
                    .arg(m_zoomY, 0, 'f', 0);
        _p.drawText(2, height() - 2, s);
        _p.drawText(2, height() - 2, s);
    }

    if(m_displayMode != Off && !Engine::getSong()->isExporting())
    {
        // Mixer const* mixer = Engine::mixer();
        // qWarning("VisualizationWidget::masterGain %f",master_output);
        int         w      = width() - 4;
        int         h      = height() - 10 - 4;
        const float half_h = -h / 2.f;  // - 1.f; //* master_output
        const float x_base = 2.f;
        const float y_base = 2.f - half_h;  // + 2.f;

        _p.setClipRegion(QRect(2, 2, w, h));

        // _p.setRenderHint( QPainter::Antialiasing );

        // if(max_level>0.0001f)
        {
            const fpp_t fpp = Engine::mixer()->framesPerPeriod();

            int size  = m_ring->size();
            int fzero = -1;
            // int izero = 0;
            if(m_stabilized)
            {
                size = qMax<int>(fpp, size - fpp);
                for(fpp_t f = 1; f < fpp - 1; f++)
                {
                    float v0 = m_ring->value(-f, 0);
                    float v1 = m_ring->value(-f + 1, 0);
                    if(v1 <= 0.f && v0 >= 0.f)
                    {
                        if(fabsf(v1) < fabsf(v0))
                            fzero = -f + 1;
                        else
                            fzero = -f;
                        break;
                    }
                }
                // izero = (int)floorf(1.f * (nbp - 1) * fzero / (fpp - 1));
            }
            if(fzero>=0) fzero=-1;
            
            int nbp = 1;
            switch(m_displayMode)
            {
                case Off:
                    break;
                case TimeLast:
                    nbp = w / 2;
                    break;
                case Time:
                    nbp = w;
                    break;
                case XYLast:
                    nbp = qMax<int>(2, fpp / m_zoomX);
                    break;
                case XY:
                    nbp = qMax<int>(2, size / m_zoomX);
                    break;
            }

            if(nbp <= 1)
                return;
            if(nbp > size)
                nbp = size;
            if(nbp > m_pointCount)
                nbp = m_pointCount;

            for(ch_cnt_t ch = DEFAULT_CHANNELS - 1; ch >= 0; --ch)
            {
                if((ch == 0 && m_channelMode == Right)
                   || (ch == 1 && m_channelMode == Left))
                    continue;

                const float maxLevel = m_ring->maxLevel();
                QColor      c;
                if(maxLevel > 1.f)
                    c = QColor(255, 0, 0);  // QColor(255, 64, 64);
                else if(maxLevel > 0.9f)
                    c = QColor(255, 180, 0);  // QColor(255, 192, 64);
                else if(ch == 0)
                    c = QColor(0, 180, 255);  // QColor(71, 253, 133);
                else
                    c = QColor(0, 255, 180);  // QColor(71, 133, 253);
                _p.setPen(c);

                for(int i = 0; i < nbp; i++)
                {
                    const float tp    = (float)i / (float)nbp;
                    f_cnt_t     frame = 0;
                    float       xp    = 0.f;
                    float       yp    = 0.f;

                    switch(m_displayMode)
                    {
                        case Off:
                            break;
                        case TimeLast:
                        case XYLast:
                            frame = fpp * (1.f - tp);  // - 1;
                            break;
                        default:
                            frame = size * (1.f - tp);  // - 1;
                            break;
                    }

                    float v0 = m_ring->value(fzero - frame, 0) * m_zoomY;
                    float v1 = m_ring->value(fzero - frame, 1) * m_zoomY;
                    applyChannelMode(v0, v1);
                    switch(m_displayMode)
                    {
                        case Off:
                            break;
                        case TimeLast:
                        case Time:
                            xp = 1.f / (nbp - 1.f) * i;
                            yp = (ch == 0 ? v0 : v1);
                            xp = x_base + (m_zoomX * (w - 1.f) * xp)
                                 - (m_zoomX - 1.f) * (w - 1.f);
                            yp = y_base + half_h * yp;
                            break;
                        case XYLast:
                        case XY:
                            if(ch == 0)
                            {
                                xp = v0;
                                yp = v1;
                            }
                            else
                            {
                                xp = v1;
                                yp = v0;
                            }
                            xp = x_base + (w - 1.f) * 0.5f + half_h * xp;
                            yp = y_base + half_h * yp;
                            break;
                    }
                    m_points[i] = QPointF(xp, yp);
                }
                // if(m_displayMode != Time || !m_stabilized || izero == 0)
                {
                    //_p.drawPolyline(m_points, nbp);

                    QPainterPath path;
                    path.moveTo(m_points[0]);
                    // path.lineTo(m_points[1]);
                    for(int i = 1; i < nbp; i++)
                    {
                        QPointF& a = (i >= 2 ? m_points[i - 2] : m_points[0]);
                        QPointF& b = m_points[i - 1];
                        QPointF& c = m_points[i];
                        QPointF& d = (i < nbp - 1 ? m_points[i + 1]
                                                  : m_points[nbp - 1]);
                        QPointF  c1(b.x() + (c.x() - a.x()) / 4.f,
                                   b.y() + (c.y() - a.y()) / 4.f);
                        QPointF  c2(c.x() + (b.x() - d.x()) / 4.f,
                                   c.y() + (b.y() - d.y()) / 4.f);
                        path.cubicTo(c1, c2, c);
                    }
                    // path.lineTo(m_points[nbp - 1]);
                    _p.drawPath(path);
                }
                /*
                else
                {
                    _p.translate(x_base - m_points[izero].x(), 0.f);
                    _p.drawPolyline(m_points + izero, nbp - 1 - izero);
                    _p.translate(-x_base + m_points[izero].x(), 0.f);
                }
                */
            }
        }
    }

    _p.setRenderHints(QPainter::Antialiasing, false);
}

void VisualizationWidget::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::LeftButton)
    {
        if(_me->modifiers() & Qt::ShiftModifier)
        {
            setChannelMode(static_cast<ChannelMode>((m_channelMode + 1)
                                                    % ChannelModeCount));
        }
        else
        {
            setDisplayMode(static_cast<DisplayMode>((m_displayMode + 1)
                                                    % DisplayModeCount));
            if(m_displayMode != Off)
                setFrozen(false);
        }
    }
    if(_me->button() == Qt::MiddleButton)
    {
        if(m_displayMode != Off)
            setFrozen(!m_ring->isFrozen());
    }
    if(_me->button() == Qt::RightButton)
    {
        if(m_displayMode != Off)
            setStabilized(!m_stabilized);
    }
}

void VisualizationWidget::wheelEvent(QWheelEvent* _we)
{
    if(_we->modifiers() & Qt::ControlModifier)
    {
        _we->accept();
        m_zoomY = qBound(
                1.f, m_zoomY * ((_we->delta() > 0) ? F_2_SQRT : F_2_SQRT_R),
                256.f);
        update();
    }
    if(_we->modifiers() & Qt::ShiftModifier)
    {
        _we->accept();
        m_zoomX = qBound(
                1.f, m_zoomX * ((_we->delta() > 0) ? F_2_SQRT : F_2_SQRT_R),
                256.f);
        update();
    }
}

void VisualizationWidget::invalidateCache()
{
    // if(!m_ring->isFrozen())
    Widget::invalidateCache();
}

void VisualizationWidget::update()
{
    // qInfo("VisualizationWidget::update");
    // if(!m_frozen)
    Widget::update();
    // else
    // PaintManager::updateLater(this);
}
