/*
 * ScatterGDXControls.cpp - controls for scatter remover effect
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

#include "ScatterGDXControls.h"
#include "ScatterGDX.h"
#include "Engine.h"
#include "Song.h"


ScatterGDXControls::ScatterGDXControls( ScatterGDXEffect* effect ) :
	EffectControls( effect ),
	m_effect( effect ),
	m_pwrModel(  0.f,  0.f, 8.f, 1.f,      this, tr( "Power"    ) ),
        m_spdModel(  1.f, -1.f, 1.f, 0.00001f, this, tr( "Speed"    ) ),
	m_frcModel(  2.f,  1.f, 8.f, 1.f,      this, tr( "Fraction" ) ),
	m_ovrModel(  0.f,  0.f, 1.f, 0.00001f, this, tr( "Override" ) ),
	m_strModel(  0.f,  0.f, 1.f, 0.00001f, this, tr( "Start"    ) )
{
/*	connect( &m_volumeModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_panModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_leftModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_rightModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );*/
}




void ScatterGDXControls::changeControl()
{
//	engine::getSong()->setModified();
}




void ScatterGDXControls::loadSettings( const QDomElement& _this )
{
	m_pwrModel.loadSettings( _this, "power" );
	m_spdModel.loadSettings( _this, "speed" );
	m_frcModel.loadSettings( _this, "fraction" );
	m_ovrModel.loadSettings( _this, "override" );
	m_strModel.loadSettings( _this, "start" );
}




void ScatterGDXControls::saveSettings( QDomDocument& doc, QDomElement& _this )
{
	m_pwrModel.saveSettings( doc, _this, "power" );
	m_spdModel.saveSettings( doc, _this, "speed" );
	m_frcModel.saveSettings( doc, _this, "fraction" );
	m_ovrModel.saveSettings( doc, _this, "override" );
	m_strModel.saveSettings( doc, _this, "start" );
}
