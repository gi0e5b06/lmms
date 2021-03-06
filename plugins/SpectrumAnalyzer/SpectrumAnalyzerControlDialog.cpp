/*
 * SpectrumAnalyzerControlDialog.cpp - view for spectrum analyzer
 *
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include <cmath>

#include <QLayout>
#include <QPainter>

#include "SpectrumAnalyzer.h"
#include "MainWindow.h"
#include "GuiApplication.h"
#include "LedCheckBox.h"
#include "embed.h"


static inline void darken( QImage& img, int x, int y, int w, int h )
{
	int imgWidth = img.width();
	QRgb * base = ( (QRgb *) img.bits() ) + y*imgWidth + x;
	for( int y = 0; y < h; ++y )
	{
		QRgb * d = base + y*imgWidth;
		for( int x = 0; x < w; ++x )
		{
			// shift each color component by 1 bit and set alpha
			// to 0xff
			d[x] = ( ( d[x] >> 1 ) & 0x7f7f7f7f ) | 0xff000000;
		}
	}
}



class SpectrumView : public QWidget
{
public:
	SpectrumView( SpectrumAnalyzer* s, QWidget * _parent ) :
		QWidget( _parent ),
		m_sa( s ),
		m_backgroundPlain( PLUGIN_NAME::getIconPixmap( "spectrum_background_plain" ).toImage() ),
		m_background( PLUGIN_NAME::getIconPixmap( "spectrum_background" ).toImage() )
	{
		setFixedSize( 249, 151 );
		connect( gui->mainWindow(), SIGNAL( periodicUpdate() ), this, SLOT( update() ) );
		setAttribute( Qt::WA_OpaquePaintEvent, true );
	}

	virtual ~SpectrumView()
	{
	}

	virtual void paintEvent( QPaintEvent* event )
	{
		QPainter p( this );
		QImage i = m_sa->m_saControls.m_linearSpec.value() ?
					m_backgroundPlain : m_background;
		const float e = m_sa->m_energy;
		if( e <= 0 )
		{
			darken( i, 0, 0, i.width(), i.height() );
			p.drawImage( 0, 0, i );
			return;
		}

		const bool lin_y = m_sa->m_saControls.m_linearYAxis.value();
		float * b = m_sa->m_bands;
		const int LOWER_Y = -60;	// dB
		int h;
		const int fh = height();
		if( m_sa->m_saControls.m_linearSpec.value() )
		{
			if( lin_y )
			{
				for( int x = 0; x < MAX_BANDS; ++x, ++b )
				{
					h = fh * 2.0 / 3.0 * (*b / e );
					if( h < 0 ) h = 0; else if( h >= fh ) continue;
					darken( i, x, 0, 1, fh-h );
				}
			}
			else
			{
				for( int x = 0; x < MAX_BANDS; ++x, ++b )
				{
					h = (int)( fh * 2.0 / 3.0 * (20*(log10( *b / e ) ) - LOWER_Y ) / (-LOWER_Y ) );
					if( h < 0 ) h = 0; else if( h >= fh ) continue;
					darken( i, x, 0, 1, fh-h );
				}
			}
		}
		else
		{
			if( lin_y )
			{
				for( int x = 0; x < 31; ++x, ++b )
				{
					h = fh * 2.0 / 3.0 * ( 1.2 * *b / e );
					if( h < 0 ) h = 0; else if( h >= fh ) continue; else h = ( h / 3 ) * 3;
					darken( i, x*8, 0, 8, fh-h );
				}
			}
			else
			{
				for( int x = 0; x < 31; ++x, ++b )
				{
					h = (int)( fh * 2.0 / 3.0 * (20*(log10( *b / e ) ) - LOWER_Y ) / (-LOWER_Y ) );
					if( h < 0 ) h = 0; else if( h >= fh ) continue; else h = ( h / 3 ) * 3;
					darken( i, x*8, 0, 8, fh-h );
				}
			}
			darken( i, 31*8, 0, 1, fh );
		}
		p.drawImage( 0, 0, i );
	}


private:
	SpectrumAnalyzer * m_sa;
	QImage m_backgroundPlain;
	QImage m_background;

} ;




SpectrumAnalyzerControlDialog::SpectrumAnalyzerControlDialog( SpectrumAnalyzerControls* controls ) :
	EffectControlDialog( controls ),
	m_controls( controls ),
	m_logXAxis( PLUGIN_NAME::getIconPixmap( "log_x_axis" ) ),
	m_logYAxis( PLUGIN_NAME::getIconPixmap( "log_y_axis" ) )
{
	setAutoFillBackground( true );
	QPalette pal;
	pal.setBrush( backgroundRole(), Qt::black );//PLUGIN_NAME::getIconPixmap( "background" ) );
	setFixedSize( 293, 229 );
	setPalette( pal );

        /*QVBoxLayout * l = new QVBoxLayout( this );*/

	SpectrumView* v = new SpectrumView( controls->m_effect, this );
	v->move( 34, 10 );

	LedCheckBox * lin_spec = new LedCheckBox( tr( "Linear spectrum (X axis)" ), this );
	lin_spec->move( 32, 182 );
	lin_spec->setModel( &controls->m_linearSpec );

	LedCheckBox * lin_y = new LedCheckBox( tr( "Linear power (Y axis)" ), this );
	lin_y->move( 32, 200 );
	lin_y->setModel( &controls->m_linearYAxis );

        Knob * channelKNB=new Knob(this);
        channelKNB->setText("CH");
        channelKNB->setModel(&controls->m_channelMode);
        channelKNB->move(281-37,182);

	//connect( &controls->m_linearSpec,  SIGNAL( dataChanged() ), this, SLOT( update() ) );
	//connect( &controls->m_linearYAxis, SIGNAL( dataChanged() ), this, SLOT( update() ) );
	//connect( &controls->m_channelMode, SIGNAL( dataChanged() ), this, SLOT( update() ) );

	/*
	l->addWidget( v );
	l->addWidget( lin_spec );
	l->addWidget( lin_y );
	*/
}

SpectrumAnalyzerControlDialog::~SpectrumAnalyzerControlDialog()
{
}

void SpectrumAnalyzerControlDialog::setVisible(bool _b)
{
	EffectControlDialog::setVisible(_b);
	if( m_controls->m_effect )
		m_controls->m_effect->setDontRun(!_b);
}

void SpectrumAnalyzerControlDialog::paintEvent( QPaintEvent * )
{
	QPainter p( this );

	if( !m_controls->m_linearSpec.value() )
	{
		p.drawPixmap( 33, 165, m_logXAxis );
	}

	if( !m_controls->m_linearYAxis.value() )
	{
		p.drawPixmap( 10, 29, m_logYAxis);
	}

}
