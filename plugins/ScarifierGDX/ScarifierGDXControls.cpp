/*
 * ScarifierGDXControls.cpp -
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ScarifierGDXControls.h"

#include "ScarifierGDX.h"

#include <QDomElement>

ScarifierGDXControls::ScarifierGDXControls(ScarifierGDX* effect) :
      EffectControls(effect), m_effect(effect), m_modeModel(this, tr("Mode")),
      m_widthModel(1., 0., 5., 0.01, this, tr("Width")),
      m_keyModel(9., 0., 11., 1., this, tr("Key")),
      m_ampModel(1., 0., 10., 0.01, this, tr("Out gain"))
{
    m_modeModel.addItem("Scar");
    m_modeModel.addItem("Select");
    m_modeModel.addItem("Boost");
    m_keyModel.setStrictStepSize(true);
}

ScarifierGDXControls::~ScarifierGDXControls()
{
}

void ScarifierGDXControls::changeControl()
{
}

void ScarifierGDXControls::loadSettings(const QDomElement& _this)
{
    m_modeModel.loadSettings(_this, "mode");
    m_widthModel.loadSettings(_this, "width");
    m_keyModel.loadSettings(_this, "key");
    m_ampModel.loadSettings(_this, "amp");
}

void ScarifierGDXControls::saveSettings(QDomDocument& _doc,
                                        QDomElement&  _this)
{
    m_modeModel.saveSettings(_doc, _this, "mode");
    m_widthModel.saveSettings(_doc, _this, "width");
    m_keyModel.saveSettings(_doc, _this, "key");
    m_ampModel.saveSettings(_doc, _this, "amp");
}
