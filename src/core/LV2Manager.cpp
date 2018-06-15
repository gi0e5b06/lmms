/*
 * LV2Manager.cpp - a class to manage loading and instantiation
 *                      of lv2 plugins
 *
 * Copyright (c) 2005-2008 Danny McRae <khjklujn@netscape.net>
 * Copyright (c) 2011-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QLibrary>

#include <math.h>

#include "ConfigManager.h"
#include "LV2Manager.h"
#include "PluginFactory.h"



LV2Manager::LV2Manager()
{
	// Make sure plugin search paths are set up
	PluginFactory::instance();

        // A world contains all the info about every plugin on the system
        m_world=new Lilv::World();

        //NP_ATOM
        NP_AUDIO    =m_world->new_uri(LILV_URI_AUDIO_PORT);
        NP_CONTROL  =m_world->new_uri(LILV_URI_CONTROL_PORT);
        //NP_CV
        //NP_EVENT    =m_world->new_uri(LILV_URI_EVENT_PORT);
        NP_INPUT    =m_world->new_uri(LILV_URI_INPUT_PORT);
        NP_MIDI     =m_world->new_uri(LILV_URI_MIDI_EVENT);
        NP_OUTPUT   =m_world->new_uri(LILV_URI_OUTPUT_PORT);
        //NP_PORT

        PP_INTEGER  =m_world->new_uri(LV2_CORE__integer);
        PP_LOGARITHM=m_world->new_uri("http://lv2plug.in/ns/ext/port-props/#logarithmic");
        PP_TOGGLED  =m_world->new_uri(LV2_CORE__toggled);
        //qWarning("LV2Manager: lilv loads world of plugins now");
        // load the entire world
        m_world->load_all();
        // then get them: so far they're only stored in memory, and now we get
        // a variable name with which we can acces / instantiate the plugins
        Lilv::Plugins tmp=(Lilv::Plugins)m_world->get_all_plugins();

        for(LilvIter* i=tmp.begin();
            !tmp.is_end(i);
            i=tmp.next(i) )
        {
                Lilv::Plugin p=tmp.get(i);
                QString u=p.get_uri().as_string();
                addPlugin(u);
        }

        qSort(m_effects);
        qSort(m_instruments);
        qSort(m_invalids);
        qSort(m_tools);
        qSort(m_others);
}

/*
        QStringList lv2Directories = QString( getenv( "LV2_PATH" ) ).
                split( LV2_PATH_SEPARATOR );
	lv2Directories += ConfigManager::inst()->lv2Dir().split( ',' );

	lv2Directories.push_back( "plugins:lv2" );
#ifndef LMMS_BUILD_WIN32
	lv2Directories.push_back( qApp->applicationDirPath() + '/' + LIB_DIR + "lv2" );
	lv2Directories.push_back( "/usr/lib/lv2" );
	lv2Directories.push_back( "/usr/lib64/lv2" );
	lv2Directories.push_back( "/usr/local/lib/lv2" );
	lv2Directories.push_back( "/usr/local/lib64/lv2" );
	lv2Directories.push_back( "/Library/Audio/Plug-Ins/LV2" );
#endif

	for( QStringList::iterator it = lv2Directories.begin(); 
             it != lv2Directories.end(); ++it )
	{
		QDir directory( ( *it ) );
		QFileInfoList list = directory.entryInfoList();
		for( QFileInfoList::iterator file = list.begin();
                     file != list.end(); ++file )
		{
			const QFileInfo & f = *file;
			if( !f.isFile() ||
                            f.fileName().right( 3 ).toLower() !=
#ifdef LMMS_BUILD_WIN32
                            "dll"
#else
                            ".so"
#endif
                            )
                        {
				continue;
			}

			QLibrary plugin_lib( f.absoluteFilePath() );

			if( plugin_lib.load() == true )
			{
				LV2_Descriptor_Function descriptorFunction =
                                        ( LV2_Descriptor_Function ) plugin_lib.resolve
                                        ("lv2_descriptor" );
				if( descriptorFunction != NULL )
				{
					addPlugins( descriptorFunction,
                                                    f.fileName() );
				}
			}
			else
			{
				qWarning() << plugin_lib.errorString();
			}
		}
	}

	l_lv2_key_t keys = m_lv2ManagerMap.keys();
	for( l_lv2_key_t::iterator it = keys.begin();
			it != keys.end(); ++it )
	{
		m_sortedPlugins.append( qMakePair( getName( *it ), *it ) );
	}
	qSort( m_sortedPlugins );
}
*/



