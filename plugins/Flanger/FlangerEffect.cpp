/*
 * flangereffect.cpp - defination of FlangerEffect class.
 *
 * Copyright (c) 2014 David French <dave/dot/french3/at/googlemail/dot/com>
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

#include "FlangerEffect.h"
#include "Engine.h"
#include "embed.h"

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT flanger_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"Flanger",
	QT_TRANSLATE_NOOP( "pluginBrowser", "A native flanger plugin" ),
	"Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>",
	0x0100,
	Plugin::Effect,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;




FlangerEffect::FlangerEffect( Model *parent, const Plugin::Descriptor::SubPluginFeatures::Key *key ) :
	Effect( &flanger_plugin_descriptor, parent, key ),
	m_flangerControls( this )
{
	m_lfo = new QuadratureLfo( Engine::mixer()->processingSampleRate() );
	m_lDelay = new MonoDelay( 1, Engine::mixer()->processingSampleRate() );
	m_rDelay = new MonoDelay( 1, Engine::mixer()->processingSampleRate() );
	m_noise = new Noise;
}




FlangerEffect::~FlangerEffect()
{
	if(m_lDelay )
	{
		delete m_lDelay;
	}
	if( m_rDelay )
	{
		delete m_rDelay;
	}
	if(m_lfo )
	{
		delete m_lfo;
	}
	if(m_noise)
	{
		delete m_noise;
	}
}




bool FlangerEffect::processAudioBuffer( sampleFrame *_buf, const fpp_t _frames )
{
        bool smoothBegin, smoothEnd;
        if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
                return false;

	const float length = m_flangerControls.m_delayTimeModel.value() * Engine::mixer()->processingSampleRate();
	const float noise = m_flangerControls.m_whiteNoiseAmountModel.value();
	float amplitude = m_flangerControls.m_lfoAmountModel.value() * Engine::mixer()->processingSampleRate();
	bool invertFeedback = m_flangerControls.m_invertFeedbackModel.value();
	m_lfo->setFrequency(  1.0/m_flangerControls.m_lfoFrequencyModel.value() );
	m_lDelay->setFeedback( m_flangerControls.m_feedbackModel.value() );
	m_rDelay->setFeedback( m_flangerControls.m_feedbackModel.value() );

	float leftLfo;
	float rightLfo;

	for( fpp_t f = 0; f < _frames; ++f )
	{
                float w0, d0, w1, d1;
                computeWetDryLevels(f, _frames, smoothBegin, smoothEnd,
                                    w0, d0, w1, d1);

                sample_t curVal0 = _buf[f][0];
                sample_t curVal1 = _buf[f][1];

		curVal0 += m_noise->tick() * noise;
		curVal1 += m_noise->tick() * noise;
		m_lfo->tick(&leftLfo, &rightLfo);
		m_lDelay->setLength( ( float )length + amplitude * (leftLfo+1.0)  );
		m_rDelay->setLength( ( float )length + amplitude * (rightLfo+1.0)  );
		if(invertFeedback)
		{
			m_lDelay->tick(&curVal1);
			m_rDelay->tick(&curVal0);
		} else
		{
			m_lDelay->tick(&curVal0);
			m_rDelay->tick(&curVal1);
		}

                _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0;
                _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
	}

	return true;
}




void FlangerEffect::changeSampleRate()
{
	m_lfo->setSampleRate( Engine::mixer()->processingSampleRate() );
	m_lDelay->setSampleRate( Engine::mixer()->processingSampleRate() );
	m_rDelay->setSampleRate( Engine::mixer()->processingSampleRate() );
}



extern "C"
{

//needed for getting plugin out of shared lib
Plugin * PLUGIN_EXPORT lmms_plugin_main( Model* parent, void* data )
{
	return new FlangerEffect( parent , static_cast<const Plugin::Descriptor::SubPluginFeatures::Key *>( data ) );
}

}}
