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

#include "ComboBox.h"
#include "Graph.h"
#include "Instrument.h"
#include "InstrumentView.h"
#include "Knob.h"
#include "LcdSpinBox.h"
#include "LedCheckBox.h"
#include "MemoryManager.h"
#include "PixmapButton.h"
#include "Plugin.h"
#include "SampleBuffer.h"
//#include "stdshims.h"

#include "embed.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QScrollBar>

class oscillator;
class MicrowaveView;

class Microwave : public Instrument
{
    Q_OBJECT

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
        return (64);
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

  private:
    FloatModel*    morph[8];
    FloatModel*    range[8];
    FloatModel*    sampLen[8];
    FloatModel     visvol;
    FloatModel*    morphMax[8];
    ComboBoxModel* modifyMode[8];
    FloatModel*    modify[8];
    ComboBoxModel* modOutSec[64];
    ComboBoxModel* modOutSig[64];
    IntModel*      modOutSecNum[64];
    ComboBoxModel* modIn[64];
    IntModel*      modInNum[64];
    IntModel*      modInOtherNum[64];
    FloatModel*    modInAmnt[64];
    FloatModel*    modInCurve[64];
    ComboBoxModel* modIn2[64];
    IntModel*      modInNum2[64];
    IntModel*      modInOtherNum2[64];
    FloatModel*    modInAmnt2[64];
    FloatModel*    modInCurve2[64];
    BoolModel*     modEnabled[64];
    ComboBoxModel* modCombineType[64];
    BoolModel*     modType[64];
    IntModel       modNum;
    FloatModel*    unisonVoices[8];
    FloatModel*    unisonDetune[8];
    FloatModel*    unisonMorph[8];
    FloatModel*    unisonModify[8];
    FloatModel*    detune[8];
    FloatModel     loadAlg;
    FloatModel     loadChnl;
    FloatModel*    phase[8];
    FloatModel*    phaseRand[8];
    FloatModel*    vol[8];
    BoolModel*     enabled[8];
    BoolModel*     muted[8];
    FloatModel     scroll;
    IntModel       subNum;
    IntModel       sampNum;
    IntModel       mainNum;
    ComboBoxModel  oversample;
    ComboBoxModel  loadMode;

    FloatModel* pan[8];

    FloatModel*    filtInVol[8];
    ComboBoxModel* filtType[8];
    ComboBoxModel* filtSlope[8];
    FloatModel*    filtCutoff[8];
    FloatModel*    filtReso[8];
    FloatModel*    filtGain[8];
    FloatModel*    filtSatu[8];
    FloatModel*    filtWetDry[8];
    FloatModel*    filtBal[8];
    FloatModel*    filtOutVol[8];
    BoolModel*     filtEnabled[8];
    FloatModel*    filtFeedback[8];
    FloatModel*    filtDetune[8];  // This changes the feedback delay time to
                                   // change the pitch
    BoolModel* filtKeytracking[8];
    BoolModel* filtMuted[8];

    GraphModel graph;

    BoolModel   visualize;
    BoolModel*  subEnabled[64];
    FloatModel* subVol[64];
    FloatModel* subPhase[64];
    FloatModel* subPhaseRand[64];
    BoolModel*  subMuted[64];
    BoolModel*  subKeytrack[64];
    FloatModel* subDetune[64];
    FloatModel* subSampLen[64];
    BoolModel*  subNoise[64];
    FloatModel* subPanning[64];
    FloatModel* subTempo[64];

    real_t             sample_realindex[8][32] = {{0}};
    real_t             sample_subindex[64]     = {0};
    FLOAT              waveforms[8][524288]    = {{0}};
    int                currentTab              = 0;
    FLOAT              subs[131072]            = {0};
    FLOAT              sampGraphs[1024]        = {0};
    std::vector<FLOAT> samples[8][2];

    BoolModel* sampleEnabled[8];
    BoolModel* sampleGraphEnabled[8];
    BoolModel* sampleMuted[8];
    BoolModel* sampleKeytracking[8];
    BoolModel* sampleLoop[8];

    FloatModel* sampleVolume[8];
    FloatModel* samplePanning[8];
    FloatModel* sampleDetune[8];
    FloatModel* samplePhase[8];
    FloatModel* samplePhaseRand[8];
    FloatModel* sampleStart[8];
    FloatModel* sampleEnd[8];

    FloatModel wtLoad1;
    FloatModel wtLoad2;
    FloatModel wtLoad3;
    FloatModel wtLoad4;

