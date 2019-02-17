/*
 * DistortorGDXControls.cpp -
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

#include "DistortorGDXControls.h"

#include "DistortorGDX.h"
#include "Engine.h"
#include "Song.h"

#include <QDomElement>

DistortorGDXControls::DistortorGDXControls(DistortorGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_simpleModel(1., 1., 10., 0.001, this, tr("Simple")),
      m_foldoverModel(1., 1., 10., 0.001, this, tr("Foldover")),
      m_modulatorModel(1., 1., 10., 0.001, this, tr("Modulator")),
      m_crossoverModel(1., 1., 10., 0.001, this, tr("Crossover")),
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

void DistortorGDXControls::changeControl()
{
    //	engine::getSong()->setModified();
}

void DistortorGDXControls::loadSettings(const QDomElement& _this)
{
    m_outGainModel.loadSettings(_this, "out_gain");
}

void DistortorGDXControls::saveSettings(QDomDocument& doc,
                                         QDomElement&  _this)
{
    m_outGainModel.saveSettings(doc, _this, "out_gain");
}
