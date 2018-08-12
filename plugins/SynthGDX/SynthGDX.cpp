/*
 * SynthGDX.cpp - modular synth
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

#include "SynthGDX.h"

#include <QDomDocument>

#include "BufferManager.h"
#include "Engine.h"
#include "InstrumentPlayHandle.h"
#include "InstrumentTrack.h"
#include "MixHelpers.h"
#include "Mixer.h"
#include "NotePlayHandle.h"
#include "PerfLog.h"
#include "SynthGDXView.h"
#include "WaveForm.h"
#include "debug.h"
#include "embed.h"
//#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT synthgdx_plugin_descriptor = {
            STRINGIFY(PLUGIN_NAME),
            "SynthGDX",
            QT_TRANSLATE_NOOP("pluginBrowser", "Oscillators vs Modulators"),
            "gi0e5b06 (on github.com)",
            0x0110,
            Plugin::Instrument,
            new PluginPixmapLoader("logo"),
            NULL,
            NULL};
}

OscillatorObject::OscillatorObject(Model* _parent, int _idx) :
      Model(_parent),
      m_enabledModel(_idx == 0, this, tr("O%1 active").arg(_idx + 1)),
      m_wave1ReverseModel(false, this, tr("O%1 reverse wave").arg(_idx + 1)),
      m_wave1BankModel(this, tr("O%1 bank").arg(_idx + 1)),
      m_wave1IndexModel(this, tr("O%1 wave index").arg(_idx + 1)),
      m_wave1AbsoluteModel(
              false, this, tr("O%1 absolute wave").arg(_idx + 1)),
      m_wave1OppositeModel(
              false, this, tr("O%1 opposite wave").arg(_idx + 1)),
      m_wave1ComplementModel(
              false, this, tr("O%1 complementary wave").arg(_idx + 1)),
      m_wave2ReverseModel(false, this, tr("O%1 reverse wave").arg(_idx + 1)),
      m_wave2BankModel(this, tr("O%1 bank").arg(_idx + 1)),
      m_wave2IndexModel(this, tr("O%1 wave index").arg(_idx + 1)),
      m_wave2AbsoluteModel(
              false, this, tr("O%1 absolute wave").arg(_idx + 1)),
      m_wave2OppositeModel(
              false, this, tr("O%1 opposite wave").arg(_idx + 1)),
      m_wave2ComplementModel(
              false, this, tr("O%1 complementary wave").arg(_idx + 1)),
      m_waveMixModel(0.f,
                     0.f,
                     1.f,
                     0.00001f,
                     this,
                     tr("O%1 wave mix").arg(_idx + 1)),
      m_volumeModel(DefaultVolume / NB_OSCILLATORS,
                    MinVolume,
                    DefaultVolume,
                    0.01f,
                    this,
                    tr("O%1 volume").arg(_idx + 1)),
      m_panModel(DefaultPanning,
                 PanningLeft,
                 PanningRight,
                 0.01f,
                 this,
                 tr("O%1 panning").arg(_idx + 1)),
      m_coarseModel(0.f,  //-_idx * KeysPerOctave,
                    -6 * KeysPerOctave,
                    6 * KeysPerOctave,
                    1.f,
                    this,
                    tr("O%1 coarse detuning").arg(_idx + 1)),
      m_fineLeftModel(0.0f,
                      -100.0f,
                      100.0f,
                      1.f,
                      this,
                      tr("O%1 fine detuning left").arg(_idx + 1)),
      m_fineRightModel(0.0f,
                       -100.0f,
                       100.0f,
                       1.f,
                       this,
                       tr("O%1 fine detuning right").arg(_idx + 1)),
      m_phaseOffsetModel(0.0f,
                         0.0f,
                         360.0f,
                         1.0f,
                         this,
                         tr("O%1 phase-offset").arg(_idx + 1)),
      m_stereoPhaseDetuningModel(
              0.0f,
              0.0f,
              360.0f,
              1.0f,
              this,
              tr("O%1 stereo phase-detuning").arg(_idx + 1)),
      m_pulseCenterModel(0.5f,
                         0.f,
                         1.f,
                         0.00001f,
                         this,
                         tr("O%1 pulse center").arg(_idx + 1)),
      m_pulseWidthModel(0.5f,
                        0.f,
                        1.f,
                        0.00001f,
                        this,
                        tr("O%1 pulse width").arg(_idx + 1)),
      m_lfoEnabledModel(false, this, tr("O%1 LFO active").arg(_idx + 1)),
      m_lfoTimeModel(55.0f,
                     0.001f,
                     20000.0f,
                     0.001f,
                     20000.0f,
                     this,
                     tr("O%1 LFO time").arg(_idx + 1)),
      m_velocityAmountModel(0.f,
                            -1.f,
                            1.0f,
                            0.01f,
                            this,
                            tr("O%1 velocity").arg(_idx + 1)),
      m_harm2Model(0.f,
                   -1.f,
                   1.f,
                   0.0001f,
                   this,
                   tr("O%1 harmonics 2").arg(_idx + 1)),
      m_harm3Model(0.f,
                   -1.f,
                   1.f,
                   0.0001f,
                   this,
                   tr("O%1 harmonics 3").arg(_idx + 1)),
      m_harm4Model(0.f,
                   -1.f,
                   1.f,
                   0.0001f,
                   this,
                   tr("O%1 harmonics 4").arg(_idx + 1)),
      m_harm5Model(0.f,
                   -1.f,
                   1.f,
                   0.0001f,
                   this,
                   tr("O%1 harmonics 5").arg(_idx + 1)),
      m_harm6Model(0.f,
                   -1.f,
                   1.f,
                   0.0001f,
                   this,
                   tr("O%1 harmonics 6").arg(_idx + 1)),
      m_harm7Model(0.f,
                   -1.f,
                   1.f,
                   0.0001f,
                   this,
                   tr("O%1 harmonics 7").arg(_idx + 1)),
      m_skewModel(
              1.f, 0.f, 1.f, 0.001f, this, tr("O%1 anti-skew").arg(_idx + 1)),
      m_smoothModel(0.f,
                    0.f,
                    1.999f,
                    0.001f,
                    this,
                    tr("O%1 smoothing").arg(_idx + 1)),
      m_portamentoModel(0.0f,
                        0.0f,
                        0.999f,
                        0.001f,
                        this,
                        tr("O%1 portamento").arg(_idx + 1)),
      m_lowPassModel(0.0f,
                     0.0f,
                     0.999f,
                     0.001f,
                     this,
                     tr("O%1 low pass").arg(_idx + 1)),
      m_highPassModel(0.0f,
                      0.0f,
                      0.999f,
                      0.001f,
                      this,
                      tr("O%1 high pass").arg(_idx + 1)),
      m_wallModel(
              0.0f, 0.0f, 1.0f, 0.001f, this, tr("O%1 wall").arg(_idx + 1)),
      m_frequencyModel(440.f,
                       1.f,
                       25000.f,
                       0.01f,
                       this,
                       tr("O%1 frequency").arg(_idx + 1)),
      m_velocityModel(
              0.f, 0.f, 1.f, 0.0001f, this, tr("O%1 velocity").arg(_idx + 1))
{
    WaveForm::fillBankModel(m_wave1BankModel);
    WaveForm::fillBankModel(m_wave2BankModel);

    WaveForm::fillIndexModel(m_wave1IndexModel, 0);
    WaveForm::fillIndexModel(m_wave2IndexModel, 0);

    m_wave1 = WaveForm::get(0, 0);
    m_wave2 = WaveForm::get(0, 0);

    m_lfoTimeModel.setScaleLogarithmic(true);

    // const fpp_t FPP = Engine::mixer()->framesPerPeriod();
    // m_graphModel = new graphModel(0.f, 1.f, FPP, NULL);
}

OscillatorObject::~OscillatorObject()
{
}

void OscillatorObject::reset(const fpp_t _f)
{
    if(m_enabledModel.value())
    {
        m_updated = false;
        updateWaves(_f);
        updateVolumes(_f);
        updateDetunings(_f);
        updatePhaseOffsets(_f);
        updateFrequencies(_f);
        updatePulses(_f);

        for(int ch = 1; ch >= 0; --ch)
        {
            m_toneBase[ch]      = 0.f;
            m_addOutputBase[ch] = 0.f;
            m_previousInput1[ch] += m_currentInput1[ch];
            m_previousInput2[ch] += m_currentInput2[ch];
            m_currentInput1[ch] = 0.f;
            m_currentInput2[ch] = 0.f;

            m_waveMixModAmp[ch]     = 1.f;
            m_waveMixModVal[ch]     = 0.f;
            m_volumeModAmp[ch]      = 1.f;
            m_volumeModVal[ch]      = 0.f;
            m_detuningModAmp[ch]    = 1.f;
            m_detuningModVal[ch]    = 0.f;
            m_phaseOffsetModAmp[ch] = 1.f;
            m_phaseOffsetModVal[ch] = 0.f;
            m_frequencyModAmp[ch]   = 1.f;
            m_frequencyModVal[ch]   = 0.f;
            m_toneModAmp[ch]        = 1.f;
            m_toneModVal[ch]        = 0.f;
            m_pulseCenterModAmp[ch] = 0.5f;
            m_pulseCenterModVal[ch] = 0.f;
            m_pulseWidthModAmp[ch]  = 0.5f;
            m_pulseWidthModVal[ch]  = 0.f;
            m_addOutputModAmp[ch]   = 1.f;
            m_addOutputModVal[ch]   = 0.f;
        }
    }
    else
    {
        m_updated  = true;
        m_velocity = 0.5f;
        for(int ch = 1; ch >= 0; --ch)
        {
            m_frequencyBase[ch]  = 55.f;
            m_phase[ch]          = 0.f;
            m_phaseOffset[ch]    = 0.f;
            m_currentOutput[ch]  = 0.f;
            m_previousOutput[ch] = 0.f;
            m_averageOutput[ch]  = 0.f;
            m_currentInput1[ch]  = 0.f;
            m_previousInput1[ch] = 0.f;
            m_currentInput2[ch]  = 0.f;
            m_previousInput2[ch] = 0.f;
        }
    }
}

bool OscillatorObject::isUpdated()
{
    return m_updated;
}

float OscillatorObject::waveAt(ch_cnt_t ch, float x)
{
    // pulse 0--p1=pc-pw/2--p--p2=pc+pw/2--1
    float pc = m_pulseCenterBase[ch]
               * (1.f + m_pulseCenterModAmp[ch] * m_pulseCenterModVal[ch]);
    float pw = m_pulseWidthBase[ch]
               * (1.f + m_pulseWidthModAmp[ch] * m_pulseWidthModVal[ch]);
    pc       = qBound(0.f, pc, 1.f);
    pw       = qBound(0.f, pw, 1.f);
    float p1 = qBound(0.f, pc - 0.5f * pw, 1.f);
    float p2 = qBound(0.f, pc + 0.5f * pw, 1.f);

    if(x > 0.f && x < 1.f)  // && pw < 1.f)
    {
        if(x < 0.25f)
            x = p1 * x / 0.25f;
        else if(x < 0.75f)
            x = p1 + (p2 - p1) * (x - 0.25f) / 0.5f;
        else
            x = p2 + (1.f - p2) * (x - 0.75f) / 0.25f;
    }

    x = qBound(0.f, x, 1.f);

    float x1 = x;
    if(m_reverse1)
        x1 = 1.f - x1;

    float y1 = m_wave1->f(x1);
    /*
    switch(m_wave1)
    {
        case 0:
            y1 = linearnsinf01(x1);
            // y1 = normsinf(x1);
            break;
        case 1:
            y1 = lineartrianglef01(x1);
            break;
        case 2:
            y1 = linearsawtoothf01(x1);
            break;
        case 3:
            y1 = linearsquaref01(x1);
            break;
        case 4:
            y1 = linearharshsawf01(x1);
            break;
        case 5:
            y1 = linearpeakexpf01(x1);
            break;
        case 6:
            y1 = linearrandf01(x1);
            break;
        case 7:
            // compatibility, user defined
            y1 = 0.f;
            break;
        case 8:
        case 9:
        case 10:
            // reserved
            y1 = 0.f;
            break;
        case 11:
            y1 = m_previousInput1[ch];
            break;
        case 12:
            y1 = m_previousInput2[ch];
            break;
        case 13:
            y1 = -1.f;
            break;
        case 14:
            y1 = -0.5f;
            break;
        case 15:
            y1 = 0.f;
            break;
        case 16:
            y1 = 0.5f;
            break;
        case 17:
            y1 = 1.f;
            break;
        default:
            y1 = 0.f;
            break;
    }
    */

    if(m_absolute1 && y1 < 0.f)
        y1 = -y1;

    if(m_opposite1)
        y1 = -y1;

    if(m_complement1)
    {
        if(y1 < 0.f)
            y1 = -1.f - y1;
        else
            y1 = 1.f - y1;
    }

    const float wm = qBound(
            0.f,
            m_waveMixBase[ch] + m_waveMixModAmp[ch] * m_waveMixModVal[ch],
            1.f);
    if(wm == 0.f)
        return y1;

    float x2 = x;
    if(m_reverse2)
        x2 = 1.f - x2;

    float y2 = m_wave2->f(x2);
    /*
    switch(m_wave2)
    {
        case 0:
            y2 = linearnsinf01(x2);
            // y2 = normsinf(x2);
            break;
        case 1:
            y2 = lineartrianglef01(x2);
            break;
        case 2:
            y2 = linearsawtoothf01(x2);
            break;
        case 3:
            y2 = linearsquaref01(x2);
            break;
        case 4:
            y2 = linearharshsawf01(x2);
            break;
        case 5:
            y2 = linearpeakexpf01(x2);
            break;
        case 6:
            y2 = linearrandf01(x2);
            break;
        case 7:
            // compatibility, user defined
            y2 = 0.f;
            break;
        case 8:
        case 9:
        case 10:
            // reserved
            y2 = 0.f;
            break;
        case 11:
            y2 = m_previousInput1[ch];
            break;
        case 12:
            y2 = m_previousInput2[ch];
            break;
        case 13:
            y2 = -1.f;
            break;
        case 14:
            y2 = -0.5f;
            break;
        case 15:
            y2 = 0.f;
            break;
        case 16:
            y2 = 0.5f;
            break;
        case 17:
            y2 = 1.f;
            break;
        default:
            y2 = 0.f;
            break;
    }
    */

    if(m_absolute2 && y2 < 0.f)
        y2 = -y2;

    if(m_opposite2)
        y2 = -y2;

    if(m_complement2)
    {
        if(y2 < 0.f)
            y2 = -1.f - y2;
        else
            y2 = 1.f - y2;
    }

    return (1.f - wm) * y1 + wm * y2;
}

