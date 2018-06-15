/*
 * LV2Manager.h - declaration of class lv2Manager
 *                    a class to manage loading and instantiation
 *                    of lv2 plugins
 *
 * Copyright (c) 2005-2008 Danny McRae <khjklujn@netscape.net>
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


#ifndef LV2_MANAGER_H
#define LV2_MANAGER_H

//#include <lv2.h>

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QString>
#include <QtCore/QStringList>

//#include <lilv/lilv.h>
#include <lilv/lilvmm.hpp>
#include "lv2/lv2plug.in/ns/ext/event/event.h"
#include "lv2/lv2plug.in/ns/ext/uri-map/uri-map.h"
#include "lv2/lv2plug.in/ns/ext/event/event-helpers.h"

#include "export.h"
#include "lmms_basics.h"
#include "Plugin.h"

//const float NOHINT = -99342.2243f;

typedef QString lv2_key_t;
typedef Lilv::Instance* LV2_Instance;
typedef float LV2_Data;
typedef QVector<lv2_key_t> l_lv2_sortable_plugin_t;

//typedef QPair<QString, lv2_key_t> lv2_sortable_plugin_t;
//typedef QList<lv2_sortable_plugin_t> l_lv2_sortable_plugin_t;
//typedef QList<lv2_key_t> l_lv2_key_t;


/* lv2Manager provides a database of LV2 plug-ins.  Upon instantiation,
it loads all of the plug-ins found in the LV2_PATH environmental variable
and stores their access descriptors according in a dictionary keyed on
the filename the plug-in was loaded from and the label of the plug-in.

The can be retrieved by using lv2_key_t.  For example, to get the
"Phase Modulated Voice" plug-in from the cmt library, you would perform the
calls using:

	lv2_key_t key( "cmt.so", "phasemod" )

as the plug-in key. */

/*
enum lv2PluginType
{
	SOURCE,
	TRANSFER,
	VALID,
	INVALID,
	SINK,
	OTHER
};

typedef struct lv2ManagerStorage
{
	LV2_Descriptor_Function descriptorFunction;
	uint32_t index;
	lv2PluginType type;
	uint16_t inputChannels;
	uint16_t outputChannels;
} lv2ManagerDescription;
*/


class EXPORT LV2Manager
{
 public:

	LV2Manager();
	virtual ~LV2Manager();

	//l_lv2_sortable_plugin_t getSortedPlugins();
	//lv2ManagerDescription* getDescription(const lv2_key_t& _plugin );

	/* This identifier can be used as a unique, case-sensitive
	identifier for the plugin type within the plugin file. Plugin
	types should be identified by file and label rather than by index
	or plugin name, which may be changed in new plugin
	versions. Labels must not contain white-space characters. */
	QString getLabel(const lv2_key_t& _key);

	/* Indicates that the plugin has a real-time dependency
	(e.g. listens to a MIDI device) and so its output must not
	be cached or subject to significant latency. */
	//bool  hasRealTimeDependency( const lv2_key_t& _key );

	/* Indicates that the plugin may cease to work correctly if the
	host elects to use the same data location for both input and output
	(see connectPort). */
	bool isInplaceBroken(const lv2_key_t& _key);

	/* Indicates that the plugin is capable of running not only in a
	conventional host but also in a 'hard real-time' environment. */
	//bool  isRealTimeCapable( const lv2_key_t& _key );

	/* Returns the name of the plug-in */
	QString getName(const lv2_key_t& _key);

	/* Returns the the plug-in's author */
	QString  getMaker( const lv2_key_t& _key);

	/* Returns the copyright for the plug-in */
	QString  getCopyright(const lv2_key_t& _key);

  	/* This indicates the number of ports (input AND output) present on
	the plugin. */
	uint32_t  getPortCount(const lv2_key_t& _key);

	/* Indicates that the port is an input. */
	bool isPortInput(const lv2_key_t& _key, uint32_t _port);

	/* Indicates that the port is an output. */
	bool isPortOutput(const lv2_key_t& _key, uint32_t _port);

	/* Indicates that the port is an audio. */
	bool isPortAudio(const lv2_key_t& _key, uint32_t _port);

	/* Indicates that the port is an control. */
	bool isPortControl(const lv2_key_t& _key, uint32_t _port);

