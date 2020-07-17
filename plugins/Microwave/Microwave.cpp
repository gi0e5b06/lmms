/*
 * Microwave.cpp - morbidly advanced and versatile wavetable synthesizer
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

#include "Microwave.h"

#include "CaptionMenu.h"
#include "Engine.h"
#include "FileDialog.h"
#include "Graph.h"
#include "GuiApplication.h"
#include "InstrumentTrack.h"
#include "Knob.h"
#include "LcdSpinBox.h"
#include "MSynth.h"
#include "MicrowaveView.h"
//#include "LedCheckbox.h"
#include "MainWindow.h"
#include "Mixer.h"
#include "NotePlayHandle.h"
#include "Oscillator.h"
#include "PixmapButton.h"
#include "SampleBuffer.h"
#include "Song.h"
#include "StringPairDrag.h"
#include "ToolTip.h"
#include "base64.h"
#include "embed.h"
#include "gui_templates.h"
#include "interpolation.h"
#include "lmms_math.h"
//#include "plugin_export.h"
#include "templates.h"

#include <QDomElement>
#include <QDropEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>

#include <iostream>
using namespace std;

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT microwave_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "Microwave",
               QT_TRANSLATE_NOOP("pluginBrowser",
                                 "Unique wavetable synthesizer"),
               "DouglasDGI",
               0x0100,
               Plugin::Instrument,
               new PluginPixmapLoader("logo"),
               nullptr,
               nullptr};
}

// Create Microwave.  Create value models (and set defaults, maximums,
// minimums, and step sizes).  Connect events to functions.
Microwave::Microwave(InstrumentTrack* _instrument_track) :
      Instrument(_instrument_track, &microwave_plugin_descriptor),
      graph(-1.0f, 1.0f, WAVE_SIZE, this),

      loadMode(this, tr("Wavetable Loading Algorithm")),
      loadChnl(0, 0, 1, 1, this, tr("Wavetable Loading Channel")),
      scroll(1, 1, 7, 0.0001, this, tr("Scroll")),
      oversample(this, tr("Oversampling")), visualize(false, this),
      visvol(100, 0, 1000, 0.01, this, tr("Visualizer Volume")),
      wtLoad1(0, 0, 3000, 1, this, tr("Wavetable Loading Knob 1")),
      wtLoad2(0, 0, 3000, 1, this, tr("Wavetable Loading Knob 2")),
      wtLoad3(0, 0, 3000, 1, this, tr("Wavetable Loading Knob 3")),
      wtLoad4(0, 0, 3000, 1, this, tr("Wavetable Loading Knob 4")),

      mainNum(1, 1, NB_MAINOSC, this, tr("Main Oscillator Number")),
      subNum(1, 1, NB_SUBOSC, this, tr("Sub Oscillator Number")),
      sampNum(1, 1, NB_SMPLR, this, tr("Sample Number")),
      modNum(1, 1, NB_MODLT, this, tr("Modulation Page Number"))  // 32?
{

    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        morph[i] = new FloatModel(0, 0, 254, 0.0001, this, tr("Morph"));
        range[i] = new FloatModel(1, 1, 16, 0.0001, this, tr("Range"));
        sampLen[i]
                = new FloatModel(WAVE_SIZE, 1, WAVE_SIZE /*8192*/, 1., this,
                                 tr("Waveform Sample Length"));  // ???
        sampLen[i]->setStrictStepSize(true);
        modifyMode[i]
                = new ComboBoxModel(this, tr("Wavetable Modifier Mode"));

        // max is updated automatically
        // 254 = MAINWAV_SIZE / WAVE_SIZE -2
        morphMax[i]
                = new FloatModel(254, 0, 254, 0.0001, this, tr("Morph Max"));
        // max is updated automatically
        modify[i] = new FloatModel(0, 0, WAVE_SIZE - 1, 0.0001, this,
                                   tr("Wavetable Modifier Value"));

        unisonVoices[i]
                = new FloatModel(1, 1, 32, 1, this, tr("Unison Voices"));
        unisonVoices[i]->setStrictStepSize(true);
        unisonDetune[i] = new FloatModel(0, 0, 2000, 0.0001, this,
                                         tr("Unison Detune"));
        unisonDetune[i]->setScaleLogarithmic(true);
        unisonMorph[i]
                = new FloatModel(0, 0, 256, 0.0001, this, tr("Unison Morph"));
        unisonModify[i] = new FloatModel(0, 0, WAVE_SIZE, 0.0001, this,
                                         tr("Unison Modify"));
        detune[i] = new FloatModel(0, -9600, 9600, 1., this, tr("Detune"));
        detune[i]->setStrictStepSize(true);
        phase[i]     = new FloatModel(0, 0, 200, 0.0001, this, tr("Phase"));
        phaseRand[i] = new FloatModel(100, 0, 100, 0.0001, this,
                                      tr("Phase Randomness"));
        vol[i] = new FloatModel(100., 0, 200., 0.0001, this, tr("Volume"));
        enabled[i] = new BoolModel(false, this);
        muted[i]   = new BoolModel(false, this);
        pan[i] = new FloatModel(0., -100., 100., 0.0001, this, tr("Panning"));

        setwavemodel(modifyMode[i]);
    }

    for(int i = 0; i < NB_FILTR; ++i)
    {
        filtInVol[i]  = new FloatModel(100, 0, 200, 0.0001, this,
                                      tr("Input Volume"));
        filtType[i]   = new ComboBoxModel(this, tr("Filter Type"));
        filtSlope[i]  = new ComboBoxModel(this, tr("Filter Slope"));
        filtCutoff[i] = new FloatModel(2000, 20, 20000, 0.0001, this,
                                       tr("Cutoff Frequency"));
        filtCutoff[i]->setScaleLogarithmic(true);
        filtReso[i]
                = new FloatModel(0.707, 0, 16, 0.0001, this, tr("Resonance"));
        filtReso[i]->setScaleLogarithmic(true);
        filtGain[i] = new FloatModel(0, -64, 64, 0.0001, this, tr("dbGain"));
        filtGain[i]->setScaleLogarithmic(true);
        filtSatu[i]
                = new FloatModel(0, 0, 100, 0.0001, this, tr("Saturation"));
        filtWetDry[i]
                = new FloatModel(100, 0, 100, 0.0001, this, tr("Wet/Dry"));
        filtBal[i]     = new FloatModel(0, -100, 100, 0.0001, this,
                                    tr("Balance/Panning"));
        filtOutVol[i]  = new FloatModel(100, 0, 200, 0.0001, this,
                                       tr("Output Volume"));
        filtEnabled[i] = new BoolModel(false, this);
        filtFeedback[i]
                = new FloatModel(0, -100, 100, 0.0001, this, tr("Feedback"));
        filtDetune[i]
                = new FloatModel(0, -4800, 4800, 0.0001, this, tr("Detune"));
        filtKeytracking[i] = new BoolModel(true, this);
        filtMuted[i]       = new BoolModel(false, this);

        filtertypesmodel(filtType[i]);
        filterslopesmodel(filtSlope[i]);
    }

    for(int i = 0; i < NB_SMPLR; ++i)
    {
        sampleEnabled[i]      = new BoolModel(false, this);
        sampleGraphEnabled[i] = new BoolModel(false, this);
        sampleMuted[i]        = new BoolModel(false, this);
        sampleKeytracking[i]  = new BoolModel(true, this);
        sampleLoop[i]         = new BoolModel(true, this);

        sampleVolume[i]
                = new FloatModel(100, 0, 200, 0.0001, this, tr("Volume"));
        samplePanning[i]
                = new FloatModel(0, -100, 100, 0.0001, this, tr("Panning"));
        sampleDetune[i]
                = new FloatModel(0, -9600, 9600, 0.0001, this, tr("Detune"));
        samplePhase[i] = new FloatModel(0, 0, 200, 0.0001, this, tr("Phase"));
        samplePhaseRand[i] = new FloatModel(0, 0, 100, 0.0001, this,
                                            tr("Phase Randomness"));
        sampleStart[i]
                = new FloatModel(0, 0, 0.9999f, 0.0001, this, tr("Start"));
        sampleEnd[i] = new FloatModel(1, 0.0001, 1, 0.0001, this, tr("End"));
    }

    for(int i = 0; i < NB_MACRO; ++i)
    {
        macro[i] = new FloatModel(0, -100, 100, 0.0001, this, tr("Macro"));
    }

    for(int i = 0; i < NB_SUBOSC; ++i)
    {
        subEnabled[i] = new BoolModel(false, this);
        subVol[i]
                = new FloatModel(100., 0., 200., 0.0001, this, tr("Volume"));
        subPhase[i] = new FloatModel(0., 0., 200., 0.0001, this, tr("Phase"));
        subPhaseRand[i] = new FloatModel(0., 0., 100., 0.0001, this,
                                         tr("Phase Randomness"));
        subDetune[i]
                = new FloatModel(0., -9600., 9600., 1., this, tr("Detune"));
        subDetune[i]->setStrictStepSize(true);
        subMuted[i]    = new BoolModel(true, this);
        subKeytrack[i] = new BoolModel(true, this);
        subSampLen[i]  = new FloatModel(WAVE_SIZE, 1., WAVE_SIZE, 1., this,
                                       tr("Sample Length"));
        subSampLen[i]->setStrictStepSize(true);
        subNoise[i]   = new BoolModel(false, this);
        subPanning[i] = new FloatModel(0., -100., 100., 0.0001, this,
                                       tr("Panning"));
        subTempo[i]   = new FloatModel(0., 0., 400., 1., this, tr("Tempo"));
        subTempo[i]->setStrictStepSize(true);
    }

    for(int i = 0; i < NB_MODLT; ++i)
    {

        modEnabled[i] = new BoolModel(false, this);

        modOutSec[i]    = new ComboBoxModel(this, tr("Modulation Section"));
        modOutSig[i]    = new ComboBoxModel(this, tr("Modulation Signal"));
        modOutSecNum[i] = new IntModel(1, 1, NB_MAINOSC, this,
                                       tr("Modulation Section Number"));

        modsectionsmodel(modOutSec[i]);
        mainoscsignalsmodel(modOutSig[i]);

        modIn[i] = new ComboBoxModel(this, tr("Modulator"));
        modInNum[i]
                = new IntModel(1, 1, NB_MODLT, this, tr("Modulator Number"));
        modInOtherNum[i]
                = new IntModel(1, 1, NB_MODLT, this, tr("Modulator Number"));

        modinmodel(modIn[i]);

        modInAmnt[i]  = new FloatModel(0., -200., 200., 0.0001, this,
                                      tr("Modulator Amount"));
        modInCurve[i] = new FloatModel(100, 0.0001, 200, 0.0001, this,
                                       tr("Modulator Curve"));

        modIn2[i]    = new ComboBoxModel(this, tr("Secondary Modulator"));
        modInNum2[i] = new IntModel(1, 1, NB_MODLT, this,
                                    tr("Secondary Modulator Number"));
        modInOtherNum2[i] = new IntModel(1, 1, NB_MODLT, this,
                                         tr("Secondary Modulator Number"));

        modinmodel(modIn2[i]);

        modInAmnt2[i]  = new FloatModel(0., -200., 200., 0.0001, this,
                                       tr("Secondary Modulator Amount"));
        modInCurve2[i] = new FloatModel(100., 0.0001, 200., 0.0001, this,
                                        tr("Secondary Modulator Curve"));

        modCombineType[i] = new ComboBoxModel(this, tr("Combination Type"));

        modcombinetypemodel(modCombineType[i]);

        modType[i] = new BoolModel(false, this);
    }

    oversamplemodel(oversample);
    oversample.setValue(1);  // 2x oversampling is default

    loadmodemodel(loadMode);

    graph.setWaveToSine();

    connect(&graph, SIGNAL(samplesChanged(int, int)), this,
            SLOT(samplesChanged(int, int)));

    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        connect(morphMax[i], SIGNAL(dataChanged()), this,
                SLOT(morphMaxChanged()), Qt::DirectConnection);
        connect(
                enabled[i], &BoolModel::dataChanged, this,
                [this, i]() { mainEnabledChanged(i); }, Qt::DirectConnection);
        connect(
                morph[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(1, i); }, Qt::DirectConnection);
        connect(
                range[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(2, i); }, Qt::DirectConnection);
        connect(
                modify[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(3, i); }, Qt::DirectConnection);
        connect(
                modifyMode[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(4, i); }, Qt::DirectConnection);
        connect(
                unisonVoices[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(6, i); }, Qt::DirectConnection);
        connect(
                unisonDetune[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(7, i); }, Qt::DirectConnection);
        connect(
                unisonMorph[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(8, i); }, Qt::DirectConnection);
        connect(
                unisonModify[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(9, i); }, Qt::DirectConnection);
        connect(
                morphMax[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(10, i); }, Qt::DirectConnection);
        connect(
                detune[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(11, i); }, Qt::DirectConnection);
        connect(
                sampLen[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(12, i); }, Qt::DirectConnection);
        connect(
                phase[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(13, i); }, Qt::DirectConnection);
        connect(
                vol[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(14, i); }, Qt::DirectConnection);
        connect(
                enabled[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(48, i); }, Qt::DirectConnection);
        connect(
                muted[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(57, i); }, Qt::DirectConnection);
        connect(
                pan[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(75, i); }, Qt::DirectConnection);
        connect(
                phaseRand[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(80, i); }, Qt::DirectConnection);

        for(int j = 1; j <= 14; ++j)
            valueChanged(j, i);
        valueChanged(48, i);
        valueChanged(57, i);
        valueChanged(75, i);
        valueChanged(80, i);
    }

    for(int i = 0; i < NB_FILTR; ++i)
    {

        connect(
                filtInVol[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(15, i); }, Qt::DirectConnection);
        connect(
                filtType[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(16, i); }, Qt::DirectConnection);
        connect(
                filtSlope[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(17, i); }, Qt::DirectConnection);
        connect(
                filtCutoff[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(18, i); }, Qt::DirectConnection);
        connect(
                filtReso[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(19, i); }, Qt::DirectConnection);
        connect(
                filtGain[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(20, i); }, Qt::DirectConnection);
        connect(
                filtSatu[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(21, i); }, Qt::DirectConnection);
        connect(
                filtWetDry[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(22, i); }, Qt::DirectConnection);
        connect(
                filtBal[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(23, i); }, Qt::DirectConnection);
        connect(
                filtOutVol[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(24, i); }, Qt::DirectConnection);
        connect(
                filtEnabled[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(25, i); }, Qt::DirectConnection);
        connect(
                filtFeedback[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(69, i); }, Qt::DirectConnection);
        connect(
                filtDetune[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(70, i); }, Qt::DirectConnection);
        connect(
                filtKeytracking[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(71, i); }, Qt::DirectConnection);
        connect(
                filtMuted[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(79, i); }, Qt::DirectConnection);

        for(int j = 15; j <= 25; ++j)
            valueChanged(j, i);
        valueChanged(69, i);
        valueChanged(70, i);
        valueChanged(71, i);
        valueChanged(79, i);

        connect(
                filtEnabled[i], &BoolModel::dataChanged, this,
                [this, i]() { filtEnabledChanged(i); }, Qt::DirectConnection);
    }

    for(int i = 0; i < NB_SMPLR; ++i)
    {
        connect(
                sampleEnabled[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(59, i); }, Qt::DirectConnection);

        connect(
                sampleGraphEnabled[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(60, i); }, Qt::DirectConnection);

        connect(
                sampleMuted[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(61, i); }, Qt::DirectConnection);

        connect(
                sampleKeytracking[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(62, i); }, Qt::DirectConnection);

        connect(
                sampleLoop[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(63, i); }, Qt::DirectConnection);

        connect(
                sampleVolume[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(64, i); }, Qt::DirectConnection);

        connect(
                samplePanning[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(65, i); }, Qt::DirectConnection);

        connect(
                sampleDetune[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(66, i); }, Qt::DirectConnection);

        connect(
                samplePhase[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(67, i); }, Qt::DirectConnection);

        connect(
                samplePhaseRand[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(68, i); }, Qt::DirectConnection);

        connect(
                sampleStart[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(73, i); }, Qt::DirectConnection);

        connect(
                sampleEnd[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(74, i); }, Qt::DirectConnection);

        connect(
                sampleEnabled[i], &BoolModel::dataChanged, this,
                [this, i]() { sampleEnabledChanged(i); },
                Qt::DirectConnection);

        for(int j = 59; j <= 68; ++j)
            valueChanged(j, i);
        valueChanged(73, i);
        valueChanged(74, i);
    }

    for(int i = 0; i < NB_MACRO; ++i)
    {
        connect(
                macro[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(78, i); }, Qt::DirectConnection);

        valueChanged(78, i);
    }

    /*
    valueChanged(51, i);
    valueChanged(52, i);
    valueChanged(53, i);
    valueChanged(54, i);
    valueChanged(55, i);
    valueChanged(56, i);
    valueChanged(58, i);
    */

    for(int i = 0; i < NB_SUBOSC; ++i)
    {
        connect(
                subEnabled[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(26, i); }, Qt::DirectConnection);
        connect(
                subVol[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(27, i); }, Qt::DirectConnection);
        connect(
                subPhase[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(28, i); }, Qt::DirectConnection);
        connect(
                subPhaseRand[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(29, i); }, Qt::DirectConnection);
        connect(
                subDetune[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(30, i); }, Qt::DirectConnection);
        connect(
                subMuted[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(31, i); }, Qt::DirectConnection);
        connect(
                subKeytrack[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(32, i); }, Qt::DirectConnection);
        connect(
                subSampLen[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(33, i); }, Qt::DirectConnection);
        connect(
                subNoise[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(34, i); }, Qt::DirectConnection);
        connect(
                subPanning[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(72, i); }, Qt::DirectConnection);
        connect(
                subTempo[i], &FloatModel::dataChanged, this,
                [this, i]() { valueChanged(76, i); }, Qt::DirectConnection);

        connect(subSampLen[i], &FloatModel::dataChanged, this,
                [this, i]() { subSampLenChanged(i); });
        connect(subEnabled[i], &BoolModel::dataChanged, this,
                [this, i]() { subEnabledChanged(i); });

        for(int j = 26; j <= 34; ++j)
            valueChanged(j, i);
        valueChanged(72, i);
        valueChanged(76, i);
    }

    for(int i = 0; i < NB_MODLT; ++i)
    {
        connect(
                modIn[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(35, i); }, Qt::DirectConnection);
        connect(
                modInNum[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(36, i); }, Qt::DirectConnection);
        connect(
                modInOtherNum[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(37, i); }, Qt::DirectConnection);
        connect(
                modInAmnt[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(38, i); }, Qt::DirectConnection);
        connect(
                modInCurve[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(39, i); }, Qt::DirectConnection);
        connect(
                modIn2[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(40, i); }, Qt::DirectConnection);
        connect(
                modInNum2[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(41, i); }, Qt::DirectConnection);
        connect(
                modInOtherNum2[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(42, i); }, Qt::DirectConnection);
        connect(
                modInAmnt2[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(43, i); }, Qt::DirectConnection);
        connect(
                modInCurve2[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(44, i); }, Qt::DirectConnection);
        connect(
                modOutSec[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(45, i); }, Qt::DirectConnection);
        connect(
                modOutSig[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(46, i); }, Qt::DirectConnection);
        connect(
                modOutSecNum[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(47, i); }, Qt::DirectConnection);
        connect(
                modEnabled[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(49, i); }, Qt::DirectConnection);
        connect(
                modCombineType[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { valueChanged(50, i); }, Qt::DirectConnection);
        connect(
                modType[i], &BoolModel::dataChanged, this,
                [this, i]() { valueChanged(77, i); }, Qt::DirectConnection);

        connect(
                modEnabled[i], &BoolModel::dataChanged, this,
                [this, i]() { modEnabledChanged(i); }, Qt::DirectConnection);

        for(int j = 35; j <= 47; ++j)
            valueChanged(j, i);
        valueChanged(49, i);
        valueChanged(50, i);
        valueChanged(77, i);
    }
}

