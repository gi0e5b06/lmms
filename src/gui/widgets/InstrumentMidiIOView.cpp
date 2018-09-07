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

#include "Engine.h"
#include "GroupBox.h"
#include "InstrumentTrack.h"
#include "LcdSpinBox.h"
//#include "LedCheckBox.h"
#include "MidiClient.h"
#include "MidiPortMenu.h"  // REQUIRED
#include "Mixer.h"
//#include "ToolTip.h"

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

    QLabel* baseVelocityHelp
            = new QLabel(tr("Specify the velocity normalization base for "
                            "MIDI-based instruments at 100% note velocity"));
    baseVelocityHelp->setWordWrap(true);
    baseVelocityHelp->setFont(pointSize<8>(baseVelocityHelp->font()));
    baseVelocityLayout->addWidget(baseVelocityHelp);

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
      QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(5);

    m_pitchGBX = new GroupBox(tr("MASTER PITCH"), this, true, false);
    m_pitchGBX->ledButton()->setModel(&it->m_useMasterPitchModel);

    QWidget*     masterPitchPNL    = new QWidget(m_pitchGBX);
    QHBoxLayout* masterPitchLayout = new QHBoxLayout(masterPitchPNL);
    masterPitchLayout->setContentsMargins(6, 6, 6, 6);
    masterPitchLayout->setSpacing(6);
    m_pitchGBX->setContentWidget(masterPitchPNL);

    QLabel* masterPitchHelp
            = new QLabel(tr("Enables the use of Master Pitch"));
    masterPitchLayout->addWidget(masterPitchHelp);
    masterPitchLayout->addStretch(1);

    layout->addWidget(m_pitchGBX);
    layout->addStretch(1);
    setLayout(layout);
}

InstrumentMiscView::~InstrumentMiscView()
{
}
