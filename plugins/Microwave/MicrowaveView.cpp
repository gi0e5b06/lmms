/*
 * MicrowaveView.cpp -
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

#include "MicrowaveView.h"

#include "CaptionMenu.h"
#include "Engine.h"
#include "FileDialog.h"
#include "Graph.h"
#include "GuiApplication.h"
#include "InstrumentTrack.h"
#include "Knob.h"
#include "LcdSpinBox.h"
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

// Creates the Microwave GUI.  Creates all GUI elements.  Connects some events
// to some functions.  Calls updateScroll() to put all of the GUi elements in
// their correct positions.
MicrowaveView::MicrowaveView(Microwave* _instrument, QWidget* _parent) :
      InstrumentView(_instrument, _parent)
{
    setAutoFillBackground(true);
    setMouseTracking(true);
    setAcceptDrops(true);
    pal.setBrush(backgroundRole(), tab1ArtworkImg);
    setPalette(pal);

    QWidget* view = new QWidget(_parent);
    view->setFixedSize(250, 250);

    QPixmap filtBoxesImg = PLUGIN_NAME::getPixmap("filterBoxes");
    filtBoxesLabel       = new QLabel(this);
    filtBoxesLabel->setPixmap(filtBoxesImg);
    filtBoxesLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QPixmap matrixBoxesImg = PLUGIN_NAME::getPixmap("matrixBoxes");
    matrixBoxesLabel       = new QLabel(this);
    matrixBoxesLabel->setPixmap(matrixBoxesImg);
    matrixBoxesLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        morphKnob[i] = new Knob(this);
        morphKnob[i]->setText("MPH");
        morphKnob[i]->setHintText(tr("Morph"), "");

        rangeKnob[i] = new Knob(this);
        rangeKnob[i]->setText("RNG");
        rangeKnob[i]->setHintText(tr("Range"), "");

        sampLenKnob[i] = new Knob(this);
        sampLenKnob[i]->setText("??1");
        sampLenKnob[i]->setHintText(tr("Waveform Sample Length"), "");

        morphMaxKnob[i] = new Knob(this);
        morphMaxKnob[i]->setText("??2");
        morphMaxKnob[i]->setHintText(tr("Morph Max"), "");

        modifyKnob[i] = new Knob(this);
        modifyKnob[i]->setText("MDFY");
        modifyKnob[i]->setHintText(tr("Modify"), "");

        unisonVoicesKnob[i] = new Knob(this);
        unisonVoicesKnob[i]->setText("NUM");
        unisonVoicesKnob[i]->setHintText(tr("Unison Voices"), "");

        unisonDetuneKnob[i] = new Knob(this);
        unisonDetuneKnob[i]->setText("DET");
        unisonDetuneKnob[i]->setHintText(tr("Unison Detune"), "");

        unisonMorphKnob[i] = new Knob(this);
        unisonMorphKnob[i]->setText("MPH");
        unisonMorphKnob[i]->setHintText(tr("Unison Morph"), "");

        unisonModifyKnob[i] = new Knob(this);
        unisonModifyKnob[i]->setText("MOD");
        unisonModifyKnob[i]->setHintText(tr("Unison Modify"), "");

        detuneKnob[i] = new Knob(this);
        detuneKnob[i]->setText("DET");
        detuneKnob[i]->setHintText(tr("Detune"), "");

        phaseKnob[i] = new Knob(this);
        phaseKnob[i]->setText("??3");
        phaseKnob[i]->setHintText(tr("Phase"), "");

        phaseRandKnob[i] = new Knob(this);
        phaseRandKnob[i]->setText("??4");
        phaseRandKnob[i]->setHintText(tr("Phase Randomness"), "");

        volKnob[i] = new Knob(this);
        volKnob[i]->setText("??5");
        volKnob[i]->setHintText(tr("Volume"), "");

        panKnob[i] = new Knob(this);
        panKnob[i]->setText("??7");
        panKnob[i]->setHintText(tr("Panning"), "");

        modifyModeBox[i] = new ComboBox(this);
        modifyModeBox[i]->setGeometry(0, 5, 42, 22);
        modifyModeBox[i]->setFont(pointSize<8>(modifyModeBox[i]->font()));

        enabledToggle[i] = new LedCheckBox("", this, tr("Oscillator Enabled"),
                                           LedCheckBox::Green);

        mutedToggle[i] = new LedCheckBox("", this, tr("Oscillator Muted"),
                                         LedCheckBox::Green);

        if(i != 0)
        {
            morphKnob[i]->hide();
            rangeKnob[i]->hide();
            sampLenKnob[i]->hide();
            morphMaxKnob[i]->hide();
            modifyKnob[i]->hide();
            unisonVoicesKnob[i]->hide();
            unisonDetuneKnob[i]->hide();
            unisonMorphKnob[i]->hide();
            unisonModifyKnob[i]->hide();
            detuneKnob[i]->hide();
            phaseKnob[i]->hide();
            phaseRandKnob[i]->hide();
            modifyModeBox[i]->hide();
            volKnob[i]->hide();
            enabledToggle[i]->hide();
            mutedToggle[i]->hide();
        }
    }

    for(int i = 0; i < NB_FILTR; ++i)
    {
        filtInVolKnob[i] = new Knob(this);
        filtInVolKnob[i]->setText("IN");
        filtInVolKnob[i]->setHintText(tr("Input Volume"), "");

        filtTypeBox[i] = new ComboBox(this);
        filtTypeBox[i]->setGeometry(1000, 5, 42, 22);
        filtTypeBox[i]->setFont(pointSize<8>(filtTypeBox[i]->font()));

        filtSlopeBox[i] = new ComboBox(this);
        filtSlopeBox[i]->setGeometry(1000, 5, 42, 22);
        filtSlopeBox[i]->setFont(pointSize<8>(filtSlopeBox[i]->font()));

        filtCutoffKnob[i] = new Knob(this);
        filtCutoffKnob[i]->setText("FREQ");
        filtCutoffKnob[i]->setHintText(tr("Cutoff Frequency"), "");

        filtResoKnob[i] = new Knob(this);
        filtResoKnob[i]->setText("RESO");
        filtResoKnob[i]->setHintText(tr("Resonance"), "");

        filtGainKnob[i] = new Knob(this);
        filtGainKnob[i]->setText("GAIN");
        filtGainKnob[i]->setHintText(tr("db Gain"), "");

        filtSatuKnob[i] = new Knob(this);
        filtSatuKnob[i]->setText("SAT");
        filtSatuKnob[i]->setHintText(tr("Saturation"), "");

        filtWetDryKnob[i] = new Knob(this);
        filtWetDryKnob[i]->setText("W/D");
        filtWetDryKnob[i]->setHintText(tr("Wet/Dry"), "");

        filtBalKnob[i] = new Knob(this);
        filtBalKnob[i]->setText("PAN");
        filtBalKnob[i]->setHintText(tr("Balance/Panning"), "");

        filtOutVolKnob[i] = new Knob(this);
        filtOutVolKnob[i]->setText("OUT");
        filtOutVolKnob[i]->setHintText(tr("Output Volume"), "");

        filtEnabledToggle[i] = new LedCheckBox("", this, tr("Filter Enabled"),
                                               LedCheckBox::Green);

        filtFeedbackKnob[i] = new Knob(this);
        filtFeedbackKnob[i]->setText("FDBK");
        filtFeedbackKnob[i]->setHintText(tr("Feedback"), "");

        filtDetuneKnob[i] = new Knob(this);
        filtDetuneKnob[i]->setText("DET");
        filtDetuneKnob[i]->setHintText(tr("Detune"), "");

        filtKeytrackingToggle[i] = new LedCheckBox(
                "", this, tr("Keytracking"), LedCheckBox::Green);

        filtMutedToggle[i]
                = new LedCheckBox("", this, tr("Muted"), LedCheckBox::Green);
    }

    for(int i = 0; i < NB_SMPLR; ++i)
    {
        sampleEnabledToggle[i] = new LedCheckBox(
                "", this, tr("Sample Enabled"), LedCheckBox::Green);
        sampleGraphEnabledToggle[i] = new LedCheckBox(
                "", this, tr("Sample Graph Enabled"), LedCheckBox::Green);
        sampleMutedToggle[i] = new LedCheckBox("", this, tr("Sample Muted"),
                                               LedCheckBox::Green);
        sampleKeytrackingToggle[i] = new LedCheckBox(
                "", this, tr("Sample Keytracking"), LedCheckBox::Green);
        sampleLoopToggle[i] = new LedCheckBox("", this, tr("Loop Sample"),
                                              LedCheckBox::Green);

        sampleVolumeKnob[i] = new Knob(this);
        sampleVolumeKnob[i]->setText("VOL");
        sampleVolumeKnob[i]->setHintText(tr("Volume"), "");

        samplePanningKnob[i] = new Knob(this);
        samplePanningKnob[i]->setText("PAN");
        samplePanningKnob[i]->setHintText(tr("Panning"), "");

        sampleDetuneKnob[i] = new Knob(this);
        sampleDetuneKnob[i]->setText("DET");
        sampleDetuneKnob[i]->setHintText(tr("Detune"), "");

        samplePhaseKnob[i] = new Knob(this);
        samplePhaseKnob[i]->setText("PHS");
        samplePhaseKnob[i]->setHintText(tr("Phase"), "");

        samplePhaseRandKnob[i] = new Knob(this);
        samplePhaseRandKnob[i]->setText("RAND");
        samplePhaseRandKnob[i]->setHintText(tr("Phase Randomness"), "");

        sampleStartKnob[i] = new Knob(this);
        sampleStartKnob[i]->setHintText(tr("Start"), "");

        sampleEndKnob[i] = new Knob(this);
        sampleEndKnob[i]->setHintText(tr("End"), "");

        if(i != 0)
        {
            sampleEnabledToggle[i]->hide();
            sampleGraphEnabledToggle[i]->hide();
            sampleMutedToggle[i]->hide();
            sampleKeytrackingToggle[i]->hide();
            sampleLoopToggle[i]->hide();
            sampleVolumeKnob[i]->hide();
            samplePanningKnob[i]->hide();
            sampleDetuneKnob[i]->hide();
            samplePhaseKnob[i]->hide();
            samplePhaseRandKnob[i]->hide();
            sampleStartKnob[i]->hide();
            sampleEndKnob[i]->hide();
            panKnob[i]->hide();
        }
    }

    for(int i = 0; i < NB_MACRO; ++i)
    {
        macroKnob[i] = new Knob(this);
        macroKnob[i]->setText("??8");
        macroKnob[i]->setHintText(tr("Macro") + " " + QString::number(i), "");
    }

    for(int i = 0; i < NB_SUBOSC; ++i)
    {
        subEnabledToggle[i] = new LedCheckBox(
                "", this, tr("Sub Oscillator Enabled"), LedCheckBox::Green);

        subVolKnob[i] = new Knob(this);
        subVolKnob[i]->setText("VOL");
        subVolKnob[i]->setHintText(tr("Sub Oscillator Volume"), "");

        subPhaseKnob[i] = new Knob(this);
        subPhaseKnob[i]->setText("PHS");
        subPhaseKnob[i]->setHintText(tr("Sub Oscillator Phase"), "");

        subPhaseRandKnob[i] = new Knob(this);
        subPhaseRandKnob[i]->setText("RAND");
        subPhaseRandKnob[i]->setHintText(
                tr("Sub Oscillator Phase Randomness"), "");

        subDetuneKnob[i] = new Knob(this);
        subDetuneKnob[i]->setText("??9");
        subDetuneKnob[i]->setHintText(tr("Sub Oscillator Pitch"), "");

        subMutedToggle[i] = new LedCheckBox(
                "", this, tr("Sub Oscillator Muted"), LedCheckBox::Green);

        subKeytrackToggle[i] = new LedCheckBox(
                "", this, tr("Sub Oscillator Keytracking Enabled"),
                LedCheckBox::Green);

        subSampLenKnob[i] = new Knob(this);
        subSampLenKnob[i]->setText("?10");
        subSampLenKnob[i]->setHintText(tr("Sub Oscillator Sample Length"),
                                       "");

        subNoiseToggle[i] = new LedCheckBox(
                "", this, tr("Sub Oscillator Noise Enabled"),
                LedCheckBox::Green);

        subPanningKnob[i] = new Knob(this);
        subPanningKnob[i]->setText("PAN");
        subPanningKnob[i]->setHintText(tr("Sub Oscillator Panning"), "");

        subTempoKnob[i] = new Knob(this);
        subTempoKnob[i]->setText("?11");
        subTempoKnob[i]->setHintText(tr("Sub Oscillator Tempo"), "");

        if(i != 0)
        {
            subEnabledToggle[i]->hide();
            subVolKnob[i]->hide();
            subPhaseKnob[i]->hide();
            subPhaseRandKnob[i]->hide();
            subDetuneKnob[i]->hide();
            subMutedToggle[i]->hide();
            subKeytrackToggle[i]->hide();
            subSampLenKnob[i]->hide();
            subNoiseToggle[i]->hide();
            subPanningKnob[i]->hide();
            subTempoKnob[i]->hide();
        }
    }

    for(int i = 0; i < NB_MODLT; ++i)
    {
        modOutSecBox[i] = new ComboBox(this);
        modOutSecBox[i]->setGeometry(2000, 5, 42, 22);
        modOutSecBox[i]->setFont(pointSize<8>(modOutSecBox[i]->font()));

        modOutSigBox[i] = new ComboBox(this);
        modOutSigBox[i]->setGeometry(2000, 5, 42, 22);
        modOutSigBox[i]->setFont(pointSize<8>(modOutSigBox[i]->font()));

        modOutSecNumBox[i]
                = new LcdSpinBox(2, "microwave", this, "Mod Output Number");

        modInBox[i] = new ComboBox(this);
        modInBox[i]->setGeometry(2000, 5, 42, 22);
        modInBox[i]->setFont(pointSize<8>(modInBox[i]->font()));

        modInNumBox[i]
                = new LcdSpinBox(2, "microwave", this, "Mod Input Number");

        modInOtherNumBox[i]
                = new LcdSpinBox(2, "microwave", this, "Mod Input Number");

        modInAmntKnob[i] = new Knob(this);
        modInAmntKnob[i]->setText("AMT");
        modInAmntKnob[i]->setHintText(tr("Modulator Amount"), "");

        modInCurveKnob[i] = new Knob(this);
        modInCurveKnob[i]->setText("CRV");
        modInCurveKnob[i]->setHintText(tr("Modulator Curve"), "");

        modInBox2[i] = new ComboBox(this);
        modInBox2[i]->setGeometry(2000, 5, 42, 22);
        modInBox2[i]->setFont(pointSize<8>(modInBox2[i]->font()));

        modInNumBox2[i] = new LcdSpinBox(2, "microwave", this,
                                         "Secondary Mod Input Number");

        modInOtherNumBox2[i] = new LcdSpinBox(2, "microwave", this,
                                              "Secondary Mod Input Number");

        modInAmntKnob2[i] = new Knob(this);
        modInAmntKnob2[i]->setText("AMT");
        modInAmntKnob2[i]->setHintText(tr("Secondary Modulator Amount"), "");

        modInCurveKnob2[i] = new Knob(this);
        modInCurveKnob2[i]->setText("CRV");
        modInCurveKnob2[i]->setHintText(tr("Secondary Modulator Curve"), "");

        modEnabledToggle[i] = new LedCheckBox(
                "", this, tr("Modulation Enabled"), LedCheckBox::Green);

        modCombineTypeBox[i] = new ComboBox(this);
        modCombineTypeBox[i]->setGeometry(2000, 5, 42, 22);
        modCombineTypeBox[i]->setFont(
                pointSize<8>(modCombineTypeBox[i]->font()));

        modTypeToggle[i] = new LedCheckBox("", this, tr("Envelope Enabled"),
                                           LedCheckBox::Green);

        modUpArrow[i] = new PixmapButton(this, tr("Move Matrix Section Up"));
        modUpArrow[i]->setActiveGraphic(PLUGIN_NAME::getPixmap("arrowup"));
        modUpArrow[i]->setInactiveGraphic(PLUGIN_NAME::getPixmap("arrowup"));
        ToolTip::add(modUpArrow[i], tr("Move Matrix Section Up"));

        modDownArrow[i]
                = new PixmapButton(this, tr("Move Matrix Section Down"));
        modDownArrow[i]->setActiveGraphic(
                PLUGIN_NAME::getPixmap("arrowdown"));
        modDownArrow[i]->setInactiveGraphic(
                PLUGIN_NAME::getPixmap("arrowdown"));
        ToolTip::add(modDownArrow[i], tr("Move Matrix Section Down"));
    }

    visvolKnob = new Knob(this);
    visvolKnob->setText("VIS");
    visvolKnob->setHintText(tr("Visualizer Volume"), "");

    loadChnlKnob = new Knob(this);
    loadChnlKnob->setText("?17");
    loadChnlKnob->setHintText(tr("Wavetable Loading Channel"), "");

    wtLoad1Knob = new Knob(this);
    wtLoad1Knob->setHintText(tr("Wavetable Loading Knob 1"), "");

    wtLoad2Knob = new Knob(this);
    wtLoad2Knob->setHintText(tr("Wavetable Loading Knob 2"), "");

    wtLoad3Knob = new Knob(this);
    wtLoad3Knob->setHintText(tr("Wavetable Loading Knob 3"), "");

    wtLoad4Knob = new Knob(this);
    wtLoad4Knob->setHintText(tr("Wavetable Loading Knob 4"), "");

    graph = new Graph(this, Graph::BarStyle, 204, 134);
    graph->setAutoFillBackground(true);
    graph->setGraphColor(QColor(121, 222, 239));

    ToolTip::add(graph, tr("Draw your own waveform here "
                           "by dragging your mouse on this graph."));

    pal = QPalette();
    pal.setBrush(backgroundRole(), PLUGIN_NAME::getPixmap("wavegraph"));
    graph->setPalette(pal);

    QPixmap filtForegroundImg = PLUGIN_NAME::getPixmap("filterForeground");
    filtForegroundLabel       = new QLabel(this);
    filtForegroundLabel->setPixmap(filtForegroundImg);
    filtForegroundLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QPixmap matrixForegroundImg = PLUGIN_NAME::getPixmap("matrixForeground");
    matrixForegroundLabel       = new QLabel(this);
    matrixForegroundLabel->setPixmap(matrixForegroundImg);
    matrixForegroundLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    sinWaveBtn = new PixmapButton(this, tr("Sine"));
    sinWaveBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("sinwave"));
    sinWaveBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("sinwave"));
    ToolTip::add(sinWaveBtn, tr("Sine wave"));

    triangleWaveBtn = new PixmapButton(this, tr("Nachos"));
    triangleWaveBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("triwave"));
    triangleWaveBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("triwave"));
    ToolTip::add(triangleWaveBtn, tr("Nacho wave"));

    sawWaveBtn = new PixmapButton(this, tr("Sawsa"));
    sawWaveBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("sawwave"));
    sawWaveBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("sawwave"));
    ToolTip::add(sawWaveBtn, tr("Sawsa wave"));

    sqrWaveBtn = new PixmapButton(this, tr("Sosig"));
    sqrWaveBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("sqrwave"));
    sqrWaveBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("sqrwave"));
    ToolTip::add(sqrWaveBtn, tr("Sosig wave"));

    whiteNoiseWaveBtn = new PixmapButton(this, tr("Metal Fork"));
    whiteNoiseWaveBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("noisewave"));
    whiteNoiseWaveBtn->setInactiveGraphic(
            PLUGIN_NAME::getPixmap("noisewave"));
    ToolTip::add(whiteNoiseWaveBtn, tr("Metal Fork"));

    usrWaveBtn = new PixmapButton(this, tr("Takeout"));
    usrWaveBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    usrWaveBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    ToolTip::add(usrWaveBtn, tr("Takeout Menu"));

    smoothBtn = new PixmapButton(this, tr("Microwave Cover"));
    smoothBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("smoothwave"));
    smoothBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("smoothwave"));
    ToolTip::add(smoothBtn, tr("Microwave Cover"));

    sinWave2Btn = new PixmapButton(this, tr("Sine"));
    sinWave2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("sinwave"));
    sinWave2Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("sinwave"));
    ToolTip::add(sinWave2Btn, tr("Sine wave"));

    triangleWave2Btn = new PixmapButton(this, tr("Nachos"));
    triangleWave2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("triwave"));
    triangleWave2Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("triwave"));
    ToolTip::add(triangleWave2Btn, tr("Nacho wave"));

    sawWave2Btn = new PixmapButton(this, tr("Sawsa"));
    sawWave2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("sawwave"));
    sawWave2Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("sawwave"));
    ToolTip::add(sawWave2Btn, tr("Sawsa wave"));

    sqrWave2Btn = new PixmapButton(this, tr("Sosig"));
    sqrWave2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("sqrwave"));
    sqrWave2Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("sqrwave"));
    ToolTip::add(sqrWave2Btn, tr("Sosig wave"));

    whiteNoiseWave2Btn = new PixmapButton(this, tr("Metal Fork"));
    whiteNoiseWave2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("noisewave"));
    whiteNoiseWave2Btn->setInactiveGraphic(
            PLUGIN_NAME::getPixmap("noisewave"));
    ToolTip::add(whiteNoiseWave2Btn, tr("Metal Fork"));

    usrWave2Btn = new PixmapButton(this, tr("Takeout"));
    usrWave2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    usrWave2Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    ToolTip::add(usrWave2Btn, tr("Takeout Menu"));

    smooth2Btn = new PixmapButton(this, tr("Microwave Cover"));
    smooth2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("smoothwave"));
    smooth2Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("smoothwave"));
    ToolTip::add(smooth2Btn, tr("Microwave Cover"));

    tab1Btn = new PixmapButton(this, tr("Wavetable Tab"));
    tab1Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab1_active"));
    tab1Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab1_active"));
    ToolTip::add(tab1Btn, tr("Wavetable Tab"));

    tab2Btn = new PixmapButton(this, tr("Sub Tab"));
    tab2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab2"));
    tab2Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab2"));
    ToolTip::add(tab2Btn, tr("Sub Tab"));

    tab3Btn = new PixmapButton(this, tr("Sample Tab"));
    tab3Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab3"));
    tab3Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab3"));
    ToolTip::add(tab3Btn, tr("Sample Tab"));

    tab4Btn = new PixmapButton(this, tr("Matrix Tab"));
    tab4Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab4"));
    tab4Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab4"));
    ToolTip::add(tab4Btn, tr("Matrix Tab"));

    tab5Btn = new PixmapButton(this, tr("Effect Tab"));
    tab5Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab5"));
    tab5Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab5"));
    ToolTip::add(tab5Btn, tr("Effect Tab"));

    tab6Btn = new PixmapButton(this, tr("Miscellaneous Tab"));
    tab6Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab6"));
    tab6Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab6"));
    ToolTip::add(tab6Btn, tr("Miscellaneous Tab"));

    mainFlipBtn = new PixmapButton(this, tr("Flip to other knobs"));
    mainFlipBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("arrowup"));
    mainFlipBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("arrowdown"));
    ToolTip::add(mainFlipBtn, tr("Flip to other knobs"));
    mainFlipBtn->setCheckable(true);

    subFlipBtn = new PixmapButton(this, tr("Flip to other knobs"));
    subFlipBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("arrowup"));
    subFlipBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("arrowdown"));
    ToolTip::add(subFlipBtn, tr("Flip to other knobs"));
    subFlipBtn->setCheckable(true);

    manualBtn = new PixmapButton(this, tr("Manual"));
    manualBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("sinwave"));
    manualBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("sinwave"));
    ToolTip::add(manualBtn, tr("Manual"));

    XBtn = new PixmapButton(this, tr("Leave wavetable loading tab"));
    XBtn->setActiveGraphic(PLUGIN_NAME::getPixmap("xbtn"));
    XBtn->setInactiveGraphic(PLUGIN_NAME::getPixmap("xbtn"));
    ToolTip::add(XBtn, tr("Leave wavetable loading tab"));

    visualizeToggle
            = new LedCheckBox("", this, tr("Visualize"), LedCheckBox::Green);

    openWaveFormBTN = new PixmapButton(this);
    openWaveFormBTN->setCursor(QCursor(Qt::PointingHandCursor));
    openWaveFormBTN->setActiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    openWaveFormBTN->setInactiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    ToolTip::add(openWaveFormBTN, tr("Open waveform"));

    confirmLoadBTN = new PixmapButton(this);
    confirmLoadBTN->setCursor(QCursor(Qt::PointingHandCursor));
    confirmLoadBTN->setActiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    confirmLoadBTN->setInactiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    ToolTip::add(confirmLoadBTN, tr("Load Wavetable"));

    subNumBox = new LcdSpinBox(2, "microwave", this, "Sub Oscillator Number");

    sampNumBox = new LcdSpinBox(2, "microwave", this, "Sample Number");

    mainNumBox = new LcdSpinBox(2, "microwave", this, "Oscillator Number");

    oversampleBox = new ComboBox(this);
    oversampleBox->setGeometry(0, 0, 42, 22);
    oversampleBox->setFont(pointSize<8>(oversampleBox->font()));

    loadModeBox = new ComboBox(this);
    loadModeBox->setGeometry(0, 0, 202, 22);
    loadModeBox->setFont(pointSize<8>(loadModeBox->font()));

    openSampleBTN = new PixmapButton(this);
    openSampleBTN->setCursor(QCursor(Qt::PointingHandCursor));
    openSampleBTN->setActiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    openSampleBTN->setInactiveGraphic(PLUGIN_NAME::getPixmap("fileload"));
    ToolTip::add(openSampleBTN, tr("Open sample"));

    scrollKnob = new Knob(this);
    scrollKnob->hide();

    effectScrollBar = new QScrollBar(Qt::Vertical, this);
    effectScrollBar->setSingleStep(1);
    effectScrollBar->setPageStep(100);
    effectScrollBar->setFixedHeight(197);
    effectScrollBar->setRange(0, 590);
    connect(effectScrollBar, SIGNAL(valueChanged(int)), this,
            SLOT(updateScroll()));

    effectScrollBar->setStyleSheet(
            "QScrollBar::handle:horizontal { background: #3f4750; border: none; border-radius: 1px; min-width: 24px; }\
					QScrollBar::handle:horizontal:hover { background: #a6adb1; }\
					QScrollBar::handle:horizontal:pressed { background: #a6adb1; }\
					QScrollBar::handle:vertical { background: #3f4750; border: none; border-radius: 1px; min-height: 24px; }\
					QScrollBar::handle:vertical:hover { background: #a6adb1; }\
					QScrollBar::handle:vertical:pressed { background: #a6adb1; }\
					QScrollBar::handle:horizontal:disabled, QScrollBar::handle:vertical:disabled  { background: #262b30; border-radius: 1px; border: none; }");

    matrixScrollBar = new QScrollBar(Qt::Vertical, this);
    matrixScrollBar->setSingleStep(1);
    matrixScrollBar->setPageStep(100);
    matrixScrollBar->setFixedHeight(197);
    matrixScrollBar->setRange(0, 6232);
    connect(matrixScrollBar, SIGNAL(valueChanged(int)), this,
            SLOT(updateScroll()));

    matrixScrollBar->setStyleSheet(
            "QScrollBar::handle:horizontal { background: #3f4750; border: none; border-radius: 1px; min-width: 24px; }\
					QScrollBar::handle:horizontal:hover { background: #a6adb1; }\
					QScrollBar::handle:horizontal:pressed { background: #a6adb1; }\
					QScrollBar::handle:vertical { background: #3f4750; border: none; border-radius: 1px; min-height: 24px; }\
					QScrollBar::handle:vertical:hover { background: #a6adb1; }\
					QScrollBar::handle:vertical:pressed { background: #a6adb1; }\
					QScrollBar::handle:horizontal:disabled, QScrollBar::handle:vertical:disabled  { background: #262b30; border-radius: 1px; border: none; }");

    connect(openWaveFormBTN, SIGNAL(clicked()), this,
            SLOT(openWaveFormFile()));
    // connect(confirmLoadBTN, SIGNAL(clicked()), this,
    //        SLOT(confirmWavetableLoadClicked()));
    connect(openSampleBTN, SIGNAL(clicked()), this, SLOT(openSampleFile()));

    connect(sinWaveBtn, SIGNAL(clicked()), this, SLOT(sinWaveClicked()));
    connect(triangleWaveBtn, SIGNAL(clicked()), this,
            SLOT(triangleWaveClicked()));
    connect(sawWaveBtn, SIGNAL(clicked()), this, SLOT(sawWaveClicked()));
    connect(sqrWaveBtn, SIGNAL(clicked()), this, SLOT(sqrWaveClicked()));
    connect(whiteNoiseWaveBtn, SIGNAL(clicked()), this,
            SLOT(noiseWaveClicked()));
    connect(usrWaveBtn, SIGNAL(clicked()), this, SLOT(usrWaveClicked()));
    connect(smoothBtn, SIGNAL(clicked()), this, SLOT(smoothClicked()));

    connect(sinWave2Btn, SIGNAL(clicked()), this, SLOT(sinWaveClicked()));
    connect(triangleWave2Btn, SIGNAL(clicked()), this,
            SLOT(triangleWaveClicked()));
    connect(sawWave2Btn, SIGNAL(clicked()), this, SLOT(sawWaveClicked()));
    connect(sqrWave2Btn, SIGNAL(clicked()), this, SLOT(sqrWaveClicked()));
    connect(whiteNoiseWave2Btn, SIGNAL(clicked()), this,
            SLOT(noiseWaveClicked()));
    connect(usrWave2Btn, SIGNAL(clicked()), this, SLOT(usrWaveClicked()));
    connect(smooth2Btn, SIGNAL(clicked()), this, SLOT(smoothClicked()));

    connect(XBtn, SIGNAL(clicked()), this, SLOT(XBtnClicked()));

    int32_t ii = 1;
    connect(tab1Btn, &PixmapButton::clicked, this,
            [this, ii]() { tabBtnClicked(ii); });
    ii = 2;
    connect(tab2Btn, &PixmapButton::clicked, this,
            [this, ii]() { tabBtnClicked(ii); });
    ii = 3;
    connect(tab3Btn, &PixmapButton::clicked, this,
            [this, ii]() { tabBtnClicked(ii); });
    ii = 4;
    connect(tab4Btn, &PixmapButton::clicked, this,
            [this, ii]() { tabBtnClicked(ii); });
    ii = 5;
    connect(tab5Btn, &PixmapButton::clicked, this,
            [this, ii]() { tabBtnClicked(ii); });
    // ii = 6;
    connect(tab6Btn, &PixmapButton::clicked, this,
            [this]() { tabBtnClicked(6); });

    connect(mainFlipBtn, SIGNAL(clicked()), this, SLOT(flipperClicked()));

    connect(subFlipBtn, SIGNAL(clicked()), this, SLOT(flipperClicked()));

    connect(visualizeToggle, SIGNAL(toggled(bool)), this,
            SLOT(visualizeToggled(bool)));

    Microwave* m = model();

    connect(&m->scroll, SIGNAL(dataChanged()), this, SLOT(updateScroll()));

    connect(scrollKnob, SIGNAL(sliderReleased()), this,
            SLOT(scrollReleased()));

    connect(&m->mainNum, SIGNAL(dataChanged()), this, SLOT(mainNumChanged()));

    connect(&m->subNum, SIGNAL(dataChanged()), this, SLOT(subNumChanged()));

    connect(&m->sampNum, SIGNAL(dataChanged()), this, SLOT(sampNumChanged()));

    connect(manualBtn, SIGNAL(clicked(bool)), this, SLOT(manualBtnClicked()));

    for(int i = 0; i < 64; ++i)
    {
        connect(
                m->modOutSec[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { modOutSecChanged(i); }, Qt::DirectConnection);
        connect(
                m->modIn[i], &ComboBoxModel::dataChanged, this,
                [this, i]() { modInChanged(i); }, Qt::DirectConnection);

        connect(modUpArrow[i], &PixmapButton::clicked, this,
                [this, i]() { modUpClicked(i); });
        connect(modDownArrow[i], &PixmapButton::clicked, this,
                [this, i]() { modDownClicked(i); });
    }

    updateScroll();
    updateBackground();

    modelChanged();
}

// Connects knobs/GUI elements to their models
void MicrowaveView::modelChanged()
{
    Microwave* m = model();

    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        morphKnob[i]->setModel(m->morph[i]);
        rangeKnob[i]->setModel(m->range[i]);
        sampLenKnob[i]->setModel(m->sampLen[i]);
        modifyKnob[i]->setModel(m->modify[i]);
        morphMaxKnob[i]->setModel(m->morphMax[i]);
        unisonVoicesKnob[i]->setModel(m->unisonVoices[i]);
        unisonDetuneKnob[i]->setModel(m->unisonDetune[i]);
        unisonMorphKnob[i]->setModel(m->unisonMorph[i]);
        unisonModifyKnob[i]->setModel(m->unisonModify[i]);
        detuneKnob[i]->setModel(m->detune[i]);
        modifyModeBox[i]->setModel(m->modifyMode[i]);
        phaseKnob[i]->setModel(m->phase[i]);
        phaseRandKnob[i]->setModel(m->phaseRand[i]);
        volKnob[i]->setModel(m->vol[i]);
        enabledToggle[i]->setModel(m->enabled[i]);
        mutedToggle[i]->setModel(m->muted[i]);
        panKnob[i]->setModel(m->pan[i]);
    }
    for(int i = 0; i < NB_FILTR; ++i)
    {
        filtInVolKnob[i]->setModel(m->filtInVol[i]);
        filtTypeBox[i]->setModel(m->filtType[i]);
        filtSlopeBox[i]->setModel(m->filtSlope[i]);
        filtCutoffKnob[i]->setModel(m->filtCutoff[i]);
        filtResoKnob[i]->setModel(m->filtReso[i]);
        filtGainKnob[i]->setModel(m->filtGain[i]);
        filtSatuKnob[i]->setModel(m->filtSatu[i]);
        filtWetDryKnob[i]->setModel(m->filtWetDry[i]);
        filtBalKnob[i]->setModel(m->filtBal[i]);
        filtOutVolKnob[i]->setModel(m->filtOutVol[i]);
        filtEnabledToggle[i]->setModel(m->filtEnabled[i]);
        filtFeedbackKnob[i]->setModel(m->filtFeedback[i]);
        filtDetuneKnob[i]->setModel(m->filtDetune[i]);
        filtKeytrackingToggle[i]->setModel(m->filtKeytracking[i]);
        filtMutedToggle[i]->setModel(m->filtMuted[i]);
    }
    for(int i = 0; i < NB_SMPLR; ++i)
    {
        sampleEnabledToggle[i]->setModel(m->sampleEnabled[i]);
        sampleGraphEnabledToggle[i]->setModel(m->sampleGraphEnabled[i]);
        sampleMutedToggle[i]->setModel(m->sampleMuted[i]);
        sampleKeytrackingToggle[i]->setModel(m->sampleKeytracking[i]);
        sampleLoopToggle[i]->setModel(m->sampleLoop[i]);
        sampleVolumeKnob[i]->setModel(m->sampleVolume[i]);
        samplePanningKnob[i]->setModel(m->samplePanning[i]);
        sampleDetuneKnob[i]->setModel(m->sampleDetune[i]);
        samplePhaseKnob[i]->setModel(m->samplePhase[i]);
        samplePhaseRandKnob[i]->setModel(m->samplePhaseRand[i]);
        sampleStartKnob[i]->setModel(m->sampleStart[i]);
        sampleEndKnob[i]->setModel(m->sampleEnd[i]);
    }
    for(int i = 0; i < NB_MACRO; ++i)
    {
        macroKnob[i]->setModel(m->macro[i]);
    }
    for(int i = 0; i < NB_SUBOSC; ++i)
    {
        subEnabledToggle[i]->setModel(m->subEnabled[i]);
        subVolKnob[i]->setModel(m->subVol[i]);
        subPhaseKnob[i]->setModel(m->subPhase[i]);
        subPhaseRandKnob[i]->setModel(m->subPhaseRand[i]);
        subDetuneKnob[i]->setModel(m->subDetune[i]);
        subMutedToggle[i]->setModel(m->subMuted[i]);
        subKeytrackToggle[i]->setModel(m->subKeytrack[i]);
        subSampLenKnob[i]->setModel(m->subSampLen[i]);
        subNoiseToggle[i]->setModel(m->subNoise[i]);
        subPanningKnob[i]->setModel(m->subPanning[i]);
        subTempoKnob[i]->setModel(m->subTempo[i]);
    }
    for(int i = 0; i < NB_MODLT; ++i)
    {
        modOutSecBox[i]->setModel(m->modOutSec[i]);
        modOutSigBox[i]->setModel(m->modOutSig[i]);
        modOutSecNumBox[i]->setModel(m->modOutSecNum[i]);
        modInBox[i]->setModel(m->modIn[i]);
        modInNumBox[i]->setModel(m->modInNum[i]);
        modInOtherNumBox[i]->setModel(m->modInOtherNum[i]);
        modInAmntKnob[i]->setModel(m->modInAmnt[i]);
        modInCurveKnob[i]->setModel(m->modInCurve[i]);
        modInBox2[i]->setModel(m->modIn2[i]);
        modInNumBox2[i]->setModel(m->modInNum2[i]);
        modInOtherNumBox2[i]->setModel(m->modInOtherNum2[i]);
        modInAmntKnob2[i]->setModel(m->modInAmnt2[i]);
        modInCurveKnob2[i]->setModel(m->modInCurve2[i]);
        modEnabledToggle[i]->setModel(m->modEnabled[i]);
        modCombineTypeBox[i]->setModel(m->modCombineType[i]);
        modTypeToggle[i]->setModel(m->modType[i]);
    }

    graph->setModel(&m->graph);
    visvolKnob->setModel(&m->visvol);
    visualizeToggle->setModel(&m->visualize);
    subNumBox->setModel(&m->subNum);
    sampNumBox->setModel(&m->sampNum);
    loadChnlKnob->setModel(&m->loadChnl);
    scrollKnob->setModel(&m->scroll);
    mainNumBox->setModel(&m->mainNum);
    oversampleBox->setModel(&m->oversample);
    loadModeBox->setModel(&m->loadMode);
    wtLoad1Knob->setModel(&m->wtLoad1);
    wtLoad2Knob->setModel(&m->wtLoad2);
    wtLoad3Knob->setModel(&m->wtLoad3);
    wtLoad4Knob->setModel(&m->wtLoad4);
    mainFlipBtn->setModel(&m->mainFlipped);
    subFlipBtn->setModel(&m->subFlipped);
}

// Puts all of the GUI elements in their correct locations, depending on
// the scroll knob value.
void MicrowaveView::updateScroll()
{
    Microwave* m = model();

    int scrollVal       = (m->scroll.value() - 1) * 250.;
    int modScrollVal    = (matrixScrollBar->value()) / 100. * 115.;
    int effectScrollVal = (effectScrollBar->value()) / 100. * 92.;
    int mainFlipped     = m->mainFlipped.value();
    int subFlipped      = m->subFlipped.value();

    int mainIsFlipped    = mainFlipped * 500.;
    int mainIsNotFlipped = !mainFlipped * 500.;
    int subIsFlipped     = subFlipped * 500.;
    int subIsNotFlipped  = !subFlipped * 500.;

    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        morphKnob[i]->move(23 - scrollVal, 172 + mainIsFlipped);
        rangeKnob[i]->move(55 - scrollVal, 172 + mainIsFlipped);
        sampLenKnob[i]->move(23 - scrollVal, 172 + mainIsNotFlipped);
        morphMaxKnob[i]->move(55 - scrollVal, 172 + mainIsNotFlipped);
        modifyKnob[i]->move(87 - scrollVal, 172 + mainIsFlipped);
        modifyModeBox[i]->move(127 - scrollVal, 186 + mainIsFlipped);
        unisonVoicesKnob[i]->move(184 - scrollVal, 172 + mainIsFlipped);
        unisonDetuneKnob[i]->move(209 - scrollVal, 172 + mainIsFlipped);
        unisonMorphKnob[i]->move(184 - scrollVal, 208 + mainIsFlipped);
        unisonModifyKnob[i]->move(209 - scrollVal, 208 + mainIsFlipped);
        detuneKnob[i]->move(152 - scrollVal, 208 + mainIsFlipped);  // 216
        phaseKnob[i]->move(86 - scrollVal, 216 + mainIsNotFlipped);
        phaseRandKnob[i]->move(113 - scrollVal, 216 + mainIsNotFlipped);
        volKnob[i]->move(87 - scrollVal, 172 + mainIsNotFlipped);
        enabledToggle[i]->move(96 - scrollVal, 222 + mainIsFlipped);
        mutedToggle[i]->move(130 - scrollVal, 222 + mainIsFlipped);
        panKnob[i]->move(55 - scrollVal, 130 + mainIsNotFlipped);
    }
    for(int i = 0; i < NB_FILTR; ++i)
    {
        filtInVolKnob[i]->move(30 + 1000 - scrollVal,
                               i * 92 + 91 - effectScrollVal);
        filtTypeBox[i]->move(128 + 1000 - scrollVal,
                             i * 92 + 63 - effectScrollVal);
        filtSlopeBox[i]->move(171 + 1000 - scrollVal,
                              i * 92 + 63 - effectScrollVal);
        filtCutoffKnob[i]->move(32 + 1000 - scrollVal,
                                i * 92 + 55 - effectScrollVal);
        filtResoKnob[i]->move(63 + 1000 - scrollVal,
                              i * 92 + 55 - effectScrollVal);
        filtGainKnob[i]->move(94 + 1000 - scrollVal,
                              i * 92 + 55 - effectScrollVal);
        filtSatuKnob[i]->move(135 + 1000 - scrollVal,
                              i * 92 + 91 - effectScrollVal);
        filtWetDryKnob[i]->move(80 + 1000 - scrollVal,
                                i * 92 + 91 - effectScrollVal);
        filtBalKnob[i]->move(105 + 1000 - scrollVal,
                             i * 92 + 91 - effectScrollVal);
        filtOutVolKnob[i]->move(55 + 1000 - scrollVal,
                                i * 92 + 91 - effectScrollVal);
        filtEnabledToggle[i]->move(28 + 1000 - scrollVal,
                                   i * 92 + 37 - effectScrollVal);
        filtFeedbackKnob[i]->move(167 + 1000 - scrollVal,
                                  i * 92 + 91 - effectScrollVal);
        filtDetuneKnob[i]->move(192 + 1000 - scrollVal,
                                i * 92 + 91 - effectScrollVal);
        filtKeytrackingToggle[i]->move(200 + 1000 - scrollVal,
                                       i * 92 + 37 - effectScrollVal);
        filtMutedToggle[i]->move(170 + 1000 - scrollVal,
                                 i * 92 + 37 - effectScrollVal);
    }
    for(int i = 0; i < NB_SMPLR; ++i)
    {
        sampleEnabledToggle[i]->move(87 + 500 - scrollVal, 231);
        sampleMutedToggle[i]->move(104 + 500 - scrollVal, 231);
        sampleKeytrackingToggle[i]->move(122 + 500 - scrollVal, 231);
        sampleGraphEnabledToggle[i]->move(139 + 500 - scrollVal, 231);
        sampleLoopToggle[i]->move(156 + 500 - scrollVal, 231);

        sampleVolumeKnob[i]->move(23 + 500 - scrollVal, 172);
        samplePanningKnob[i]->move(55 + 500 - scrollVal, 172);
        sampleDetuneKnob[i]->move(93 + 500 - scrollVal, 172);
        samplePhaseKnob[i]->move(180 + 500 - scrollVal, 172);
        samplePhaseRandKnob[i]->move(206 + 500 - scrollVal, 172);
        sampleStartKnob[i]->move(121 + 500 - scrollVal, 172);
        sampleEndKnob[i]->move(145 + 500 - scrollVal, 172);
    }
    for(int i = 0; i < NB_SUBOSC; ++i)
    {
        subEnabledToggle[i]->move(109 + 250 - scrollVal, 216 + subIsFlipped);
        subVolKnob[i]->move(23 + 250 - scrollVal, 172 + subIsFlipped);
        subPhaseKnob[i]->move(180 + 250 - scrollVal, 172 + subIsFlipped);
        subPhaseRandKnob[i]->move(206 + 250 - scrollVal, 172 + subIsFlipped);
        subDetuneKnob[i]->move(95 + 250 - scrollVal, 172 + subIsFlipped);
        subMutedToggle[i]->move(148 + 250 - scrollVal, 216 + subIsFlipped);
        subKeytrackToggle[i]->move(109 + 250 - scrollVal, 231 + subIsFlipped);
        subSampLenKnob[i]->move(130 + 250 - scrollVal, 172 + subIsFlipped);
        subNoiseToggle[i]->move(148 + 250 - scrollVal, 231 + subIsFlipped);
        subPanningKnob[i]->move(55 + 250 - scrollVal, 172 + subIsFlipped);
        subTempoKnob[i]->move(35 + 250 - scrollVal, 172 + subIsNotFlipped);
    }
    for(int i = 0; i < NB_MODLT; ++i)
    {
        modInBox[i]->move(43 + 750 - scrollVal, i * 115 + 57 - modScrollVal);
        modInNumBox[i]->move(88 + 750 - scrollVal,
                             i * 115 + 57 - modScrollVal);
        modInOtherNumBox[i]->move(122 + 750 - scrollVal,
                                  i * 115 + 57 - modScrollVal);
        modInAmntKnob[i]->move(167 + 750 - scrollVal,
                               i * 115 + 53 - modScrollVal);
        modInCurveKnob[i]->move(192 + 750 - scrollVal,
                                i * 115 + 53 - modScrollVal);
        modOutSecBox[i]->move(27 + 750 - scrollVal,
                              i * 115 + 88 - modScrollVal);
        modOutSigBox[i]->move(69 + 750 - scrollVal,
                              i * 115 + 88 - modScrollVal);
        modOutSecNumBox[i]->move(112 + 750 - scrollVal,
                                 i * 115 + 88 - modScrollVal);
        modInBox2[i]->move(43 + 750 - scrollVal,
                           i * 115 + 118 - modScrollVal);
        modInNumBox2[i]->move(88 + 750 - scrollVal,
                              i * 115 + 118 - modScrollVal);
        modInOtherNumBox2[i]->move(122 + 750 - scrollVal,
                                   i * 115 + 118 - modScrollVal);
        modInAmntKnob2[i]->move(167 + 750 - scrollVal,
                                i * 115 + 114 - modScrollVal);
        modInCurveKnob2[i]->move(192 + 750 - scrollVal,
                                 i * 115 + 114 - modScrollVal);
        modEnabledToggle[i]->move(28 + 750 - scrollVal,
                                  i * 115 + 38 - modScrollVal);
        modCombineTypeBox[i]->move(149 + 750 - scrollVal,
                                   i * 115 + 88 - modScrollVal);
        modTypeToggle[i]->move(196 + 750 - scrollVal,
                               i * 115 + 100 - modScrollVal);

        modUpArrow[i]->move(181 + 750 - scrollVal,
                            i * 115 + 37 - modScrollVal);
        modDownArrow[i]->move(199 + 750 - scrollVal,
                              i * 115 + 37 - modScrollVal);
    }

    visvolKnob->move(222 - scrollVal, 2);  // 230 - scrollVal, 24);
    scrollKnob->move(0, 220);

    loadChnlKnob->move(1500 + 50 - scrollVal, 160);
    visualizeToggle->move(228 - scrollVal, 36);  // 213 - scrollVal, 26);

    subNumBox->move(250 + 18 - scrollVal, 219);
    sampNumBox->move(500 + 18 - scrollVal, 219);
    mainNumBox->move(18 - scrollVal, 219);
    graph->move(scrollVal >= 500 ? 500 + 23 - scrollVal : 23, 30);
    tabChanged(m->scroll.value() - 1);
    openWaveFormBTN->move(54 - scrollVal, 220);
    openSampleBTN->move(54 + 500 - scrollVal, 220);

    sinWaveBtn->move(179 + 250 - scrollVal, 212 + subIsFlipped);
    triangleWaveBtn->move(197 + 250 - scrollVal, 212 + subIsFlipped);
    sawWaveBtn->move(215 + 250 - scrollVal, 212 + subIsFlipped);
    sqrWaveBtn->move(179 + 250 - scrollVal, 227 + subIsFlipped);
    whiteNoiseWaveBtn->move(197 + 250 - scrollVal, 227 + subIsFlipped);
    smoothBtn->move(215 + 250 - scrollVal, 227 + subIsFlipped);
    usrWaveBtn->move(54 + 250 - scrollVal, 220);

    sinWave2Btn->move(179 + 500 - scrollVal, 212);
    triangleWave2Btn->move(197 + 500 - scrollVal, 212);
    sawWave2Btn->move(215 + 500 - scrollVal, 212);
    sqrWave2Btn->move(179 + 500 - scrollVal, 227);
    whiteNoiseWave2Btn->move(197 + 500 - scrollVal, 227);
    smooth2Btn->move(215 + 500 - scrollVal, 227);
    usrWave2Btn->move(54 + 500 - scrollVal, 220);

    oversampleBox->move(30 + 1250 - scrollVal, 30);

    effectScrollBar->move(221 + 1000 - scrollVal, 32);
    matrixScrollBar->move(221 + 750 - scrollVal, 32);

    filtForegroundLabel->move(1000 - scrollVal, 0);
    filtBoxesLabel->move(1000 + 24 - scrollVal, 35 - (effectScrollVal % 92));

    matrixForegroundLabel->move(750 - scrollVal, 0);
    matrixBoxesLabel->move(750 + 24 - scrollVal, 35 - (modScrollVal % 115));

    /*
    macroKnob[0]->move(1250 + 30 - scrollVal, 100);
    macroKnob[1]->move(1250 + 60 - scrollVal, 100);
    macroKnob[2]->move(1250 + 90 - scrollVal, 100);
    macroKnob[3]->move(1250 + 120 - scrollVal, 100);
    macroKnob[4]->move(1250 + 30 - scrollVal, 130);
    macroKnob[5]->move(1250 + 60 - scrollVal, 130);
    macroKnob[6]->move(1250 + 90 - scrollVal, 130);
    macroKnob[7]->move(1250 + 120 - scrollVal, 130);
    */
    for(int i = 0; i < NB_MACRO; ++i)
        macroKnob[0]->move(1280 + 30 * (i % 4) - scrollVal,
                           100 + 30 * (i / 4));

    tab1Btn->move(1, 48);
    tab2Btn->move(1, 63);
    tab3Btn->move(1, 78);
    tab4Btn->move(1, 93);
    tab5Btn->move(1, 108);
    tab6Btn->move(1, 123);

    mainFlipBtn->move(3 - scrollVal, 145);
    subFlipBtn->move(250 + 3 - scrollVal, 145);

    loadModeBox->move(1500 + 0 - scrollVal, 0);

    manualBtn->move(1250 + 50 - scrollVal, 200);

    confirmLoadBTN->move(1500 + 200 - scrollVal, 50);
    XBtn->move(230 + 1500 - scrollVal, 10 + subIsFlipped);

    wtLoad1Knob->move(1500 + 30 - scrollVal, 48);
    wtLoad2Knob->move(1500 + 60 - scrollVal, 48);
    wtLoad3Knob->move(1500 + 90 - scrollVal, 48);
    wtLoad4Knob->move(1500 + 120 - scrollVal, 48);

    /*
    QRect rect(scrollVal, 0, 250, 250);
    pal.setBrush( backgroundRole(), fullArtworkImg.copy(rect) );
    setPalette( pal );
    */
}

