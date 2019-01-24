/*
 * NormalizeGDXControls.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include "NormalizeGDXControls.h"

//#include "Knob.h"
#include "NormalizeGDX.h"
//#include "WaveForm.h"
//#include "Engine.h"
//#include "Song.h"
//#include "BufferManager.h"

#include <QDomElement>

NormalizeGDXControls::NormalizeGDXControls(NormalizeGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_skewEnabledModel(true, this, tr("Skew active")),
      m_volumeEnabledModel(true, this, tr("Volume active")),
      m_balanceEnabledModel(true, this, tr("Balance active")),
      m_skewSpeedModel(0.005, 0., 1., 0.001, this, tr("Skew speed")),
      m_volumeUpSpeedModel(0.005, 0., 1., 0.001, this, tr("Volume up speed")),
      m_volumeDownSpeedModel(
              0.005, 0., 1., 0.001, this, tr("Volume down speed")),
      m_balanceSpeedModel(0.005, 0., 1., 0.001, this, tr("Balance speed")),
      m_outGainModel(0.8, 0., 1., 0.001, this, tr("Out gain"))
{
}

NormalizeGDXControls::~NormalizeGDXControls()
{
}

void NormalizeGDXControls::loadSettings(const QDomElement& _this)
{
    m_skewEnabledModel.loadSettings(_this, "skew_enabled");
    m_volumeEnabledModel.loadSettings(_this, "volume_enabled");
    m_balanceEnabledModel.loadSettings(_this, "balance_enabled");
    m_skewSpeedModel.loadSettings(_this, "skew_speed");
    m_volumeUpSpeedModel.loadSettings(_this, "volume_up_speed");
    m_volumeDownSpeedModel.loadSettings(_this, "volume_down_speed");
    m_balanceSpeedModel.loadSettings(_this, "balance_speed");
    m_outGainModel.loadSettings(_this, "out_gain");
}

void NormalizeGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_skewEnabledModel.saveSettings(doc, _this, "skew_enabled");
    m_volumeEnabledModel.saveSettings(doc, _this, "volume_enabled");
    m_balanceEnabledModel.saveSettings(doc, _this, "balance_enabled");
    m_skewSpeedModel.saveSettings(doc, _this, "skew_speed");
    m_volumeUpSpeedModel.saveSettings(doc, _this, "volume_up_speed");
    m_volumeDownSpeedModel.saveSettings(doc, _this, "volume_down_speed");
    m_balanceSpeedModel.saveSettings(doc, _this, "balance_speed");
    m_outGainModel.saveSettings(doc, _this, "out_gain");
}
