/*
 * InputGDX.cpp - merging audio input
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

#include "InputGDX.h"

#include "ValueBuffer.h"
#include "WaveForm.h"
#include "embed.h"

#include <math.h>
//#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT inputgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "InputGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A merging audio plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

InputGDX::InputGDX(Model*                                    parent,
                   const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&inputgdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_curVol(0.), m_curBal(0.)
{
}

InputGDX::~InputGDX()
{
}

bool InputGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    const ValueBuffer* leftSignalBuf
            = m_gdxControls.m_leftSignalModel.valueBuffer();
    const ValueBuffer* rightSignalBuf
            = m_gdxControls.m_rightSignalModel.valueBuffer();
    const ValueBuffer* volumeBuf = m_gdxControls.m_volumeModel.valueBuffer();
    const ValueBuffer* balanceBuf
            = m_gdxControls.m_balanceModel.valueBuffer();
    const ValueBuffer* mixingBuf = m_gdxControls.m_mixingModel.valueBuffer();
    const ValueBuffer* deltaBuf  = m_gdxControls.m_deltaModel.valueBuffer();

    if(leftSignalBuf == nullptr)
        m_gdxControls.m_leftSignalModel.setControlledValue(0.);
    if(rightSignalBuf == nullptr)
        m_gdxControls.m_rightSignalModel.setControlledValue(0.);

    if(leftSignalBuf == nullptr)
        qInfo("Left SIGNAL is null");
    if(rightSignalBuf == nullptr)
        qInfo("Right SIGNAL is null");

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        // qInfo("WDWD %f %f %f %f", w0, d0, w1, d1);

        const real_t mixing = mixingBuf ? mixingBuf->value(f)
                                        : m_gdxControls.m_mixingModel.value();
        const real_t delta = deltaBuf ? deltaBuf->value(f)
                                      : m_gdxControls.m_deltaModel.value();

        sample_t curVal0 = mixing * _buf[f][0];
        sample_t curVal1 = mixing * _buf[f][1];

        if(leftSignalBuf != nullptr)
            curVal0 += (1. - mixing) * leftSignalBuf->value(f);
        if(rightSignalBuf != nullptr)
            curVal1 += (1. - mixing) * rightSignalBuf->value(f);

        real_t vol = volumeBuf ? volumeBuf->value(f)
                               : m_gdxControls.m_volumeModel.value();
        if(delta < 1.)
            vol = m_curVol = (1. - delta) * m_curVol + delta * vol;
        curVal0 *= vol;
        curVal1 *= vol;

        real_t bal = balanceBuf ? balanceBuf->value(f)
                                : m_gdxControls.m_balanceModel.value();
        if(delta < 1.)
            bal = m_curBal = (1. - delta) * m_curBal + delta * bal;
        const real_t balLeft  = (bal >= 0. ? 1. - bal : 1.);
        const real_t balRight = (bal >= 0. ? 1. : 1. - bal);
        curVal0 *= balLeft;
        curVal1 *= balRight;

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
        return new InputGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
