/*
 * DistortorGDX.cpp -
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

#include "DistortorGDX.h"

#include "WaveForm.h"
#include "embed.h"
#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT distortorgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "DistortorGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A compressor plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

DistortorGDX::DistortorGDX(Model*                                    parent,
                           const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&distortorgdx_plugin_descriptor, parent, key),
      m_gdxControls(this)  //, m_fact0(0.), m_sact0(0.)
{
    setColor(QColor(160, 160, 74));
}

DistortorGDX::~DistortorGDX()
{
}

bool DistortorGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    const ValueBuffer* simpleBuf = m_gdxControls.m_simpleModel.valueBuffer();
    const ValueBuffer* foldoverBuf
            = m_gdxControls.m_foldoverModel.valueBuffer();
    const ValueBuffer* modulatorBuf
            = m_gdxControls.m_modulatorModel.valueBuffer();
    const ValueBuffer* crossoverBuf
            = m_gdxControls.m_crossoverModel.valueBuffer();
    const ValueBuffer* outGainBuf
            = m_gdxControls.m_outGainModel.valueBuffer();

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        real_t simple = simpleBuf ? simpleBuf->value(f)
                                  : m_gdxControls.m_simpleModel.value();
        real_t foldover = foldoverBuf ? foldoverBuf->value(f)
                                      : m_gdxControls.m_foldoverModel.value();
        real_t modulator = modulatorBuf
                                   ? modulatorBuf->value(f)
                                   : m_gdxControls.m_modulatorModel.value();
        real_t crossover = crossoverBuf
                                   ? crossoverBuf->value(f)
                                   : m_gdxControls.m_crossoverModel.value();
        real_t outGain = outGainBuf ? outGainBuf->value(f)
                                    : m_gdxControls.m_outGainModel.value();

        sample_t curVal0 = bound(-1., _buf[f][0] * simple, 1.);
        sample_t curVal1 = bound(-1., _buf[f][1] * simple, 1.);

        if(foldover > 1.)
        {
            curVal0 = fmod((curVal0 + 1.) * foldover, 4.);
            if(curVal0 > 2.)
                curVal0 = 4. - curVal0;
            curVal0 -= 1.;
            curVal1 = fmod((curVal1 + 1.) * foldover, 4.);
            if(curVal1 > 2.)
                curVal1 = 4. - curVal1;
            curVal1 -= 1.;
        }
        if(modulator > 1.)
        {
            curVal0 = sign(curVal0) * fmod(abs(curVal0) * modulator, 1.);
            curVal1 = sign(curVal1) * fmod(abs(curVal1) * modulator, 1.);
        }
        if(crossover > 1.)
        {
            curVal0 = fmod(curVal0 * crossover + 1., 2.) - 1.;
            curVal1 = fmod(curVal1 * crossover + 1., 2.) - 1.;
        }

        curVal0 = bound(-1., curVal0, 1.);
        curVal1 = bound(-1., curVal1, 1.);

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0 * outGain;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1 * outGain;
    }

    return shouldKeepRunning(_buf, _frames);
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new DistortorGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
