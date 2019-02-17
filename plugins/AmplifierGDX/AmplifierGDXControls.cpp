/*
 * AmplifierGDXControls.cpp -
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

#include "AmplifierGDXControls.h"

#include "AmplifierGDX.h"
#include "Engine.h"
#include "Song.h"

#include <QDomElement>

AmplifierGDXControls::AmplifierGDXControls(AmplifierGDXEffect* effect) :
      EffectControls(effect), m_effect(effect),
      m_volumeModel(100., 0., 500., 0.1, this, tr("Volume")),
      m_balanceModel(0., -100., 100., 0.1, this, tr("Balance")),
      m_leftVolumeModel(100., 0., 100., 0.1, this, tr("Left volume")),
      m_rightVolumeModel(100., 0., 100., 0.1, this, tr("Right volume")),
      m_widthModel(100., -100., 100., 0.1, this, tr("Width")),
      m_leftPanningModel(-100., -100., 100., 0.1, this, tr("Left panning")),
      m_rightPanningModel(100., -100., 100., 0.1, this, tr("Right panning")),
      m_responseModel(0., -1., 1., 0.001, this, tr("Response"))
{
    /*
      connect( &m_volumeModel, SIGNAL( dataChanged() ), this, SLOT(
      changeControl() ) ); connect( &m_balanceModel, SIGNAL( dataChanged() ),
      this, SLOT( changeControl() ) ); connect( &m_leftVolumeModel, SIGNAL(
      dataChanged() ), this, SLOT( changeControl() ) ); connect(
      &m_rightVolumeModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
    */
}

void AmplifierGDXControls::changeControl()
{
    // engine::getSong()->setModified();
}

void AmplifierGDXControls::loadSettings(const QDomElement& _this)
{
    m_volumeModel.loadSettings(_this, "volume");
    m_balanceModel.loadSettings(_this, "balance");
    m_leftVolumeModel.loadSettings(_this, "left_volume");
    m_rightVolumeModel.loadSettings(_this, "right_volume");
    m_widthModel.loadSettings(_this, "width");
    m_leftPanningModel.loadSettings(_this, "left_panning");
    m_rightPanningModel.loadSettings(_this, "right_panning");
    m_responseModel.loadSettings(_this, "response");
}

void AmplifierGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_volumeModel.saveSettings(doc, _this, "volume");
    m_balanceModel.saveSettings(doc, _this, "balance");
    m_leftVolumeModel.saveSettings(doc, _this, "left_volume");
    m_rightVolumeModel.saveSettings(doc, _this, "right_volume");
    m_widthModel.saveSettings(doc, _this, "width");
    m_leftPanningModel.saveSettings(doc, _this, "left_panning");
    m_rightPanningModel.saveSettings(doc, _this, "right_panning");
    m_responseModel.saveSettings(doc, _this, "response");
}