void OscillatorObject::update()
{
    if(m_updated)
        return;

    for(int ch = 1; ch >= 0; --ch)
    {
        float w = (m_frequencyBase[ch]
                           * (1.f
                              + m_frequencyModAmp[ch] * m_frequencyModVal[ch])
                   + 1000.f * m_toneModAmp[ch] * m_toneModVal[ch])
                  * (m_detuningBase[ch]
                     * (1.f + m_detuningModAmp[ch] * m_detuningModVal[ch]));
        w = qBound(-192000.f, w, 192000.f);

        float a = m_volumeBase[ch]
                  * (1.f + m_volumeModAmp[ch] * m_volumeModVal[ch]);
        a = qMin(fabsf(a), 1.f);
        // a=qBound(0.f,a,1.f);

        float x = m_phase[ch];
        x += 44100.0f / Engine::mixer()->processingSampleRate();
        x -= m_phaseOffset[ch];
        m_phaseOffset[ch]
                = m_phaseOffsetBase[ch]
                  * (1.f + m_phaseOffsetModAmp[ch] * m_phaseOffsetModVal[ch]);
        x += m_phaseOffset[ch];
        x = fraction(x);
        if(x < 0.f)
            x = fraction(x + 2.f);
        m_phase[ch] = x;

        float y = waveAt(ch, x);

        // harmonics
        if(m_harm2 != 0.f)
        {
            float xh2 = 2.f * (x - m_phaseOffset[ch]) + m_phaseOffset[ch];
            float yh2 = m_harm2 * waveAt(ch, xh2);
            y += yh2;
        }
        if(m_harm3 != 0.f)
        {
            float xh3 = 3.f * (x - m_phaseOffset[ch]) + m_phaseOffset[ch];
            float yh3 = m_harm3 * waveAt(ch, xh3);
            y += yh3;
        }
        if(m_harm4 != 0.f)
        {
            float xh4 = 4.f * (x - m_phaseOffset[ch]) + m_phaseOffset[ch];
            float yh4 = m_harm4 * waveAt(ch, xh4);
            y += yh4;
        }
        if(m_harm5 != 0.f)
        {
            float xh5 = 5.f * (x - m_phaseOffset[ch]) + m_phaseOffset[ch];
            float yh5 = m_harm5 * waveAt(ch, xh5);
            y += yh5;
        }
        if(m_harm6 != 0.f)
        {
            float xh6 = 6.f * (x - m_phaseOffset[ch]) + m_phaseOffset[ch];
            float yh6 = m_harm6 * waveAt(ch, xh6);
            y += yh6;
        }
        if(m_harm7 != 0.f)
        {
            float xh7 = 7.f * (x - m_phaseOffset[ch]) + m_phaseOffset[ch];
            float yh7 = m_harm7 * waveAt(ch, xh7);
            y += yh7;
        }

        if(m_wall > 0.f)
            y = (y + waveAt(ch, fraction(x + 1.f - m_wall))) / 2.f;

        y = a * y + m_addOutputModAmp[ch] * m_addOutputModVal[ch];

        y *= m_velocity;

        if(m_lowPass > 0.f)
        {
            y = m_lowPass * m_previousOutput[ch] + (1.f - m_lowPass) * y;
            y = qBound(-1.f, y, 1.f);
        }
        if(m_highPass > 0.f)
        {
            y = (1.f - m_highPass) * y - m_highPass * m_previousOutput[ch];
            y = qBound(-1.f, y, 1.f);
        }

        if(m_smooth > 0.f)
        {
            float dy = 2.f - m_smooth;
            float py = m_previousOutput[ch];
            // qInfo("y=%f py=%f dy=%f", y, py, dy);
            if(fabsf(y - py) > dy)
            {
                y = py + dy * sign(y - py);
                // qInfo("--> y=%f", y);
            }
        }

        m_previousOutput[ch] = m_currentOutput[ch];
        m_averageOutput[ch]  = 0.999 * m_averageOutput[ch] + 0.001 * y;
        if(m_skew < 1.f)
            y -= (1.f - m_skew) * m_averageOutput[ch];
        m_currentOutput[ch] = y;
        m_phase[ch] += w;
    }

    m_updated = true;
}