LV2Manager::~LV2Manager()
{
        delete m_world;
        /*
	for( lv2ManagerMapType::iterator it = m_lv2ManagerMap.begin();
             it != m_lv2ManagerMap.end(); ++it )
	{
		delete it.value();
	}
        */
}


Lilv::Plugin LV2Manager::plugin(const lv2_key_t& _key)
{
        return ((Lilv::Plugins)m_world->get_all_plugins())
                .get_by_uri(m_world->new_uri(qPrintable(_key)));
}


void LV2Manager::addPlugin(const lv2_key_t& k)
{
        if(m_keys.contains(k)) return;

        m_keys.append(k);
        Lilv::Plugin p=plugin(k);
        QString n=p.get_name().as_string();
        QString c=p.get_class().get_label().as_string();
        qWarning("LV2: %s %s [%s]",
                 qPrintable(n),
                 qPrintable(k),
                 qPrintable(c));

        if(c=="Instrument") m_instruments.push_back(k);
        else if(c=="Analyzer") m_tools.push_back(k);
        else if((c=="Plugin")||(c=="Utility")) m_others.push_back(k);
        else m_effects.push_back(k);

        /*
        Plugin::Descriptor d;
        d.name              =k;
        d.displayName       =nameNode.as_string();
        d.description       ="";
        d.author            =_p.get_author_name().as_string();
        d.version           =0;
        d.type              =Plugin::Effect;
        d.logo              =NULL;
        d.supportedFileTypes=NULL;
        m_descriptors.put(k,d);

        PluginInfo info;
        info.file      =file;
        info.library   =_p.get_library_uri().as_uri();
        info.descriptor=d;
        */
}


/*

lv2ManagerDescription * LV2Manager::getDescription(const lv2_key_t& _key)
{
	if( m_lv2ManagerMap.contains( _key ) )
	{
		return( m_lv2ManagerMap[_key] );
	}
	else
	{
		return( NULL );
	}
}
*/


 /*
void LV2Manager::addPlugins(LV2_Descriptor_Function _descriptor_func,
                            const QString & _file )
{
	const LV2_Descriptor * descriptor;

	for( long pluginIndex = 0;
             ( descriptor = _descriptor_func( pluginIndex ) ) != NULL;
             ++pluginIndex )
	{
		lv2_key_t key( _file, QString( descriptor->Label ) );
		if( m_lv2ManagerMap.contains( key ) )
		{
			continue;
		}

		lv2ManagerDescription * plugIn = 
				new lv2ManagerDescription;
		plugIn->descriptorFunction = _descriptor_func;
		plugIn->index = pluginIndex;
		plugIn->inputChannels = getPluginInputs( descriptor );
		plugIn->outputChannels = getPluginOutputs( descriptor );

		if( plugIn->inputChannels == 0 &&
                    plugIn->outputChannels > 0 )
		{
			plugIn->type = SOURCE;
		}
		else if( plugIn->inputChannels > 0 &&
                         plugIn->outputChannels > 0 )
		{
			plugIn->type = TRANSFER;
		}
		else if( plugIn->inputChannels > 0 &&
                         plugIn->outputChannels == 0 )
		{
			plugIn->type = SINK;
		}
		else
		{
			plugIn->type = OTHER;
		}

		m_lv2ManagerMap[key] = plugIn;
	}
}
 */


