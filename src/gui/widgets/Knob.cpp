/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Knob.cpp - knob widget
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "Knob.h"

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
#include <QStyleOption>
#include <QWhatsThis>

TextFloat* Knob::s_textFloat = nullptr;

// new FloatModel(0.7,0.,1.,0.01, nullptr, _name, true ), this ),

Knob::Knob(RealModel* _model, QWidget* _parent, knobTypes _knobNum) :
      Widget(_parent), RealModelView(_model, this), m_pressLeft(false),
      m_text(""),
      /*m_interactive(true), */ m_knobPixmap(nullptr), m_angle(-1000.),
      // m_cache( nullptr ),
      m_lineWidth(0.), m_textColor(255, 255, 255), m_knobNum(_knobNum)
{
    initUi();
}

Knob::Knob(QWidget* _parent, const QString& _displayName) :
      Knob(knobBright_26, _parent, _displayName)
{
}

Knob::Knob(knobTypes      _knob_num,
           QWidget*       _parent,
           const QString& _displayName) :
      Widget(_parent),
      RealModelView(this, _displayName), m_pressLeft(false), m_text(""),
      /*m_interactive(true), */ m_knobPixmap(nullptr), m_angle(-1000.),
      // m_cache( nullptr ),
      m_lineWidth(0.), m_textColor(255, 255, 255), m_knobNum(_knob_num)
{
    initUi();
}

void Knob::initUi()
{
    if(s_textFloat == nullptr)
        s_textFloat = new TextFloat();

    RealModel* m = castModel<RealModel>();
    setObjectName(m->objectName() + "Knob");
    setWindowTitle(m->displayName());
    setCursor(Qt::PointingHandCursor);
    // setAttribute(Qt::WA_DeleteOnClose, true);
    // setAttribute(Qt::WA_OpaquePaintEvent);

    onKnobNumUpdated();
    setTotalAngle(270.);
    setInnerRadius(1.);
    setOuterRadius(10.);
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
                embed::getPixmap(knobFilename.toUtf8().constData()));

        setMinimumSize(m_knobPixmap->width(), m_knobPixmap->height());
        resize(m_knobPixmap->width(), m_knobPixmap->height());
        update();
    }
}

Knob::~Knob()
{
    if(m_knobPixmap != nullptr)
    {
        delete m_knobPixmap;
        m_knobPixmap = nullptr;
    }
    // if( m_cache ) delete m_cache;
}

void Knob::setLabel(const QString& txt)
{
    setText(txt);
}

QString Knob::text() const
{
    return m_text;
}

void Knob::setText(const QString& _t)
{
    if(m_text == _t)
        return;

    m_text = _t;
    onTextUpdated();
}

void Knob::onTextUpdated()
{
    if(m_knobPixmap == nullptr)
        return;

    QStyleOption opt;
    opt.init(this);

    const QString& t = text();
    int            w = m_knobPixmap->width();
    int            h = m_knobPixmap->height();
    if(!t.isEmpty())
    {
        const QFont        ft = pointSizeF(font(), 7.);
        const QFontMetrics mx(ft);
        w = qMax<int>(w, qMin<int>(2 * w, mx.width(t)));
        h += 7;  // 10;
    }

    QMargins mw = contentsMargins();
    w += mw.left() + mw.right();
    h += mw.top() + mw.bottom();

    setMinimumSize(w, h);
    resize(w, h);
    update();
}

/*
bool Knob::isInteractive() const
{
    return m_interactive;
}

void Knob::setInteractive(bool _b)
{
    if(m_interactive == _b)
        return;

    m_interactive = _b;
    update();
}
*/

void Knob::setTotalAngle(real_t angle)
{
    if(angle < 10.)
        angle = 10.;

    if(angle != m_totalAngle)
    {
        m_totalAngle = angle;
        update();
    }
}

real_t Knob::innerRadius() const
{
    return m_innerRadius;
}

void Knob::setInnerRadius(real_t r)
{
    m_innerRadius = r;
}

real_t Knob::outerRadius() const
{
    return m_outerRadius;
}

void Knob::setOuterRadius(real_t r)
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

real_t Knob::centerPointX() const
{
    return m_centerPoint.x();
}

void Knob::setCenterPointX(real_t c)
{
    m_centerPoint.setX(c);
}

real_t Knob::centerPointY() const
{
    return m_centerPoint.y();
}

void Knob::setCenterPointY(real_t c)
{
    m_centerPoint.setY(c);
}

