/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * InstrumentMidiIOView.cpp - MIDI-IO-View
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "InstrumentMidiIOView.h"

#include "ComboBox.h"
#include "Engine.h"
#include "GroupBox.h"
#include "InstrumentTrack.h"
#include "Knob.h"
#include "LcdSpinBox.h"
#include "LedCheckBox.h"
#include "MidiClient.h"
#include "MidiPortMenu.h"  // REQUIRED
#include "MidiProcessorView.h"
#include "Mixer.h"
//#include "ToolTip.h"
#include "Scale.h"
#include "embed.h"
#include "gui_templates.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
//#include <QMenu>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

InstrumentMidiIOView::InstrumentMidiIOView(InstrumentTrack* it,
                                           QWidget*         parent) :
      QWidget(parent),
      ModelView(it, this), m_track(it), m_rpBtn(nullptr),
      m_readablePortsMenu(nullptr), m_wpBtn(nullptr),
      m_writablePortsMenu(nullptr)
{
    allowModelChange(true);
    // allowNullModel(true);
    setAutoFillBackground(true);

    QScrollArea* mainSCA = new QScrollArea(this);
    mainSCA->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mainSCA->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainSCA->setFrameStyle(QFrame::NoFrame);
    mainSCA->setLineWidth(1);

    QVBoxLayout* thisLOT = new QVBoxLayout(this);
    setLayout(thisLOT);
    thisLOT->setContentsMargins(0, 0, 0, 0);
    thisLOT->addWidget(mainSCA, 0, 0);
    // thisLOT->setColumnStretch(0, 1);
    // thisLOT->setRowStretch(0, 1);

    QWidget*     mainPNL = new QWidget(mainSCA);
    QGridLayout* mainLOT = new QGridLayout(mainPNL);
    mainLOT->setContentsMargins(3, 3, 3, 3);
    mainLOT->setHorizontalSpacing(0);
    mainLOT->setVerticalSpacing(3);
    mainPNL->setLayout(mainLOT);

    /// Processor

    QWidget*     midiProcPNL = new QWidget(mainPNL);
    QHBoxLayout* midiProcLOT = new QHBoxLayout(midiProcPNL);
    midiProcLOT->setContentsMargins(6, 6, 6, 6);
    midiProcLOT->setSpacing(6);
    m_midiInProc  = new MidiProcessorView(true, it, midiProcPNL);
    m_midiOutProc = new MidiProcessorView(false, it, midiProcPNL);
    midiProcLOT->addWidget(m_midiInProc);
    midiProcLOT->addWidget(m_midiOutProc);

    /// Control Changes

    m_controlChangesGBX
            = new GroupBox(tr("CONTROL CHANGES"), mainPNL, false, true);
    QWidget*     controlChangesPNL = new QWidget(m_controlChangesGBX);
    QGridLayout* controlChangesLOT = new QGridLayout(controlChangesPNL);
    controlChangesLOT->setContentsMargins(3, 3, 3, 3);
    controlChangesLOT->setSpacing(3);
    m_controlChangesGBX->setContentWidget(controlChangesPNL);

    m_ccVolumeLCB = new LedCheckBox(m_controlChangesGBX);
    m_ccVolumeLCB->setText(tr("Volume"));
    m_ccVolumeLCB->setModel(m_track->volumeEnabledModel());
    // m_ccVolumeLCB->setEnabled(false);
    controlChangesLOT->addWidget(m_ccVolumeLCB, 0, 0);

    m_ccPanningLCB = new LedCheckBox(m_controlChangesGBX);
    m_ccPanningLCB->setText(tr("Panning"));
    m_ccPanningLCB->setModel(m_track->panningEnabledModel());
    // m_ccPanningLCB->setEnabled(false);
    controlChangesLOT->addWidget(m_ccPanningLCB, 1, 0);

    m_ccBendingLCB = new LedCheckBox(m_controlChangesGBX);
    m_ccBendingLCB->setText(tr("Bending"));
    m_ccBendingLCB->setModel(m_track->bendingEnabledModel());
    // m_ccBendingLCB->setEnabled(false);
    controlChangesLOT->addWidget(m_ccBendingLCB, 0, 1);

    m_ccModulatingLCB = new LedCheckBox(m_controlChangesGBX);
    m_ccModulatingLCB->setText(tr("(TODO) Modulating"));
    // m_ccModulatingLCB->setModel(m_track->modulatingEnabledModel());
    // m_ccModulatingLCB->setEnabled(false);
    controlChangesLOT->addWidget(m_ccModulatingLCB, 1, 1);

    controlChangesLOT->setColumnStretch(2, 1);
    controlChangesLOT->setRowStretch(2, 1);

    /// Midi Input

    m_midiInputGBX = new GroupBox(tr("MIDI INPUT"), mainPNL, true, false);
    QWidget*     midiInputPNL = new QWidget(m_midiInputGBX);
    QHBoxLayout* midiInputLOT = new QHBoxLayout(midiInputPNL);
    midiInputLOT->setContentsMargins(6, 6, 6, 6);
    midiInputLOT->setSpacing(6);
    m_midiInputGBX->setContentWidget(midiInputPNL);

    if(!Engine::mixer()->midiClient()->isRaw())
    {
        m_rpBtn = new QToolButton;
        m_rpBtn->setMinimumSize(32, 32);
        m_rpBtn->setText(tr("MIDI devices to receive MIDI events from"));
        m_rpBtn->setIcon(embed::getIconPixmap("piano"));
        m_rpBtn->setPopupMode(QToolButton::InstantPopup);
        midiInputLOT->addWidget(m_rpBtn, Qt::AlignTop | Qt::AlignHCenter);
    }

    m_inputChannelSpinBox = new LcdSpinBox(2, midiInputPNL);
    m_inputChannelSpinBox->addTextForValue(0, "--");
    m_inputChannelSpinBox->setLabel(tr("CHANNEL"));
    m_inputChannelSpinBox->setEnabled(false);
    midiInputLOT->addWidget(m_inputChannelSpinBox);

    m_fixedInputVelocitySpinBox = new LcdSpinBox(3, midiInputPNL);
    // m_fixedInputVelocitySpinBox->setDisplayOffset( 1 );
    m_fixedInputVelocitySpinBox->addTextForValue(-1, "---");  // 0
    m_fixedInputVelocitySpinBox->setLabel(tr("VELOCITY"));
    m_fixedInputVelocitySpinBox->setEnabled(false);
    midiInputLOT->addWidget(m_fixedInputVelocitySpinBox);

    m_transposeInputSpinBox = new LcdSpinBox(4, midiInputPNL);
    m_transposeInputSpinBox->addTextForValue(0, "----");
    m_transposeInputSpinBox->setText(tr("TRANSPOSE"));
    m_transposeInputSpinBox->setEnabled(false);
    midiInputLOT->addWidget(m_transposeInputSpinBox);

    midiInputLOT->addStretch(1);
    // m_midiInputGBX->setMinimumWidth(230);

    connect(m_midiInputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_inputChannelSpinBox, SLOT(setEnabled(bool)));
    connect(m_midiInputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_fixedInputVelocitySpinBox, SLOT(setEnabled(bool)));
    connect(m_midiInputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_transposeInputSpinBox, SLOT(setEnabled(bool)));

    /// Midi Output

    m_midiOutputGBX = new GroupBox(tr("MIDI OUTPUT"), mainPNL, true, false);
    QWidget*     midiOutputPNL = new QWidget(m_midiOutputGBX);
    QGridLayout* midiOutputLOT = new QGridLayout(midiOutputPNL);
    midiOutputLOT->setContentsMargins(6, 6, 6, 6);
    midiOutputLOT->setSpacing(6);
    m_midiOutputGBX->setContentWidget(midiOutputPNL);

    if(!Engine::mixer()->midiClient()->isRaw())
    {
        m_wpBtn = new QToolButton;
        m_wpBtn->setMinimumSize(32, 32);
        m_wpBtn->setText(tr("MIDI devices to send MIDI events to"));
        m_wpBtn->setIcon(embed::getIconPixmap("piano"));
        m_wpBtn->setPopupMode(QToolButton::InstantPopup);
        midiOutputLOT->addWidget(m_wpBtn, 0, 0, 2, 1,
                                 Qt::AlignTop | Qt::AlignHCenter);
    }

    m_outputChannelSpinBox = new LcdSpinBox(2, midiOutputPNL);
    m_outputChannelSpinBox->setLabel(tr("CHANNEL"));
    m_outputChannelSpinBox->setEnabled(false);
    midiOutputLOT->addWidget(m_outputChannelSpinBox, 0, 1);

    m_fixedOutputVelocitySpinBox = new LcdSpinBox(3, midiOutputPNL);
    // m_fixedOutputVelocitySpinBox->setDisplayOffset( 1 );
    m_fixedOutputVelocitySpinBox->addTextForValue(-1, "---");  // 0
    m_fixedOutputVelocitySpinBox->setLabel(tr("VELOCITY"));
    m_fixedOutputVelocitySpinBox->setEnabled(false);
    midiOutputLOT->addWidget(m_fixedOutputVelocitySpinBox, 0, 2);

    m_transposeOutputSpinBox = new LcdSpinBox(4, midiOutputPNL);
    m_transposeOutputSpinBox->addTextForValue(0, "----");
    m_transposeOutputSpinBox->setText(tr("TRANSPOSE"));
    m_transposeOutputSpinBox->setEnabled(false);
    midiOutputLOT->addWidget(m_transposeOutputSpinBox, 0, 3);

    m_outputProgramSpinBox = new LcdSpinBox(3, midiOutputPNL);
    m_outputProgramSpinBox->setLabel(tr("PROGRAM"));
    m_outputProgramSpinBox->setEnabled(false);
    midiOutputLOT->addWidget(m_outputProgramSpinBox, 1, 3);

    m_fixedOutputNoteSpinBox = new LcdSpinBox(3, midiOutputPNL);
    // m_fixedOutputNoteSpinBox->setDisplayOffset( 1 );
    m_fixedOutputNoteSpinBox->addTextForValue(-1, "---");  // 0
    m_fixedOutputNoteSpinBox->setLabel(tr("NOTE"));
    m_fixedOutputNoteSpinBox->setEnabled(false);
    midiOutputLOT->addWidget(m_fixedOutputNoteSpinBox, 1, 2);

    // midiOutputLOT->addStretch(1);
    midiOutputLOT->setColumnStretch(4, 1);
    midiOutputLOT->setRowStretch(2, 1);
    // m_midiOutputGBX->setMinimumWidth(230);

    connect(m_midiOutputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_outputChannelSpinBox, SLOT(setEnabled(bool)));
    connect(m_midiOutputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_fixedOutputVelocitySpinBox, SLOT(setEnabled(bool)));
    connect(m_midiOutputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_transposeOutputSpinBox, SLOT(setEnabled(bool)));
    connect(m_midiOutputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_outputProgramSpinBox, SLOT(setEnabled(bool)));
    connect(m_midiOutputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_fixedOutputNoteSpinBox, SLOT(setEnabled(bool)));

    /// Base Velocity

    m_baseVelocityGBX
            = new GroupBox(tr("CUSTOM BASE VELOCITY"), mainPNL, true, false);
    QWidget*     baseVelocityPNL = new QWidget(m_baseVelocityGBX);
    QHBoxLayout* baseVelocityLOT = new QHBoxLayout(baseVelocityPNL);
    midiOutputLOT->setContentsMargins(6, 6, 6, 6);
    midiOutputLOT->setSpacing(6);
    m_baseVelocityGBX->setContentWidget(baseVelocityPNL);

    /*
    QLabel* baseVelocityHelp
            = new QLabel(tr("Specify the velocity normalization base for "
                            "MIDI-based instruments at 100% note velocity"));
    baseVelocityHelp->setWordWrap(true);
    baseVelocityHelp->setFont(pointSize<8>(baseVelocityHelp->font()));
    baseVelocityLOT->addWidget(baseVelocityHelp);
    */

    m_baseVelocitySpinBox = new LcdSpinBox(3, baseVelocityPNL);
    m_baseVelocitySpinBox->setLabel(tr("BASE VELOCITY"));
    m_baseVelocitySpinBox->setEnabled(false);
    baseVelocityLOT->addWidget(m_baseVelocitySpinBox);

    baseVelocityLOT->addStretch(1);
    // m_baseVelocityGBX->setMinimumWidth(230);

    connect(m_baseVelocityGBX->ledButton(), SIGNAL(toggled(bool)),
            m_baseVelocitySpinBox, SLOT(setEnabled(bool)));

    /*
    m_midiInputGBX->setMinimumWidth(230);
    m_midiOutputGBX->setMinimumWidth(230);
    m_baseVelocityGBX->setMinimumWidth(230);
    */

    /// All Midi CC

    m_allMidiCCGBX = new GroupBox(tr("ALL MIDI CC"), mainPNL, false, true);
    QWidget*     allMidiCCPNL = new QWidget(m_allMidiCCGBX);
    QGridLayout* allMidiCCLOT = new QGridLayout(allMidiCCPNL);
    allMidiCCLOT->setContentsMargins(3, 3, 3, 3);
    allMidiCCLOT->setSpacing(3);
    m_allMidiCCGBX->setContentWidget(allMidiCCPNL);
    for(int i = 0; i < MidiControllerCount; i++)
    {
        m_midiCCKNB[i] = new Knob(allMidiCCPNL);
        m_midiCCKNB[i]->setText(QString("CC %1").arg(i));
        m_midiCCKNB[i]->setModel(m_track->midiCCModel(i));
        m_midiCCKNB[i]->setHintText(QString("MIDI CC %1, V ").arg(i), "");
        m_midiCCKNB[i]->setWhatsThis("");

        /*
        connect(m_track->m_midiCCKNB[i], SIGNAL(dataChanged()), this,
                SLOT(onDataChanged()));
        */

        allMidiCCLOT->addWidget(m_midiCCKNB[i], i / 7, i % 7);
    }

    mainLOT->addWidget(midiProcPNL, 0, 0, Qt::AlignHCenter);
    mainLOT->addWidget(m_controlChangesGBX, 1, 0);
    mainLOT->addWidget(m_midiInputGBX, 2, 0);
    mainLOT->addWidget(m_midiOutputGBX, 3, 0);
    mainLOT->addWidget(m_baseVelocityGBX, 4, 0);
    mainLOT->addWidget(m_allMidiCCGBX, 5, 0);

    mainLOT->setColumnStretch(0, 1);
    mainLOT->setRowStretch(6, 1);

    // mainPNL->setMinimumSize(236,350);
    mainSCA->setWidget(mainPNL);
    mainSCA->setWidgetResizable(true);
    // setFixedWidth(234);
}