/*
uint16_t LV2Manager::getPluginInputs(
		const LV2_Descriptor * _descriptor )
{
	uint16_t inputs = 0;
	
	for( uint16_t port = 0; port < _descriptor->PortCount; port++ )
	{
		if( LV2_IS_PORT_INPUT( 
				_descriptor->PortDescriptors[port] ) &&
			LV2_IS_PORT_AUDIO( 
				_descriptor->PortDescriptors[port] ) )
		{
			QString name = QString( 
					_descriptor->PortNames[port] );
			if( name.toUpper().contains( "IN" ) )
			{
				inputs++;
			}
		}
	}
	return inputs;
}




uint16_t LV2Manager::getPluginOutputs(
		const LV2_Descriptor * _descriptor )
{
	uint16_t outputs = 0;
	
	for( uint16_t port = 0; port < _descriptor->PortCount; port++ )
	{
		if( LV2_IS_PORT_OUTPUT( 
				_descriptor->PortDescriptors[port] ) &&
			LV2_IS_PORT_AUDIO( 
				_descriptor->PortDescriptors[port] ) )
		{
			QString name = QString( 
					_descriptor->PortNames[port] );
			if( name.toUpper().contains( "OUT" ) )
			{
				outputs++;
			}
		}
	}
	return outputs;
}




l_sortable_plugin_t LV2Manager::getSortedPlugins()
{
	return( m_sortedPlugins );
}
*/


// only one plugin by file
QString LV2Manager::getLabel(const lv2_key_t& _key)
{
	return m_keys.contains(_key)
                ? _key
                : "";
}


/*
bool LV2Manager::hasRealTimeDependency(const lv2_key_t&  _key)
{
	if( m_lv2ManagerMap.contains( _key ) )
	{
		LV2_Descriptor_Function descriptorFunction =
			m_lv2ManagerMap[_key]->descriptorFunction;
		const LV2_Descriptor * descriptor =
				descriptorFunction(
					m_lv2ManagerMap[_key]->index );
		return( LV2_IS_REALTIME( descriptor->Properties ) );
	}
	else
	{
		return( false );
	}
}
*/


bool LV2Manager::isInplaceBroken( const lv2_key_t &  _key )
{
        //TODO
        return true;
}


/*
bool LV2Manager::isRealTimeCapable(
					const lv2_key_t &  _key )
{
	if( m_lv2ManagerMap.contains( _key ) )
	{
		LV2_Descriptor_Function descriptorFunction =
			m_lv2ManagerMap[_key]->descriptorFunction;
		const LV2_Descriptor * descriptor =
				descriptorFunction(
					m_lv2ManagerMap[_key]->index );
		return( LV2_IS_HARD_RT_CAPABLE( descriptor->Properties ) );
	}
	else
	{
		return( false );
	}
}
*/



QString LV2Manager::getName(const lv2_key_t& _key)
{
        if(!m_keys.contains(_key)) return "";
        Lilv::Plugin p=LV2Manager::plugin(_key);
	return p.get_name().as_string();
}


/*
QString LV2Manager::getLibrary(const lv2_key_t& _key)
{
	Lilv::Plugin* p=LV2Manager::plugin(_key);
	return p
                ? p->get_library_uri().as_uri()
                : "";
}
*/


QString LV2Manager::getMaker( const lv2_key_t & _key )
{
        if(!m_keys.contains(_key)) return "";
	Lilv::Plugin p=LV2Manager::plugin(_key);
	return QString("%1 <%2>")
                .arg(p.get_author_name().as_string())
                .arg(p.get_author_email().as_string());
}


QString LV2Manager::getCopyright( const lv2_key_t & _key )
{
        if(!m_keys.contains(_key)) return "";
	Lilv::Plugin p=LV2Manager::plugin(_key);
	return p.get_author_homepage().as_string();
}


