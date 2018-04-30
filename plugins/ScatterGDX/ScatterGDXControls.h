/*
 * ScatterGDXControls.h - controls for scatter remover effect
 *
 * Copyright (c) 2017
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

#ifndef SCATTERGDX_CONTROLS_H
#define SCATTERGDX_CONTROLS_H

#include "EffectControls.h"
#include "ScatterGDXControlDialog.h"
#include "Knob.h"


class ScatterGDXEffect;


class ScatterGDXControls : public EffectControls
{
	Q_OBJECT
public:
	ScatterGDXControls( ScatterGDXEffect* effect );
	virtual ~ScatterGDXControls()
	{
	}

	virtual void saveSettings( QDomDocument & _doc, QDomElement & _parent );
	virtual void loadSettings( const QDomElement & _this );
	inline virtual QString nodeName() const
	{
		return "ScatterGDXControls";
	}

	virtual int controlCount()
	{
		return 4;
	}

	virtual EffectControlDialog* createView()
	{
		return new ScatterGDXControlDialog( this );
	}


private slots:
	void changeControl();

private:
	ScatterGDXEffect* m_effect;

	FloatModel m_pwrModel; //power
        FloatModel m_spdModel; //speed
        FloatModel m_frcModel; //fraction
        FloatModel m_ovrModel; //override
        FloatModel m_strModel; //start

	friend class ScatterGDXControlDialog;
	friend class ScatterGDXEffect;

} ;

#endif
