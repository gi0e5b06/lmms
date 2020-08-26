/*
 * Inspector.cpp - Dynamic Qt object inspector/editor for LMMS
 *
 * Copyright (c) 2020 Dominic Clark <mrdomclark/at/gmail.com>
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

#include "Inspector.h"

#include "embed.h"
#include "export.h"

#include "InspectorView.h"

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT inspector_plugin_descriptor{
	STRINGIFY(PLUGIN_NAME),
	"Inspector",
	QT_TRANSLATE_NOOP("Inspector", "Inspect and modify the widget hierarchy"),
	"Dominic Clark <mrdomclark/at/gmail.com>",
	0x0001,
	Plugin::Tool,
	new PluginPixmapLoader{"logo"},
	nullptr,
	nullptr
};

PLUGIN_EXPORT Plugin *lmms_plugin_main(Model *parent, void *data)
{
	return new Inspector{parent};
}

} // extern "C"

Inspector::Inspector(Model *parent) :
	ToolPlugin{&inspector_plugin_descriptor, parent}
{ }

PluginView *Inspector::instantiateView(QWidget *parent)
{
	return new InspectorView{this};
}

QString Inspector::nodeName() const
{
	return "inspector";
}

void Inspector::saveSettings(QDomDocument &doc, QDomElement &elem)
{ }

void Inspector::loadSettings(const QDomElement &elem)
{ }
