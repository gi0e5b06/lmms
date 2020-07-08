/*
 * WallGDX.cpp - wall effect
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

#include "WallGDX.h"

#include <math.h>

#include "embed.h"
#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT wallgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "WallGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A wall effect plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

WallGDXEffect::WallGDXEffect(Model*                                    parent,
                             const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&wallgdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_buffer(NULL), m_len(0), m_idx(0)
{
    // TODO: move that to startRunning()
    m_len    = Engine::mixer()->processingSampleRate();
    m_buffer = MM_ALLOC(sampleFrame, m_len);
    memset(m_buffer, 0, m_len * sizeof(sampleFrame));
}

WallGDXEffect::~WallGDXEffect()
{
    m_len = 0;
    MM_FREE(m_buffer);
    m_buffer = NULL;
}

bool WallGDXEffect::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    const ValueBuffer* distanceBuf
            = m_gdxControls.m_distanceModel.valueBuffer();
    const ValueBuffer* dryBuf = m_gdxControls.m_dryModel.valueBuffer();
    const ValueBuffer* wetBuf = m_gdxControls.m_wetModel.valueBuffer();

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        m_idx              = (m_idx + 1) % m_len;
        m_buffer[m_idx][0] = _buf[f][0];
        m_buffer[m_idx][1] = _buf[f][1];

        real_t distance = distanceBuf ? distanceBuf->value(f)
                                     : m_gdxControls.m_distanceModel.value();
        real_t dry = dryBuf
                                  ? dryBuf->value(f)
                                  : m_gdxControls.m_dryModel.value();
        real_t wet = wetBuf
                                  ? wetBuf->value(f)
                                  : m_gdxControls.m_wetModel.value();

        int32_t t = (m_idx + static_cast<int32_t>(m_len * (1.f - distance)))
                    % m_len;

        sample_t curVal0 = dry * _buf[f][0] - wet * m_buffer[t][1];
        sample_t curVal1 = dry * _buf[f][1] - wet * m_buffer[t][1];

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
        return new WallGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
