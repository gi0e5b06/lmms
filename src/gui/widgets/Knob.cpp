/*
 * Knob.cpp - knob widget
 *
 * Copyright (c) 2017-2018 gi0e5b06
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "Knob.h"

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include "CaptionMenu.h"
#include "ConfigManager.h"
#include "ControllerConnection.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "PaintManager.h"
#include "ProjectJournal.h"
//#include "Song.h"
#include "StringPairDrag.h"
#include "TextFloat.h"
#include "embed.h"
#include "gui_templates.h"
//#include "debug.h"
#include "lmms_math.h"
//#include "PerfLog.h"

#include <QApplication>
//#include <QBitmap>
#include <QFontMetrics>
#include <QInputDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QWhatsThis>

TextFloat* Knob::s_textFloat = NULL;

// new FloatModel(0.7f,0.f,1.f,0.01f, NULL, _name, true ), this ),

Knob::Knob(QWidget* _parent, const QString& _name) :
      Knob(knobBright_26, _parent, _name)
{
}

Knob::Knob(knobTypes _knob_num, QWidget* _parent, const QString& _name) :
      Widget(_parent), FloatModelView(NULL, this), m_pressLeft(false),
      m_label(""), m_knobPixmap(NULL), m_volumeKnob(false),
      m_volumeRatio(100.0, 0.0, 1000000.0), m_angle(-1000.f),
      // m_cache( NULL ),
      m_lineWidth(0.f), m_textColor(255, 255, 255), m_knobNum(_knob_num)
{
    initUi(_name);
}

void Knob::initUi(const QString& _name)
{
    qRegisterMetaType<knobTypes>("knobTypes");

    if(s_textFloat == NULL)
    {
        s_textFloat = new TextFloat;
    }

    setWindowTitle(_name);
    setCursor(Qt::PointingHandCursor);
    // setAttribute(Qt::WA_OpaquePaintEvent);

    onKnobNumUpdated();
    setTotalAngle(270.0f);
    setInnerRadius(1.0f);
    setOuterRadius(10.0f);
    setFocusPolicy(Qt::ClickFocus);

    // This is a workaround to enable style sheets for knobs which are not
    // styled knobs.
    //
    // It works as follows: the palette colors that are assigned as the line
    // color previously had been hard coded in the drawKnob method for the
    // different knob types. Now the drawKnob method uses the line color to
    // draw the lines. By assigning the palette colors as the line colors here
    // the knob lines will be drawn in this color unless the stylesheet
    // overrides that color.
    switch(knobNum())
    {
        case knobSmall_17:
        case knobBright_26:
        case knobDark_28:
            setlineColor(QApplication::palette().color(QPalette::Active,
                                                       QPalette::WindowText));
            break;
        case knobVintage_32:
            setlineColor(QApplication::palette().color(QPalette::Active,
                                                       QPalette::Shadow));
            break;
        default:
            break;
    }

    m_statusColor = Qt::black;
    setPointColor(lineColor());
    // doConnections();
    // update();
}

void Knob::onKnobNumUpdated()
{
    if(m_knobNum != knobStyled)
    {
        QString knobFilename;
        switch(m_knobNum)
        {
            case knobDark_28:
                knobFilename = "knob01";
                break;
            case knobBright_26:
                knobFilename = "knob02";
                break;
            case knobSmall_17:
                knobFilename = "knob03";
                break;
            case knobVintage_32:
                knobFilename = "knob05";
                break;
            case knobStyled:  // only here to stop the compiler from
                              // complaining
                break;
        }

        // If knobFilename is still empty here we should get the fallback
        // pixmap of size 1x1
        m_knobPixmap = new QPixmap(
                embed::getIconPixmap(knobFilename.toUtf8().constData()));

        setMinimumSize(m_knobPixmap->width(), m_knobPixmap->height());
        resize(m_knobPixmap->width(), m_knobPixmap->height());
        update();
    }
}

Knob::~Knob()
{
    if(m_knobPixmap)
        delete m_knobPixmap;
    // if( m_cache ) delete m_cache;
}

void Knob::setLabel(const QString& txt)
{
    setText(txt);
}

QString Knob::text() const
{
    return m_label;
}

void Knob::setText(const QString& txt)
{
    if(m_label == txt)
        return;

    if(m_knobPixmap)
    {
        int w = m_knobPixmap->width();
        int h = m_knobPixmap->height();
        if(!txt.isEmpty())
        {
            QFontMetrics mx(pointSizeF(font(), 6.5));
            w = qMax<int>(w, qMin<int>(2 * w, mx.width(txt)));
            h += 10;
        }
        setMinimumSize(w, h);
        resize(w, h);
    }
    m_label = txt;
    update();
}

void Knob::setTotalAngle(float angle)
{
    if(angle < 10.f)
        angle = 10.f;

    if(angle != m_totalAngle)
    {
        m_totalAngle = angle;
        update();
    }
}

float Knob::innerRadius() const
{
    return m_innerRadius;
}

void Knob::setInnerRadius(float r)
{
    m_innerRadius = r;
}

float Knob::outerRadius() const
{
    return m_outerRadius;
}

void Knob::setOuterRadius(float r)
{
    m_outerRadius = r;
}

knobTypes Knob::knobNum() const
{
    return m_knobNum;
}

void Knob::setknobNum(knobTypes k)
{
    if(m_knobNum != k)
    {
        m_knobNum = k;
        onKnobNumUpdated();
    }
}

QPointF Knob::centerPoint() const
{
    return m_centerPoint;
}

float Knob::centerPointX() const
{
    return m_centerPoint.x();
}

void Knob::setCenterPointX(float c)
{
    m_centerPoint.setX(c);
}

float Knob::centerPointY() const
{
    return m_centerPoint.y();
}

void Knob::setCenterPointY(float c)
{
    m_centerPoint.setY(c);
}

float Knob::lineWidth() const
{
    return m_lineWidth;
}

void Knob::setLineWidth(float w)
{
    m_lineWidth = w;
}

QColor Knob::outerColor() const
{
    return m_outerColor;
}

void Knob::setOuterColor(const QColor& c)
{
    m_outerColor = c;
}

QColor Knob::lineColor() const
{
    return m_lineColor;
}

void Knob::setlineColor(const QColor& c)
{
    m_lineColor = c;
}

QColor Knob::arcColor() const
{
    return m_arcColor;
}

void Knob::setarcColor(const QColor& c)
{
    m_arcColor = c;
}

QColor Knob::pointColor() const
{
    return m_pointColor;
}

void Knob::setPointColor(const QColor& c)
{
    if(m_pointColor != c)
    {
        m_pointColor = c;
        update();
    }
}

QColor Knob::textColor() const
{
    return m_textColor;
}

void Knob::setTextColor(const QColor& c)
{
    if(m_textColor != c)
    {
        m_textColor = c;
        update();
    }
}

QLineF Knob::calculateLine(const QPointF& _mid,
                           float          _radius,
                           float          _innerRadius) const
{
    const float rarc = m_angle * F_PI / 180.f;
    const float ca   = cos(rarc);
    const float sa   = -sin(rarc);

    return QLineF(_mid.x() - sa * _innerRadius, _mid.y() - ca * _innerRadius,
                  _mid.x() - sa * _radius, _mid.y() - ca * _radius);
}

bool Knob::updateAngle()
{
    FloatModel* m = model();
    if(!m)
        return true;

    float angle = 0.f;
    if(m)  //&& m->maxValue() != m->minValue() )
    {
        // angle = angleFromValue( m->inverseScaledValue( m->value() ),
        // m->minValue(), m->maxValue(), m_totalAngle ); float
        // v=m->minValue()+qRound( (m->value()-m->minValue()) /
        //                              m->step<float>() ) * m->step<float>();
        // v=m->normalizedValue(v);
        float v = m->value();
        if(m->isScaleLogarithmic())
            v = ::linearToLogScale(m->minValue(), m->maxValue(), v);
        v     = (v - m->minValue()) / m->range();
        angle = m_totalAngle * (v - 0.5f);
    }
    if(qAbs(angle - m_angle) > 3.f)
    {
        m_angle = angle;
        return true;
    }
    return false;
}

float Knob::angleFromValue(float value,
                           float minValue,
                           float maxValue,
                           float totalAngle) const
{
    // return fmodf((value - 0.5f*(minValue + maxValue)) / (maxValue -
    // minValue)*m_totalAngle,360);
    return m_totalAngle * (fmodf(value, 1.f) - 0.5f);
}

QColor Knob::statusColor()
{
    FloatModel* m = model();
    QColor      r(255, 255, 255, 192);

    if(!m)
        r = QColor(128, 128, 128, 192);  // no model...
    else if(m->isAutomated())
        r = QColor(128, 128, 255, 192);  // automation pattern
    else if(m->isControlled())
        r = QColor(255, 192, 0, 192);  // midi
    else if(m->hasLinkedModels())
        r = QColor(128, 128, 255, 192);  // link

    return r;
}

void Knob::drawWidget(QPainter& _p)
{
    drawKnob(_p);
    drawText(_p);
}

void Knob::drawKnob(QPainter& _p)
{
    FloatModel* m = model();
    if(!m)
        return;

    QPointF mid;
    QColor  pc = pointColor();
    if(isVolumeKnob())
        pc = Qt::red;
    // QColor lc=pc;//lineColor();
    int lw = lineWidth();

    if(m_knobNum == knobStyled)
    {
        mid = centerPoint();
        // _p.setRenderHint( QPainter::Antialiasing );

        // Perhaps this can move to setOuterRadius()
        if(m_outerColor.isValid())
        {
            QRadialGradient gradient(mid, outerRadius());
            gradient.setColorAt(0.4, _p.pen().brush().color());
            gradient.setColorAt(1, m_outerColor);

            _p.setPen(QPen(gradient, lw, Qt::SolidLine, Qt::RoundCap));
        }
        else
        {
            QPen pen = _p.pen();
            pen.setWidth(lw);
            pen.setCapStyle(Qt::RoundCap);

            _p.setPen(pen);
        }

        _p.drawLine(calculateLine(mid, outerRadius(), innerRadius()));
    }
    else if(m)
    {
        // Old-skool knobs
        const float radius = m_knobPixmap->width() / 2.0f - 1.f;
        mid = QPointF(width() / 2.f, m_knobPixmap->height() / 2.f);

        /*
        _p.drawPixmap( QPointF((width()-m_knobPixmap->width())/2.f,
                              0.f),*m_knobPixmap );
        */

        // _p.setRenderHint( QPainter::Antialiasing );

        const float centerAngle
                = angleFromValue(m->inverseScaledValue(m->centerValue()),
                                 m->minValue(), m->maxValue(), m_totalAngle);

        const float arcLineWidth = 3.f;
        const float arcRectSize
                = m_knobPixmap->width() - arcLineWidth - 2.f;  // 0

        /*
        QColor col;
        if( m_knobNum == knobVintage_32 )
                {	col = QApplication::palette().color( QPalette::Active,
        QPalette::Shadow ); } else {	col = QApplication::palette().color(
        QPalette::Active, QPalette::WindowText ); } col.setAlpha( 70 );
        */

        QLineF line;
        switch(m_knobNum)
        {
                /*
        case knobSmall_17:
                reline=calculateLine( mid, radius-2 );
                        break;
        case knobBright_26:
                        line=calculateLine( mid, radius-2 ); //-5
                        break;
                */
            case knobDark_28:
            {
                const float rb = qMax<float>((radius - 10) / 3.0, 0.0);
                const float re = qMax<float>((radius - 4), 0.0);
                line           = calculateLine(mid, re, rb);
                line.translate(1, 1);
            }
            break;
            case knobVintage_32:
                line = calculateLine(mid, radius - 2, 2);
                break;
            default:
                line = calculateLine(mid, radius);
                break;
        }

        QPen pen0(QColor(255, 255, 255, 192), 1, Qt::SolidLine, Qt::RoundCap);
        _p.setPen(pen0);

        _p.setBrush(m_statusColor);
        float re = radius;  // + 2.f;
        _p.drawEllipse(mid.x() - re / 2.f, mid.y() - re / 2.f, re,
                       re);  //+1.f,re+1.f);

        QPen pen1(QColor(0, 0, 0, 96), 2, Qt::SolidLine, Qt::RoundCap);
        _p.setPen(pen1);
        _p.drawArc(mid.x() - arcRectSize / 2.f, 1.f, arcRectSize, arcRectSize,
                   16.f * 315, 16.f * m_totalAngle);

        QPen pen2(QColor(0, 0, 0, 96), 4, Qt::SolidLine, Qt::RoundCap);
        _p.setPen(pen2);
        _p.drawLine(line);
        _p.drawArc(mid.x() - arcRectSize / 2.f, 1.f, arcRectSize, arcRectSize,
                   16.f * (90.f - centerAngle),
                   -16.f * (m_angle - centerAngle));
        _p.setBrush(QColor(0, 0, 0, 96));
        // _p.drawEllipse(mid.x()-2.f,mid.y()-2.f,5.f,5.f);
        _p.drawEllipse(mid.x() - 1.f, mid.y() - 1.f, 3.f, 3.f);

        QPen pen3(pc, 2, Qt::SolidLine, Qt::RoundCap);
        _p.setPen(pen3);
        _p.drawLine(line);
        _p.drawArc(mid.x() - arcRectSize / 2, 1, arcRectSize, arcRectSize,
                   16.f * (90.f - centerAngle),
                   -16.f * (m_angle - centerAngle));
        _p.setBrush(pc);
        _p.drawEllipse(mid.x() - 1.f, mid.y() - 1.f, 3.f, 3.f);
    }
}