uint32_t LV2Manager::getPortCount(const lv2_key_t& _key)
{
        if(!m_keys.contains(_key)) return 0;
        Lilv::Plugin p=LV2Manager::plugin(_key);
	return p.get_num_ports();
}


bool LV2Manager::isPortInput(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return false;
        Lilv::Plugin p=LV2Manager::plugin(_key);
        return p.get_port_by_index(_port).is_a(NP_INPUT);
}


bool LV2Manager::isPortOutput(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return false;
        Lilv::Plugin p=LV2Manager::plugin(_key);
        return p.get_port_by_index(_port).is_a(NP_OUTPUT);
}


bool LV2Manager::isPortAudio(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return false;
        Lilv::Plugin p=LV2Manager::plugin(_key);
        return p.get_port_by_index(_port).is_a(NP_AUDIO);
}


bool LV2Manager::isPortControl(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return false;
        Lilv::Plugin p=LV2Manager::plugin(_key);
        return p.get_port_by_index(_port).is_a(NP_CONTROL);
}


/*
bool LV2Manager::areHintsSampleRateDependent(const lv2_key_t& _key, uint32_t _port)
{
        //TODO
        return false;
}
*/


float LV2Manager::getLowerBound(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return 0.f;

        //TODO caching
        uint32_t nbp=getPortCount(_key);
        float vmin[nbp];
        float vmax[nbp];
        float vdef[nbp];
        for(int i=0;i<nbp;i++)
                vmin[i]=vdef[i]=vmax[i]=0.f;

        Lilv::Plugin p=LV2Manager::plugin(_key);
        p.get_port_ranges_float(vmin,vmax,vdef);
        return vmin[_port];
}


float LV2Manager::getUpperBound(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return 0.f;

        //TODO caching
        uint32_t nbp=getPortCount(_key);
        float vmin[nbp];
        float vmax[nbp];
        float vdef[nbp];
        for(int i=0;i<nbp;i++)
                vmin[i]=vdef[i]=vmax[i]=0.f;

        Lilv::Plugin p=LV2Manager::plugin(_key);
        p.get_port_ranges_float(vmin,vmax,vdef);
        return vmax[_port];
}


float LV2Manager::getDefaultSetting(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return 0.f;

        //TODO caching
        uint32_t nbp=getPortCount(_key);
        float vmin[nbp];
        float vmax[nbp];
        float vdef[nbp];
        for(int i=0;i<nbp;i++)
                vmin[i]=vdef[i]=vmax[i]=0.f;

        Lilv::Plugin p=LV2Manager::plugin(_key);
        p.get_port_ranges_float(vmin,vmax,vdef);
        return vdef[_port];
}


bool LV2Manager::isPortToggled(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return false;
        Lilv::Plugin p=LV2Manager::plugin(_key);
        bool r=p.get_port_by_index(_port).has_property(PP_TOGGLED);
        if(r) qInfo("port #%d is toggled",_port);
        return r;
}


bool LV2Manager::isLogarithmic( const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return false;
        Lilv::Plugin p=LV2Manager::plugin(_key);
        return p.get_port_by_index(_port).has_property(PP_LOGARITHM);
}


bool LV2Manager::isInteger(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return false;
        Lilv::Plugin p=LV2Manager::plugin(_key);
        bool r=p.get_port_by_index(_port).has_property(PP_INTEGER);
        if(r) qInfo("port #%d is integer",_port);
        return r;
}


QString LV2Manager::getPortName(const lv2_key_t& _key, uint32_t _port)
{
        if(!m_keys.contains(_key)) return "";
        Lilv::Plugin p=LV2Manager::plugin(_key);
        Lilv::Node   n=p.get_port_by_index(_port).get_name();
        return n.as_string();
}


const void* LV2Manager::getImplementationData(const lv2_key_t& _key)
{
        //TODO
        return NULL;
}