// Snaps scroll to nearest tab when the scroll knob is released.
void MicrowaveView::scrollReleased()
{
    Microwave*   m         = model();
    const real_t scrollVal = m->scroll.value();
    m->scroll.setValue(round(scrollVal - 0.25));
}

void MicrowaveView::mouseMoveEvent(QMouseEvent* _me)
{
    // model()->morph[0]->setValue(_me->x());
}

void MicrowaveView::wheelEvent(QWheelEvent* _me)
{
    Microwave* m = model();

    if(_me->x() <= 18 && _me->y() >= 48
       && _me->y() <= 138)  // If scroll over tab buttons
    {
        if(m->scroll.value() != 7)
        {
            if(_me->delta() < 0 && m->scroll.value() != 6)
                m->scroll.setValue(m->scroll.value() + 1);
            else if(_me->delta() > 0)
                m->scroll.setValue(m->scroll.value() - 1);
        }
    }
}

// Trades out the GUI elements when switching between oscillators
void MicrowaveView::mainNumChanged()
{
    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        morphKnob[i]->hide();
        rangeKnob[i]->hide();
        sampLenKnob[i]->hide();
        morphMaxKnob[i]->hide();
        modifyKnob[i]->hide();
        unisonVoicesKnob[i]->hide();
        unisonDetuneKnob[i]->hide();
        unisonMorphKnob[i]->hide();
        unisonModifyKnob[i]->hide();
        detuneKnob[i]->hide();
        phaseKnob[i]->hide();
        phaseRandKnob[i]->hide();
        volKnob[i]->hide();
        enabledToggle[i]->hide();
        mutedToggle[i]->hide();
        modifyModeBox[i]->hide();
        panKnob[i]->hide();
        if(model()->mainNum.value() - 1 == i)
        {
            morphKnob[i]->show();
            rangeKnob[i]->show();
            sampLenKnob[i]->show();
            morphMaxKnob[i]->show();
            modifyKnob[i]->show();
            unisonVoicesKnob[i]->show();
            unisonDetuneKnob[i]->show();
            unisonMorphKnob[i]->show();
            unisonModifyKnob[i]->show();
            detuneKnob[i]->show();
            phaseKnob[i]->show();
            phaseRandKnob[i]->show();
            volKnob[i]->show();
            enabledToggle[i]->show();
            mutedToggle[i]->show();
            modifyModeBox[i]->show();
            panKnob[i]->show();
        }
    }
}

