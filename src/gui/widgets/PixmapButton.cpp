/*
 * PixmapButton.cpp - implementation of pixmap-button (often used as "themed"
 *                     checkboxes/radiobuttons etc)
 *
 * Copyright (c) 2004-2013 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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


#include <QInputDialog>
#include <QMouseEvent>
#include <QPainter>

#include "PixmapButton.h"
#include "MainWindow.h"
#include "embed.h"
#include "Backtrace.h"


PixmapButton::PixmapButton( QWidget * _parent, const QString & _name ) :
	AutomatableButton( _parent, _name ),
	m_activePixmap(),
	m_inactivePixmap(),
	m_pressed( false )
{
	setActiveGraphic  ( embed::getIconPixmap("led_yellow"));
	setInactiveGraphic( embed::getIconPixmap("led_off"   ),false );
}




PixmapButton::~PixmapButton()
{
}




void PixmapButton::paintEvent( QPaintEvent * )
{
	QPainter p( this );

	if( ( model() != NULL && model()->value() ) || m_pressed )
	{
		if( !m_activePixmap.isNull() )
		{
			//p.drawPixmap( 0, 0, m_activePixmap.scaled
			//	      (size().boundedTo(m_activePixmap.size())));
                        p.drawPixmap(0,0,m_activePixmap.scaled(size()));
		}
	}
	else if( !m_inactivePixmap.isNull() )
	{
		//p.drawPixmap( 0, 0, m_inactivePixmap.scaled
		//	      (size().boundedTo(m_activePixmap.size())));
                p.drawPixmap(0,0,m_inactivePixmap.scaled(size()));
	}

        //p.setPen(Qt::red);
        //p.drawRect(0,0,width()-1,height()-1);
}


void PixmapButton::resizeEvent(QResizeEvent * _re)
{
	//qInfo("Knob::resizeEvent()");
        //BACKTRACE
        QSize sh=sizeHint();
        //qInfo("PixmapButton::resizeEvent %dx%d -> %dx%d",width(),height(),sh.width(),sh.height());
        if(sh!=size())
        {
                setFixedSize(sh);
                resize(sh);
        }
}


void PixmapButton::mousePressEvent( QMouseEvent * _me )
{
	// Show pressing graphics if this isn't checkable
	if( !isCheckable() )
	{
		m_pressed = true;
		update();
	}

	AutomatableButton::mousePressEvent( _me );
}




void PixmapButton::mouseReleaseEvent( QMouseEvent * _me )
{
	AutomatableButton::mouseReleaseEvent( _me );

	if( !isCheckable() )
	{
		m_pressed = false;
		update();
	}
}




void PixmapButton::mouseDoubleClickEvent( QMouseEvent * _me )
{
	emit doubleClicked();
	_me->accept();
}




void PixmapButton::setActiveGraphic( const QPixmap & _pm, bool _update )
{
	m_activePixmap = _pm;
	setFixedSize(sizeHint());//resize( sizeHint() );//m_activePixmap.width(), m_activePixmap.height() );
	if( _update ) update();
}




void PixmapButton::setInactiveGraphic( const QPixmap & _pm, bool _update )
{
	m_inactivePixmap = _pm;
	setFixedSize(sizeHint());//resize( sizeHint() );
	if( _update ) update();
}

QSize PixmapButton::sizeHint() const
{
        /*
	if( ( model() != NULL && model()->value() ) || m_pressed )
	{
		//return minimumSize().expandedTo(m_activePixmap.size()).boundedTo(maximumSize());
                return m_activePixmap.size();
	}
	else
	{
		//return minimumSize().expandedTo(m_inactivePixmap.size()).boundedTo(maximumSize());
                return m_inactivePixmap.size();
	}
        */

        QSize r=QSize(0,0);
        if(!m_activePixmap  .isNull()) r=r.expandedTo(m_activePixmap  .size());
        if(!m_inactivePixmap.isNull()) r=r.expandedTo(m_inactivePixmap.size());
        return r;
}



void PixmapButton::enterValue()
{
        BoolModel* m=model();
        if(!m) return;

	bool ok;
	bool new_val;

        new_val = QInputDialog::getInt(this, windowTitle(),
                                       tr( "Please enter a new value between "
                                           "%1 and %2:" ).
                                       arg( m->minValue() ).
                                       arg( m->maxValue() ),
                                       m->value(),//m->getRoundedValue(),
                                       m->minValue(),
                                       m->maxValue(),
                                       0,//m->getDigitCount(),
                                       &ok );

	if( ok )
	{
		m->setValue( new_val );
	}
}
