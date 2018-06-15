/*
 * LV2EffectGDX.h - class for handling LV2 effect plugins
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

#ifndef LV2_EFFECT_GDX_H
#define LV2_EFFECT_GDX_H

//#include <QMutex>

#include "Effect.h"
#include "LV2Control.h"
#include "LV2Manager.h"
#include "LV2EffectGDXControls.h"


typedef QVector<lv2_port_desc_t*> lv2_multi_proc_t;

class LV2EffectGDX : public Effect
{
	Q_OBJECT
 public:
	LV2EffectGDX( Model* _parent,
                      const Descriptor::SubPluginFeatures::Key* _key );
	virtual ~LV2EffectGDX();

        //virtual uint32_t portCount();

	virtual bool processAudioBuffer( sampleFrame* _buf,
					 const fpp_t _frames );

	void setControl( int _control, LV2_Data _data );

	virtual EffectControls* controls()
	{
		return m_controls;
	}

        inline const lv2_multi_proc_t& getPortControls()
	{
		return m_portControls;
	}

 private slots:
	void changeSampleRate();


 private:
	void pluginInstantiation();
	void pluginDestruction();

        uint32_t inputChannels();
        uint32_t outputChannels();

	//static sample_rate_t maxSamplerate( const QString& _name );


	QMutex m_pluginMutex;
	LV2EffectGDXControls* m_controls;

	//sample_rate_t m_maxSampleRate;
	lv2_key_t m_uri;
	int m_portCount;
	bool m_inPlaceBroken;

	const LV2_Descriptor* m_descriptor;
	QVector<LV2_Instance> m_handles;

	QVector<lv2_multi_proc_t> m_ports;
	lv2_multi_proc_t m_portControls;

} ;

#endif
