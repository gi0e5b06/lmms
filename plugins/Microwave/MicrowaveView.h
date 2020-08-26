/*
 * MicrowaveView.h -
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

#ifndef MICROWAVE_VIEW_H
#define MICROWAVE_VIEW__H

#include "ComboBox.h"
#include "Graph.h"
#include "InstrumentView.h"
#include "Knob.h"
#include "LcdSpinBox.h"
#include "LedCheckBox.h"
#include "Microwave.h"
//#include "MemoryManager.h"
#include "PixmapButton.h"
#include "Plugin.h"
#include "SampleBuffer.h"
//#include "stdshims.h"

#include "embed.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QScrollBar>

class MicrowaveView : public InstrumentView
{
    Q_OBJECT

  public:
    MicrowaveView(Microwave* _instrument, QWidget* _parent);
    virtual ~MicrowaveView(){};

    Microwave* model()
    {
        return castModel<Microwave>();
    }

    const Microwave* model() const
    {
        return castModel<Microwave>();
    }

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

    // void chooseWavetableFile();
    void openWaveFormFile();
    // void openWaveFormFileBtnClicked();
    void openSampleFile();
    // void openSampleFileBtnClicked();
    // void confirmWavetableLoadClicked();

    void modUpClicked(int);
    void modDownClicked(int);

    void tabBtnClicked(int);

    void manualBtnClicked();

    void updateBackground();

    void flipperClicked();

    void XBtnClicked();

  protected:
    void loadWaveForm(SampleBuffer* sampleBuffer);
    void loadSample(SampleBuffer* sampleBuffer);

  private:
    virtual void modelChanged();

    virtual void mouseMoveEvent(QMouseEvent* _me);
    virtual void wheelEvent(QWheelEvent* _me);
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

    PixmapButton* openWaveFormBTN;
    PixmapButton* confirmLoadBTN;
    PixmapButton* openSampleBTN;

    PixmapButton* modUpArrow[NB_MODLT];
    PixmapButton* modDownArrow[NB_MODLT];

    PixmapButton* tab1Btn;
    PixmapButton* tab2Btn;
    PixmapButton* tab3Btn;
    PixmapButton* tab4Btn;
    PixmapButton* tab5Btn;
    PixmapButton* tab6Btn;

    PixmapButton* mainFlipBtn;
    PixmapButton* subFlipBtn;

    PixmapButton* manualBtn;

    Knob*     loadAlgKnob;
    Knob*     loadChnlKnob;
    Knob*     scrollKnob;
    Knob*     visvolKnob;
    ComboBox* oversampleBox;
    ComboBox* loadModeBox;
    // static QPixmap* s_artwork;
    Graph*       graph;
    LedCheckBox* visualizeToggle;
    Knob*        wtLoad1Knob;
    Knob*        wtLoad2Knob;
    Knob*        wtLoad3Knob;
    Knob*        wtLoad4Knob;

    LcdSpinBox* mainNumBox;
    LcdSpinBox* subNumBox;
    LcdSpinBox* sampNumBox;
    LcdSpinBox* modNumBox;

    Knob*        morphKnob[NB_MAINOSC];
    Knob*        rangeKnob[NB_MAINOSC];
    ComboBox*    modifyModeBox[NB_MAINOSC];
    Knob*        modifyKnob[NB_MAINOSC];
    Knob*        sampLenKnob[NB_MAINOSC];
    Knob*        morphMaxKnob[NB_MAINOSC];
    Knob*        unisonVoicesKnob[NB_MAINOSC];
    Knob*        unisonDetuneKnob[NB_MAINOSC];
    Knob*        unisonMorphKnob[NB_MAINOSC];
    Knob*        unisonModifyKnob[NB_MAINOSC];
    Knob*        detuneKnob[NB_MAINOSC];
    Knob*        phaseKnob[NB_MAINOSC];
    Knob*        phaseRandKnob[NB_MAINOSC];
    Knob*        volKnob[NB_MAINOSC];
    LedCheckBox* enabledToggle[NB_MAINOSC];
    LedCheckBox* mutedToggle[NB_MAINOSC];
    Knob*        panKnob[NB_MAINOSC];

    Knob*        filtInVolKnob[NB_FILTR];
    ComboBox*    filtTypeBox[NB_FILTR];
    ComboBox*    filtSlopeBox[NB_FILTR];
    Knob*        filtCutoffKnob[NB_FILTR];
    Knob*        filtResoKnob[NB_FILTR];
    Knob*        filtGainKnob[NB_FILTR];
    Knob*        filtSatuKnob[NB_FILTR];
    Knob*        filtWetDryKnob[NB_FILTR];
    Knob*        filtBalKnob[NB_FILTR];
    Knob*        filtOutVolKnob[NB_FILTR];
    LedCheckBox* filtEnabledToggle[NB_FILTR];
    Knob*        filtFeedbackKnob[NB_FILTR];
    Knob*        filtDetuneKnob[NB_FILTR];
    LedCheckBox* filtKeytrackingToggle[NB_FILTR];
    LedCheckBox* filtMutedToggle[NB_FILTR];

    ComboBox*    modOutSecBox[NB_MODLT];
    ComboBox*    modOutSigBox[NB_MODLT];
    LcdSpinBox*  modOutSecNumBox[NB_MODLT];
    ComboBox*    modInBox[NB_MODLT];
    LcdSpinBox*  modInNumBox[NB_MODLT];
    LcdSpinBox*  modInOtherNumBox[NB_MODLT];
    Knob*        modInAmntKnob[NB_MODLT];
    Knob*        modInCurveKnob[NB_MODLT];
    ComboBox*    modInBox2[NB_MODLT];
    LcdSpinBox*  modInNumBox2[NB_MODLT];
    LcdSpinBox*  modInOtherNumBox2[NB_MODLT];
    Knob*        modInAmntKnob2[NB_MODLT];
    Knob*        modInCurveKnob2[NB_MODLT];
    LedCheckBox* modEnabledToggle[NB_MODLT];
    ComboBox*    modCombineTypeBox[NB_MODLT];
    LedCheckBox* modTypeToggle[NB_MODLT];

    LedCheckBox* subEnabledToggle[NB_SUBOSC];
    Knob*        subVolKnob[NB_SUBOSC];
    Knob*        subPhaseKnob[NB_SUBOSC];
    Knob*        subPhaseRandKnob[NB_SUBOSC];
    Knob*        subDetuneKnob[NB_SUBOSC];
    LedCheckBox* subMutedToggle[NB_SUBOSC];
    LedCheckBox* subKeytrackToggle[NB_SUBOSC];
    Knob*        subSampLenKnob[NB_SUBOSC];
    LedCheckBox* subNoiseToggle[NB_SUBOSC];
    Knob*        subPanningKnob[NB_SUBOSC];
    Knob*        subTempoKnob[NB_SUBOSC];

    LedCheckBox* sampleEnabledToggle[NB_SMPLR];
    LedCheckBox* sampleGraphEnabledToggle[NB_SMPLR];
    LedCheckBox* sampleMutedToggle[NB_SMPLR];
    LedCheckBox* sampleKeytrackingToggle[NB_SMPLR];
    LedCheckBox* sampleLoopToggle[NB_SMPLR];
    Knob*        sampleVolumeKnob[NB_SMPLR];
    Knob*        samplePanningKnob[NB_SMPLR];
    Knob*        sampleDetuneKnob[NB_SMPLR];
    Knob*        samplePhaseKnob[NB_SMPLR];
    Knob*        samplePhaseRandKnob[NB_SMPLR];
    Knob*        sampleStartKnob[NB_SMPLR];
    Knob*        sampleEndKnob[NB_SMPLR];

    Knob* macroKnob[NB_MACRO];

    QScrollBar* effectScrollBar;
    QScrollBar* matrixScrollBar;

    QLabel* filtForegroundLabel;
    QLabel* filtBoxesLabel;
    QLabel* matrixForegroundLabel;
    QLabel* matrixBoxesLabel;

    QPalette pal            = QPalette();
    QPixmap  tab1ArtworkImg = PLUGIN_NAME::getPixmap("tab1_artwork");
    QPixmap  tab1FlippedArtworkImg
            = PLUGIN_NAME::getPixmap("tab1_artwork_flipped");
    QPixmap tab2ArtworkImg = PLUGIN_NAME::getPixmap("tab2_artwork");
    QPixmap tab2FlippedArtworkImg
            = PLUGIN_NAME::getIconPixmap("tab2_artwork_flipped");
    QPixmap tab3ArtworkImg = PLUGIN_NAME::getPixmap("tab3_artwork");
    QPixmap tab4ArtworkImg = PLUGIN_NAME::getPixmap("tab4_artwork");
    QPixmap tab5ArtworkImg = PLUGIN_NAME::getPixmap("tab5_artwork");
    QPixmap tab6ArtworkImg = PLUGIN_NAME::getPixmap("tab6_artwork");
    QPixmap tab7ArtworkImg = PLUGIN_NAME::getPixmap("tab7_artwork");

    // QString wavetableFileName = "";

    Microwave* microwave;

    // real_t temp1;
    // real_t temp2;

    friend class MSynth;
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
