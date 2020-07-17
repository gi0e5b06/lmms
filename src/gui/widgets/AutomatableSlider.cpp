/*
 * AutomatableSlider.cpp - implementation of class AutomatableSlider
 *
 * Copyright (c) 2006-2007 Javier Serrano Polo
 * <jasp00/at/users.sourceforge.net> Copyright (c) 2007-2009 Tobias Doerffel
 * <tobydox/at/users.sourceforge.net>
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

#include "AutomatableSlider.h"

#include "CaptionMenu.h"

#include <QMouseEvent>
//#include "MainWindow.h"

AutomatableSlider::AutomatableSlider(QWidget*       _parent,
                                     const QString& _displayName,
                                     const QString& _objectName) :
      QSlider(_parent),
      // IntModelView( new IntModel( 0, 0, 0, NULL, _name, true ), this ),
      FloatModelView(
              new FloatModel(
                      0, 0, 0, 1, nullptr, _displayName, _objectName, true),
              this),
      m_showStatus(false)
{
    setWindowTitle(_displayName);
    setCursor(Qt::PointingHandCursor);

    connect(this, SIGNAL(valueChanged(int)), this, SLOT(changeValue(int)));
    connect(this, SIGNAL(sliderMoved(int)), this, SLOT(moveSlider(int)));
}

AutomatableSlider::~AutomatableSlider()
{
}

/*
void AutomatableSlider::convert(const QPoint& _p, float& value_, float& dist_)
{
        //dist_ = (_p.x()*_p.x()+_p.y()*_p.y())/22500.f; //-100.f
        dist_=1.+qAbs(_p.x()/125.f);
        //if(dist_<0.f) dist_=0.f;
        //if(dist_>1.f) dist_=1.f;

        value_=-_p.y()/250.f;
        if(value_<-1.f) value_=-1.f;
        if(value_> 1.f) value_= 1.f;
        value_=0.5f+value_/2.f;

        qWarning("x=%d y=%d d=%f v=%f",_p.x(),_p.y(),dist_,value_);
}
*/

/*
void AutomatableSlider::setPosition( const QPoint & _p, bool _shift )
{
        float value,dist;
        convert(_p,value,dist);

        const float step = model()->step<float>();

        if(_shift)
        {
                //m_pressValue=model()->rawValue();
                dist/=4.f;
                qWarning("shift pv=%f dist=%f",m_pressValue,dist);
        }


        // absolute
        //float roundedValue = model()->minValue()+
        //	qRound( ( dist*value*model()->range()+
        //		  (1.f-dist)*(m_pressValue-model()->minValue()) ) /
step ) * step;

        // relative
        float roundedValue=qRound( ((m_pressValue-model()->minValue())+
                                    dist*(value-0.5)*qMin(50.f*step,model()->range()))
                                   / step ) * step;
        //model()->setValue( roundedValue );
        model()->setValue(
model()->minValue()+qMax(0.f,qMin(roundedValue,model()->range()))); qWarning("
rv=%f dist=%f",roundedValue,dist);
}
*/

void AutomatableSlider::contextMenuEvent(QContextMenuEvent* _cme)
{
    CaptionMenu contextMenu(model()->displayName());
    addDefaultActions(&contextMenu);
    contextMenu.exec(QCursor::pos());
}

void AutomatableSlider::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::LeftButton
       && !(_me->modifiers() & Qt::ControlModifier))
    {
        m_showStatus = true;
        setCursor(Qt::SplitVCursor);
        QSlider::mousePressEvent(_me);
    }
    else
    {
        // IntModelView::mousePressEvent( _me );
        FloatModelView::mousePressEvent(_me);
    }
}

void AutomatableSlider::mouseReleaseEvent(QMouseEvent* _me)
{
    m_showStatus = false;
    setCursor(Qt::PointingHandCursor);
    QSlider::mouseReleaseEvent(_me);
}

void AutomatableSlider::mouseMoveEvent(QMouseEvent* _me)
{
    QSlider::mouseMoveEvent(_me);
}

void AutomatableSlider::wheelEvent(QWheelEvent* _we)
{
    if(_we->modifiers() & Qt::ShiftModifier)
    {
        bool old_status = m_showStatus;
        m_showStatus    = true;
        QSlider::wheelEvent(_we);
        m_showStatus = old_status;
    }
}

void AutomatableSlider::modelChanged()
{
    QSlider::setRange(model()->minValue(), model()->maxValue());
    updateSlider();
    connect(model(), SIGNAL(dataChanged()), this, SLOT(updateSlider()));
}

void AutomatableSlider::changeValue(int _value)
{
    model()->setValue(_value);
    emit logicValueChanged(model()->rawValue());
}

void AutomatableSlider::moveSlider(int _value)
{
    model()->setValue(_value);
    emit logicSliderMoved(model()->rawValue());
}

void AutomatableSlider::updateSlider()
{
    QSlider::setValue(model()->rawValue());
}
