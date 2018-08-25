/*
 * FrequencyGDX.cpp -
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

#include "FrequencyGDX.h"

//#include "WaveForm.h"
#include "embed.h"
//#include "lmms_math.h"

#include <cmath>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT frequencygdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "FrequencyGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A wave shaping plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

static float REF_FREQS[128];

int closest_key(float _f, float _delta)
{
    if(_f < 7.9f)
        return -1;
    if(_f > 13000.f)
        return 128;

    float vmin = 100000.f;
    int   kmin = 0;
    for(int k = 0; k < 128; k++)
    {
        float v = fabsf(_f / REF_FREQS[k] - 1.f);
        if(v <= vmin)
        {
            vmin = v;
            kmin = k;
        }
    }
    return (vmin <= _delta ? kmin : -1);
}

FrequencyGDX::FrequencyGDX(Model*                                    parent,
                           const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&frequencygdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_framesFilledUp(0), m_ak(-1), m_energy(0)
{
    memset(m_buffer, 0, 2 * FFT_BUFFER_SIZE);

    m_specBuf = (fftwf_complex*)fftwf_malloc((FFT_BUFFER_SIZE + 1)
                                             * sizeof(fftwf_complex));
    m_fftPlan = fftwf_plan_dft_r2c_1d(FFT_BUFFER_SIZE * 2, m_buffer,
                                      m_specBuf, FFTW_MEASURE);

    // A4 69 440Hz
    for(int k = 0; k < 128; k++)
    {
        float pitch  = (k - 69) / 12.f;
        REF_FREQS[k] = 440.f * powf(2.f, pitch);
    }
}

FrequencyGDX::~FrequencyGDX()
{
    fftwf_destroy_plan(m_fftPlan);
    fftwf_free(m_specBuf);
}

bool FrequencyGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    int frames = qMin<int>(_frames, 2 * FFT_BUFFER_SIZE);
    memmove(m_buffer, m_buffer + frames, 2 * FFT_BUFFER_SIZE - frames);
    int start = 2 * FFT_BUFFER_SIZE - frames;
    // memset(m_buffer + start, 0, frames);

    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
    {
        m_framesFilledUp = 0;
        return false;
    }

    const int cm = LeftChannel;  // m_saControls.m_channelMode.value();

    switch(cm)
    {
        case MergeChannels:
            for(fpp_t f = 0; f < frames; ++f)
            {
                m_buffer[start + f] = (_buf[f][0] + _buf[f][1]) * 0.5;
            }
            break;
        case LeftChannel:
            for(fpp_t f = 0; f < frames; ++f)
            {
                m_buffer[start + f] = _buf[f][0];
            }
            break;
        case RightChannel:
            for(fpp_t f = 0; f < frames; ++f)
            {
                m_buffer[start + f] = _buf[f][1];
            }
            break;
    }

    m_framesFilledUp += frames;

    // qInfo("ffu=%d FBS=%d frames=%d", m_framesFilledUp, FFT_BUFFER_SIZE,
    //      frames);

    if(m_framesFilledUp < 2 * FFT_BUFFER_SIZE)
        return isRunning();
    else
        m_framesFilledUp = 2 * FFT_BUFFER_SIZE;

    fftwf_execute(m_fftPlan);
    absspec(m_specBuf, m_absSpecBuf, FFT_BUFFER_SIZE + 1);

    const int topf = topband(m_absSpecBuf, FFT_BUFFER_SIZE + 1);
    m_energy       = maximum(m_absSpecBuf, FFT_BUFFER_SIZE + 1);
    // qInfo("FrequencyGDX: %d Hz, E=%f", topf, m_energy);

    if(topf > 0)
    {
        float tf = float(topf) * 22050.f / float(FFT_BUFFER_SIZE);
        m_gdxControls.m_topFrequencyModel.setValue(tf);

        int tk = closest_key(tf, 0.16f);
        // if(tk >= 0 && tk <= 127)
        {
            m_gdxControls.m_topKeyModel.setValue(tk);
            m_gdxControls.m_topNoteModel.setValue(tk < 0 ? -1 : tk % 12);
        }

        int ak = closest_key(tf, 0.01f);
        if(ak == m_ak && ak >= 0 && ak <= 127)
        {
            float af = REF_FREQS[ak];
            m_gdxControls.m_avgFrequencyModel.setValue(af);
            m_gdxControls.m_avgKeyModel.setValue(ak);
            m_gdxControls.m_avgNoteModel.setValue(ak < 0 ? -1 : ak % 12);
        }

        if(ak==m_ak && ak == tk)
        {
            int mk = tk;
            m_gdxControls.m_mainKeyModel.setValue(mk);
            m_gdxControls.m_mainNoteModel.setValue(mk < 0 ? -1 : mk % 12);
            if(mk >= 0 && mk <= 127)
            {
                float mf = REF_FREQS[mk];
                m_gdxControls.m_mainFrequencyModel.setValue(mf);
            }
        }

        m_ak = ak;
    }

    m_framesFilledUp = 0;

    return shouldKeepRunning(_buf, _frames);
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new FrequencyGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
