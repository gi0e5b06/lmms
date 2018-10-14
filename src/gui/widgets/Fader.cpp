/*
 * Fader.cpp - fader-widget used in mixer - partly taken from Hydrogen
 *
 * Copyright (c) 2008-2012 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

/*
 * Hydrogen
 * Copyright(c) 2002-2008 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "Fader.h"

#include "CaptionMenu.h"
#include "ConfigManager.h"
#include "TextFloat.h"
//#include "MainWindow.h"
//#include "GuiApplication.h"

//#include "debug.h"
#include "embed.h"
#include "lmms_math.h"

#include <QInputDialog>
#include <QMouseEvent>
#include <QPainter>

TextFloat* Fader::s_textFloat = NULL;

Fader::Fader(FloatModel* _model, const QString& _name, QWidget* _parent) :
      Widget(_parent), FloatModelView(_model, this),
      // m_needsUpdate( true ),
      m_fPeakValue_L(0.0), m_fPeakValue_R(0.0), m_persistentPeak_L(0.0),
      m_persistentPeak_R(0.0), m_fMinPeak(0.01f), m_fMaxPeak(1.1),
      m_displayConversion(true), m_levelsDisplayedInDBFS(false),
      m_moveStartPoint(-1), m_startValue(0), m_peakGreen(0, 0, 0),
      m_peakRed(0, 0, 0), m_peakYellow(0, 0, 0)
{
    if(s_textFloat == NULL)
    {
        s_textFloat = new TextFloat;
    }

    /*
    if( ! s_back )
    {
            s_back = new QPixmap( embed::getIconPixmap( "fader_background" )
    );
    }

    if( ! s_leds )
    {
            s_leds = new QPixmap( embed::getIconPixmap( "fader_leds" ) );
    }
    if( ! s_knob )
    {
            s_knob = new QPixmap( embed::getIconPixmap( "fader_knob" ) );
    }
    */

    m_back = embed::getPixmap("fader_background");
    m_leds = embed::getPixmap("fader_leds");
    m_knob = embed::getPixmap("fader_knob");

    init(_model, _name);
}

Fader::Fader(FloatModel*    model,
             const QString& name,
             QWidget*       parent,
             const QPixmap& back,
             const QPixmap& leds,
             const QPixmap& knob) :
      Widget(parent),
      FloatModelView(model, this), m_fPeakValue_L(0.0), m_fPeakValue_R(0.0),
      m_persistentPeak_L(0.0), m_persistentPeak_R(0.0), m_fMinPeak(0.01f),
      m_fMaxPeak(1.1), m_displayConversion(false),
      m_levelsDisplayedInDBFS(false), m_moveStartPoint(-1), m_startValue(0),
      m_peakGreen(0, 0, 0), m_peakRed(0, 0, 0)
{
    if(s_textFloat == NULL)
    {
        s_textFloat = new TextFloat;
    }

    m_back = back;
    m_leds = leds;
    m_knob = knob;

    init(model, name);
}

Fader::~Fader()
{
}

void Fader::init(FloatModel* model, QString const& name)
{
    setWindowTitle(name);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    QSize backgroundSize = m_back.size();
    setMinimumSize(backgroundSize);
    setMaximumSize(backgroundSize);
    resize(backgroundSize);
    setModel(model);
    setHintText("Volume:", "%");
}

void Fader::modelChanged()
{
    // qInfo("Fader::modelChanged()");
    update();
}

void Fader::contextMenuEvent(QContextMenuEvent* _ev)
{
    CaptionMenu contextMenu(windowTitle());
    addDefaultActions(&contextMenu);
    contextMenu.exec(QCursor::pos());
    _ev->accept();
}