void Knob::drawText(QPainter& _p)
{
    if(!m_label.isEmpty())
    {
        _p.setFont(pointSizeF(_p.font(), 6.5));
        _p.setPen(textColor());
        QFontMetrics metrix = _p.fontMetrics();
        QString text = metrix.elidedText(m_label, Qt::ElideRight, width());
        int     x    = width() / 2 - metrix.width(text) / 2;
        int     y    = height() - 2;
        if(m_knobPixmap)
            y = qMin(y, m_knobPixmap->height() + 7);
        _p.drawText(x, y, text);
    }
}

/*
void Knob::resizeEvent(QResizeEvent* _re)
{
    resizeCache(width(), height());
    // qInfo("Knob::resizeEvent()");
    // invalidateCache(); update();
    refresh();
}
*/

void Knob::setModel(Model* _m, bool isOldModelValid)
{
    // qInfo("Knob::setModel p=%p",this);
    FloatModelView::setModel(_m, isOldModelValid);
    // update();
}

void Knob::modelChanged()
{
    // qInfo("Knob::modelChanged p=%p", this);
    update();  // refresh();
}

void Knob::convert(const QPoint& _p, float& value_, float& dist_)
{
    // arcane mathemagicks for calculating knob movement
    // value = ( ( _p.y() + _p.y() * qMin( qAbs( _p.y() / 2.5f ), 6.0f ) ) )
    // / 12.0f;

    dist_ = (_p.x() * _p.x() + _p.y() * _p.y() - 80.f) / 6400.f;
    if(dist_ < 0.f)
        dist_ = 0.f;
    if(dist_ > 1.f)
        dist_ = 1.f;

    float angle
            = fmodf(atan2(_p.y() / dist_, _p.x() / dist_) + 3.f * M_PI / 2.,
                    2.f * M_PI);
    angle  = qMax(0.80f, qMin(5.49f, angle));
    value_ = (angle - 0.80f) / (5.49f - 0.80f);

    // qInfo("x=%d y=%d d=%f a=%f v=%f",_p.x(),_p.y(),dist_,angle,value_);
    if(value_ < 0.f)
        value_ = 0.f;
    if(value_ > 1.f)
        value_ = 1.f;

    // if shift pressed we want slower movement
    /*
    if( gui->mainWindow()->isShiftPressed() )
    {
            value /= 4.0f;
            value = qBound( -4.0f, value, 4.0f );
    }
    */
    // return value;// * pageSize();
}

