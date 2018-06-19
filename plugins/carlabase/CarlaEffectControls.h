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
#include "CarlaEffectControlDialog.h"
//#include "Knob.h"


class CarlaEffect;


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
		return 0;
	}

	virtual EffectControlDialog* createView();


private slots:
	void changeControl();

private:
	CarlaEffect* m_effect;

	friend class CarlaEffectControlDialog;
	friend class CarlaEffect;

} ;

#endif
