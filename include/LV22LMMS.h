/*
 * LV22LMMS.h - class that identifies and instantiates LV2 effects
 *                   for use with LMMS
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

#ifndef LV2_2_LMMS_H
#define LV2_2_LMMS_H

#ifdef WANT_LV2


#include "LV2Manager.h"


class EXPORT LV22LMMS : public LV2Manager
{
 public:
	inline l_lv2_sortable_plugin_t getInstruments()
	{
		return m_instruments;
	}


	inline l_lv2_sortable_plugin_t getValidEffects()
	{
		return m_effects;
	}

	inline l_lv2_sortable_plugin_t getInvalidEffects()
	{
		return m_invalids;
	}

	inline l_lv2_sortable_plugin_t getAnalysisTools()
	{
		return m_tools;
	}

	inline l_lv2_sortable_plugin_t getOthers()
	{
		return m_others;
	}

	QString getShortName( const lv2_key_t & _key );

private:
	LV22LMMS();
	virtual ~LV22LMMS();

        /*
	l_lv2_sortable_plugin_t m_instruments;
	l_lv2_sortable_plugin_t m_effects;//validEffects;
	//l_lv2_sortable_plugin_t m_invalidEffects;
	l_lv2_sortable_plugin_t m_tools;//m_analysisTools;
	l_lv2_sortable_plugin_t m_others;//m_otherPlugins;
        */

	friend class LmmsCore;

} ;

#endif
#endif
