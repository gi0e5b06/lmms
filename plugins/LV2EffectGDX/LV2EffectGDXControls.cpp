/*
 * LV2EffectGDXControls.cpp - model for LV2 plugin controls
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

#include <QDomElement>

#include "LV2EffectGDX.h"


LV2EffectGDXControls::LV2EffectGDXControls(LV2EffectGDX* _effect) :
	EffectControls(_effect),
	m_effect(_effect),
	m_processors(_effect->processorCount()),
	m_noLink(false),
	m_stereoLinkModel(true,this)
{
	connect( &m_stereoLinkModel, SIGNAL( dataChanged() ),
				this, SLOT( updateLinkStatesFromGlobal() ) );

	lv2_multi_proc_t controls = m_effect->getPortControls();
	m_controlCount = controls.count();
        qInfo("LV2EffectGDXControls::LV2EffectGDXControls m_controlCount=%d",m_controlCount);

	for( ch_cnt_t proc = 0; proc < m_processors; proc++ )
	{
		lv2_control_list_t p;

		const bool linked_control = ( m_processors > 1 && proc == 0 );

		for( lv2_multi_proc_t::Iterator it = controls.begin(); it != controls.end(); it++ )
		{
			if( (*it)->proc == proc )
			{
				(*it)->control = new LV2Control( this, *it,
							linked_control );

				p.append( (*it)->control );

				if( linked_control )
				{
					connect( (*it)->control, SIGNAL( linkChanged( int, bool ) ),
								this, SLOT( linkPort( int, bool ) ) );
				}
			}
		}

		m_controls.append( p );
	}

	// now link all controls
	if( m_processors > 1 )
	{
		for( lv2_multi_proc_t::Iterator it = controls.begin(); 
						it != controls.end(); it++ )
		{
			if( (*it)->proc == 0 )
			{
				linkPort( ( *it )->control_id, true );
			}
		}
	}
}




LV2EffectGDXControls::~LV2EffectGDXControls()
{
	for( ch_cnt_t proc = 0; proc < m_processors; proc++ )
	{
		m_controls[proc].clear();
	}
	m_controls.clear();
}




void LV2EffectGDXControls::saveSettings( QDomDocument & _doc, QDomElement & _this )
{
	if( m_processors > 1 )
	{
		_this.setAttribute( "link", m_stereoLinkModel.value() );
	}

	lv2_multi_proc_t controls = m_effect->getPortControls();
	_this.setAttribute( "ports", controls.count() );
	for( lv2_multi_proc_t::Iterator it = controls.begin(); 
						it != controls.end(); it++ )
	{
		QString n = "port" + QString::number( (*it)->proc ) + 
					QString::number( (*it)->port_id );
		(*it)->control->saveSettings( _doc, _this, n );
	}
}




void LV2EffectGDXControls::loadSettings( const QDomElement & _this )
{
	if( m_processors > 1 )
	{
		m_stereoLinkModel.setValue( _this.attribute( "link" ).toInt() );
	}

	lv2_multi_proc_t controls = m_effect->getPortControls();
	for( lv2_multi_proc_t::Iterator it = controls.begin(); 
						it != controls.end(); it++ )
	{
		QString n = "port" + QString::number( (*it)->proc ) + 
					QString::number( (*it)->port_id );
		(*it)->control->loadSettings( _this, n );
	}
}




void LV2EffectGDXControls::linkPort( int _port, bool _state )
{
	LV2Control * first = m_controls[0][_port];
	if( _state )
	{
		for( ch_cnt_t proc = 1; proc < m_processors; proc++ )
		{
			first->linkControls( m_controls[proc][_port] );
		}
	}
	else
	{
		for( ch_cnt_t proc = 1; proc < m_processors; proc++ )
		{
			first->unlinkControls( m_controls[proc][_port] );
		}
		m_noLink = true;
		m_stereoLinkModel.setValue( false );
	}
}



void LV2EffectGDXControls::updateLinkStatesFromGlobal()
{
	if( m_stereoLinkModel.value() )
	{
		for( int port = 0; port < m_controlCount / m_processors; port++ )
		{
			m_controls[0][port]->setLink( true );
		}
	}
	else if( !m_noLink )
	{
		for( int port = 0; port < m_controlCount / m_processors; port++ )
		{
			m_controls[0][port]->setLink( false );
		}
	}

	// if global channel link state has changed, always ignore link
	// status of individual ports in the future
	m_noLink = false;
}

