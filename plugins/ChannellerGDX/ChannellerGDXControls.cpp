/*
 * ChannellerGDXControls.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include "ChannellerGDXControls.h"

#include "ChannellerGDX.h"

#include <QDomElement>

ChannellerGDXControls::ChannellerGDXControls(ChannellerGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_operationModel(this, tr("Operation"))
{
    m_operationModel.addItem(tr("Silence"));
    m_operationModel.addItem(tr("Identity"));
    m_operationModel.addItem(tr("Swap"));
    m_operationModel.addItem(tr("Silence right"));
    m_operationModel.addItem(tr("Silence left"));
    m_operationModel.addItem(tr("Mono"));
    m_operationModel.addItem(tr("To Mid/Side"));
    m_operationModel.addItem(tr("From Mid/Side"));
    m_operationModel.addItem(tr("To polar"));
    m_operationModel.addItem(tr("From polar"));
    m_operationModel.setValue(1);
}

ChannellerGDXControls::~ChannellerGDXControls()
{
}

void ChannellerGDXControls::changeControl()
{
}

void ChannellerGDXControls::loadSettings(const QDomElement& _this)
{
    m_operationModel.loadSettings(_this, "operation");
}

void ChannellerGDXControls::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    m_operationModel.saveSettings(_doc, _this, "operation");
}
