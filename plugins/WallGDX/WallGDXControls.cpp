/*
 * WallGDXControls.cpp - controls for wall effect
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

#include "WallGDXControls.h"

#include <QDomElement>

#include "Engine.h"
#include "Song.h"
#include "WallGDX.h"

WallGDXControls::WallGDXControls(WallGDXEffect* effect) :
      EffectControls(effect), m_effect(effect),
      m_distanceModel(0.5f, 0.f, 1.f, 0.00001f, this, tr("Distance")),
      m_dryModel(0.75f, 0.0f, 1.f, 0.00001f, this, tr("Dry")),
      m_wetModel(0.75f, 0.0f, 1.f, 0.00001f, this, tr("Wet"))
{
    /*
    connect( &m_distanceModel, SIGNAL( dataChanged() ), this, SLOT(
      changeControl() ) );
    connect( &m_wetModel, SIGNAL(
      dataChanged() ), this, SLOT( changeControl() ) );
    */
}

void WallGDXControls::changeControl()
{
    // engine::getSong()->setModified();
}

void WallGDXControls::loadSettings(const QDomElement& _this)
{
    m_distanceModel.loadSettings(_this, "distance");
    m_dryModel.loadSettings(_this, "dry");
    m_wetModel.loadSettings(_this, "wet");
}

void WallGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_distanceModel.saveSettings(doc, _this, "distance");
    m_dryModel.saveSettings(doc, _this, "dry");
    m_wetModel.saveSettings(doc, _this, "wet");
}