    BoolModel mainFlipped;
    BoolModel subFlipped;

    FloatModel* macro[8];

    SampleBuffer sampleBuffer;

    // Below is for passing to mSynth initialization
    int    modifyModeArr[8]      = {0};
    real_t modifyArr[8]          = {0};
    real_t morphArr[8]           = {0};
    real_t rangeArr[8]           = {0};
    real_t unisonVoicesArr[8]    = {0};
    real_t unisonDetuneArr[8]    = {0};
    real_t unisonMorphArr[8]     = {0};
    real_t unisonModifyArr[8]    = {0};
    real_t morphMaxArr[8]        = {0};
    real_t detuneArr[8]          = {0};
    int    sampLenArr[8]         = {0};
    int    modInArr[64]          = {0};
    int    modInNumArr[64]       = {0};
    int    modInOtherNumArr[64]  = {0};
    real_t modInAmntArr[64]      = {0};
    real_t modInCurveArr[64]     = {0};
    int    modIn2Arr[64]         = {0};
    int    modInNum2Arr[64]      = {0};
    int    modInOtherNum2Arr[64] = {0};
    real_t modInAmnt2Arr[64]     = {0};
    real_t modInCurve2Arr[64]    = {0};
    int    modOutSecArr[64]      = {0};
    int    modOutSigArr[64]      = {0};
    int    modOutSecNumArr[64]   = {0};
    bool   modEnabledArr[64]     = {false};
    int    modCombineTypeArr[64] = {0};
    bool   modTypeArr[64]        = {0};
    real_t phaseArr[8]           = {0};
    real_t phaseRandArr[8]       = {0};
    real_t volArr[8]             = {0};
    bool   enabledArr[8]         = {false};
    bool   mutedArr[8]           = {false};

    real_t panArr[8] = {0};

    bool   subEnabledArr[64]   = {false};
    real_t subVolArr[64]       = {0};
    real_t subPhaseArr[64]     = {0};
    real_t subPhaseRandArr[64] = {0};
    real_t subDetuneArr[64]    = {0};
    bool   subMutedArr[64]     = {false};
    bool   subKeytrackArr[64]  = {false};
    real_t subSampLenArr[64]   = {0};
    bool   subNoiseArr[64]     = {false};
    real_t subPanningArr[64]   = {0};
    real_t subTempoArr[64]     = {0};

    real_t filtInVolArr[8]       = {0};
    int    filtTypeArr[8]        = {0};
    int    filtSlopeArr[8]       = {0};
    real_t filtCutoffArr[8]      = {0};
    real_t filtResoArr[8]        = {0};
    real_t filtGainArr[8]        = {0};
    real_t filtSatuArr[8]        = {0};
    real_t filtWetDryArr[8]      = {0};
    real_t filtBalArr[8]         = {0};
    real_t filtOutVolArr[8]      = {0};
    bool   filtEnabledArr[8]     = {false};
    real_t filtFeedbackArr[8]    = {0};
    real_t filtDetuneArr[8]      = {0};
    bool   filtKeytrackingArr[8] = {false};
    bool   filtMutedArr[8]       = {false};

    bool sampleEnabledArr[8]      = {false};
    bool sampleGraphEnabledArr[8] = {false};
    bool sampleMutedArr[8]        = {false};
    bool sampleKeytrackingArr[8]  = {false};
    bool sampleLoopArr[8]         = {false};

    real_t sampleVolumeArr[8]    = {0};
    real_t samplePanningArr[8]   = {0};
    real_t sampleDetuneArr[8]    = {0};
    real_t samplePhaseArr[8]     = {0};
    real_t samplePhaseRandArr[8] = {0};
    real_t sampleStartArr[8]     = {0};
    real_t sampleEndArr[8]       = {1, 1, 1, 1, 1, 1, 1, 1};

    real_t macroArr[8] = {0};
    // Above is for passing to mSynth initialization

    int maxMainEnabled = 0;  // The highest number of main oscillator sections
                             // that must be looped through
    int maxModEnabled = 0;  // The highest number of matrix sections that must
                            // be looped through
    int maxSubEnabled = 0;  // The highest number of sub oscillator sections
                            // that must be looped through
    int maxSampleEnabled = 0;  // The highest number of sample sections that
                               // must be looped through
    int maxFiltEnabled = 0;    // The highest number of filter sections that
                               // must be looped through

