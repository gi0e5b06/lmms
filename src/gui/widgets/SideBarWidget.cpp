/*
 * SideBarWidget.cpp - implementation of base-widget for side-bar
 *
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "SideBarWidget.h"

//#include <QApplication>
#include <QFontMetrics>
#include <QPainter>

SideBarWidget::SideBarWidget( const QString & _title, const QPixmap & _icon,
							QWidget * _parent ) :
	QWidget( _parent ),
	m_title( _title ),
	m_icon( _icon )
{
	m_contents = new QWidget( this );
	m_layout = new QVBoxLayout( m_contents );
	m_layout->setSpacing( 5 );
	m_layout->setMargin( 0 );
}




SideBarWidget::~SideBarWidget()
{
}




void SideBarWidget::paintEvent( QPaintEvent * )
{
	QPainter p( this );

	p.fillRect( 0, 0, width(), height(), p.background());

	QFont f = p.font();
	const int tx = m_icon.width()+12;

	QFontMetrics metrics( f );
	const int ty = m_icon.height()+0;//metrics.ascent();
	p.drawText( tx, ty, m_title );

	p.drawPixmap( 3, 3, m_icon.transformed( QTransform().rotate( 270 ) ) );
}



void SideBarWidget::resizeEvent( QResizeEvent * )
{
	const int MARGIN = 3;
	const int HEAD   = 37;
	m_contents->setGeometry( MARGIN, HEAD + MARGIN, width() - MARGIN * 2,
						height() - MARGIN * 2 - HEAD );
}