float OscillatorObject::output(const ch_cnt_t _ch)
{
    return m_updated ? m_currentOutput[_ch] : m_previousOutput[_ch];
}

void OscillatorObject::input1(const ch_cnt_t _ch, const float _in)
{
    if(!m_updated)
        m_previousInput1[_ch] = _in;
    else
        m_currentInput1[_ch] = _in;
}

void OscillatorObject::input2(const ch_cnt_t _ch, const float _in)
{
    if(!m_updated)
        m_previousInput2[_ch] = _in;
    else
        m_currentInput2[_ch] = _in;
}

OscillatorObject::OscState::OscState()
{
    // m_wave1 = WaveForm::get(0, 0);
    // m_wave2 = WaveForm::get(0, 0);

    for(int ch = 1; ch >= 0; --ch)
    {
        m_phase[ch]          = 0.f;
        m_phaseOffset[ch]    = 0.f;
        m_currentOutput[ch]  = 0.f;
        m_previousOutput[ch] = 0.f;
        m_averageOutput[ch]  = 0.f;
        m_currentInput1[ch]  = 0.f;
        m_previousInput1[ch] = 0.f;
        m_currentInput2[ch]  = 0.f;
        m_previousInput2[ch] = 0.f;
    }
}

void OscillatorObject::restoreFromState(OscState* _state)
{
    if(_state == NULL)
        return;

    m_frequencyModel.setAutomatedValue(_state->m_frequency);
    m_velocityModel.setAutomatedValue(_state->m_velocity);
    // m_wave1 = _state->m_wave1;
    // m_wave2 = _state->m_wave2;
    for(int ch = 1; ch >= 0; --ch)
    {
        m_phase[ch]          = _state->m_phase[ch];
        m_phaseOffset[ch]    = _state->m_phaseOffset[ch];
        m_currentOutput[ch]  = _state->m_currentOutput[ch];
        m_previousOutput[ch] = _state->m_previousOutput[ch];
        m_averageOutput[ch]  = _state->m_averageOutput[ch];
        m_currentInput1[ch]  = _state->m_currentInput1[ch];
        m_previousInput1[ch] = _state->m_previousInput1[ch];
        m_currentInput2[ch]  = _state->m_currentInput2[ch];
        m_previousInput2[ch] = _state->m_previousInput2[ch];
    }
}

