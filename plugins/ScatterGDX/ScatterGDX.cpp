/*
 * ScatterGDX.cpp - A scatter
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
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

#include "ScatterGDX.h"

#include "Engine.h"
#include "Song.h"
#include "embed.h"

#include <math.h>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT scattergdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "ScatterGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A scatter plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

ScatterGDXEffect::ScatterGDXEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&scattergdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_len(0), m_prev(0), m_pos(0), m_time(0), m_end(0)
{
    setColor(QColor(59, 160, 74));
    m_buffer = MM_ALLOC(sampleFrame, 1536000);  // 8s*192k
}

ScatterGDXEffect::~ScatterGDXEffect()
{
    if(m_buffer)
        MM_FREE(m_buffer);
}

bool ScatterGDXEffect::processAudioBuffer(sampleFrame* _buf,
                                          const fpp_t  _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    uint32_t MAXT = (uint32_t)(4. * Engine::mixer()->processingSampleRate()
                               * 120. / Engine::getSong()->getTempo());
    while(2 * MAXT > 1536000)
        MAXT /= 2;
    // uint32_t MAXT=1536000;
    // uint32_t MINT=MAXT/32; //unused

    int p = (int)m_gdxControls.m_pwrModel.value();
    // if(p<0) p=0;
    // if(p>8) p=8;
    if(p == 0)
    {
        if(m_pos < MAXT)
        {
            for(fpp_t f = 0; (f < _frames) && (m_pos < 2 * MAXT);
                ++f, ++m_pos)
            {
                m_buffer[m_pos][0] = _buf[f][0];
                m_buffer[m_pos][1] = _buf[f][1];
            }
        }
        m_prev = 0;
        return false;
    }

    int f = (int)m_gdxControls.m_frcModel.value();
    if(f == 2)
        m_len = 1 << (p - 1);
    else
        m_len = lround(pow(f, p - 1));

    if(m_len != m_prev)
    {
        m_time = 0;
        m_end  = MAXT / m_len;
    }

    if(m_prev == 0)
        m_pos = 0;
    m_prev = m_len;

    if(m_pos < 2 * MAXT)
    {
        for(fpp_t f = 0; (f < _frames) && (m_pos < MAXT); ++f, ++m_pos)
        {
            m_buffer[m_pos][0] = _buf[f][0];
            m_buffer[m_pos][1] = _buf[f][1];
        }
    }

    real_t str = m_gdxControls.m_strModel.value();
    m_start    = lround(str * MAXT / m_len);
    m_end      = m_start + MAXT / m_len;

    // qInfo("MAXT=%d MINT=%d time=%d end=%d
    // pos=%d",MAXT,MINT,m_time,m_end,m_pos);

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        sample_t curVal0 = _buf[f][0];
        sample_t curVal1 = _buf[f][1];

        if(m_time >= m_end)
            m_time = m_start;
        if(m_time < m_start)
            m_time = m_end - 1;

        real_t s = m_gdxControls.m_spdModel.value();
        int    t = abs(s) * m_time;

        curVal0 = m_buffer[t][0];
        curVal1 = m_buffer[t][1];

        real_t o = m_gdxControls.m_ovrModel.value();
        if(o != 0.)
        {
            m_buffer[m_time][0] += o * _buf[f][0];
            m_buffer[m_time][1] += o * _buf[f][1];
        }

        if(s >= 0.)
            m_time++;
        else
            m_time--;

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
        return new ScatterGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
