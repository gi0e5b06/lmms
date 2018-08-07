/*
 * OutputGDXControls.cpp - controls for audio output properties
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

#include "OutputGDXControls.h"

#include <QDomElement>

#include "Engine.h"
#include "OutputGDX.h"
#include "Song.h"

OutputGDXControls::OutputGDXControls(OutputGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_leftModel(0.f, -1.f, 1.f, 0.00001f, this, tr("Left")),
      m_rightModel(0.f, -1.f, 1.f, 0.00001f, this, tr("Right")),
      m_rmsModel(0.f, 0.f, 1.f, 0.0001f, this, tr("RMS")),
      m_volModel(0.f, 0.f, 1.f, 0.0001f, this, tr("VOL")),
      m_panModel(0.f, -1.f, 1.f, 0.0001f, this, tr("PAN")),
      m_frequencyModel(440.f, 1.f, 25000.f, 1.f, this, tr("Frequency"))
{
    m_leftModel.setFrequentlyUpdated(true);
    m_rightModel.setFrequentlyUpdated(true);
    m_rmsModel.setFrequentlyUpdated(true);
    m_volModel.setFrequentlyUpdated(true);
    m_panModel.setFrequentlyUpdated(true);
    m_frequencyModel.setFrequentlyUpdated(true);

    //connect(&m_rmsModel, SIGNAL(dataChanged()), this, SLOT(changeControl()));
}

void OutputGDXControls::changeControl()
{
    // qInfo("OutputGDXControls::changeControl rms changed, value: %f",
    // m_rmsModel.value());
    // engine::getSong()->setModified();
}

void OutputGDXControls::loadSettings(const QDomElement& _this)
{
    // m_distanceModel.loadSettings(_this, "distance");
}

void OutputGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    // m_distanceModel.saveSettings(doc, _this, "distance");
}
