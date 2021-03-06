/*
 * EnvelopeAndLfo.cpp - class EnvelopeAndLfo
 *
 * Copyright (c) 2019      gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "EnvelopeAndLfo.h"

#include "Engine.h"
#include "Mixer.h"
#include "Oscillator.h"
#include "WaveFormStandard.h"

#include <QDomElement>
#include <QTimer>

// minimum number of frames for ENV/LFO stages that mustn't be '0'
const f_cnt_t minimumFrames = 1;

EnvelopeAndLfo::LfoInstances*
        EnvelopeAndLfo::s_lfoInstances
        = nullptr;

void EnvelopeAndLfo::LfoInstances::trigger()
{
    QMutexLocker m(&m_lfoListMutex);
    for(LfoList::Iterator it = m_lfos.begin(); it != m_lfos.end(); ++it)
    {
        (*it)->m_lfoFrame += Engine::mixer()->framesPerPeriod();
        (*it)->m_bad_lfoShapeData = true;
    }
}

void EnvelopeAndLfo::LfoInstances::reset()
{
    QMutexLocker m(&m_lfoListMutex);
    for(LfoList::Iterator it = m_lfos.begin(); it != m_lfos.end(); ++it)
    {
        (*it)->m_lfoFrame         = 0;
        (*it)->m_bad_lfoShapeData = true;
    }
}

void EnvelopeAndLfo::LfoInstances::add(
        EnvelopeAndLfo* lfo)
{
    QMutexLocker m(&m_lfoListMutex);
    m_lfos.append(lfo);
}

void EnvelopeAndLfo::LfoInstances::remove(
        EnvelopeAndLfo* lfo)
{
    QMutexLocker m(&m_lfoListMutex);
    m_lfos.removeAll(lfo);
}

EnvelopeAndLfo::EnvelopeAndLfo(
        real_t _value_for_zero_amount, Model* _parent) :
      Model(_parent, "Envelope"),
      m_used(false),
      m_paramMutex("EnvelopeAndLfo::m_paramMutex", true),
      m_predelayModel(0.,
                      0.,
                      2.,
                      0.001,
                      SECS_PER_ENV_SEGMENT * 1000.,
                      this,
                      tr("Predelay")),
      m_attackModel(0.04,
                    0.,
                    2.,
                    0.001,
                    SECS_PER_ENV_SEGMENT * 1000.,
                    this,
                    tr("Attack")),
      m_holdModel(0.25,
                  0.,
                  2.,
                  0.001,
                  SECS_PER_ENV_SEGMENT * 1000.,
                  this,
                  tr("Hold")),
      m_decayModel(0.5,
                   0.,
                   2.,
                   0.001,
                   SECS_PER_ENV_SEGMENT * 1000.,
                   this,
                   tr("Decay")),
      m_sustainModel(0.5, 0., 1., 0.001, this, tr("Sustain")),
      m_releaseModel(0.1,
                     0.,
                     2.,
                     0.001,
                     SECS_PER_ENV_SEGMENT * 1000.,
                     this,
                     tr("Release")),
      m_amountModel(0., -1., 1., 0.005, this, tr("Modulation")),
      m_valueForZeroAmount(_value_for_zero_amount), m_pahdFrames(0),
      m_rFrames(0), m_pahdEnv(nullptr), m_rEnv(nullptr), m_pahdBufSize(0),
      m_rBufSize(0),
      m_lfoPredelayModel(0., 0., 1., 0.001, this, tr("LFO Predelay")),
      m_lfoAttackModel(0., 0., 1., 0.001, this, tr("LFO Attack")),
      m_lfoSpeedModel(0.1,
                      0.0001,
                      1.,
                      0.0001,
                      SECS_PER_LFO_OSCILLATION * 1000.,
                      this,
                      tr("LFO speed")),
      m_lfoAmountModel(0., -1., 1., 0.005, this, tr("LFO Modulation")),
      // m_lfoWaveModel( SineWave, 0, NumLfoShapes, this, tr( "LFO Wave Shape"
      // ) ),
      m_lfoWaveBankModel(this, tr("LFO Wave Bank")),
      m_lfoWaveIndexModel(this, tr("LFO Wave Index")),
      m_x100Model(false, this, tr("Freq x 100")),
      m_controlEnvAmountModel(false, this, tr("Modulate Envelope")),
      m_outModel(0., -1, 1., 0.0001, this, tr("Envelope Out")),
      m_outBuffer(Engine::mixer()->framesPerPeriod()), m_lfoFrame(0),
      m_lfoAmountIsZero(false), m_lfoShapeData(nullptr)
{
    WaveFormStandard::fillBankModel(m_lfoWaveBankModel);
    WaveFormStandard::fillIndexModel(m_lfoWaveIndexModel,
                                     m_lfoWaveBankModel.value());

    m_outModel.setJournalling(false);
    m_outModel.setFrequentlyUpdated(true);

    m_amountModel.setCenterValue(0);
    m_lfoAmountModel.setCenterValue(0);

    if(s_lfoInstances == nullptr)
    {
        s_lfoInstances = new LfoInstances();
    }

    instances()->add(this);

    connect(&m_predelayModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_attackModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_holdModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_decayModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_sustainModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_releaseModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_amountModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));

    connect(&m_lfoPredelayModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_lfoAttackModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_lfoSpeedModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_lfoAmountModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    // connect( &m_lfoWaveModel, SIGNAL( dataChanged() ),
    //  this, SLOT( updateSampleVars() ) );
    connect(&m_lfoWaveBankModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_lfoWaveIndexModel, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));
    connect(&m_x100Model, SIGNAL(dataChanged()), this,
            SLOT(updateSampleVars()));

    connect(this, SIGNAL(sendOut(const ValueBuffer*)), &m_outModel,
            SLOT(setAutomatedBuffer(const ValueBuffer*)));

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(updateSampleVars()));

    m_lfoShapeData = MM_ALLOC(sample_t, Engine::mixer()->framesPerPeriod());
    //    m_lfoShapeData = new sample_t[Engine::mixer()->framesPerPeriod()];

    updateSampleVars();
}

EnvelopeAndLfo::~EnvelopeAndLfo()
{
    m_predelayModel.disconnect(this);
    m_attackModel.disconnect(this);
    m_holdModel.disconnect(this);
    m_decayModel.disconnect(this);
    m_sustainModel.disconnect(this);
    m_releaseModel.disconnect(this);
    m_amountModel.disconnect(this);
    m_lfoPredelayModel.disconnect(this);
    m_lfoAttackModel.disconnect(this);
    m_lfoSpeedModel.disconnect(this);
    m_lfoAmountModel.disconnect(this);
    // m_lfoWaveModel.disconnect( this );
    m_lfoWaveBankModel.disconnect(this);
    m_lfoWaveIndexModel.disconnect(this);
    m_x100Model.disconnect(this);

    /*
    delete[] m_pahdEnv;
    delete[] m_rEnv;
    delete[] m_lfoShapeData;
    */
    if(m_pahdEnv != nullptr)
        MM_FREE(m_pahdEnv);
    if(m_rEnv != nullptr)
        MM_FREE(m_rEnv);
    if(m_lfoShapeData != nullptr)
        MM_FREE(m_lfoShapeData);

    instances()->remove(this);

    if(instances()->isEmpty())
    {
        delete instances();
        s_lfoInstances = nullptr;
    }
}

