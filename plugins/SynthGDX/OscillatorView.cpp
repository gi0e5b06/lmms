/*
 * OscillatorView.cpp - SynthGDX 8 oscillators
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

#include "OscillatorView.h"

#include "ComboBox.h"
//#include "HwWaveWidget.h"
#include "Knob.h"
#include "LedCheckBox.h"
#include "SynthGDX.h"
#include "TempoSyncKnob.h"
#include "VisualizationWidget.h"

#include <QGridLayout>
#include <QLabel>

OscillatorView::OscillatorView(OscillatorObject* _osc,
                               const int         _idx,
                               QWidget*          _parent) :
      QWidget(_parent),
      m_osc(_osc)
{
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);  // 6, 6, 6, 6);
    // mainLayout->setColumnStretch(8, 1);
    // mainLayout->setRowStretch(4, 1);
    mainLayout->setHorizontalSpacing(2);  // 6);
    mainLayout->setVerticalSpacing(2);

    m_showWave = new VisualizationWidget(200, 75, this, _osc->waveRing());
    // m_showWave->setDisplayMode(VisualizationWidget::Time);
    QObject::connect(_osc, SIGNAL(waveUpdated()), this,
                     SLOT(updateVisualizationWidget()));
    //_osc->updateWaveRing();

    // HwWaveWidget* ww = new HwWaveWidget(this, 100, 25);

    // QLabel* numLBL = new QLabel(QString("O%1").arg(_idx + 1));
    // numLBL->setAutoFillBackground(true);
    // QPalette pal(numLBL->palette());
    // pal.setBrush(backgroundRole(), QColor(128, 96, 96));
    // numLBL->setPalette(pal);

    LedCheckBox* enabledLCB = new LedCheckBox(this, LedCheckBox::Green);
    enabledLCB->setModel(&m_osc->m_enabledModel);
    enabledLCB->setText(QString("O%1").arg(_idx + 1));
    enabledLCB->setTextAnchorPoint(Qt::AnchorBottom);
    enabledLCB->setContentsMargins(0, 4, 0, 0);
    /*
    QPalette pal=enabledLCB->palette();
    pal.setColor(QPalette::Background, QColor(128, 96, 96));
    enabledLCB->setAutoFillBackground(true);
    enabledLCB->setPalette(pal);
    */
    enabledLCB->setStyleSheet("background-color:#806060;");
    enabledLCB->setMinimumSize(14, 249+72);

    Knob* waveMixKNB = new Knob(this);
    waveMixKNB->setModel(&m_osc->m_waveMixModel);
    waveMixKNB->setPointColor(Qt::white);
    waveMixKNB->setText("WMIX");

    Knob* waveAntialiasKNB = new Knob(this);
    waveAntialiasKNB->setModel(&m_osc->m_waveAntialiasModel);
    waveAntialiasKNB->setPointColor(Qt::white);
    waveAntialiasKNB->setText("AA");
    // waveAntialiasKNB->setTextAnchorPoint(Qt::AnchorBottom);

    // wave 1

    LedCheckBox* symetric1LCB = new LedCheckBox(this, LedCheckBox::White);
    symetric1LCB->setModel(&m_osc->m_wave1SymetricModel);
    symetric1LCB->setText("SYM");
    symetric1LCB->setTextAnchorPoint(Qt::AnchorBottom);

    LedCheckBox* reverse1LCB = new LedCheckBox(this, LedCheckBox::White);
    reverse1LCB->setModel(&m_osc->m_wave1ReverseModel);
    reverse1LCB->setText("REV");
    reverse1LCB->setTextAnchorPoint(Qt::AnchorBottom);

    ComboBox* bank1CMB = new ComboBox(this);
    bank1CMB->setModel(&m_osc->m_wave1BankModel);
    bank1CMB->setMinimumWidth(3 * 27 + 4);
    connect(&m_osc->m_wave1BankModel, SIGNAL(dataChanged()), this,
            SLOT(updateWave1IndexModel()));
    //connect(&m_osc->m_wave1BankModel, SIGNAL(dataChanged()), m_osc,
    //        SLOT(updateWaveRing()));

    ComboBox* index1CMB = new ComboBox(this);
    index1CMB->setModel(&m_osc->m_wave1IndexModel);
    index1CMB->setMinimumWidth(6 * 27 + 12);
    updateWave1IndexModel();

    LedCheckBox* absolute1LCB = new LedCheckBox(this, LedCheckBox::White);
    absolute1LCB->setModel(&m_osc->m_wave1AbsoluteModel);
    absolute1LCB->setText("ABS");
    absolute1LCB->setTextAnchorPoint(Qt::AnchorBottom);

    LedCheckBox* opposite1LCB = new LedCheckBox(this, LedCheckBox::White);
    opposite1LCB->setModel(&m_osc->m_wave1OppositeModel);
    opposite1LCB->setText("OPP");
    opposite1LCB->setTextAnchorPoint(Qt::AnchorBottom);

    LedCheckBox* complement1LCB = new LedCheckBox(this, LedCheckBox::White);
    complement1LCB->setModel(&m_osc->m_wave1ComplementModel);
    complement1LCB->setText("CPL");
    complement1LCB->setTextAnchorPoint(Qt::AnchorBottom);

    // wave 2

    LedCheckBox* symetric2LCB = new LedCheckBox(this, LedCheckBox::White);
    symetric2LCB->setModel(&m_osc->m_wave2SymetricModel);
    symetric2LCB->setText("SYM");
    symetric2LCB->setTextAnchorPoint(Qt::AnchorBottom);

    LedCheckBox* reverse2LCB = new LedCheckBox(this, LedCheckBox::White);
    reverse2LCB->setModel(&m_osc->m_wave2ReverseModel);
    reverse2LCB->setText("REV");
    reverse2LCB->setTextAnchorPoint(Qt::AnchorBottom);

    ComboBox* bank2CMB = new ComboBox(this);
    bank2CMB->setModel(&m_osc->m_wave2BankModel);
    bank2CMB->setMinimumWidth(3 * 27 + 4);
    connect(&m_osc->m_wave2BankModel, SIGNAL(dataChanged()), this,
            SLOT(updateWave2IndexModel()));

    ComboBox* index2CMB = new ComboBox(this);
    index2CMB->setModel(&m_osc->m_wave2IndexModel);
    index2CMB->setMinimumWidth(6 * 27 + 12);
    updateWave2IndexModel();

    LedCheckBox* absolute2LCB = new LedCheckBox(this, LedCheckBox::White);
    absolute2LCB->setModel(&m_osc->m_wave2AbsoluteModel);
    absolute2LCB->setText("ABS");
    absolute2LCB->setTextAnchorPoint(Qt::AnchorBottom);

    LedCheckBox* opposite2LCB = new LedCheckBox(this, LedCheckBox::White);
    opposite2LCB->setModel(&m_osc->m_wave2OppositeModel);
    opposite2LCB->setText("OPP");
    opposite2LCB->setTextAnchorPoint(Qt::AnchorBottom);

    LedCheckBox* complement2LCB = new LedCheckBox(this, LedCheckBox::White);
    complement2LCB->setModel(&m_osc->m_wave2ComplementModel);
    complement2LCB->setText("CPL");
    complement2LCB->setTextAnchorPoint(Qt::AnchorBottom);

    // setup volume-knob
    Knob* volumeKNB = new Knob(this);
    volumeKNB->setModel(&m_osc->m_volumeModel);
    volumeKNB->setText("VOL");
    volumeKNB->setVolumeKnob(true);
    volumeKNB->setHintText(tr("Osc %1 volume:").arg(_idx + 1), "%");
    volumeKNB->setWhatsThis(tr("With this knob you can set the volume of "
                               "oscillator %1. When setting a value of 0 the "
                               "oscillator is turned off. Otherwise you can "
                               "hear the oscillator as loud as you set it "
                               "here.")
                                    .arg(_idx + 1));

    // setup panning-knob
    Knob* panKNB = new Knob(this);
    panKNB->setModel(&m_osc->m_panModel);
    panKNB->setPointColor(Qt::magenta);
    panKNB->setText("PAN");
    panKNB->setHintText(tr("Osc %1 panning:").arg(_idx + 1), "");
    panKNB->setWhatsThis(tr("With this knob you can set the panning of the "
                            "oscillator %1. A value of -100 means 100% "
                            "left and a value of 100 moves oscillator-"
                            "output right.")
                                 .arg(_idx + 1));

    // setup coarse-knob
    Knob* coarseKNB = new Knob(this);
    coarseKNB->setModel(&m_osc->m_coarseModel);
    coarseKNB->setPointColor(Qt::cyan);
    coarseKNB->setText("DET");
    coarseKNB->setHintText(tr("Osc %1 coarse detuning:").arg(_idx + 1),
                           " " + tr("semitones"));
    coarseKNB->setWhatsThis(
            tr("With this knob you can set the coarse detuning of "
               "oscillator %1. You can detune the oscillator "
               "24 semitones (2 octaves) up and down. This is "
               "useful for creating sounds with a chord.")
                    .arg(_idx + 1));

    // setup knob for left fine-detuning
    Knob* fineLeftKNB = new Knob(this);
    fineLeftKNB->setModel(&m_osc->m_fineLeftModel);
    fineLeftKNB->setPointColor(Qt::cyan);
    fineLeftKNB->setText("FLT");
    fineLeftKNB->setHintText(tr("Osc %1 fine detuning left:").arg(_idx + 1),
                             " " + tr("cents"));
    fineLeftKNB->setWhatsThis(
            tr("With this knob you can set the fine detuning of "
               "oscillator %1 for the left channel. The fine-"
               "detuning is ranged between -100 cents and "
               "+100 cents. This is useful for creating "
               "\"fat\" sounds.")
                    .arg(_idx + 1));

    // setup knob for right fine-detuning
    Knob* fineRightKNB = new Knob(this);
    fineRightKNB->setModel(&m_osc->m_fineRightModel);
    fineRightKNB->setPointColor(Qt::cyan);
    fineRightKNB->setText("FRT");
    fineRightKNB->setHintText(tr("Osc %1 fine detuning right:").arg(_idx + 1),
                              " " + tr("cents"));
    fineRightKNB->setWhatsThis(
            tr("With this knob you can set the fine detuning of "
               "oscillator %1 for the right channel. The "
               "fine-detuning is ranged between -100 cents "
               "and +100 cents. This is useful for creating "
               "\"fat\" sounds.")
                    .arg(_idx + 1));

    // setup phase-offset-knob
    Knob* phaseOffset = new Knob(this);
    phaseOffset->setModel(&m_osc->m_phaseOffsetModel);
    phaseOffset->setPointColor(QColor("orange"));
    phaseOffset->setText("PHS");
    phaseOffset->setHintText(tr("Osc %1 phase-offset:").arg(_idx + 1),
                             " " + tr("degrees"));
    phaseOffset->setWhatsThis(
            tr("With this knob you can set the phase-offset of "
               "oscillator %1. That means you can move the "
               "point within an oscillation where the "
               "oscillator begins to oscillate. For example "
               "if you have a sine-wave and have a phase-"
               "offset of 180 degrees the wave will first go "
               "down. It's the same with a square-wave.")
                    .arg(_idx + 1));

    // setup stereo-phase-detuning-knob
    Knob* stereoPhaseDetuningKNB = new Knob(this);
    stereoPhaseDetuningKNB->setModel(&m_osc->m_stereoPhaseDetuningModel);
    stereoPhaseDetuningKNB->setPointColor(QColor("orange"));
    stereoPhaseDetuningKNB->setText("SPD");
    stereoPhaseDetuningKNB->setHintText(
            tr("Osc %1 stereo phase-detuning:").arg(_idx + 1),
            " " + tr("degrees"));
    stereoPhaseDetuningKNB->setWhatsThis(
            tr("With this knob you can set the stereo phase-"
               "detuning of oscillator %1. The stereo phase-"
               "detuning specifies the size of the difference "
               "between the phase-offset of left and right "
               "channel. This is very good for creating wide "
               "stereo sounds.")
                    .arg(_idx + 1));

    // wave pulse center
    Knob* pulseCenterKNB = new Knob(this);
    pulseCenterKNB->setModel(&m_osc->m_pulseCenterModel);
    pulseCenterKNB->setPointColor(QColor("orange"));
    pulseCenterKNB->setText("PC");

    // wave pulse width
    Knob* pulseWidthKNB = new Knob(this);
    pulseWidthKNB->setModel(&m_osc->m_pulseWidthModel);
    pulseWidthKNB->setPointColor(QColor("orange"));
    pulseWidthKNB->setText("PW");

    // lfo active
    LedCheckBox* lfoEnabledLCB = new LedCheckBox(this);
    lfoEnabledLCB->setModel(&m_osc->m_lfoEnabledModel);
    lfoEnabledLCB->setText("LFO");
    lfoEnabledLCB->setTextAnchorPoint(Qt::AnchorBottom);

    // lfo time
    TempoSyncKnob* lfoTimeKNB = new TempoSyncKnob(this);
    lfoTimeKNB->setModel(&m_osc->m_lfoTimeModel);
    lfoTimeKNB->setPointColor(QColor("green"));
    lfoTimeKNB->setText("TIME");
    lfoTimeKNB->setHintText(tr("O%1 LFO time:").arg(_idx + 1), "ms");

    // velocity amount
    Knob* velocityKNB = new Knob(this);
    velocityKNB->setModel(&m_osc->m_velocityAmountModel);
    velocityKNB->setPointColor(Qt::red);
    velocityKNB->setText("VEL");

    // harmonics
    Knob* harm2KNB = new Knob(this);
    harm2KNB->setModel(&m_osc->m_harm2Model);
    harm2KNB->setPointColor(QColor(0, 218, 36));
    harm2KNB->setText("H2");

    Knob* harm3KNB = new Knob(this);
    harm3KNB->setModel(&m_osc->m_harm3Model);
    harm3KNB->setPointColor(QColor(0, 182, 72));
    harm3KNB->setText("H3");

    Knob* harm4KNB = new Knob(this);
    harm4KNB->setModel(&m_osc->m_harm4Model);
    harm4KNB->setPointColor(QColor(0, 145, 109));
    harm4KNB->setText("H4");

    Knob* harm5KNB = new Knob(this);
    harm5KNB->setModel(&m_osc->m_harm5Model);
    harm5KNB->setPointColor(QColor(0, 109, 145));
    harm5KNB->setText("H5");

    Knob* harm6KNB = new Knob(this);
    harm6KNB->setModel(&m_osc->m_harm6Model);
    harm6KNB->setPointColor(QColor(0, 72, 182));
    harm6KNB->setText("H6");

    Knob* harm7KNB = new Knob(this);
    harm7KNB->setModel(&m_osc->m_harm7Model);
    harm7KNB->setPointColor(QColor(0, 36, 218));
    harm7KNB->setText("H7");

    // slope
    Knob* slopeKNB = new Knob(this);
    slopeKNB->setModel(&m_osc->m_slopeModel);
    slopeKNB->setPointColor(QColor("green"));
    slopeKNB->setText("SLOPE");

    // low pass
    Knob* lowPassKNB = new Knob(this);
    lowPassKNB->setModel(&m_osc->m_lowPassModel);
    lowPassKNB->setPointColor(Qt::yellow);
    lowPassKNB->setText("LOW");

    // high pass
    Knob* highPassKNB = new Knob(this);
    highPassKNB->setModel(&m_osc->m_highPassModel);
    highPassKNB->setPointColor(Qt::yellow);
    highPassKNB->setText("HIGH");

    // wall
    Knob* wallKNB = new Knob(this);
    wallKNB->setModel(&m_osc->m_wallModel);
    wallKNB->setPointColor(Qt::yellow);
    wallKNB->setText("WALL");

    // skew
    Knob* skewKNB = new Knob(this);
    skewKNB->setModel(&m_osc->m_skewModel);
    skewKNB->setPointColor(Qt::yellow);
    skewKNB->setText("SKEW");

    // smooth
    Knob* smoothKNB = new Knob(this);
    smoothKNB->setModel(&m_osc->m_smoothModel);
    smoothKNB->setPointColor(Qt::yellow);
    smoothKNB->setText("SMTH");

    // portamento
    Knob* portamentoKNB = new Knob(this);
    portamentoKNB->setModel(&m_osc->m_portamentoModel);
    portamentoKNB->setPointColor(QColor("green"));
    portamentoKNB->setText("PRTM");

    mainLayout->addWidget(enabledLCB, 0, 0, 9, 1,
                          Qt::AlignTop | Qt::AlignHCenter);

    int col = 0, row = 0;
    mainLayout->addWidget(m_showWave, row, ++col, 1, 7,
                          Qt::AlignBottom | Qt::AlignHCenter);
    /*
    col = 0;
    row++;
    mainLayout->addWidget(ww, row, ++col, 1, 7,
                          Qt::AlignBottom | Qt::AlignHCenter);
    */

    col = 0;  // first row
    row++;
    mainLayout->addWidget(bank1CMB, row, ++col, 1, 3,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(reverse1LCB, row, col = 4, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(absolute1LCB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(opposite1LCB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(complement1LCB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    col = 0;
    row++;
    mainLayout->addWidget(index1CMB, row, ++col, 1, 6,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(symetric1LCB, row, col = 7, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    col = 0;
    row++;  // second row
    mainLayout->addWidget(bank2CMB, row, ++col, 1, 3,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(reverse2LCB, row, col = 4, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(absolute2LCB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(opposite2LCB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(complement2LCB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    col = 0;
    row++;
    mainLayout->addWidget(index2CMB, row, ++col, 1, 6,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(symetric2LCB, row, col = 7, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    col = 0;
    row++;  // third row
    mainLayout->addWidget(volumeKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(velocityKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(panKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(coarseKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(fineLeftKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(fineRightKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(lfoEnabledLCB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignRight);
    col = 0;
    row++;  // fourth row
    mainLayout->addWidget(waveMixKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(waveAntialiasKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(phaseOffset, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(stereoPhaseDetuningKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(pulseCenterKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(pulseWidthKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(lfoTimeKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    col = 0;
    row++;  // fifth row
    mainLayout->addWidget(harm2KNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(harm3KNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(harm4KNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(harm5KNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(harm6KNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(harm7KNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(slopeKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    col = 0;
    row++;  // sixth row
    mainLayout->addWidget(wallKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(lowPassKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(highPassKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(smoothKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    ++col;
    mainLayout->addWidget(skewKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(portamentoKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    setFixedWidth(234);
}

OscillatorView::~OscillatorView()
{
    /*
disconnect(&m_osc->m_wave1BankModel, SIGNAL(dataChanged()), this,
           SLOT(updateWave1IndexModel()));
disconnect(&m_osc->m_wave2BankModel, SIGNAL(dataChanged()), this,
           SLOT(updateWave2IndexModel()));
m_osc = NULL;
    */
}

void OscillatorView::updateWave1IndexModel()
{
    int bank1 = m_osc->m_wave1BankModel.value();
    int old   = m_osc->m_wave1IndexModel.value();
    WaveFormStandard::fillIndexModel(m_osc->m_wave1IndexModel, bank1);
    m_osc->m_wave1IndexModel.setValue(old);
}

void OscillatorView::updateWave2IndexModel()
{
    int bank2 = m_osc->m_wave2BankModel.value();
    int old   = m_osc->m_wave2IndexModel.value();
    WaveFormStandard::fillIndexModel(m_osc->m_wave2IndexModel, bank2);
    m_osc->m_wave2IndexModel.setValue(old);
}

/*
void OscillatorView::contextMenuEvent( QContextMenuEvent * _cme )
{
        Subwindow::putWidgetOnWorkspace(graph,true,false,false,false);
}
*/

void OscillatorView::updateVisualizationWidget()
{
    // qInfo("OscillatorView::updateVisualizationWidget");
    m_showWave->update();
}
