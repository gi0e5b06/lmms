/*
 * FrequencyGDXControls.cpp -
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

#include "FrequencyGDXControls.h"

#include "FrequencyGDX.h"

#include <QDomElement>

FrequencyGDXControls::FrequencyGDXControls(FrequencyGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_topFrequencyModel(0.f, 0.f, 22050.f, 1.f, this, tr("Top frequency")),
      m_avgFrequencyModel(
              0.f, 0.f, 22050.f, 1.f, this, tr("Average frequency")),
      m_mainFrequencyModel(
              0.f, 0.f, 22050.f, 1.f, this, tr("Main frequency")),
      m_topKeyModel(-1.f, -1.f, 128.f, 1.f, this, tr("Top key")),
      m_avgKeyModel(-1.f, -1.f, 128.f, 1.f, this, tr("Average key")),
      m_mainKeyModel(-1.f, -1.f, 128.f, 1.f, this, tr("Main key")),
      m_topNoteModel(-1.f, -1.f, 11.f, 1.f, this, tr("Top note")),
      m_avgNoteModel(-1.f, -1.f, 11.f, 1.f, this, tr("Average note")),
      m_mainNoteModel(-1.f, -1.f, 11.f, 1.f, this, tr("Main note"))
{
}

FrequencyGDXControls::~FrequencyGDXControls()
{
}

void FrequencyGDXControls::changeControl()
{
}

void FrequencyGDXControls::loadSettings(const QDomElement& _this)
{
}

void FrequencyGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
}
