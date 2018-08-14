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



LcdSpinBox::LcdSpinBox( int numDigits, QWidget* parent, const QString& name ) :
	LcdWidget( numDigits, parent, name ),
	IntModelView( new IntModel( 0, 0, 0, NULL, name, true ), this ),
	m_pressLeft( false )//,
	//m_displayOffset( 0 )
{
        setCursor(Qt::PointingHandCursor);
}




LcdSpinBox::LcdSpinBox( int numDigits, const QString& style, QWidget* parent, const QString& name ) :
	LcdWidget( numDigits, style, parent, name ),
	IntModelView( new IntModel( 0, 0, 0, NULL, name, true ), this ),
	m_pressLeft( false )//,
	//m_displayOffset( 0 )
{
        setCursor(Qt::PointingHandCursor);
}



LcdSpinBox::~LcdSpinBox()
{
}




void LcdSpinBox::update()
{
	if(model())
                LcdWidget::setValue( model()->value() );//+ m_displayOffset );
	QWidget::update();
}



void LcdSpinBox::convert(const QPoint& _p, float& value_, float& dist_)
{
	//dist_ = (_p.x()*_p.x()+_p.y()*_p.y())/22500.f; //-100.f
	dist_=1.+qAbs(_p.x()/50.f);
	//if(dist_<0.f) dist_=0.f;
	//if(dist_>1.f) dist_=1.f;

	value_=-_p.y()/125.f;
	if(value_<-1.f) value_=-1.f;
	if(value_> 1.f) value_= 1.f;
	value_=0.5f+value_/2.f;

	//qInfo("x=%d y=%d d=%f v=%f",_p.x(),_p.y(),dist_,value_);
}

void LcdSpinBox::setPosition( const QPoint & _p, bool _shift )
{
	float value,dist;
	convert(_p,value,dist);

        if(!model()) return;

	//const float oldValue = model()->value();

	if(_shift)
        {
		//m_pressValue=model()->value();
		dist/=5.f;
		//qInfo("shift pv=%f dist=%f",m_pressValue,dist);
	}

	const float step = model()->step<float>();

	/*
	if( model()->isScaleLogarithmic() ) // logarithmic code
	{
		float roundedValue = qRound( (dist*(value*model()->range()+model()->minValue())
					     +(1.f-dist)*m_pressValue) / step ) * step;
		model()->setValue( roundedValue );
	}

	else // linear code
	*/
	{
		/*
		// absolute
		float roundedValue = model()->minValue()+
			qRound( ( dist*value*model()->range()+
				  (1.f-dist)*(m_pressValue-model()->minValue()) ) / step ) * step;
		*/
		// relative
		float roundedValue=qRound( ((m_pressValue-model()->minValue())+
					    dist*(value-0.5)*qMin(50.f*step,model()->range()))
					   / step ) * step;
		//model()->setValue( roundedValue );
		model()->setValue( model()->minValue()+qMax(0.f,qMin(roundedValue,model()->range())));
		//qInfo("       rv=%f dist=%f",roundedValue,dist);
	}
}




void LcdSpinBox::contextMenuEvent( QContextMenuEvent* event )
{
        if(!model()) return;

	// for the case, the user clicked right while pressing left mouse-
	// button, the context-menu appears while mouse-cursor is still hidden
	// and it isn't shown again until user does something which causes
	// an QApplication::restoreOverrideCursor()-call...
	//mouseReleaseEvent( NULL );

	CaptionMenu contextMenu( model()->displayName() );
	addDefaultActions( &contextMenu );
	contextMenu.exec( QCursor::pos() );
}




void LcdSpinBox::mousePressEvent( QMouseEvent* _me )
{
	if( _me->button() == Qt::LeftButton &&
	    ! ( _me->modifiers() & Qt::ControlModifier ) &&
	    ! ( _me->modifiers() & Qt::ShiftModifier ) &&
	    displayRect().contains(_me->pos()) )  //y() < cellHeight() + 2  )
	{
		AutomatableModel *thisModel = model();
		if( thisModel )
		{
			thisModel->addJournalCheckPoint();
			thisModel->saveJournallingState( false );
		}

		if( model() ) m_pressValue=model()->value();
		else          m_pressValue=0.f;

		m_pressLeft = true;
		m_pressPos = _me->pos();

		//emit sliderPressed();

		setCursor(Qt::SplitVCursor);
		//QApplication::setOverrideCursor( Qt::BlankCursor );
		//s_textFloat->setText( displayValue() );
		//s_textFloat->moveGlobal( this, QPoint( width() + 2, 0 ) );
		//s_textFloat->show();
	}
	else
	{
		IntModelView::mousePressEvent( _me );
	}
}




void LcdSpinBox::mouseMoveEvent( QMouseEvent* _me )
{
	if( m_pressLeft && _me->y() != m_pressPos.y() )
	{
		//if( gui->mainWindow()->isShiftPressed() )
		//dy = qBound( -4, dy/4, 4 );
		//if( dy > 1 || dy < -1 )
		//{
		//model()->setInitValue( model()->value() -
		//dy / 2 * model()->step<int>() );
		setPosition( _me->pos()-m_pressPos,
			     _me->modifiers() & Qt::ShiftModifier );
		emit manualChange();
		//QCursor::setPos( m_pressPos );
		//}
	}
}




void LcdSpinBox::mouseReleaseEvent( QMouseEvent* )
{
	if( m_pressLeft )
	{
		if(model()) model()->restoreJournallingState();

		//QCursor::setPos( m_pressPos );
		setCursor(Qt::PointingHandCursor);
		//QApplication::restoreOverrideCursor();

		m_pressLeft = false;
	}
}




void LcdSpinBox::wheelEvent( QWheelEvent * _we )
{
	if( _we->modifiers() & Qt::ShiftModifier )
        {
		_we->accept();
                if(model())
                        model()->setInitValue
                                ( model()->value() +
                                  ( ( _we->delta() > 0 ) ? 1 : -1 ) *
                                  model()->step<int>() );
		emit manualChange();
	}
}

void LcdSpinBox::mouseDoubleClickEvent( QMouseEvent * )
{
	enterValue();
}

void LcdSpinBox::enterValue()
{
        if(!model()) return;

	bool ok;
	int new_val;

	new_val = QInputDialog::getInt(
			this, windowTitle(),
			tr( "Please enter a new value between %1 and %2:" ).
			arg( model()->minValue() ).
			arg( model()->maxValue() ),
			model()->value(),
			model()->minValue(),
			model()->maxValue(), 4, &ok );

	if( ok )
	{
		model()->setValue( new_val );
	}
}
