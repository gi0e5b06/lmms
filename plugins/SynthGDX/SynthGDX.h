/*
 * SynthGDX.h - modular synth
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

#ifndef _SYNTHGDX_H
#define _SYNTHGDX_H

#include "AutomatableModel.h"
#include "ComboBoxModel.h"
#include "Instrument.h"
#include "NotePlayHandle.h"
#include "TempoSyncKnobModel.h"
#include "WaveForm.h"
//#include "Graph.h"

// class NotePlayHandle;

// const int NB_FILTERS     = 8;
const int NB_MODULATORS  = 6;
const int NB_OSCILLATORS = 4;
const int SZ_BUFFER      = 48000;

class OscillatorObject : public Model
{
    MM_OPERATORS
    Q_OBJECT

  public:
    OscillatorObject(Model* _parent, int _idx);
    virtual ~OscillatorObject();

    real_t output(const ch_cnt_t _ch);
    void   input1(const ch_cnt_t _ch, const real_t _in);
    void   input2(const ch_cnt_t _ch, const real_t _in);

  protected:
    // note-related state
    class OscState
    {
      public:
        OscState();

        real_t m_frequency;
        real_t m_velocity;
        // const WaveForm* m_wave1;
        // const WaveForm* m_wave2;
        real_t m_phase[2];
        real_t m_phaseOffset[2];
        real_t m_currentOutput[2];
        real_t m_previousOutput[2];
        real_t m_averageOutput[2];
        real_t m_currentInput1[2];
        real_t m_previousInput1[2];
        real_t m_currentInput2[2];
        real_t m_previousInput2[2];
        real_t m_buffer[SZ_BUFFER][2];

        friend class SynthGDX;
    };

    void saveToState(OscState* _state);
    void restoreFromState(OscState* _state);

  private:
    // from user ui
    BoolModel          m_enabledModel;
    BoolModel          m_wave1SymetricModel;
    BoolModel          m_wave1ReverseModel;
    ComboBoxModel      m_wave1BankModel;
    ComboBoxModel      m_wave1IndexModel;
    BoolModel          m_wave1AbsoluteModel;
    BoolModel          m_wave1OppositeModel;
    BoolModel          m_wave1ComplementModel;
    BoolModel          m_wave2SymetricModel;
    BoolModel          m_wave2ReverseModel;
    ComboBoxModel      m_wave2BankModel;
    ComboBoxModel      m_wave2IndexModel;
    BoolModel          m_wave2AbsoluteModel;
    BoolModel          m_wave2OppositeModel;
    BoolModel          m_wave2ComplementModel;
    FloatModel         m_waveMixModel;
    FloatModel         m_waveAntialiasModel;
    FloatModel         m_volumeModel;
    FloatModel         m_panModel;
    FloatModel         m_coarseModel;
    FloatModel         m_fineLeftModel;
    FloatModel         m_fineRightModel;
    FloatModel         m_phaseOffsetModel;
    FloatModel         m_stereoPhaseDetuningModel;
    FloatModel         m_pulseCenterModel;
    FloatModel         m_pulseWidthModel;
    BoolModel          m_lfoEnabledModel;
    TempoSyncKnobModel m_lfoTimeModel;
    FloatModel         m_velocityAmountModel;
    FloatModel         m_harm2Model;
    FloatModel         m_harm3Model;
    FloatModel         m_harm4Model;
    FloatModel         m_harm5Model;
    FloatModel         m_harm6Model;
    FloatModel         m_harm7Model;
    FloatModel         m_skewModel;
    FloatModel         m_smoothModel;
    FloatModel         m_portamentoModel;
    FloatModel         m_lowPassModel;
    FloatModel         m_highPassModel;
    FloatModel         m_wallModel;

    // transient state values
    const WaveForm* m_wave1;
    const WaveForm* m_wave2;
    FloatModel      m_frequencyModel;
    FloatModel      m_velocityModel;
    real_t          m_phase[2];
    real_t          m_phaseOffset[2];
    real_t          m_currentOutput[2];
    real_t          m_previousOutput[2];
    real_t          m_averageOutput[2];
    real_t          m_currentInput1[2];
    real_t          m_previousInput1[2];
    real_t          m_currentInput2[2];
    real_t          m_previousInput2[2];
    real_t          m_buffer[SZ_BUFFER][2];

    // transient frame values
    bool   m_symetric1;
    bool   m_reverse1;
    bool   m_absolute1;
    bool   m_opposite1;
    bool   m_complement1;
    bool   m_symetric2;
    bool   m_reverse2;
    bool   m_absolute2;
    bool   m_opposite2;
    bool   m_complement2;
    real_t m_waveAntialias;
    bool   m_updated;
    real_t m_stereoPhase;
    real_t m_harm2;
    real_t m_harm3;
    real_t m_harm4;
    real_t m_harm5;
    real_t m_harm6;
    real_t m_harm7;
    real_t m_skew;
    real_t m_smooth;
    real_t m_lowPass;
    real_t m_highPass;
    real_t m_wall;
    real_t m_velocity;

    real_t m_waveMixBase[2];
    real_t m_waveMixModAmp[2];
    real_t m_waveMixModVal[2];
    real_t m_volumeBase[2];
    real_t m_volumeModAmp[2];
    real_t m_volumeModVal[2];
    real_t m_detuningBase[2];
    real_t m_detuningModAmp[2];
    real_t m_detuningModVal[2];
    real_t m_phaseOffsetBase[2];
    real_t m_phaseOffsetModAmp[2];
    real_t m_phaseOffsetModVal[2];
    real_t m_frequencyBase[2];
    real_t m_frequencyModAmp[2];
    real_t m_frequencyModVal[2];
    real_t m_toneBase[2];
    real_t m_toneModAmp[2];
    real_t m_toneModVal[2];
    real_t m_pulseCenterBase[2];
    real_t m_pulseCenterModAmp[2];
    real_t m_pulseCenterModVal[2];
    real_t m_pulseWidthBase[2];
    real_t m_pulseWidthModAmp[2];
    real_t m_pulseWidthModVal[2];
    real_t m_addOutputBase[2];
    real_t m_addOutputModAmp[2];
    real_t m_addOutputModVal[2];

    void   reset(const fpp_t _f);
    bool   isUpdated();
    void   update();
    real_t waveAt(ch_cnt_t ch, real_t x, real_t w);

    // bool  syncOK(const real_t _coeff, const ch_cnt_t _ch);
    // real_t syncInit(const ch_cnt_t _ch);

    void updateWaves(const fpp_t _f);
    void updatePulses(const fpp_t _f);
    void updateFrequencies(const fpp_t _f);
    void updateVolumes(const fpp_t _f);
    void updateDetunings(const fpp_t _f);
    void updatePhaseOffsets(const fpp_t _f);

    friend class SynthGDX;
    friend class SynthGDXView;
    friend class OscillatorView;
    friend class ModulatorObject;
};

class ModulatorObject : public Model
{
    MM_OPERATORS
    Q_OBJECT

  public:
    ModulatorObject(Model* _parent, int _idx);
    virtual ~ModulatorObject();

  private:
    BoolModel     m_enabledModel;
    ComboBoxModel m_algoModel;
    ComboBoxModel m_modulatedModel;
    ComboBoxModel m_modulatorModel;

    bool m_applied;

    void reset(f_cnt_t _f);
    bool isApplied();
    void apply(OscillatorObject* modulated, OscillatorObject* modulator);

    friend class SynthGDX;
    friend class SynthGDXView;
    friend class ModulatorView;
};

/*
class FilterObject : public Model
{
    MM_OPERATORS
    Q_OBJECT
};
*/

