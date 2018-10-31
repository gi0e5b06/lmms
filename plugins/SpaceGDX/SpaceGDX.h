/*
 * SpaceGDX.h - wall effect
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

#ifndef SPACEGDX_H
#define SPACEGDX_H

#include "BasicFilters.h"
#include "Effect.h"
#include "Ring.h"
#include "SpaceGDXControls.h"
#include "lmms_math.h"

class SpaceGDXEffect : public Effect
{
  public:
    SpaceGDXEffect(Model*                                    parent,
                   const Descriptor::SubPluginFeatures::Key* key);
    virtual ~SpaceGDXEffect();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_gdxControls;
    }

  private:
    SpaceGDXControls m_gdxControls;
    sample_rate_t    m_sampleRate;
    Ring             m_ring;
    BasicFilters<1>  m_leftLowFilter, m_rightLowFilter;
    BasicFilters<1>  m_leftHighFilter, m_rightHighFilter;
    frequency_t      m_prevLeftLowFreq, m_prevRightLowFreq;
    frequency_t      m_prevLeftHighFreq, m_prevRightHighFreq;

    friend class SpaceGDXControls;
};

#endif
