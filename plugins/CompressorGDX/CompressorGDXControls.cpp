/*
 * CompressorGDXControls.cpp - controls for click remover effect
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

#include "CompressorGDXControls.h"
#include "CompressorGDX.h"
#include "Engine.h"
#include "Song.h"


CompressorGDXControls::CompressorGDXControls( CompressorGDXEffect* effect ) :
	EffectControls( effect ),
	m_effect( effect ),
	m_rndAmpModel( 0.0f, 0.0f, 1.0f, 0.001f, this, tr( "Treshold" ) ),
	m_fixAmpModel( 1.0f, 0.0f, 1.0f, 0.001f, this, tr( "Ratio" ) ),
	m_sngPosModel( 1.0f, 0.0f, 5.0f,  0.01f, this, tr( "Out gain" ) ),
	m_delPosModel( 0.0f, 0.0f, 1.0f, 0.001f, this, tr( "Delay" ) )
{
/*
	connect( &m_volumeModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_panModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_leftModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_rightModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
*/
}




void CompressorGDXControls::changeControl()
{
//	engine::getSong()->setModified();
}




void CompressorGDXControls::loadSettings( const QDomElement& _this )
{
	m_rndAmpModel.loadSettings( _this, "treshold" );
	m_fixAmpModel.loadSettings( _this, "ratio" );
	m_sngPosModel.loadSettings( _this, "out_gain" );
	//m_delPosModel.loadSettings( _this, "del_pos" );
}




void CompressorGDXControls::saveSettings( QDomDocument& doc, QDomElement& _this )
{
	m_rndAmpModel.saveSettings( doc, _this, "treshold" );
	m_fixAmpModel.saveSettings( doc, _this, "ratio" );
	m_sngPosModel.saveSettings( doc, _this, "out_gain" );
	//m_delPosModel.saveSettings( doc, _this, "del_pos" );
}