    InstrumentTrack* microwaveTrack = this->instrumentTrack();

    friend class MicrowaveView;
    friend class mSynth;
};

class MicrowaveView : public InstrumentView
{
    Q_OBJECT
  public:
    MicrowaveView(Instrument* _instrument, QWidget* _parent);

    virtual ~MicrowaveView(){};

  protected slots:
    void updateScroll();
    void scrollReleased();
    void mainNumChanged();
    void subNumChanged();
    void sampNumChanged();
    void modOutSecChanged(int i);
    void modInChanged(int i);
    void tabChanged(int tabnum);
    void visualizeToggled(bool value);
    void sinWaveClicked();
    void triangleWaveClicked();
    void sqrWaveClicked();
    void sawWaveClicked();
    void noiseWaveClicked();
    void usrWaveClicked();
    void smoothClicked(void);
    void chooseWavetableFile();
    void openWavetableFile(QString fileName = "");
    void openWavetableFileBtnClicked();
    void openSampleFile();
    void openSampleFileBtnClicked();

    void modUpClicked(int);
    void modDownClicked(int);

    void confirmWavetableLoadClicked();

    void tabBtnClicked(int);

    void manualBtnClicked();

    void updateBackground();

    void flipperClicked();

    void XBtnClicked();

  private:
    virtual void modelChanged();

    void         mouseMoveEvent(QMouseEvent* _me);
    void         wheelEvent(QWheelEvent* _me);
    virtual void dropEvent(QDropEvent* _de);
    virtual void dragEnterEvent(QDragEnterEvent* _dee);

    PixmapButton* sinWaveBtn;
    PixmapButton* triangleWaveBtn;
    PixmapButton* sqrWaveBtn;
    PixmapButton* sawWaveBtn;
    PixmapButton* whiteNoiseWaveBtn;
    PixmapButton* smoothBtn;
    PixmapButton* usrWaveBtn;

    PixmapButton* sinWave2Btn;
    PixmapButton* triangleWave2Btn;
    PixmapButton* sqrWave2Btn;
    PixmapButton* sawWave2Btn;
    PixmapButton* whiteNoiseWave2Btn;
    PixmapButton* smooth2Btn;
    PixmapButton* usrWave2Btn;

    PixmapButton* XBtn;  // For leaving wavetable loading section

    PixmapButton* openWavetableButton;
    PixmapButton* confirmLoadButton;
    PixmapButton* openSampleButton;

    PixmapButton* modUpArrow[64];
    PixmapButton* modDownArrow[64];

    PixmapButton* tab1Btn;
    PixmapButton* tab2Btn;
    PixmapButton* tab3Btn;
    PixmapButton* tab4Btn;
    PixmapButton* tab5Btn;
    PixmapButton* tab6Btn;

    PixmapButton* mainFlipBtn;
    PixmapButton* subFlipBtn;

    PixmapButton* manualBtn;

    Knob*        morphKnob[8];
    Knob*        rangeKnob[8];
    Knob*        visvolKnob;
    Knob*        sampLenKnob[8];
    Knob*        morphMaxKnob[8];
    Knob*        unisonVoicesKnob[8];
    Knob*        unisonDetuneKnob[8];
    Knob*        unisonMorphKnob[8];
    Knob*        unisonModifyKnob[8];
    Knob*        detuneKnob[8];
    Knob*        loadAlgKnob;
    Knob*        loadChnlKnob;
    Knob*        phaseKnob[8];
    Knob*        phaseRandKnob[8];
    Knob*        volKnob[8];
    LedCheckBox* enabledToggle[8];
    LedCheckBox* mutedToggle[8];
    Knob*        scrollKnob;
    Knob*        panKnob[8];

    Knob*        filtInVolKnob[8];
    ComboBox*    filtTypeBox[8];
    ComboBox*    filtSlopeBox[8];
    Knob*        filtCutoffKnob[8];
    Knob*        filtResoKnob[8];
    Knob*        filtGainKnob[8];
    Knob*        filtSatuKnob[8];
    Knob*        filtWetDryKnob[8];
    Knob*        filtBalKnob[8];
    Knob*        filtOutVolKnob[8];
    LedCheckBox* filtEnabledToggle[8];
    Knob*        filtFeedbackKnob[8];
    Knob*        filtDetuneKnob[8];
    LedCheckBox* filtKeytrackingToggle[8];
    LedCheckBox* filtMutedToggle[8];

