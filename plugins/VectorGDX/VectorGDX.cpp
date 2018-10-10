/*
 * VectorGDX.cpp -
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

#include "VectorGDX.h"

#include "Ring.h"
#include "WaveForm.h"
#include "embed.h"
#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT vectorgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "VectorGDX",
               QT_TRANSLATE_NOOP("pluginBrowser",
                                 "A generic matrix effect plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

VectorGDXEffect::VectorGDXEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&vectorgdx_plugin_descriptor, parent, key),
      m_gdxControls(this)
{
    const sample_rate_t SR  = Engine::mixer()->baseSampleRate();
    const f_cnt_t       rsz = 2 * SR + 1;

    m_ring     = new Ring(rsz);
    m_feedback = new Ring(rsz);
}

VectorGDXEffect::~VectorGDXEffect()
{
}

bool VectorGDXEffect::processAudioBuffer(sampleFrame* _buf,
                                         const fpp_t  _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    m_ring->write(_buf, _frames);

    const real_t  gain = m_gdxControls.m_gainModel.value();
    const int*    p    = m_gdxControls.pos();
    const real_t* m    = m_gdxControls.vector();
    const int     n    = m_gdxControls.size();
    const int     last = n - 1;

    const real_t feedback
            = WaveForm::sqrt(m_gdxControls.m_feedbackModel.value() / 100.);
    const int latency = m_gdxControls.m_latencyModel.value();

    for(fpp_t f = 0; f < _frames; ++f)
    {
        float w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        sampleFrame s = {0., 0.};
        for(int ch = 0; ch < 2; ch++)
        {
            for(int i = last; i >= 0; --i)
            {
                real_t q = m_ring->value(-_frames + 1 + f - p[i], ch);
                /*
                if(feedback != 0.)
                q = (1. - feedback) * q
                        + feedback / 2.
                                  * m_feedback->value(-latency - 2 * _frames
                                                              + 1 + f - p[i],
                                                      ch);
                */
                s[ch] += m[i] * q;
                // if(f==0 && i==0) qInfo("i=%d m=%f p=%d q=%f
                // s=%f",i,m[i],p[i],q,s[ch]);
            }

            if(feedback != 0.)
                s[ch] = (1. - feedback) * s[ch]
                        + feedback
                                  * m_feedback->value(-latency - 2 * _frames
                                                              + 1 + f,
                                                      ch);
            s[ch] *= gain;
        }

        _buf[f][0] = d0 * _buf[f][0] + w0 * bound(-1., s[0], 1.);
        _buf[f][1] = d1 * _buf[f][1] + w1 * bound(-1., s[1], 1.);
    }

    m_feedback->write(_buf, _frames);

    return true;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new VectorGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
