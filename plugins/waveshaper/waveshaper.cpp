/*
 * waveshaper.cpp - waveshaper effect-plugin
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "waveshaper.h"

#include "embed.h"
#include "interpolation.h"
#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT waveshaper_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "Waveshaper Effect",
               QT_TRANSLATE_NOOP("pluginBrowser", "plugin for waveshaping"),
               "Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

waveShaperEffect::waveShaperEffect(
        Model* _parent, const Descriptor::SubPluginFeatures::Key* _key) :
      Effect(&waveshaper_plugin_descriptor, _parent, _key),
      m_wsControls(this)
{
}

waveShaperEffect::~waveShaperEffect()
{
}

bool waveShaperEffect::processAudioBuffer(sampleFrame* _buf,
                                          const fpp_t  _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    real_t       input   = m_wsControls.m_inputModel.value();
    real_t       output  = m_wsControls.m_outputModel.value();
    const float* samples = m_wsControls.m_waveGraphModel.samples();
    const bool   clip    = m_wsControls.m_clipModel.value();

    ValueBuffer* inputBuffer = m_wsControls.m_inputModel.valueBuffer();
    ValueBuffer* outputBufer = m_wsControls.m_outputModel.valueBuffer();

    int inputInc  = inputBuffer ? 1 : 0;
    int outputInc = outputBufer ? 1 : 0;

    const real_t* inputPtr
            = inputBuffer ? &(inputBuffer->values()[0]) : &input;
    const real_t* outputPtr
            = outputBufer ? &(outputBufer->values()[0]) : &output;

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        sample_t s[2] = {_buf[f][0], _buf[f][1]};

        for(int i = 0; i < 2; ++i)
        {
            // apply input gain
            s[i] *= *inputPtr;

            // clip if clip enabled
            if(clip)
                s[i] = qBound<sample_t>(-1., s[i], 1.);

            // start effect
            const int    lookup = static_cast<int>(qAbs(s[i]) * 200.);
            const real_t frac   = fraction(qAbs(s[i]) * 200.);
            const real_t posneg = s[i] < 0. ? -1. : 1.;

            if(lookup < 1)
            {
                s[i] = frac * samples[0] * posneg;
            }
            else if(lookup < 200)
            {
                s[i] = linearInterpolate(real_t(samples[lookup - 1]),
                                         real_t(samples[lookup]), frac)
                       * posneg;
            }
            else
            {
                s[i] *= samples[199];
            }

            // apply output gain
            s[i] *= *outputPtr;
        }

        // mix wet/dry signals
        _buf[f][0] = d0 * _buf[f][0] + w0 * s[0];
        _buf[f][1] = d1 * _buf[f][1] + w1 * s[1];

        outputPtr += outputInc;
        inputPtr += inputInc;
    }

    return true;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* _parent, void* _data)
    {
        return (new waveShaperEffect(
                _parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        _data)));
    }
}
