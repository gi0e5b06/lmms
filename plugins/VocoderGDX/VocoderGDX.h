/*
 * VocoderGDX.h -
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

#ifndef VOCODERGDX_H
#define VOCODERGDX_H

#include "Effect.h"
#include "VocoderGDXControls.h"
#include "ValueBuffer.h"
//#include "fft_helpers.h"
#include "lmms_math.h"

#include <fftw3.h>

class PLUGIN_EXPORT VocoderGDX : public Effect
{
  public:
    enum ChannelModes
    {
        MergeChannels,
        LeftChannel,
        RightChannel
    };

    VocoderGDX(Model*                                    parent,
                 const Descriptor::SubPluginFeatures::Key* key);
    virtual ~VocoderGDX();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_gdxControls;
    }

  private:
    VocoderGDXControls m_gdxControls;

    fftwf_plan m_r2cPlan0;
    fftwf_plan m_r2cPlan1;
    fftwf_plan m_c2rPlan;

    fftwf_complex* m_cBuf0;
    fftwf_complex* m_cBuf1;
    float*         m_oBuf;
    float*         m_rBuf0;
    float*         m_rBuf1;

    int m_size;
    int m_filled;

    friend class VocoderGDXControls;
};

#endif
