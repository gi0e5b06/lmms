/*
 * VocoderGDX.cpp -
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

#include "VocoderGDX.h"

//#include "WaveForm.h"
#include "embed.h"
//#include "lmms_math.h"
#include "denormals.h"

#include <cmath>

//#define FFT_BUFFER_SIZE 2048
// 22050

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT vocodergdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "VocoderGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A scarifying plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

VocoderGDX::VocoderGDX(Model*                                    parent,
                       const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&vocodergdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_filled(0)
{
        m_size = Engine::mixer()->processingSampleRate(); //20;  // 5Hz
    // m_size = qMax<int>(2048, Engine::mixer()->framesPerPeriod());
    //m_size = qMax<int>(128, Engine::mixer()->framesPerPeriod());

    m_rBuf0 = (float*)fftwf_malloc(m_size * sizeof(float));
    m_rBuf1 = (float*)fftwf_malloc(m_size * sizeof(float));
    m_oBuf  = (float*)fftwf_malloc(m_size * sizeof(float));
    memset(m_rBuf0, 0, m_size * sizeof(float));
    memset(m_rBuf1, 0, m_size * sizeof(float));
    memset(m_oBuf, 0, m_size * sizeof(float));

    m_cBuf0 = (fftwf_complex*)fftwf_malloc((m_size / 2 + 1)
                                           * sizeof(fftwf_complex));
    m_cBuf1 = (fftwf_complex*)fftwf_malloc((m_size / 2 + 1)
                                           * sizeof(fftwf_complex));

    m_r2cPlan0
            = fftwf_plan_dft_r2c_1d(m_size, m_rBuf0, m_cBuf0, FFTW_MEASURE);
    m_r2cPlan1
            = fftwf_plan_dft_r2c_1d(m_size, m_rBuf1, m_cBuf1, FFTW_MEASURE);

    m_c2rPlan = fftwf_plan_dft_c2r_1d(m_size, m_cBuf1, m_oBuf, FFTW_MEASURE);
}

VocoderGDX::~VocoderGDX()
{
    fftwf_destroy_plan(m_c2rPlan);
    fftwf_destroy_plan(m_r2cPlan1);
    fftwf_destroy_plan(m_r2cPlan0);
    fftwf_free(m_cBuf0);
    fftwf_free(m_cBuf1);
    fftwf_free(m_oBuf);
    fftwf_free(m_rBuf0);
    fftwf_free(m_rBuf1);
}

/*
bool VocoderGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
    {
        return false;
    }

    for(fpp_t f = 0; f < _frames; ++f)
    {
        float w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        float curVal0 = _buf[f][0];
        float curVal1 = _buf[f][1];

        curVal0 = sign(curVal0) * abs(curVal1);
        curVal1 = curVal0;

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
    }

    return shouldKeepRunning(_buf, _frames);
}
*/

