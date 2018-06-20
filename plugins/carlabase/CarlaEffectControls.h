/*
 * Carla for LMMS
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

#ifndef CARLA_EFFECT_CONTROLS_H
#define CARLA_EFFECT_CONTROLS_H

#include "EffectControls.h"
//#include "CarlaEffectControlDialog.h"
//#include "Knob.h"


class CarlaEffect;


#define NB_KNOBS      18
#define NB_LEDS        8
#define NB_LCDS        4
#define NB_KNOB_START 40
#define NB_LED_START  60
#define NB_LCD_START  70
#define MIDI_CH        1


class CarlaEffectControls : public EffectControls
{
	Q_OBJECT
public:
	CarlaEffectControls( CarlaEffect* effect );
	virtual ~CarlaEffectControls() { }

	virtual void saveSettings( QDomDocument & _doc, QDomElement & _parent );
	virtual void loadSettings( const QDomElement & _this );
	inline virtual QString nodeName() const
	{
		return "CarlaEffectControls";
	}

	virtual int controlCount()
	{
                return NB_KNOBS+NB_LEDS+NB_LCDS;
	}

	virtual EffectControlDialog* createView();

private slots:
	void changeControl();

private:
	CarlaEffect* m_effect;

        FloatModel*  m_knobs[NB_KNOBS];
        BoolModel*   m_leds [NB_LEDS ];
        IntModel*    m_lcds [NB_LCDS ];

	friend class CarlaEffectControlDialog;
	friend class CarlaEffect;

} ;

#endif