void Knob::setPosition(const QPoint& _p, bool _shift)
{
    FloatModel* m = model();
    if(!m)
        return;

    float value, dist;
    convert(_p, value, dist);

    // const float step = model()->step<float>();
    // const float oldValue = model()->value();

    if(_shift)
    {
        m_pressValue = 0.3f * value + 0.7f * m->normalizedValue(m->value());
        dist /= 4.f;
        // qInfo("shift pv=%f dist=%f",m_pressValue,dist);
    }

    // if( m->isScaleLogarithmic() ) // logarithmic code
    //{
    /*
    const float pos = m->minValue() < 0
            ? oldValue / qMax( qAbs( m->maxValue() ), qAbs( m->minValue() ) )
            : ( oldValue - m->minValue() ) / m->range();
    const float ratio = 0.1f + qAbs( pos ) * 15.f;
    float newValue = value * ratio;
    if( qAbs( newValue ) >= step )
    {
            float roundedValue = qRound( ( oldValue - value ) / step ) * step;
            m->setValue( roundedValue );
            m_leftOver = 0.0f;
    }
    else
    {
            m_leftOver = value;
    }
    */
    /*                float v=m->value();
            if(m->isScaleLogarithmic())
       v=::linearToLogScale(m->minValue(),m->maxValue(),v);
            v=(v-m->minValue())/m->range();
            angle=m_totalAngle*(v-0.5f);
    */
    float v = dist * value + (1.f - dist) * m_pressValue;
    if(m->isScaleLogarithmic())
        v = ::logToLinearScale(0.f, 1.f, v);
    v = m->minValue() + m->range() * v;
    m->setValue(v);
    // m_leftOver = 0.0f;
    //	}

    // else // linear code
    //	{
    /*
    if( qAbs( value ) >= step )
    {
            float roundedValue = qRound( ( oldValue - value ) / step ) * step;
            m->setValue( roundedValue );
            m_leftOver = 0.0f;
    }
    else
    {
            m_leftOver = value;
    }
    */
    // float roundedValue=qRound( (dist*value*m->range()+
    //(1.f-dist)*(m_pressValue-m->minValue()))
    //        / step ) * step;
    // m->setValue( m->minValue()+qMax(0.f,qMin(roundedValue,m->range())));
    // m_leftOver = 0.0f;
    //}
}