// Trades out the GUI elements when switching between oscillators, and
// adjusts graph length when needed
void MicrowaveView::subNumChanged()
{
    Microwave* m = model();

    m->graph.setLength(m->subSampLen[m->subNum.value() - 1]->value());
    for(int i = 0; i < WAVE_SIZE; ++i)
    {
        m->graph.setSampleAt(
                i, m->subs[(m->subNum.value() - 1) * WAVE_SIZE + i]);
    }
    for(int i = 0; i < NB_SUBOSC; ++i)
    {
        if(i != m->subNum.value() - 1)
        {
            subEnabledToggle[i]->hide();
            subVolKnob[i]->hide();
            subPhaseKnob[i]->hide();
            subPhaseRandKnob[i]->hide();
            subDetuneKnob[i]->hide();
            subMutedToggle[i]->hide();
            subKeytrackToggle[i]->hide();
            subSampLenKnob[i]->hide();
            subNoiseToggle[i]->hide();
            subPanningKnob[i]->hide();
            subTempoKnob[i]->hide();
        }
        else
        {
            subEnabledToggle[i]->show();
            subVolKnob[i]->show();
            subPhaseKnob[i]->show();
            subPhaseRandKnob[i]->show();
            subDetuneKnob[i]->show();
            subMutedToggle[i]->show();
            subKeytrackToggle[i]->show();
            subSampLenKnob[i]->show();
            subNoiseToggle[i]->show();
            subPanningKnob[i]->show();
            subTempoKnob[i]->show();
        }
    }
}

