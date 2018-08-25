/*
 * FrequencyGDX.h -
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

#ifndef FREQUENCYGDX_H
#define FREQUENCYGDX_H

#include "Effect.h"
#include "FrequencyGDXControls.h"
#include "ValueBuffer.h"
#include "fft_helpers.h"
#include "lmms_math.h"

#define FFT_BUFFER_SIZE 4096
// 22050

class PLUGIN_EXPORT FrequencyGDX : public Effect
{
  public:
    enum ChannelModes
    {
        MergeChannels,
        LeftChannel,
        RightChannel
    };

    static const int MAX_BANDS = 22050;

    FrequencyGDX(Model*                                    parent,
                 const Descriptor::SubPluginFeatures::Key* key);
    virtual ~FrequencyGDX();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_gdxControls;
    }

  private:
    FrequencyGDXControls m_gdxControls;

    fftwf_plan m_fftPlan;

    fftwf_complex* m_specBuf;
    float          m_absSpecBuf[FFT_BUFFER_SIZE + 1];
    float          m_buffer[FFT_BUFFER_SIZE * 2];
    int            m_framesFilledUp;

    int   m_ak;
    float m_energy;

    friend class FrequencyGDXControls;
};

#endif