Microwave::~Microwave()
{
}

PluginView* Microwave::instantiateView(QWidget* _parent)
{
    return (new MicrowaveView(this, _parent));
}

/*
QString Microwave::nodeName() const
{
    return (microwave_plugin_descriptor.name);
}
*/

void Microwave::saveSettings(QDomDocument& _doc, QDomElement& _this)
{

    // Save plugin version
    _this.setAttribute("version", "0.9");

    visvol.saveSettings(_doc, _this, "visualizervolume");
    loadMode.saveSettings(_doc, _this, "loadingalgorithm");
    loadChnl.saveSettings(_doc, _this, "loadingchannel");

    QString saveString;

    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        if(enabled[i]->value())
        {
            saveString = base64::encodeReals(waveforms[i], MAINWAV_SIZE);
            _this.setAttribute("waveforms" + QString::number(i), saveString);
        }
    }

    saveString = base64::encodeReals(subs, 131072);
    _this.setAttribute("subs", saveString);

    saveString = base64::encodeFloats(sampGraphs, 1024);
    _this.setAttribute("sampGraphs", saveString);

    int32_t sampleSizes[NB_SMPLR] = {0};
    for(int i = 0; i < NB_SMPLR; ++i)
    {
        if(sampleEnabled[i]->value())
        {
            for(int j = 0; j < 2; ++j)
            {
                saveString = base64::encodeReals(samples[i][j].data(),
                                                 samples[i][j].size());
                _this.setAttribute("samples_" + QString::number(i) + "_"
                                           + QString::number(j),
                                   saveString);
            }

            sampleSizes[i] = samples[i][0].size();
        }
    }

    saveString = base64::encodeChars((const char*)sampleSizes,
                                     NB_SMPLR * sizeof(int));
    _this.setAttribute("sampleSizes", saveString);

    for(int i = 0; i < maxMainEnabled; ++i)
    {
        if(enabled[i]->value())
        {
            morph[i]->saveSettings(_doc, _this,
                                   "morph_" + QString::number(i));
            range[i]->saveSettings(_doc, _this,
                                   "range_" + QString::number(i));
            modify[i]->saveSettings(_doc, _this,
                                    "modify_" + QString::number(i));
            modifyMode[i]->saveSettings(_doc, _this,
                                        "modifyMode_" + QString::number(i));
            unisonVoices[i]->saveSettings(
                    _doc, _this, "unisonVoices_" + QString::number(i));
            unisonDetune[i]->saveSettings(
                    _doc, _this, "unisonDetune_" + QString::number(i));
            unisonMorph[i]->saveSettings(_doc, _this,
                                         "unisonMorph_" + QString::number(i));
            unisonModify[i]->saveSettings(
                    _doc, _this, "unisonModify_" + QString::number(i));
            morphMax[i]->saveSettings(_doc, _this,
                                      "morphMax_" + QString::number(i));
            detune[i]->saveSettings(_doc, _this,
                                    "detune_" + QString::number(i));
            sampLen[i]->saveSettings(_doc, _this,
                                     "sampLen_" + QString::number(i));
            phase[i]->saveSettings(_doc, _this,
                                   "phase_" + QString::number(i));
            phaseRand[i]->saveSettings(_doc, _this,
                                       "phaseRand_" + QString::number(i));
            vol[i]->saveSettings(_doc, _this, "vol_" + QString::number(i));
            enabled[i]->saveSettings(_doc, _this,
                                     "enabled_" + QString::number(i));
            muted[i]->saveSettings(_doc, _this,
                                   "muted_" + QString::number(i));
            pan[i]->saveSettings(_doc, _this, "pan_" + QString::number(i));
        }
    }

    for(int i = 0; i < maxSampleEnabled; ++i)
    {
        if(sampleEnabled[i]->value())
        {
            sampleEnabled[i]->saveSettings(
                    _doc, _this, "sampleEnabled_" + QString::number(i));
            sampleGraphEnabled[i]->saveSettings(
                    _doc, _this, "sampleGraphEnabled_" + QString::number(i));
            sampleMuted[i]->saveSettings(_doc, _this,
                                         "sampleMuted_" + QString::number(i));
            sampleKeytracking[i]->saveSettings(
                    _doc, _this, "sampleKeytracking_" + QString::number(i));
            sampleLoop[i]->saveSettings(_doc, _this,
                                        "sampleLoop_" + QString::number(i));
            sampleVolume[i]->saveSettings(
                    _doc, _this, "sampleVolume_" + QString::number(i));
            samplePanning[i]->saveSettings(
                    _doc, _this, "samplePanning_" + QString::number(i));
            sampleDetune[i]->saveSettings(
                    _doc, _this, "sampleDetune_" + QString::number(i));
            samplePhase[i]->saveSettings(_doc, _this,
                                         "samplePhase_" + QString::number(i));
            samplePhaseRand[i]->saveSettings(
                    _doc, _this, "samplePhaseRand_" + QString::number(i));
            sampleStart[i]->saveSettings(_doc, _this,
                                         "sampleStart_" + QString::number(i));
            sampleEnd[i]->saveSettings(_doc, _this,
                                       "sampleEnd_" + QString::number(i));
        }
    }

    for(int i = 0; i < maxFiltEnabled; ++i)
    {
        if(filtEnabled[i]->value())
        {
            filtInVol[i]->saveSettings(_doc, _this,
                                       "filtInVol_" + QString::number(i));
            filtType[i]->saveSettings(_doc, _this,
                                      "filtType_" + QString::number(i));
            filtSlope[i]->saveSettings(_doc, _this,
                                       "filtSlope_" + QString::number(i));
            filtCutoff[i]->saveSettings(_doc, _this,
                                        "filtCutoff_" + QString::number(i));
            filtReso[i]->saveSettings(_doc, _this,
                                      "filtReso_" + QString::number(i));
            filtGain[i]->saveSettings(_doc, _this,
                                      "filtGain_" + QString::number(i));
            filtSatu[i]->saveSettings(_doc, _this,
                                      "filtSatu_" + QString::number(i));
            filtWetDry[i]->saveSettings(_doc, _this,
                                        "filtWetDry_" + QString::number(i));
            filtBal[i]->saveSettings(_doc, _this,
                                     "filtBal_" + QString::number(i));
            filtOutVol[i]->saveSettings(_doc, _this,
                                        "filtOutVol_" + QString::number(i));
            filtEnabled[i]->saveSettings(_doc, _this,
                                         "filtEnabled_" + QString::number(i));
            filtFeedback[i]->saveSettings(
                    _doc, _this, "filtFeedback_" + QString::number(i));
            filtDetune[i]->saveSettings(_doc, _this,
                                        "filtDetune_" + QString::number(i));
            filtKeytracking[i]->saveSettings(
                    _doc, _this, "filtKeytracking_" + QString::number(i));
            filtMuted[i]->saveSettings(_doc, _this,
                                       "filtMuted_" + QString::number(i));
        }
    }

    for(int i = 0; i < maxSubEnabled; ++i)
    {
        if(subEnabled[i]->value())
        {
            subEnabled[i]->saveSettings(_doc, _this,
                                        "subEnabled_" + QString::number(i));
            subVol[i]->saveSettings(_doc, _this,
                                    "subVol_" + QString::number(i));
            subPhase[i]->saveSettings(_doc, _this,
                                      "subPhase_" + QString::number(i));
            subPhaseRand[i]->saveSettings(
                    _doc, _this, "subPhaseRand_" + QString::number(i));
            subDetune[i]->saveSettings(_doc, _this,
                                       "subDetune_" + QString::number(i));
            subMuted[i]->saveSettings(_doc, _this,
                                      "subMuted_" + QString::number(i));
            subKeytrack[i]->saveSettings(_doc, _this,
                                         "subKeytrack_" + QString::number(i));
            subSampLen[i]->saveSettings(_doc, _this,
                                        "subSampLen_" + QString::number(i));
            subNoise[i]->saveSettings(_doc, _this,
                                      "subNoise_" + QString::number(i));
            subPanning[i]->saveSettings(_doc, _this,
                                        "subPanning_" + QString::number(i));
            subTempo[i]->saveSettings(_doc, _this,
                                      "subTempo_" + QString::number(i));
        }
    }

    for(int i = 0; i < maxModEnabled; ++i)
    {
        if(modEnabled[i]->value())
        {
            modIn[i]->saveSettings(_doc, _this,
                                   "modIn_" + QString::number(i));
            modInNum[i]->saveSettings(_doc, _this,
                                      "modInNu" + QString::number(i));
            modInOtherNum[i]->saveSettings(
                    _doc, _this, "modInOtherNu" + QString::number(i));
            modInAmnt[i]->saveSettings(_doc, _this,
                                       "modInAmnt_" + QString::number(i));
            modInCurve[i]->saveSettings(_doc, _this,
                                        "modInCurve_" + QString::number(i));
            modIn2[i]->saveSettings(_doc, _this,
                                    "modIn2_" + QString::number(i));
            modInNum2[i]->saveSettings(_doc, _this,
                                       "modInNum2_" + QString::number(i));
            modInOtherNum2[i]->saveSettings(
                    _doc, _this, "modInOtherNum2_" + QString::number(i));
            modInAmnt2[i]->saveSettings(_doc, _this,
                                        "modAmnt2_" + QString::number(i));
            modInCurve2[i]->saveSettings(_doc, _this,
                                         "modCurve2_" + QString::number(i));
            modOutSec[i]->saveSettings(_doc, _this,
                                       "modOutSec_" + QString::number(i));
            modOutSig[i]->saveSettings(_doc, _this,
                                       "modOutSig_" + QString::number(i));
            modOutSecNum[i]->saveSettings(_doc, _this,
                                          "modOutSecNu" + QString::number(i));
            modEnabled[i]->saveSettings(_doc, _this,
                                        "modEnabled_" + QString::number(i));
            modCombineType[i]->saveSettings(
                    _doc, _this, "modCombineType_" + QString::number(i));
            modType[i]->saveSettings(_doc, _this,
                                     "modType_" + QString::number(i));
        }
    }

    for(int i = 0; i < NB_MACRO; ++i)
    {
        macro[i]->saveSettings(_doc, _this, "macro_" + QString::number(i));
    }
}

