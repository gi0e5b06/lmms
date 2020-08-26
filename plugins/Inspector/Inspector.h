/*
 * Inspector.h - Dynamic Qt object inspector/editor for LMMS
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

#ifndef INSPECTOR_H
#define INSPECTOR_H

#include "ToolPlugin.h"

class Inspector : public ToolPlugin
{
	Q_OBJECT
public:
	explicit Inspector(Model *parent);

	PluginView *instantiateView(QWidget *parent) override;
	QString nodeName() const override;
	void saveSettings(QDomDocument &doc, QDomElement &elem) override;
	void loadSettings(const QDomElement &elem) override;
};

#endif // INSPECTOR_H