    LcdSpinBox* subNumBox;
    LcdSpinBox* sampNumBox;
    LcdSpinBox* mainNumBox;

    ComboBox* oversampleBox;
    ComboBox* loadModeBox;

    ComboBox*    modifyModeBox[8];
    Knob*        modifyKnob[8];
    ComboBox*    modOutSecBox[64];
    ComboBox*    modOutSigBox[64];
    LcdSpinBox*  modOutSecNumBox[64];
    ComboBox*    modInBox[64];
    LcdSpinBox*  modInNumBox[64];
    LcdSpinBox*  modInOtherNumBox[64];
    Knob*        modInAmntKnob[64];
    Knob*        modInCurveKnob[64];
    ComboBox*    modInBox2[64];
    LcdSpinBox*  modInNumBox2[64];
    LcdSpinBox*  modInOtherNumBox2[64];
    Knob*        modInAmntKnob2[64];
    Knob*        modInCurveKnob2[64];
    LedCheckBox* modEnabledToggle[64];
    ComboBox*    modCombineTypeBox[64];
    LedCheckBox* modTypeToggle[64];

    LcdSpinBox* modNumBox;

    static QPixmap* s_artwork;

    Graph*       graph;
    LedCheckBox* visualizeToggle;
    LedCheckBox* subEnabledToggle[64];
    Knob*        subVolKnob[64];
    Knob*        subPhaseKnob[64];
    Knob*        subPhaseRandKnob[64];
    Knob*        subDetuneKnob[64];
    LedCheckBox* subMutedToggle[64];
    LedCheckBox* subKeytrackToggle[64];
    Knob*        subSampLenKnob[64];
    LedCheckBox* subNoiseToggle[64];
    Knob*        subPanningKnob[64];
    Knob*        subTempoKnob[64];

    LedCheckBox* sampleEnabledToggle[8];
    LedCheckBox* sampleGraphEnabledToggle[8];
    LedCheckBox* sampleMutedToggle[8];
    LedCheckBox* sampleKeytrackingToggle[8];
    LedCheckBox* sampleLoopToggle[8];

    Knob* sampleVolumeKnob[8];
    Knob* samplePanningKnob[8];
    Knob* sampleDetuneKnob[8];
    Knob* samplePhaseKnob[8];
    Knob* samplePhaseRandKnob[8];
    Knob* sampleStartKnob[8];
    Knob* sampleEndKnob[8];

    Knob* wtLoad1Knob;
    Knob* wtLoad2Knob;
    Knob* wtLoad3Knob;
    Knob* wtLoad4Knob;

    Knob* macroKnob[8];

    QScrollBar* effectScrollBar;
    QScrollBar* matrixScrollBar;

    QLabel* filtForegroundLabel;
    QLabel* filtBoxesLabel;
    QLabel* matrixForegroundLabel;
    QLabel* matrixBoxesLabel;

    QPalette pal            = QPalette();
    QPixmap  tab1ArtworkImg = PLUGIN_NAME::getIconPixmap("tab1_artwork");
    QPixmap  tab1FlippedArtworkImg
            = PLUGIN_NAME::getIconPixmap("tab1_artwork_flipped");
    QPixmap tab2ArtworkImg = PLUGIN_NAME::getIconPixmap("tab2_artwork");
    QPixmap tab2FlippedArtworkImg
            = PLUGIN_NAME::getIconPixmap("tab2_artwork_flipped");
    QPixmap tab3ArtworkImg = PLUGIN_NAME::getIconPixmap("tab3_artwork");
    QPixmap tab4ArtworkImg = PLUGIN_NAME::getIconPixmap("tab4_artwork");
    QPixmap tab5ArtworkImg = PLUGIN_NAME::getIconPixmap("tab5_artwork");
    QPixmap tab6ArtworkImg = PLUGIN_NAME::getIconPixmap("tab6_artwork");
    QPixmap tab7ArtworkImg = PLUGIN_NAME::getIconPixmap("tab7_artwork");

    QString wavetableFileName = "";

    Microwave* microwave;

    real_t temp1;
    real_t temp2;

    friend class mSynth;
};

class mSynth
{
    MM_OPERATORS

