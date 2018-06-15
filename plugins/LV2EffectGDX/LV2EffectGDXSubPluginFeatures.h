/*
 * LV2EffectGDXSubPluginFeatures.h - derivation from
 *                             Plugin::Descriptor::SubPluginFeatures for
 *                             hosting LV2-plugins
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

#ifndef LV2_EFFECT_GDX_SUBPLUGIN_FEATURES_H
#define LV2_EFFECT_GDX_SUBPLUGIN_FEATURES_H

#include "LV2Manager.h"
#include "Plugin.h"


class LV2EffectGDXSubPluginFeatures : public Plugin::Descriptor::SubPluginFeatures
{
public:
	LV2EffectGDXSubPluginFeatures(Plugin::PluginTypes _type);

	virtual void fillDescriptionWidget(QWidget* _parent,
                                           const Key* _key) const;

	virtual void listSubPluginKeys(const Plugin::Descriptor* _desc,
                                       KeyList& _kl) const;

	static lv2_key_t subPluginKeyToLV2Key(const Key* _key);

} ;

#endif