void OscillatorObject::saveToState(OscState* _state)
{
    if(_state == NULL)
        return;

    _state->m_frequency = m_frequencyModel.value();
    _state->m_velocity  = m_velocityModel.value();
    //_state->m_wave1     = m_wave1;
    //_state->m_wave2     = m_wave2;

    for(int ch = 1; ch >= 0; --ch)
    {
        _state->m_phase[ch]          = m_phase[ch];
        _state->m_phaseOffset[ch]    = m_phaseOffset[ch];
        _state->m_currentOutput[ch]  = m_currentOutput[ch];
        _state->m_previousOutput[ch] = m_previousOutput[ch];
        _state->m_averageOutput[ch]  = m_averageOutput[ch];
        _state->m_currentInput1[ch]  = m_currentInput1[ch];
        _state->m_previousInput1[ch] = m_previousInput1[ch];
        _state->m_currentInput2[ch]  = m_currentInput2[ch];
        _state->m_previousInput2[ch] = m_previousInput2[ch];
    }
}

/*
bool OscillatorObject::syncOK(const float _coeff, const ch_cnt_t _ch)
{
    const float old = m_phase[_ch];
    m_phase[_ch] += _coeff;
    // check whether m_phase is in next period
    return (floorf(m_phase[_ch]) > floorf(old));
}

float OscillatorObject::syncInit(const ch_cnt_t _ch)
{
        recalculatePhase(_ch);
    return m_frequency[_ch] * m_detuning[_ch];
}
*/

