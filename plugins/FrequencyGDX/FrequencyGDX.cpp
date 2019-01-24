/*
 * FrequencyGDX.cpp -
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
               QT_TRANSLATE_NOOP("pluginBrowser", "A frequency detector"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

static frequency_t REF_FREQS[128];

int closest_key(frequency_t _f, real_t _delta)
{
    if(_f < 7.9)
        return -1;
    if(_f > 13000.)
        return 128;

    real_t vmin = 100000.;
    int    kmin = 0;
    for(int k = 0; k < 128; k++)
    {
        real_t v = abs(_f / REF_FREQS[k] - 1.);
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
    setColor(QColor(59, 128, 74));

    memset(m_buffer, 0, 2 * FFT_BUFFER_SIZE * sizeof(FLOAT));

    m_specBuf = (fftwf_complex*)fftwf_malloc((FFT_BUFFER_SIZE + 1)
                                             * sizeof(fftwf_complex));
    m_fftPlan = fftwf_plan_dft_r2c_1d(FFT_BUFFER_SIZE * 2, m_buffer,
                                      m_specBuf, FFTW_MEASURE);

    // A4 69 440Hz
    for(int k = 0; k < 128; k++)
    {
        real_t pitch = (k - 69) / 12.;
        REF_FREQS[k] = 440. * pow(2., pitch);
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

    const int   mode = m_gdxControls.m_modeModel.value();
    frequency_t tf   = 0.;
    switch(mode)
    {
        case 2:
        {
            fftwf_execute(m_fftPlan);
            absspec(m_specBuf, m_absSpecBuf, FFT_BUFFER_SIZE + 1);

            const int topfq = topband(m_absSpecBuf, FFT_BUFFER_SIZE + 1);
            m_energy        = maximum(m_absSpecBuf, FFT_BUFFER_SIZE + 1);
            tf = round(real_t(topfq) * 22050. / real_t(FFT_BUFFER_SIZE));
        }
        break;
        case 0:
        {
            int    topfq = 0;
            real_t first = -1;
            int    f     = 0;
            while(f < m_framesFilledUp - 1)
            {
                int i = 2 * FFT_BUFFER_SIZE - m_framesFilledUp + f;
                f++;
                if(m_buffer[i] < 0. && m_buffer[i + 1] >= 0.)
                //|| (m_buffer[i] > 0. && m_buffer[i + 1] <= 0.))
                {
                    first = i
                            + abs(m_buffer[i])
                                      / (abs(m_buffer[i]) + m_buffer[i + 1]);
                    break;
                }
            }
            real_t second = -1;
            if(first >= 0)
            {
                while(f < m_framesFilledUp - 1)
                {
                    int i = 2 * FFT_BUFFER_SIZE - m_framesFilledUp + f;
                    f++;
                    if(m_buffer[i] < 0. && m_buffer[i + 1] >= 0.)
                    //|| (m_buffer[i] > 0. && m_buffer[i + 1] <= 0.))
                    {
                        second = i
                                 + abs(m_buffer[i])
                                           / (abs(m_buffer[i])
                                              + m_buffer[i + 1]);
                        ;
                        break;
                    }
                }
                if(second >= 0. && (second - first > 4.))
                {
                    topfq = round(
                            real_t(Engine::mixer()->processingSampleRate())
                            / (1. * (second - first)));
                }
            }
            // qInfo("FrequencyGDX first=%d second=%d topfq=%d", first,
            // second,
            //      topfq);
            m_energy = 0.;
            tf = frequency_t(topfq);  // * 22050. / real_t(FFT_BUFFER_SIZE);
        }
        break;
        case 1:
        default:
        {
            m_energy = 0.;
            tf       = 0.;
        }
        break;
    }
    qInfo("FrequencyGDX: %f Hz, E=%f", tf, m_energy);

    if(tf > 0.)
    {
        m_gdxControls.m_topFrequencyModel.setValue(tf);

        int tk = closest_key(tf, 0.16);
        // if(tk >= 0 && tk <= 127)
        {
            m_gdxControls.m_topKeyModel.setValue(tk);
            m_gdxControls.m_topNoteModel.setValue(tk < 0 ? -1 : tk % 12);
        }

        int ak = closest_key(tf, 0.01);
        if(ak == m_ak && ak >= 0 && ak <= 127)
        {
            frequency_t af = REF_FREQS[ak];
            m_gdxControls.m_avgFrequencyModel.setValue(af);
            m_gdxControls.m_avgKeyModel.setValue(ak);
            m_gdxControls.m_avgNoteModel.setValue(ak < 0 ? -1 : ak % 12);
        }

        if(ak == m_ak && ak == tk)
        {
            int mk = tk;
            m_gdxControls.m_mainKeyModel.setValue(mk);
            m_gdxControls.m_mainNoteModel.setValue(mk < 0 ? -1 : mk % 12);
            if(mk >= 0 && mk <= 127)
            {
                frequency_t mf = REF_FREQS[mk];
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
