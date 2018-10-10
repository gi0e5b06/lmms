/*
 * InstrumentMidiIOView.cpp - MIDI-IO-View
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "InstrumentMidiIOView.h"

#include "ComboBox.h"
#include "Engine.h"
#include "GroupBox.h"
#include "InstrumentTrack.h"
#include "LcdSpinBox.h"
#include "LedCheckBox.h"
#include "MidiClient.h"
#include "MidiPortMenu.h"  // REQUIRED
#include "Mixer.h"
//#include "ToolTip.h"
#include "Scale.h"
#include "embed.h"
#include "gui_templates.h"

#include <QHBoxLayout>
#include <QLabel>
//#include <QMenu>
#include <QToolButton>
#include <QVBoxLayout>

InstrumentMidiIOView::InstrumentMidiIOView(QWidget* parent) :
      QWidget(parent), ModelView(NULL, this), m_rpBtn(NULL), m_wpBtn(NULL)
{
    setAutoFillBackground(true);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 3, 2, 5);

    /// Midi Input

    m_midiInputGBX = new GroupBox(tr("MIDI INPUT"), this, true, false);
    QWidget*     midiInputPNL    = new QWidget(m_midiInputGBX);
    QHBoxLayout* midiInputLayout = new QHBoxLayout(midiInputPNL);
    midiInputLayout->setContentsMargins(6, 6, 6, 6);
    midiInputLayout->setSpacing(6);
    m_midiInputGBX->setContentWidget(midiInputPNL);

    if(!Engine::mixer()->midiClient()->isRaw())
    {
        m_rpBtn = new QToolButton;
        m_rpBtn->setMinimumSize(32, 32);
        m_rpBtn->setText(tr("MIDI devices to receive MIDI events from"));
        m_rpBtn->setIcon(embed::getIconPixmap("piano"));
        m_rpBtn->setPopupMode(QToolButton::InstantPopup);
        midiInputLayout->addWidget(m_rpBtn);
    }

    m_inputChannelSpinBox = new LcdSpinBox(2, midiInputPNL);
    m_inputChannelSpinBox->addTextForValue(0, "--");
    m_inputChannelSpinBox->setLabel(tr("CHANNEL"));
    m_inputChannelSpinBox->setEnabled(false);
    midiInputLayout->addWidget(m_inputChannelSpinBox);

    m_fixedInputVelocitySpinBox = new LcdSpinBox(3, midiInputPNL);
    // m_fixedInputVelocitySpinBox->setDisplayOffset( 1 );
    m_fixedInputVelocitySpinBox->addTextForValue(-1, "---");  // 0
    m_fixedInputVelocitySpinBox->setLabel(tr("VELOCITY"));
    m_fixedInputVelocitySpinBox->setEnabled(false);
    midiInputLayout->addWidget(m_fixedInputVelocitySpinBox);

    midiInputLayout->addStretch(1);
    // m_midiInputGBX->setMinimumWidth(230);
    layout->addWidget(m_midiInputGBX);

    connect(m_midiInputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_inputChannelSpinBox, SLOT(setEnabled(bool)));
    connect(m_midiInputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_fixedInputVelocitySpinBox, SLOT(setEnabled(bool)));

    /// Midi Output

    m_midiOutputGBX = new GroupBox(tr("MIDI OUTPUT"), this, true, false);
    QWidget*     midiOutputPNL    = new QWidget(m_midiOutputGBX);
    QHBoxLayout* midiOutputLayout = new QHBoxLayout(midiOutputPNL);
    midiOutputLayout->setContentsMargins(6, 6, 6, 6);
    midiOutputLayout->setSpacing(6);
    m_midiOutputGBX->setContentWidget(midiOutputPNL);

    if(!Engine::mixer()->midiClient()->isRaw())
    {
        m_wpBtn = new QToolButton;
        m_wpBtn->setMinimumSize(32, 32);
        m_wpBtn->setText(tr("MIDI devices to send MIDI events to"));
        m_wpBtn->setIcon(embed::getIconPixmap("piano"));
        m_wpBtn->setPopupMode(QToolButton::InstantPopup);
        midiOutputLayout->addWidget(m_wpBtn);
    }

    m_outputChannelSpinBox = new LcdSpinBox(2, midiOutputPNL);
    m_outputChannelSpinBox->setLabel(tr("CHANNEL"));
    m_outputChannelSpinBox->setEnabled(false);
    midiOutputLayout->addWidget(m_outputChannelSpinBox);

    m_fixedOutputVelocitySpinBox = new LcdSpinBox(3, midiOutputPNL);
    // m_fixedOutputVelocitySpinBox->setDisplayOffset( 1 );
    m_fixedOutputVelocitySpinBox->addTextForValue(-1, "---");  // 0
    m_fixedOutputVelocitySpinBox->setLabel(tr("VELOCITY"));
    m_fixedOutputVelocitySpinBox->setEnabled(false);
    midiOutputLayout->addWidget(m_fixedOutputVelocitySpinBox);

    m_outputProgramSpinBox = new LcdSpinBox(3, midiOutputPNL);
    m_outputProgramSpinBox->setLabel(tr("PROGRAM"));
    m_outputProgramSpinBox->setEnabled(false);
    midiOutputLayout->addWidget(m_outputProgramSpinBox);

    m_fixedOutputNoteSpinBox = new LcdSpinBox(3, midiOutputPNL);
    // m_fixedOutputNoteSpinBox->setDisplayOffset( 1 );
    m_fixedOutputNoteSpinBox->addTextForValue(-1, "---");  // 0
    m_fixedOutputNoteSpinBox->setLabel(tr("NOTE"));
    m_fixedOutputNoteSpinBox->setEnabled(false);
    midiOutputLayout->addWidget(m_fixedOutputNoteSpinBox);

    midiOutputLayout->addStretch(1);
    // m_midiOutputGBX->setMinimumWidth(230);
    layout->addWidget(m_midiOutputGBX);

    connect(m_midiOutputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_outputChannelSpinBox, SLOT(setEnabled(bool)));
    connect(m_midiOutputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_fixedOutputVelocitySpinBox, SLOT(setEnabled(bool)));
    connect(m_midiOutputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_outputProgramSpinBox, SLOT(setEnabled(bool)));
    connect(m_midiOutputGBX->ledButton(), SIGNAL(toggled(bool)),
            m_fixedOutputNoteSpinBox, SLOT(setEnabled(bool)));


    /// Base Velocity

    m_baseVelocityGBX
            = new GroupBox(tr("CUSTOM BASE VELOCITY"), this, true, false);
    QWidget*     baseVelocityPNL    = new QWidget(m_baseVelocityGBX);
    QHBoxLayout* baseVelocityLayout = new QHBoxLayout(baseVelocityPNL);
    midiOutputLayout->setContentsMargins(6, 6, 6, 6);
    midiOutputLayout->setSpacing(6);
    m_baseVelocityGBX->setContentWidget(baseVelocityPNL);

    /*
    QLabel* baseVelocityHelp
            = new QLabel(tr("Specify the velocity normalization base for "
                            "MIDI-based instruments at 100% note velocity"));
    baseVelocityHelp->setWordWrap(true);
    baseVelocityHelp->setFont(pointSize<8>(baseVelocityHelp->font()));
    baseVelocityLayout->addWidget(baseVelocityHelp);
    */
 
    m_baseVelocitySpinBox = new LcdSpinBox(3, baseVelocityPNL);
    m_baseVelocitySpinBox->setLabel(tr("BASE VELOCITY"));
    m_baseVelocitySpinBox->setEnabled(false);
    baseVelocityLayout->addWidget(m_baseVelocitySpinBox);

    baseVelocityLayout->addStretch(1);
    // m_baseVelocityGBX->setMinimumWidth(230);
    layout->addWidget(m_baseVelocityGBX);

    connect(m_baseVelocityGBX->ledButton(), SIGNAL(toggled(bool)),
            m_baseVelocitySpinBox, SLOT(setEnabled(bool)));

    layout->addStretch(1);
    setLayout(layout);
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

    m_midiOutputGBX->ledButton()->setModel(&mp->m_writableModel);
    m_outputChannelSpinBox->setModel(&mp->m_outputChannelModel);
    m_fixedOutputVelocitySpinBox->setModel(&mp->m_fixedOutputVelocityModel);
    m_fixedOutputNoteSpinBox->setModel(&mp->m_fixedOutputNoteModel);
    m_outputProgramSpinBox->setModel(&mp->m_outputProgramModel);

    //#ifdef PROVIDE_CUSTOM_BASE_VELOCITY_UI
    m_baseVelocitySpinBox->setModel(&mp->m_baseVelocityModel);
    //#endif

    if(m_rpBtn)
    {
        m_rpBtn->setMenu(mp->m_readablePortsMenu);
    }

    if(m_wpBtn)
    {
        m_wpBtn->setMenu(mp->m_writablePortsMenu);
    }
}