void Microwave::loadSettings(const QDomElement& _this)
{
    visvol.loadSettings(_this, "visualizervolume");
    loadMode.loadSettings(_this, "loadingalgorithm");
    loadChnl.loadSettings(_this, "loadingchannel");

    graph.setLength(2048);

    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        enabled[i]->loadSettings(_this, "enabled_" + QString::number(i));
        if(enabled[i]->value())
        {
            morph[i]->loadSettings(_this, "morph_" + QString::number(i));
            range[i]->loadSettings(_this, "range_" + QString::number(i));
            modify[i]->loadSettings(_this, "modify_" + QString::number(i));
            modifyMode[i]->loadSettings(_this,
                                        "modifyMode_" + QString::number(i));
            unisonVoices[i]->loadSettings(
                    _this, "unisonVoices_" + QString::number(i));
            unisonDetune[i]->loadSettings(
                    _this, "unisonDetune_" + QString::number(i));
            unisonMorph[i]->loadSettings(_this,
                                         "unisonMorph_" + QString::number(i));
            unisonModify[i]->loadSettings(
                    _this, "unisonModify_" + QString::number(i));
            morphMax[i]->loadSettings(_this,
                                      "morphMax_" + QString::number(i));
            detune[i]->loadSettings(_this, "detune_" + QString::number(i));
            sampLen[i]->loadSettings(_this, "sampLen_" + QString::number(i));
            phase[i]->loadSettings(_this, "phase_" + QString::number(i));
            phaseRand[i]->loadSettings(_this,
                                       "phaseRand_" + QString::number(i));
            vol[i]->loadSettings(_this, "vol_" + QString::number(i));
            muted[i]->loadSettings(_this, "muted_" + QString::number(i));
            pan[i]->loadSettings(_this, "pan_" + QString::number(i));
        }

        for(int i = 0; i < NB_FILTR; ++i)
        {
            filtEnabled[i]->loadSettings(_this,
                                         "filtEnabled_" + QString::number(i));
            if(filtEnabled[i]->value())
            {
                filtInVol[i]->loadSettings(_this,
                                           "filtInVol_" + QString::number(i));
                filtType[i]->loadSettings(_this,
                                          "filtType_" + QString::number(i));
                filtSlope[i]->loadSettings(_this,
                                           "filtSlope_" + QString::number(i));
                filtCutoff[i]->loadSettings(
                        _this, "filtCutoff_" + QString::number(i));
                filtReso[i]->loadSettings(_this,
                                          "filtReso_" + QString::number(i));
                filtGain[i]->loadSettings(_this,
                                          "filtGain_" + QString::number(i));
                filtSatu[i]->loadSettings(_this,
                                          "filtSatu_" + QString::number(i));
                filtWetDry[i]->loadSettings(
                        _this, "filtWetDry_" + QString::number(i));
                filtBal[i]->loadSettings(_this,
                                         "filtBal_" + QString::number(i));
                filtOutVol[i]->loadSettings(
                        _this, "filtOutVol_" + QString::number(i));
                filtFeedback[i]->loadSettings(
                        _this, "filtFeedback_" + QString::number(i));
                filtDetune[i]->loadSettings(
                        _this, "filtDetune_" + QString::number(i));
                filtKeytracking[i]->loadSettings(
                        _this, "filtKeytracking_" + QString::number(i));
                filtMuted[i]->loadSettings(_this,
                                           "filtMuted_" + QString::number(i));
            }
        }

        for(int i = 0; i < NB_SMPLR; ++i)
        {
            sampleEnabled[i]->loadSettings(
                    _this, "sampleEnabled_" + QString::number(i));
            if(sampleEnabled[i]->value())
            {
                sampleGraphEnabled[i]->loadSettings(
                        _this, "sampleGraphEnabled_" + QString::number(i));
                sampleMuted[i]->loadSettings(
                        _this, "sampleMuted_" + QString::number(i));
                sampleKeytracking[i]->loadSettings(
                        _this, "sampleKeytracking_" + QString::number(i));
                sampleLoop[i]->loadSettings(
                        _this, "sampleLoop_" + QString::number(i));
                sampleVolume[i]->loadSettings(
                        _this, "sampleVolume_" + QString::number(i));
                samplePanning[i]->loadSettings(
                        _this, "samplePanning_" + QString::number(i));
                sampleDetune[i]->loadSettings(
                        _this, "sampleDetune_" + QString::number(i));
                samplePhase[i]->loadSettings(
                        _this, "samplePhase_" + QString::number(i));
                samplePhaseRand[i]->loadSettings(
                        _this, "samplePhaseRand_" + QString::number(i));
                sampleStart[i]->loadSettings(
                        _this, "sampleStart_" + QString::number(i));
                sampleEnd[i]->loadSettings(_this,
                                           "sampleEnd_" + QString::number(i));
            }
        }

        for(int i = 0; i < NB_MACRO; ++i)
        {
            macro[i]->loadSettings(_this, "macro_" + QString::number(i));
        }

        for(int i = 0; i < NB_SUBOSC; ++i)
        {
            subEnabled[i]->loadSettings(_this,
                                        "subEnabled_" + QString::number(i));
            if(subEnabled[i]->value())
            {
                subVol[i]->loadSettings(_this,
                                        "subVol_" + QString::number(i));
                subPhase[i]->loadSettings(_this,
                                          "subPhase_" + QString::number(i));
                subPhaseRand[i]->loadSettings(
                        _this, "subPhaseRand_" + QString::number(i));
                subDetune[i]->loadSettings(_this,
                                           "subDetune_" + QString::number(i));
                subMuted[i]->loadSettings(_this,
                                          "subMuted_" + QString::number(i));
                subKeytrack[i]->loadSettings(
                        _this, "subKeytrack_" + QString::number(i));
                subSampLen[i]->loadSettings(
                        _this, "subSampLen_" + QString::number(i));
                subNoise[i]->loadSettings(_this,
                                          "subNoise_" + QString::number(i));
                subPanning[i]->loadSettings(
                        _this, "subPanning_" + QString::number(i));
                subTempo[i]->loadSettings(_this,
                                          "subTempo_" + QString::number(i));
            }
        }

        for(int i = 0; i < NB_MODLT; ++i)
        {
            modEnabled[i]->loadSettings(_this,
                                        "modEnabled_" + QString::number(i));
            if(modEnabled[i]->value())
            {
                modIn[i]->loadSettings(_this, "modIn_" + QString::number(i));
                modInNum[i]->loadSettings(_this,
                                          "modInNu" + QString::number(i));
                modInOtherNum[i]->loadSettings(
                        _this, "modInOtherNu" + QString::number(i));
                modInAmnt[i]->loadSettings(_this,
                                           "modInAmnt_" + QString::number(i));
                modInCurve[i]->loadSettings(
                        _this, "modInCurve_" + QString::number(i));
                modIn2[i]->loadSettings(_this,
                                        "modIn2_" + QString::number(i));
                modInNum2[i]->loadSettings(_this,
                                           "modInNum2_" + QString::number(i));
                modInOtherNum2[i]->loadSettings(
                        _this, "modInOtherNum2_" + QString::number(i));
                modInAmnt2[i]->loadSettings(_this,
                                            "modAmnt2_" + QString::number(i));
                modInCurve2[i]->loadSettings(
                        _this, "modCurve2_" + QString::number(i));
                modOutSec[i]->loadSettings(_this,
                                           "modOutSec_" + QString::number(i));
                modOutSig[i]->loadSettings(_this,
                                           "modOutSig_" + QString::number(i));
                modOutSecNum[i]->loadSettings(
                        _this, "modOutSecNu" + QString::number(i));
                modCombineType[i]->loadSettings(
                        _this, "modCombineType_" + QString::number(i));
                modType[i]->loadSettings(_this,
                                         "modType_" + QString::number(i));
            }
        }

        // Load arrays
        char*   cdst = nullptr;
        FLOAT*  fdst = nullptr;
        int32_t size = 0;

        for(int j = 0; j < NB_MAINOSC; ++j)
        {
            if(enabled[j]->value())
            {
                size = base64::decodeFloats(
                        _this.attribute("waveforms" + QString::number(j)),
                        &fdst);
                if(size != MAINWAV_SIZE)
                    qWarning("Microwave: bad size waveforms: %d", size);
                for(int i = 0; i < MAINWAV_SIZE; ++i)
                    waveforms[j][i] = fdst[i];
                delete[] fdst;
            }
        }

        size = base64::decodeFloats(_this.attribute("subs"), &fdst);
        if(size != SUBWAV_SIZE)
            qWarning("Microwave: bad size subs: %d", size);
        for(int i = 0; i < SUBWAV_SIZE; ++i)
            subs[i] = fdst[i];
        delete[] fdst;

        size = base64::decodeFloats(_this.attribute("sampGraphs"), &fdst);
        if(size != 1024)
            qWarning("Microwave: bad size sampGraphs: %d", size);
        for(int i = 0; i < 1024; ++i)
            sampGraphs[i] = fdst[i];
        delete[] fdst;

        int32_t sampleSizes[NB_SMPLR] = {0};
        size = base64::decodeChars(_this.attribute("sampleSizes"), &cdst);
        if(size != NB_SMPLR * sizeof(int))
            qWarning("Microwave: bad size sampleSizes: %d != %d", size,
                     NB_SMPLR);
        for(int i = 0; i < NB_SMPLR; ++i)
            sampleSizes[i] = ((int*)cdst)[i];
        delete[] cdst;

        for(int i = 0; i < NB_SMPLR; ++i)
        {
            if(sampleEnabled[i]->value())
            {
                for(int j = 0; j < NB_CHAN; ++j)
                {
                    size = base64::decodeFloats(
                            _this.attribute("samples_" + QString::number(i)
                                            + "_" + QString::number(j)),
                            &fdst);
                    if(size != sampleSizes[i])
                        qWarning("Microwave: bad size samples: %d != %d",
                                 size, sampleSizes[i]);

                    for(int k = 0; k < sampleSizes[i]; ++k)
                        samples[i][j].push_back(fdst[k]);
                    delete[] fdst;
                }
            }
        }
    }
}