void OscillatorObject::updateWaves(const fpp_t _f)
{
    m_wave1       = WaveForm::get(m_wave1BankModel.value(),
                            m_wave1IndexModel.value());
    m_reverse1    = m_wave1ReverseModel.value();
    m_absolute1   = m_wave1AbsoluteModel.value();
    m_opposite1   = m_wave1OppositeModel.value();
    m_complement1 = m_wave1ComplementModel.value();

    m_wave2       = WaveForm::get(m_wave2BankModel.value(),
                            m_wave2IndexModel.value());
    m_reverse2    = m_wave2ReverseModel.value();
    m_absolute2   = m_wave2AbsoluteModel.value();
    m_opposite2   = m_wave2OppositeModel.value();
    m_complement2 = m_wave2ComplementModel.value();

    m_waveMixBase[0] = m_waveMixModel.value() * 2.f - 1;
    m_waveMixBase[1] = m_waveMixBase[0];

    m_harm2 = m_harm2Model.value();
    m_harm3 = m_harm3Model.value();
    m_harm4 = m_harm4Model.value();
    m_harm5 = m_harm5Model.value();
    m_harm6 = m_harm6Model.value();
    m_harm7 = m_harm7Model.value();
}

void OscillatorObject::updatePulses(const fpp_t _f)
{
    m_pulseCenterBase[0] = m_pulseCenterModel.value();
    m_pulseCenterBase[1] = m_pulseCenterBase[0];
    m_pulseWidthBase[0]  = m_pulseWidthModel.value();
    m_pulseWidthBase[1]  = m_pulseWidthBase[0];
}

void OscillatorObject::updateFrequencies(const fpp_t _f)
{
    float w;

    if(m_lfoEnabledModel.value())
        w = 1000.f / m_lfoTimeModel.value();
    else
        w = m_frequencyModel.value();

    m_frequencyBase[0] = w;
    m_frequencyBase[1] = w;

    m_lowPass  = m_lowPassModel.value();
    m_highPass = m_highPassModel.value();
    m_wall     = m_wallModel.value();
}

void OscillatorObject::updateVolumes(const fpp_t _f)
{
    if(m_panModel.value() >= 0.0f)
    {
        m_volumeBase[1] = m_volumeModel.value() / DefaultVolume;
        const float panningFactorLeft
                = 1.f - m_panModel.value() / (float)PanningRight;
        m_volumeBase[0] = panningFactorLeft * m_volumeBase[1];
    }
    else
    {
        m_volumeBase[0] = m_volumeModel.value() / DefaultVolume;
        const float panningFactorRight
                = 1.f + m_panModel.value() / (float)PanningRight;
        m_volumeBase[1] = panningFactorRight * m_volumeBase[0];
    }

    float vv   = m_velocityModel.value();
    float va   = m_velocityAmountModel.value();
    m_velocity = (1.f - va) + va * (vv - 0.5f);

    float sv = m_smoothModel.value();
    m_smooth = sv * sv / 2.f;

    float sk = m_skewModel.value();
    m_skew   = sk;
}

void OscillatorObject::updateDetunings(const fpp_t _f)
{
    m_detuningBase[0] = powf(2.0f, (m_coarseModel.value() * 100.0f
                                    + m_fineLeftModel.value())
                                           / 1200.0f)
                        / Engine::mixer()->processingSampleRate();
    m_detuningBase[1] = powf(2.0f, (m_coarseModel.value() * 100.0f
                                    + m_fineRightModel.value())
                                           / 1200.0f)
                        / Engine::mixer()->processingSampleRate();
}

void OscillatorObject::updatePhaseOffsets(const fpp_t _f)
{
    m_stereoPhase        = m_stereoPhaseDetuningModel.value() / 360.0f;
    m_phaseOffsetBase[1] = m_phaseOffsetModel.value() / 360.0f;
    m_phaseOffsetBase[0] = m_phaseOffsetBase[1] + m_stereoPhase;
}

ModulatorObject::ModulatorObject(Model* _parent, int _idx) :
      Model(_parent),
      m_enabledModel(false, this, tr("M%1 active").arg(_idx + 1)),
      m_algoModel(this, tr("M%1 type").arg(_idx + 1)),
      m_modulatedModel(this, tr("Modulated oscillator")),  //.arg(_idx + 1)),
      m_modulatorModel(this, tr("Modulating oscillator"))  //.arg(_idx + 1))
{
    m_algoModel.addItem("Phase");                // 0
    m_algoModel.addItem("Amplitude");            // 1
    m_algoModel.addItem("Signal mix");           // 2
    m_algoModel.addItem("Synchronization");      // 3
    m_algoModel.addItem("Frequency");            // 4
    m_algoModel.addItem("Tone (CV VpO)");        // 5
    m_algoModel.addItem("Pulse Center");         // 6
    m_algoModel.addItem("Pulse Width");          // 7
    m_algoModel.addItem("Additive Output");      // 8
    m_algoModel.addItem("Substractive Output");  // 9
    m_algoModel.addItem("Wave Mixing");          // 10
    m_algoModel.addItem("Input 1");              // 11
    m_algoModel.addItem("Input 2");              // 12

    for(int o = 0; o < NB_OSCILLATORS; ++o)
    {
        m_modulatedModel.addItem(QString::number(o + 1));
        m_modulatorModel.addItem(QString::number(o + 1));
    }
}

ModulatorObject::~ModulatorObject()
{
}

bool ModulatorObject::isApplied()
{
    return m_applied;
}

void ModulatorObject::reset(f_cnt_t)
{
    m_applied = !m_enabledModel.value();
}

void ModulatorObject::apply(OscillatorObject* _modulated,
                            OscillatorObject* _modulator)
{
    for(int ch = 1; ch >= 0; --ch)
    {
        float val = _modulator->output(ch);
        switch(m_algoModel.value())
        {
            case 0:
                _modulated->m_phaseOffsetModVal[ch] += val;
                break;
            case 1:
                _modulated->m_volumeModVal[ch] += val;
                break;
            case 4:
                _modulated->m_frequencyModVal[ch] += val;
                break;
            case 5:
                _modulated->m_toneModVal[ch] += val;
                break;
            case 6:
                _modulated->m_pulseCenterModVal[ch] += val;
                break;
            case 7:
                _modulated->m_pulseWidthModVal[ch] += val;
                break;
            case 8:
                _modulated->m_addOutputModVal[ch] += val;
                break;
            case 9:
                _modulated->m_addOutputModVal[ch] -= val;
                break;
            case 10:
                _modulated->m_waveMixModVal[ch] += val;
                break;
            case 11:
                _modulated->input1(ch, val);
                break;
            case 12:
                _modulated->input2(ch, val);
                break;
        }
    }
    m_applied = true;
}

