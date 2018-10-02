/*
 * Bitcrush.h - A native bitcrusher
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef BITCRUSH_H
#define BITCRUSH_H

#include "BasicFilters.h"
#include "BitcrushControls.h"
#include "Effect.h"
#include "ValueBuffer.h"
#include "lmms_math.h"

class BitcrushEffect : public Effect
{
  public:
    BitcrushEffect(Model*                                    parent,
                   const Descriptor::SubPluginFeatures::Key* key);
    virtual ~BitcrushEffect();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_controls;
    }

  private:
    void   sampleRateChanged();
    real_t depthCrush(real_t in);
    real_t noise(real_t amt);

    BitcrushControls m_controls;

    sampleFrame*        m_buffer;
    real_t              m_sampleRate;
    StereoLinkwitzRiley m_filter;

    real_t m_bitCounterL;
    real_t m_rateCoeffL;
    real_t m_bitCounterR;
    real_t m_rateCoeffR;
    bool   m_rateEnabled;

    real_t m_left;
    real_t m_right;

    int    m_levels;
    real_t m_levelsRatio;
    bool   m_depthEnabled;

    real_t m_inGain;
    real_t m_outGain;
    real_t m_outClip;

    bool m_needsUpdate;

    int m_silenceCounter;

    friend class BitcrushControls;
};

#endif
