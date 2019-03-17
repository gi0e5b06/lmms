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
      EffectControls(effect), m_effect(effect), m_modeModel(this, tr("Mode")),
      m_topFrequencyModel(0., 0., 22050., 1., this, tr("Top frequency")),
      m_avgFrequencyModel(0., 0., 22050., 1., this, tr("Average frequency")),
      m_mainFrequencyModel(0., 0., 22050., 1., this, tr("Main frequency")),
      m_topKeyModel(-1., -1., 128., 1., this, tr("Top key")),
      m_avgKeyModel(-1., -1., 128., 1., this, tr("Average key")),
      m_mainKeyModel(-1., -1., 128., 1., this, tr("Main key")),
      m_topNoteModel(-1., -1., 11., 1., this, tr("Top note")),
      m_avgNoteModel(-1., -1., 11., 1., this, tr("Average note")),
      m_mainNoteModel(-1., -1., 11., 1., this, tr("Main note"))
{
    m_modeModel.addItem(tr("Simple"));
    m_modeModel.addItem(tr("Goertzel"));
    m_modeModel.addItem(tr("FFT"));
}

FrequencyGDXControls::~FrequencyGDXControls()
{
}

void FrequencyGDXControls::changeControl()
{
}

void FrequencyGDXControls::loadSettings(const QDomElement& _this)
{
    // TODO: mode
}

void FrequencyGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    // TODO: mode
}
