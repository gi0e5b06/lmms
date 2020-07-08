/*
 * ScarifierGDX.cpp -
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
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

#include "ScarifierGDX.h"

//#include "WaveForm.h"
#include "embed.h"
//#include "lmms_math.h"
#include "denormals.h"

#include <cmath>

//#define FFT_BUFFER_SIZE 2048
// 22050

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT scarifiergdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "ScarifierGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A scarifying plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               nullptr,
               nullptr};
}

ScarifierGDX::ScarifierGDX(Model*                                    parent,
                           const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&scarifiergdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_filled(0)
{
    // m_size = Engine::mixer()->processingSampleRate();  // / 20;  // 5Hz
    m_size = qMax<int>(2048, Engine::mixer()->framesPerPeriod());

    m_rBuf = (FLOAT*)fftwf_malloc(m_size * sizeof(FLOAT));
    memset(m_rBuf, 0, m_size * sizeof(FLOAT));

    m_cBuf    = (fftwf_complex*)fftwf_malloc((m_size / 2 + 1)
                                          * sizeof(fftwf_complex));
    m_r2cPlan = fftwf_plan_dft_r2c_1d(m_size, m_rBuf, m_cBuf, FFTW_MEASURE);

    m_c2rPlan = fftwf_plan_dft_c2r_1d(m_size, m_cBuf, m_rBuf, FFTW_MEASURE);
}

ScarifierGDX::~ScarifierGDX()
{
    fftwf_destroy_plan(m_c2rPlan);
    fftwf_destroy_plan(m_r2cPlan);
    fftwf_free(m_cBuf);
    fftwf_free(m_rBuf);
}

bool ScarifierGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    int frames = qMin<int>(_frames, m_size);
    // memmove(m_rBuf, m_rBuf + frames, (m_size - frames));
    int start = m_size - frames;
    // memset(m_rBuf + start, 0, frames);

    // m_filled=0;
    do
    {
        for(int f = frames; f < m_size; f++)
            m_rBuf[f - frames] = m_rBuf[f];

        for(fpp_t f = 0; f < frames; ++f)
        {
            m_rBuf[start + f] = (_buf[f][0] + _buf[f][1]) * 0.5;
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

    for(fpp_t f = 0; f < frames; ++f)
    {
        _buf[f][0] = 0.;
        _buf[f][1] = 0.;
    }

    fftwf_execute(m_r2cPlan);

    for(int f = 0; f < m_size; f++)
        m_rBuf[f] = 0.;

    real_t base  = pow(2., (24 + m_gdxControls.m_keyModel.value()) / 12.);
    real_t width = m_gdxControls.m_widthModel.value();
    int    mode  = m_gdxControls.m_modeModel.value();
    for(int w = 0; w < m_size / 2 + 1; w++)
    {
        int f = w * Engine::mixer()->processingSampleRate() / m_size;
        // qInfo("%5d f=%d r=%f o=%f c0=%f c1=%f", w, f, r, o,
        // m_cBuf[w][0], m_cBuf[w][1]);

        real_t a = 0.;
        for(int z = 0; z <= 8; z++)
        {
            real_t aa
                    = 1.
                      / (1. + pow(abs(f - base * pow(2., real_t(z))), width));
            if(!isnan(aa))
                a += aa;
            else
                qWarning("AA is nan!");
        }
        a = bound(0., a, 1.);
        switch(mode)
        {
            case 0:
                a = 1. - a;
                break;
            case 1:
                break;
            case 2:
                a = 0.5 + a / 2.;
                break;
        }
        m_cBuf[w][0] *= a;
        m_cBuf[w][1] *= a;

        // if(f > 100. && f < 600.)
        //    if(fastrandf01inc() < 0.0005)
        //        qInfo("%5d f=%d a=%f c0=%f c1=%f", w, f, a, m_cBuf[w][0],
        //              m_cBuf[w][1]);
    }

    fftwf_execute(m_c2rPlan);

    {
        real_t a = 1. / m_size * m_gdxControls.m_ampModel.value();
        for(int f = 0; f < m_size; f++)
            m_rBuf[f] = bound(-1., m_rBuf[f] * a, 1.);
    }

    for(fpp_t f = 0; f < frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        sample_t v = m_rBuf[start + f];

        // if(fastrandf01inc() < 0.05)
        //    if(f == 0)
        //        qInfo("v[0]=%f", v);

        sample_t curVal0 = v;
        sample_t curVal1 = curVal0;

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
        return new ScarifierGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
