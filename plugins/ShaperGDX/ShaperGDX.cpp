/*
 * ShaperGDX.cpp -
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

#include "ShaperGDX.h"

#include "Configuration.h"
#include "WaveForm.h"
#include "embed.h"
//#include "lmms_math.h"

#include <cmath>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT shapergdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "ShaperGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A wave shaping plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

ShaperGDX::ShaperGDX(Model*                                    parent,
                     const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&shapergdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_fpp(Engine::mixer()->framesPerPeriod()),
      m_phase(0.)
{
    setColor(QColor(160, 160, 74));

    int periodsPerDisplayRefresh = qMax(
            1, int(ceil(real_t(Engine::mixer()->processingSampleRate())
                        / m_fpp / CONFIG_GET_INT("ui.framespersecond"))));
    m_ring = new Ring(m_fpp * periodsPerDisplayRefresh);
}

ShaperGDX::~ShaperGDX()
{
}

bool ShaperGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    const ValueBuffer* timeBuf  = m_gdxControls.m_timeModel.valueBuffer();
    const ValueBuffer* ratioBuf = m_gdxControls.m_ratioModel.valueBuffer();
    const ValueBuffer* fillBuf  = m_gdxControls.m_fillModel.valueBuffer();
    const ValueBuffer* hardBuf  = m_gdxControls.m_hardModel.valueBuffer();
    const ValueBuffer* outGainBuf
            = m_gdxControls.m_outGainModel.valueBuffer();

    const WaveForm* wf
            = WaveForm::get(m_gdxControls.m_waveBankModel.value(),
                            m_gdxControls.m_waveIndexModel.value());

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        const real_t time = timeBuf ? timeBuf->value(f)
                                    : m_gdxControls.m_timeModel.value();
        const real_t ratio = ratioBuf ? ratioBuf->value(f)
                                      : m_gdxControls.m_ratioModel.value();
        const real_t fill = fillBuf ? fillBuf->value(f)
                                    : m_gdxControls.m_fillModel.value();
        real_t outGain = outGainBuf ? outGainBuf->value(f)
                                    : m_gdxControls.m_outGainModel.value();
        const real_t hard = hardBuf ? hardBuf->value(f)
                                    : m_gdxControls.m_hardModel.value();

        sample_t curVal0 = _buf[f][0];
        sample_t curVal1 = _buf[f][1];

        m_phase = fraction(m_phase);

        real_t waveGain = wf->f(m_phase);
        real_t waveAmpl = abs(waveGain);

        if(ratio < 1.)
        {
            waveGain = ratio * waveGain + (1. - ratio);
        }

        m_phase += (1000. / time / Engine::mixer()->processingSampleRate());

        curVal0 = ((1. - hard) * curVal0 + hard * abs(curVal0)) * waveGain;
        curVal1 = ((1. - hard) * curVal1 + hard * abs(curVal1)) * waveGain;

        if(fill > 0.)
        {
            if(abs(curVal0) < waveAmpl)
                curVal0 = sign(curVal0)
                          * qMin(waveAmpl, abs(curVal0) * (1. + fill));
            if(abs(curVal1) < waveAmpl)
                curVal1 = sign(curVal1)
                          * qMin(waveAmpl, abs(curVal1) * (1. + fill));
        }

        curVal0 = bound(-1., curVal0 * outGain, 1.);
        curVal1 = bound(-1., curVal1 * outGain, 1.);

        sampleFrame s = {waveGain, curVal0};
        m_ring->write(s);

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
    }

    // m_gdxControls.emit nextStereoBuffer(_buf);

    return shouldKeepRunning(_buf, _frames);
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new ShaperGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
