/*
 * stereo_matrix.cpp - stereo-matrix-effect-plugin
 *
 * Copyright (c) 2008 Paul Giblock <drfaygo/at/gmail/dot/com>
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


#include "stereo_matrix.h"

#include "embed.h"


extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT stereomatrix_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"Stereo Matrix",
	QT_TRANSLATE_NOOP( "pluginBrowser",
				"Plugin for freely manipulating stereo output" ),
	"Paul Giblock <drfaygo/at/gmail.com>",
	0x0100,
	Plugin::Effect,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;

}



stereoMatrixEffect::stereoMatrixEffect(
			Model * _parent,
			const Descriptor::SubPluginFeatures::Key * _key ) :
	Effect( &stereomatrix_plugin_descriptor, _parent, _key ),
	m_smControls( this )
{
}




stereoMatrixEffect::~stereoMatrixEffect()
{
}



bool stereoMatrixEffect::processAudioBuffer( sampleFrame * _buf,
							const fpp_t _frames )
{
        bool smoothBegin, smoothEnd;
        if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
                return false;

        for( fpp_t f = 0; f < _frames; ++f )
        {
                float w0, d0, w1, d1;
                computeWetDryLevels(f, _frames, smoothBegin, smoothEnd,
                                    w0, d0, w1, d1);

		sample_t l = _buf[f][0];
		sample_t r = _buf[f][1];

		_buf[f][0] = l * d0 +
                        ( m_smControls.m_llModel.value( f ) * l  +
                          m_smControls.m_rlModel.value( f ) * r ) * w0;

		_buf[f][1] = r * d1 +
                        ( m_smControls.m_lrModel.value( f ) * l  +
                          m_smControls.m_rrModel.value( f ) * r ) * w1;
	}

	return true;
}




extern "C"
{

// necessary for getting instance out of shared lib
Plugin * PLUGIN_EXPORT lmms_plugin_main( Model * _parent, void * _data )
{
	return( new stereoMatrixEffect( _parent,
		static_cast<const Plugin::Descriptor::SubPluginFeatures::Key *>(
								_data ) ) );
}

}
