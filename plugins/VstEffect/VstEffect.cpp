/*
 * VstEffect.cpp - class for handling VST effect plugins
 *
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

#include <QMessageBox>

#include "VstEffect.h"
#include "Song.h"
#include "TextFloat.h"
#include "VstSubPluginFeatures.h"

#include "embed.h"


extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT vsteffect_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"VST",
	QT_TRANSLATE_NOOP( "pluginBrowser",
				"plugin for using arbitrary VST effects inside LMMS." ),
	"Tobias Doerffel <tobydox/at/users.sf.net>",
	0x0200,
	Plugin::Effect,
	new PluginPixmapLoader("logo"),
	NULL,
	new VstSubPluginFeatures( Plugin::Effect )
} ;

}


VstEffect::VstEffect( Model * _parent,
			const Descriptor::SubPluginFeatures::Key * _key ) :
	Effect( &vsteffect_plugin_descriptor, _parent, _key ),
	m_plugin( NULL ),
	m_pluginMutex(),
	m_key( *_key ),
	m_vstControls( this )
{
        setColor(QColor(128,96,74));

	if( !m_key.attributes["file"].isEmpty() )
	{
		openPlugin( m_key.attributes["file"] );
	}
	setDisplayName( m_key.attributes["file"].section( ".dll", 0, 0 ).isEmpty()
		? m_key.name : m_key.attributes["file"].section( ".dll", 0, 0 ) );
}




VstEffect::~VstEffect()
{
	closePlugin();
}




bool VstEffect::processAudioBuffer( sampleFrame * _buf, const fpp_t _frames )
{
        bool smoothBegin, smoothEnd;
        if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
                return false;

	if(! m_plugin ) return false;

        sampleFrame* vstbuf = MM_ALLOC(sampleFrame,_frames);

        memcpy( vstbuf, _buf, sizeof( sampleFrame ) * _frames );

        if(m_pluginMutex.tryLock())
        {
                m_plugin->process( vstbuf, vstbuf );
                m_pluginMutex.unlock();
        }

        for(fpp_t f = 0; f < _frames; ++f)
        {
                float w0, d0, w1, d1;
                computeWetDryLevels(f, _frames, smoothBegin, smoothEnd,
                                    w0, d0, w1, d1);

                _buf[f][0] = d0 * _buf[f][0] + w0 * vstbuf[f][0];
                _buf[f][1] = d1 * _buf[f][1] + w1 * vstbuf[f][1];
        }

        MM_FREE(vstbuf);

        return true;
}




void VstEffect::openPlugin( const QString & _plugin )
{
	TextFloat * tf = TextFloat::displayMessage(
		VstPlugin::tr( "Loading plugin" ),
		VstPlugin::tr( "Please wait while loading VST plugin..." ),
			PLUGIN_NAME::getIconPixmap( "logo", 24, 24 ), 0 );
	m_pluginMutex.lock();
	m_plugin = new VstPlugin( _plugin );
	if( m_plugin->failed() )
	{
		m_pluginMutex.unlock();
		closePlugin();
		delete tf;
		collectErrorForUI( VstPlugin::tr( "The VST plugin %1 could not be loaded." ).arg( _plugin ) );
		return;
	}

	VstPlugin::connect( Engine::getSong(), SIGNAL( tempoChanged( bpm_t ) ), m_plugin, SLOT( setTempo( bpm_t ) ) );
	m_plugin->setTempo( Engine::getSong()->getTempo() );

	m_pluginMutex.unlock();

	delete tf;

	m_key.attributes["file"] = _plugin;
}



void VstEffect::closePlugin()
{
	m_pluginMutex.lock();
	if( m_plugin && m_plugin->pluginWidget() != NULL )
	{
		delete m_plugin->pluginWidget();
	}
	delete m_plugin;
	m_plugin = NULL;
	m_pluginMutex.unlock();
}





extern "C"
{

// necessary for getting instance out of shared lib
Plugin * PLUGIN_EXPORT lmms_plugin_main( Model * _parent, void * _data )
{
	return new VstEffect( _parent,
		static_cast<const Plugin::Descriptor::SubPluginFeatures::Key *>(
								_data ) );
}

}

