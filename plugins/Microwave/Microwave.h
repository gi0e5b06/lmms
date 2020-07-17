/*
 * Microwave.h - declaration of class Microwave (a wavetable synthesizer)
 *
 * Copyright (c) 2019 Robert Black AKA DouglasDGI AKA Lost Robot
 * <r94231/at/gmail/dot/com>
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

#ifndef MICROWAVE_H
#define MICROWAVE_H

//#include "ComboBox.h"
#include "Graph.h"
#include "Instrument.h"
//#include "InstrumentView.h"
//#include "Knob.h"
//#include "LcdSpinBox.h"
//#include "LedCheckBox.h"
#include "MemoryManager.h"
//#include "PixmapButton.h"
#include "Plugin.h"
#include "SampleBuffer.h"
//#include "stdshims.h"

//#include "embed.h"

#include <QPair>
//#include <QLabel>
//#include <QPlainTextEdit>
//#include <QScrollBar>

class oscillator;
class MicrowaveView;
class MSynth;

// Channels, Stereo (hardcoded...)
#define NB_CHAN 2
// Main table-based oscillators OSC_COUNT main*
#define NB_MAINOSC 8
// Suboscillators SUB_COUNT sub*
#define NB_SUBOSC 64
// Samples SMP_COUNT sample* smp*
#define NB_SMPLR 8
// Modulation matrix MOD_COUNT mod*
#define NB_MODLT 64
// Filters FLT_COUNT filt*
#define NB_FILTR 8
// Macros mac*
#define NB_MACRO 8

// Waveform count per main osc
#define NB_WAVES 32
// Waveform size for main osc and sub osc
#define WAVE_SIZE 2048
// Waveform buffer size 524288
//#define MAINWAV_SIZE (NB_MAINOSC * NB_WAVES * WAVE_SIZE)
#define MAINWAV_SIZE (NB_WAVES * WAVE_SIZE)
// Subosc wav buffer size 131072
#define SUBWAV_SIZE (NB_SUBOSC * WAVE_SIZE)
// SampGraph 1024
#define GRAPHONE_SIZE 128
#define GRAPHALL_SIZE (NB_SMPLR * GRAPHONE_SIZE)
//#define SMPLWAV_SIZEX (NB_SMPLR * 128)

template <typename T>
class StereoPrimitiveBase
{
  public:
    T left;
    T right;

    StereoPrimitiveBase(QString _name = QString::null)
    {
        left = right = 0.;
        m_name       = _name;
    }

    T& operator[](int ch)
    {
        if(ch == 0)
            return left;
        if(ch == 1)
            return right;
        BACKTRACE
        static T error;
        qWarning("StereoPrimitiveBase '%s' invalid channel ch=%d",
                 qPrintable(m_name), ch);
        return error;
    }

  private:
    QString m_name;
};

template <typename T>
class StereoSeparateVectorBase
{
  public:
    QVector<T> left;
    QVector<T> right;

    StereoSeparateVectorBase(int _size = 0, QString _name = QString::null)
    {
        if(_size > 0)
        {
            left.resize(_size);
            right.resize(_size);
        }
        m_name = _name;
    }

    QVector<T>& operator[](int ch)
    {
        if(ch == 0)
            return left;
        if(ch == 1)
            return right;
        BACKTRACE
        static QVector<T> error;
        qWarning("StereoSeparateVectorBase '%s' invalid channel ch=%d",
                 qPrintable(m_name), ch);
        return error;
    }

  private:
    QString m_name;
};

template <typename T, int N>
class StereoInterlacedVectorBase
{
  public:
    QVector<StereoPrimitiveBase<T>> both;

    StereoInterlacedVectorBase(QString _name = QString::null)
    {
        both.resize(N);
        m_name = _name;
    }

    StereoPrimitiveBase<T>& operator[](int i)
    {
        if(i >= 0 && i < both.size())
            return both[i];
        BACKTRACE
        static StereoPrimitiveBase<T> error;
        qWarning("StereoInterlacedVectorBase '%s' invalid index i=%d size=%d",
                 qPrintable(m_name), i, N);
        return error;
    }

  private:
    QString m_name;
};

template <typename T, int N>
class NumArray
{
  public:
    QVector<T> array;

    NumArray(const QString& _name = QString::null)
    {
        array.resize(N);
        array.fill(0);
        m_name = _name;
    }

    T& operator[](int i)
    {
        if(i >= 0 && i < array.size())
            return array[i];
        BACKTRACE
        static T error = 0;
        qWarning("NumArray '%s' invalid index i=%d size=%d",
                 qPrintable(m_name), i, N);
        return error;
    }

    operator T*()
    {
        return array.data();
    }

    operator const T*() const
    {
        return array.data();
    }

  private:
    QString m_name;
};

template <class T, int N>
class PtrArray
{
  public:
    QVector<T*> array;

    PtrArray(const QString& _name = QString::null)
    {
        array.resize(N);
        array.fill(nullptr);
        m_name = _name;
    }

    T*& operator[](int i)
    {
        if(i >= 0 && i < array.size())
            return array[i];
        BACKTRACE
        static T* error = nullptr;
        qWarning("PtrArray '%s' invalid index i=%d size=%d",
                 qPrintable(m_name), i, N);
        return error;
    }

    operator T*()
    {
        return array.data();
    }

    operator const T*() const
    {
        return array.data();
    }

  private:
    QString m_name;
};

template <class T, int N>
class ObjArray
{
  public:
    QVector<T> array;

    ObjArray(const QString& _name = QString::null)
    {
        array.resize(N);
        // array.fill(nullptr);
        m_name = _name;
    }

    T& operator[](int i)
    {
        if(i >= 0 && i < array.size())
            return array[i];
        BACKTRACE
        static T error;  // = nullptr;
        qWarning("ObjArray '%s' invalid index i=%d size=%d",
                 qPrintable(m_name), i, N);
        return error;
    }

    operator T*()
    {
        return array.data();
    }

    operator const T*() const
    {
        return array.data();
    }

  private:
    QString m_name;
};

// typedef sample_t MonoFrame;
// typedef QPair<sample_t, sample_t> StereoFrame;
// typedef std::vector<sample_t> MonoClip;
// typedef std::vector<sample_t> StereoClip[2];
// typedef sample_t MWave[MAINWAV_SIZE];

class StereoSample : public StereoPrimitiveBase<sample_t>
{
};

class StereoReal : public StereoPrimitiveBase<real_t>
{
};

class StereoInt : public StereoPrimitiveBase<int>
{
};

class StereoClip : public ObjArray<std::vector<sample_t>, 2>
{
};

// class StereoSampleBuffer : public StereoSeparateVectorBase<sample_t,
// sample_t>

class Microwave : public Instrument
{
    Q_OBJECT

  public:
    Microwave(InstrumentTrack* _instrument_track);
    virtual ~Microwave();
    virtual PluginView* instantiateView(QWidget* _parent);

    // virtual QString nodeName() const;
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    virtual void playNote(NotePlayHandle* _n, sampleFrame* _working_buffer);
    virtual void deleteNotePluginData(NotePlayHandle* _n);

    virtual f_cnt_t desiredReleaseFrames() const
    {
        return 64;
    }

    void switchMatrixSections(int source, int destination);

  protected slots:
    void valueChanged(int, int);
    void morphMaxChanged();
    void sampLenChanged();
    void subSampLenChanged(int);
    void mainEnabledChanged(int);
    void subEnabledChanged(int);
    void modEnabledChanged(int);
    void filtEnabledChanged(int);
    void sampleEnabledChanged(int);
    void samplesChanged(int, int);

  protected:
    int        currentTab = 0;
    GraphModel graph;
    real_t     sample_realindex[NB_MAINOSC][NB_WAVES] = {{0}};
    real_t     sample_subindex[NB_SUBOSC]             = {0};
    // MWave waveforms[NB_MAINOSC] = {{0}};
    ObjArray<NumArray<sample_t, MAINWAV_SIZE>, NB_MAINOSC> waveforms;

    // sample_t subs[SUBWAV_SIZE] = {0};
    NumArray<sample_t, SUBWAV_SIZE> subs;
    // FLOAT    sampGraphs[1024]  = {0};
    NumArray<FLOAT, GRAPHALL_SIZE> sampGraphs;
    StereoClip                     samples[NB_SMPLR];

    ComboBoxModel loadMode;
    FloatModel    loadAlg;
    FloatModel    loadChnl;
    FloatModel    scroll;
    ComboBoxModel oversample;
    BoolModel     visualize;
    FloatModel    visvol;
    FloatModel    wtLoad1;
    FloatModel    wtLoad2;
    FloatModel    wtLoad3;
    FloatModel    wtLoad4;
    BoolModel     mainFlipped;
    BoolModel     subFlipped;

    IntModel mainNum;
    IntModel subNum;
    IntModel sampNum;
    IntModel modNum;

    // FloatModel* morph[NB_MAINOSC];
    PtrArray<FloatModel, NB_MAINOSC> morph;
    FloatModel*                      range[NB_MAINOSC];
    FloatModel*                      modify[NB_MAINOSC];
    ComboBoxModel*                   modifyMode[NB_MAINOSC];
    // FloatModel* sampLen[NB_MAINOSC];
    PtrArray<FloatModel, NB_MAINOSC> sampLen;
    BoolModel*                       enabled[NB_MAINOSC];
    BoolModel*                       muted[NB_MAINOSC];
    FloatModel*                      detune[NB_MAINOSC];
    FloatModel*                      unisonVoices[NB_MAINOSC];
    FloatModel*                      unisonDetune[NB_MAINOSC];
    FloatModel*                      unisonMorph[NB_MAINOSC];
    FloatModel*                      unisonModify[NB_MAINOSC];
    FloatModel*                      morphMax[NB_MAINOSC];
    FloatModel*                      phase[NB_MAINOSC];
    FloatModel*                      phaseRand[NB_MAINOSC];
    FloatModel*                      vol[NB_MAINOSC];
    FloatModel*                      pan[NB_MAINOSC];

    BoolModel*  subEnabled[NB_SUBOSC];
    FloatModel* subVol[NB_SUBOSC];
    FloatModel* subPhase[NB_SUBOSC];
    FloatModel* subPhaseRand[NB_SUBOSC];
    BoolModel*  subMuted[NB_SUBOSC];
    BoolModel*  subKeytrack[NB_SUBOSC];
    FloatModel* subDetune[NB_SUBOSC];
    FloatModel* subSampLen[NB_SUBOSC];
    BoolModel*  subNoise[NB_SUBOSC];
    FloatModel* subPanning[NB_SUBOSC];
    FloatModel* subTempo[NB_SUBOSC];

    ComboBoxModel* modOutSec[NB_MODLT];
    ComboBoxModel* modOutSig[NB_MODLT];
    IntModel*      modOutSecNum[NB_MODLT];
    ComboBoxModel* modIn[NB_MODLT];
    IntModel*      modInNum[NB_MODLT];
    IntModel*      modInOtherNum[NB_MODLT];
    FloatModel*    modInAmnt[NB_MODLT];
    FloatModel*    modInCurve[NB_MODLT];
    ComboBoxModel* modIn2[NB_MODLT];
    IntModel*      modInNum2[NB_MODLT];
    IntModel*      modInOtherNum2[NB_MODLT];
    FloatModel*    modInAmnt2[NB_MODLT];
    FloatModel*    modInCurve2[NB_MODLT];
    BoolModel*     modEnabled[NB_MODLT];
    ComboBoxModel* modCombineType[NB_MODLT];
    BoolModel*     modType[NB_MODLT];

    FloatModel*    filtInVol[NB_FILTR];
    ComboBoxModel* filtType[NB_FILTR];
    ComboBoxModel* filtSlope[NB_FILTR];
    FloatModel*    filtCutoff[NB_FILTR];
    FloatModel*    filtReso[NB_FILTR];
    FloatModel*    filtGain[NB_FILTR];
    FloatModel*    filtSatu[NB_FILTR];
    FloatModel*    filtWetDry[NB_FILTR];
    FloatModel*    filtBal[NB_FILTR];
    FloatModel*    filtOutVol[NB_FILTR];
    BoolModel*     filtEnabled[NB_FILTR];
    FloatModel*    filtFeedback[NB_FILTR];
    // This changes the feedback delay time to change the pitch
    FloatModel* filtDetune[NB_FILTR];
    BoolModel*  filtKeytracking[NB_FILTR];
    BoolModel*  filtMuted[NB_FILTR];

    BoolModel*  sampleEnabled[NB_SMPLR];
    BoolModel*  sampleGraphEnabled[NB_SMPLR];
    BoolModel*  sampleMuted[NB_SMPLR];
    BoolModel*  sampleKeytracking[NB_SMPLR];
    BoolModel*  sampleLoop[NB_SMPLR];
    FloatModel* sampleVolume[NB_SMPLR];
    FloatModel* samplePanning[NB_SMPLR];
    FloatModel* sampleDetune[NB_SMPLR];
    FloatModel* samplePhase[NB_SMPLR];
    FloatModel* samplePhaseRand[NB_SMPLR];
    FloatModel* sampleStart[NB_SMPLR];
    FloatModel* sampleEnd[NB_SMPLR];

    FloatModel* macro[NB_MACRO];

    // SampleBuffer sampleBuffer;

    // Below is for passing to mSynth initialization
    int32_t modifyModeArr[NB_MAINOSC]   = {0};
    real_t  modifyArr[NB_MAINOSC]       = {0};
    real_t  morphArr[NB_MAINOSC]        = {0};
    real_t  rangeArr[NB_MAINOSC]        = {0};
    int32_t unisonVoicesArr[NB_MAINOSC] = {0};
    int32_t unisonDetuneArr[NB_MAINOSC] = {0};
    real_t  unisonMorphArr[NB_MAINOSC]  = {0};
    real_t  unisonModifyArr[NB_MAINOSC] = {0};
    real_t  morphMaxArr[NB_MAINOSC]     = {0};
    int32_t detuneArr[NB_MAINOSC]       = {0};
    int32_t sampLenArr[NB_MAINOSC]      = {0};
    real_t  phaseArr[NB_MAINOSC]        = {0};
    real_t  phaseRandArr[NB_MAINOSC]    = {0};
    real_t  volArr[NB_MAINOSC]          = {0};
    bool    enabledArr[NB_MAINOSC]      = {false};
    bool    mutedArr[NB_MAINOSC]        = {false};
    real_t  panArr[NB_MAINOSC]          = {0};

    bool    subEnabledArr[NB_SUBOSC]   = {false};
    real_t  subVolArr[NB_SUBOSC]       = {0};
    real_t  subPhaseArr[NB_SUBOSC]     = {0};
    real_t  subPhaseRandArr[NB_SUBOSC] = {0};
    int32_t subDetuneArr[NB_SUBOSC]    = {0};
    bool    subMutedArr[NB_SUBOSC]     = {false};
    bool    subKeytrackArr[NB_SUBOSC]  = {false};
    int32_t subSampLenArr[NB_SUBOSC]   = {0};
    bool    subNoiseArr[NB_SUBOSC]     = {false};
    real_t  subPanningArr[NB_SUBOSC]   = {0};
    int32_t subTempoArr[NB_SUBOSC]     = {0};

    bool    sampleEnabledArr[NB_SMPLR]      = {false};
    bool    sampleGraphEnabledArr[NB_SMPLR] = {false};
    bool    sampleMutedArr[NB_SMPLR]        = {false};
    bool    sampleKeytrackingArr[NB_SMPLR]  = {false};
    bool    sampleLoopArr[NB_SMPLR]         = {false};
    real_t  sampleVolumeArr[NB_SMPLR]       = {0};
    real_t  samplePanningArr[NB_SMPLR]      = {0};
    int32_t sampleDetuneArr[NB_SMPLR]       = {0};
    real_t  samplePhaseArr[NB_SMPLR]        = {0};
    real_t  samplePhaseRandArr[NB_SMPLR]    = {0};
    real_t  sampleStartArr[NB_SMPLR]        = {0};
    real_t  sampleEndArr[NB_SMPLR]          = {1, 1, 1, 1, 1, 1, 1, 1};

    int32_t modInArr[NB_MODLT]          = {0};
    int32_t modInNumArr[NB_MODLT]       = {0};
    int32_t modInOtherNumArr[NB_MODLT]  = {0};
    real_t  modInAmntArr[NB_MODLT]      = {0};
    real_t  modInCurveArr[NB_MODLT]     = {0};
    int32_t modIn2Arr[NB_MODLT]         = {0};
    int32_t modInNum2Arr[NB_MODLT]      = {0};
    int32_t modInOtherNum2Arr[NB_MODLT] = {0};
    real_t  modInAmnt2Arr[NB_MODLT]     = {0};
    real_t  modInCurve2Arr[NB_MODLT]    = {0};
    int32_t modOutSecArr[NB_MODLT]      = {0};
    int32_t modOutSigArr[NB_MODLT]      = {0};
    int32_t modOutSecNumArr[NB_MODLT]   = {0};
    bool    modEnabledArr[NB_MODLT]     = {false};
    int32_t modCombineTypeArr[NB_MODLT] = {0};
    bool    modTypeArr[NB_MODLT]        = {0};

    real_t  filtInVolArr[NB_FILTR]       = {0};
    int32_t filtTypeArr[NB_FILTR]        = {0};
    int32_t filtSlopeArr[NB_FILTR]       = {0};
    real_t  filtCutoffArr[NB_FILTR]      = {0};
    real_t  filtResoArr[NB_FILTR]        = {0};
    real_t  filtGainArr[NB_FILTR]        = {0};
    real_t  filtSatuArr[NB_FILTR]        = {0};
    real_t  filtWetDryArr[NB_FILTR]      = {0};
    real_t  filtBalArr[NB_FILTR]         = {0};
    real_t  filtOutVolArr[NB_FILTR]      = {0};
    bool    filtEnabledArr[NB_FILTR]     = {false};
    real_t  filtFeedbackArr[NB_FILTR]    = {0};
    int32_t filtDetuneArr[NB_FILTR]      = {0};
    bool    filtKeytrackingArr[NB_FILTR] = {false};
    bool    filtMutedArr[NB_FILTR]       = {false};

    real_t macroArr[NB_MACRO] = {0};
    // Above is for passing to mSynth initialization

    int32_t maxMainEnabled = 0;    // The highest number of main oscillator
                                   // sections that must be looped through
    int32_t maxSubEnabled = 0;     // The highest number of sub oscillator
                                   // sections that must be looped through
    int32_t maxSampleEnabled = 0;  // The highest number of sample sections
                                   // that must be looped through
    int32_t maxModEnabled = 0;     // The highest number of matrix sections
                                   // that must be looped through
    int32_t maxFiltEnabled = 0;    // The highest number of filter sections
                                   // that must be looped through

    InstrumentTrack* microwaveTrack = this->instrumentTrack();

    friend class MicrowaveView;
    friend class MSynth;
};

#define setwavemodel(name)                                                  \
    name->addItem(tr("None"), new PluginPixmapLoader("none"));              \
    name->addItem(tr("Pulse Width"), new PluginPixmapLoader("sin"));        \
    name->addItem(tr("Weird 1"), new PluginPixmapLoader("noise"));          \
    name->addItem(tr("Weird 2"), new PluginPixmapLoader("noise"));          \
    name->addItem(tr("Asym To Right"), new PluginPixmapLoader("saw"));      \
    name->addItem(tr("Asym To Left"), new PluginPixmapLoader("ramp"));      \
    name->addItem(tr("Stretch From Center"),                                \
                  new PluginPixmapLoader("sinabs"));                        \
    name->addItem(tr("Squish To Center"), new PluginPixmapLoader("exp"));   \
    name->addItem(tr("Stretch And Squish"), new PluginPixmapLoader("tri")); \
    name->addItem(tr("Cut Off Right"), new PluginPixmapLoader("saw"));      \
    name->addItem(tr("Cut Off Left"), new PluginPixmapLoader("ramp"));      \
    name->addItem(tr("Squarify"), new PluginPixmapLoader("sqr"));           \
    name->addItem(tr("Pulsify"), new PluginPixmapLoader("sqr"));            \
    name->addItem(tr("Flip"), new PluginPixmapLoader("sqr"));               \
    name->addItem(tr("Clip"), new PluginPixmapLoader("sqr"));               \
    name->addItem(tr("Inverse Clip"), new PluginPixmapLoader("sqr"));       \
    name->addItem(tr("Sine"), new PluginPixmapLoader("sin"));               \
    name->addItem(tr("Atan"), new PluginPixmapLoader("tri"));

#define modsectionsmodel(name)                                             \
    name->addItem(tr("None"), new PluginPixmapLoader("none"));             \
    name->addItem(tr("Main OSC"), new PluginPixmapLoader("sin"));          \
    name->addItem(tr("Sub OSC"), new PluginPixmapLoader("sqr"));           \
    name->addItem(tr("Sample OSC"), new PluginPixmapLoader("noise"));      \
    name->addItem(tr("Matrix"), new PluginPixmapLoader("ramp"));           \
    name->addItem(tr("Filter Input"), new PluginPixmapLoader("moog"));     \
    name->addItem(tr("Filter Parameters"), new PluginPixmapLoader("saw")); \
    name->addItem(tr("Macro"), new PluginPixmapLoader("letter_m"));

#define mainoscsignalsmodel(name)                                       \
    name->addItem(tr("None"), new PluginPixmapLoader("none"));          \
    name->addItem(tr("Morph"), new PluginPixmapLoader("tri"));          \
    name->addItem(tr("Range"), new PluginPixmapLoader("sqr"));          \
    name->addItem(tr("Modify"), new PluginPixmapLoader("moog"));        \
    name->addItem(tr("Detune"), new PluginPixmapLoader("saw"));         \
    name->addItem(tr("Phase"), new PluginPixmapLoader("sin"));          \
    name->addItem(tr("Volume"), new PluginPixmapLoader("ramp"));        \
    name->addItem(tr("Panning"), new PluginPixmapLoader("ramp"));       \
    name->addItem(tr("Unison Number"), new PluginPixmapLoader("ramp")); \
    name->addItem(tr("Unison Detune"), new PluginPixmapLoader("saw"));  \
    name->addItem(tr("Unison Morph"), new PluginPixmapLoader("tri"));   \
    name->addItem(tr("Unison Modify"), new PluginPixmapLoader("moog"));

#define subsignalsmodel(name)                                   \
    name->addItem(tr("None"), new PluginPixmapLoader("none"));  \
    name->addItem(tr("Detune"), new PluginPixmapLoader("saw")); \
    name->addItem(tr("Phase"), new PluginPixmapLoader("sin"));  \
    name->addItem(tr("Volume"), new PluginPixmapLoader("ramp"));

#define samplesignalsmodel(name)                                  \
    name->addItem(tr("None"), new PluginPixmapLoader("none"));    \
    name->addItem(tr("Detune"), new PluginPixmapLoader("saw"));   \
    name->addItem(tr("Phase"), new PluginPixmapLoader("sin"));    \
    name->addItem(tr("Volume"), new PluginPixmapLoader("ramp"));  \
    name->addItem(tr("Panning"), new PluginPixmapLoader("ramp")); \
    name->addItem(tr("Length"), new PluginPixmapLoader("sin"));

#define matrixsignalsmodel(name)                                          \
    name->addItem(tr("None"), new PluginPixmapLoader("none"));            \
    name->addItem(tr("Amount"), new PluginPixmapLoader("sin"));           \
    name->addItem(tr("Curve"), new PluginPixmapLoader("moog"));           \
    name->addItem(tr("Secondary Amount"), new PluginPixmapLoader("sin")); \
    name->addItem(tr("Secondary Curve"), new PluginPixmapLoader("moog"));

#define filtersignalsmodel(name)                                           \
    name->addItem(tr("None"), new PluginPixmapLoader("none"));             \
    name->addItem(tr("Cutoff Frequency"), new PluginPixmapLoader("moog")); \
    name->addItem(tr("Resonance"), new PluginPixmapLoader("ramp"));        \
    name->addItem(tr("db Gain"), new PluginPixmapLoader("ramp"));          \
    name->addItem(tr("Filter Type"), new PluginPixmapLoader("ramp"));      \
    name->addItem(tr("Slope"), new PluginPixmapLoader("ramp"));            \
    name->addItem(tr("Input Volume"), new PluginPixmapLoader("sin"));      \
    name->addItem(tr("Output Volume"), new PluginPixmapLoader("ramp"));    \
    name->addItem(tr("Wet/Dry"), new PluginPixmapLoader("ramp"));          \
    name->addItem(tr("Balance/Panning"), new PluginPixmapLoader("ramp"));  \
    name->addItem(tr("Saturation"), new PluginPixmapLoader("ramp"));       \
    name->addItem(tr("Feedback"), new PluginPixmapLoader("ramp"));         \
    name->addItem(tr("Detune"), new PluginPixmapLoader("ramp"));

#define modinmodel(name)                                                \
    name->addItem(tr("None"), new PluginPixmapLoader("none"));          \
    name->addItem(tr("Main OSC"), new PluginPixmapLoader("sqr"));       \
    name->addItem(tr("Sub OSC"), new PluginPixmapLoader("sin"));        \
    name->addItem(tr("Sample OSC"), new PluginPixmapLoader("noise"));   \
    name->addItem(tr("Filter Output"), new PluginPixmapLoader("moog")); \
    name->addItem(tr("Velocity"), new PluginPixmapLoader("saw"));       \
    name->addItem(tr("Panning"), new PluginPixmapLoader("ramp"));       \
    name->addItem(tr("Humanizer"), new PluginPixmapLoader("sinabs"));   \
    name->addItem(tr("Macro"), new PluginPixmapLoader("letter_m"));

#define mod8model(name)                                         \
    name->addItem(tr("1"), new PluginPixmapLoader("number_1")); \
    name->addItem(tr("2"), new PluginPixmapLoader("number_2")); \
    name->addItem(tr("3"), new PluginPixmapLoader("number_3")); \
    name->addItem(tr("4"), new PluginPixmapLoader("number_4")); \
    name->addItem(tr("5"), new PluginPixmapLoader("number_5")); \
    name->addItem(tr("6"), new PluginPixmapLoader("number_6")); \
    name->addItem(tr("7"), new PluginPixmapLoader("number_7")); \
    name->addItem(tr("8"), new PluginPixmapLoader("number_8"));

#define filtertypesmodel(name)                                              \
    name->addItem(tr("Lowpass"), new PluginPixmapLoader("filter_lowpass")); \
    name->addItem(tr("Highpass"),                                           \
                  new PluginPixmapLoader("filter_highpass"));               \
    name->addItem(tr("Bandpass"),                                           \
                  new PluginPixmapLoader("filter_bandpass"));               \
    name->addItem(tr("Low Shelf"),                                          \
                  new PluginPixmapLoader("filter_lowshelf"));               \
    name->addItem(tr("High Shelf"),                                         \
                  new PluginPixmapLoader("filter_highshelf"));              \
    name->addItem(tr("Peak"), new PluginPixmapLoader("filter_peak"));       \
    name->addItem(tr("Notch"), new PluginPixmapLoader("filter_notch"));     \
    name->addItem(tr("Allpass"), new PluginPixmapLoader("filter_allpass")); \
    name->addItem(tr("Moog Lowpass (Note: Slope is doubled)"),              \
                  new PluginPixmapLoader("filter_moog"));

#define filterslopesmodel(name)                                     \
    name->addItem(tr("12 db"), new PluginPixmapLoader("number_1")); \
    name->addItem(tr("24 db"), new PluginPixmapLoader("number_2")); \
    name->addItem(tr("36 db"), new PluginPixmapLoader("number_3")); \
    name->addItem(tr("48 db"), new PluginPixmapLoader("number_4")); \
    name->addItem(tr("60 db"), new PluginPixmapLoader("number_5")); \
    name->addItem(tr("72 db"), new PluginPixmapLoader("number_6")); \
    name->addItem(tr("84 db"), new PluginPixmapLoader("number_7")); \
    name->addItem(tr("96 db"), new PluginPixmapLoader("number_8"));

#define modcombinetypemodel(name)                      \
    name->addItem(tr("Add Bidirectional"),             \
                  new PluginPixmapLoader("number_1")); \
    name->addItem(tr("Multiply Bidirectional"),        \
                  new PluginPixmapLoader("number_2")); \
    name->addItem(tr("Add Unidirectional"),            \
                  new PluginPixmapLoader("number_3")); \
    name->addItem(tr("Multiply Unidirectional"),       \
                  new PluginPixmapLoader("number_"     \
                                         "4"));

#define oversamplemodel(name)                                   \
    name.addItem(tr("1x"), new PluginPixmapLoader("number_1")); \
    name.addItem(tr("2x"), new PluginPixmapLoader("number_2")); \
    name.addItem(tr("3x"), new PluginPixmapLoader("number_3")); \
    name.addItem(tr("4x"), new PluginPixmapLoader("number_4")); \
    name.addItem(tr("5x"), new PluginPixmapLoader("number_5")); \
    name.addItem(tr("6x"), new PluginPixmapLoader("number_6")); \
    name.addItem(tr("7x"), new PluginPixmapLoader("number_7")); \
    name.addItem(tr("8x"), new PluginPixmapLoader("number_8"));

#define loadmodemodel(name)                          \
    name.addItem(tr("Load sample without changes")); \
    name.addItem(tr("Load wavetable file"));         \
    name.addItem(tr("Autocorrelation method"));      \
    name.addItem(tr("Choose waveform length"));      \
    name.addItem(tr("Similarity Test Method"));      \
    name.addItem(tr("6x"));                          \
    name.addItem(tr("7x"));                          \
    name.addItem(tr("8x"));

#endif