real_t Knob::lineWidth() const
{
    return m_lineWidth;
}

void Knob::setLineWidth(real_t w)
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
                           real_t         _outerRadius,
                           real_t         _innerRadius) const
{
    const real_t rarc = m_angle * F_PI / 180.;
    const real_t ca   = cos(rarc);
    const real_t sa   = -sin(rarc);

    return QLineF(_mid.x() - sa * _innerRadius, _mid.y() - ca * _innerRadius,
                  _mid.x() - sa * _outerRadius, _mid.y() - ca * _outerRadius);
}

bool Knob::updateAngle()
{
    RealModel* m = model();
    // if(m == nullptr) return true;

    real_t angle = 0.;
    // if(m)  //&& m->maxValue() != m->minValue() )
    {
        // angle = angleFromValue( m->inverseScaledValue( m->rawValue() ),
        // m->minValue(), m->maxValue(), m_totalAngle ); real_t
        // v=m->minValue()+qRound( (m->rawValue()-m->minValue()) /
        //                              m->step<real_t>() ) *
        //                              m->step<real_t>();
        // v=m->normalizedValue(v);
        real_t v = m->rawValue();
        if(m->isScaleLogarithmic())
            v = ::linearToLogScale(m->minValue(), m->maxValue(), v);
        v     = (v - m->minValue()) / m->range();
        angle = m_totalAngle * (v - 0.5);
    }
    if(abs(angle - m_angle) > 3.)
    {
        m_angle = angle;
        return true;
    }
    return false;
}

real_t Knob::angleFromValue(real_t value,
                            real_t minValue,
                            real_t maxValue,
                            real_t totalAngle) const
{
    // return fmod((value - 0.5*(minValue + maxValue)) / (maxValue -
    // minValue)*m_totalAngle,360);
    return m_totalAngle * (fmod(value, 1.) - 0.5);
}

QColor Knob::statusColor()
{
    RealModel* m = model();
    QColor     r(255, 255, 255, 192);

    if(m->isDefaultConstructed())
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
    _p.setRenderHints(QPainter::Antialiasing, true);
    drawKnob(_p);
    drawText(_p);
}