// Trades out the GUI elements when switching between oscillators
void MicrowaveView::sampNumChanged()
{
    Microwave* m = model();

    for(int i = 0; i < GRAPHONE_SIZE; ++i)
    {
        m->graph.setSampleAt(
                i,
                m->sampGraphs[(m->sampNum.value() - 1) * GRAPHONE_SIZE + i]);
    }
    for(int i = 0; i < NB_SMPLR; ++i)
    {
        if(i != m->sampNum.value() - 1)
        {
            sampleEnabledToggle[i]->hide();
            sampleGraphEnabledToggle[i]->hide();
            sampleMutedToggle[i]->hide();
            sampleKeytrackingToggle[i]->hide();
            sampleLoopToggle[i]->hide();
            sampleVolumeKnob[i]->hide();
            samplePanningKnob[i]->hide();
            sampleDetuneKnob[i]->hide();
            samplePhaseKnob[i]->hide();
            samplePhaseRandKnob[i]->hide();
            sampleStartKnob[i]->hide();
            sampleEndKnob[i]->hide();
        }
        else
        {
            sampleEnabledToggle[i]->show();
            sampleGraphEnabledToggle[i]->show();
            sampleMutedToggle[i]->show();
            sampleKeytrackingToggle[i]->show();
            sampleLoopToggle[i]->show();
            sampleVolumeKnob[i]->show();
            samplePanningKnob[i]->show();
            sampleDetuneKnob[i]->show();
            samplePhaseKnob[i]->show();
            samplePhaseRandKnob[i]->show();
            sampleStartKnob[i]->show();
            sampleEndKnob[i]->show();
        }
    }
}

