/*
 * ScatterGDXControlDialog.cpp - control dialog for scatter remover effect
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

#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>

#include "ScatterGDXControlDialog.h"
#include "ScatterGDXControls.h"
#include "embed.h"



ScatterGDXControlDialog::ScatterGDXControlDialog( ScatterGDXControls* controls ) :
	EffectControlDialog( controls )
{
	setAutoFillBackground( true );
	QPalette pal;
	pal.setBrush( backgroundRole(), PLUGIN_NAME::getIconPixmap( "artwork" ) );
	setPalette( pal );
	setFixedSize( 160, 120 );

        Knob* pwrKnob = new Knob( knobBright_26, this );
	pwrKnob -> move( 10, 10 );
	pwrKnob->setModel( &controls->m_pwrModel );
	pwrKnob->setLabel( tr( "Power" ) );
	pwrKnob->setHintText( tr( "Power:" ) , "/bar" );

        Knob* spdKnob = new Knob( knobBright_26, this );
	spdKnob -> move( 80, 10 );
	spdKnob->setModel( &controls->m_spdModel );
	spdKnob->setLabel( tr( "Speed" ) );
	spdKnob->setHintText( tr( "Speed:" ) , "" );

        Knob* frcKnob = new Knob( knobBright_26, this );
	frcKnob -> move( 10, 60 );
	frcKnob->setModel( &controls->m_frcModel );
	frcKnob->setLabel( tr( "Fraction" ) );
	frcKnob->setHintText( tr( "Fraction:" ) , "" );

        Knob* ovrKnob = new Knob( knobBright_26, this );
	ovrKnob -> move( 80, 60 );
	ovrKnob->setModel( &controls->m_ovrModel );
	ovrKnob->setLabel( tr( "Override" ) );
	ovrKnob->setHintText( tr( "Override:" ) , "" );
}
