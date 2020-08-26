/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * InstrumentMidiIOView.h - widget in instrument-track-window for setting
 *                          up MIDI-related stuff
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

#ifndef INSTRUMENT_MIDI_IO_VIEW_H
#define INSTRUMENT_MIDI_IO_VIEW_H

#include "ComboBoxModel.h"
#include "Midi.h"
#include "ModelView.h"

#include <QWidget>

class GroupBox;
class Knob;
class LcdSpinBox;
class LedCheckBox;
class QToolButton;
class LedCheckBox;
class InstrumentTrack;
class MidiProcessorView;
class MidiPortMenu;

class InstrumentMidiIOView : public QWidget, public ModelView
{
    Q_OBJECT

  public:
    InstrumentMidiIOView(InstrumentTrack* _it, QWidget* _parent);
    virtual ~InstrumentMidiIOView();

    InstrumentTrack* model()
    {
        return castModel<InstrumentTrack>();
    }

    const InstrumentTrack* model() const
    {
        return castModel<InstrumentTrack>();
    }

  protected:
    void modelChanged() override;

  private:
    InstrumentTrack* m_track;

    MidiProcessorView* m_midiInProc;
    MidiProcessorView* m_midiOutProc;

    GroupBox*    m_controlChangesGBX;
    LedCheckBox* m_ccVolumeLCB;
    LedCheckBox* m_ccPanningLCB;
    LedCheckBox* m_ccBendingLCB;
    LedCheckBox* m_ccModulatingLCB;

    GroupBox*     m_midiInputGBX;
    LcdSpinBox*   m_inputChannelSpinBox;
    LcdSpinBox*   m_fixedInputVelocitySpinBox;
    LcdSpinBox*   m_transposeInputSpinBox;
    QToolButton*  m_rpBtn;
    MidiPortMenu* m_readablePortsMenu;

    GroupBox*     m_midiOutputGBX;
    LcdSpinBox*   m_outputChannelSpinBox;
    LcdSpinBox*   m_fixedOutputVelocitySpinBox;
    LcdSpinBox*   m_transposeOutputSpinBox;
    LcdSpinBox*   m_outputProgramSpinBox;
    LcdSpinBox*   m_fixedOutputNoteSpinBox;
    QToolButton*  m_wpBtn;
    MidiPortMenu* m_writablePortsMenu;

    GroupBox*   m_baseVelocityGBX;
    LcdSpinBox* m_baseVelocitySpinBox;

    GroupBox* m_allMidiCCGBX;
    Knob*     m_midiCCKNB[MidiControllerCount];
};

class InstrumentMiscView : public QWidget, public ModelView
{
    Q_OBJECT

  public:
    InstrumentMiscView(InstrumentTrack* _it, QWidget* _parent);
    virtual ~InstrumentMiscView();

    InstrumentTrack* model()
    {
        return castModel<InstrumentTrack>();
    }

    const InstrumentTrack* model() const
    {
        return castModel<InstrumentTrack>();
    }

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

    GroupBox* m_masterPitchGBX;

    GroupBox*     m_clefGBX;
    GroupBox*     m_rootModeGBX;
    GroupBox*     m_scaleGBX;
    ComboBoxModel m_scaleBankModel;
    ComboBoxModel m_scaleIndexModel;
};

#endif