// Moves/changes the GUI around depending on the mod out section value
void MicrowaveView::modOutSecChanged(int32_t i)
{
    Microwave* m = model();

    switch(m->modOutSec[i]->value())
    {
        case 0:  // None
        {
            modOutSigBox[i]->hide();
            modOutSecNumBox[i]->hide();
            break;
        }
        case 1:  // Main OSC
        {
            modOutSigBox[i]->show();
            modOutSecNumBox[i]->show();
            m->modOutSig[i]->clear();
            mainoscsignalsmodel(m->modOutSig[i]);
            m->modOutSecNum[i]->setRange(1., NB_MAINOSC, 1.);
            break;
        }
        case 2:  // Sub OSC
        {
            modOutSigBox[i]->show();
            modOutSecNumBox[i]->show();
            m->modOutSig[i]->clear();
            subsignalsmodel(m->modOutSig[i]);
            m->modOutSecNum[i]->setRange(1., NB_SUBOSC, 1.);
            break;
        }
        case 3:  // Filter Input
        {
            modOutSigBox[i]->show();
            modOutSecNumBox[i]->hide();
            m->modOutSig[i]->clear();
            mod8model(m->modOutSig[i]);
            m->modOutSecNum[i]->setRange(1., NB_MODLT, 1.);  // was commented
            break;
        }
        case 4:  // Filter Parameters
        {
            modOutSigBox[i]->show();
            modOutSecNumBox[i]->hide();
            m->modOutSig[i]->clear();
            filtersignalsmodel(m->modOutSig[i]);
            m->modOutSecNum[i]->setRange(1., NB_FILTR, 1.);  // was commented
            break;
        }
        case 5:  // Matrix Parameters
        {
            modOutSigBox[i]->show();
            modOutSecNumBox[i]->show();
            m->modOutSig[i]->clear();
            matrixsignalsmodel(m->modOutSig[i]);
            m->modOutSecNum[i]->setRange(1., NB_MODLT, 1.);
            break;
        }
        case 6:  // Sampler
        {
            modOutSigBox[i]->show();
            modOutSecNumBox[i]->show();
            m->modOutSig[i]->clear();
            samplesignalsmodel(m->modOutSig[i]);
            m->modOutSecNum[i]->setRange(1., NB_SMPLR, 1.);
            break;
        }
        default:
        {
            break;
        }
    }
}

// Moves/changes the GUI around depending on the Mod In Section value
void MicrowaveView::modInChanged(int32_t i)
{
    Microwave* m = model();

    switch(m->modIn[i]->value())
    {
        case 0:
        {
            modInNumBox[i]->hide();
            modInOtherNumBox[i]->hide();
            break;
        }
        case 1:  // Main OSC
        {
            modInNumBox[i]->show();
            modInOtherNumBox[i]->hide();
            m->modInNum[i]->setRange(1, NB_MAINOSC, 1);
            break;
        }
        case 2:  // Sub OSC
        {
            modInNumBox[i]->show();
            modInOtherNumBox[i]->hide();
            m->modInNum[i]->setRange(1, NB_SUBOSC, 1);
            break;
        }
        case 3:  // Filter
        {
            modInNumBox[i]->show();
            modInOtherNumBox[i]->hide();
            m->modInNum[i]->setRange(1, NB_FILTR, 1);
            // m->modInOtherNum[i]->setRange( 1, 8, 1 );
            break;
        }
        case 4:  // Samplers
        {
            modInNumBox[i]->show();
            modInOtherNumBox[i]->hide();
            m->modInNum[i]->setRange(1, NB_SMPLR, 1);
            break;
        }
    }
}

// Does what is necessary when the user visits a new tab
void MicrowaveView::tabChanged(int tabnum)
{
    Microwave* m = model();

    if(m->currentTab != tabnum)
    {
        m->currentTab = tabnum;

        updateBackground();

        switch(tabnum)
        {
            case 0:  // Wavetable
            {
                m->graph.setLength(WAVE_SIZE);
                mainNumChanged();
                break;
            }
            case 1:  // Sub
            {
                subNumChanged();  // Graph length is set in here
                break;
            }
            case 2:  // Sample
            {
                m->graph.setLength(GRAPHONE_SIZE);
                sampNumChanged();
                break;
            }
        }

        if(tabnum != 0)
        {
            tab1Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab1"));
            tab1Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab1"));
        }
        else
        {
            tab1Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab1_active"));
            tab1Btn->setInactiveGraphic(
                    PLUGIN_NAME::getPixmap("tab1_active"));
        }

        if(tabnum != 1)
        {
            tab2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab2"));
            tab2Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab2"));
        }
        else
        {
            tab2Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab2_active"));
            tab2Btn->setInactiveGraphic(
                    PLUGIN_NAME::getPixmap("tab2_active"));
        }

        if(tabnum != 2)
        {
            tab3Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab3"));
            tab3Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab3"));
        }
        else
        {
            tab3Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab3_active"));
            tab3Btn->setInactiveGraphic(
                    PLUGIN_NAME::getPixmap("tab3_active"));
        }

        if(tabnum != 3)
        {
            tab4Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab4"));
            tab4Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab4"));
        }
        else
        {
            tab4Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab4_active"));
            tab4Btn->setInactiveGraphic(
                    PLUGIN_NAME::getPixmap("tab4_active"));
        }

        if(tabnum != 4)
        {
            tab5Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab5"));
            tab5Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab5"));
        }
        else
        {
            tab5Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab5_active"));
            tab5Btn->setInactiveGraphic(
                    PLUGIN_NAME::getPixmap("tab5_active"));
        }

        if(tabnum != 5)
        {
            tab6Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab6"));
            tab6Btn->setInactiveGraphic(PLUGIN_NAME::getPixmap("tab6"));
        }
        else
        {
            tab6Btn->setActiveGraphic(PLUGIN_NAME::getPixmap("tab6_active"));
            tab6Btn->setInactiveGraphic(
                    PLUGIN_NAME::getPixmap("tab6_active"));
        }
    }
}

