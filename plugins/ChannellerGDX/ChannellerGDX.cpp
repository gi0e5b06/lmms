/*
 * ChannellerGDX.cpp -
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

#include "ChannellerGDX.h"

//#include "WaveForm.h"
#include "embed.h"
//#include "lmms_math.h"
#include "denormals.h"

#include <cmath>
#include <complex>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT channellergdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "ChannellerGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A channelling plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

ChannellerGDX::ChannellerGDX(Model*                                    parent,
                             const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&channellergdx_plugin_descriptor, parent, key),
      m_gdxControls(this)
{
}

ChannellerGDX::~ChannellerGDX()
{
}

bool ChannellerGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    const int operation = m_gdxControls.m_operationModel.value();
    for(fpp_t f = 0; f < _frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, _frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        sample_t curVal0 = _buf[f][0];
        sample_t curVal1 = _buf[f][1];

        switch(operation)
        {
            case 0:
                curVal0 = curVal1 = 0.;
                break;
            case 1:
                break;
            case 2:
            {
                const sample_t tmpVal = curVal0;
                curVal0               = curVal1;
                curVal1               = tmpVal;
            }
            break;
            case 3:
                curVal1 = 0.;
                break;
            case 4:
                curVal0 = 0.;
                break;
            case 5:
            {
                const sample_t tmpVal = 0.5 * (curVal0 + curVal1);
                curVal0 = curVal1 = tmpVal;
            }
            break;
            case 6:
            {
                const sample_t sides = 0.5 * (curVal0 - curVal1);
                const sample_t mid   = 0.5 * (curVal0 + curVal1);
                curVal0              = mid;
                curVal1              = sides;
            }
            break;
            case 7:
            {
                const sample_t mid   = curVal0;
                const sample_t sides = curVal1;
                curVal0              = mid + sides;
                curVal1              = mid - sides;
            }
            break;
            case 8:
            {
                const complex<real_t> tmpVal(curVal0, curVal1);
                curVal0 = sqrt(std::norm<real_t>(tmpVal / 2.));
                curVal1 = std::arg<real_t>(tmpVal) / R_2PI;
            }
            break;
            case 9:
            {
                const complex<real_t> tmpVal
                        = std::polar<real_t>(curVal0 * 2., curVal1 * R_2PI);
                curVal0 = std::real<real_t>(tmpVal);
                curVal1 = std::imag<real_t>(tmpVal);
                if(abs(curVal0)<=SILENCE) curVal0=0.;
                if(abs(curVal1)<=SILENCE) curVal1=0.;
            }
            break;
        }

        curVal0 = bound(-1., curVal0, 1.);
        curVal1 = bound(-1., curVal1, 1.);

        _buf[f][0] = d0 * _buf[f][0] + w0 * curVal0;
        _buf[f][1] = d1 * _buf[f][1] + w1 * curVal1;
    }

    bool r = shouldKeepRunning(_buf, _frames);
    return r;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new ChannellerGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
