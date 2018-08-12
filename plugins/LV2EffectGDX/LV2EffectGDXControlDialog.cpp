/*
 * LV2EffectGDXControlDialog.cpp - dialog for displaying and editing control port
 *                             values for LV2 plugins
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
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

#include <QGroupBox>
#include <QLayout>

#include "LV2EffectGDX.h"
#include "LV2EffectGDXControlDialog.h"
#include "LV2ControlView.h"
#include "LedCheckBox.h"



LV2EffectGDXControlDialog::LV2EffectGDXControlDialog(LV2EffectGDXControls* _controls) :
	EffectControlDialog(_controls),
	m_effectLayout(NULL),
	m_stereoLink(NULL)
{
	QVBoxLayout* mainLay=new QVBoxLayout(this);

	m_effectLayout=new QHBoxLayout();
	mainLay->addLayout(m_effectLayout);

	updateEffectView(_controls);

	if(_controls->m_processors > 1 )
	{
		mainLay->addSpacing( 3 );
		QHBoxLayout * center = new QHBoxLayout();
		mainLay->addLayout( center );
		m_stereoLink = new LedCheckBox( tr( "Link Channels" ), this );
		m_stereoLink->setModel( &_controls->m_stereoLinkModel );
		center->addWidget( m_stereoLink );
	}
}




LV2EffectGDXControlDialog::~LV2EffectGDXControlDialog()
{
}




void LV2EffectGDXControlDialog::updateEffectView(LV2EffectGDXControls* _controls)
{
	QList<QGroupBox *> list = findChildren<QGroupBox *>();
	for( QList<QGroupBox *>::iterator it = list.begin(); it != list.end();
									++it )
	{
		delete *it;
	}

	m_effectControls=_controls;


	const int cols = static_cast<int>( sqrt( 
		static_cast<double>( _controls->m_controlCount /
						_controls->m_processors ) ) );
	for( ch_cnt_t proc = 0; proc < _controls->m_processors; proc++ )
	{
		lv2_control_list_t & controls = _controls->m_controls[proc];
		int row = 0;
		int col = 0;
		buffer_data_t last_port = NONE;

		QGroupBox * grouper;
		if( _controls->m_processors > 1 )
		{
			grouper = new QGroupBox( tr( "Channel " ) +
						QString::number( proc + 1 ),
								this );
		}
		else
		{
			grouper = new QGroupBox( this );
		}

		QGridLayout * gl = new QGridLayout( grouper );
		grouper->setLayout( gl );
		grouper->setAlignment( Qt::Vertical );

		int cw=0,rh=0;
		for( lv2_control_list_t::iterator it = controls.begin();
                     it != controls.end(); ++it )
		{
			if( (*it)->port()->proc == proc )
			{
				if( last_port != NONE &&
				    (*it)->port()->data_type == TOGGLED &&
				    !( (*it)->port()->data_type == TOGGLED &&
				       last_port == TOGGLED ) )
				{
					++row;
					col = 0;
				}
				QWidget* qw=new LV2ControlView( grouper, *it );
				gl->addWidget(qw,row,col);
				cw=qMax<int>(cw,qw->sizeHint().width());
				rh=qMax<int>(rh,qw->sizeHint().height());
				if( ++col == cols )
				{
					++row;
					col = 0;
				}
				last_port = (*it)->port()->data_type;
			}
		}

		const int rows=(col==0 ? row : row+1);

		for(col=0;col<cols;col++)
		{
			gl->setColumnMinimumWidth(col,cw);
			gl->setColumnStretch(col,1.f);
		}

		for(row=0;row<rows;row++)
		{
			gl->setRowMinimumHeight(row,rh);
			gl->setRowStretch(row,1.f);
		}

		m_effectLayout->addWidget( grouper );
	}

	if( _controls->m_processors > 1 && m_stereoLink != NULL )
	{
		m_stereoLink->setModel( &_controls->m_stereoLinkModel );
	}

	connect( _controls, SIGNAL( effectModelChanged( LV2EffectGDXControls* ) ),
                 this, SLOT( updateEffectView( LV2EffectGDXControls* ) ),
                 Qt::DirectConnection );
}