void Knob::contextMenuEvent(QContextMenuEvent*)
{
    FloatModel* m = model();
    if(!m)
        return;

    // for the case, the user clicked right while pressing left mouse-
    // button, the context-menu appears while mouse-cursor is still hidden
    // and it isn't shown again until user does something which causes
    // an QApplication::restoreOverrideCursor()-call...
    // mouseReleaseEvent( NULL );

    CaptionMenu contextMenu(model()->displayName(), this);
    addDefaultActions(&contextMenu);
    contextMenu.addAction(QPixmap(),
                          m->isScaleLogarithmic() ? tr("Set linear")
                                                  : tr("Set logarithmic"),
                          this, SLOT(toggleScale()));
    contextMenu.addSeparator();
    contextMenu.addHelpAction();
    contextMenu.exec(QCursor::pos());
}

void Knob::toggleScale()
{
    FloatModel* m = model();
    if(!m)
        return;

    m->setScaleLogarithmic(!m->isScaleLogarithmic());
    // update();
}

void Knob::dragEnterEvent(QDragEnterEvent* _dee)
{
    FloatModel* m = model();
    if(!m)
        return;

    StringPairDrag::processDragEnterEvent(_dee,
                                          "float_value,automatable_model");
}

void Knob::dropEvent(QDropEvent* _de)
{
    FloatModel* m = model();
    if(!m)
        return;

    QString type = StringPairDrag::decodeKey(_de);
    QString val  = StringPairDrag::decodeValue(_de);
    if(type == "float_value")
    {
        m->setValue(val.toFloat());
        _de->accept();
        update();
    }
    else if(type == "automatable_model")
    {
        if(!m->controllerConnection())
        {
            AutomatableModel* mod = dynamic_cast<AutomatableModel*>(
                    Engine::projectJournal()->journallingObject(val.toInt()));
            if(mod)
            {
                // not a midi or dummy
                // if(! mod->controllerConnection() )
                if(mod->controllerConnection())
                    qInfo("src Knob has a controller Connection");
                else
                    qInfo("src Knob has NO controller Connection");

                {
                    AutomatableModel::linkModels(m, mod);
                    mod->setValue(m->value());
                    _de->accept();
                    mod->emit propertiesChanged();
                    update();
                }
            }
        }
        else
            qInfo("dst Knob has a controller Connection");
    }
}

