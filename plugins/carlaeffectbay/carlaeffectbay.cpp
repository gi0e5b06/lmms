/*
 * carlaeffectbay.cpp - Carla for LMMS (Effectbay)
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
 * Copyright (C) 2014 Filipe Coelho <falktx@falktx.com>
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

#include "CarlaEffect.h"

#include "embed.h"

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT carlaeffectbay_plugin_descriptor =
{
    STRINGIFY( PLUGIN_NAME ),
    "Carla Patchbay FX",
    QT_TRANSLATE_NOOP( "pluginBrowser",
                       "Carla Patchbay Effect" ),
    "gi0e5b06, falkTX",
    0x0195,
    Plugin::Effect,
    new PluginPixmapLoader( "logo.png" ),
    NULL,
    NULL
} ;

Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* _parent, void*)
{
    qInfo("carlaeffectbay::lmms_plugin_main");
    return new CarlaEffect(_parent, &carlaeffectbay_plugin_descriptor, true);
}

}