bool VocoderGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    int frames = qMin<int>(_frames, m_size);
    // memmove(m_rBuf, m_rBuf + frames, (m_size - frames));
    int start = m_size - frames;
    // memset(m_rBuf + start, 0, frames);

    // m_filled=0;
    do
    {
        for(int f = frames; f < m_size; f++)
        {
            m_rBuf0[f - frames] = m_rBuf0[f];
            m_rBuf1[f - frames] = m_rBuf1[f];
        }

        for(fpp_t f = 0; f < frames; ++f)
        {
            m_rBuf0[start + f] = _buf[f][0];
            m_rBuf1[start + f] = _buf[f][1];
        }

        m_filled += frames;
    } while(m_filled < m_size);

    if(m_filled < m_size)
    {
        // qInfo("filled=%d", m_filled);
        return true;
    }

    m_filled = m_size;

    /*
    if(!m_r2cPlan || !m_c2rPlan)
    {
        qCritical("bad plans");
        return false;
    }
    */

    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
    {
        return false;
    }

    /*
    for(fpp_t f = 0; f < frames; ++f)
    {
        _buf[f][0] = 0.f;
        _buf[f][1] = 0.f;
    }
    */

    fftwf_execute(m_r2cPlan0);
    fftwf_execute(m_r2cPlan1);

    for(int f = 0; f < m_size; f++)
        m_oBuf[f] = 0.f;

    // float base  = powf(2.f, (24 + m_gdxControls.m_keyModel.value())
    // / 12.f);int   mode =
    // m_gdxControls.m_modeModel.value();

    /*
    const int S = 21;
    for(int w = 0; w < m_size / 2 + 1 - S; w++)
    {
        for(int d = 1; d < S; d++)
            m_cBuf0[w][0] += m_cBuf0[w + d][0];
        m_cBuf0[w][0] /= S;
    }
    for(int w = 0; w < m_size / 2 + 1 - S; w++)
    {
        for(int d = 1; d < S; d++)
            m_cBuf1[w][0] += m_cBuf1[w + d][0];
        m_cBuf1[w][0] /= S;
        // if(w%2) m_cBuf1[w][0]=0.;
    }
    */

    float width = m_gdxControls.m_widthModel.value();
    for(int w = 0; w < m_size / 2 + 1; w++)
    {
        // m_cBuf1[w][0] = (m_cBuf1[w][0] > width ? m_cBuf0[w][0] : 0.f);
        // m_cBuf1[w][1] = (m_cBuf1[w][1] > width ? m_cBuf0[w][1] : 0.f);

        /*
          m_cBuf1[w][0] =sign(m_cBuf1[w][0])*
          (abs(0.8f*m_cBuf1[w][0])+0.2f*abs(m_cBuf0[w][0]));
          m_cBuf1[w][1] =sign(m_cBuf1[w][1])*
          (abs(0.8f*m_cBuf1[w][1])+0.2f*abs(m_cBuf0[w][1]));
        */
        /*
          m_cBuf1[w][0]
          = qMin(abs(m_cBuf1[w][0]), abs(m_cBuf0[w][0]));
          m_cBuf1[w][1]
          = qMin(abs(m_cBuf1[w][1]), abs(m_cBuf0[w][1]));
        */
        // m_cBuf1[w][0] *= abs(m_cBuf0[w][0]);
        // m_cBuf1[w][1] *= abs(m_cBuf0[w][1]);
        const float rho1 = qBound(0.f,
                                  sqrt(m_cBuf1[w][0] * m_cBuf1[w][0]
                                       + m_cBuf1[w][1] * m_cBuf1[w][1]),
                                  1.f);
        const float rho0 = qBound(0.f,
                                  sqrt(m_cBuf0[w][0] * m_cBuf0[w][0]
                                       + m_cBuf0[w][1] * m_cBuf0[w][1]),
                                  1.f);
        if(rho0 <= width)
        {
            m_cBuf1[w][0] = 0.;
            m_cBuf1[w][1] = 0.;
        }
        else
        {
            m_cBuf1[w][0] = qBound(-1.f, m_cBuf0[w][0] * rho1 / rho0, 1.f);
            m_cBuf1[w][1] = qBound(-1.f, m_cBuf0[w][1] * rho1 / rho0, 1.f);
        }
        if(w > 2000)
        {
            m_cBuf1[w][0] *= 1. / (w / 100 - 19);
            m_cBuf1[w][1] *= 1. / (w / 100 - 19);
        }
        if(w < 100)
        {
            m_cBuf1[w][0] *= 1. / (11 - w / 10);
            m_cBuf1[w][1] *= 1. / (11 - w / 10);
        }
        /*
if(w > 100 && w < 600)
            if(fastrandf01inc() < 0.0005f)
                qInfo("w=%d rho=%f c0=%f c1=%f", w, rho, m_cBuf1[w][0],
                      m_cBuf1[w][1]);
        */
    }

    fftwf_execute(m_c2rPlan);

    {
        float a = 1.f / m_size * m_gdxControls.m_ampModel.value();
        for(int f = 0; f < m_size; f++)
            m_oBuf[f] = qBound(-1.f, m_oBuf[f] * a, 1.f);
    }

    for(fpp_t f = 0; f < frames; ++f)
    {
        float w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        float v = m_oBuf[start + f];

        // if(fastrandf01inc() < 0.05f)
        //    if(f == 0)
        //        qInfo("v[0]=%f", v);

        float curVal0 = v;
        float curVal1 = curVal0;

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
    }

    bool r = shouldKeepRunning(_buf, _frames);
    // if(!r)
    //    m_filled = 0;
    return r;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new VocoderGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
