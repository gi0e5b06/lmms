/*
 * ChainGDX.cpp - A chaining effect
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

#include "ChainGDX.h"

//#include <math.h>

#include "MemoryManager.h"
#include "embed.h"
//#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT chaingdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "ChainGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A chaining effect"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

ChainGDXEffect::ChainGDXEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&chaingdx_plugin_descriptor, parent, key),
      m_gdxControls(this)
{
    m_chain = new EffectChain(this);
    m_chain->setEnabled(true);
}

ChainGDXEffect::~ChainGDXEffect()
{
    delete m_chain;
}

bool ChainGDXEffect::processAudioBuffer(sampleFrame* _buf,
                                        const fpp_t  _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    sampleFrame* ecb = MM_ALLOC(sampleFrame, _frames);
    memcpy(ecb, _buf, sizeof(sampleFrame) * _frames);
    //m_chain->startRunning();
    bool r = m_chain->processAudioBuffer(ecb, _frames, true);

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        _buf[f][0] = d0 * _buf[f][0] + w0 * ecb[f][0];
        _buf[f][1] = d1 * _buf[f][1] + w1 * ecb[f][1];
    }

    MM_FREE(ecb);
    return r;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new ChainGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
