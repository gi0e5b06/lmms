/*
 * ScatterGDX.cpp - A scatter
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include <math.h>
#include "ScatterGDX.h"
#include "embed.h"
#include "Engine.h"
#include "Song.h"

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT scattergdx_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"ScatterGDX",
	QT_TRANSLATE_NOOP( "pluginBrowser", "A scatter plugin" ),
	"gi0e5b06 (on github.com)",
	0x0100,
	Plugin::Effect,
	new PluginPixmapLoader("logo"),
	NULL,
	NULL
} ;

}



ScatterGDXEffect::ScatterGDXEffect( Model* parent, const Descriptor::SubPluginFeatures::Key* key ) :
	Effect( &scattergdx_plugin_descriptor, parent, key ),
	m_gdxControls( this ),
        m_len(0),
        m_prev(0),
        m_pos(0),
        m_time(0),
        m_end(0)
{
        m_buffer=new sampleFrame[384000];
}




ScatterGDXEffect::~ScatterGDXEffect()
{
}




bool ScatterGDXEffect::processAudioBuffer( sampleFrame* buf, const fpp_t frames )
{
	if( !isEnabled() || !isRunning () ) return false;

        uint32_t MAXT=(uint32_t)(4.f*Engine::mixer()->baseSampleRate()*120.f/Engine::getSong()->getTempo());
        while(MAXT>384000) MAXT/=2;
        uint32_t MINT=MAXT/32; //unused

        int p=(int)m_gdxControls.m_lenModel.value();
        if(p<0) p=0;
        if(p>8) p=8;
        if(p==0)
        {
                if(m_pos<MAXT)
                {
                        for(fpp_t f=0; (f<frames)&&(m_pos<MAXT); ++f, ++m_pos)
                        {
                                m_buffer[m_pos][0]=buf[f][0];
                                m_buffer[m_pos][1]=buf[f][1];
                        }
                }
                m_prev=0;
                return false;
        }

        m_len=1<<(p-1);

        if(m_len!=m_prev)
	{
                m_time=0;
                m_end=MAXT/m_len;
	}

        if(m_prev==0) m_pos=0;
        m_prev=m_len;

        if(m_pos<MAXT)
        {
                for(fpp_t f=0; (f<frames)&&(m_pos<MAXT); ++f, ++m_pos)
                {
                        m_buffer[m_pos][0]=buf[f][0];
                        m_buffer[m_pos][1]=buf[f][1];
                }
        }

        m_end=MAXT/m_len;

        qInfo("MAXT=%d MINT=%d time=%d end=%d pos=%d",MAXT,MINT,m_time,m_end,m_pos);

	float curVal0;
	float curVal1;

	double outSum = 0.0;

	const float d = dryLevel();
	const float w = wetLevel();

	for( fpp_t f = 0; f < frames; ++f )
	{
		curVal0=buf[f][0];
		curVal1=buf[f][1];
		outSum += curVal0*curVal0+curVal1*curVal1;

                if(m_time>=m_end) m_time=0;

                curVal0=m_buffer[m_time][0];
                curVal1=m_buffer[m_time][1];
                m_time++;

		buf[f][0] = d * buf[f][0] + w * curVal0;
		buf[f][1] = d * buf[f][1] + w * curVal1;
	}

	checkGate( outSum / frames );

	return isRunning();
}


extern "C"
{

// necessary for getting instance out of shared lib
Plugin * PLUGIN_EXPORT lmms_plugin_main( Model* parent, void* data )
{
	return new ScatterGDXEffect( parent, static_cast<const Plugin::Descriptor::SubPluginFeatures::Key *>( data ) );
}

}