void Knob::mousePressEvent(QMouseEvent* _me)
{
    FloatModel* m = model();
    if(!m)
        return;

    if(_me->button() == Qt::LeftButton
       && !(_me->modifiers() & Qt::ControlModifier)
       && !(_me->modifiers() & Qt::ShiftModifier))
    {
        m->addJournalCheckPoint();
        m->saveJournallingState(false);

        if(m)
            m_pressValue = m->normalizedValue(m->value());
        else
            m_pressValue = 0.f;

        m_pressPos  = _me->pos();
        m_pressLeft = true;
        // const QPoint & p = _me->pos();
        // m_origMousePos = p;
        // m_mouseOffset = QPoint(0, 0);
        // m_leftOver = 0.0f;

        emit sliderPressed();

        setCursor(Qt::SizeAllCursor);
        // QApplication::setOverrideCursor( Qt::BlankCursor );
        s_textFloat->setText(displayValue());
        s_textFloat->moveGlobal(this, QPoint(width() + 2, 0));
        s_textFloat->show();
    }
    /*
    else if( _me->button() == Qt::LeftButton &&
                    gui->mainWindow()->isShiftPressed() == true )
    {
            new StringPairDrag( "float_value",
                                    QString::number( m->value() ),
                                                    QPixmap(), this );
    }
    */
    else
    {
        FloatModelView::mousePressEvent(_me);
    }
}

