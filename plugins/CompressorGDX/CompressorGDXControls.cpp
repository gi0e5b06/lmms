/*
 * CompressorGDXControls.cpp -
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

#include "CompressorGDXControls.h"

#include "CompressorGDX.h"
#include "Engine.h"
#include "Song.h"

#include <QDomElement>

CompressorGDXControls::CompressorGDXControls(CompressorGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_inGainModel(1., 0., 10., 0.001, this, tr("In gain")),
      m_thresholdModel(1., 0., 1., 0.001, this, tr("Threshold")),
      m_ratioModel(1., 0., 1., 0.001, this, tr("Ratio")),
      m_modeModel(2., 0., 5., 1., this, tr("Mode")),
      m_boostModel(1., 1., 100., 0.001, this, tr("Boost")),
      m_outGainModel(1., 0., 1., 0.001, this, tr("Out gain"))
{
    /*
      connect( &m_volumeModel, SIGNAL( dataChanged() ), this, SLOT(
      changeControl() ) ); connect( &m_panModel, SIGNAL( dataChanged() ),
      this, SLOT( changeControl() ) ); connect( &m_leftModel, SIGNAL(
      dataChanged() ), this, SLOT( changeControl() ) ); connect(
      &m_rightModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() )
      );
    */
}

void CompressorGDXControls::changeControl()
{
    //	engine::getSong()->setModified();
}

void CompressorGDXControls::loadSettings(const QDomElement& _this)
{
    m_inGainModel.loadSettings(_this, "in_gain");
    m_thresholdModel.loadSettings(_this, "treshold");
    m_ratioModel.loadSettings(_this, "ratio");
    m_modeModel.loadSettings(_this, "mode");
    m_outGainModel.loadSettings(_this, "out_gain");
    m_boostModel.loadSettings(_this, "boost");
}

void CompressorGDXControls::saveSettings(QDomDocument& doc,
                                         QDomElement&  _this)
{
    m_inGainModel.saveSettings(doc, _this, "in_gain");
    m_thresholdModel.saveSettings(doc, _this, "treshold");
    m_ratioModel.saveSettings(doc, _this, "ratio");
    m_modeModel.saveSettings(doc, _this, "mode");
    m_outGainModel.saveSettings(doc, _this, "out_gain");
    m_boostModel.saveSettings(doc, _this, "boost");
}