void MicrowaveView::updateBackground()
{
    Microwave* m = model();

    int  backgroundnum = m->currentTab;
    bool mainFlipped   = m->mainFlipped.value();
    bool subFlipped    = m->subFlipped.value();

    switch(backgroundnum)
    {
        case 0:  // Wavetable
        {
            m->graph.setLength(2048);
            mainNumChanged();

            if(!mainFlipped)
            {
                pal.setBrush(backgroundRole(), tab1ArtworkImg.copy());
            }
            else
            {
                pal.setBrush(backgroundRole(), tab1FlippedArtworkImg.copy());
            }

            setPalette(pal);
            break;
        }
        case 1:  // Sub
        {
            subNumChanged();  // Graph length is set in here

            if(!subFlipped)
            {
                pal.setBrush(backgroundRole(), tab2ArtworkImg.copy());
            }
            else
            {
                pal.setBrush(backgroundRole(), tab2FlippedArtworkImg.copy());
            }

            setPalette(pal);
            break;
        }
        case 2:  // Sample
        {
            m->graph.setLength(128);
            sampNumChanged();

            pal.setBrush(backgroundRole(), tab3ArtworkImg.copy());
            setPalette(pal);
            break;
        }
        case 3:  // Matrix
        {
            pal.setBrush(backgroundRole(), tab4ArtworkImg.copy());
            setPalette(pal);
            break;
        }
        case 4:  // Effect
        {
            pal.setBrush(backgroundRole(), tab5ArtworkImg.copy());
            setPalette(pal);
            break;
        }
        case 5:  // Miscellaneous
        {
            pal.setBrush(backgroundRole(), tab6ArtworkImg.copy());
            setPalette(pal);
            break;
        }
        case 6:  // Wavetable Loading
        {
            pal.setBrush(backgroundRole(), tab7ArtworkImg.copy());
            setPalette(pal);
            break;
        }
    }
}

// This doesn't do anything right now.  I should probably delete it until
// I need it.
void MicrowaveView::visualizeToggled(bool value)
{
}

// Buttons that change the graph
void MicrowaveView::sinWaveClicked()
{
    graph->model()->setWaveToSine();
    Engine::getSong()->setModified();
}

void MicrowaveView::triangleWaveClicked()
{
    graph->model()->setWaveToTriangle();
    Engine::getSong()->setModified();
}

void MicrowaveView::sawWaveClicked()
{
    graph->model()->setWaveToSaw();
    Engine::getSong()->setModified();
}

void MicrowaveView::sqrWaveClicked()
{
    graph->model()->setWaveToSquare();
    Engine::getSong()->setModified();
}

void MicrowaveView::noiseWaveClicked()
{
    graph->model()->setWaveToNoise();
    Engine::getSong()->setModified();
}

void MicrowaveView::usrWaveClicked()
{
    QString fileName = graph->model()->setWaveToUser();
    ToolTip::add(usrWaveBtn, fileName);
    Engine::getSong()->setModified();
}

void MicrowaveView::smoothClicked()
{
    graph->model()->smooth();
    Engine::getSong()->setModified();
}
// Buttons that change the graph

void MicrowaveView::flipperClicked()
{
    updateBackground();
    updateScroll();
}

void MicrowaveView::XBtnClicked()
{
    model()->scroll.setValue(0);
}

void MicrowaveView::modUpClicked(int32_t i)
{
    if(i > 0)
    {
        model()->switchMatrixSections(i, i - 1);
    }
}

void MicrowaveView::modDownClicked(int32_t i)
{
    if(i < 63)
    {
        model()->switchMatrixSections(i, i + 1);
    }
}

void MicrowaveView::tabBtnClicked(int32_t i)
{
    model()->scroll.setValue(i);
}

// Calls MicrowaveView::openWavetableFile when the wavetable opening
// button is clicked.
/*
void MicrowaveView::openWavetableFileBtnClicked()
{
    model()->scroll.setValue(7);
    chooseWavetableFile();
}
*/

/*
void MicrowaveView::chooseWavetableFile()
{
    SampleBuffer* sampleBuffer = new SampleBuffer();
    wavetableFileName          = sampleBuffer->openAndSetWaveformFile();
    // sharedObject::unref(sampleBuffer);
    delete sampleBuffer;
}
*/

/*
void MicrowaveView::confirmWavetableLoadClicked()
{
   openWavetableFile();
}
*/

// All of the code and algorithms for loading wavetables from samples.
// Please don't expect this code to look neat.
void MicrowaveView::openWaveFormFile()
{
    SampleBuffer* sampleBuffer = new SampleBuffer();
    QString       fileName     = sampleBuffer->openAndSetWaveFormFile();
    if(!fileName.isEmpty() == false)
        loadWaveForm(sampleBuffer);
    delete sampleBuffer;
}

void MicrowaveView::loadWaveForm(SampleBuffer* sampleBuffer)
{
    // const sample_rate_t sr = Engine::mixer()->processingSampleRate();
    const f_cnt_t frames    = sampleBuffer->frames();
    Microwave*    mwm       = model();
    const int     oscilNum  = mwm->mainNum.value() - 1;
    const int     algorithm = mwm->loadMode.value();
    const int     channel   = mwm->loadChnl.value();

    sampleBuffer->dataReadLock();

    // const real_t lengthOfSample = real_t(sr) / 1000. * frames;  // in
    // samples

    // for(real_t i = 0; i < lengthOfSample; ++i)
    switch(algorithm)
    {
        case 0:  // Load sample without changes
        {
            for(int i = 0; i < qMin(frames, MAINWAV_SIZE); ++i)
            {
                mwm->waveforms[oscilNum][i] = sampleBuffer->userWaveSample(
                        real_t(i) / frames, channel);
            }
            for(int i = qMin(frames, MAINWAV_SIZE); i < MAINWAV_SIZE; ++i)
            {
                mwm->waveforms[oscilNum][i] = 0.;
            }

            mwm->morphMax[oscilNum]->setValue(
                    real_t(qMin(frames, MAINWAV_SIZE))
                    / real_t(MAINWAV_SIZE));
            // i / mwm->sampLen[oscilNum]->value());
            mwm->morphMaxChanged();
            break;
        }

        case 1:  // For loading wavetable files
        default:
        {
            for(int i = 0; i < MAINWAV_SIZE; ++i)
                mwm->waveforms[oscilNum][i] = sampleBuffer->userWaveSample(
                        real_t(i) / real_t(MAINWAV_SIZE), channel);
            mwm->morphMax[oscilNum]->setValue(1.);
            mwm->morphMaxChanged();
            break;
        }
            /*
            case 2:  // Autocorrelation
            {
                // This uses a method called autocorrelation to detect the
                // pitch.  It can get a few Hz off (especially at higher
                // frequencies), so I also compare it with the zero
                // crossings to see if I can get it even more accurate.

                // Estimate pitch using autocorrelation:

                real_t checkLength = qMin<real_t>(
                        4000.,
                        lengthOfSample);  // 4000 samples should be long
                                          // enough to be able to
                                          // accurately detect most
                                          // frequencies this way

                real_t threshold = -1;
                real_t combined  = 0;
                real_t oldcombined;
                int    stage = 0;

                real_t period = 0;
                for(int i = 0; i < checkLength; ++i)
                {
                    oldcombined = combined;
                    combined    = 0;
                    for(int k = 0; k < checkLength - i; ++k)
                    {
                        combined += (sampleBuffer->userWaveSample(
                                             k / lengthOfSample, channel)
                                             * sampleBuffer->userWaveSample(
                                                     (k + i) /
            lengthOfSample, channel)
                                     + 1) * 0.5
                                    - 0.5;
                    }

                    if(stage == 2 && combined - oldcombined <= 0)
                    {
                        stage  = 3;
                        period = i;
                    }

                    if(stage == 1 && combined > threshold
                       && combined - oldcombined > 0)
                    {
                        stage = 2;
                    }

                    if(!i)
                    {
                        threshold = combined * 0.5;
                        stage     = 1;
                    }
                }

                if(!period)
                {
                    break;
                }

                cout << sample_rate / period << std::flush;

                // Now see if the zero crossings can aid in getting the
                // pitch even more accurate:

                // Note:  If the zero crossings give a result very close
                // to the autocorrelation, then it is likely to be more
                // accurate. Otherwise, the zero crossing result is
                // probably very inaccurate (common with complex sounds)
                // and is ignored.

                std::vector<real_t> crossings;
                crossings.push_back(0);
                std::vector<real_t> crossingsDif;
                bool above = (sampleBuffer->userWaveSample(1 /
            lengthOfSample, channel) > 0);

                for(int i = 0; i < checkLength; ++i)
                {
                    if((sampleBuffer->userWaveSample(i / lengthOfSample,
            channel) > 0)
                       != above)
                    {
                        above = !above;
                        if(above)
                        {
                            crossingsDif.push_back(
                                    i - crossings[crossings.size() - 1]);
                            crossings.push_back(i);
                        }
                    }
                }

                crossings.erase(crossings.begin());

                if(crossingsDif.size() >= 3)
                {
                    real_t crossingsMean =
            std::accumulate(crossingsDif.begin(), crossingsDif.end(), 0.)
            / crossingsDif.size(); std::vector<real_t> crossingsToRemove;
                    for(int i = 0; i < crossingsDif.size(); ++i)
                    {
                        if(crossingsDif[i] < crossingsMean)
                        {
                            crossingsToRemove.push_back(i);
                        }
                    }
                    for(int i = crossingsToRemove.size() - 1; i >= 0; --i)
                    {
                        crossingsDif.erase(crossingsDif.begin()
                                           + crossingsToRemove[i]);
                    }
                    if(crossingsDif.size() >= 2)
                    {
                        real_t crossingsMedian
                                = crossingsDif[int(crossingsDif.size()
            / 2.f)]; cout << crossingsMedian << std::flush; if(abs(period
            - crossingsMedian) < 5.f + period / 100.)
                        {
                            period = crossingsMedian;
                        }
                    }
                }

                for(int i = 0; i < MAINWAV_SIZE; ++i)
                {
                    mwm->waveforms[oscilNum][i] =
            sampleBuffer->userWaveSample(
                            ((i / 2048.f) * period) / lengthOfSample,
            channel);
                }

                break;
            }
            */
            /*
                            case 2:// Zero-crossing method
                                        {
                                                const int gap =
                   mwm->wtLoad1.value(); std::vector<real_t> crossings;
                   int loc = 0;

                                                while(
                   sampleBuffer->userWaveSample( loc / lengthOfSample,
                   channel ) == 0 )// Skips any silence at the beginning
                                                {
                                                        ++loc;
                                                }

                                                crossings.push_back( loc
                   == 0 ? loc : loc - 1 );

                                                bool above = (
                   sampleBuffer->userWaveSample( loc, channel ) > 0 );
                   bool startsAsAbove = above;// Whether the waveform
                   starts positive or negative

                                                while( loc <=
                   lengthOfSample )
                                                {
                                                        ++loc;
                                                        temp1 =
                   sampleBuffer->userWaveSample( loc, channel ) > 0; if(
                   temp1
                   != above )
                                                        {
                                                                if( temp1
                   == startsAsAbove )
                                                                {
                                                                        crossings.push_back(
                   loc );
                                                                }
                                                                above =
                   !above; loc += gap;
                                                        }
                                                }

                                                crossings.push_back( loc
                   ); crossings.push_back( lengthOfSample ); loc = 0;

                                                for( int j = 0; j < 256;
                   ++j )
                                                {
                                                        if(
                   crossings.size() < j )
                                                        {
                                                                break;
                                                        }
                                                        temp1 =
                   crossings[j]; temp2 = crossings[j+1]; for( int i = 0; i
                   < 2048; ++i )
                                                        {
                                                                mwm->waveforms[oscilNum][j*2048+i]
                   = sampleBuffer->userWaveSample(
                   ((i/2048.f)*(temp2-temp1)+temp1)/lengthOfSample,
                   channel );
                                                        }
                                                }

                                                break;
                                        }
            */
            /*
            case 7:  // Squeeze entire sample into 256 waveforms
            {
                mwm->morphMax[oscilNum]->setValue(
                        real_t(WAV_SIZE) /
            mwm->sampLen[oscilNum]->value()); mwm->morphMaxChanged();
                for(int i = 0; i < MAINWAV_SIZE; ++i)
                {
                    mwm->waveforms[oscilNum][i] =
            sampleBuffer->userWaveSample( qMin<real_t>(real_t(i) /
            real_t(WAV_SIZE), 1.), channel);
                }
                break;
            }
            case 11:  // Delete this.  Makes end of waveform match with
                      // beginning.
            {
                for(int i = 0; i < MAINWAV_SIZE; ++i)
                {
                    if(fmod(i, mwm->sampLen[oscilNum]->value())
                       >= mwm->sampLen[oscilNum]->value() - 200)
                    {
                        real_t thing = (-fmod(i,
            mwm->sampLen[oscilNum]->value())
                                        + mwm->sampLen[oscilNum]->value())
                                       / 200.;
                        mwm->waveforms[oscilNum][i]
                                = (mwm->waveforms[oscilNum][i] * thing)
                                  + ((-thing + 1)
                                     * mwm->waveforms[oscilNum][int(
                                             i
                                             - (fmod(i,
            mwm->sampLen[oscilNum]
                                                                ->value())))]);
                    }
                }
                break;
            }
            */
    }

    // DON'T DELETE THIS

    /*real_t start;
    real_t end;
    for( int j = 0; j < 256; ++j )
    {
            start = -mwm->waveforms[oscilNum][j*2048];
            end = -mwm->waveforms[oscilNum][j*2048+2047];
            for( int i = 0; i < 2048; ++i )
            {
                    mwm->waveforms[oscilNum][j*2048+i] +=
    (i/2048.f)*end +
    ((2048.f-i)/2048.f)*start;
            }
    }*/

    sampleBuffer->dataUnlock();

    /*
    else  // Delete this
    {
        for(int i = 0; i < 256; ++i)
        {
            real_t highestVolume = 0;
            for(int j = 0; j < 2048; ++j)
            {
                highestVolume = abs(mwm->waveforms[oscilNum][(i *
    2048) + j]) > highestVolume ? abs(mwm->waveforms[oscilNum]
                                                          [(i * 2048)
    + j]) : highestVolume;
            }
            if(highestVolume)
            {
                real_t multiplierThing = 1. / highestVolume;
                for(int j = 0; j < 2048; ++j)
                {
                    mwm->waveforms[oscilNum][(i * 2048) + j] *=
    multiplierThing;
                }
            }
        }
    }
    */
}

