/*
 * RandomGDXControls.cpp - controls for click remover effect
 *
 * Copyright (c) 2017
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

#include "RandomGDXControls.h"
#include "RandomGDX.h"
#include "Engine.h"
#include "Song.h"


RandomGDXControls::RandomGDXControls( RandomGDXEffect* effect ) :
	EffectControls( effect ),
	m_effect( effect ),
	m_rndAmpModel( 0.0f, 0.0f, 1.0f, 0.00001f, this, tr( "Random Amp" ) ),
	m_fixAmpModel( 1.0f, 0.0f, 1.0f, 0.00001f, this, tr( "Fixed Amp" ) ),
	m_sngPosModel( 0.0f, 0.0f, 1.0f, 0.00001f, this, tr( "Singularity" ) ),
	m_delPosModel( 0.0f, 0.0f, 1.0f,   0.001f, this, tr( "Delay" ) )
{
/*
	connect( &m_volumeModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_panModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_leftModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_rightModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
*/
}




void RandomGDXControls::changeControl()
{
//	engine::getSong()->setModified();
}




void RandomGDXControls::loadSettings( const QDomElement& _this )
{
	m_rndAmpModel.loadSettings( _this, "rnd_amp" );
	m_fixAmpModel.loadSettings( _this, "fix_amp" );
	m_sngPosModel.loadSettings( _this, "sng_pos" );
	m_delPosModel.loadSettings( _this, "del_pos" );
}




void RandomGDXControls::saveSettings( QDomDocument& doc, QDomElement& _this )
{
	m_rndAmpModel.saveSettings( doc, _this, "rnd_amp" );
	m_fixAmpModel.saveSettings( doc, _this, "fix_amp" );
	m_sngPosModel.saveSettings( doc, _this, "sng_pos" );
}
