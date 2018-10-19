/*
 * InputGDXControls.cpp - controls for audio output properties
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

#include "InputGDXControls.h"

#include "Engine.h"
#include "InputGDX.h"
#include "Song.h"

#include <QDomElement>

InputGDXControls::InputGDXControls(InputGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_leftSignalModel(0., -1., 1., 0.00000001, this, tr("Left")),
      m_rightSignalModel(0., -1., 1., 0.00000001, this, tr("Right")),
      m_volumeModel(1., 0., 1., 0.00000001, this, tr("Volume")),
      m_balanceModel(0., -1., 1., 0.00000001, this, tr("Balance")),
      m_mixingModel(0.5, 0., 1., 0.00000001, this, tr("Mixing")),
      m_deltaModel(0.001, 0.001, 1., 0.001, this, tr("Delta"))
{
    m_leftSignalModel.setFrequentlyUpdated(true);
    m_rightSignalModel.setFrequentlyUpdated(true);
    m_volumeModel.setFrequentlyUpdated(true);
    m_balanceModel.setFrequentlyUpdated(true);
    //m_mixingModel.setFrequentlyUpdated(true);

    /*
    connect(effect,SIGNAL(sendLeft(const ValueBuffer*)),
             &m_leftModel, SLOT(setAutomatedBuffer(const ValueBuffer*)));
    connect(effect,SIGNAL(sendRight(const ValueBuffer*)),
             &m_rightModel, SLOT(setAutomatedBuffer(const ValueBuffer*)));
    connect(effect,SIGNAL(sendRms(const real_t)),
             &m_rmsModel, SLOT(setAutomatedValue(real_t)));
    connect(effect,SIGNAL(sendVol(const real_t)),
             &m_volModel, SLOT(setAutomatedValue(real_t)));
    connect(effect,SIGNAL(sendPan(const real_t)),
             &m_panModel, SLOT(setAutomatedValue(real_t)));
    */
}

void InputGDXControls::loadSettings(const QDomElement& _this)
{
    m_volumeModel.loadSettings(_this, "volume");
    m_balanceModel.loadSettings(_this, "balance");
    m_mixingModel.loadSettings(_this, "mixing");
    m_deltaModel.loadSettings(_this, "delta");
}

void InputGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_volumeModel.saveSettings(doc, _this, "volume");
    m_balanceModel.saveSettings(doc, _this, "balance");
    m_mixingModel.saveSettings(doc, _this, "mixing");
    m_deltaModel.saveSettings(doc, _this, "delta");
}
