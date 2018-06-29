/*
 * LV2EffectGDXControls.h - model for LV2 effect controls
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

#ifndef LV2_EFFECT_GDX_CONTROLS_H
#define LV2_EFFECT_GDX_CONTROLS_H

#include "EffectControls.h"
#include "LV2EffectGDXControls.h"
#include "LV2EffectGDXControlDialog.h"

class LV2Control;
class LV2EffectGDX;


typedef QVector<LV2Control*> lv2_control_list_t;


class LV2EffectGDXControls : public EffectControls
{
	Q_OBJECT
public:
	LV2EffectGDXControls( LV2EffectGDX* _effect );
	virtual ~LV2EffectGDXControls();

	inline int controlCount()
	{
		return m_controlCount;
	}

	virtual void saveSettings( QDomDocument & _doc, QDomElement & _parent );
	virtual void loadSettings( const QDomElement & _this );

	inline virtual QString nodeName() const
	{
		return "lv2effectgdxcontrols";
	}

	virtual EffectControlDialog* createView()
	{
		return new LV2EffectGDXControlDialog(this);
	}


protected slots:
	void updateLinkStatesFromGlobal();
	void linkPort( int _port, bool _state );


private:
	LV2EffectGDX* m_effect;
	ch_cnt_t m_processors;
	ch_cnt_t m_controlCount;
	bool m_noLink;
	BoolModel m_stereoLinkModel;
	QVector<lv2_control_list_t> m_controls;

	friend class LV2EffectGDXControlDialog;
	friend class LV2EffectGDX;

signals:
	void effectModelChanged(LV2EffectGDXControls* _controls);

} ;

#endif
