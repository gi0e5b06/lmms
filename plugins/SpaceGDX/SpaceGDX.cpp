/*
 * SpaceGDX.cpp - wall effect
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

#include "SpaceGDX.h"

#include "MixHelpers.h"
#include "Ring.h"
#include "embed.h"
#include "lmms_math.h"

#include <cmath>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT spacegdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "SpaceGDX",
               QT_TRANSLATE_NOOP("pluginBrowser",
                                 "A spacializing effect plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

SpaceGDXEffect::SpaceGDXEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&spacegdx_plugin_descriptor, parent, key),
      m_gdxControls(this),
      m_sampleRate(Engine::mixer()->processingSampleRate()),
      m_ring(m_sampleRate), m_leftLowFilter(m_sampleRate),
      m_rightLowFilter(m_sampleRate), m_leftHighFilter(m_sampleRate),
      m_rightHighFilter(m_sampleRate), m_prevLeftLowFreq(0.f),
      m_prevRightLowFreq(0.f), m_prevLeftHighFreq(0.f),
      m_prevRightHighFreq(0.f)
{
    // TODO: move that to startRunning()
    // m_len    = Engine::mixer()->processingSampleRate();
    // m_buffer = MM_ALLOC(sampleFrame, m_len);
    // memset(m_buffer, 0, m_len * sizeof(sampleFrame));

    m_leftLowFilter.setFilterType(BasicFilters<1>::HiPass);
    m_leftHighFilter.setFilterType(BasicFilters<1>::LowPass);
    m_leftLowFilter.clearHistory();
    m_leftHighFilter.clearHistory();

    m_rightLowFilter.setFilterType(BasicFilters<1>::HiPass);
    m_rightHighFilter.setFilterType(BasicFilters<1>::LowPass);
    m_rightLowFilter.clearHistory();
    m_rightHighFilter.clearHistory();
}

SpaceGDXEffect::~SpaceGDXEffect()
{
    // m_len = 0;
    // MM_FREE(m_buffer);
    // m_buffer = NULL;
}

bool SpaceGDXEffect::processAudioBuffer(sampleFrame* _buf,
                                        const fpp_t  _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    m_ring.write(_buf, _frames);

    const ValueBuffer* rightPhaseBuf
            = m_gdxControls.m_rightPhaseModel.valueBuffer();
    const ValueBuffer* rightGainBuf
            = m_gdxControls.m_rightGainModel.valueBuffer();
    const ValueBuffer* rightLowBuf
            = m_gdxControls.m_rightLowModel.valueBuffer();
    const ValueBuffer* rightHighBuf
            = m_gdxControls.m_rightLowModel.valueBuffer();

    for(fpp_t f = 0; f < _frames; ++f)
    {
        float w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        float rightPhase = rightPhaseBuf
                                   ? rightPhaseBuf->value(f)
                                   : m_gdxControls.m_rightPhaseModel.value();
        float rightGain = rightGainBuf
                                  ? rightGainBuf->value(f)
                                  : m_gdxControls.m_rightGainModel.value();
        float rightLow = rightLowBuf ? rightLowBuf->value(f)
                                     : m_gdxControls.m_rightLowModel.value();
        float rightHigh = rightHighBuf
                                  ? rightHighBuf->value(f)
                                  : m_gdxControls.m_rightHighModel.value();

        float leftPhase = 1.f - rightPhase;
        float leftGain  = 1.f - rightGain;
        float leftLow   = rightLow;
        float leftHigh  = rightHigh;

        int f0 = -_frames + 1 + f - roundf(leftPhase * 200.f);
        int f1 = -_frames + 1 + f - roundf(rightPhase * 200.f);

        sample_t v00 = m_ring.value(f0, 0);
        sample_t v01 = m_ring.value(f0, 1);
        sample_t v10 = m_ring.value(f1, 0);
        sample_t v11 = m_ring.value(f1, 1);

        sample_t curVal0 = v00 + v01;
        sample_t curVal1 = v10 + v11;

        if(fabsf(curVal0)>=SILENCE)
            curVal0 /= (v00 * v00 + v01 * v01);
        if(fabsf(curVal1)>=SILENCE)
            curVal1 /= (v10 * v10 + v11 * v11);

        // LEFT EAR

        curVal0 *= leftGain;

        const float leftLowFreq
                = roundf(5000.f + 115000.f * leftLow) / 1000.f;
        if(leftLowFreq != m_prevLeftLowFreq)
        {
            qInfo("Left Low: %f Hz", leftLowFreq);
            m_prevLeftLowFreq = leftLowFreq;
            m_leftLowFilter.calcFilterCoeffs(leftLowFreq, 0.05f);
        }
        curVal0 = m_leftLowFilter.update(curVal0, 0);

        const float leftHighFreq = qMin<float>(
                m_sampleRate / 2.f, roundf(22050.f - 11025.f * leftHigh));
        if(leftHighFreq != m_prevLeftHighFreq)
        {
            qInfo("Left High: %f Hz", leftHighFreq);
            m_prevLeftHighFreq = leftHighFreq;
            m_leftHighFilter.calcFilterCoeffs(leftHighFreq, 0.05f);
        }
        curVal0 = m_leftHighFilter.update(curVal0, 0);

        // RIGHT EAR

        curVal1 *= rightGain;

        const float rightLowFreq
                = roundf(5000.f + 115000.f * rightLow) / 1000.f;
        if(rightLowFreq != m_prevRightLowFreq)
        {
            qInfo("Right Low: %f Hz", rightLowFreq);
            m_prevRightLowFreq = rightLowFreq;
            m_rightLowFilter.calcFilterCoeffs(rightLowFreq, 0.05f);
        }
        curVal1 = m_rightLowFilter.update(curVal1, 0);

        const float rightHighFreq = qMin<float>(
                m_sampleRate / 2.f, roundf(22050.f - 11025.f * rightHigh));
        if(rightHighFreq != m_prevRightHighFreq)
        {
            qInfo("Right High: %f Hz", rightHighFreq);
            m_prevRightHighFreq = rightHighFreq;
            m_rightHighFilter.calcFilterCoeffs(rightHighFreq, 0.05f);
        }
        curVal1 = m_rightHighFilter.update(curVal1, 0);

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal1;  // 1
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
    }

    return true;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new SpaceGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
