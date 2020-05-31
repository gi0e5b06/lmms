/*
 * LfoController.cpp - implementation of class controller which handles
 *                      remote-control of AutomatableModels
 *
 * Copyright (c) 2008 Paul Giblock <drfaygo/at/gmail.com>
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

#include "LfoController.h"

#include "Mixer.h"
#include "Oscillator.h"
#include "Song.h"

#include <QDomElement>
//#include <QObject>

LfoController::LfoController(Model* _parent) :
      Controller(Controller::LfoController, _parent, tr("LFO Controller")),
      m_baseModel(0.5, 0.0, 1.0, 0.001, this, tr("Base value")),
      m_speedModel(
              2.0, 0.01, 20.0, 0.0001, 20000.0, this, tr("Oscillator speed")),
      m_amountModel(1.0, -1.0, 1.0, 0.005, this, tr("Oscillator amount")),
      m_phaseModel(0.0, 0.0, 360.0, 4.0, this, tr("Oscillator phase")),
      m_waveModel(Oscillator::SineWave,
                  0,
                  Oscillator::NumWaveShapes,
                  this,
                  tr("Oscillator waveform")),
      m_multiplierModel(0, 0, 2, this, tr("Frequency Multiplier")),
      m_duration(1000), m_phaseOffset(0), m_currentPhase(0),
      m_sampleFunction(&Oscillator::sinSample),
      m_userDefSampleBuffer(new SampleBuffer)
{
    setFrequentlyUpdated(true);
    setSampleExact(true);
    connect(&m_waveModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleFunction()));

    connect(&m_speedModel, SIGNAL(dataChanged()), this,
            SLOT(updateDuration()));
    connect(&m_multiplierModel, SIGNAL(dataChanged()), this,
            SLOT(updateDuration()));
    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(updateDuration()));

    connect(Engine::getSong(), SIGNAL(playbackStateChanged()), this,
            SLOT(updatePhase()));
    connect(Engine::getSong(), SIGNAL(playbackPositionChanged()), this,
            SLOT(updatePhase()));

    updateDuration();
}

LfoController::~LfoController()
{
    sharedObject::unref(m_userDefSampleBuffer);
    m_baseModel.disconnect(this);
    m_speedModel.disconnect(this);
    m_amountModel.disconnect(this);
    m_phaseModel.disconnect(this);
    m_waveModel.disconnect(this);
    m_multiplierModel.disconnect(this);
}

void LfoController::fillValueBuffer()
{
    m_phaseOffset = m_phaseModel.value() / 360.0;
    real_t phase  = m_currentPhase + m_phaseOffset;

    // roll phase up until we're in sync with period counter
    m_lastUpdatedPeriod++;
    if(m_lastUpdatedPeriod < s_periods)
    {
        int diff = s_periods - m_lastUpdatedPeriod;
        phase += static_cast<real_t>(Engine::mixer()->framesPerPeriod()
                                     * diff)
                 / m_duration;
        m_lastUpdatedPeriod += diff;
    }

    real_t       amount       = m_amountModel.value();
    ValueBuffer* amountBuffer = m_amountModel.valueBuffer();
    const int    amountInc    = amountBuffer ? 1 : 0;
    real_t*      amountPtr = amountBuffer ? amountBuffer->values() : &amount;

    // for( real_t& f : m_valueBuffer )
    int     len = m_valueBuffer.length();
    real_t* val = m_valueBuffer.values();
    for(int i = 0; i < len; ++i)
    {
        const real_t currentSample
                = m_sampleFunction != NULL
                          ? m_sampleFunction(phase)
                          : m_userDefSampleBuffer->userWaveSample(phase);

        // f
        // m_valueBuffer.set(i,...);
        val[i] = qBound(
                0., m_baseModel.value() + (*amountPtr * currentSample / 2.),
                1.);

        phase += 1.0 / m_duration;
        amountPtr += amountInc;
    }

    m_currentPhase = positivefraction(phase - m_phaseOffset);
    // m_lastUpdatedPeriod = s_periods;
}

void LfoController::updatePhase()
{
    m_currentPhase      = (Engine::getSong()->getFrames()) / m_duration;
    m_lastUpdatedPeriod = s_periods - 1;
}

void LfoController::updateDuration()
{
    real_t newDurationF
            = Engine::mixer()->processingSampleRate() * m_speedModel.value();

    switch(m_multiplierModel.value())
    {
        case 1:
            newDurationF /= 100.0;
            break;

        case 2:
            newDurationF *= 100.0;
            break;

        default:
            break;
    }

    m_duration = newDurationF;
}

void LfoController::updateSampleFunction()
{
    switch(m_waveModel.value())
    {
        case Oscillator::SineWave:
            m_sampleFunction = &Oscillator::sinSample;
            break;
        case Oscillator::TriangleWave:
            m_sampleFunction = &Oscillator::triangleSample;
            break;
        case Oscillator::SawWave:
            m_sampleFunction = &Oscillator::sawSample;
            break;
        case Oscillator::SquareWave:
            m_sampleFunction = &Oscillator::squareSample;
            break;
        case Oscillator::MoogSawWave:
            m_sampleFunction = &Oscillator::moogSawSample;
            break;
        case Oscillator::ExponentialWave:
            m_sampleFunction = &Oscillator::expSample;
            break;
        case Oscillator::WhiteNoise:
            m_sampleFunction = &Oscillator::noiseSample;
            break;
        case Oscillator::UserDefinedWave:
            m_sampleFunction = NULL;
            /*TODO: If C++11 is allowed, should change the type of
             m_sampleFunction be std::function<sample_t(const real_t)>
             and use the line below:
            */
            // m_sampleFunction = &(m_userDefSampleBuffer->userWaveSample)
            break;
    }
}

void LfoController::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    Controller::saveSettings(_doc, _this);

    m_baseModel.saveSettings(_doc, _this, "base");
    m_speedModel.saveSettings(_doc, _this, "speed");
    m_amountModel.saveSettings(_doc, _this, "amount");
    m_phaseModel.saveSettings(_doc, _this, "phase");
    m_waveModel.saveSettings(_doc, _this, "wave");
    m_multiplierModel.saveSettings(_doc, _this, "multiplier");
    _this.setAttribute("userwavefile", m_userDefSampleBuffer->audioFile());
}

void LfoController::loadSettings(const QDomElement& _this)
{
    Controller::loadSettings(_this);

    m_baseModel.loadSettings(_this, "base");
    m_speedModel.loadSettings(_this, "speed");
    m_amountModel.loadSettings(_this, "amount");
    m_phaseModel.loadSettings(_this, "phase");
    m_waveModel.loadSettings(_this, "wave");
    m_multiplierModel.loadSettings(_this, "multiplier");
    m_userDefSampleBuffer->setAudioFile(_this.attribute("userwavefile"));

    updateSampleFunction();
}

QString LfoController::nodeName() const
{
    return "lfocontroller";
}

ControllerDialog* LfoController::createDialog(QWidget* _parent)
{
    return new LfoControllerDialog(this, _parent);
}
