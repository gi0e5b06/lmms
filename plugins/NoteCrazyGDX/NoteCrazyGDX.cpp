/*
 * NoteCrazyGDX.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include "NoteCrazyGDX.h"

#include "embed.h"

extern "C"
{
    Plugin::Descriptor PLUGIN_EXPORT notecrazygdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "NoteCrazyGDX",
               QT_TRANSLATE_NOOP(
                       "pluginBrowser",
                       "The first instrument-function, note-effect plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::NoteEffect,
               new PluginPixmapLoader("logo"),
               nullptr,
               nullptr};
}

NoteCrazyGDX::NoteCrazyGDX(Model*                                    _parent,
                           const Descriptor::SubPluginFeatures::Key* _key) :
      Plugin(&notecrazygdx_plugin_descriptor, _parent),
      InstrumentFunction(_parent,
                         notecrazygdx_plugin_descriptor.displayName())
{
}

NoteCrazyGDX::~NoteCrazyGDX()
{
}

bool NoteCrazyGDX::processNote(NotePlayHandle* n)
{
    return false;
}

void NoteCrazyGDX::saveSettings(QDomDocument& _doc, QDomElement& _parent)
{
}

void NoteCrazyGDX::loadSettings(const QDomElement& _this)
{
}

QString NoteCrazyGDX::nodeName() const
{
    // const Descriptor* d = descriptor();
    // return (d == nullptr) ? "dummynoteeffect" : d->name();
    return descriptor()->name();
}

InstrumentFunctionView* NoteCrazyGDX::createView()
{
    return nullptr;
}

PluginView* NoteCrazyGDX::instantiateView(QWidget*)
{
    return nullptr;
}

bool NoteCrazyGDX::shouldProcessNote(NotePlayHandle* n)
{
    return false;
}

extern "C"
{
    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* _parent, void* _data)
    {
        return new NoteCrazyGDX(
                _parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        _data));
    }
}
