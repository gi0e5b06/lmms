/*
 * OutputGDXControls.cpp - controls for audio output properties
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
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

#include "OutputGDXControls.h"

#include "Engine.h"
#include "OutputGDX.h"
#include "Song.h"

#include <QDomElement>

OutputGDXControls::OutputGDXControls(OutputGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_leftModel(0., -1., 1., 0.000001, this, tr("Left")),
      m_rightModel(0., -1., 1., 0.000001, this, tr("Right")),
      m_rmsModel(0., 0., 1., 0.001, this, tr("RMS")),
      m_volModel(0., 0., 1., 0.001, this, tr("VOL")),
      m_panModel(0., -1., 1., 0.001, this, tr("BAL"))
{
    m_leftModel.setJournalling(false);;
    m_rightModel.setJournalling(false);;
    m_rmsModel.setJournalling(false);;
    m_volModel.setJournalling(false);;
    m_panModel.setJournalling(false);;

    m_leftModel.setFrequentlyUpdated(true);
    m_rightModel.setFrequentlyUpdated(true);
    m_rmsModel.setFrequentlyUpdated(true);
    m_volModel.setFrequentlyUpdated(true);
    m_panModel.setFrequentlyUpdated(true);
}

void OutputGDXControls::loadSettings(const QDomElement& _this)
{
}

void OutputGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
}