	/* Indicates that any bounds specified should be interpreted as
	multiples of the sample rate. For instance, a frequency range from
	0Hz to the Nyquist frequency (half the sample rate) could be requested
	by this hint in conjunction with LowerBound = 0 and UpperBound = 0.5.
	Hosts that support bounds at all must support this hint to retain
	meaning. */
	//bool  areHintsSampleRateDependent( const lv2_key_t& _key,
        //                                   uint32_t _port );

  	/* Returns the lower boundary value for the given port. If
	no lower bound is provided by the plug-in, returns -999e-99. When
	areHintsSampleRateDependent() is also true then this value should be
	multiplied by the relevant sample rate. */
	float getLowerBound(const lv2_key_t& _key, uint32_t _port);

  	/* Returns the upper boundary value for the given port. If
	no upper bound is provided by the plug-in, returns -999e-99. When
	areHintsSampleRateDependent() is also true then this value should be
	multiplied by the relevant sample rate. */
	float getUpperBound(const lv2_key_t& _key, uint32_t _port);

	/* Indicates whether the given port should be considered 0 or 1
	boolean switch. */
	bool isPortToggled(const lv2_key_t& _key, uint32_t _port);

	/* Retrieves any default setting hints offered by the plug-in for
	the given port. */
	float getDefaultSetting(const lv2_key_t& _key, uint32_t _port);

	/* Indicates that it is likely that the user will find it more
	intuitive to view values using a logarithmic scale. This is
	particularly useful for frequencies and gains. */
	bool isLogarithmic(const lv2_key_t& _key, uint32_t _port);

	/* Indicates that a user interface would probably wish to provide a
	stepped control taking only integer values. Any bounds set should be
	slightly wider than the actual integer range required to avoid floating
	point rounding errors. For instance, the integer set {0,1,2,3} might
	be described as [-0.1, 3.1]. */
	bool isInteger(const lv2_key_t& _key, uint32_t _port);

	/* Returns the name of the port. */
	QString  getPortName(const lv2_key_t& _key, uint32_t _port);


	/* This may be used by the plugin developer to pass any custom
	implementation data into an instantiate call. It must not be used
	or interpreted by the host. It is expected that most plugin
	writers will not use this facility as LV2_Instance should be
	used to hold instance data. */
	const void* getImplementationData(const lv2_key_t& _key);


	/* Returns a pointer to the plug-in's descriptor from which control
	of the plug-in is accessible */
	//const LV2_Descriptor* getDescriptor(const lv2_key_t& _key);


	/* The following methods are convenience functions for use during
	development.  A real instrument should use the getDescriptor()
	method and implement the plug-in manipulations internally to avoid
	the overhead associated with QMap lookups. */


	/* Returns a handle to an instantiation of the given plug-in. */
	LV2_Instance instantiate(const lv2_key_t& _key, uint32_t _sampleRate);

  	/* This method calls a function pointer that connects a port on an
	instantiated plugin to a memory location at which a block of data
	for the port will be read/written. The data location is expected
	to be an array of LV2_Data for audio ports or a single
	LV2_Data value for control ports. Memory issues will be
	managed by the host. The plugin must read/write the data at these
	locations every time run() or runAdding() is called and the data
	present at the time of this connection call should not be
	considered meaningful.

	connectPort() may be called more than once for a plugin instance
	to allow the host to change the buffers that the plugin is
	reading or writing. These calls may be made before or after
	activate() or deactivate() calls.

	connectPort() must be called at least once for each port before
	run() or runAdding() is called. */
	bool connectPort(const lv2_key_t& _key,
                         LV2_Instance _instance,
                         uint32_t _port,
                         LV2_Data* _dataLocation);

  	/* This method calls a function pointer that initialises a plugin
	instance and activates it for use. This is separated from
	instantiate() to aid real-time support and so that hosts can
	reinitialise a plugin instance by calling deactivate() and then
	activate(). In this case the plugin instance must reset all state
	information dependent on the history of the plugin instance
	except for any data locations provided by connectPort() and any
	gain set by setRunAddingGain(). If there is nothing for
	activate() to do then the plugin writer may provide a NULL rather
	than an empty function.

	When present, hosts must call this function once before run() (or
	runAdding()) is called for the first time. This call should be
	made as close to the run() call as possible and indicates to
	real-time plugins that they are now live. Plugins should not rely
	on a prompt call to run() after activate(). activate() may not be
	called again unless deactivate() is called first. Note that
	connectPort() may be called before or after a call to
	activate(). */
	bool activate(const lv2_key_t& _key, LV2_Instance _instance);

