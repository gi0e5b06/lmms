/*
 * SpaceGDX.cpp -
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
      m_ring(48000), m_leftLowFilter(m_sampleRate),
      m_rightLowFilter(m_sampleRate), m_leftHighFilter(m_sampleRate),
      m_rightHighFilter(m_sampleRate), m_prevLeftLowFreq(0.),
      m_prevRightLowFreq(0.), m_prevLeftHighFreq(0.), m_prevRightHighFreq(0.)
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
            = m_gdxControls.m_rightHighModel.valueBuffer();

    const ValueBuffer* leftPhaseBuf
            = m_gdxControls.m_leftPhaseModel.valueBuffer();
    const ValueBuffer* leftGainBuf
            = m_gdxControls.m_leftGainModel.valueBuffer();
    const ValueBuffer* leftLowBuf
            = m_gdxControls.m_leftLowModel.valueBuffer();
    const ValueBuffer* leftHighBuf
            = m_gdxControls.m_leftHighModel.valueBuffer();

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        real_t rightPhase = rightPhaseBuf != nullptr
                                    ? rightPhaseBuf->value(f)
                                    : m_gdxControls.m_rightPhaseModel.value();
        real_t rightGain = rightGainBuf != nullptr
                                   ? rightGainBuf->value(f)
                                   : m_gdxControls.m_rightGainModel.value();
        real_t rightLow = rightLowBuf != nullptr
                                  ? rightLowBuf->value(f)
                                  : m_gdxControls.m_rightLowModel.value();
        real_t rightHigh = rightHighBuf != nullptr
                                   ? rightHighBuf->value(f)
                                   : m_gdxControls.m_rightHighModel.value();

        real_t leftPhase = leftPhaseBuf != nullptr
                                   ? leftPhaseBuf->value(f)
                                   : m_gdxControls.m_leftPhaseModel.value();
        real_t leftGain = leftGainBuf != nullptr
                                  ? leftGainBuf->value(f)
                                  : m_gdxControls.m_leftGainModel.value();
        real_t leftLow = leftLowBuf != nullptr
                                 ? leftLowBuf->value(f)
                                 : m_gdxControls.m_leftLowModel.value();
        real_t leftHigh = leftHighBuf != nullptr
                                  ? leftHighBuf->value(f)
                                  : m_gdxControls.m_leftHighModel.value();

        int g[4][2];
        g[0][0] = -_frames + 1 + f;
        g[1][0] = g[0][0] - round(leftPhase * 5000.);
        g[2][0] = g[1][0] - round(rightPhase * 5000.);
        g[3][0] = g[2][0] - round(leftPhase * 5000.);

        g[0][1] = -_frames + 1 + f;
        g[1][1] = g[0][1] - round(rightPhase * 5000.);
        g[2][1] = g[1][1] - round(leftPhase * 5000.);
        g[3][1] = g[2][1] - round(rightPhase * 5000.);

        sample_t v[4][2];  //={{0.,0.},{0.,0.},{0.,0.}};

        sample_t curVal[2] = {0., 0.};

        for(int s = 0; s < 4; s++)
            for(int ch = 0; ch < 2; ch++)
            {
                int i   = (s + ch) % 2;
                v[s][i] = m_ring.value(g[s][i], 0);
                if(v[s][i] <= SILENCE)
                    v[s][i] = 0.;
                curVal[i] += v[s][i] / (s + 1);
            }

        // LEFT EAR

        curVal[0] *= leftGain;

        const frequency_t leftLowFreq
                = round(5000. + 115000. * leftLow) / 1000.;
        if(leftLowFreq != m_prevLeftLowFreq)
        {
            qInfo("Left Low: %f Hz", leftLowFreq);
            m_prevLeftLowFreq = leftLowFreq;
            m_leftLowFilter.calcFilterCoeffs(leftLowFreq, 0.05);
        }
        curVal[0] = m_leftLowFilter.update(curVal[0], 0);

        const frequency_t leftHighFreq = qMin<real_t>(
                m_sampleRate / 2., round(22050. - 11025. * leftHigh));
        if(leftHighFreq != m_prevLeftHighFreq)
        {
            qInfo("Left High: %f Hz", leftHighFreq);
            m_prevLeftHighFreq = leftHighFreq;
            m_leftHighFilter.calcFilterCoeffs(leftHighFreq, 0.05);
        }
        curVal[0] = m_leftHighFilter.update(curVal[0], 0);

        // RIGHT EAR

        curVal[1] *= rightGain;

        const frequency_t rightLowFreq
                = round(5000. + 115000. * rightLow) / 1000.;
        if(rightLowFreq != m_prevRightLowFreq)
        {
            qInfo("Right Low: %f Hz", rightLowFreq);
            m_prevRightLowFreq = rightLowFreq;
            m_rightLowFilter.calcFilterCoeffs(rightLowFreq, 0.05);
        }
        curVal[1] = m_rightLowFilter.update(curVal[1], 0);

        const frequency_t rightHighFreq = qMin<frequency_t>(
                m_sampleRate / 2., round(22050. - 11025. * rightHigh));
        if(rightHighFreq != m_prevRightHighFreq)
        {
            qInfo("Right High: %f Hz", rightHighFreq);
            m_prevRightHighFreq = rightHighFreq;
            m_rightHighFilter.calcFilterCoeffs(rightHighFreq, 0.05);
        }
        curVal[1] = m_rightHighFilter.update(curVal[1], 0);

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal[0];
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal[1];
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
