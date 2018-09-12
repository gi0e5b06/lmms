/*
 * SmashGDXControls.cpp - controls for wall effect
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

#include "SmashGDXControls.h"

#include "Engine.h"
#include "SmashGDX.h"
#include "Song.h"

#include <QDomElement>

SmashGDXControls::SmashGDXControls(SmashGDXEffect* effect) :
      EffectControls(effect), m_effect(effect),
      m_rateModel(0.f, 0.f, 0.005f, 0.00001f, this, tr("Rate")),
      m_levelModel(0.f, 0.f, 1.f, 0.00001f, this, tr("Level"))  //,
// m_wetModel(0.75f, 0.f, 1.f, 0.00001f, this, tr("Wet"))
{
    /*
    connect( &m_rateModel, SIGNAL( dataChanged() ), this, SLOT(
      changeControl() ) );
    connect( &m_wetModel, SIGNAL(
      dataChanged() ), this, SLOT( changeControl() ) );
    */
}

void SmashGDXControls::changeControl()
{
    // engine::getSong()->setModified();
}

void SmashGDXControls::loadSettings(const QDomElement& _this)
{
    m_rateModel.loadSettings(_this, "rate");
    m_levelModel.loadSettings(_this, "level");
    // m_wetModel.loadSettings(_this, "wet");
}

void SmashGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_rateModel.saveSettings(doc, _this, "rate");
    m_levelModel.saveSettings(doc, _this, "level");
    // m_wetModel.saveSettings(doc, _this, "wet");
}
