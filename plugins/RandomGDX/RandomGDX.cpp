/*
 * RandomGDX.cpp - A click remover
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

#include "RandomGDX.h"

#include <math.h>

#include "embed.h"
#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT randomgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "RandomGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A randomizer plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

RandomGDXEffect::RandomGDXEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&randomgdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_fact0(0.0f), m_sact0(0.0f)
{
}

RandomGDXEffect::~RandomGDXEffect()
{
}

bool RandomGDXEffect::processAudioBuffer(sampleFrame* _buf,
                                         const fpp_t  _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    const ValueBuffer* rndAmpBuf = m_gdxControls.m_rndAmpModel.valueBuffer();
    const ValueBuffer* fixAmpBuf = m_gdxControls.m_fixAmpModel.valueBuffer();
    const ValueBuffer* sngPosBuf = m_gdxControls.m_sngPosModel.valueBuffer();
    const ValueBuffer* delPosBuf = m_gdxControls.m_delPosModel.valueBuffer();

    for(fpp_t f = 0; f < _frames; ++f)
    {
        float w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        float ramp = (float)(rndAmpBuf ? rndAmpBuf->value(f)
                                       : m_gdxControls.m_rndAmpModel.value());
        float famp = (float)(fixAmpBuf ? fixAmpBuf->value(f)
                                       : m_gdxControls.m_fixAmpModel.value());
        float spos = (float)(sngPosBuf ? sngPosBuf->value(f)
                                       : m_gdxControls.m_sngPosModel.value());
        float dpos = (float)(delPosBuf ? delPosBuf->value(f)
                                       : m_gdxControls.m_delPosModel.value());

        float curVal0 = _buf[f][0];
        float curVal1 = _buf[f][1];

        // dpos*=0.05f;

        {
            float fact = ramp * fastrandf01inc();
            m_fact0    = (m_fact0 * dpos + fact * 0.001f) / (dpos + 0.001f);
            fact       = m_fact0;
            float sact = abs(abs(curVal0) - spos);
            m_sact0    = (m_sact0 * dpos + sact * 0.001f) / (dpos + 0.001f);
            sact       = m_sact0;
            curVal0    = (sign(curVal0) * sact * (1.0f + fact)) * famp;
        }

        {
            float fact = ramp * fastrandf01inc();
            m_fact1    = (m_fact1 * dpos + fact * 0.001f) / (dpos + 0.001f);
            fact       = m_fact1;
            float sact = abs(abs(curVal1) - spos);
            m_sact1    = (m_sact1 * dpos + sact * 0.001f) / (dpos + 0.001f);
            sact       = m_sact1;
            curVal1    = (sign(curVal1) * sact * (1.0f + fact)) * famp;
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
        return new RandomGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
