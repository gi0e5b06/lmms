/*
 * OscilloscopeGDX.cpp -
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

#include "OscilloscopeGDX.h"

#include "Engine.h"
#include "Mixer.h"
#include "embed.h"
//#include "lmms_math.h"

#include <cmath>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT oscilloscopegdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "OscilloscopeGDX",
               QT_TRANSLATE_NOOP("pluginBrowser", "A wave shaping plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

OscilloscopeGDX::OscilloscopeGDX(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&oscilloscopegdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_fpp(Engine::mixer()->framesPerPeriod())
{
    setColor(QColor(59, 66, 160));

    int periodsPerDisplayRefresh = qMax(
            1, int(ceilf(float(Engine::mixer()->processingSampleRate())
                         / m_fpp / CONFIG_GET_INT("ui.framespersecond"))));
    m_ring = new Ring(m_fpp * periodsPerDisplayRefresh);
}

OscilloscopeGDX::~OscilloscopeGDX()
{
    delete m_ring;
}

bool OscilloscopeGDX::processAudioBuffer(sampleFrame* _buf,
                                         const fpp_t  _frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
        return false;

    m_ring->write(_buf, _frames);

    return true;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new OscilloscopeGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