class PathObject : public Model
{
    MM_OPERATORS
    Q_OBJECT
};

class SynthGDX : public Instrument
{
    MM_OPERATORS
    Q_OBJECT

  public:
    SynthGDX(InstrumentTrack* _track);
    virtual ~SynthGDX();

    // virtual void play(sampleFrame* _buf);
    virtual void playNote(NotePlayHandle* _n, sampleFrame* _buf);
    virtual void deleteNotePluginData(NotePlayHandle* _n);

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    virtual QString nodeName() const;

    // virtual Flags flags() const
    //{
    //  return IsSingleStreamed;
    //}

    virtual f_cnt_t desiredReleaseFrames() const
    {
        return 0;  // 1024;
    }

    virtual PluginView* instantiateView(QWidget* _parent);

  protected slots:
    void updateAllDetuning();

  protected:
    // note-related state
    class InstrState
    {
      public:
        OscillatorObject::OscState m_oscState[NB_OSCILLATORS];
    };

    void saveToState(InstrState* _state);
    void restoreFromState(InstrState* _state);

  private:
    // void processNote(NotePlayHandle* _n, sampleFrame* _buf);

    // FilterObject*     m_fil[NB_FILTERS];
    ModulatorObject*  m_mod[NB_MODULATORS];
    OscillatorObject* m_osc[NB_OSCILLATORS];
    // GraphModel* m_GraphModel;

    QMutex m_mtx;

    friend class SynthGDXView;
};

#endif
