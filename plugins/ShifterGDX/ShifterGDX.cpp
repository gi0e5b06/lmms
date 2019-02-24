/*
 * ShifterGDX.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include "ShifterGDX.h"

//#include "WaveForm.h"
#include "embed.h"
//#include "lmms_math.h"
#include "denormals.h"

#include <cmath>

//#define FFT_BUFFER_SIZE 2048
// 22050

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT shiftergdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "ShifterGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A shifting plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

ShifterGDX::ShifterGDX(Model*                                    parent,
                       const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&shiftergdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_filled(0)
{
    m_size = 40000;
    // m_size = Engine::mixer()->processingSampleRate(); //20;  // 5Hz
    // m_size = qMax<int>(2048, Engine::mixer()->framesPerPeriod());
    // m_size = qMax<int>(128, Engine::mixer()->framesPerPeriod());

    m_rBuf0 = (float*)fftwf_malloc(m_size * sizeof(float));
    m_rBuf1 = (float*)fftwf_malloc(m_size * sizeof(float));
    m_oBuf0 = (float*)fftwf_malloc(m_size * sizeof(float));
    m_oBuf1 = (float*)fftwf_malloc(m_size * sizeof(float));
    memset(m_rBuf0, 0, m_size * sizeof(float));
    memset(m_rBuf1, 0, m_size * sizeof(float));

    // memset(m_oBuf0, 0, m_size * sizeof(float));
    // memset(m_oBuf1, 0, m_size * sizeof(float));

    m_cBuf0 = (fftwf_complex*)fftwf_malloc((m_size / 2 + 1)
                                           * sizeof(fftwf_complex));
    m_cBuf1 = (fftwf_complex*)fftwf_malloc((m_size / 2 + 1)
                                           * sizeof(fftwf_complex));

    m_r2cPlan0
            = fftwf_plan_dft_r2c_1d(m_size, m_rBuf0, m_cBuf0, FFTW_MEASURE);
    m_r2cPlan1
            = fftwf_plan_dft_r2c_1d(m_size, m_rBuf1, m_cBuf1, FFTW_MEASURE);
    m_c2rPlan0
            = fftwf_plan_dft_c2r_1d(m_size, m_cBuf0, m_oBuf0, FFTW_MEASURE);
    m_c2rPlan1
            = fftwf_plan_dft_c2r_1d(m_size, m_cBuf1, m_oBuf1, FFTW_MEASURE);
}

ShifterGDX::~ShifterGDX()
{
    fftwf_destroy_plan(m_c2rPlan0);
    fftwf_destroy_plan(m_c2rPlan1);
    fftwf_destroy_plan(m_r2cPlan0);
    fftwf_destroy_plan(m_r2cPlan1);
    fftwf_free(m_cBuf0);
    fftwf_free(m_cBuf1);
    fftwf_free(m_oBuf0);
    fftwf_free(m_oBuf1);
    fftwf_free(m_rBuf0);
    fftwf_free(m_rBuf1);
}

bool ShifterGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
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

    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
    {
        return false;
    }

    int df = m_gdxControls.m_deltaFrequencyModel.value();

    fftwf_execute(m_r2cPlan0);
    fftwf_execute(m_r2cPlan1);

    if(df > 0)
    {
        if(df > m_size / 2)
            df = m_size / 2 + 1;

        // qInfo("Shifter 1");
        for(int f = m_size / 2 - df; f >= 0; --f)
        {
            m_cBuf0[f + df][0] = m_cBuf0[f][0];
            m_cBuf0[f + df][1] = m_cBuf0[f][1];
            m_cBuf1[f + df][0] = m_cBuf1[f][0];
            m_cBuf1[f + df][1] = m_cBuf1[f][1];
        }
        // qInfo("Shifter 2");
        for(int f = 0; f < df; ++f)
        {
            m_cBuf0[f][0] = 0.;
            m_cBuf0[f][1] = 0.;
            m_cBuf1[f][0] = 0.;
            m_cBuf1[f][1] = 0.;
        }
        // qInfo("Shifter 3");
    }
    else if(df < 0)
    {
        if(df < -m_size / 2)
            df = -m_size / 2 - 1;

        for(int f = -df; f <= m_size / 2; ++f)
        {
            m_cBuf0[f + df][0] = m_cBuf0[f][0];
            m_cBuf0[f + df][1] = m_cBuf0[f][1];
            m_cBuf1[f + df][0] = m_cBuf1[f][0];
            m_cBuf1[f + df][1] = m_cBuf1[f][1];
        }
        for(int f = m_size / 2 + df; f <= m_size / 2; ++f)
        {
            m_cBuf0[f][0] = 0.;
            m_cBuf0[f][1] = 0.;
            m_cBuf1[f][0] = 0.;
            m_cBuf1[f][1] = 0.;
        }
    }

    int    lf = m_gdxControls.m_lowFrequencyModel.value();
    real_t xf = m_gdxControls.m_factorFrequencyModel.value() / 1000.;
    if(lf <= m_size / 2)
    {
        real_t factor = 1.;
        for(int f = lf - 1; f >= 0; --f)
        {
            factor -= xf;
            if(factor < 0.)
                factor = 0.;
            m_cBuf0[f][0] *= factor;
            m_cBuf0[f][1] *= factor;
            m_cBuf1[f][0] *= factor;
            m_cBuf1[f][1] *= factor;
        }
    }

    int hf = m_gdxControls.m_highFrequencyModel.value();
    if(hf <= m_size / 2)
    {
        real_t factor = 1.;
        for(int f = hf + 1; f <= m_size / 2; f++)
        {
            factor -= xf;
            if(factor < 0.)
                factor = 0.;
            m_cBuf0[f][0] *= factor;
            m_cBuf0[f][1] *= factor;
            m_cBuf1[f][0] *= factor;
            m_cBuf1[f][1] *= factor;
        }
    }

    // qInfo("Shifter 4");
    fftwf_execute(m_c2rPlan0);
    fftwf_execute(m_c2rPlan1);
    // qInfo("Shifter 5");

    real_t dv = m_gdxControls.m_deltaVolumeModel.value();
    real_t lv = m_gdxControls.m_lowVolumeModel.value();
    real_t hv = m_gdxControls.m_highVolumeModel.value();
    // qInfo("Shifter LV=%f HV=%f",lv,hv);

    for(fpp_t f = 0; f < _frames; ++f)
    {
        float w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        sample_t curVal0 = bound(lv, m_oBuf0[start + f] / m_size + dv, hv);
        sample_t curVal1 = bound(lv, m_oBuf1[start + f] / m_size + dv, hv);

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
    }
    // qInfo("Shifter 6");

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
        return new ShifterGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
