/*
 * LedCheckbox.cpp - class LedCheckBox, an improved QCheckBox
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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


#include <QFontMetrics>
#include <QInputDialog>
#include <QPainter>

#include "LedCheckbox.h"
#include "embed.h"
#include "gui_templates.h"


static const QString names[LedCheckBox::NumColors] =
{
	"led_yellow", "led_green", "led_red", "led_blue"
} ;




//! @todo: in C++11, we can use delegating ctors
#define DEFAULT_LEDCHECKBOX_INITIALIZER_LIST \
	AutomatableButton( _parent, _name )


LedCheckBox::LedCheckBox( const QString & _text, QWidget * _parent,
				const QString & _name, LedColors _color ) :
	DEFAULT_LEDCHECKBOX_INITIALIZER_LIST,
	m_text( _text ),
        m_textAnchor( Qt::AnchorLeft )
{
	initUi( _color );
}


LedCheckBox::LedCheckBox( QWidget * _parent,
				const QString & _name, LedColors _color ) :
	DEFAULT_LEDCHECKBOX_INITIALIZER_LIST
{
	initUi( _color );
}

#undef DEFAULT_LEDCHECKBOX_INITIALIZER_LIST



LedCheckBox::~LedCheckBox()
{
	delete m_ledOnPixmap;
	delete m_ledOffPixmap;
}




QString LedCheckBox::text() const
{
        return m_text;
}


void LedCheckBox::setText(const QString& _s)
{
	m_text=_s;
	onTextUpdated();
}


Qt::AnchorPoint LedCheckBox::textAnchorPoint() const
{
        return m_textAnchor;
}


void LedCheckBox::setTextAnchorPoint(Qt::AnchorPoint _a)
{
        m_textAnchor=_a;
	onTextUpdated();
}


void LedCheckBox::paintEvent( QPaintEvent * )
{
	QPainter p( this );

        QFont ft=pointSize<7>(font());
	p.setFont(ft);

        int xl=0;
        int yl=-3;

        QString t=text();
        if(!t.isEmpty())
        {
                const QFontMetrics mx=p.fontMetrics();
                // pointSizeF( font(), 7) ); //6.5
                int ww=width();
                //int hw=height();
                int wl=m_ledOffPixmap->width();
                int hl=m_ledOffPixmap->height();
                int xt=1;
                int yt=mx.height()-1;

                Qt::AnchorPoint a=textAnchorPoint();

                if(a==Qt::AnchorLeft)
                {
                        xt=wl+3;
                }
                else
                if(a==Qt::AnchorRight)
                {
                        xl=ww-wl;
                }
                else
                if(a==Qt::AnchorBottom)
                {
                        xl=(ww-wl)/2;
                        yt+=yl+hl;
                        yt-=7; //tmp
                }
                else
                if(a==Qt::AnchorTop)
                {
                        xl=(ww-wl)/2;
                        yl+=yt;
                }

                //p.setPen( QColor( 64, 64, 64 ) );
                //p.drawText( m_ledOffPixmap->width() + 4, 11, text() );
                p.setPen(Qt::white);// QColor( 255, 255, 255 ) );
                p.drawText(xt,yt,text());
        }

        p.drawPixmap(xl,yl,
                     model()->value()
                     ? *m_ledOnPixmap
                     : *m_ledOffPixmap);

        //p.setPen(Qt::red);
        //p.drawRect(0,0,width()-1,height()-1);
}




void LedCheckBox::initUi( LedColors _color )
{
	setCheckable( true );

	if( _color >= NumColors || _color < Yellow )
	{
		_color = Yellow;
	}

	m_ledOnPixmap =new QPixmap(embed::getIconPixmap(names[_color].toUtf8().constData()));
	m_ledOffPixmap=new QPixmap(embed::getIconPixmap("led_off"));

	setFont( pointSize<7>( font() ) );
	setText( m_text );
}




void LedCheckBox::onTextUpdated()
{
        const QString& t=text();
        int w=m_ledOffPixmap->width();
        int h=m_ledOffPixmap->height();
        if(!t.isEmpty())
        {
                Qt::AnchorPoint a=textAnchorPoint();
                QFont ft=pointSize<7>(font());
                QFontMetrics mx(ft);
                if((a==Qt::AnchorLeft)||
                   (a==Qt::AnchorRight))
                {
                        w+=2+mx.width(t);//4
                        h=qMax<int>(h,mx.height()+2);
                }
                if((a==Qt::AnchorTop)||
                   (a==Qt::AnchorBottom))
                {
                        w=qMax<int>(w,qMin<int>(2*w,mx.width(t))+2);
                        h+=mx.height()+2;
                }
        }
        setFixedSize(w,h);
}


void LedCheckBox::enterValue()
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



