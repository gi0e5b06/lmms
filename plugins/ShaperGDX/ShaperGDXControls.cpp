/*
 * ShaperGDXControls.cpp -
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

#include "ShaperGDXControls.h"

#include "Knob.h"
#include "ShaperGDX.h"
#include "WaveForm.h"
//#include "Engine.h"
//#include "Song.h"
#include "BufferManager.h"

#include <QDomElement>

ShaperGDXControls::ShaperGDXControls(ShaperGDX* effect) :
      EffectControls(effect), m_effect(effect),
      m_waveBankModel(this, tr("Wave Bank")),
      m_waveIndexModel(this, tr("Wave Index")), m_timeModel(55.0f,
                                                            0.00001f,
                                                            20000.0f,
                                                            0.00001f,
                                                            20000.0f,
                                                            this,
                                                            tr("Time")),
      m_ratioModel(1.0f, 0.0f, 1.0f, 0.001f, this, tr("Ratio")),
      m_outGainModel(1.0f, 0.0f, 10.0f, 0.001f, this, tr("Out gain")),
      m_hardModel(0.f, 0.f, 1.f, 0.001f, this, tr("Mode")),
      m_ring(600)
{
    WaveForm::fillBankModel(m_waveBankModel);
    WaveForm::fillIndexModel(m_waveIndexModel, 0);

    m_timeModel.setScaleLogarithmic(true);

    connect(&m_waveBankModel, SIGNAL(dataChanged()), this,
            SLOT(updateWaveIndexModel()));

    /*
    connect( &m_volumeModel, SIGNAL( dataChanged() ), this, SLOT(
    changeControl() ) ); connect( &m_panModel, SIGNAL( dataChanged() ), this,
    SLOT( changeControl() ) ); connect( &m_leftModel, SIGNAL( dataChanged() ),
    this, SLOT( changeControl() ) ); connect( &m_rightModel, SIGNAL(
    dataChanged() ), this, SLOT( changeControl() ) );
    */
}

ShaperGDXControls::~ShaperGDXControls()
{
}

Ring* ShaperGDXControls::ring()
{
    return m_effect->m_ring;
}

void ShaperGDXControls::changeControl()
{
    //	engine::getSong()->setModified();
}

void ShaperGDXControls::loadSettings(const QDomElement& _this)
{
    m_waveBankModel.loadSettings(_this, "wave_bank");
    m_waveIndexModel.loadSettings(_this, "wave_index");
    m_timeModel.loadSettings(_this, "time");
    m_ratioModel.loadSettings(_this, "ratio");
    m_outGainModel.loadSettings(_this, "out_gain");
    m_hardModel.loadSettings(_this, "hard");
}

void ShaperGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_waveBankModel.saveSettings(doc, _this, "wave_bank");
    m_waveIndexModel.saveSettings(doc, _this, "wave_index");
    m_timeModel.saveSettings(doc, _this, "time");
    m_ratioModel.saveSettings(doc, _this, "ratio");
    m_outGainModel.saveSettings(doc, _this, "out_gain");
    m_hardModel.saveSettings(doc, _this, "hard");
}

void ShaperGDXControls::updateWaveIndexModel()
{
    int bank = m_waveBankModel.value();
    int old  = m_waveIndexModel.value();
    WaveForm::fillIndexModel(m_waveIndexModel, bank);
    m_waveIndexModel.setValue(old);
}