SynthGDX::SynthGDX(InstrumentTrack* _instrumentTrack) :
      Instrument(_instrumentTrack, &synthgdx_plugin_descriptor)
{
    for(int o = NB_OSCILLATORS - 1; o >= 0; --o)
    {
        m_osc[o] = new OscillatorObject(this, o);
        // m_osc[o]->m_enabledModel.setValue(o == 0);
    }

    for(int m = NB_MODULATORS - 1; m >= 0; --m)
    {
        m_mod[m] = new ModulatorObject(this, m);
        // m_mod[m]->m_enabledModel.setValue(false);
        m_mod[m]->m_modulatedModel.setValue(0);
        m_mod[m]->m_modulatorModel.setValue((m + 1) % NB_OSCILLATORS);
    }

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(updateAllDetuning()));
}

SynthGDX::~SynthGDX()
{
    for(int o = NB_OSCILLATORS - 1; o >= 0; --o)
    {
        // delete m_osc[i];
        m_osc[o] = NULL;
    }

    for(int m = NB_MODULATORS - 1; m >= 0; --m)
    {
        // delete m_mod[i];
        m_mod[m] = NULL;
    }
}

void SynthGDX::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    for(int i = 0; i < NB_OSCILLATORS; ++i)
    {
        QString is = QString::number(i);
        m_osc[i]->m_enabledModel.saveSettings(_doc, _this, "enabled" + is);

        m_osc[i]->m_waveMixModel.saveSettings(_doc, _this, "wave_mix" + is);
        m_osc[i]->m_wave1ReverseModel.saveSettings(_doc, _this,
                                                   "wave1_reverse" + is);

        /* deprecated
        m_osc[i]->m_wave1IndexModel.saveSettings(_doc, _this,
                                                 "wave1type" + is);
        m_osc[i]->m_wave2IndexModel.saveSettings(_doc, _this,
                                                 "wave2type" + is);
        */

        m_osc[i]->m_wave1BankModel.saveSettings(_doc, _this,
                                                "wave1_bank" + is);
        m_osc[i]->m_wave1IndexModel.saveSettings(_doc, _this,
                                                 "wave1_index" + is);
        m_osc[i]->m_wave1AbsoluteModel.saveSettings(_doc, _this,
                                                    "wave1_absolute" + is);
        m_osc[i]->m_wave1OppositeModel.saveSettings(_doc, _this,
                                                    "wave1_opposite" + is);
        m_osc[i]->m_wave1ComplementModel.saveSettings(
                _doc, _this, "wave1_complement" + is);

        m_osc[i]->m_wave2ReverseModel.saveSettings(_doc, _this,
                                                   "wave2_reverse" + is);
        m_osc[i]->m_wave2BankModel.saveSettings(_doc, _this,
                                                "wave2_bank" + is);
        m_osc[i]->m_wave2IndexModel.saveSettings(_doc, _this,
                                                 "wave2_index" + is);
        m_osc[i]->m_wave2AbsoluteModel.saveSettings(_doc, _this,
                                                    "wave2_absolute" + is);
        m_osc[i]->m_wave2OppositeModel.saveSettings(_doc, _this,
                                                    "wave2_opposite" + is);
        m_osc[i]->m_wave2ComplementModel.saveSettings(
                _doc, _this, "wave2_complement" + is);

        m_osc[i]->m_volumeModel.saveSettings(_doc, _this, "vol" + is);
        m_osc[i]->m_panModel.saveSettings(_doc, _this, "pan" + is);
        m_osc[i]->m_coarseModel.saveSettings(_doc, _this, "coarse" + is);
        m_osc[i]->m_fineLeftModel.saveSettings(_doc, _this, "finel" + is);
        m_osc[i]->m_fineRightModel.saveSettings(_doc, _this, "finer" + is);
        m_osc[i]->m_lowPassModel.saveSettings(_doc, _this,
                                              "filter_lowpass" + is);
        m_osc[i]->m_highPassModel.saveSettings(_doc, _this,
                                               "filter_highpass" + is);
        m_osc[i]->m_wallModel.saveSettings(_doc, _this, "filter_wall" + is);

        m_osc[i]->m_phaseOffsetModel.saveSettings(_doc, _this,
                                                  "phoffset" + is);
        m_osc[i]->m_stereoPhaseDetuningModel.saveSettings(_doc, _this,
                                                          "stphdetun" + is);
        m_osc[i]->m_pulseCenterModel.saveSettings(_doc, _this,
                                                  "pulse_center" + is);
        m_osc[i]->m_pulseWidthModel.saveSettings(_doc, _this,
                                                 "pulse_width" + is);
        m_osc[i]->m_lfoEnabledModel.saveSettings(_doc, _this,
                                                 "lfo_enabled" + is);
        m_osc[i]->m_lfoTimeModel.saveSettings(_doc, _this, "lfo_time" + is);
        m_osc[i]->m_portamentoModel.saveSettings(_doc, _this,
                                                 "portamento" + is);
    }

    for(int i = 0; i < NB_MODULATORS; ++i)
    {
        QString is = QString::number(i);
        m_mod[i]->m_enabledModel.saveSettings(_doc, _this, "enabled" + is);
        m_mod[i]->m_algoModel.saveSettings(_doc, _this, "algo" + is);
        m_mod[i]->m_modulatedModel.saveSettings(_doc, _this,
                                                "modulated" + is);
        m_mod[i]->m_modulatorModel.saveSettings(_doc, _this,
                                                "modulator" + is);
    }
}