inline sample_t EnvelopeAndLfo::lfoShapeSample(fpp_t _frame_offset)
{
    const real_t phase
            = m_lfoOscillationFrames > 0
                      ? positivefraction(real_t(m_lfoFrame + _frame_offset)
                                         / real_t(m_lfoOscillationFrames))
                      : 0.;

    sample_t shape_sample = WaveFormStandard::get(m_lfoWaveBankModel.value(),
                                                  m_lfoWaveIndexModel.value())
                                    ->f(phase);

    /*
    switch( m_lfoWaveModel.value()  )
    {
            case TriangleWave:
                    shape_sample = Oscillator::triangleSample( phase );
                    break;
            case SquareWave:
                    shape_sample = Oscillator::squareSample( phase );
                    break;
            case SawWave:
                    shape_sample = Oscillator::sawSample( phase );
                    break;
            case UserDefinedWave:
                    shape_sample = m_userWave.userWaveSample( phase );
                    break;
            case RandomWave:
                    if( frame == 0 )
                    {
                            m_random = Oscillator::noiseSample( 0. );
                    }
                    shape_sample = m_random;
                    break;
            case SineWave:
            default:
                    shape_sample = Oscillator::sinSample( phase );
                    break;
    }
    */
    return shape_sample * m_lfoAmount;
}