void Knob::mouseMoveEvent(QMouseEvent* _me)
{
    FloatModel* m = model();
    if(!m)
        return;

    if(m_pressLeft && _me->pos() != m_pressPos)  // m_origMousePos )
    {
        // m_mouseOffset = _me->pos() - m_origMousePos;
        // setPosition( m_mouseOffset, _me->modifiers() & Qt::ShiftModifier);
        setPosition(_me->pos() - m_pressPos,
                    _me->modifiers() & Qt::ShiftModifier);
        emit sliderMoved(m->value());
        // QCursor::setPos( mapToGlobal( m_origMousePos ) );
    }
    s_textFloat->setText(displayValue());
}

void Knob::mouseReleaseEvent(QMouseEvent* event)
{
    FloatModel* m = model();
    if(!m)
        return;

    if(event && event->button() == Qt::LeftButton)
    {
        m->restoreJournallingState();
    }

    m_pressLeft = false;

    emit sliderReleased();

    setCursor(Qt::PointingHandCursor);
    // setCursor(Qt::ArrowCursor);
    // QApplication::restoreOverrideCursor();

    s_textFloat->hide();
}

void Knob::focusOutEvent(QFocusEvent* _fe)
{
    // make sure we don't loose mouse release event
    mouseReleaseEvent(NULL);
    QWidget::focusOutEvent(_fe);
}

void Knob::mouseDoubleClickEvent(QMouseEvent*)
{
    enterValue();
}

/*
void Knob::paintEvent(QPaintEvent* _pe)
{
    // PAINT_THREAD_CHECK
    // DEBUG_THREAD_PRINT
    Widget::paintEvent(_pe);
}
*/

void Knob::wheelEvent(QWheelEvent* _we)
{
    FloatModel* m = model();
    if(!m)
        return;

    if(_we->modifiers() & Qt::ShiftModifier)
    {
        _we->accept();

        int ns = int(roundf(m->range() / m->step<float>() / 100.f));
        if(_we->modifiers() & Qt::ControlModifier)
            ns /= 10;
        ns = qBound(1, ns, 1000);

        if(_we->delta() > 0)
            m->incrValue(ns);
        else if(_we->delta() < 0)
            m->decrValue(ns);

        s_textFloat->setText(displayValue());
        s_textFloat->moveGlobal(this, QPoint(width() + 2, 0));
        s_textFloat->setVisibilityTimeOut(1000);

        emit sliderMoved(m->value());
    }
}