// Loads sample for sample oscillator
void MicrowaveView::openSampleFile()
{
    SampleBuffer* sampleBuffer = new SampleBuffer();
    QString       fileName     = sampleBuffer->openAndSetSampleFile();
    if(!fileName.isEmpty() == false)
        loadSample(sampleBuffer);
    delete sampleBuffer;
}

void MicrowaveView::loadSample(SampleBuffer* sampleBuffer)
{
    // const sample_rate_t sr = Engine::mixer()->processingSampleRate();
    const f_cnt_t frames  = sampleBuffer->frames();
    Microwave*    mwm     = model();
    const int32_t sampNum = mwm->sampNum.value() - 1;

    sampleBuffer->dataReadLock();

    mwm->samples[sampNum][0].clear();
    mwm->samples[sampNum][1].clear();

    // const real_t lengthOfSample = real_t(sr) / 1000. * frames;  // in
    // samples
    for(int i = 0; i < frames; ++i)
    {
        mwm->samples[sampNum][0].push_back(
                sampleBuffer->userWaveSample(real_t(i) / real_t(frames), 0));
        mwm->samples[sampNum][1].push_back(
                sampleBuffer->userWaveSample(real_t(i) / real_t(frames), 1));
    }

    sampleBuffer->dataUnlock();
}

void MicrowaveView::dropEvent(QDropEvent* _de)
{
    int tabNum = model()->currentTab;
    qInfo("MicrowaveView::dropEvent START tab=%d", tabNum);

    StringPair p = StringPairDrag::convertExternal(_de->mimeData());

    if(p.key() == "samplefile")
    {
        qInfo("MicrowaveView::dropEvent %s/%s", qPrintable(p.key()),
              qPrintable(p.value()));
        // openWaveFormFile(p.value());
        SampleBuffer* sampleBuffer = new SampleBuffer(p.value());
        if(tabNum == 0 || tabNum == 1)
            loadWaveForm(sampleBuffer);
        if(tabNum == 2)
            loadSample(sampleBuffer);
        delete sampleBuffer;
        _de->accept();
        return;
    }

    _de->ignore();
}

void MicrowaveView::dragEnterEvent(QDragEnterEvent* _dee)
{
    for(const QString& s: _dee->mimeData()->formats())
        qInfo("MicrowaveView::dragEnterEvent %s", qPrintable(s));

    StringPair p = StringPairDrag::convertExternal(_dee->mimeData());

    qInfo("MicrowaveView::dragEnterEvent key='%s' value='%s'",
          qPrintable(p.key()), qPrintable(p.value()));

    if(p.key() == "samplefile")
    {
        int tabNum = model()->currentTab;
        qInfo("MicrowaveView::dropEvent START tab=%d", tabNum);
        if(tabNum == 0 || tabNum == 1 || tabNum == 2)
        {
            _dee->acceptProposedAction();
            return;
        }
    }

    _dee->ignore();
}

