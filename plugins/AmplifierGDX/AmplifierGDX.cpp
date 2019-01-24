/*
 * AmplifierGDX.cpp -
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

#include "AmplifierGDX.h"

#include "embed.h"
#include "lmms_math.h"

#include <QTimer>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT amplifiergdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "AmplifierGDX",
               QT_TRANSLATE_NOOP("pluginBrowser",
                                 "A native amplifierGDX plugin"),
               "Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

AmplifierGDXEffect::AmplifierGDXEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&amplifiergdx_plugin_descriptor, parent, key),
      m_ampControls(this)
{
    setColor(QColor(160, 99, 111));
}

AmplifierGDXEffect::~AmplifierGDXEffect()
{
}

bool AmplifierGDXEffect::processAudioBuffer(sampleFrame* buf,
                                            const fpp_t  frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(buf, frames, smoothBegin, smoothEnd))
        return false;

    const ValueBuffer* volumeBuf = m_ampControls.m_volumeModel.valueBuffer();
    const ValueBuffer* balanceBuf
            = m_ampControls.m_balanceModel.valueBuffer();
    const ValueBuffer* leftVolumeBuf
            = m_ampControls.m_leftVolumeModel.valueBuffer();
    const ValueBuffer* rightVolumeBuf
            = m_ampControls.m_rightVolumeModel.valueBuffer();
    const ValueBuffer* widthBuf = m_ampControls.m_widthModel.valueBuffer();
    const ValueBuffer* leftPanningBuf
            = m_ampControls.m_leftPanningModel.valueBuffer();
    const ValueBuffer* rightPanningBuf
            = m_ampControls.m_rightPanningModel.valueBuffer();

    for(fpp_t f = 0; f < frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        // qDebug( "offset %d, value %f", f,
        // m_ampControls.m_volumeModel.value( f ) );
        sample_t s[2] = {buf[f][0], buf[f][1]};

        // 1st stage: vol+bal
        real_t vol = (volumeBuf ? volumeBuf->value(f)
                                : m_ampControls.m_volumeModel.value())
                     * 0.01;
        if(vol <= SILENCE)
            vol = 0.;

        const real_t balance = balanceBuf
                                       ? balanceBuf->value(f)
                                       : m_ampControls.m_balanceModel.value();
        const real_t left1  = balance <= 0. ? 1. : 1. - balance * 0.01;
        const real_t right1 = balance >= 0. ? 1. : 1. + balance * 0.01;

        // 2nd stage: L+R vol
        const real_t left2
                = (leftVolumeBuf ? leftVolumeBuf->value(f)
                                 : m_ampControls.m_leftVolumeModel.value())
                  * 0.01;
        const real_t right2
                = (rightVolumeBuf ? rightVolumeBuf->value(f)
                                  : m_ampControls.m_rightVolumeModel.value())
                  * 0.01;

        real_t amp0 = vol * left1 * left2;
        real_t amp1 = vol * right1 * right2;

        if(amp0 <= SILENCE)
            s[0] = amp0 = 0.;
        else
            s[0] *= amp0;

        if(amp1 <= SILENCE)
            s[1] = amp1 = 0.;
        else
            s[1] *= amp1;

        // 3rd stage: width
        real_t width = (widthBuf ? widthBuf->value(f)
                                 : m_ampControls.m_widthModel.value())
                       * 0.01;

        if(width < 0.)
        {
            const real_t tmp = s[0];

            s[0]  = s[1];
            s[1]  = tmp;
            width = -width;
        }

        if(width < 1.)
        {
            const real_t tmp = (s[0] + s[1]) * 0.5;

            s[0] = width * s[0] + (1. - width) * tmp;
            s[1] = width * s[1] + (1. - width) * tmp;
        }

        // 4th stage: L+R pan

        const real_t left3
                = (leftPanningBuf ? leftPanningBuf->value(f)
                                  : m_ampControls.m_leftPanningModel.value())
                  * 0.01;
        const real_t right3
                = (rightPanningBuf
                           ? rightPanningBuf->value(f)
                           : m_ampControls.m_rightPanningModel.value())
                  * 0.01;
        if(left3 != -1. || right3 != 1.)
        {
            const real_t tmp0 = s[0];
            const real_t tmp1 = s[1];

            s[0] = (1. - left3) / 2. * tmp0 + (1. + left3) / 2. * tmp1;
            s[1] = (1. - right3) / 2. * tmp0 + (1. + right3) / 2. * tmp1;
        }

        buf[f][0] = d0 * buf[f][0] + w0 * s[0];
        buf[f][1] = d1 * buf[f][1] + w1 * s[1];
    }

    bool r = shouldKeepRunning(buf, frames, true);
    if(r && volumeBuf == nullptr && isClipping())
    {
        setClipping(false);
        m_ampControls.m_volumeModel.setAutomatedValue(
          m_ampControls.m_volumeModel.rawValue() * 0.995);
    }
    return r;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new AmplifierGDXEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