void Knob::drawKnob(QPainter& _p)
{
    RealModel* m = model();
    // if(m == nullptr) return;

    QPointF mid;
    QColor  pc = pointColor();
    // if(isVolumeKnob())
    //    pc = Qt::red;
    int lw = lineWidth();

    if(m_knobNum == knobStyled)
    {
        mid = centerPoint();

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
    else  // if(m)
    {
        // Old-skool knobs
        const real_t radius = m_knobPixmap->width() / 2. - 3.;
        mid = QPointF(width() / 2., m_knobPixmap->height() / 2.);

        _p.drawPixmap(QPointF((width() - m_knobPixmap->width()) / 2.,
                              0.),  //(height()-m_knobPixmap->height())/2.),
                      *m_knobPixmap);

        // _p.setRenderHint( QPainter::Antialiasing );

        const real_t centerAngle
                = angleFromValue(m->inverseScaledValue(m->centerValue()),
                                 m->minValue(), m->maxValue(), m_totalAngle);

        const real_t arcLineWidth = 3.;
        const real_t arcRectSize
                = m_knobPixmap->width() - arcLineWidth;  // - 2.;  // 0

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
                const real_t rb = qMax<real_t>((radius - 8) / 3.0, 0.0);
                const real_t re = qMax<real_t>((radius - 2), 0.0);
                line            = calculateLine(mid, re, rb);
                // line.translate(1, 1);
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
        real_t re = 11.;  // radius;
        _p.drawEllipse(QRectF(mid.x() - re / 2., mid.y() - re / 2., re,
                              re));  //+1.,re+1.);

        QPen pen1(QColor(0, 0, 0, 96), 2, Qt::SolidLine, Qt::RoundCap);
        _p.setPen(pen1);
        _p.drawArc(QRectF(mid.x() - arcRectSize / 2., 1., arcRectSize,
                          arcRectSize),
                   16. * 315, 16. * m_totalAngle);

        QPen pen2(QColor(0, 0, 0, 96), 4, Qt::SolidLine, Qt::RoundCap);
        _p.setPen(pen2);
        _p.drawLine(line);
        _p.drawArc(QRectF(mid.x() - arcRectSize / 2., 1., arcRectSize,
                          arcRectSize),
                   16. * (90. - centerAngle), -16. * (m_angle - centerAngle));
        _p.setBrush(QColor(0, 0, 0, 96));
        // _p.drawEllipse(mid.x()-2.,mid.y()-2.,5.,5.);
        _p.drawEllipse(QRectF(mid.x() - 1., mid.y() - 1., 3., 3.));

        QPen pen3(pc, 2, Qt::SolidLine, Qt::RoundCap);
        _p.setPen(pen3);
        _p.drawLine(line);
        _p.drawArc(QRectF(mid.x() - arcRectSize / 2, 1, arcRectSize,
                          arcRectSize),
                   16. * (90. - centerAngle), -16. * (m_angle - centerAngle));

        if(isInteractive())
        {
            _p.setBrush(pc);
            _p.drawEllipse(QRectF(mid.x() - 1., mid.y() - 1., 3., 3.));
        }
        else
        {
            _p.setBrush(m_statusColor);
            _p.setPen(m_statusColor);
            re = 9.;
            _p.drawEllipse(
                    QRectF(mid.x() - re / 2., mid.y() - re / 2., re, re));
            _p.setBrush(Qt::black);
            _p.setPen(Qt::black);
            re = 7.;
            _p.drawEllipse(
                    QRectF(mid.x() - re / 2., mid.y() - re / 2., re, re));
        }
    }
}

void Knob::drawText(QPainter& _p)
{
    if(!m_text.isEmpty())
    {
        _p.setFont(pointSizeF(font(), 7.));
        _p.setPen(textColor());
        QFontMetrics metrix = _p.fontMetrics();
        QString text = metrix.elidedText(m_text, Qt::ElideRight, width());
        int     x    = width() / 2 - metrix.width(text) / 2;
        int     y    = height() - 1;
        // if(m_knobPixmap)
        //    y = qMin(y, m_knobPixmap->height() + 7);
        _p.drawText(x, y, text);
        _p.drawText(x, y, text);  // twice for aa
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

/*
void Knob::setModel(Model* _m, bool isOldModelValid)
{
    // qInfo("Knob::setModel p=%p",this);
    RealModelView::setModel(_m, isOldModelValid);
    // update();
}
*/

void Knob::modelChanged()
{
    RealModel* m = model();
    setWindowTitle(m->displayName());
    // qInfo("Knob::modelChanged p=%p", this);
    update();  // refresh();
}

void Knob::convert(const QPoint& _p, real_t& value_, real_t& dist_)
{
    // arcane mathemagicks for calculating knob movement
    // value = ( ( _p.y() + _p.y() * qMin( qAbs( _p.y() / 2.5 ), 6. ) ) )
    // / 12.;

    dist_ = (_p.x() * _p.x() + _p.y() * _p.y() - 80.) / 6400.;
    if(dist_ < 0.)
        dist_ = 0.;
    if(dist_ > 1.)
        dist_ = 1.;

    real_t angle = fmod(atan2(_p.y() / dist_, _p.x() / dist_) + 3. * R_PI_2,
                        R_2PI);
    angle        = qMax(0.80, qMin(5.49, angle));
    value_       = (angle - 0.80) / (5.49 - 0.80);

    // qInfo("x=%d y=%d d=%f a=%f v=%f",_p.x(),_p.y(),dist_,angle,value_);
    if(value_ < 0.)
        value_ = 0.;
    if(value_ > 1.)
        value_ = 1.;

    // if shift pressed we want slower movement
    /*
    if( gui->mainWindow()->isShiftPressed() )
    {
            value /= 4.;
            value = qBound( -4., value, 4. );
    }
    */
    // return value;// * pageSize();
}

void Knob::setPosition(const QPoint& _p, bool _shift)
{
    RealModel* m = model();
    // if(m == nullptr) return;

    real_t value, dist;
    convert(_p, value, dist);

    // const real_t step = model()->step<real_t>();
    // const real_t oldValue = model()->rawValue();

    if(_shift)
    {
        m_pressValue = 0.3 * value + 0.7 * m->normalizedValue(m->rawValue());
        dist /= 4.;
        // qInfo("shift pv=%f dist=%f",m_pressValue,dist);
    }

    // if( m->isScaleLogarithmic() ) // logarithmic code
    //{
    /*
    const real_t pos = m->minValue() < 0
            ? oldValue / qMax( qAbs( m->maxValue() ), qAbs( m->minValue() ) )
            : ( oldValue - m->minValue() ) / m->range();
    const real_t ratio = 0.1 + qAbs( pos ) * 15.;
    real_t newValue = value * ratio;
    if( qAbs( newValue ) >= step )
    {
            real_t roundedValue = qRound( ( oldValue - value ) / step ) *
    step; m->setValue( roundedValue ); m_leftOver = 0.;
    }
    else
    {
            m_leftOver = value;
    }
    */
    /*                real_t v=m->rawValue();
            if(m->isScaleLogarithmic())
       v=::linearToLogScale(m->minValue(),m->maxValue(),v);
            v=(v-m->minValue())/m->range();
            angle=m_totalAngle*(v-0.5);
    */
    real_t v = dist * value + (1. - dist) * m_pressValue;
    if(m->isScaleLogarithmic())
        v = ::logToLinearScale(0., 1., v);
    v = m->minValue() + m->range() * v;
    m->setValue(v);
    // m_leftOver = 0.;
    //	}

    // else // linear code
    //	{
    /*
    if( qAbs( value ) >= step )
    {
            real_t roundedValue = qRound( ( oldValue - value ) / step ) *
    step; m->setValue( roundedValue ); m_leftOver = 0.;
    }
    else
    {
            m_leftOver = value;
    }
    */
    // real_t roundedValue=qRound( (dist*value*m->range()+
    //(1.-dist)*(m_pressValue-m->minValue()))
    //        / step ) * step;
    // m->setValue( m->minValue()+qMax(0.,qMin(roundedValue,m->range())));
    // m_leftOver = 0.;
    //}
}

void Knob::contextMenuEvent(QContextMenuEvent*)
{
    RealModel* m = model();
    // if(m == nullptr) return;

    // for the case, the user clicked right while pressing left mouse-
    // button, the context-menu appears while mouse-cursor is still hidden
    // and it isn't shown again until user does something which causes
    // an QApplication::restoreOverride Cursor()-call...
    // mouseReleaseEvent( nullptr );

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
    RealModel* m = model();
    // if(m == nullptr) return;

    m->setScaleLogarithmic(!m->isScaleLogarithmic());
    // update();
}

void Knob::dragEnterEvent(QDragEnterEvent* _dee)
{
    // RealModel* m = model();
    // if(m == nullptr) return;

    StringPairDrag::processDragEnterEvent(_dee,
                                          "float_value,automatable_model");
}

void Knob::dropEvent(QDropEvent* _de)
{
    RealModel* m = model();
    // if(m == nullptr) return;

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
        if(m->controllerConnection() == nullptr)
        {
            AutomatableModel* mod = dynamic_cast<AutomatableModel*>(
                    Engine::projectJournal()->journallingObject(val.toInt()));
            if(mod != nullptr)
            {
                // not a midi or dummy
                // if(! mod->controllerConnection() )
                if(mod->controllerConnection())
                    qInfo("src Knob has a controller Connection");
                else
                    qInfo("src Knob has NO controller Connection");

                {
                    AutomatableModel::linkModels(m, mod);
                    mod->setValue(m->rawValue());
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
    RealModel* m = model();
    // if(m == nullptr) return;
    if(!isInteractive())
        return;

    if(_me->button() == Qt::LeftButton
       && !(_me->modifiers() & Qt::ControlModifier)
       && !(_me->modifiers() & Qt::ShiftModifier))
    {
        m->addJournalCheckPoint();
        m->saveJournallingState(false);
        m_pressValue = m->normalizedValue(m->rawValue());

        m_pressPos  = _me->pos();
        m_pressLeft = true;
        // const QPoint & p = _me->pos();
        // m_origMousePos = p;
        // m_mouseOffset = QPoint(0, 0);
        // m_leftOver = 0.;

        emit sliderPressed();

        setCursor(Qt::SizeAllCursor);
        // QApplication::setOverride Cursor( Qt::BlankCursor );
        s_textFloat->setText(displayValue());
        s_textFloat->moveGlobal(this, QPoint(width() + 2, 0));
        s_textFloat->show();
    }
    /*
    else if( _me->button() == Qt::LeftButton &&
                    gui->mainWindow()->isShiftPressed() == true )
    {
            new StringPairDrag( "float_value",
                                    QString::number( m->rawValue() ),
                                                    QPixmap(), this );
    }
    */
    else
    {
        RealModelView::mousePressEvent(_me);
        m_pressLeft = false;
    }
}

void Knob::mouseMoveEvent(QMouseEvent* _me)
{
    RealModel* m = model();
    // if(m == nullptr) return;
    if(!isInteractive())
        return;

    if(m_pressLeft && _me->pos() != m_pressPos)  // m_origMousePos )
    {
        // m_mouseOffset = _me->pos() - m_origMousePos;
        // setPosition( m_mouseOffset, _me->modifiers() & Qt::ShiftModifier);
        setPosition(_me->pos() - m_pressPos,
                    _me->modifiers() & Qt::ShiftModifier);
        emit sliderMoved(m->rawValue());
        // QCursor::setPos( mapToGlobal( m_origMousePos ) );
    }
    s_textFloat->setText(displayValue());
}

void Knob::mouseReleaseEvent(QMouseEvent* event)
{
    RealModel* m = model();
    // if(m == nullptr) return;
    if(!isInteractive())
        return;

    if(event != nullptr && event->button() == Qt::LeftButton && m_pressLeft)
        m->restoreJournallingState();

    m_pressLeft = false;

    emit sliderReleased();

    setCursor(Qt::PointingHandCursor);
    // setCursor(Qt::ArrowCursor);
    // QApplication::restoreOverride Cursor();

    s_textFloat->hide();
}

void Knob::focusOutEvent(QFocusEvent* _fe)
{
    // make sure we don't loose mouse release event
    mouseReleaseEvent(nullptr);
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
    RealModel* m = model();
    // if(m == nullptr) return;
    if(!isInteractive())
        return;

    if(_we->modifiers() & Qt::ShiftModifier)
    {
        _we->accept();

        int ns = int(round(m->range() / m->step() / 100.));
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

        emit sliderMoved(m->rawValue());
    }
}

void Knob::enterValue()
{
    RealModel* m = model();
    // if(m == nullptr) return;
    if(!isInteractive())
        return;

    bool   ok;
    double newVal;

    /*
    if(isVolumeKnob()
       && ConfigManager::inst()->value("app", "displaydbfs").toInt())
    {
        newVal = QInputDialog::getDouble(
                this, windowTitle(),
                tr("Please enter a new value between "
                   "-96.0 dBFS and 6.0 dBFS:"),
                ampToDbfs(m->getRoundedValue() / 100.), -96., 6.,
                m->getDigitCount(), &ok);
        if(newVal <= -96.)
        {
            newVal = 0.;
        }
        else
        {
            newVal = dbfsToAmp(newVal) * 100.;
        }
    }
    else
    */
    {
        newVal = QInputDialog::getDouble(
                this, windowTitle(),
                tr("Please enter a new value between "
                   "%1 and %2:")
                        .arg(m->minValue())
                        .arg(m->maxValue()),
                m->getRoundedValue(), m->minValue(), m->maxValue(),
                m->getDigitCount(), &ok);
    }

    if(ok)
        m->setValue(newVal);
}

void Knob::editRandomization()
{
    RealModel* m = model();
    // if(m == nullptr) return;
    if(!isInteractive())
        return;

    bool   ok;
    double newVal = QInputDialog::getDouble(
            this, windowTitle(),
            QObject::tr("Please enter a randomization ratio between "
                        "0 and 100:"),
            100. * m->randomRatio(), 0., 100., 2, &ok);

    if(ok)
        m->setRandomRatio(newVal / 100.);
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
    RealModel* m = model();
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
        RealModel* m = model();
        // qInfo("Knob::friendlyRefresh 1");
        if(!m->isAutomated() || !m->isControlled() ||  //????
           !m->frequentlyUpdated() || m->controllerConnection() == nullptr
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
    const RealModel* m = model();
    // if(m == nullptr)       return "";

    /*
    if(isVolumeKnob()
       && ConfigManager::inst()->value("app", "displaydbfs").toInt())
    {
        return m_description.trimmed() + " "
               + QString::number(
                       ampToDbfs(m->getRoundedValue() / volumeRatio()), 'f',
                       2)
               + " dBFS";
    }
    */

    //+ QString( " %1" ).arg( m->getRoundedValue(), 0, 'f',
    // m->getDigitCount() )
    return (m_description + ": " + m->displayValue(m->rawValue()) + m_unit)
            .trimmed();
}

void Knob::doConnections()
{
    RealModelView::doConnections();
    RealModel* m = model();
    // if(m)
    {
        // qInfo("Knob::doConnections p=%p model()=%p",this,m);
        // m->disconnect(this);
        disconnect(m, SIGNAL(dataChanged()), this, SLOT(update()));
        connect(m, SIGNAL(dataChanged()), this, SLOT(friendlyUpdate()));
        // connect(m, SIGNAL(propertiesChanged()), this, SLOT(update()));
        // QObject::connect( m, SIGNAL( controllerValueChanged() ),
        //                  this, SLOT( refresh() ) );
    }
}

void Knob::undoConnections()
{
    RealModel* m = model();
    connect(m, SIGNAL(dataChanged()), this, SLOT(update()));
    disconnect(m, SIGNAL(dataChanged()), this, SLOT(friendlyUpdate()));
    RealModelView::undoConnections();
}

void Knob::displayHelp()
{
    QWhatsThis::showText(mapToGlobal(rect().bottomRight()), whatsThis());
}

QLine Knob::cableFrom() const
{
    if(m_knobPixmap != nullptr)
    {
        int    w = m_knobPixmap->width();
        int    h = m_knobPixmap->height();
        QPoint p(w / 2, h / 2);
        return QLine(p, p + QPoint(0, 50));
    }

    return QLine();
}

QLine Knob::cableTo() const
{
    if(m_knobPixmap != nullptr)
    {
        int    w = m_knobPixmap->width() + 1;
        int    h = m_knobPixmap->height() + 1;
        QPoint p(w / 2, h / 2);
        return QLine(p, p + QPoint(0, 60));
    }

    return QLine();
}

QColor Knob::cableColor() const
{
    return m_pointColor;
}

BalanceKnob::BalanceKnob(QWidget* _parent) :
      Knob(knobBright_26, _parent, "[balance knob]")
{
    setPointColor(Qt::magenta);
    setText(tr("BAL"));
    setDescription(tr("Balance"));
    setUnit("%");
}

CutoffFrequencyKnob::CutoffFrequencyKnob(QWidget* _parent) :
      Knob(knobBright_26, _parent, "[cutoff frequency knob]")
{
    setPointColor(Qt::green);
    setText(tr("CUT"));
    setDescription(tr("Cutoff frequency"));
    setUnit("Hz");
}

FrequencyKnob::FrequencyKnob(QWidget* _parent) :
      Knob(knobBright_26, _parent, "[frequency knob]")
{
    setPointColor(Qt::green);
    setText(tr("FREQ"));
    setDescription(tr("Frequency"));
    setUnit("Hz");
}

MixKnob::MixKnob(QWidget* _parent) :
      Knob(knobBright_26, _parent, "[mix knob]")
{
    // setPointColor(Qt::green);
    setText(tr("MIX"));
    setDescription(tr("Mix"));
    // setUnit("Hz");
}

ResonanceKnob::ResonanceKnob(QWidget* _parent) :
      Knob(knobBright_26, _parent, "[resonance knob]")
{
    // setPointColor(Qt::green);
    setText(tr("RESO"));
    setDescription(tr("Resonance"));
    // setUnit("Hz");
}

VolumeKnob::VolumeKnob(QWidget* _parent) : VolumeKnob(knobBright_26, _parent)
{
}

VolumeKnob::VolumeKnob(knobTypes _knobNum, QWidget* _parent) :
      Knob(_knobNum, _parent, "[volume knob]"),
      // m_volumeKnob(false, nullptr, "[volume knob]"),
      m_volumeRatio(100., 0., 1000000., 0., nullptr, "[volume ratio]")
{
    setPointColor(Qt::red);
    setText(tr("VOL"));
    setDescription(tr("Volume"));
    setUnit("%");
}

void VolumeKnob::enterValue()
{
    if(ConfigManager::inst()->value("app", "displaydbfs").toInt())
    {
        RealModel* m = model();
        // if(m == nullptr) return;
        if(!isInteractive())
            return;

        bool   ok;
        double newVal;

        newVal = QInputDialog::getDouble(
                this, windowTitle(),
                tr("Please enter a new value between "
                   "-96.0 dBFS and 6.0 dBFS:"),
                ampToDbfs(m->getRoundedValue() / 100.), -96., 6.,
                m->getDigitCount(), &ok);
        if(newVal <= -96.)
            newVal = 0.;
        else
            newVal = dbfsToAmp(newVal) * 100.;

        if(ok)
            m->setValue(newVal);

        return;
    }

    return Knob::enterValue();
}

QString VolumeKnob::displayValue() const
{
    if(ConfigManager::inst()->value("app", "displaydbfs").toInt())
    {
        const RealModel* m = model();
        // if(m == nullptr) return "";

        return m_description + " "
               + QString::number(
                       ampToDbfs(m->getRoundedValue() / volumeRatio()), 'f',
                       2)
               + " dBFS";
    }

    return Knob::displayValue();
}