QString MicrowaveManualView::s_manualText
        = "H<b>OW TO OPERATE YOUR MICROWAVE<br>"
          "<br>"
          "Table of Contents:<br>"
          "<br>"
          "1. Feature Overview<br>"
          " a. Wavetable tab<br>"
          " b. Sub Oscillator Tab<br>"
          " c. Sample Tab<br>"
          " d. Matrix Tab<br>"
          " e. Filter Tab<br>"
          " f. Miscellaneous Tab<br>"
          "2. CPU Preservation Guidelines<br>"
          "<br>"
          "<br>"
          "<br>"
          "<br>"
          "<br>"
          "<b>==FEATURE OVERVIEW==<br>"
          "<br>"
          "<br>"
          "-=WAVETABLE TAB=-<br>"
          "<br>"
          "If you zoom in all the way on a sound or waveform, you'll see "
          "these little \"audio pixels\".  These are called \"samples\", "
          "not "
          "to be confused with the common term \"sound sample\" which "
          "refers "
          "to any stored piece of audio.  These \"audio pixels\" can "
          "easily "
          "be seen when using LMMS's BitInvader.<br>"
          "<br>"
          "A \"wavetable synthesizer\" is a synthesizer that stores its "
          "waveforms as a list of samples.  This means synthesizers like "
          "BitInvader and WatSyn are technically wavetable synthesizers. "
          " "
          "But, the term \"wavetable synthesizer\" more commonly (but "
          "not "
          "more or less correctly) refers to a synthesizer that stores "
          "multiple waveforms, plays one waveform and repeats it, and "
          "allows "
          "the user to move a knob to change which waveform is being "
          "played. "
          " Synthesizers of this nature, even the basic ones, are "
          "unimaginably powerful.  Microwave is one of them.<br>"
          "<br>"
          "Microwave's wavetables have 256 waveforms, at 2048 samples "
          "each.  "
          "The Morph (MPH) knob chooses which of the 256 waveforms in "
          "the "
          "wavetable to play.  It is important to note that Microwave "
          "does "
          "not have any wavetable loaded by default, so no sound will be "
          "heard currently.  Load a sound file as a wavetable now (click "
          "the "
          "folder button at the bottom).  If you play a note while "
          "moving "
          "the Morph knob, you'll notice the waveform that is playing "
          "morphing to create new timbres.<br>"
          "<br>"
          "Range (RNG) is a feature unique to Microwave.  It takes "
          "waveforms "
          "in the wavetable near the one chosen by the Morph one, and "
          "mixes "
          "those in with the main waveform (at a lower volume as you get "
          "further away from the main waveform).  For example, a Morph "
          "of 5 "
          "and a Range of 2 will give you a mix between waveform 5, "
          "waveform "
          "4 at half volume, and waveform 6 at half volume.<br>"
          "<br>"
          "MDFY (Modify) and the dropdown box next to it (Modify Mode) "
          "are "
          "used to warp your wavetable realtime, using formulas I "
          "created "
          "myself.  Change the Modify Mode and move the Modify knob "
          "while "
          "playing a note.  Hear how each Modify Mode causes a "
          "drastically "
          "different change to the sound.  These are extremely useful "
          "for "
          "injecting more flavor and awesomeness into your sound.  Use "
          "all "
          "of them to learn what they can do.<br>"
          "<br>"
          "DET stands for Detune, which changes the pitch of that "
          "oscillator, in cents.  PHS stands for Phase, which simply "
          "phase "
          "shifts the oscillator, and RAND next to it is Phase "
          "Randomness, "
          "which sets the oscillator to a random phase with each "
          "note.<br>"
          "<br>"
          "Microwave supports very advanced unison abillities.  Unison "
          "is "
          "when you clone the oscillator multiple times, and play them "
          "all "
          "at the same time, usually with slight differences applied to "
          "them.  The original sound as well as the clones are called "
          "\"voices\".  In the UNISON box, NUM chooses the number of "
          "voices. "
          " DET detunes each voice slightly, a common unison feature.  "
          "MPH "
          "and MOD are special.  They change the Morph and Modify "
          "(respectively) values for each individual voice, which can "
          "create "
          "an amazing 3D sound.  It is important to note that every "
          "unison "
          "voice is calculated individually, so using large numbers of "
          "unison voices can be very detrimental to your CPU.<br>"
          "<br>"
          "Earlier I mentioned that Microwave's wavetables have 256 "
          "waveforms, at 2048 samples each.  This can be changed using "
          "the "
          "Sample Length knob.  This knob is meant to be used for "
          "finetuning "
          "your wavetable if the loading was slightly inaccurate.  If "
          "you "
          "notice your waveform moving left/right too much in the "
          "visualizer "
          "as you morph through the wavetable, the Sample Length knob "
          "may be "
          "able to fix that.<br>"
          "<br>"
          "With most synthesizers, CPU would be a major concern when "
          "using "
          "wavetables.  Luckily, I have put an obscene amount of work "
          "into "
          "optimizing Microwave, so this should be much less of a "
          "problem.  "
          "Feel free to go crazy.<br>"
          "<br>"
          "<br>"
          "-=SUB TAB=-<br>"
          "<br>"
          "This tab behaves a lot like BitInvader, but is significantly "
          "more "
          "useful in the context of Microwave.  This tab is meant to be "
          "used "
          "for many things:<br>"
          "1. Single-waveform oscillators/modulators<br>"
          "2. LFOs<br>"
          "3. Envelopes<br>"
          "4. Step Sequencers<br>"
          "5. Noise Generators<br>"
          "<br>"
          "In very early versions of Microwave, the five things listed "
          "above "
          "were all in their own tabs, and were later combined into one "
          "for "
          "obvious user-friendliness reasons.  I would like to quickly "
          "note "
          "here that I implore all of you to use Step Sequencers in "
          "Microwave all the time.  I wanted it to be one of the more "
          "highlighted features of Microwave, but never really had the "
          "chance.  Step Sequencers are an awesome and unique way to add "
          "rhythmic modulations to your sound.<br>"
          "<br>"
          "The LENGTH knob changes the length of the oscillator.  "
          "Decreasing "
          "this to a small number makes it very easy to use this as a "
          "Step "
          "Sequencer.  In some special cases you may also want to "
          "automate "
          "this knob for some interesting effects.<br>"
          "<br>"
          "There are four LEDs you can see at the bottom.  The top left "
          "is "
          "whether the oscillator is enabled.  The top right is "
          "\"Muted\", "
          "which is different.  When an oscillator is enabled but muted, "
          "it "
          "is still calculated and still used for modulation, but that "
          "oscillator's sound is never played.  You'll usually want this "
          "on "
          "when using this as an envelope/LFO/step sequencer.  The "
          "bottom "
          "left is keytracking.  When keytracking is disabled, the "
          "oscillator always plays at the same frequency regardless of "
          "the "
          "note you press.  You'll want to turn this off when you need "
          "your "
          "envelopes/LFOs/step sequencers to always go at the same "
          "speed.  "
          "The bottom right LED converts the oscillator into a noise "
          "generator, which generates a different flavor of noise "
          "depending "
          "on the graph you draw.<br>"
          "<br>"
          "When the tempo knob is set to anything other than 0, the "
          "pitch is "
          "decreased drastically (you'll probably want to mute it) so "
          "that "
          "it perfectly matches up with the set tempo when detune is set "
          "to "
          "0.  If you need it at half speed, double speed, etc., just "
          "change "
          "its pitch by octaves (because increasing by an octave doubles "
          "the "
          "frequency).<br>"
          "<br>"
          "Note that explanations on how to use this for modulation is "
          "explained in the Matrix Tab section.<br>"
          "<br>"
          "<br>"
          "-=SAMPLE TAB=-<br>"
          "<br>"
          "This tab is used to import entire samples to use as "
          "oscillators.  "
          "This means you can frequency modulate your cowbells with your "
          "airhorns, which you can then phase modulate with an ogre "
          "yelling "
          "about his swamp, which you can then amplitude modulate with a "
          "full-length movie about bees.  Alternatively, you could just "
          "use "
          "it as a simple way to layer in a sound or some noise sample "
          "with "
          "the sound you have already created.  It is important to note "
          "that "
          "imported samples are stored within Microwave, which means two "
          "things:<br>"
          "1. Unlike AudioFileProcessor, where the size of the sample "
          "does "
          "not impact the project file size, any samples in Microwave "
          "will "
          "be losslessly stored inside the project file and preset, "
          "which "
          "can make the project file extremely large.<br>"
          "2. When sending your project file or Microwave preset to "
          "somebody "
          "else, they do not need to have the sample to open it, unlike "
          "with "
          "AudioFileProcessor.<br>"
          "<br>"
          "With that being said, Microwave's Sample Tab is not meant as "
          "a "
          "replacement to AudioFileProcessor.  Microwave will use more "
          "CPU, "
          "and some audio problems may show up when playing notes other "
          "than "
          "A4 (e.g. unstable pitch and stuff).  In most cases, if what "
          "you "
          "want can be done with AudioFileProcessor, you should use "
          "AudioFileProcessor.  Otherwise, totally use Microwave.  The "
          "Sample Tab is useful for many reasons, especially for its "
          "modulation capabilities and the weird way Microwave can morph "
          "samples depending on its graph waveform.<br>"
          "<br>"
          "The Sample Tab has two new knobs.  Those change the start and "
          "end "
          "position of the sample.<br>"
          "<br>"
          "There are two new LEDs for this tab, at the right.  The "
          "second to "
          "last one enables the graph.  The graph determines the order "
          "in "
          "which the sample is played.  A saw wave will play the sample "
          "normally, and a reverse wave wave will play it backward.  "
          "Draw a "
          "random squiggle on it and... well, you'll hear it.  Pretty "
          "much, "
          "left/right is time, up/down is position in the sample.  Note "
          "that "
          "this will almost definitely change the pitch in most "
          "circumstances, because changing a sound's speed also changes "
          "its "
          "pitch.  The last LED enables and disabled sample looping.<br>"
          "<br>"
          "<br>"
          "-=MATRIX TAB=-<br>"
          "<br>"
          "This tab is used for a lot of things, ranging from "
          "modulation, "
          "effect routing, humanization, etc.  If you think it looks "
          "difficult, it's a lot easier than it looks.  If you think it "
          "looks easy, it's much more difficult than it looks.<br>"
          "<br>"
          "The matrix works by taking one or two inputs (whether it be "
          "from "
          "an oscillator, effect output, humanizer, or anything else) "
          "and "
          "outputting it somewhere (e.g. to control/modulate a knob, to "
          "an "
          "effect input, etc.).  It's fairly simple.<br>"
          "<br>"
          "Notice how there are three rows.  Only focus on the top two, "
          "as "
          "the top and bottom ones are functionally identical.  The top "
          "left "
          "dropdown box chooses the matrix input, and the LCD Spinboxes "
          "choose which input (e.g. which oscillator, which filter, "
          "etc.) to "
          "grab from.  The AMT knob chooses the Amount of the input that "
          "is "
          "fed into the output (e.g. input volume to effect, how much to "
          "move a knob by, etc.).  The CRV (Curve) knob gives that input "
          "a "
          "bias upward or downward, which can be used as an extremely "
          "simple "
          "way to shape and sculpt your modulations in absolutely "
          "brilliant "
          "ways.<br>"
          "<br>"
          "The middle left dropdown box sends which section to send the "
          "output to (e.g. which tab), and the dropdown box to the right "
          "of "
          "it is more specific (e.g. the Morph knob of the main tab, the "
          "third filter, etc.), as well as the LCD Spinbox following "
          "(for "
          "e.g. which oscillator to send it to).  The dropdown box to "
          "the "
          "right of that lets you choose between unidirectional and "
          "bidirectional modulation, as well as choosing how the two "
          "inputs "
          "are combined (e.g. add vs multiply).  The LED to the right of "
          "that converts the inputs from LFOs to envelopes if "
          "applicable.  "
          "In other words, it ignores repetitions of the input "
          "oscillators.<br>"
          "<br>"
          "It seems simple, but this section is one of the most "
          "important "
          "parts of Microwave.  Most synthesizers only have some "
          "combination "
          "of FM, PM, AM, and PWM modulation types.  Because of how "
          "Microwave's matrix tab is set up, allowing any parameter to "
          "be "
          "controlled by any oscillator at any speed, Microwave has "
          "those "
          "modulation types as well as over a hundred more.  Welcome to "
          "Choice Paralysis.  There will never be any point in time when "
          "the "
          "amount of choice you have will not overwhelm you.  Have fun "
          "with "
          "your freedom!<br>"
          "<br>"
          "<br>"
          "-=EFFECT TAB=-<br>"
          "(temporarily AKA Filter Tab)<br>"
          "<br>"
          "The current version of Microwave only has filters for "
          "effects, "
          "but that will be changed in future versions.<br>"
          "<br>"
          "FREQ is the filter's cutoff frequency, and RESO is the "
          "filter's "
          "resonance.  GAIN is the gain for peak filters, shelf filters, "
          "etc.  TYPE is the type of filter.  Microwave currently has "
          "lowpass, highpass, bandpass, peak, notch, low shelf, high "
          "shelf, "
          "allpass, and moog lowpass filters.  SLOPE runs the sound "
          "through "
          "the filter multiple times, changing the filter's slope at "
          "(usually) increments of 12 db.<br>"
          "<br>"
          "IN and OUT are volume knobs for the filter's input and "
          "output.  "
          "W/D is Wet/Dry.  PAN changes the Wet/Dry amount in individual "
          "ears, allowing you to use interesting panning filters.  SAT "
          "stands for Saturation, which allows you to add some simple "
          "distortion to your sound after it is filtered.<br>"
          "<br>"
          "FDBK (Feedback) stores the filter's output and sends it back "
          "to "
          "the filter's input after a certain amount of time.  This is a "
          "very odd, unique, interesting, and useful feature.  Without a "
          "filter in effect, increasing the feedback turns this into a "
          "comb "
          "filter.  Having a filter selected and working can create some "
          "ridiculous tibres you'd rarely ever hear out of most other "
          "synthesizers.  The change the feedback makes to the sound is "
          "very "
          "tonal, and the pitch it creates depends on its delay.  "
          "Because of "
          "this, I made it so the delay is keytracking by default, so "
          "the "
          "change it makes to your sound matches the note you play.  DET "
          "detunes that, and the keytracking button in the top right "
          "turns "
          "keytracking off for that.  Definitely have fun with this "
          "feature, "
          "you can get weird and amazing sound out of this.  Notice that "
          "this feature entirely allows Karplus-Strong synthesis, as "
          "well as "
          "any other ridiculous adaptations that pop into your head.<br>"
          "<br>"
          "<br>"
          "-=MISCELLANEOUS TAB=-<br>"
          "<br>"
          "The oversampling dropdown box is ***VERY*** important when it "
          "comes to Microwave's audio quality.  The higher it is, the "
          "cleaner your sound will be.  But, oversampling is also "
          "extremely "
          "detrimental to your CPU.  The multiplier of oversampling will "
          "be "
          "almost exactly the multiplier it applies to the processing "
          "power "
          "it uses.<br>"
          "<br>"
          "But how do you know whether you need oversampling?  2x should "
          "be "
          "appropriate in most (?) cases, but definitely not all of "
          "them.  "
          "If your sound is very basic, and all matrix inputs that "
          "control "
          "knobs only move very very slowly, then it's possible that as "
          "low "
          "as 1x oversampling (which means no oversampling) may be "
          "appropriate.  But, if the sound makes use of modulation, then "
          "higher values of oversampling may be appropriate, especially "
          "if "
          "the modulator contains or plays at high frequencies.  When in "
          "doubt, use your ears and compare.  Never neglect the "
          "oversampling.  If you're making a sound that uses modulation "
          "and "
          "it sounds a bit too much like a dying engine, bumping up the "
          "oversampling a bit may be all that is needed to bring the "
          "sound "
          "from okay to awesome.  I strongly suggest making oversampling "
          "tweaking a part of your habitual workflow when using "
          "Microwave.<br>"
          "<br>"
          "<br>"
          "<br>"
          "<br>"
          "<b>==CPU PRESERVATION GUIDELINES==<br>"
          "<br>"
          "<br>"
          "First and foremost, turn the wavetable tab's visualizer off.  "
          "That uses a ton of processing power.<br>"
          "<br>"
          "Microwave stores the highest oscillator that is enabled, and "
          "checks every oscillator from the first one to the highest "
          "enabled "
          "one to see whether it's enabled.  So, having just the 50th "
          "oscillator enabled will take significantly more processing "
          "power "
          "than having just the 1st one enabled, because it would need "
          "to "
          "check oscillators 1-50 to see whether they're enabled.<br>"
          "<br>"
          "Increasing the Range knob will use more CPU (since it needs "
          "to "
          "calculate nearby waveforms as well).  With very large Range "
          "values the CPU hit can get quite noticeable.  But, even "
          "though "
          "this needs to calculate nearby waveforms, it doesn't need to "
          "recalculate the entire oscillator, so increasing the Range "
          "won't "
          "use nearly as much CPU as, for example, increasing the number "
          "of "
          "unison voices.  <br>"
          "<br>"
          "Having a larger number of unison voices increases the CPU "
          "usage "
          "by around the voice amount.  For example, having 30 voices "
          "will "
          "use approximately 15x as much processing power as just two "
          "voices.  This increase is almost exactly linear (with the "
          "exception of using only one voice, which uses less than half "
          "the "
          "CPU as two voices, since having unison disabled entirely will "
          "prevent unison stuff from being calculated entirely).<br>"
          "<br>"
          "The values of the Morph and Modify knobs in the UNISON box have "
          "no impact on processing power needed, except for a small "
          "performance gain when they are at exactly 0.<br>"
          "<br>"
          "Having both of the DET knobs in and out of the UNISON box set "
          "to exactly 0 will result in a significant performance gain in "
          "the wavetable tab.  Any values other than 0 will have a near "
          "identical impact in comparison to each other.<br>"
          "<br>"
          "Even when the modify is not in use, having the Modify Mode set "
          "to None will use (sometimes significantly) less CPU than if it "
          "is set to something else.<br>"
          "<br>"
          "When using modify, expect the largest CPU hit from modes that "
          "require accessing other parts of the waveform to work (e.g. "
          "Squarify and Pulsify).<br>"
          "<br>"
          "Oversampling results in an almost exact multiplication in "
          "processing power needed.  So 8x oversampling will use "
          "approximately 4x as much CPU as 2x oversampling.<br>"
          "<br>";

MicrowaveManualView::MicrowaveManualView() : QTextEdit(s_manualText)
{
    setWindowTitle("Microwave Manual");
    setTextInteractionFlags(Qt::TextSelectableByKeyboard
                            | Qt::TextSelectableByMouse);
    // gui->mainWindow()->addWindowedWidget(this);
    // parentWidget()->setAttribute(Qt::WA_DeleteOnClose, false);
    SubWindow::putWidgetOnWorkspace(this, false, false, false, false);
    parentWidget()->setWindowIcon(PLUGIN_NAME::getIcon("logo"));
    parentWidget()->resize(640, 480);
}

void MicrowaveView::manualBtnClicked()
{
    MicrowaveManualView::getInstance()->hide();
    MicrowaveManualView::getInstance()->show();
}