// When a knob is changed, send the new value to the array holding the
// knob values, as well as the note values within MSynths already
// initialized (notes already playing)
void Microwave::valueChanged(int32_t which, int32_t num)
{
    // Send new values to array
    switch(which)
    {
        case 1:
            morphArr[num] = morph[num]->value();
            break;
        case 2:
            rangeArr[num] = range[num]->value();
            break;
        case 3:
            modifyArr[num] = modify[num]->value();
            break;
        case 4:
            modifyModeArr[num] = modifyMode[num]->value();
            break;
        case 6:
            unisonVoicesArr[num] = unisonVoices[num]->value();
            break;
        case 7:
            unisonDetuneArr[num] = unisonDetune[num]->value();
            break;
        case 8:
            unisonMorphArr[num] = unisonMorph[num]->value();
            break;
        case 9:
            unisonModifyArr[num] = unisonModify[num]->value();
            break;
        case 10:
            morphMaxArr[num] = morphMax[num]->value();
            break;
        case 11:
            detuneArr[num] = detune[num]->value();
            break;
        case 12:
            sampLenArr[num] = sampLen[num]->value();
            break;
        case 13:
            phaseArr[num] = phase[num]->value();
            break;
        case 14:
            volArr[num] = vol[num]->value();
            break;
        case 15:
            filtInVolArr[num] = filtInVol[num]->value();
            break;
        case 16:
            filtTypeArr[num] = filtType[num]->value();
            break;
        case 17:
            filtSlopeArr[num] = filtSlope[num]->value();
            break;
        case 18:
            filtCutoffArr[num] = filtCutoff[num]->value();
            break;
        case 19:
            filtResoArr[num] = filtReso[num]->value();
            break;
        case 20:
            filtGainArr[num] = filtGain[num]->value();
            break;
        case 21:
            filtSatuArr[num] = filtSatu[num]->value();
            break;
        case 22:
            filtWetDryArr[num] = filtWetDry[num]->value();
            break;
        case 23:
            filtBalArr[num] = filtBal[num]->value();
            break;
        case 24:
            filtOutVolArr[num] = filtOutVol[num]->value();
            break;
        case 25:
            filtEnabledArr[num] = filtEnabled[num]->value();
            break;
        case 26:
            subEnabledArr[num] = subEnabled[num]->value();
            break;
        case 27:
            subVolArr[num] = subVol[num]->value();
            break;
        case 28:
            subPhaseArr[num] = subPhase[num]->value();
            break;
        case 29:
            subPhaseRandArr[num] = subPhaseRand[num]->value();
            break;
        case 30:
            subDetuneArr[num] = subDetune[num]->value();
            break;
        case 31:
            subMutedArr[num] = subMuted[num]->value();
            break;
        case 32:
            subKeytrackArr[num] = subKeytrack[num]->value();
            break;
        case 33:
            subSampLenArr[num] = subSampLen[num]->value();
            break;
        case 34:
            subNoiseArr[num] = subNoise[num]->value();
            break;
        case 35:
            modInArr[num] = modIn[num]->value();
            break;
        case 36:
            modInNumArr[num] = modInNum[num]->value();
            break;
        case 37:
            modInOtherNumArr[num] = modInOtherNum[num]->value();
            break;
        case 38:
            modInAmntArr[num] = modInAmnt[num]->value();
            break;
        case 39:
            modInCurveArr[num] = modInCurve[num]->value();
            break;
        case 40:
            modIn2Arr[num] = modIn2[num]->value();
            break;
        case 41:
            modInNum2Arr[num] = modInNum2[num]->value();
            break;
        case 42:
            modInOtherNum2Arr[num] = modInOtherNum2[num]->value();
            break;
        case 43:
            modInAmnt2Arr[num] = modInAmnt2[num]->value();
            break;
        case 44:
            modInCurve2Arr[num] = modInCurve2[num]->value();
            break;
        case 45:
            modOutSecArr[num] = modOutSec[num]->value();
            break;
        case 46:
            modOutSigArr[num] = modOutSig[num]->value();
            break;
        case 47:
            modOutSecNumArr[num] = modOutSecNum[num]->value();
            break;
        case 48:
            enabledArr[num] = enabled[num]->value();
            break;
        case 49:
            modEnabledArr[num] = modEnabled[num]->value();
            break;
        case 50:
            modCombineTypeArr[num] = modCombineType[num]->value();
            break;
        case 57:
            mutedArr[num] = muted[num]->value();
            break;
        case 59:
            sampleEnabledArr[num] = sampleEnabled[num]->value();
            break;
        case 60:
            sampleGraphEnabledArr[num] = sampleGraphEnabled[num]->value();
            break;
        case 61:
            sampleMutedArr[num] = sampleMuted[num]->value();
            break;
        case 62:
            sampleKeytrackingArr[num] = sampleKeytracking[num]->value();
            break;
        case 63:
            sampleLoopArr[num] = sampleLoop[num]->value();
            break;
        case 64:
            sampleVolumeArr[num] = sampleVolume[num]->value();
            break;
        case 65:
            samplePanningArr[num] = samplePanning[num]->value();
            break;
        case 66:
            sampleDetuneArr[num] = sampleDetune[num]->value();
            break;
        case 67:
            samplePhaseArr[num] = samplePhase[num]->value();
            break;
        case 68:
            samplePhaseRandArr[num] = samplePhaseRand[num]->value();
            break;
        case 69:
            filtFeedbackArr[num] = filtFeedback[num]->value();
            break;
        case 70:
            filtDetuneArr[num] = filtDetune[num]->value();
            break;
        case 71:
            filtKeytrackingArr[num] = filtKeytracking[num]->value();
            break;
        case 72:
            subPanningArr[num] = subPanning[num]->value();
            break;
        case 73:
            sampleStartArr[num] = sampleStart[num]->value();
            break;
        case 74:
            sampleEndArr[num] = sampleEnd[num]->value();
            break;
        case 75:
            panArr[num] = pan[num]->value();
            break;
        case 76:
            subTempoArr[num] = subTempo[num]->value();
            break;
        case 77:
            modTypeArr[num] = modType[num]->value();
            break;
        case 78:
            macroArr[num] = macro[num]->value();
            break;
        case 79:
            filtMutedArr[num] = filtMuted[num]->value();
            break;
        case 80:
            phaseRandArr[num] = phaseRand[num]->value();
            break;
    }

    ConstNotePlayHandles nphs
            //= NotePlayHandle::nphsOfInstrumentTrack(microwaveTrack);
            = Engine::mixer()->nphsOfTrack(microwaveTrack);

    nphs.map([this, which, num](const NotePlayHandle* nph) {
        /*
        MSynth* ps;
        do
        {
            ps = static_cast<MSynth*>(nph->m_pluginData);
        } while(!ps);
        */
        MSynth* ps = static_cast<MSynth*>(nph->m_pluginData);
        if(ps == nullptr)
        {
            qWarning("Microwave: null MSynth plugindata key=%d", nph->key());
            return;
        }
        // Makes sure "ps" isn't assigned a null value, if
        // m_pluginData hasn't been created yet.
        // Above is possible CPU concern

        // Send new knob values to notes already playing
        switch(which)
        {
            case 1:
                ps->morphVal[num] = morph[num]->value();
                break;
            case 2:
                ps->rangeVal[num] = range[num]->value();
                break;
            case 3:
                ps->modifyVal[num] = modify[num]->value();
                break;
            case 4:
                ps->modifyModeVal[num] = modifyMode[num]->value();
                break;
            case 6:
                ps->unisonVoices[num] = unisonVoices[num]->value();
                break;
            case 7:
                ps->unisonDetune[num] = unisonDetune[num]->value();
                break;
            case 8:
                ps->unisonMorph[num] = unisonMorph[num]->value();
                break;
            case 9:
                ps->unisonModify[num] = unisonModify[num]->value();
                break;
            case 10:
                ps->morphMaxVal[num] = morphMax[num]->value();
                break;
            case 11:
                ps->detuneVal[num] = detune[num]->value();
                break;
            case 12:
                ps->sampLen[num] = sampLen[num]->value();
                break;
            case 13:
                ps->phase[num] = phase[num]->value();
                break;
            case 14:
                ps->vol[num] = vol[num]->value();
                break;
            case 15:
                ps->filtInVol[num] = filtInVol[num]->value();
                break;
            case 16:
                ps->filtType[num] = filtType[num]->value();
                break;
            case 17:
                ps->filtSlope[num] = filtSlope[num]->value();
                break;
            case 18:
                ps->filtCutoff[num] = filtCutoff[num]->value();
                break;
            case 19:
                ps->filtReso[num] = filtReso[num]->value();
                break;
            case 20:
                ps->filtGain[num] = filtGain[num]->value();
                break;
            case 21:
                ps->filtSatu[num] = filtSatu[num]->value();
                break;
            case 22:
                ps->filtWetDry[num] = filtWetDry[num]->value();
                break;
            case 23:
                ps->filtBal[num] = filtBal[num]->value();
                break;
            case 24:
                ps->filtOutVol[num] = filtOutVol[num]->value();
                break;
            case 25:
                ps->filtEnabled[num] = filtEnabled[num]->value();
                break;
            case 26:
                ps->subEnabled[num] = subEnabled[num]->value();
                break;
            case 27:
                ps->subVol[num] = subVol[num]->value();
                break;
            case 28:
                ps->subPhase[num] = subPhase[num]->value();
                break;
            case 29:
                ps->subPhaseRand[num] = subPhaseRand[num]->value();
                break;
            case 30:
                ps->subDetune[num] = subDetune[num]->value();
                break;
            case 31:
                ps->subMuted[num] = subMuted[num]->value();
                break;
            case 32:
                ps->subKeytrack[num] = subKeytrack[num]->value();
                break;
            case 33:
                ps->subSampLen[num] = subSampLen[num]->value();
                break;
            case 34:
                ps->subNoise[num] = subNoise[num]->value();
                break;
            case 35:
                ps->modIn[num] = modIn[num]->value();
                break;
            case 36:
                ps->modInNum[num] = modInNum[num]->value();
                break;
            case 37:
                ps->modInOtherNum[num] = modInOtherNum[num]->value();
                break;
            case 38:
                ps->modInAmnt[num] = modInAmnt[num]->value();
                break;
            case 39:
                ps->modInCurve[num] = modInCurve[num]->value();
                break;
            case 40:
                ps->modIn2[num] = modIn2[num]->value();
                break;
            case 41:
                ps->modInNum2[num] = modInNum2[num]->value();
                break;
            case 42:
                ps->modInOtherNum2[num] = modInOtherNum2[num]->value();
                break;
            case 43:
                ps->modInAmnt2[num] = modInAmnt2[num]->value();
                break;
            case 44:
                ps->modInCurve2[num] = modInCurve2[num]->value();
                break;
            case 45:
                ps->modOutSec[num] = modOutSec[num]->value();
                break;
            case 46:
                ps->modOutSig[num] = modOutSig[num]->value();
                break;
            case 47:
                ps->modOutSecNum[num] = modOutSecNum[num]->value();
                break;
            case 48:
                ps->enabled[num] = enabled[num]->value();
                break;
            case 49:
                ps->modEnabled[num] = modEnabled[num]->value();
                break;
            case 50:
                ps->modCombineType[num] = modCombineType[num]->value();
                break;
            case 57:
                ps->muted[num] = muted[num]->value();
                break;
            case 59:
                ps->sampleEnabled[num] = sampleEnabled[num]->value();
                break;
            case 60:
                ps->sampleGraphEnabled[num]
                        = sampleGraphEnabled[num]->value();
                break;
            case 61:
                ps->sampleMuted[num] = sampleMuted[num]->value();
                break;
            case 62:
                ps->sampleKeytracking[num] = sampleKeytracking[num]->value();
                break;
            case 63:
                ps->sampleLoop[num] = sampleLoop[num]->value();
                break;
            case 64:
                ps->sampleVolume[num] = sampleVolume[num]->value();
                break;
            case 65:
                ps->samplePanning[num] = samplePanning[num]->value();
                break;
            case 66:
                ps->sampleDetune[num] = sampleDetune[num]->value();
                break;
            case 67:
                ps->samplePhase[num] = samplePhase[num]->value();
                break;
            case 68:
                ps->samplePhaseRand[num] = samplePhaseRand[num]->value();
                break;
            case 69:
                ps->filtFeedback[num] = filtFeedback[num]->value();
                break;
            case 70:
                ps->filtDetune[num] = filtDetune[num]->value();
                break;
            case 71:
                ps->filtKeytracking[num] = filtKeytracking[num]->value();
                break;
            case 72:
                ps->subPanning[num] = subPanning[num]->value();
                break;
            case 73:
                ps->sampleStart[num] = sampleStart[num]->value();
                break;
            case 74:
                ps->sampleEnd[num] = sampleEnd[num]->value();
                break;
            case 75:
                ps->pan[num] = pan[num]->value();
                break;
            case 76:
                ps->subTempo[num] = subTempo[num]->value();
                break;
            case 77:
                ps->modType[num] = modType[num]->value();
                break;
            case 78:
                ps->macro[num] = macro[num]->value();
                break;
            case 79:
                ps->filtMuted[num] = filtMuted[num]->value();
                break;
            case 80:
                ps->phaseRand[num] = phaseRand[num]->value();
                break;
        }
    });
}