  public:
    mSynth(NotePlayHandle*     _nph,
           const sample_rate_t _sample_rate,
           real_t*             phaseRand,
           int*                modifyModeVal,
           real_t*             modifyVal,
           real_t*             morphVal,
           real_t*             rangeVal,
           real_t*             unisonVoices,
           real_t*             unisonDetune,
           real_t*             unisonMorph,
           real_t*             unisonModify,
           real_t*             morphMaxVal,
           real_t*             detuneVal,
           FLOAT               waveforms[8][524288],
           FLOAT*              subs,
           bool*               subEnabled,
           real_t*             subVol,
           real_t*             subPhase,
           real_t*             subPhaseRand,
           real_t*             subDetune,
           bool*               subMuted,
           bool*               subKeytrack,
           real_t*             subSampLen,
           bool*               subNoise,
           int*                sampLen,
           int*                modIn,
           int*                modInNum,
           int*                modInOtherNum,
           real_t*             modInAmnt,
           real_t*             modInCurve,
           int*                modIn2,
           int*                modInNum2,
           int*                modInOtherNum2,
           real_t*             modInAmnt2,
           real_t*             modInCurve2,
           int*                modOutSec,
           int*                modOutSig,
           int*                modOutSecNum,
           int*                modCombineType,
           bool*               modType,
           real_t*             phase,
           real_t*             vol,
           real_t*             filtInVol,
           int*                filtType,
           int*                filtSlope,
           real_t*             filtCutoff,
           real_t*             filtReso,
           real_t*             filtGain,
           real_t*             filtSatu,
           real_t*             filtWetDry,
           real_t*             filtBal,
           real_t*             filtOutVol,
           bool*               filtEnabled,
           bool*               enabled,
           bool*               modEnabled,
           FLOAT*              sampGraphs,
           bool*               muted,
           bool*               sampleEnabled,
           bool*               sampleGraphEnabled,
           bool*               sampleMuted,
           bool*               sampleKeytracking,
           bool*               sampleLoop,
           real_t*             sampleVolume,
           real_t*             samplePanning,
           real_t*             sampleDetune,
           real_t*             samplePhase,
           real_t*             samplePhaseRand,
           std::vector<FLOAT> (&samples)[8][2],
           real_t* filtFeedback,
           real_t* filtDetune,
           bool*   filtKeytracking,
           real_t* subPanning,
           real_t* sampleStart,
           real_t* sampleEnd,
           real_t* pan,
           real_t* subTempo,
           real_t* macro,
           bool*   filtMuted);
    virtual ~mSynth();

    std::vector<real_t> nextStringSample(FLOAT (&waveforms)[8][524288],
                                         FLOAT* subs,
                                         FLOAT* sampGraphs,
                                         std::vector<FLOAT> (&samples)[8][2],
                                         int maxFiltEnabled,
                                         int maxModEnabled,
                                         int maxSubEnabled,
                                         int maxSampleEnabled,
                                         int maxMainEnabled,
                                         int sample_rate);

    inline real_t detuneWithCents(real_t pitchValue, real_t detuneValue);

    void refreshValue(int which, int num);

  private:
    real_t              sample_realindex[8][32] = {(0)};
    real_t              sample_subindex[64]     = {0};
    real_t              sample_sampleindex[8]   = {0};
    NotePlayHandle*     nph;
    const sample_rate_t sample_rate;

    int    noteDuration;
    real_t noteFreq = 0;

    real_t lastMainOscVal[8][2] = {{0}};
    real_t lastSubVal[64][2]    = {{0}};
    real_t sample_step_sub      = 0;
    real_t noiseSampRand        = 0;
    real_t lastSampleVal[8][2]  = {{0}};
    real_t sample_step_sample   = 0;

    real_t lastMainOscEnvVal[8][2] = {{0}};
    real_t lastSubEnvVal[64][2]    = {{0}};
    real_t lastSampleEnvVal[8][2]  = {{0}};

    bool lastMainOscEnvDone[8] = {false};
    bool lastSubEnvDone[64]    = {false};
    bool lastSampleEnvDone[8]  = {false};

    int    loopStart             = 0;
    int    loopEnd               = 0;
    real_t currentRangeValInvert = 0;
    int    currentSampLen        = 0;
    int    currentIndex          = 0;

    real_t filtInputs[8][2]           = {{0}};  // [filter number][channel]
    real_t filtOutputs[8][2]          = {{0}};  // [filter number][channel]
    real_t filtPrevSampIn[8][8][5][2] = {
            {{0}}};  // [filter number][slope][samples back in time][channel]
    real_t filtPrevSampOut[8][8][5][2] = {
            {{0}}};  // [filter number][slope][samples back in time][channel]
    real_t filtModOutputs[8][2] = {{0}};  // [filter number][channel]

