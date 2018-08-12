/*
 * OutputGDX.cpp - audio output properties
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

#include "OutputGDX.h"

#include <math.h>

#include "ValueBuffer.h"
#include "WaveForm.h"
#include "embed.h"
//#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT outputgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "OutputGDX",
               QT_TRANSLATE_NOOP("pluginBrowser",
                                 "A audio output properties plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

OutputGDX::OutputGDX(Model*                                    parent,
                     const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&outputgdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_left(Engine::mixer()->framesPerPeriod()),
      m_right(Engine::mixer()->framesPerPeriod())
{
    const fpp_t FPP = Engine::mixer()->framesPerPeriod();
    m_left          = new ValueBuffer(FPP);
    m_right         = new ValueBuffer(FPP);

    connect(this, SIGNAL(sendRms(const float)), &m_gdxControls.m_rmsModel,
            SLOT(setAutomatedValue(const float)));
    connect(this, SIGNAL(sendVol(const float)), &m_gdxControls.m_volModel,
            SLOT(setAutomatedValue(const float)));
    connect(this, SIGNAL(sendPan(const float)), &m_gdxControls.m_panModel,
            SLOT(setAutomatedValue(const float)));
    connect(this, SIGNAL(sendLeft(const ValueBuffer*)),
            &m_gdxControls.m_leftModel, SLOT(copyFrom(const ValueBuffer*)));
    connect(this, SIGNAL(sendRight(const ValueBuffer*)),
            &m_gdxControls.m_rightModel, SLOT(copyFrom(const ValueBuffer*)));
    connect(this, SIGNAL(sendFrequency(const float)),
            &m_gdxControls.m_frequencyModel,
            SLOT(setAutomatedValue(const float)));
}

OutputGDX::~OutputGDX()
{
    // delete m_left;
    // delete m_right;
}

bool OutputGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    if(m_left.length() < _frames || m_right.length() < _frames)
    {
        qWarning("OutputGDX::processAudioBuffer");
        return false;
    }
    // float rms = computeRMS(_buf, _frames);
    // sendRms(rms);

    // qInfo("OutputGDX::processAudioBuffer rms=%f scl=%f val=%f", rms,
    //      m_rmsModel.scaledValue(rms), m_rmsModel.value());
    float rms[2] = {0.f, 0.f};
    float max[2] = {0.f, 0.f};

    for(fpp_t f = 0; f < _frames; ++f)
    {
        float curVal0 = _buf[f][0];
        float curVal1 = _buf[f][1];

        m_left.set(f, curVal0);
        m_right.set(f, curVal1);
        rms[0] += curVal0 * curVal0;
        rms[1] += curVal1 * curVal1;
        max[0] = qMax(max[0], fabsf(curVal0));
        max[1] = qMax(max[1], fabsf(curVal1));
    }

    sendLeft(&m_left);
    sendRight(&m_right);
    sendRms(WaveForm::sqrt(qBound(0.f, rms[0] + rms[1], 1.f)));
    sendVol(qMax(max[0], max[1]));
    sendPan(WaveForm::sqrt(qBound(0.f, rms[1], 1.f))
            - WaveForm::sqrt(qBound(0.f, rms[0], 1.f)));

    // sendFrequency(440.f);
    // qInfo("OutputGDX::processAudioBuffer rms=%f",rms);

    return true;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new OutputGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
