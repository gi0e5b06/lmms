/*
 * ShifterGDX.h -
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef SHIFTERGDX_H
#define SHIFTERGDX_H

#include "Effect.h"
#include "ShifterGDXControls.h"
#include "ValueBuffer.h"
//#include "fft_helpers.h"
#include "lmms_math.h"

#include <complex>
#include <fftw3.h>

class PLUGIN_EXPORT ShifterGDX : public Effect
{
  public:
    enum ChannelModes
    {
        MergeChannels,
        LeftChannel,
        RightChannel
    };

    ShifterGDX(Model*                                    parent,
                 const Descriptor::SubPluginFeatures::Key* key);
    virtual ~ShifterGDX();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_gdxControls;
    }

  private:
    ShifterGDXControls m_gdxControls;

    fftwf_plan m_r2cPlan0;
    fftwf_plan m_r2cPlan1;
    fftwf_plan m_c2rPlan0;
    fftwf_plan m_c2rPlan1;

    fftwf_complex* m_cBuf0;
    fftwf_complex* m_cBuf1;
    FLOAT*         m_oBuf0;
    FLOAT*         m_oBuf1;
    FLOAT*         m_rBuf0;
    FLOAT*         m_rBuf1;

    int m_size;
    int m_filled;

    friend class ShifterGDXControls;
};

#endif
