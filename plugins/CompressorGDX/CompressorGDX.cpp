/*
 * CompressorGDX.cpp -
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

#include "CompressorGDX.h"

#include "WaveForm.h"
#include "embed.h"
#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT compressorgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "CompressorGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A compressor plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

CompressorGDX::CompressorGDX(Model*                                    parent,
                             const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&compressorgdx_plugin_descriptor, parent, key),
      m_gdxControls(this)  //, m_fact0(0.0f), m_sact0(0.0f)
{
}

CompressorGDX::~CompressorGDX()
{
}

bool CompressorGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    const ValueBuffer* thresholdBuf
            = m_gdxControls.m_thresholdModel.valueBuffer();
    const ValueBuffer* ratioBuf = m_gdxControls.m_ratioModel.valueBuffer();
    const ValueBuffer* outGainBuf
            = m_gdxControls.m_outGainModel.valueBuffer();
    const ValueBuffer* modeBuf = m_gdxControls.m_modeModel.valueBuffer();

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        real_t threshold = thresholdBuf
                                   ? thresholdBuf->value(f)
                                   : m_gdxControls.m_thresholdModel.value();
        real_t ratio = ratioBuf ? ratioBuf->value(f)
                                : m_gdxControls.m_ratioModel.value();
        real_t outGain = outGainBuf ? outGainBuf->value(f)
                                    : m_gdxControls.m_outGainModel.value();
        int mode = (int)(modeBuf ? modeBuf->value(f)
                                 : m_gdxControls.m_modeModel.value());

        sample_t curVal0 = _buf[f][0];
        sample_t curVal1 = _buf[f][1];

        if(threshold > 0. || ratio > 0.)
            outGain /= (threshold + ratio * (1. - threshold));

        {
            real_t y0 = bound(0., abs(curVal0) - threshold, 1.);
            if(y0 > 0.)
            {
                real_t s0 = sign(curVal0);
                switch(mode)
                {
                    case 0:
                        // clip
                        if(y0 > ratio)
                            y0 = ratio;
                        break;
                    case 2:
                        y0 *= (2. - y0) * ratio;
                        break;
                    case 3:
                        y0 *= WaveForm::sqrt(y0) * ratio;
                        break;
                    case 4:
                        y0 *= WaveForm::sine(y0 / 4.) * ratio;
                        break;
                    case 5:
                        y0 *= (1. + erf(2. * y0) / 2.) * ratio;
                        break;
                    case 1:
                    default:
                        y0 *= ratio;
                        break;
                }
                curVal0 = s0 * (threshold + y0) * outGain;
                // curVal0 = qBound(-1., curVal0, 1.);
            }
            else
            {
                curVal0 *= outGain;
                // curVal0 = qBound(-1., curVal0, 1.);
            }
        }

        {
            real_t y1 = bound(0., abs(curVal1) - threshold, 1.);
            if(y1 > 0.)
            {
                real_t s1 = sign(curVal1);
                switch(mode)
                {
                    case 0:
                        // clip
                        if(y1 > ratio)
                            y1 = ratio;
                        break;
                    case 2:
                        y1 *= (2 - y1) * ratio;
                        break;
                    case 3:
                        y1 *= WaveForm::sqrt(y1) * ratio;
                        break;
                    case 4:
                        y1 *= WaveForm::sine(y1 / 4.f) * ratio;
                        break;
                    case 5:
                        y1 *= (1.f + erf(2.f * y1) / 2.f) * ratio;
                        break;
                    case 1:
                    default:
                        y1 *= ratio;
                        break;
                }
                curVal1 = s1 * (threshold + y1) * outGain;
                // curVal1 = qBound(-1.f, curVal1, 1.f);
            }
            else
            {
                curVal1 *= outGain;
                // curVal1 = qBound(-1.f, curVal1, 1.f);
            }
        }

        curVal0 = bound(-1., curVal0, 1.);
        curVal1 = bound(-1., curVal1, 1.);

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
    }

    return shouldKeepRunning(_buf, _frames);
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new CompressorGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