// Set the range of Morph based on Morph Max
void Microwave::morphMaxChanged()
{
    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        morph[i]->setRange(morph[i]->minValue(), morphMax[i]->value(),
                           morph[i]->step());
    }
}

// Set the range of morphMax and Modify based on new sample length
void Microwave::sampLenChanged()
{
    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        morphMax[i]->setRange(morphMax[i]->minValue(),
                              real_t(MAINWAV_SIZE) / sampLen[i]->value() - 2.,
                              morphMax[i]->step());
        modify[i]->setRange(modify[i]->minValue(), sampLen[i]->value() - 1,
                            modify[i]->step());
    }
}

// Change graph length to sample length
void Microwave::subSampLenChanged(int32_t num)
{
    graph.setLength(subSampLen[num]->value());
}

// Stores the highest enabled main oscillator.  Helps with CPU benefit,
// refer to its use in MSynth::nextStringSample
void Microwave::mainEnabledChanged(int32_t num)
{
    for(int i = 0; i < NB_MAINOSC; ++i)
        if(enabled[i]->value())
            maxMainEnabled = i + 1;
}

// Stores the highest enabled sub oscillator.  Helps with major CPU
// benefit, refer to its use in MSynth::nextStringSample
void Microwave::subEnabledChanged(int32_t num)
{
    for(int i = 0; i < NB_SUBOSC; ++i)
        if(subEnabled[i]->value())
            maxSubEnabled = i + 1;
}