void SynthGDX::loadSettings(const QDomElement& _this)
{
    for(int i = 0; i < NB_OSCILLATORS; ++i)
    {
        const QString is = QString::number(i);
        m_osc[i]->m_enabledModel.loadSettings(_this, "enabled" + is);

        // tmp compat
        m_osc[i]->m_wave1IndexModel.loadSettings(_this, "wave1type" + is, false);
        m_osc[i]->m_wave2IndexModel.loadSettings(_this, "wave2type" + is, false);
        m_osc[i]->m_wave1ReverseModel.loadSettings(_this,
                                                   "wave_reverse" + is, false);
        m_osc[i]->m_wave1AbsoluteModel.loadSettings(_this,
                                                    "wave_absolute" + is, false);
        m_osc[i]->m_wave1OppositeModel.loadSettings(_this,
                                                    "wave_opposite" + is, false);
        m_osc[i]->m_wave1ComplementModel.loadSettings(_this,
                                                      "wave_complement" + is, false);
        m_osc[i]->m_wave1IndexModel.loadSettings(_this, "wave1_shape" + is, false);
        m_osc[i]->m_wave2IndexModel.loadSettings(_this, "wave2_shape" + is, false);

        // correct
        m_osc[i]->m_waveMixModel.loadSettings(_this, "wave_mix" + is);

        m_osc[i]->m_wave1ReverseModel.loadSettings(_this,
                                                   "wave1_reverse" + is);
        m_osc[i]->m_wave1BankModel.loadSettings(_this, "wave1_bank" + is);
        m_osc[i]->m_wave1IndexModel.loadSettings(_this, "wave1_index" + is);
        m_osc[i]->m_wave1AbsoluteModel.loadSettings(_this,
                                                    "wave1_absolute" + is);
        m_osc[i]->m_wave1OppositeModel.loadSettings(_this,
                                                    "wave1_opposite" + is);
        m_osc[i]->m_wave1ComplementModel.loadSettings(
                _this, "wave1_complement" + is);

        m_osc[i]->m_wave2ReverseModel.loadSettings(_this,
                                                   "wave2_reverse" + is);
        m_osc[i]->m_wave2BankModel.loadSettings(_this, "wave2_bank" + is);
        m_osc[i]->m_wave2IndexModel.loadSettings(_this, "wave2_index" + is);
        m_osc[i]->m_wave2AbsoluteModel.loadSettings(_this,
                                                    "wave2_absolute" + is);
        m_osc[i]->m_wave2OppositeModel.loadSettings(_this,
                                                    "wave2_opposite" + is);
        m_osc[i]->m_wave2ComplementModel.loadSettings(
                _this, "wave2_complement" + is);

        m_osc[i]->m_volumeModel.loadSettings(_this, "vol" + is);
        m_osc[i]->m_panModel.loadSettings(_this, "pan" + is);
        m_osc[i]->m_coarseModel.loadSettings(_this, "coarse" + is);
        m_osc[i]->m_fineLeftModel.loadSettings(_this, "finel" + is);
        m_osc[i]->m_fineRightModel.loadSettings(_this, "finer" + is);
        m_osc[i]->m_lowPassModel.loadSettings(_this, "filter_lowpass" + is);
        m_osc[i]->m_highPassModel.loadSettings(_this, "filter_highpass" + is);
        m_osc[i]->m_wallModel.loadSettings(_this, "filter_wall" + is);

        m_osc[i]->m_phaseOffsetModel.loadSettings(_this, "phoffset" + is);
        m_osc[i]->m_stereoPhaseDetuningModel.loadSettings(_this,
                                                          "stphdetun" + is);
        m_osc[i]->m_pulseCenterModel.loadSettings(_this, "pulse_center" + is);
        m_osc[i]->m_pulseWidthModel.loadSettings(_this, "pulse_width" + is);
        m_osc[i]->m_lfoEnabledModel.loadSettings(_this, "lfo_enabled" + is);
        m_osc[i]->m_lfoTimeModel.loadSettings(_this, "lfo_time" + is);
        m_osc[i]->m_portamentoModel.loadSettings(_this, "portamento" + is);
    }

    for(int i = 0; i < NB_MODULATORS; ++i)
    {
        const QString is = QString::number(i);
        m_mod[i]->m_enabledModel.loadSettings(_this, "enabled" + is);
        m_mod[i]->m_algoModel.loadSettings(_this, "algo" + is);
        m_mod[i]->m_modulatedModel.loadSettings(_this, "modulated" + is);
        m_mod[i]->m_modulatorModel.loadSettings(_this, "modulator" + is);
    }
}

QString SynthGDX::nodeName() const
{
    return synthgdx_plugin_descriptor.name;
}