void Fader::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    if(m_moveStartPoint >= 0)
    {
        int dy = m_moveStartPoint - mouseEvent->globalY();

        real_t delta = dy * (model()->maxValue() - model()->minValue())
                       / (real_t)(height() - m_knob.height());

        const real_t step     = model()->step<real_t>();
        real_t       newValue = static_cast<real_t>(static_cast<int>(
                                  (m_startValue + delta) / step + 0.5))
                          * step;
        model()->setValue(newValue);

        updateTextFloat();
    }
}

void Fader::mousePressEvent(QMouseEvent* mouseEvent)
{
    if(mouseEvent->button() == Qt::LeftButton
       && !(mouseEvent->modifiers() & Qt::ControlModifier))
    {
        AutomatableModel* thisModel = model();
        if(thisModel)
        {
            thisModel->addJournalCheckPoint();
            thisModel->saveJournallingState(false);
        }

        if(mouseEvent->y() >= knobPosY() - m_knob.height()
           && mouseEvent->y() < knobPosY())
        {
            updateTextFloat();
            s_textFloat->show();

            m_moveStartPoint = mouseEvent->globalY();
            m_startValue     = model()->rawValue();

            mouseEvent->accept();
        }
        else
        {
            m_moveStartPoint = -1;
        }
    }
    else
    {
        AutomatableModelView::mousePressEvent(mouseEvent);
    }
}

void Fader::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    enterValue();
}

void Fader::enterValue()
{
    bool   ok;
    real_t newValue;
    // TODO: dbV handling
    if(m_displayConversion)
    {
        newValue = QInputDialog::getDouble(
                           this, windowTitle(),
                           tr("Please enter a new value between %1 and %2:")
                                   .arg(model()->minValue() * 100)
                                   .arg(model()->maxValue() * 100),
                           model()->getRoundedValue() * 100,
                           model()->minValue() * 100,
                           model()->maxValue() * 100,
                           model()->getDigitCount(), &ok)
                   * 0.01f;
    }
    else
    {
        newValue = QInputDialog::getDouble(
                this, windowTitle(),
                tr("Please enter a new value between %1 and %2:")
                        .arg(model()->minValue())
                        .arg(model()->maxValue()),
                model()->getRoundedValue(), model()->minValue(),
                model()->maxValue(), model()->getDigitCount(), &ok);
    }

    if(ok)
    {
        model()->setValue(newValue);
    }
}

void Fader::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    if(mouseEvent && mouseEvent->button() == Qt::LeftButton)
    {
        AutomatableModel* thisModel = model();
        if(thisModel)
        {
            thisModel->restoreJournallingState();
        }
    }

    s_textFloat->hide();
}

void Fader::wheelEvent(QWheelEvent* _we)
{
    if(_we->modifiers() & Qt::ShiftModifier)
    {
        _we->accept();
        if(_we->delta() > 0)
            model()->incrValue();
        else if(_we->delta() < 0)
            model()->decrValue();

        updateTextFloat();
        s_textFloat->setVisibilityTimeOut(1000);
    }
}

///
/// Set peak value (0.0 .. 1.0)
///
void Fader::setPeak(real_t  fPeak,
                    real_t& targetPeak,
                    real_t& persistentPeak,
                    QTime&  lastPeakTime)
{
    if(fPeak < m_fMinPeak)
    {
        fPeak = m_fMinPeak;
    }
    else if(fPeak > m_fMaxPeak)
    {
        fPeak = m_fMaxPeak;
    }

    if(targetPeak != fPeak)
    {
        targetPeak = fPeak;
        if(targetPeak >= persistentPeak)
        {
            persistentPeak = targetPeak;
            lastPeakTime.restart();
        }
        update();
        // m_needsUpdate=true;
    }

    if(persistentPeak > 0. && lastPeakTime.elapsed() >= 500)
    {
        persistentPeak = qMax<real_t>(0, persistentPeak - 0.0125);
        update();
        // m_needsUpdate=true;
    }
}

void Fader::setPeak_L(real_t fPeak)
{
    setPeak(fPeak, m_fPeakValue_L, m_persistentPeak_L, m_lastPeakTime_L);
}