InstrumentMiscView::InstrumentMiscView(InstrumentTrack* it, QWidget* parent) :
      QWidget(parent), m_track(it)
{
    setAutoFillBackground(true);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 3, 2, 5);


    /// Master pitch

    m_masterPitchGBX = new GroupBox(tr("MASTER PITCH"), this, true, false);
    m_masterPitchGBX->ledButton()->setModel(&it->m_useMasterPitchModel);

    QWidget*     masterPitchPNL = new QWidget(m_masterPitchGBX);
    QHBoxLayout* masterPitchLOT = new QHBoxLayout(masterPitchPNL);
    masterPitchLOT->setContentsMargins(6, 6, 6, 6);
    masterPitchLOT->setSpacing(6);
    m_masterPitchGBX->setContentWidget(masterPitchPNL);

    QLabel* masterPitchHelp
            = new QLabel(tr("Enables the use of Master Pitch"));
    masterPitchLOT->addWidget(masterPitchHelp);
    masterPitchLOT->addStretch(1);


    /// Control Changes

    m_controlChangesGBX
            = new GroupBox(tr("CONTROL CHANGES"), this, false, false);
    QWidget*     controlChangesPNL = new QWidget(m_controlChangesGBX);
    QGridLayout* controlChangesLOT = new QGridLayout(controlChangesPNL);
    controlChangesLOT->setContentsMargins(3,3,3,3);
    controlChangesLOT->setSpacing(3);
    m_controlChangesGBX->setContentWidget(controlChangesPNL);

    m_ccVolumeLCB=new LedCheckBox(m_controlChangesGBX);
    m_ccVolumeLCB->setText(tr("Volume"));
    m_ccVolumeLCB->setModel(m_track->volumeEnabledModel());
    //m_ccVolumeLCB->setEnabled(false);
    controlChangesLOT->addWidget(m_ccVolumeLCB,0,0);

    m_ccPanningLCB=new LedCheckBox(m_controlChangesGBX);
    m_ccPanningLCB->setText(tr("Panning"));
    m_ccPanningLCB->setModel(m_track->panningEnabledModel());
    //m_ccPanningLCB->setEnabled(false);
    controlChangesLOT->addWidget(m_ccPanningLCB,1,0);

    m_ccBendingLCB=new LedCheckBox(m_controlChangesGBX);
    m_ccBendingLCB->setText(tr("Bending"));
    m_ccBendingLCB->setModel(m_track->bendingEnabledModel());
    //m_ccBendingLCB->setEnabled(false);
    controlChangesLOT->addWidget(m_ccBendingLCB,0,1);

    m_ccModulatingLCB=new LedCheckBox(m_controlChangesGBX);
    m_ccModulatingLCB->setText(tr("(TODO) Modulating"));
    //m_ccModulatingLCB->setModel(m_track->modulatingEnabledModel());
    //m_ccModulatingLCB->setEnabled(false);
    controlChangesLOT->addWidget(m_ccModulatingLCB,1,1);


    /// Scale

    m_scaleGBX = new GroupBox(tr("SCALE"), this, false, false);

    QWidget*     ScalePNL = new QWidget(m_scaleGBX);
    QGridLayout* scaleLOT = new QGridLayout(ScalePNL);
    scaleLOT->setContentsMargins(6, 6, 6, 6);
    scaleLOT->setSpacing(6);
    m_scaleGBX->setContentWidget(ScalePNL);

    // add scale combo boxes
    ComboBox* bankCMB = new ComboBox(this);
    bankCMB->setModel(&m_scaleBankModel);
    bankCMB->setMinimumWidth(3 * 27 + 4);

    ComboBox* indexCMB = new ComboBox(this);
    indexCMB->setModel(&m_scaleIndexModel);
    indexCMB->setMinimumWidth(7 * 27 + 12);

    Scale::fillBankModel(m_scaleBankModel);
    Scale::fillIndexModel(m_scaleIndexModel,0);
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

    layout->addWidget(m_controlChangesGBX);
    layout->addWidget(m_masterPitchGBX);
    layout->addWidget(m_scaleGBX);
    layout->addStretch(1);
    setLayout(layout);
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
