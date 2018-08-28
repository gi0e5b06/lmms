/*
 * ScarifierGDX.h -
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

#ifndef SCARIFIERGDX_H
#define SCARIFIERGDX_H

#include "Effect.h"
#include "ScarifierGDXControls.h"
#include "ValueBuffer.h"
//#include "fft_helpers.h"
#include "lmms_math.h"

#include <fftw3.h>

class PLUGIN_EXPORT ScarifierGDX : public Effect
{
  public:
    enum ChannelModes
    {
        MergeChannels,
        LeftChannel,
        RightChannel
    };

    ScarifierGDX(Model*                                    parent,
                 const Descriptor::SubPluginFeatures::Key* key);
    virtual ~ScarifierGDX();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_gdxControls;
    }

  private:
    ScarifierGDXControls m_gdxControls;

    fftwf_plan m_r2cPlan;
    fftwf_plan m_c2rPlan;

    fftwf_complex* m_cBuf;
    float*         m_rBuf;

    int m_size;
    int m_filled;

    friend class ScarifierGDXControls;
};

#endif