/*
const LV2_Descriptor* LV2Manager::getDescriptor(const lv2_key_t& _key)
{
	if( m_lv2ManagerMap.contains( _key ) )
	{
		LV2_Descriptor_Function descriptorFunction =
			m_lv2ManagerMap[_key]->descriptorFunction;
		const LV2_Descriptor * descriptor =
				descriptorFunction(
					m_lv2ManagerMap[_key]->index );
		return( descriptor );
	}

        //TODO
        return NULL;
}
*/


LV2_Instance LV2Manager::instantiate(const lv2_key_t& _key, uint32_t _sampleRate)
{
        Lilv::Plugin p=LV2Manager::plugin(_key);
        return Lilv::Instance::create(p, (double)_sampleRate, NULL);
}




bool LV2Manager::connectPort(const lv2_key_t& _key,
                             LV2_Instance _instance,
                             uint32_t _port,
                             LV2_Data* _dataLocation)
{
        if(!m_keys.contains(_key)) return false;

        if(!_instance)
        {
                qWarning("LV2Manager::connectPort null instance");
                return false;
        }
        if(!_dataLocation)
        {
                qWarning("LV2Manager::connectPort null data location");
                return false;
        }

        _instance->connect_port(_port,_dataLocation);
        return true;
}


bool LV2Manager::activate(const lv2_key_t& _key,
                          LV2_Instance _instance)
{
        if(!m_keys.contains(_key)) return false;

        if(!_instance)
        {
                qWarning("LV2Manager::activate null instance");
                return false;
        }

        _instance->activate();
        return true;
}


bool LV2Manager::run(const lv2_key_t& _key,
                     LV2_Instance _instance,
                     uint32_t _sampleCount)
{
        if(!m_keys.contains(_key)) return false;

        if(!_instance)
        {
                qWarning("LV2Manager::run null instance");
                return false;
        }

        _instance->run(_sampleCount);
        return true;
}


/*
bool LV2Manager::runAdding( const lv2_key_t & _key,
						LV2_Instance _instance,
						uint32_t _sample_count )
{
	if( m_lv2ManagerMap.contains( _plugin ) )
	{
		LV2_Descriptor_Function descriptorFunction =
			m_lv2ManagerMap[_plugin]->descriptorFunction;
		const LV2_Descriptor * descriptor =
				descriptorFunction(
					m_lv2ManagerMap[_plugin]->index );
		if( descriptor->run_adding != NULL &&
			  	descriptor->set_run_adding_gain != NULL )
		{
			( descriptor->run_adding ) ( _instance, _sample_count );
			return( true );
		}
	}
	return( false );
}




bool LV2Manager::setRunAddingGain( const lv2_key_t & _plugin,
						LV2_Instance _instance,
						LV2_Data _gain )
{
	if( m_lv2ManagerMap.contains( _plugin ) )
	{
		LV2_Descriptor_Function descriptorFunction =
			m_lv2ManagerMap[_plugin]->descriptorFunction;
		const LV2_Descriptor * descriptor =
				descriptorFunction(
					m_lv2ManagerMap[_plugin]->index );
		if( descriptor->run_adding != NULL &&
				  descriptor->set_run_adding_gain != NULL )
		{
			( descriptor->set_run_adding_gain )
							( _instance, _gain );
			return( true );
		}
	}
	return( false );
}
*/


bool LV2Manager::deactivate(const lv2_key_t& _key, LV2_Instance _instance)
{
        if(!m_keys.contains(_key)) return false;

        if(!_instance)
        {
                qWarning("LV2Manager::deactivate null instance");
                return false;
        }

        _instance->deactivate();
        return true;
}


bool LV2Manager::cleanup(const lv2_key_t& _key, LV2_Instance _instance)
{
        if(!m_keys.contains(_key)) return false;

        if(!_instance)
        {
                qWarning("LV2Manager::cleanup null instance");
                return false;
        }

        //_instance->cleanup();
        return true;
}