// Stores the highest enabled mod section.  Helps with major CPU benefit,
// refer to its use in MSynth::nextStringSample
void Microwave::modEnabledChanged(int32_t num)
{
    for(int i = 0; i < NB_MODLT; ++i)
        if(modEnabled[i]->value())
            maxModEnabled = i + 1;

    // matrixScrollBar.setRange( 0, maxModEnabled * 100. );
}

// Stores the highest enabled sample.  Helps with CPU benefit, refer to
// its use in MSynth::nextStringSample
void Microwave::sampleEnabledChanged(int32_t num)
{
    for(int i = 0; i < NB_SMPLR; ++i)
        if(sampleEnabled[i]->value())
            maxSampleEnabled = i + 1;
}

// Stores the highest enabled filter section.  Helps with CPU benefit,
// refer to its use in MSynth::nextStringSample
void Microwave::filtEnabledChanged(int32_t num)
{
    for(int i = 0; i < NB_FILTR; ++i)
        if(filtEnabled[i]->value())
            maxFiltEnabled = i + 1;
}

// When user drawn on graph, send new values to the correct arrays
void Microwave::samplesChanged(int32_t _begin, int32_t _end)
{
    switch(currentTab)
    {
        case 0:
            // Main
            break;
        case 1:
            // Sub
            for(int i = _begin; i <= _end; ++i)
                subs[i + ((subNum.value() - 1) * WAVE_SIZE)]
                        = graph.samples()[i];
            break;
        case 2:
            // Smplr
            for(int i = _begin; i <= _end; ++i)
                sampGraphs[i + ((sampNum.value() - 1) * 128)]
                        = graph.samples()[i];
            break;
        default:
            break;
    }
}