    std::vector<real_t> filtDelayBuf[8][2];  // [filter number][channel]

    real_t filty1[2]    = {0};
    real_t filty2[2]    = {0};
    real_t filty3[2]    = {0};
    real_t filty4[2]    = {0};
    real_t filtoldx[2]  = {0};
    real_t filtoldy1[2] = {0};
    real_t filtoldy2[2] = {0};
    real_t filtoldy3[2] = {0};
    real_t filtx[2]     = {0};

    std::vector<int> modValType;
    std::vector<int> modValNum;

    int    modifyModeVal[8];
    real_t modifyVal[8];
    real_t morphVal[8];
    real_t rangeVal[8];
    real_t unisonVoices[8];
    real_t unisonDetune[8];
    real_t unisonMorph[8];
    real_t unisonModify[8];
    real_t morphMaxVal[8];
    real_t detuneVal[8];
    int    sampLen[8];
    int    modIn[64];
    int    modInNum[64];
    int    modInOtherNum[64];
    real_t modInAmnt[64];
    real_t modInCurve[64];
    int    modIn2[64];
    int    modInNum2[64];
    int    modInOtherNum2[64];
    real_t modInAmnt2[64];
    real_t modInCurve2[64];
    int    modOutSec[64];
    int    modOutSig[64];
    int    modOutSecNum[64];
    bool   modEnabled[64];
    int    modCombineType[64];
    bool   modType[64];
    real_t phase[8];
    real_t phaseRand[8];
    real_t vol[8];
    bool   enabled[8];
    bool   muted[8];
    real_t pan[8];

    bool   subEnabled[64];
    real_t subVol[64];
    real_t subPhase[64];
    real_t subPhaseRand[64];
    real_t subDetune[64];
    bool   subMuted[64];
    bool   subKeytrack[64];
    real_t subSampLen[64];
    bool   subNoise[64];
    real_t subPanning[64];
    real_t subTempo[64];

    real_t filtInVol[8];
    int    filtType[8];
    int    filtSlope[8];
    real_t filtCutoff[8];
    real_t filtReso[8];
    real_t filtGain[8];
    real_t filtSatu[8];
    real_t filtWetDry[8];
    real_t filtBal[8];
    real_t filtOutVol[8];
    bool   filtEnabled[8];
    real_t filtFeedback[8];
    real_t filtDetune[8];
    bool   filtKeytracking[8];
    bool   filtMuted[8];

    bool sampleEnabled[8];
    bool sampleGraphEnabled[8];
    bool sampleMuted[8];
    bool sampleKeytracking[8];
    bool sampleLoop[8];

    real_t sampleVolume[8];
    real_t samplePanning[8];
    real_t sampleDetune[8];
    real_t samplePhase[8];
    real_t samplePhaseRand[8];
    real_t sampleStart[8];
    real_t sampleEnd[8];

    real_t cutoff;
    int    mode;
    real_t reso;
    real_t dbgain;
    real_t Fs;
    real_t a0;
    real_t a1;
    real_t a2;
    real_t b0;
    real_t b1;
    real_t b2;
    real_t alpha;
    real_t w0;
    real_t A;
    real_t f;
    real_t k;
    real_t p;
    real_t scale;
    real_t r;

    real_t humanizer[8] = {0};

    real_t unisonDetuneAmounts[8][32] = {0};

    real_t temp1;
    real_t temp2;
    real_t temp3;

    real_t curModVal[2]       = {0};
    real_t curModVal2[2]      = {0};
    real_t curModValCurve[2]  = {0};
    real_t curModVal2Curve[2] = {0};
    real_t comboModVal[2]     = {0};
    real_t comboModValMono    = 0;

    real_t sample_morerealindex[8][32] = {{0}};
    real_t sample_step[8][32]          = {{0}};
    real_t sample_length_modify[8][32] = {{0}};

    real_t unisonVoicesMinusOne = 0;

    bool updateFrequency = 0;

    real_t macro[8];

    friend class Microwave;
};

class MicrowaveManualView : public QTextEdit
{
    Q_OBJECT
  public:
    static MicrowaveManualView* getInstance()
    {
        static MicrowaveManualView instance;
        return &instance;
    }
    static void finalize()
    {
    }

  private:
    MicrowaveManualView();
    static QString s_manualText;
};

#endif
