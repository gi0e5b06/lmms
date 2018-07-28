/*
 * ChainGDXControls.cpp - controls for chaining effect
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

#include "ChainGDXControls.h"

#include <QDomElement>

#include "ChainGDX.h"
#include "Engine.h"
#include "Song.h"

ChainGDXControls::ChainGDXControls(ChainGDXEffect* effect) :
      EffectControls(effect), m_effect(effect)
{
}

void ChainGDXControls::changeControl()
{
    //	engine::getSong()->setModified();
}

void ChainGDXControls::loadSettings(const QDomElement& _this)
{
    m_effect->m_chain->loadSettings(_this);
}

void ChainGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_effect->m_chain->saveSettings(doc, _this);
}
