/*
 * SpaceGDXControls.cpp - controls for wall effect
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

#include "SpaceGDXControls.h"

#include "Engine.h"
#include "Song.h"
#include "SpaceGDX.h"

#include <QDomElement>

SpaceGDXControls::SpaceGDXControls(SpaceGDXEffect* effect) :
      EffectControls(effect), m_effect(effect),
      m_rightPhaseModel(0., 0., 1., 0.0001, this, tr("Phase")),
      m_rightGainModel(1., 0., 1., 0.0001, this, tr("Gain")),
      m_rightLowModel(0., 0., 1., 0.0001, this, tr("Low Pass")),
      m_rightHighModel(0., 0., 1., 0.0001, this, tr("High Pass"))
{
    /*
    connect( &m_distanceModel, SIGNAL( dataChanged() ), this, SLOT(
      changeControl() ) );
    connect( &m_wetModel, SIGNAL(
      dataChanged() ), this, SLOT( changeControl() ) );
    */
}

void SpaceGDXControls::changeControl()
{
    // engine::getSong()->setModified();
}

void SpaceGDXControls::loadSettings(const QDomElement& _this)
{
    /*    m_distanceModel.loadSettings(_this, "distance");
m_dryModel.loadSettings(_this, "dry");
m_wetModel.loadSettings(_this, "wet");
    */
}

void SpaceGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    /*
m_distanceModel.saveSettings(doc, _this, "distance");
m_dryModel.saveSettings(doc, _this, "dry");
m_wetModel.saveSettings(doc, _this, "wet");
    */
}
