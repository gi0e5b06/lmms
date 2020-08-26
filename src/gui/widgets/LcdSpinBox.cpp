/*
 * LcdSpinBox.cpp - class LcdSpinBox, an improved QLCDNumber
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2008 Paul Giblock <pgllama/at/gmail.com>
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

#include "LcdSpinBox.h"

#include "CaptionMenu.h"
#include "GuiApplication.h"
#include "MainWindow.h"

//#include "debug.h"

#include <QApplication>
//#include <QLabel>
#include <QMouseEvent>
//#include <QPainter>
//#include <QStyleOptionFrameV2>
#include <QInputDialog>

LcdSpinBox::LcdSpinBox(int            _numDigits,
                       QWidget*       _parent,
                       const QString& _displayName,
                       const QString& _objectName) :
      LcdSpinBox(_numDigits, "19default", _parent, _displayName, _objectName)
{
}

LcdSpinBox::LcdSpinBox(int            _numDigits,
                       const QString& _style,
                       QWidget*       _parent,
                       const QString& _displayName,
                       const QString& _objectName) :
      LcdWidget(_numDigits, _style, _parent, _displayName),
      IntModelView(this,_displayName),
      //new IntModel(0, 0, 0, nullptr, _displayName, _objectName, true),
      //this),
      m_pressLeft(false)
//, m_displayOffset( 0 )
{
    LcdWidget::setValue(0);
    setCursor(Qt::PointingHandCursor);
}

LcdSpinBox::~LcdSpinBox()
{
}

void LcdSpinBox::modelChanged()
{
    ModelView::modelChanged();
    IntModel* m = model();
    // if(m != nullptr)
    LcdWidget::setValue(m->rawValue());
}

void LcdSpinBox::update()
{
    IntModel* m = model();
    // if(m != nullptr)
    LcdWidget::setValue(m->rawValue());
    LcdWidget::update();
}

void LcdSpinBox::convert(const QPoint& _p, real_t& value_, real_t& dist_)
{
    // dist_ = (_p.x()*_p.x()+_p.y()*_p.y())/22500.; //-100.
    dist_ = 1. + qAbs(_p.x() / 50.);
    // if(dist_<0.) dist_=0.;
    // if(dist_>1.) dist_=1.;

    value_ = -_p.y() / 125.;
    if(value_ < -1.)
        value_ = -1.;
    if(value_ > 1.)
        value_ = 1.;
    value_ = 0.5 + value_ / 2.;

    // qInfo("x=%d y=%d d=%f v=%f",_p.x(),_p.y(),dist_,value_);
}

void LcdSpinBox::setPosition(const QPoint& _p, bool _shift)
{
    real_t value, dist;
    convert(_p, value, dist);

    IntModel* m = model();
    // if(m == nullptr) return;

    // const real_t oldValue = model()->rawValue();

    if(_shift)
    {
        // m_pressValue=model()->rawValue();
        dist /= 5.;
        // qInfo("shift pv=%f dist=%f",m_pressValue,dist);
    }

    const real_t step = m->step();

    /*
    if( model()->isScaleLogarithmic() ) // logarithmic code
    {
            real_t roundedValue = qRound(
    (dist*(value*model()->range()+model()->minValue())
                                         +(1.-dist)*m_pressValue) / step ) *
    step; model()->setValue( roundedValue );
    }

    else // linear code
    */
    {
        /*
        // absolute
        real_t roundedValue = model()->minValue()+
                qRound( ( dist*value*model()->range()+
                          (1.-dist)*(m_pressValue-model()->minValue()) ) /
        step ) * step;
        */
        // relative
        real_t roundedValue
                = qRound(((m_pressValue - m->minValue())
                          + dist * (value - 0.5)
                                    * qMin<real_t>(50. * step, m->range()))
                         / step)
                  * step;
        // model()->setValue( roundedValue );
        real_t v = m->minValue()
                   + qMax<real_t>(0., qMin<real_t>(roundedValue, m->range()));
        m->setValue(v);
        LcdWidget::setValue(v);
        // qInfo("       rv=%f dist=%f",roundedValue,dist);
    }
}

