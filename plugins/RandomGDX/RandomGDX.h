/*
 * RandomGDX.h - click remover effect
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


#ifndef RANDOMGDX_H
#define RANDOMGDX_H

#include "lmms_math.h"
#include "Effect.h"
#include "RandomGDXControls.h"
#include "ValueBuffer.h"

class RandomGDXEffect : public Effect
{
 public:
	RandomGDXEffect( Model* parent, const Descriptor::SubPluginFeatures::Key* key );
	virtual ~RandomGDXEffect();
	virtual bool processAudioBuffer( sampleFrame* buf, const fpp_t frames );

	virtual EffectControls* controls()
	{
		return &m_gdxControls;
	}

 private:
	RandomGDXControls m_gdxControls;
        float             m_fact0,m_fact1;
        float             m_sact0,m_sact1;

	friend class RandomGDXControls;

} ;

#endif
