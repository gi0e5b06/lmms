/*
 * kicker.h - drum synthesizer
 *
 * Copyright (c) 2006-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2014 grejppi <grejppi/at/gmail.com>
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

#ifndef KICKER_H
#define KICKER_H

#include "ComboBox.h"
#include "Instrument.h"
#include "InstrumentView.h"
#include "Knob.h"
#include "LedCheckBox.h"
#include "TempoSyncKnob.h"

#include <QObject>

#define KICKER_PRESET_VERSION 2

class kickerInstrumentView;
class NotePlayHandle;

class kickerInstrument : public Instrument
{
    Q_OBJECT
  public:
    kickerInstrument(InstrumentTrack* _instrument_track);
    virtual ~kickerInstrument();

    virtual void playNote(NotePlayHandle* _n, sampleFrame* _working_buffer);

    virtual void deleteNotePluginData(NotePlayHandle* _n);

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    //virtual QString nodeName() const;

    virtual Flags flags() const
    {
        return IsNotBendable | IsMonophonic;
    }

    virtual f_cnt_t desiredReleaseFrames() const
    {
        return 512;
    }

    virtual PluginView* instantiateView(QWidget* _parent);

  public slots:
    void updateSinIndexModel();
    void updateWhnIndexModel();

  private:
    FloatModel         m_startFreqModel;
    FloatModel         m_endFreqModel;
    TempoSyncKnobModel m_decayModel;
    FloatModel         m_distModel;
    FloatModel         m_distEndModel;
    FloatModel         m_gainModel;
    FloatModel         m_envModel;
    FloatModel         m_tailModel;
    FloatModel         m_noiseModel;
    FloatModel         m_clickModel;
    FloatModel         m_slopeModel;
    FloatModel         m_phaseFactorModel;
    BoolModel          m_startNoteModel;
    BoolModel          m_endNoteModel;
    IntModel           m_versionModel;
    ComboBoxModel      m_sinBankModel;
    ComboBoxModel      m_sinIndexModel;
    ComboBoxModel      m_whnBankModel;
    ComboBoxModel      m_whnIndexModel;

    friend class kickerInstrumentView;
};

class kickerInstrumentView : public InstrumentView
{
    Q_OBJECT
  public:
    kickerInstrumentView(Instrument* _instrument, QWidget* _parent);
    virtual ~kickerInstrumentView();

  private:
    virtual void modelChanged();

    Knob* m_startFreqKnob;
    Knob* m_endFreqKnob;
    Knob* m_decayKnob;
    Knob* m_distKnob;
    Knob* m_distEndKnob;
    Knob* m_gainKnob;
    Knob* m_envKnob;
    Knob* m_tailKnob;
    Knob* m_noiseKnob;
    Knob* m_clickKnob;
    Knob* m_slopeKnob;
    Knob* m_phaseFactorKnob;

    LedCheckBox* m_startNoteToggle;
    LedCheckBox* m_endNoteToggle;

    ComboBox* m_sinBankCMB;
    ComboBox* m_sinIndexCMB;
    ComboBox* m_whnBankCMB;
    ComboBox* m_whnIndexCMB;
};

#endif