void Fader::setPeak_R(real_t fPeak)
{
    setPeak(fPeak, m_fPeakValue_R, m_persistentPeak_R, m_lastPeakTime_R);
}

// update tooltip showing value and adjust position while changing fader value
void Fader::updateTextFloat()
{
    if(ConfigManager::inst()->value("app", "displaydbfs").toInt()
       && m_displayConversion)
    {
        s_textFloat->setText(
                m_description.trimmed() + " "
                + QString::number(ampToDbfs(model()->rawValue()), 'f', 2)
                + " dBFS");
    }
    else
    {
        s_textFloat->setText(
                m_description.trimmed() + " "
                + QString::number(
                          m_displayConversion ? model()->rawValue() * 100
                                              : model()->rawValue(),
                          'f',
                          m_displayConversion
                                  ? qMax(0, model()->getDigitCount() - 2)
                                  : model()->getDigitCount())
                + " " + m_unit);
    }
    s_textFloat->moveGlobal(
            this, QPoint(width() - m_knob.width() - 5, knobPosY() - 46));
}

inline int Fader::calculateDisplayPeak(real_t fPeak)
{
    int peak = (int)(m_back.height()
                     - (fPeak / (m_fMaxPeak - m_fMinPeak)) * m_back.height());

    return qMin(peak, m_back.height());
}

void Fader::drawWidget(QPainter& _p)
{
    // Draw the background
    //_p.drawPixmap( ev->rect(), *m_back, ev->rect() );
    _p.drawPixmap(0, 0, m_back);

    // Draw the levels with peaks
    /*
    if (areLevelsDisplayedInDBFS())
    {
            paintDBFSLevels(ev, _p);
    }
    else
    {
            paintLinearLevels(ev, _p);
    }
    */
    if(m_levelsDisplayedInDBFS)
        drawDBFSLevels(_p);
    else
        drawLinearLevels(_p);

    // Draw the knob
    _p.drawPixmap(0, knobPosY() - m_knob.height(), m_knob);
}

void Fader::drawDBFSLevels(QPainter& _p)
{
    int height = m_back.height();
    int width  = m_back.width() / 2;
    int center = m_back.width() - width;

    real_t const maxDB(ampToDbfs(m_fMaxPeak));
    real_t const minDB(ampToDbfs(m_fMinPeak));

    // We will need to divide by the span between min and max several times.
    // It's more efficient to calculate the reciprocal once and then to
    // multiply.
    real_t const fullSpanReciprocal = 1 / (maxDB - minDB);

    // Draw left levels
    real_t const leftSpan
            = ampToDbfs(qMax<real_t>(0.0001, m_fPeakValue_L)) - minDB;
    int   peak_L = height * leftSpan * fullSpanReciprocal;
    QRect drawRectL(0, height - peak_L, width,
                    peak_L);  // Source and target are identical
    _p.drawPixmap(drawRectL, m_leds, drawRectL);

    real_t const persistentLeftPeakDBFS
            = ampToDbfs(qMax<real_t>(0.0001, m_persistentPeak_L));
    int persistentPeak_L
            = height
              * (1 - (persistentLeftPeakDBFS - minDB) * fullSpanReciprocal);
    // the LED's have a  4px padding and we don't want the peaks
    // to draw on the fader background
    if(persistentPeak_L <= 4)
    {
        persistentPeak_L = 4;
    }
    if(persistentLeftPeakDBFS > minDB)
    {
        QColor const& peakColor = isClipping(m_persistentPeak_L)
                                          ? peakRed()
                                          : persistentLeftPeakDBFS >= -6
                                                    ? peakYellow()
                                                    : peakGreen();
        _p.fillRect(QRect(2, persistentPeak_L, 7, 1), peakColor);
    }

    // Draw right levels
    real_t const rightSpan
            = ampToDbfs(qMax<real_t>(0.0001, m_fPeakValue_R)) - minDB;
    int         peak_R = height * rightSpan * fullSpanReciprocal;
    QRect const drawRectR(center, height - peak_R, width,
                          peak_R);  // Source and target are identical
    _p.drawPixmap(drawRectR, m_leds, drawRectR);

    real_t const persistentRightPeakDBFS
            = ampToDbfs(qMax<real_t>(0.0001, m_persistentPeak_R));
    int persistentPeak_R
            = height
              * (1 - (persistentRightPeakDBFS - minDB) * fullSpanReciprocal);
    // the LED's have a  4px padding and we don't want the peaks
    // to draw on the fader background
    if(persistentPeak_R <= 4)
    {
        persistentPeak_R = 4;
    }
    if(persistentRightPeakDBFS > minDB)
    {
        QColor const& peakColor = isClipping(m_persistentPeak_R)
                                          ? peakRed()
                                          : persistentRightPeakDBFS >= -6
                                                    ? peakYellow()
                                                    : peakGreen();
        _p.fillRect(QRect(14, persistentPeak_R, 7, 1), peakColor);
    }
}

