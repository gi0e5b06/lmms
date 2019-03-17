/*
 * InstrumentSoundShaping.h -
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef INSTRUMENT_SOUND_SHAPING_H
#define INSTRUMENT_SOUND_SHAPING_H

#include "BasicFilters.h"
#include "ComboBoxModel.h"
#include "TempoSyncKnobModel.h"

class InstrumentTrack;
class EnvelopeAndLfoParameters;
class NotePlayHandle;
class InstrumentPlayHandle;

class InstrumentSoundShaping : public Model, public JournallingObject
{
    Q_OBJECT

  public:
    InstrumentSoundShaping(InstrumentTrack* _instrument_track);
    virtual ~InstrumentSoundShaping();

    void processAudioBuffer(sampleFrame*    _ab,
                            const fpp_t     _frames,
                            NotePlayHandle* _n);
    void processAudioBuffer(sampleFrame*    _buffer,
                            const fpp_t     _frames,
                            BasicFilters<>* _filter1,
                            BasicFilters<>* _filter2,
                            f_cnt_t         _envTotalFrames,
                            f_cnt_t         _envReleaseBegin,
                            bool            _legato,
                            bool            _marcato,
                            bool            _staccato);

    enum Targets
    {
        Volume,
        Cut,
        Resonance
    };
    constexpr static int NumTargets = Resonance + 1;

    f_cnt_t envFrames(const bool _only_vol = false) const;
    f_cnt_t releaseFrames() const;

    real_t volumeLevel(NotePlayHandle* _n, const f_cnt_t _frame);

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "eldata";
    }

  private:
    EnvelopeAndLfoParameters* m_envLfoParameters[NumTargets];
    InstrumentTrack*          m_instrumentTrack;

    BoolModel          m_filter1EnabledModel;
    ComboBoxModel      m_filter1TypeModel;
    FloatModel         m_filter1CutModel;
    FloatModel         m_filter1ResModel;  // Q, BW
    ComboBoxModel      m_filter1PassesModel;
    FloatModel         m_filter1GainModel;
    FloatModel         m_filter1ResponseModel;
    FloatModel         m_filter1FeedbackAmountModel;
    TempoSyncKnobModel m_filter1FeedbackDelayModel;

    BoolModel          m_filter2EnabledModel;
    ComboBoxModel      m_filter2TypeModel;
    FloatModel         m_filter2CutModel;
    FloatModel         m_filter2ResModel;  // Q, BW
    ComboBoxModel      m_filter2PassesModel;
    FloatModel         m_filter2GainModel;
    FloatModel         m_filter2ResponseModel;
    FloatModel         m_filter2FeedbackAmountModel;
    TempoSyncKnobModel m_filter2FeedbackDelayModel;

    static const QString targetNames[InstrumentSoundShaping::NumTargets][3];

    friend class InstrumentSoundShapingView;
};

#endif
