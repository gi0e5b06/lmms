/*
 * LV2Base.h - basic declarations concerning LV2
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

#ifndef LV2_BASE_H
#define LV2_BASE_H

#include "lmmsconfig.h"

#ifdef LMMS_HAVE_LILV

#include "LV2Manager.h"
#include "Plugin.h"

class LV2Control;


typedef enum BufferRates
{
	CHANNEL_IN,
	CHANNEL_OUT,
	AUDIO_RATE_INPUT,
	AUDIO_RATE_OUTPUT,
	CONTROL_RATE_INPUT,
	CONTROL_RATE_OUTPUT,
	MIDI_RATE_INPUT,
	MIDI_RATE_OUTPUT,
        IGNORED
} buffer_rate_t;

typedef enum BufferData
{
	TOGGLED,
	INTEGER,
	FLOATING,
	TIME,
	NONE
} buffer_data_t;

//! This struct is used to hold port descriptions internally
//! which where received from the lv2 plugin
typedef struct LV2PortDescription
{
	QString       name;
	ch_cnt_t      proc;
	uint16_t      port_id;
	uint16_t      control_id;
	buffer_rate_t rate;
	buffer_data_t data_type;
	float         scale;
	LV2_Data      max;
	LV2_Data      min;
	LV2_Data      def;
	LV2_Data      value;
	//! This is true iff lv2 suggests logscale
	//! Note however that the model can still decide to use a linear scale
	bool          suggests_logscale;
	LV2_Data*     buffer;
	LV2Control*   control;
} lv2_port_desc_t;


inline Plugin::Descriptor::SubPluginFeatures::Key lv2KeyToSubPluginKey
  (const Plugin::Descriptor * _desc,
   const QString & _name,
   const lv2_key_t& _key)
{
	Plugin::Descriptor::SubPluginFeatures::Key::AttributeMap m;
	QString uri=_key;
	m["uri"]=uri;
	//m["plugin"]=key.second;
	return Plugin::Descriptor::SubPluginFeatures::Key( _desc, _name, m );
}


#endif
#endif