void EnvelopeAndLfo::updateLfoShapeData()
{
    const fpp_t frames = Engine::mixer()->framesPerPeriod();

    for(fpp_t offset = 0; offset < frames; ++offset)
        m_lfoShapeData[offset] = lfoShapeSample(offset);

    m_bad_lfoShapeData = false;
}

inline void EnvelopeAndLfo::fillLfoLevel(real_t*     _buf,
                                                   f_cnt_t     _frame,
                                                   const fpp_t _frames)
{
    if(m_lfoAmountIsZero || _frame <= m_lfoPredelayFrames)
    {
        for(fpp_t offset = 0; offset < _frames; ++offset)
            *_buf++ = 0.;
        return;
    }

    _frame -= m_lfoPredelayFrames;

    if(m_bad_lfoShapeData)
        updateLfoShapeData();

    fpp_t        offset = 0;
    const real_t lafI   = 1. / qMax(minimumFrames, m_lfoAttackFrames);

    for(; offset < _frames && _frame < m_lfoAttackFrames; ++offset, ++_frame)
        *_buf++ = m_lfoShapeData[offset] * _frame * lafI;

    for(; offset < _frames; ++offset)
        *_buf++ = m_lfoShapeData[offset];
}

void EnvelopeAndLfo::fillLevel(real_t*       _buf,
                                         f_cnt_t       _frame,
                                         const f_cnt_t _releaseBegin,
                                         const fpp_t   _frames,
                                         const bool    _legato,
                                         const bool    _marcato,
                                         const bool    _staccato)
{
    QMutexLocker m(&m_paramMutex);

    if(_frame < 0 || _releaseBegin < 0)
        return;

    fillLfoLevel(_buf, _frame, _frames);

    m_outBuffer.lock();
    if(m_outBuffer.period() <= AutomatableModel::periodCounter())
        m_outBuffer.fill(0.);

    f_cnt_t sustainBegin = m_pahdFrames;
    f_cnt_t releaseBegin = _releaseBegin;

    if(_legato)
        sustainBegin = 0;
    if(_staccato)
        releaseBegin
                = qMin(qMax(sustainBegin, releaseBegin / 2), releaseBegin);

    for(fpp_t offset = 0; offset < _frames; ++offset, ++_buf, ++_frame)
    {
        real_t env_level;
        if(_frame < releaseBegin)
        {
            if(_frame < sustainBegin)
            {
                env_level = m_pahdEnv[_frame];
            }
            else
            {
                env_level = m_sustainLevel;
            }

            if(_marcato)
                env_level *= 1. + 2. * (releaseBegin - _frame) / releaseBegin;
        }
        else if((_frame - releaseBegin) < m_rFrames)
        {
            env_level = m_rEnv[_frame - releaseBegin]
                        * ((releaseBegin < m_pahdFrames)
                                   ? m_pahdEnv[releaseBegin]
                                   : m_sustainLevel);
        }
        else
        {
            env_level = 0.;
        }

        // at this point, *_buf is LFO level
        *_buf = bound(-1.,
                      m_controlEnvAmountModel.value()
                              ? env_level * (1. + *_buf)
                              : env_level + *_buf,
                      1.);
        if(offset < m_outBuffer.length())
            m_outBuffer.set(
                    offset,
                    bound(-1., *_buf + m_outBuffer.value(offset), 1.));
    }

    m_outBuffer.setPeriod(AutomatableModel::periodCounter() + 1);
    m_outBuffer.unlock();
    emit sendOut(&m_outBuffer);
}

void EnvelopeAndLfo::saveSettings(QDomDocument& _doc,
                                            QDomElement&  _parent)
{
    m_predelayModel.saveSettings(_doc, _parent, "pdel");
    m_attackModel.saveSettings(_doc, _parent, "att");
    m_holdModel.saveSettings(_doc, _parent, "hold");
    m_decayModel.saveSettings(_doc, _parent, "dec");
    m_sustainModel.saveSettings(_doc, _parent, "sustain");
    m_releaseModel.saveSettings(_doc, _parent, "rel");
    m_amountModel.saveSettings(_doc, _parent, "amt");
    m_lfoPredelayModel.saveSettings(_doc, _parent, "lpdel");
    m_lfoAttackModel.saveSettings(_doc, _parent, "latt");
    m_lfoSpeedModel.saveSettings(_doc, _parent, "lspd");
    m_lfoAmountModel.saveSettings(_doc, _parent, "lamt");
    m_x100Model.saveSettings(_doc, _parent, "x100");
    m_controlEnvAmountModel.saveSettings(_doc, _parent, "ctlenvamt");
    m_outModel.saveSettings(_doc, _parent, "out");

    _parent.setAttribute("userwavefile", m_userWave.audioFile());

    m_lfoWaveBankModel.saveSettings(_doc, _parent, "lfo_wave_bank");
    m_lfoWaveIndexModel.saveSettings(_doc, _parent, "lfo_wave_index");
    m_lfoWaveIndexModel.saveSettings(_doc, _parent, "lshp");  // compatibility
}