void SynthGDX::playNote(NotePlayHandle* _n, sampleFrame* _buf)
{
    if(_n == NULL)
    {
        qWarning("SynthGDX::playNote _n is null");
        return;
    }

    if(_buf == NULL)
    {
        qWarning("SynthGDX::playNote _buf is null");
        return;
    }

    const fpp_t FPP = Engine::mixer()->framesPerPeriod();
    // const bool    isFinished = _n->isFinished();
    // const bool    isReleased = _n->isReleased();
    const fpp_t   frames = _n->framesLeftForCurrentPeriod();
    const f_cnt_t offset = _n->noteOffset();

    if(offset < 0 || offset >= FPP)
    {
        qWarning("SynthGDX::playNote invalid offset %d", offset);
        return;
    }

    if(frames < 0 || frames > FPP)
    {
        qWarning("SynthGDX::playNote invalid frames %d, offset %d", frames,
                 offset);
        return;
    }

    if(frames == 0)
    {
        qWarning("SynthGDX::playNote zero frames");
        return;
    }

    QMutexLocker locker(&m_mtx);

    SynthGDX::InstrState* state
            = static_cast<SynthGDX::InstrState*>(_n->m_pluginData);
    if(state == NULL)
    {
        state = new SynthGDX::InstrState();

        for(int o = NB_OSCILLATORS - 1; o >= 0; --o)
        {
            state->m_oscState[o].m_frequency
                    = m_osc[o]->m_frequencyModel.value();
            state->m_oscState[o].m_velocity
                    = m_osc[o]->m_velocityModel.value();
            /*
            qInfo("new state n=%p O%d: f=%f v=%f", _n, o,
                  m_osc[o]->m_frequencyModel.value(),
                  m_osc[o]->m_velocityModel.value());
            */
        }
        _n->m_pluginData = state;

        /*
        qTrace("start phases: %f / %f", m_osc[0]->m_phase[0],
               m_osc[0]->m_phase[1]);
        */
    }

    restoreFromState(state);

    // PL_BEGIN("synthgdx computing");
    const int tfp = _n->totalFramesPlayed();
    const int fl  = _n->framesLeft();

    /*
    qTrace("before phases: %f / %f", m_osc[0]->m_phase[0],
           m_osc[0]->m_phase[1]);
    */

    for(f_cnt_t f = offset; f < frames; ++f)
    {
        for(int o = NB_OSCILLATORS - 1; o >= 0; --o)
        {
            float portamento = m_osc[o]->m_portamentoModel.value();
            portamento       = 1.f
                         - (1.f - portamento) * 44.1f
                                   / Engine::mixer()->processingSampleRate();
            float ow = m_osc[o]->m_frequencyModel.value();
            float nw = _n->frequency();
            if(ow != nw)
            {
                ow = nw * (1.f - portamento) + ow * portamento;
                /*
                if(f == 0)
                    qTrace("portamento n=%p p=%f nw=%f ow=%f", _n, portamento,
                           nw, ow);
                */
                m_osc[o]->m_frequencyModel.setAutomatedValue(ow);
            }

            float ov = m_osc[o]->m_velocityModel.value();
            float nv = _n->getVolume() / DefaultVolume;
            if(ov != nv)
            {
                ov = nv * (1.f - portamento) + ov * portamento;
                m_osc[o]->m_velocityModel.setAutomatedValue(nv);
            }
        }

        for(int o = NB_OSCILLATORS - 1; o >= 0; --o)
            m_osc[o]->reset(f);

        for(int m = NB_MODULATORS - 1; m >= 0; --m)
            m_mod[m]->reset(f);

        for(int t = 1; t >= 0; --t)
        {
            for(int o = NB_OSCILLATORS - 1; o >= 0; --o)
            {
                if(m_osc[o]->isUpdated())
                    continue;

                bool ready = true;
                for(int m = NB_MODULATORS - 1; m >= 0; --m)
                {
                    if(m_mod[m]->isApplied())
                        continue;

                    int modulated = m_mod[m]->m_modulatedModel.value();
                    if(modulated != o)
                        continue;
                    if(modulated < 0 || modulated >= NB_OSCILLATORS)
                        continue;

                    int modulator = m_mod[m]->m_modulatorModel.value();
                    if(modulator < 0 || modulator >= NB_OSCILLATORS)
                        continue;

                    ready = false;
                    if(m_osc[modulator]->isUpdated())
                    {
                        // qInfo("apply modulation %d <-- %d",
                        // modulated, modulator);
                        m_mod[m]->apply(m_osc[modulated], m_osc[modulator]);
                    }
                }
                if(ready || t == 0)
                {
                    // qInfo("update oscillator %d (t=%d)", o, t);
                    m_osc[o]->update();
                }
            }
        }

        // if(f == FPP - 1)
        // qTrace("before: signal %f/%f", _buf[f][0], _buf[f][1]);

        const int fadeSz = 1024;  // 256;
        float     fadeIn = (tfp + f - offset < fadeSz
                                ? float(tfp + f - offset) / fadeSz
                                : 1.f);
        float fadeOut = (fl < fadeSz ? float(fl + f - offset) / fadeSz : 1.f);
        _buf[f][0]
                += fadeIn * fadeOut * qBound(-1.f, m_osc[0]->output(0), 1.f);
        _buf[f][1]
                += fadeIn * fadeOut * qBound(-1.f, m_osc[0]->output(1), 1.f);

        // if(fadeOut<1.f)
        //  qTrace("after: signal %f/%f fade %f/%f", _buf[f][0], _buf[f][1],
        //  fadeIn, fadeOut);
    }
    // PL_END("synthgdx computing");

    /*
    qTrace("end phases: %f / %f",
           m_osc[0]->m_phase[0],
           m_osc[0]->m_phase[1]);
    */

    saveToState(state);
    applyRelease(_buf, _n);
    // m_graphModel->setSamples(_buf);
    instrumentTrack()->processAudioBuffer(_buf, frames + offset, _n);
}

void SynthGDX::saveToState(InstrState* _state)
{
    for(int o = NB_OSCILLATORS - 1; o >= 0; --o)
        m_osc[o]->saveToState(&_state->m_oscState[o]);
}

void SynthGDX::restoreFromState(InstrState* _state)
{
    for(int o = NB_OSCILLATORS - 1; o >= 0; --o)
        m_osc[o]->restoreFromState(&_state->m_oscState[o]);
}

void SynthGDX::deleteNotePluginData(NotePlayHandle* _n)
{
    delete static_cast<OscillatorObject::OscState*>(_n->m_pluginData);
    _n->m_pluginData = NULL;  // TMP ???
}

PluginView* SynthGDX::instantiateView(QWidget* _parent)
{
    return new SynthGDXView(this, _parent);
}

void SynthGDX::updateAllDetuning()
{
    for(int i = 0; i < NB_OSCILLATORS; ++i)
    {
        m_osc[i]->updateDetunings(0);
    }
}

extern "C"
{
    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model*, void* _data)
    {
        return new SynthGDX(static_cast<InstrumentTrack*>(_data));
    }
}