InstrumentMidiIOView::~InstrumentMidiIOView()
{
}

void InstrumentMidiIOView::modelChanged()
{
    MidiPort* mp = castModel<MidiPort>();

    m_midiInputGBX->ledButton()->setModel(&mp->m_readableModel);
    m_inputChannelSpinBox->setModel(&mp->m_inputChannelModel);
    m_fixedInputVelocitySpinBox->setModel(&mp->m_fixedInputVelocityModel);
    m_transposeInputSpinBox->setModel(&mp->m_transposeInputModel);

    m_midiOutputGBX->ledButton()->setModel(&mp->m_writableModel);
    m_outputChannelSpinBox->setModel(&mp->m_outputChannelModel);
    m_fixedOutputVelocitySpinBox->setModel(&mp->m_fixedOutputVelocityModel);
    m_transposeOutputSpinBox->setModel(&mp->m_transposeOutputModel);
    m_fixedOutputNoteSpinBox->setModel(&mp->m_fixedOutputNoteModel);
    m_outputProgramSpinBox->setModel(&mp->m_outputProgramModel);

    //#ifdef PROVIDE_CUSTOM_BASE_VELOCITY_UI
    m_baseVelocitySpinBox->setModel(&mp->m_baseVelocityModel);
    //#endif

    if(m_rpBtn != nullptr)
    {
        m_readablePortsMenu = new MidiPortMenu(mp, MidiPort::Input);
        m_rpBtn->setMenu(m_readablePortsMenu);
    }

    if(m_wpBtn != nullptr)
    {
        m_writablePortsMenu = new MidiPortMenu(mp, MidiPort::Output);
        m_wpBtn->setMenu(m_writablePortsMenu);
    }
}