void EnvelopeAndLfo::loadSettings(const QDomElement& _this)
{
    m_predelayModel.loadSettings(_this, "pdel");
    m_attackModel.loadSettings(_this, "att");
    m_holdModel.loadSettings(_this, "hold");
    m_decayModel.loadSettings(_this, "dec");
    m_sustainModel.loadSettings(_this, "sustain");
    m_releaseModel.loadSettings(_this, "rel");
    m_amountModel.loadSettings(_this, "amt");
    m_lfoPredelayModel.loadSettings(_this, "lpdel");
    m_lfoAttackModel.loadSettings(_this, "latt");
    m_lfoSpeedModel.loadSettings(_this, "lspd");
    m_lfoAmountModel.loadSettings(_this, "lamt");
    m_x100Model.loadSettings(_this, "x100");
    m_controlEnvAmountModel.loadSettings(_this, "ctlenvamt");
    m_outModel.loadSettings(_this, "out", false);

    if(_this.hasAttribute("lfo_wave_bank"))
        m_lfoWaveBankModel.loadSettings(_this, "lfo_wave_bank");
    else
        m_lfoWaveBankModel.setValue(0);

    // m_lfoWaveModel.loadSettings( _this, "lshp" );
    if(_this.hasAttribute("lfo_wave_index"))
        m_lfoWaveIndexModel.loadSettings(_this, "lfo_wave_index");

    /*	 ### TODO:
            Old reversed sustain kept for backward compatibility
            with 4.15 file format*/

    if(_this.hasAttribute("sus"))
    {
        m_sustainModel.loadSettings(_this, "sus");
        m_sustainModel.setValue(1.0 - m_sustainModel.value());
    }

    // ### TODO:
    /*	// Keep compatibility with version 2.1 file format
            if( _this.hasAttribute( "lfosyncmode" ) )
            {
                    m_lfoSpeedKnob->setSyncMode(
                    ( TempoSyncKnob::TtempoSyncMode ) _this.attribute(
                                                    "lfosyncmode" ).toInt() );
            }*/

    // qInfo("EnvelopeAndLfo::loadSettings 1");
    m_userWave.setAudioFile(_this.attribute("userwavefile"));
    // qInfo("EnvelopeAndLfo::loadSettings 2");
    updateSampleVars();
    // qInfo("EnvelopeAndLfo::loadSettings 3");
}

