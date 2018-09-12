/*
 * SmashGDX.cpp -
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

#include "SmashGDX.h"

#include "embed.h"
#include "lmms_math.h"

#include <math.h>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT smashgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "SmashGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A wall effect plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

SmashGDXEffect::SmashGDXEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&smashgdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_idx(0), m_refVal0(0.), m_refVal1(0.)
{
}

SmashGDXEffect::~SmashGDXEffect()
{
}

bool SmashGDXEffect::processAudioBuffer(sampleFrame* _buf,
                                        const fpp_t  _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    const int SR = Engine::mixer()->processingSampleRate();

    const ValueBuffer* rateBuf  = m_gdxControls.m_rateModel.valueBuffer();
    const ValueBuffer* levelBuf = m_gdxControls.m_levelModel.valueBuffer();
    // const ValueBuffer* wetBuf = m_gdxControls.m_wetModel.valueBuffer();

    for(fpp_t f = 0; f < _frames; ++f)
    {
        float w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        float rate = rateBuf ? rateBuf->value(f)
                             : m_gdxControls.m_rateModel.value();
        float level = levelBuf ? levelBuf->value(f)
                               : m_gdxControls.m_levelModel.value();
        // float wet = wetBuf
        //  ? wetBuf->value(f)
        //  : m_gdxControls.m_wetModel.value();

        if(m_idx >= SR * rate)
        {
            m_refVal0 = _buf[f][0];
            m_refVal1 = _buf[f][1];
            m_idx     = 0;
        }
        else
            m_idx++;

        sample_t curVal0 = m_refVal0;
        sample_t curVal1 = m_refVal1;

        if(level > 0.f)
        {
            curVal0 = roundf(curVal0 / level) * level;
            curVal1 = roundf(curVal1 / level) * level;
        }

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
    }

    return true;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new SmashGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