InstrumentMiscView::InstrumentMiscView(InstrumentTrack* it, QWidget* parent) :
      QWidget(parent), ModelView(it, this), m_track(it),
      m_scaleBankModel(it, tr("Scale bank")),
      m_scaleIndexModel(it, tr("Scale index"))
{
    allowModelChange(true);
    // allowNullModel(true);
    setAutoFillBackground(true);

    QScrollArea* mainSCA = new QScrollArea(this);
    mainSCA->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    mainSCA->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainSCA->setFrameStyle(QFrame::NoFrame);
    // mainSCA->setLineWidth(1);

    QVBoxLayout* thisLOT = new QVBoxLayout(this);
    setLayout(thisLOT);
    thisLOT->setContentsMargins(0, 0, 0, 0);
    thisLOT->addWidget(mainSCA, 0, 0);
    // thisLOT->setColumnStretch(0, 1);
    // thisLOT->setRowStretch(0, 1);

    QWidget*     mainPNL = new QWidget(mainSCA);
    QGridLayout* mainLOT = new QGridLayout(mainPNL);
    mainLOT->setContentsMargins(3, 3, 3, 3);
    mainLOT->setHorizontalSpacing(0);
    mainLOT->setVerticalSpacing(3);
    mainPNL->setLayout(mainLOT);

    /// Master pitch

    m_masterPitchGBX = new GroupBox(tr("MASTER PITCH"), mainPNL, true, true);
    m_masterPitchGBX->ledButton()->setModel(&it->m_useMasterPitchModel);

    QWidget*     masterPitchPNL = new QWidget(m_masterPitchGBX);
    QHBoxLayout* masterPitchLOT = new QHBoxLayout(masterPitchPNL);
    masterPitchLOT->setContentsMargins(6, 6, 6, 6);
    masterPitchLOT->setSpacing(6);
    m_masterPitchGBX->setContentWidget(masterPitchPNL);

    QLabel* masterPitchHelp
            = new QLabel(tr("Enables the use of Master Pitch"));
    masterPitchHelp->setWordWrap(true);
    masterPitchHelp->setFixedWidth(220);
    masterPitchLOT->addWidget(masterPitchHelp);
    masterPitchLOT->addStretch(1);

    /// Clef

    m_clefGBX = new GroupBox(tr("CLEF"), mainPNL, false, true);

    QWidget*     clefPNL = new QWidget(m_clefGBX);
    QGridLayout* clefLOT = new QGridLayout(clefPNL);
    clefLOT->setContentsMargins(6, 6, 6, 6);
    clefLOT->setSpacing(6);
    m_clefGBX->setContentWidget(clefPNL);

    // add clef combo box
    ComboBox* clefCMB = new ComboBox();
    // clefCMB->setModel(&it->m_clefModel);
    clefCMB->setMinimumWidth(7 * 27 + 4);

    /// Root and Mode

    m_rootModeGBX = new GroupBox(tr("ROOT AND MODE"), mainPNL, false, true);

    QWidget*     rootModePNL = new QWidget(m_rootModeGBX);
    QGridLayout* rootModeLOT = new QGridLayout(rootModePNL);
    rootModeLOT->setContentsMargins(6, 6, 6, 6);
    rootModeLOT->setSpacing(6);
    m_rootModeGBX->setContentWidget(rootModePNL);

    // add rootMode combo box
    ComboBox* rootCMB = new ComboBox();
    rootCMB->setModel(&it->m_rootModel);
    rootCMB->setMinimumWidth(3 * 27 + 4);

    ComboBox* modeCMB = new ComboBox();
    modeCMB->setModel(&it->m_modeModel);
    modeCMB->setMinimumWidth(7 * 27 + 4);

    rootModeLOT->addWidget(rootCMB, 0, 0, 1, 3);
    rootModeLOT->addWidget(modeCMB, 0, 1, 1, 7);
    rootModeLOT->setRowStretch(2, 1);
    rootModeLOT->setColumnStretch(7, 1);

    /// Scale

    m_scaleGBX = new GroupBox(tr("SCALE"), mainPNL, false, true);

    QWidget*     ScalePNL = new QWidget(m_scaleGBX);
    QGridLayout* scaleLOT = new QGridLayout(ScalePNL);
    scaleLOT->setContentsMargins(6, 6, 6, 6);
    scaleLOT->setSpacing(6);
    m_scaleGBX->setContentWidget(ScalePNL);

    // add scale combo boxes
    ComboBox* bankCMB = new ComboBox();
    bankCMB->setModel(&m_scaleBankModel);
    bankCMB->setMinimumWidth(3 * 27 + 4);

    ComboBox* indexCMB = new ComboBox();
    indexCMB->setModel(&m_scaleIndexModel);
    indexCMB->setMinimumWidth(7 * 27 + 12);

    Scale::fillBankModel(m_scaleBankModel);
    Scale::fillIndexModel(m_scaleIndexModel, 0);
    connect(&m_scaleBankModel, SIGNAL(dataChanged()), this,
            SLOT(updateScaleIndexModel()));
    connect(&m_scaleBankModel, SIGNAL(dataChanged()), this,
            SLOT(updateScale()));
    connect(&m_scaleIndexModel, SIGNAL(dataChanged()), this,
            SLOT(updateScale()));
    const Scale* scl = it->scale();
    m_scaleBankModel.setValue(scl != nullptr ? scl->bank() : 0);
    m_scaleIndexModel.setValue(scl != nullptr ? scl->index() : 0);

    int col = 0, row = 0;  // first row
    scaleLOT->addWidget(bankCMB, row, col++, 1, 3,
                        Qt::AlignBottom | Qt::AlignHCenter);
    col = 0;
    row++;
    scaleLOT->addWidget(indexCMB, row, col++, 1, 7,
                        Qt::AlignBottom | Qt::AlignHCenter);

    scaleLOT->setRowStretch(2, 1);
    scaleLOT->setColumnStretch(7, 1);

    mainLOT->addWidget(m_masterPitchGBX, 0, 0);
    mainLOT->addWidget(m_clefGBX, 1, 0);
    mainLOT->addWidget(m_rootModeGBX, 2, 0);
    mainLOT->addWidget(m_scaleGBX, 3, 0);

    mainLOT->setColumnStretch(0, 1);
    mainLOT->setRowStretch(4, 1);
    mainSCA->setWidget(mainPNL);
    mainSCA->setWidgetResizable(true);
}

InstrumentMiscView::~InstrumentMiscView()
{
}

void InstrumentMiscView::updateScaleIndexModel()
{
    int bank = m_scaleBankModel.value();
    int old  = m_scaleIndexModel.value();
    Scale::fillIndexModel(m_scaleIndexModel, bank);
    m_scaleIndexModel.setValue(old);
}

void InstrumentMiscView::updateScale()
{
    m_track->setScale(
            Scale::get(m_scaleBankModel.value(), m_scaleIndexModel.value()));
}