void EnvelopeAndLfo::updateSampleVars()
{
    // qInfo("EnvelopeAndLfo::updateSampleVars");

    QMutexLocker m(&m_paramMutex);

    const real_t frames_per_env_seg
            = SECS_PER_ENV_SEGMENT * Engine::mixer()->processingSampleRate();

    // TODO: Remove the expKnobVals, time should be linear
    const f_cnt_t predelay_frames = static_cast<f_cnt_t>(
            frames_per_env_seg * expKnobVal(m_predelayModel.value()));

    const f_cnt_t attack_frames
            = qMax(minimumFrames,
                   static_cast<f_cnt_t>(frames_per_env_seg
                                        * expKnobVal(m_attackModel.value())));

    const f_cnt_t hold_frames = static_cast<f_cnt_t>(
            frames_per_env_seg * expKnobVal(m_holdModel.value()));

    const f_cnt_t decay_frames
            = qMax(minimumFrames,
                   static_cast<f_cnt_t>(
                           frames_per_env_seg
                           * expKnobVal(m_decayModel.value()
                                        * (1 - m_sustainModel.value()))));

    m_sustainLevel = m_sustainModel.value();
    m_amount       = m_amountModel.value();

    if(m_amount >= 0)
        m_amountAdd = (1. - m_amount) * m_valueForZeroAmount;
    else
        m_amountAdd = m_valueForZeroAmount;

    m_pahdFrames
            = predelay_frames + attack_frames + hold_frames + decay_frames;
    m_rFrames = static_cast<f_cnt_t>(frames_per_env_seg
                                     * expKnobVal(m_releaseModel.value()));
    m_rFrames = qMax(minimumFrames, m_rFrames);

    if(static_cast<int>(floor(m_amount * 1000.)) == 0)
        m_rFrames = minimumFrames;

    // qInfo("EALP: pahdFrames=%d rFrames=%d", m_pahdFrames, m_rFrames);

    // if the buffers are too small, make bigger ones - so we only alloc new
    // memory when necessary
    if(m_pahdBufSize < m_pahdFrames)
    {
        sample_t* tmp = m_pahdEnv;
        m_pahdEnv     = MM_ALLOC(sample_t, m_pahdFrames);
        if(tmp != nullptr)
            MM_FREE(tmp);
        /* m_pahdEnv = new sample_t[m_pahdFrames]; delete[] tmp; */
        m_pahdBufSize = m_pahdFrames;
    }

    if(m_rBufSize < m_rFrames)
    {
        sample_t* tmp = m_rEnv;
        m_rEnv        = MM_ALLOC(sample_t, m_rFrames);
        if(tmp != nullptr)
            MM_FREE(tmp);
        /* m_rEnv = new sample_t[m_rFrames]; delete[] tmp; */
        m_rBufSize = m_rFrames;
    }

    const real_t aa = m_amountAdd;
    for(f_cnt_t i = 0; i < predelay_frames; ++i)
    {
        m_pahdEnv[i] = aa;
    }

    f_cnt_t add = predelay_frames;

    const real_t afI = (1. / attack_frames) * m_amount;
    for(f_cnt_t i = 0; i < attack_frames; ++i)
    {
        m_pahdEnv[add + i] = i * afI + aa;
    }

    add += attack_frames;
    const real_t amsum = m_amount + m_amountAdd;
    for(f_cnt_t i = 0; i < hold_frames; ++i)
    {
        m_pahdEnv[add + i] = amsum;
    }

    add += hold_frames;
    const real_t dfI = (1.0 / decay_frames) * (m_sustainLevel - 1) * m_amount;
    for(f_cnt_t i = 0; i < decay_frames; ++i)
    {
        /*
                        m_pahdEnv[add + i] = ( m_sustainLevel + ( 1. -
                                                        (real_t)i /
           decay_frames ) * ( 1. - m_sustainLevel ) ) * m_amount +
           m_amountAdd;
        */
        m_pahdEnv[add + i] = amsum + i * dfI;
    }

    const real_t rfI = (1. / m_rFrames) * m_amount;
    for(f_cnt_t i = 0; i < m_rFrames; ++i)
    {
        m_rEnv[i] = (real_t)(m_rFrames - i) * rfI;
    }

    // save this calculation in real-time-part
    m_sustainLevel = m_sustainLevel * m_amount + m_amountAdd;

    const real_t frames_per_lfo_oscillation
            = SECS_PER_LFO_OSCILLATION
              * Engine::mixer()->processingSampleRate();
    m_lfoPredelayFrames
            = static_cast<f_cnt_t>(frames_per_lfo_oscillation
                                   * expKnobVal(m_lfoPredelayModel.value()));
    m_lfoAttackFrames
            = static_cast<f_cnt_t>(frames_per_lfo_oscillation
                                   * expKnobVal(m_lfoAttackModel.value()));
    m_lfoOscillationFrames = static_cast<f_cnt_t>(frames_per_lfo_oscillation
                                                  * m_lfoSpeedModel.value());

    // qInfo("EALP: m_lfoSpeedModel = %f", m_lfoSpeedModel.value());
    // qInfo("EALP: m_lfoOscillationFrames = %d", m_lfoOscillationFrames);

    if(m_x100Model.value())
    {
        m_lfoOscillationFrames /= 100;
    }

    m_lfoAmount = m_lfoAmountModel.value();  // why O.5? * 0.5;

    m_used = true;
    if(static_cast<int>(floor(m_lfoAmount * 1000.)) == 0)
    {
        m_lfoAmountIsZero = true;
        if(static_cast<int>(floor(m_amount * 1000.)) == 0)
        {
            m_used = false;
        }
    }
    else
    {
        m_lfoAmountIsZero = false;
    }

    m_bad_lfoShapeData = true;

    emit dataChanged();
}