void Microwave::switchMatrixSections(int32_t source, int32_t destination)
{
    int32_t modInTemp          = modInArr[destination];
    int32_t modInNumTemp       = modInNumArr[destination];
    real_t  modInOtherNumTemp  = modInOtherNumArr[destination];
    real_t  modInAmntTemp      = modInAmntArr[destination];
    real_t  modInCurveTemp     = modInCurveArr[destination];
    int32_t modIn2Temp         = modIn2Arr[destination];
    int32_t modInNum2Temp      = modInNum2Arr[destination];
    int32_t modInOtherNum2Temp = modInOtherNum2Arr[destination];
    real_t  modInAmnt2Temp     = modInAmnt2Arr[destination];
    real_t  modInCurve2Temp    = modInCurve2Arr[destination];
    int32_t modOutSecTemp      = modOutSecArr[destination];
    int32_t modOutSigTemp      = modOutSigArr[destination];
    int32_t modOutSecNumTemp   = modOutSecNumArr[destination];
    bool    modEnabledTemp     = modEnabledArr[destination];
    int32_t modCombineTypeTemp = modCombineTypeArr[destination];
    bool    modTypeTemp        = modTypeArr[destination];

    modIn[destination]->setValue(modInArr[source]);
    modInNum[destination]->setValue(modInNumArr[source]);
    modInOtherNum[destination]->setValue(modInOtherNumArr[source]);
    modInAmnt[destination]->setValue(modInAmntArr[source]);
    modInCurve[destination]->setValue(modInCurveArr[source]);
    modIn2[destination]->setValue(modIn2Arr[source]);
    modInNum2[destination]->setValue(modInNum2Arr[source]);
    modInOtherNum2[destination]->setValue(modInOtherNum2Arr[source]);
    modInAmnt2[destination]->setValue(modInAmnt2Arr[source]);
    modInCurve2[destination]->setValue(modInCurve2Arr[source]);
    modOutSec[destination]->setValue(modOutSecArr[source]);
    modOutSig[destination]->setValue(modOutSigArr[source]);
    modOutSecNum[destination]->setValue(modOutSecNumArr[source]);
    modEnabled[destination]->setValue(modEnabledArr[source]);
    modCombineType[destination]->setValue(modCombineTypeArr[source]);
    modType[destination]->setValue(modTypeArr[source]);

    modIn[source]->setValue(modInTemp);
    modInNum[source]->setValue(modInNumTemp);
    modInOtherNum[source]->setValue(modInOtherNumTemp);
    modInAmnt[source]->setValue(modInAmntTemp);
    modInCurve[source]->setValue(modInCurveTemp);
    modIn2[source]->setValue(modIn2Temp);
    modInNum2[source]->setValue(modInNum2Temp);
    modInOtherNum2[source]->setValue(modInOtherNum2Temp);
    modInAmnt2[source]->setValue(modInAmnt2Temp);
    modInCurve2[source]->setValue(modInCurve2Temp);
    modOutSec[source]->setValue(modOutSecTemp);
    modOutSig[source]->setValue(modOutSigTemp);
    modOutSecNum[source]->setValue(modOutSecNumTemp);
    modEnabled[source]->setValue(modEnabledTemp);
    modCombineType[source]->setValue(modCombineTypeTemp);
    modType[source]->setValue(modTypeTemp);
}

