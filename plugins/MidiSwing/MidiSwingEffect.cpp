/*
 * MidiSwingEffect.cpp - MIDI swing effect on time (noteon), velocity,
 *                       length (noteoff)
 *
 * Copyright (c) 2017 gi0e5b06
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


#include "MidiSwingEffect.h"

#include "embed.h"


extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT midiswing_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"Swing",
	QT_TRANSLATE_NOOP( "pluginBrowser",
			   "Plugin" ),
	"gi0e5b06 on github.com",
	0x0100,
	Plugin::MidiEffect,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;

}



MidiSwingEffect::MidiSwingEffect(
			Model * _parent,
			const Descriptor::SubPluginFeatures::Key * _key )
//:
	//Effect( &midiswing_plugin_descriptor, _parent, _key ),
	//m_smControls( this )
{
}




MidiSwingEffect::~MidiSwingEffect()
{
}



