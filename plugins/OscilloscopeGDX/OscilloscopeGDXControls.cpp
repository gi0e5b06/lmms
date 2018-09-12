/*
 * OscilloscopeGDXControls.cpp -
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

#include "OscilloscopeGDXControls.h"

//#include "Knob.h"
#include "OscilloscopeGDX.h"
//#include "WaveForm.h"
//#include "Engine.h"
//#include "Song.h"
//#include "BufferManager.h"

#include <QDomElement>

OscilloscopeGDXControls::OscilloscopeGDXControls(OscilloscopeGDX* effect) :
      EffectControls(effect), m_effect(effect)
{
}

OscilloscopeGDXControls::~OscilloscopeGDXControls()
{
}

Ring* OscilloscopeGDXControls::ring()
{
    return m_effect->m_ring;
}

void OscilloscopeGDXControls::onControlChanged()
{
    //	engine::getSong()->setModified();
}

void OscilloscopeGDXControls::loadSettings(const QDomElement& _this)
{
    /*
m_waveBankModel.loadSettings(_this, "wave_bank");
m_waveIndexModel.loadSettings(_this, "wave_index");
m_timeModel.loadSettings(_this, "time");
m_ratioModel.loadSettings(_this, "ratio");
m_outGainModel.loadSettings(_this, "out_gain");
m_modeModel.loadSettings(_this, "mode");
    */
}

void OscilloscopeGDXControls::saveSettings(QDomDocument& doc,
                                           QDomElement&  _this)
{
    /*
m_waveBankModel.saveSettings(doc, _this, "wave_bank");
m_waveIndexModel.saveSettings(doc, _this, "wave_index");
m_timeModel.saveSettings(doc, _this, "time");
m_ratioModel.saveSettings(doc, _this, "ratio");
m_outGainModel.saveSettings(doc, _this, "out_gain");
m_modeModel.saveSettings(doc, _this, "mode");
    */
}
