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

CompressorGDXEffect::CompressorGDXEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&compressorgdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_fact0(0.0f), m_sact0(0.0f)
{
}

CompressorGDXEffect::~CompressorGDXEffect()
{
}

bool CompressorGDXEffect::processAudioBuffer(sampleFrame* _buf,
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
        float oamp = (float)(sngPosBuf ? sngPosBuf->value(f)
                                       : m_gdxControls.m_sngPosModel.value());
        /*
        float dpos = (float)(delPosBuf ? delPosBuf->value(f)
                                       : m_gdxControls.m_delPosModel.value());
        */

        float curVal0 = _buf[f][0];
        float curVal1 = _buf[f][1];

        {
                if(fabs(curVal0)>=ramp)
                        curVal0=sign(curVal0)*(ramp+(fabs(curVal0)-ramp)*famp);
        }

        {
                if(fabs(curVal1)>=ramp)
                        curVal1=sign(curVal1)*(ramp+(fabs(curVal1)-ramp)*famp);
        }

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0 * oamp;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1 * oamp;
    }

    return shouldKeepRunning(_buf,_frames);
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new CompressorGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