	/* This method calls a function pointer that runs an instance of a
	plugin for a block. Two parameters are required: the first is a
	handle to the particular instance to be run and the second
	indicates the block size (in samples) for which the plugin
	instance may run.

	Note that if an activate() function exists then it must be called
	before run() or run_adding(). If deactivate() is called for a
	plugin instance then the plugin instance may not be reused until
	activate() has been called again. */
	bool run(const lv2_key_t& _key, LV2_Instance _instance, uint32_t _sampleCount);

	/* This method calls a function pointer that runs an instance of a
	plugin for a block. This has identical behaviour to run() except
	in the way data is output from the plugin. When run() is used,
	values are written directly to the memory areas associated with
	the output ports. However when runAdding() is called, values
	must be added to the values already present in the memory
	areas. Furthermore, output values written must be scaled by the
	current gain set by setRunAddingGain() (see below) before
	addition.

	runAdding() is optional. When it is not provided by a plugin,
	this function pointer must be set to NULL. When it is provided,
	the function setRunAddingGain() must be provided also. */
	//bool runAdding(const lv2_key_t& _key,
        //               LV2_Instance _instance,
        //               uint32_t _sample_count);

  	/* This method calls a function pointer that sets the output gain for
	use when runAdding() is called (see above). If this function is
	never called the gain is assumed to default to 1. Gain
	information should be retained when activate() or deactivate()
	are called.

	This function should be provided by the plugin if and only if the
	runAdding() function is provided. When it is absent this
	function pointer must be set to NULL. */
	//bool setRunAddingGain(const lv2_key_t& _key,
        //                      LV2_Instance _instance,
        //                      LV2_Data _gain);

	/* This is the counterpart to activate() (see above). If there is
	nothing for deactivate() to do then the plugin writer may provide
	a NULL rather than an empty function.

	Hosts must deactivate all activated units after they have been
	run() (or run_adding()) for the last time. This call should be
	made as close to the last run() call as possible and indicates to
	real-time plugins that they are no longer live. Plugins should
	not rely on prompt deactivation. Note that connect_port() may be
	called before or after a call to deactivate().

	Deactivation is not similar to pausing as the plugin instance
	will be reinitialised when activate() is called to reuse it. */
	bool deactivate(const lv2_key_t& _key, LV2_Instance _instance);

	/* Once an instance of a plugin has been finished with it can be
	deleted using the following function. The instance handle passed
	ceases to be valid after this call.

	If activate() was called for a plugin instance then a
	corresponding call to deactivate() must be made before cleanup()
	is called. */
	bool cleanup(const lv2_key_t& _key, LV2_Instance _instance);

 protected:
        //l_lv2_sortable_plugin_t
        QVector<lv2_key_t> m_effects;
        QVector<lv2_key_t> m_instruments;
        QVector<lv2_key_t> m_invalids;
        QVector<lv2_key_t> m_others;
        QVector<lv2_key_t> m_tools;
        //QMap<QString,Plugin::Descriptor> m_descriptors;

 private:
        Lilv::World*    m_world;
        //QMap<lv2_key_t,Lilv::Plugin*> m_plugins;
        QVector<lv2_key_t> m_keys;

        //Lilv::Plugin*   plugin;
        //Lilv::Instance* instance;
        LilvNode* NP_AUDIO    ;
        LilvNode* NP_CONTROL  ;
        LilvNode* NP_INPUT    ;
        LilvNode* NP_MIDI     ;
        LilvNode* NP_OUTPUT   ;
        LilvNode* PP_INTEGER  ;
        LilvNode* PP_LOGARITHM;
        LilvNode* PP_TOGGLED  ;

        Lilv::Plugin plugin(const lv2_key_t& _key);
        void addPlugin(const lv2_key_t& _key);

        //void  addPlugins( LV2_Descriptor_Function _descriptor_func,
        //                  const QString & _file );
        //uint16_t  getPluginInputs( const LV2_Descriptor * _descriptor );
        //uint16_t  getPluginOutputs( const LV2_Descriptor * _descriptor );

	//typedef QMap<lv2_key_t, lv2ManagerDescription *>
	//					lv2ManagerMapType;
	//lv2ManagerMapType m_lv2ManagerMap;

	//l_lv2_sortable_plugin_t m_sortedPlugins;

} ;

#endif
