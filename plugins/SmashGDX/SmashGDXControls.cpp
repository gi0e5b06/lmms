/*
 * SmashGDXControls.cpp - controls for wall effect
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

#include "SmashGDXControls.h"

#include "Engine.h"
#include "SmashGDX.h"
#include "Song.h"

#include <QDomElement>

SmashGDXControls::SmashGDXControls(SmashGDXEffect* effect) :
      EffectControls(effect), m_effect(effect),
      m_rateModel(0., 0., 0.01, 0.00001, this, tr("Rate")),
      m_phaseModel(0., -1., 1., 0.00001, this, tr("Phase")),
      m_levelModel(0., 0., 1., 0.00001, this, tr("Level")),
      m_bitsModel(64., 0., 64., 1., this, tr("Bits"))
{
    m_bitsModel.setStrictStepSize(true);
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
    m_bitsModel.loadSettings(_this, "bits");
}

void SmashGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_rateModel.saveSettings(doc, _this, "rate");
    m_levelModel.saveSettings(doc, _this, "level");
    m_bitsModel.saveSettings(doc, _this, "bits");
}
