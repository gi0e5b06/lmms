/*
 * SplitGDX.cpp - A splitting effect (tree node)
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

#include "SplitGDX.h"

//#include <math.h>

#include "MemoryManager.h"
#include "embed.h"
//#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT splitgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "SplitGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A chaining effect"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

SplitGDXEffect::SplitGDXEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&splitgdx_plugin_descriptor, parent, key),
      m_gdxControls(this)
{
    m_splitChain = new EffectChain(this);
    m_wetChain   = new EffectChain(this);
    m_remChain   = new EffectChain(this);

    m_splitChain->setEnabled(true);
    m_wetChain->setEnabled(true);
    m_remChain->setEnabled(true);
}

SplitGDXEffect::~SplitGDXEffect()
{
    delete m_splitChain;
    delete m_wetChain;
    delete m_remChain;
}

bool SplitGDXEffect::processAudioBuffer(sampleFrame* _buf,
                                        const fpp_t  _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    bool r = false;

    sampleFrame* splitb = MM_ALLOC(sampleFrame, _frames);
    sampleFrame* wetb   = MM_ALLOC(sampleFrame, _frames);
    sampleFrame* remb   = MM_ALLOC(sampleFrame, _frames);

    memcpy(splitb, _buf, sizeof(sampleFrame) * _frames);
    if(m_splitChain->isEnabled())
    {
        m_splitChain->startRunning();
        r |= m_splitChain->processAudioBuffer(splitb, _frames, true);

        memcpy(wetb, splitb, sizeof(sampleFrame) * _frames);

        for(fpp_t f = 0; f < _frames; ++f)
        {
            remb[f][0] = _buf[f][0] - splitb[f][0];
            remb[f][1] = _buf[f][1] - splitb[f][1];
        }
    }
    else
    {
        memcpy(wetb, splitb, sizeof(sampleFrame) * _frames);
        memcpy(remb, splitb, sizeof(sampleFrame) * _frames);
    }

    m_wetChain->startRunning();
    r |= m_wetChain->processAudioBuffer(wetb, _frames, true);

    m_remChain->startRunning();
    r |= m_remChain->processAudioBuffer(remb, _frames, true);

    const ValueBuffer* splitBuf = m_gdxControls.m_splitModel.valueBuffer();
    const ValueBuffer* wetBuf   = m_gdxControls.m_splitModel.valueBuffer();
    const ValueBuffer* remBuf   = m_gdxControls.m_splitModel.valueBuffer();

    for(fpp_t f = 0; f < _frames; ++f)
    {
        float w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        const float splitVal
                = (splitBuf ? splitBuf->value(f)
                            : m_gdxControls.m_splitModel.value());
        const float wetVal = (splitBuf ? wetBuf->value(f)
                                       : m_gdxControls.m_wetModel.value());
        const float remVal = (remBuf ? remBuf->value(f)
                                     : m_gdxControls.m_remModel.value());

        _buf[f][0]
                = d0 * _buf[f][0]
                  + w0
                            * (splitVal * splitb[f][0] + wetVal * wetb[f][0]
                               + remVal * remb[f][0]);
        _buf[f][1]
                = d1 * _buf[f][1]
                  + w1
                            * (splitVal * splitb[f][1] + wetVal * wetb[f][1]
                               + remVal * remb[f][1]);
    }

    MM_FREE(splitb);
    MM_FREE(wetb);
    MM_FREE(remb);
    return r;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new SplitGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