void Fader::drawLinearLevels(QPainter& _p)
{
    // peak leds
    // real_t fRange = abs( m_fMaxPeak ) + abs( m_fMinPeak );

    int height = m_back.height();
    int width  = m_back.width() / 2;
    int center = m_back.width() - width;

    int peak_L           = calculateDisplayPeak(m_fPeakValue_L - m_fMinPeak);
    int persistentPeak_L = qMax<int>(
            3, calculateDisplayPeak(m_persistentPeak_L - m_fMinPeak));
    _p.drawPixmap(QRect(0, peak_L, width, height - peak_L), m_leds,
                  QRect(0, peak_L, width, height - peak_L));

    if(m_persistentPeak_L > 0.05)
    {
        _p.fillRect(QRect(2, persistentPeak_L, 7, 1),
                    (m_persistentPeak_L < 1.0) ? peakGreen() : peakRed());
    }

    int peak_R           = calculateDisplayPeak(m_fPeakValue_R - m_fMinPeak);
    int persistentPeak_R = qMax<int>(
            3, calculateDisplayPeak(m_persistentPeak_R - m_fMinPeak));
    _p.drawPixmap(QRect(center, peak_R, width, height - peak_R), m_leds,
                  QRect(center, peak_R, width, height - peak_R));

    if(m_persistentPeak_R > 0.05)
    {
        _p.fillRect(QRect(14, persistentPeak_R, 7, 1),
                    (m_persistentPeak_R < 1.0) ? peakGreen() : peakRed());
    }
}

QColor const& Fader::peakGreen() const
{
    return m_peakGreen;
}

QColor const& Fader::peakRed() const
{
    return m_peakRed;
}

QColor const& Fader::peakYellow() const
{
    return m_peakYellow;
}

void Fader::setPeakGreen(const QColor& c)
{
    m_peakGreen = c;
}

void Fader::setPeakRed(const QColor& c)
{
    m_peakRed = c;
}

void Fader::setPeakYellow(const QColor& c)
{
    m_peakYellow = c;
}

void Fader::update()
{
    // qInfo("Fader::update");
    invalidateCache();
    friendlyUpdate();
    // Widget::update();
}

void Fader::friendlyUpdate()
{
    // qInfo("Fader::friendlyUpdate");
    // if(!isCacheValid())
    Widget::update();  // PaintManager::updateLater(this);
}

void Fader::doConnections()
{
    FloatModel* m = model();
    if(m)
    {
        // qInfo("Fader::doConnections p=%p model()=%p", this, m);
        m->disconnect(this);
        connect(m, SIGNAL(dataChanged()), this, SLOT(friendlyUpdate()));
        connect(m, SIGNAL(propertiesChanged()), this, SLOT(update()));
        // QObject::connect( m, SIGNAL( controllerValueChanged() ),
        //                  this, SLOT( refresh() ) );
    }
}
