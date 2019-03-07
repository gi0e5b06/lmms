/*
 * ShifterGDXControls.cpp -
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

#include "ShifterGDXControls.h"

#include "ShifterGDX.h"

#include <QDomElement>

ShifterGDXControls::ShifterGDXControls(ShifterGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_deltaFrequencyModel(
              0., -20000., 20000., 1., this, tr("Frequency delta")),
      m_lowFrequencyModel(
              0., 0., 20000., 1., this, tr("Low frequency")),
      m_highFrequencyModel(
              20000., 0., 20000., 1., this, tr("High frequency")),
      m_slopeFrequencyModel(
              0.220, 0., 1., 0.001, this, tr("Frequency slope")),
      m_deltaVolumeModel(0., -1., 1., 0.0001, this, tr("Volume delta")),
      m_lowVolumeModel(
              -1., -1., 1., 0.0001, this, tr("Low volume")),
      m_highVolumeModel(
                        1., -1., 1., 0.0001, this, tr("High volume")),
      m_slopeVolumeModel(
              0.220, 0., 1., 0.001, this, tr("Volume slope"))
{
    m_deltaFrequencyModel.setStrictStepSize(true);
    m_lowFrequencyModel.setStrictStepSize(true);
    m_highFrequencyModel.setStrictStepSize(true);
}

ShifterGDXControls::~ShifterGDXControls()
{
}

void ShifterGDXControls::changeControl()
{
}

void ShifterGDXControls::loadSettings(const QDomElement& _this)
{
    m_deltaFrequencyModel.loadSettings(_this, "frequency_delta");
    m_lowFrequencyModel.loadSettings(_this, "frequency_low");
    m_highFrequencyModel.loadSettings(_this, "frequency_high");
    m_slopeFrequencyModel.loadSettings(_this, "frequency_slope");

    m_deltaVolumeModel.loadSettings(_this, "volume_delta");
    m_lowVolumeModel.loadSettings(_this, "volume_low");
    m_highVolumeModel.loadSettings(_this, "volume_high");
    m_slopeVolumeModel.loadSettings(_this, "volume_slope");
}

void ShifterGDXControls::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    m_deltaFrequencyModel.saveSettings(_doc, _this, "frequency_delta");
    m_lowFrequencyModel.saveSettings(_doc, _this, "frequency_low");
    m_highFrequencyModel.saveSettings(_doc, _this, "frequency_high");
    m_slopeFrequencyModel.saveSettings(_doc, _this, "frequency_slope");

    m_deltaVolumeModel.saveSettings(_doc, _this, "volume_delta");
    m_lowVolumeModel.saveSettings(_doc, _this, "volume_low");
    m_highVolumeModel.saveSettings(_doc, _this, "volume_high");
    m_slopeVolumeModel.saveSettings(_doc, _this, "volume_slope");
}