void Knob::enterValue()
{
    FloatModel* m = model();
    if(!m)
        return;

    bool  ok;
    float new_val;

    if(isVolumeKnob()
       && ConfigManager::inst()->value("app", "displaydbfs").toInt())
    {
        new_val = QInputDialog::getDouble(
                this, windowTitle(),
                tr("Please enter a new value between "
                   "-96.0 dBFS and 6.0 dBFS:"),
                ampToDbfs(m->getRoundedValue() / 100.0), -96.0, 6.0,
                m->getDigitCount(), &ok);
        if(new_val <= -96.0)
        {
            new_val = 0.0f;
        }
        else
        {
            new_val = dbfsToAmp(new_val) * 100.0;
        }
    }
    else
    {
        new_val = QInputDialog::getDouble(
                this, windowTitle(),
                tr("Please enter a new value between "
                   "%1 and %2:")
                        .arg(m->minValue())
                        .arg(m->maxValue()),
                m->getRoundedValue(), m->minValue(), m->maxValue(),
                m->getDigitCount(), &ok);
    }

    if(ok)
    {
        m->setValue(new_val);
    }
}

/*
void Knob::refresh()
{
    Widget::refresh();
}
*/

void Knob::update()
{
    // qInfo("Knob::update");
    invalidateCache();
    friendlyUpdate();
    // Widget::update();
}

void Knob::friendlyUpdate()
{
    /*
    FloatModel* m = model();
    if(!m)
    {
        // qInfo("Knob::drawKnob NO MODEL p=%p",this);
        if(!isCacheValid())
            update();
        return;
    }
    */

    if(updateAngle())
        invalidateCache();

    QColor newStatusColor = statusColor();
    if(newStatusColor != m_statusColor)
    {
        m_statusColor = newStatusColor;
        invalidateCache();
    }

    // qInfo("Knob::friendlyUpdate 1");
    if(!isCacheValid())
        Widget::update();  // PaintManager::updateLater(this);

    /*
        FloatModel* m = model();
        // qInfo("Knob::friendlyRefresh 1");
        if(!m->isAutomated() || !m->isControlled() ||  //????
           !m->frequentlyUpdated() || m->controllerConnection() == NULL
           || m->controllerConnection()->getController()->frequentlyUpdated()
                      == false
           ||
           // m->hasLinkedModels() ||
           Controller::runningFrames() % (256 * 4) == 0)
        {
            // qInfo("Knob::friendlyRefresh 2 p=%p",this);
            update();
        }
        else
        {
            qInfo("Knob::friendlyRefresh skipped");
            // update();
        }
    */
}

/*
void Knob::refresh()
{
    // qInfo("Knob::refresh p=%p",this);
    invalidateCache();
    updateNow();
}
*/

QString Knob::displayValue() const
{
    const FloatModel* m = model();
    if(!m)
        return "";

    if(isVolumeKnob()
       && ConfigManager::inst()->value("app", "displaydbfs").toInt())
    {
        return m_description.trimmed() + " "
               + QString::number(
                         ampToDbfs(m->getRoundedValue() / volumeRatio()), 'f',
                         2)
               + " dBFS";
    }
    return m_description.trimmed()
           + " "
           //+ QString( " %1" ).arg( m->getRoundedValue(), 0, 'f',
           // m->getDigitCount() )
           + m->displayValue(m->value()) + " " + m_unit;
}

void Knob::doConnections()
{
    FloatModel* m = model();
    if(m)
    {
        // qInfo("Knob::doConnections p=%p model()=%p",this,m);
        m->disconnect(this);
        connect(m, SIGNAL(dataChanged()), this, SLOT(friendlyUpdate()));
        connect(m, SIGNAL(propertiesChanged()), this, SLOT(update()));
        // QObject::connect( m, SIGNAL( controllerValueChanged() ),
        //                  this, SLOT( refresh() ) );
    }
}

void Knob::displayHelp()
{
    QWhatsThis::showText(mapToGlobal(rect().bottomRight()), whatsThis());
}
