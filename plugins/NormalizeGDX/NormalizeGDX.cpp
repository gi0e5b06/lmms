/*
 * NormalizeGDX.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include "NormalizeGDX.h"

#include "Configuration.h"
#include "WaveForm.h"
#include "embed.h"
//#include "lmms_math.h"

#include <cmath>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT normalizegdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "NormalizeGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A wave shaping plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

NormalizeGDX::NormalizeGDX(Model*                                    parent,
                           const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&normalizegdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_skew(0.), m_volume(1.), m_balance(0.)
{
}

NormalizeGDX::~NormalizeGDX()
{
}

bool NormalizeGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    const bool skewEnabled    = m_gdxControls.m_skewEnabledModel.value();
    const bool volumeEnabled  = m_gdxControls.m_volumeEnabledModel.value();
    const bool balanceEnabled = m_gdxControls.m_balanceEnabledModel.value();

    real_t skew    = 0.;
    real_t volume  = 0.;
    real_t balance = 0.;
    for(fpp_t f = 0; f < _frames; ++f)
    {
        const sample_t curVal0 = _buf[f][0];
        const sample_t curVal1 = _buf[f][1];

        skew += curVal0;
        volume = qMax(volume, abs(curVal0));
        balance -= abs(curVal0);

        skew += curVal1;
        volume = qMax(volume, abs(curVal1));
        balance += abs(curVal1);
    }

    skew /= 2. * real_t(_frames);
    // volume /= 2. * real_t(_frames);
    // balance /= 0.2 * real_t(_frames);
    balance = bound(-1., balance, 1.);

    if(skewEnabled)
    {
        const real_t skewSpeed = m_gdxControls.m_skewSpeedModel.value();
        m_skew                 = m_skew * (1. - skewSpeed) + skew * skewSpeed;
        skew                   = m_skew;
    }
    else
    {
        skew = 0.;
    }

    if(volumeEnabled)
    {
        if(volume < 1.)
        {
            const real_t volumeUpSpeed
                    = m_gdxControls.m_volumeUpSpeedModel.value();
            m_volume = m_volume * (1. - volumeUpSpeed)
                       + volume * volumeUpSpeed;
            if(m_volume <= 0.)
                volume = 1.;
            else
                volume = 1. / m_volume;
        }
        else
        {
            const real_t volumeDownSpeed
                    = m_gdxControls.m_volumeDownSpeedModel.value();
            m_volume = m_volume * (1. - volumeDownSpeed)
                       + volume * volumeDownSpeed;
            if(m_volume <= 0.)
                volume = 1.;
            else
                volume = 1. / m_volume;
        }
    }
    else
    {
        volume = 1.;
    }

    if(balanceEnabled)
    {
        const real_t balanceSpeed = m_gdxControls.m_balanceSpeedModel.value();
        m_balance = m_balance * (1. - balanceSpeed) + balance * balanceSpeed;
        balance   = m_balance;
    }
    else
    {
        balance = 0.;
    }

    const real_t outGain = m_gdxControls.m_outGainModel.value();

    // qInfo("skew=%f volume=%f balance=%f", skew, volume, balance);

    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        sample_t curVal0 = _buf[f][0];
        sample_t curVal1 = _buf[f][1];

        curVal0 = ((curVal0 * volume) - skew);
        curVal1 = ((curVal1 * volume) - skew);

        if(balance < 0.)
            curVal1 -= balance * curVal0;
        else if(balance > 0.)
            curVal0 += balance * curVal1;

        curVal0 = bound(-outGain, outGain * curVal0, outGain);
        curVal1 = bound(-outGain, outGain * curVal1, outGain);

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
    }

    return shouldKeepRunning(_buf, _frames);
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new NormalizeGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
