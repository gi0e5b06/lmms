/*
 * VocoderGDXControls.cpp -
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

#include "VocoderGDXControls.h"

#include "VocoderGDX.h"

#include <QDomElement>

VocoderGDXControls::VocoderGDXControls(VocoderGDX* effect) :
        EffectControls(effect), m_effect(effect), m_modeModel(this,tr("Mode")),
      m_widthModel(0.15f, 0.f, 1.f, 0.01f, this, tr("Width")),
      m_keyModel(9.f, 0.f, 11.f, 1.f, this, tr("Key")),
      m_ampModel(1.f, 0.f, 10.f, 0.01f, this, tr("Out gain"))
{
        m_modeModel.addItem("Scar");
        m_modeModel.addItem("Select");
        m_modeModel.addItem("Boost");
        m_keyModel.setStrictStepSize(true);
}

VocoderGDXControls::~VocoderGDXControls()
{
}

void VocoderGDXControls::changeControl()
{
}

void VocoderGDXControls::loadSettings(const QDomElement& _this)
{
    m_modeModel.loadSettings(_this, "mode");
    m_widthModel.loadSettings(_this, "width");
    m_keyModel.loadSettings(_this, "key");
    m_ampModel.loadSettings(_this, "amp");
}

void VocoderGDXControls::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    m_modeModel.saveSettings(_doc, _this, "mode");
    m_widthModel.saveSettings(_doc, _this, "width");
    m_keyModel.saveSettings(_doc, _this, "key");
    m_ampModel.saveSettings(_doc, _this, "amp");
}