// For when notes are playing.  This initializes a new MSynth if the note
// is new.  It also uses MSynth::nextStringSample to get the synthesizer
// output. This is where oversampling and the visualizer are handled.
void Microwave::playNote(NotePlayHandle* _n, sampleFrame* _buf)
{
    // qInfo("Microwave::playNote start");

    MSynth* ps = static_cast<MSynth*>(_n->m_pluginData);
    if(ps == nullptr)  // || _n->totalFramesPlayed() == 0)
    {
        // qInfo("Microwave::playNote new MSynth");
        ps = new MSynth(
                _n, Engine::mixer()->processingSampleRate(),

                morphArr, rangeArr, modifyArr, modifyModeArr, sampLenArr,
                enabledArr, mutedArr, detuneArr, unisonVoicesArr,
                unisonDetuneArr, unisonMorphArr, unisonModifyArr, morphMaxArr,
                phaseArr, phaseRandArr, volArr, panArr,
                //waveforms,

                subEnabledArr, subVolArr, subPhaseArr, subPhaseRandArr,
                subDetuneArr, subMutedArr, subKeytrackArr, subSampLenArr,
                subNoiseArr, subPanningArr, subTempoArr,
                //subs,

                modInArr, modInNumArr, modInOtherNumArr, modInAmntArr,
                modInCurveArr, modIn2Arr, modInNum2Arr, modInOtherNum2Arr,
                modInAmnt2Arr, modInCurve2Arr, modOutSecArr, modOutSigArr,
                modOutSecNumArr, modCombineTypeArr, modTypeArr, modEnabledArr,

                filtInVolArr, filtTypeArr, filtSlopeArr, filtCutoffArr,
                filtResoArr, filtGainArr, filtSatuArr, filtWetDryArr,
                filtBalArr, filtOutVolArr, filtEnabledArr, filtFeedbackArr,
                filtDetuneArr, filtKeytrackingArr, filtMutedArr,

                // sampGraphs,

                sampleEnabledArr, sampleGraphEnabledArr, sampleMutedArr,
                sampleKeytrackingArr, sampleLoopArr, sampleVolumeArr,
                samplePanningArr, sampleDetuneArr, samplePhaseArr,
                samplePhaseRandArr, sampleStartArr, sampleEndArr, samples,

                macroArr);

        _n->m_pluginData = ps;
    }

    const fpp_t   frames = _n->framesLeftForCurrentPeriod();
    const f_cnt_t offset = _n->noteOffset();

    for(fpp_t frame = offset; frame < frames + offset; ++frame)
    {
        // Process some samples and ignore the output, depending on the
        // oversampling value.  For example, if the oversampling is set to
        // 4x, it will process 4 samples and output 1 of those.
        for(int i = 0; i < oversample.value(); ++i)
        {
            ps->nextStringSample(waveforms, subs, sampGraphs, samples,
                                 maxFiltEnabled, maxModEnabled, maxSubEnabled,
                                 maxSampleEnabled, maxMainEnabled,
                                 Engine::mixer()->processingSampleRate()
                                         * (oversample.value() + 1));
        }
        // Get the actual synthesizer output
        StereoSample sample = ps->nextStringSample(
                waveforms, subs, sampGraphs, samples, maxFiltEnabled,
                maxModEnabled, maxSubEnabled, maxSampleEnabled,
                maxMainEnabled,
                Engine::mixer()->processingSampleRate()
                        * (oversample.value() + 1));
        _buf[frame][0] = sample[0];
        _buf[frame][1] = sample[1];
        /*
        for(ch_cnt_t chnl = 0; chnl < DEFAULT_CHANNELS; ++chnl)
        {
            // Send to output
            _buf[frame][chnl] = sample[chnl];
        }
        */

        // update visualizer
        if(visualize.value())
        {
            if(abs(const_cast<float*>(
                           graph.samples())[int(ps->sample_realindex[0][0])]
                   - (((sample[0] + sample[1]) * 0.5) * visvol.value()
                      * 0.01))
               >= 0.01)
            {
                graph.setSampleAt(ps->sample_realindex[0][0],
                                  ((sample[0] + sample[1]) * 0.5)
                                          * visvol.value() * 0.01);
            }
        }
    }

    applyRelease(_buf, _n);

    instrumentTrack()->processAudioBuffer(_buf, frames + offset, _n);
}

void Microwave::deleteNotePluginData(NotePlayHandle* _n)
{
}

extern "C"
{

    // necessary for getting instance out of shared lib
    PLUGIN_EXPORT Plugin* lmms_plugin_main(Model* m, void*)
    {
        return (new Microwave(static_cast<InstrumentTrack*>(m)));
    }
}
