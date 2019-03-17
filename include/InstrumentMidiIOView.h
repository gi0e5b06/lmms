/*
 * InstrumentMidiIOView.h - widget in instrument-track-window for setting
 *                          up MIDI-related stuff
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
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

#ifndef INSTRUMENT_MIDI_IO_VIEW_H
#define INSTRUMENT_MIDI_IO_VIEW_H

#include "ComboBoxModel.h"
#include "ModelView.h"
#include "Midi.h"

#include <QWidget>

class GroupBox;
class Knob;
class LcdSpinBox;
class LedCheckBox;
class QToolButton;
class LedCheckBox;
class InstrumentTrack;
class MidiProcessorView;

class InstrumentMidiIOView : public QWidget, public ModelView
{
    Q_OBJECT

  public:
    InstrumentMidiIOView(QWidget* parent);
    virtual ~InstrumentMidiIOView();

  private:
    virtual void modelChanged();

    GroupBox*    m_midiInputGBX;
    LcdSpinBox*  m_inputChannelSpinBox;
    LcdSpinBox*  m_fixedInputVelocitySpinBox;
    QToolButton* m_rpBtn;

    GroupBox*    m_midiOutputGBX;
    LcdSpinBox*  m_outputChannelSpinBox;
    LcdSpinBox*  m_fixedOutputVelocitySpinBox;
    LcdSpinBox*  m_outputProgramSpinBox;
    LcdSpinBox*  m_fixedOutputNoteSpinBox;
    QToolButton* m_wpBtn;

    GroupBox*   m_baseVelocityGBX;
    LcdSpinBox* m_baseVelocitySpinBox;
};

class InstrumentMiscView : public QWidget
{
    Q_OBJECT

  public:
    InstrumentMiscView(InstrumentTrack* it, QWidget* parent);
    virtual ~InstrumentMiscView();

    GroupBox* pitchGroupBox()
    {
        return m_masterPitchGBX;
    }

  public slots:
    // void updateScaleBankModel();
    void updateScaleIndexModel();
    void updateScale();

  private:
    InstrumentTrack* m_track;

    MidiProcessorView* m_midiInProc;
    MidiProcessorView* m_midiOutProc;

    GroupBox*    m_controlChangesGBX;
    LedCheckBox* m_ccVolumeLCB;
    LedCheckBox* m_ccPanningLCB;
    LedCheckBox* m_ccBendingLCB;
    LedCheckBox* m_ccModulatingLCB;

    GroupBox* m_allMidiCCGBX;
    Knob*     m_midiCCKNB[MidiControllerCount];

    GroupBox* m_masterPitchGBX;

    GroupBox*     m_scaleGBX;
    ComboBoxModel m_scaleBankModel;
    ComboBoxModel m_scaleIndexModel;
};

#endif