void LcdSpinBox::contextMenuEvent(QContextMenuEvent* event)
{
    IntModel* m = model();
    // if(m == nullptr) return;

    // for the case, the user clicked right while pressing left mouse-
    // button, the context-menu appears while mouse-cursor is still hidden
    // and it isn't shown again until user does something which causes
    // an QApplication::restoreOverride Cursor()-call...
    // mouseReleaseEvent( nullptr );

    CaptionMenu contextMenu(m->displayName());
    addDefaultActions(&contextMenu);
    contextMenu.exec(QCursor::pos());
}

void LcdSpinBox::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::LeftButton
       && !(_me->modifiers() & Qt::ControlModifier)
       && !(_me->modifiers() & Qt::ShiftModifier)
       && displayRect().contains(_me->pos()))  // y() < cellHeight() + 2  )
    {
        IntModel* m = model();
        // if(m != nullptr)
        {
            m->addJournalCheckPoint();
            m->saveJournallingState(false);
            m_pressValue = m->rawValue();
        }
        /*
        else
        {
            m_pressValue = 0.;
        }
        */

        m_pressLeft = true;
        m_pressPos  = _me->pos();

        // emit sliderPressed();

        setCursor(Qt::SplitVCursor);
        // QApplication::setOverride Cursor( Qt::BlankCursor );
        // s_textFloat->setText( displayValue() );
        // s_textFloat->moveGlobal( this, QPoint( width() + 2, 0 ) );
        // s_textFloat->show();
    }
    else
    {
        IntModelView::mousePressEvent(_me);
    }
}

void LcdSpinBox::mouseMoveEvent(QMouseEvent* _me)
{
    if(m_pressLeft && _me->y() != m_pressPos.y())
    {
        // if( gui->mainWindow()->isShiftPressed() )
        // dy = qBound( -4, dy/4, 4 );
        // if( dy > 1 || dy < -1 )
        //{
        // model()->setInitValue( model()->rawValue() -
        // dy / 2 * model()->step<int>() );
        setPosition(_me->pos() - m_pressPos,
                    _me->modifiers() & Qt::ShiftModifier);
        emit manualChange();
        // QCursor::setPos( m_pressPos );
        //}
    }
}

void LcdSpinBox::mouseReleaseEvent(QMouseEvent*)
{
    if(m_pressLeft)
    {
        IntModel* m = model();
        // if(m != nullptr)
        m->restoreJournallingState();

        // QCursor::setPos( m_pressPos );
        setCursor(Qt::PointingHandCursor);
        // QApplication::restoreOverride Cursor();

        m_pressLeft = false;
    }
}

void LcdSpinBox::wheelEvent(QWheelEvent* _we)
{
    if(_we->modifiers() & Qt::ShiftModifier)
    {
        _we->accept();
        IntModel* m = model();
        // if(m != nullptr)
        m->setInitValue(m->rawValue()
                        + ((_we->delta() > 0) ? 1 : -1) * m->step());
        emit manualChange();
    }
}

void LcdSpinBox::mouseDoubleClickEvent(QMouseEvent*)
{
    enterValue();
}

void LcdSpinBox::enterValue()
{
    IntModel* m = model();
    // if(m == nullptr) return;

    bool ok;
    int  new_val;

    new_val = QInputDialog::getInt(
            this, windowTitle(),
            tr("Please enter a new value between %1 and %2:")
                    .arg(m->minValue())
                    .arg(m->maxValue()),
            m->rawValue(), m->minValue(), m->maxValue(), 4, &ok);

    if(ok)
    {
        m->setValue(new_val);
        LcdWidget::setValue(new_val);
    }
}
