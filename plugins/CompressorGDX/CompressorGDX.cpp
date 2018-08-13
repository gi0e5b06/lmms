/*
 * CompressorGDX.cpp - A click remover
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

#include <math.h>

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
      m_gdxControls(this)//, m_fact0(0.0f), m_sact0(0.0f)
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
        float w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        float threshold
                = (float)(thresholdBuf
                                  ? thresholdBuf->value(f)
                                  : m_gdxControls.m_thresholdModel.value());
        float ratio = (float)(ratioBuf ? ratioBuf->value(f)
                                       : m_gdxControls.m_ratioModel.value());
        float outGain
                = (float)(outGainBuf ? outGainBuf->value(f)
                                     : m_gdxControls.m_outGainModel.value());
        float mode = (float)(modeBuf ? modeBuf->value(f)
                                     : m_gdxControls.m_modeModel.value());

        Q_UNUSED(mode);

        float curVal0 = _buf[f][0];
        float curVal1 = _buf[f][1];

        float exp = (1.f - threshold) * (1.f - ratio);
        outGain *= (1.f + exp);

        {
            if(fabs(curVal0) >= threshold)
                curVal0 = sign(curVal0)
                          * (threshold + (fabs(curVal0) - threshold) * ratio);
            curVal0 = qBound(0.f, curVal0 * outGain, 1.f);
        }

        {
            if(fabs(curVal1) >= threshold)
                curVal1 = sign(curVal1)
                          * (threshold + (fabs(curVal1) - threshold) * ratio);
            curVal1 = qBound(0.f, curVal1 * outGain, 1.f);
        }

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
