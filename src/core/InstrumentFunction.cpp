/*
 * InstrumentFunction.cpp - models for instrument-function-tab
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "InstrumentFunction.h"

#include "Engine.h"
#include "InstrumentFunctionView.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "Piano.h"
#include "PresetPreviewPlayHandle.h"
#include "Song.h"
#include "embed.h"

#include <QDomElement>
#include <QMutexLocker>
//#include "lmms_math.h"

#include <time.h>

InstrumentFunction::InstrumentFunction(Model* _parent, QString _name) :
      Model(_parent, _name), m_enabledModel(false, this, tr("Enabled")),
      m_minNoteGenerationModel(0, 0, 9, this, tr("Min")),
      m_maxNoteGenerationModel(0, 0, 9, this, tr("Max"))
{
}

bool InstrumentFunction::shouldProcessNote(NotePlayHandle* n)
{
    if(!n)
        return false;
    if(!m_enabledModel.value())
        return false;
    int gen = n->generation();
    if(gen < m_minNoteGenerationModel.value()
       || gen > m_maxNoteGenerationModel.value())
        return false;
    return true;
}

InstrumentFunctionNoteStacking::ChordTable::Init
        InstrumentFunctionNoteStacking::ChordTable::s_initTable[]
        = {{QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "octave"),
            {0, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Major"),
            {0, 4, 7, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Majb5"),
            {0, 4, 6, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "minor"),
            {0, 3, 7, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "minb5"),
            {0, 3, 6, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "sus2"),
            {0, 2, 7, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "sus4"),
            {0, 5, 7, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "aug"),
            {0, 4, 8, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "augsus4"),
            {0, 5, 8, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "tri"),
            {0, 3, 6, 9, -1}},

           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "6"),
            {0, 4, 7, 9, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "6sus4"),
            {0, 5, 7, 9, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "6add9"),
            {0, 4, 7, 9, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m6"),
            {0, 3, 7, 9, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m6add9"),
            {0, 3, 7, 9, 14, -1}},

           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7"),
            {0, 4, 7, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7sus4"),
            {0, 5, 7, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7#5"),
            {0, 4, 8, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7b5"),
            {0, 4, 6, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7#9"),
            {0, 4, 7, 10, 15, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7b9"),
            {0, 4, 7, 10, 13, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7#5#9"),
            {0, 4, 8, 10, 15, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7#5b9"),
            {0, 4, 8, 10, 13, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7b5b9"),
            {0, 4, 6, 10, 13, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7add11"),
            {0, 4, 7, 10, 17, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7add13"),
            {0, 4, 7, 10, 21, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "7#11"),
            {0, 4, 7, 10, 18, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj7"),
            {0, 4, 7, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj7b5"),
            {0, 4, 6, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj7#5"),
            {0, 4, 8, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj7#11"),
            {0, 4, 7, 11, 18, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj7add13"),
            {0, 4, 7, 11, 21, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m7"),
            {0, 3, 7, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m7b5"),
            {0, 3, 6, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m7b9"),
            {0, 3, 7, 10, 13, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m7add11"),
            {0, 3, 7, 10, 17, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m7add13"),
            {0, 3, 7, 10, 21, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m-Maj7"),
            {0, 3, 7, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "m-Maj7add11"),
            {0, 3, 7, 11, 17, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "m-Maj7add13"),
            {0, 3, 7, 11, 21, -1}},

           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "9"),
            {0, 4, 7, 10, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "9sus4"),
            {0, 5, 7, 10, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "add9"),
            {0, 4, 7, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "9#5"),
            {0, 4, 8, 10, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "9b5"),
            {0, 4, 6, 10, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "9#11"),
            {0, 4, 7, 10, 14, 18, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "9b13"),
            {0, 4, 7, 10, 14, 20, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj9"),
            {0, 4, 7, 11, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj9sus4"),
            {0, 5, 7, 11, 15, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj9#5"),
            {0, 4, 8, 11, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj9#11"),
            {0, 4, 7, 11, 14, 18, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m9"),
            {0, 3, 7, 10, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "madd9"),
            {0, 3, 7, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m9b5"),
            {0, 3, 6, 10, 14, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m9-Maj7"),
            {0, 3, 7, 11, 14, -1}},

           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "11"),
            {0, 4, 7, 10, 14, 17, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "11b9"),
            {0, 4, 7, 10, 13, 17, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj11"),
            {0, 4, 7, 11, 14, 17, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m11"),
            {0, 3, 7, 10, 14, 17, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m-Maj11"),
            {0, 3, 7, 11, 14, 17, -1}},

           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "13"),
            {0, 4, 7, 10, 14, 21, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "13#9"),
            {0, 4, 7, 10, 15, 21, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "13b9"),
            {0, 4, 7, 10, 13, 21, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "13b5b9"),
            {0, 4, 6, 10, 13, 21, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Maj13"),
            {0, 4, 7, 11, 14, 21, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m13"),
            {0, 3, 7, 10, 14, 21, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "m-Maj13"),
            {0, 3, 7, 11, 14, 21, -1}},

           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Major"),
            {0, 2, 4, 5, 7, 9, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Harmonic minor"),
            {0, 2, 3, 5, 7, 8, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Melodic minor"),
            {0, 2, 3, 5, 7, 9, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Whole tone"),
            {0, 2, 4, 6, 8, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Diminished"),
            {0, 2, 3, 5, 6, 8, 9, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Major pentatonic"),
            {0, 2, 4, 7, 9, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Minor pentatonic"),
            {0, 3, 5, 7, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Jap in sen"),
            {0, 1, 5, 7, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Major bebop"),
            {0, 2, 4, 5, 7, 8, 9, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Dominant bebop"),
            {0, 2, 4, 5, 7, 9, 10, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Blues"),
            {0, 3, 5, 6, 7, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Arabic"),
            {0, 1, 4, 5, 7, 8, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Enigmatic"),
            {0, 1, 4, 6, 8, 10, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Neopolitan"),
            {0, 1, 3, 5, 7, 9, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Neopolitan minor"),
            {0, 1, 3, 5, 7, 8, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Hungarian minor"),
            {0, 2, 3, 6, 7, 8, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Dorian"),
            {0, 2, 3, 5, 7, 9, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Phrygian"),
            {0, 1, 3, 5, 7, 8, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Lydian"),
            {0, 2, 4, 6, 7, 9, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Mixolydian"),
            {0, 2, 4, 5, 7, 9, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Aeolian"),
            {0, 2, 3, 5, 7, 8, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Locrian"),
            {0, 1, 3, 5, 6, 8, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Minor"),
            {0, 2, 3, 5, 7, 8, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Chromatic"),
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Half-Whole Diminished"),
            {0, 1, 3, 4, 6, 7, 9, 10, -1}},

           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "5"),
            {0, 7, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Phrygian dominant"),
            {0, 1, 4, 5, 7, 8, 10, -1}},
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking", "Persian"),
            {0, 1, 4, 5, 6, 8, 11, -1}}};

InstrumentFunctionNoteStacking::Chord::Chord(
        const char* _name, const ChordSemitones& _semitones) :
      m_name(InstrumentFunctionNoteStacking::tr(_name))
{
    for(m_size = 0; m_size < MAX_CHORD_POLYPHONY; m_size++)
    {
        if(_semitones[m_size] == -1)
            break;

        m_semitones[m_size] = _semitones[m_size];
    }
}

bool InstrumentFunctionNoteStacking::Chord::hasSemitone(
        int8_t _semitone) const
{
    for(int i = 0; i < size(); ++i)
        if(_semitone == m_semitones[i])
            return true;

    return false;
}

InstrumentFunctionNoteStacking::ChordTable::ChordTable() : QVector<Chord>()
{
    for(int i = 0;
        i < static_cast<int>(sizeof s_initTable / sizeof *s_initTable); i++)
    {
        push_back(Chord(s_initTable[i].m_name, s_initTable[i].m_semitones));
    }
}

const InstrumentFunctionNoteStacking::Chord&
        InstrumentFunctionNoteStacking::ChordTable::getByName(
                const QString& name, bool is_scale) const
{
    for(int i = 0; i < size(); i++)
    {
        if(at(i).getName() == name && is_scale == at(i).isScale())
            return at(i);
    }

    static Chord empty;
    return empty;
}

InstrumentFunctionNoteStacking::InstrumentFunctionNoteStacking(
        Model* _parent) :
      InstrumentFunction(_parent, tr("Chords")),
      // m_enabledModel( false, this ),
      m_chordsModel(this, tr("Chord type")),
      m_chordRangeModel(1., 1., 9., 1., this, tr("Chord range"))
{
    const ChordTable& chord_table = ChordTable::getInstance();
    for(int i = 0; i < chord_table.size(); ++i)
    {
        m_chordsModel.addItem(chord_table[i].getName());
    }
}

InstrumentFunctionNoteStacking::~InstrumentFunctionNoteStacking()
{
}

bool InstrumentFunctionNoteStacking::processNote(NotePlayHandle* _n)
{
    if(!shouldProcessNote(_n))
        return true;
    // qInfo("InstrumentFunctionNoteStacking::processNote n.key=%d in
    // g=%d",_n->key(),_n->generation());
    if(_n->totalFramesPlayed() != 0 || _n->isReleased())
        return true;
    // qInfo("InstrumentFunctionNoteStacking::processNote n.key=%d OK
    // g=%d",_n->key(),_n->generation());
    if(_n->generation() >= 9)
        return true;

    const int         base_note_key = _n->key();
    const ChordTable& chord_table   = ChordTable::getInstance();
    // we add chord-subnotes to note if either note is a base-note and
    // arpeggio is not used or note is part of an arpeggio
    // at the same time we only add sub-notes if nothing of the note was
    // played yet, because otherwise we would add chord-subnotes every
    // time an audio-buffer is rendered...
    // if(  _n->origin() == NotePlayHandle::OriginArpeggio ||
    //   ( _n->hasParent() == false &&
    //   _n->instrumentTrack()->isArpeggioEnabled() == false ) )
    {
        // then insert sub-notes for chord
        const int selected_chord = m_chordsModel.value();

        for(int octave_cnt = 0; octave_cnt < m_chordRangeModel.value();
            ++octave_cnt)
        {
            const int subnote_key_base
                    = base_note_key + octave_cnt * KeysPerOctave;

            // process all notes in the chord
            for(int i = 0; i < chord_table[selected_chord].size(); ++i)
            {
                // add interval to sub-note-key
                const int subnote_key = subnote_key_base
                                        + (int)chord_table[selected_chord][i];
                // maybe we're out of range -> let's get outta here!
                // range-checking
                if(subnote_key >= NumKeys || subnote_key < 0
                   || Engine::mixer()->criticalXRuns() || _n->isTrackMuted())
                {
                    // qInfo("InstrumentFunctionNoteStacking::processNote
                    // subnote break");
                    continue;
                }

                // if(MM_SAFE(NotePlayHandle,1))
                {

                    // create copy of base-note
                    Note subnote(_n->length(), 0, subnote_key,
                                 _n->getVolume(), _n->getPanning(),
                                 _n->detuning());

                    // create sub-note-play-handle, only note is
                    // different
                    NotePlayHandle* nph = NotePlayHandleManager::acquire(
                            _n->instrumentTrack(), _n->offset(), _n->frames(),
                            subnote, _n, -1,
                            NotePlayHandle::OriginNoteStacking,
                            _n->generation() + 1);
                    Engine::mixer()->emit playHandleToAdd(nph);
                }
            }
        }
    }

    return true;
}

void InstrumentFunctionNoteStacking::saveSettings(QDomDocument& _doc,
                                                  QDomElement&  _this)
{
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");
    m_enabledModel.saveSettings(_doc, _this, "chord-enabled");
    m_chordsModel.saveSettings(_doc, _this, "chord");
    m_chordRangeModel.saveSettings(_doc, _this, "chordrange");
}

void InstrumentFunctionNoteStacking::loadSettings(const QDomElement& _this)
{
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");
    m_enabledModel.loadSettings(_this, "chord-enabled");
    m_chordsModel.loadSettings(_this, "chord");
    m_chordRangeModel.loadSettings(_this, "chordrange");
}

InstrumentFunctionView* InstrumentFunctionNoteStacking::createView()
{
    return new InstrumentFunctionNoteStackingView(this);
}

InstrumentFunctionArpeggio::InstrumentFunctionArpeggio(Model* _parent) :
      InstrumentFunction(_parent, tr("Arpeggio")),
      // m_enabledModel( false ),
      m_arpModel(this, tr("Arpeggio type")),
      m_arpRangeModel(1., 1., 9., 1., this, tr("Arpeggio range")),
      m_arpCycleModel(0., 0., 11., 1., this, tr("Cycle steps")),
      m_arpSkipModel(0., 0., 100., 1., this, tr("Skip rate")),
      m_arpMissModel(0., 0., 100., 1., this, tr("Miss rate")),
      m_arpTimeModel(200., 25., 2000., 1., 2000, this, tr("Arpeggio time")),
      m_arpGateModel(100., 1., 200., 1., this, tr("Arpeggio gate")),
      m_arpDirectionModel(this, tr("Arpeggio direction")),
      m_arpModeModel(this, tr("Arpeggio mode")),
      m_arpBaseModel(0., 0., 11., 1., this, tr("Base")),
      m_arpRepeatModel(0., 0., 15., 1., this, tr("Repeat"))

{
    const InstrumentFunctionNoteStacking::ChordTable& chord_table
            = InstrumentFunctionNoteStacking::ChordTable::getInstance();
    for(int i = 0; i < chord_table.size(); ++i)
    {
        m_arpModel.addItem(chord_table[i].getName());
    }

    m_arpDirectionModel.addItem(tr("Up"), new PixmapLoader("arp_up"));
    m_arpDirectionModel.addItem(tr("Down"), new PixmapLoader("arp_down"));
    m_arpDirectionModel.addItem(tr("Up and down"),
                                new PixmapLoader("arp_up_and_down"));
    m_arpDirectionModel.addItem(tr("Down and up"),
                                new PixmapLoader("arp_up_and_down"));
    m_arpDirectionModel.addItem(tr("Random"), new PixmapLoader("arp_random"));
    m_arpDirectionModel.setInitValue(ArpDirUp);

    m_arpModeModel.addItem(tr("Free"), new PixmapLoader("arp_free"));
    m_arpModeModel.addItem(tr("Sort"), new PixmapLoader("arp_sort"));
    m_arpModeModel.addItem(tr("Sync"), new PixmapLoader("arp_sync"));
}

InstrumentFunctionArpeggio::~InstrumentFunctionArpeggio()
{
}

bool InstrumentFunctionArpeggio::processNote(NotePlayHandle* _n)
{
    if(!shouldProcessNote(_n))
        return true;

    // if( _n->origin() == NotePlayHandle::OriginArpeggio ||
    //    _n->origin() == NotePlayHandle::OriginNoteStacking ||
    //    (_n->isReleased()
    //     && _n->releaseFramesDone() >= _n->actualReleaseFramesToDo()))

    if(_n->isReleased())
        return true;

    if(_n->generation() >= 9)
        return true;

    // qInfo("InstrumentFunctionArpeggio::processNote n.key=%d in
    // g=%d",_n->key(),_n->generation()); if(_n->totalFramesPlayed()!=0 /*||
    // _n->isReleased()*/ ) return true;
    // qInfo("InstrumentFunctionArpeggio::processNote n.key=%d OK
    // g=%d",_n->key(),_n->generation());

    const int base_note_key = _n->key();
    const int selected_arp  = m_arpModel.value();

    ConstNotePlayHandleList cnphv
            //= NotePlayHandle::nphsOfInstrumentTrack(_n->instrumentTrack());
            = Engine::mixer()->nphsOfTrack(_n->instrumentTrack());

    if(m_arpModeModel.value() != FreeMode && cnphv.size() == 0)
    {
        // maybe we're playing only a preset-preview-note?
        cnphv = PresetPreviewPlayHandle::nphsOfInstrumentTrack(
                _n->instrumentTrack());
        if(cnphv.size() == 0)
        {
            // still nothing found here, so lets return
            // return;
            cnphv.append(_n);
        }
    }

    const InstrumentFunctionNoteStacking::ChordTable& chord_table
            = InstrumentFunctionNoteStacking::ChordTable::getInstance();
    const int cur_chord_size = chord_table[selected_arp].size();
    const int range       = (int)(cur_chord_size * m_arpRangeModel.value());
    const int total_range = range * cnphv.size();

    // number of frames that every note should be played
    const f_cnt_t arp_frames
            = (f_cnt_t)(m_arpTimeModel.value() / 1000.
                        * Engine::mixer()->processingSampleRate());
    const f_cnt_t gated_frames
            = (f_cnt_t)(m_arpGateModel.value() * arp_frames / 100.);

    // used for calculating remaining frames for arp-note, we have to add
    // arp_frames-1, otherwise the first arp-note will not be setup
    // correctly... -> arp_frames frames silence at the start of every note!
    int cur_frame = ((m_arpModeModel.value() != FreeMode)
                             ? cnphv.first()->totalFramesPlayed()
                             : _n->totalFramesPlayed())
                    + arp_frames - 1;
    // int cur_frame = _n->totalFramesPlayed() + arp_frames - 1;

    // used for loop
    f_cnt_t frames_processed = (m_arpModeModel.value() != FreeMode)
                                       ? cnphv.first()->noteOffset()
                                       : _n->noteOffset();
    // f_cnt_t frames_processed = _n->noteOffset();

    while(frames_processed < Engine::mixer()->framesPerPeriod())
    {
        const f_cnt_t remaining_frames_for_cur_arp
                = arp_frames - (cur_frame % arp_frames);
        // does current arp-note fill whole audio-buffer?
        if(remaining_frames_for_cur_arp > Engine::mixer()->framesPerPeriod())
        {
            // then we don't have to do something!
            break;
        }

        frames_processed += remaining_frames_for_cur_arp;

        // in sorted mode: is it our turn or do we have to be quiet for
        // now?
        if(m_arpModeModel.value() == SortMode
           && ((cur_frame / arp_frames) % total_range) / range
                      != cnphv.indexOf(_n))
        //(f_cnt_t)_n->index()) // <-- suspicious
        {
            // Set master note if not playing arp note or it will play as an
            // ordinary note
            // if(_n->generation() == 0)
            //   _n->setMasterNote();
            // update counters
            frames_processed += arp_frames;
            cur_frame += arp_frames;
            continue;
        }

        // Skip notes randomly
        if(m_arpSkipModel.value())
        {
            if(100. * fastrand01exc() < m_arpSkipModel.value())
            // real_t(rand()) / real_t(RAND_MAX + 1.)
            {
                // Set master note to prevent the note to extend over skipped
                // notes This may only be needed for lb302
                // if(_n->generation() == 0)
                //    _n->setMasterNote();
                // update counters
                frames_processed += arp_frames;
                cur_frame += arp_frames;
                continue;
            }
        }

        int dir = m_arpDirectionModel.value();

        // Miss notes randomly. We intercept int dir and abuse it
        // after need.  :)

        if(m_arpMissModel.value())
        {
            if(100. * fastrand01exc() < m_arpMissModel.value())
            {
                dir = ArpDirRandom;
            }
        }

        int armv        = m_arpRepeatModel.value() + 1;
        int cur_arp_idx = 0;
        // process according to arpeggio-direction...
        if(dir == ArpDirUp)
        {
            cur_arp_idx = (cur_frame / armv / arp_frames) % range;
        }
        else if(dir == ArpDirDown)
        {
            cur_arp_idx = range - (cur_frame / armv / arp_frames) % range - 1;
        }
        else if(dir == ArpDirUpAndDown && range > 1)
        {
            // imagine, we had to play the arp once up and then
            // once down -> makes 2 * range possible notes...
            // because we don't play the lower and upper notes
            // twice, we have to subtract 2
            cur_arp_idx = (cur_frame / armv / arp_frames) % (range * 2 - 2);
            // if greater than range, we have to play down...
            // looks like the code for arp_dir==DOWN... :)
            if(cur_arp_idx >= range)
            {
                cur_arp_idx = range - cur_arp_idx % (range - 1) - 1;
            }
        }
        else if(dir == ArpDirDownAndUp && range > 1)
        {
            // copied from ArpDirUpAndDown above
            cur_arp_idx = (cur_frame / armv / arp_frames) % (range * 2 - 2);
            // if greater than range, we have to play down...
            // looks like the code for arp_dir==DOWN... :)
            if(cur_arp_idx >= range)
            {
                cur_arp_idx = range - cur_arp_idx % (range - 1) - 1;
            }
            // inverts direction
            cur_arp_idx = range - cur_arp_idx - 1;
        }
        else if(dir == ArpDirRandom)
        {
            // just pick a random chord-index
            cur_arp_idx = int(round(range * fastrand01inc()));
        }

        int key    = cur_arp_idx % range;
        int octave = key / cur_chord_size;

        key %= cur_chord_size;
        // key *= cur_chord_size;
        // key %= KeysPerOctave;

        // Cycle, Base, Repeat
        int acmv = m_arpCycleModel.value();
        int abmv = m_arpBaseModel.value();
        // if(dir != ArpDirRandom)
        // if(armv > 0)
        //    key /= armv + 1;
        if(acmv > 0)
            key *= acmv + 1;
        key += abmv;
        key %= cur_chord_size;

        // now calculate final key for our arp-note
        const int subnote_key = base_note_key + octave * KeysPerOctave
                                + chord_table[selected_arp][key];

        // range-checking
        if(subnote_key >= NumKeys || subnote_key < 0
           || Engine::mixer()->criticalXRuns() || _n->isTrackMuted())
        {
            // qInfo("InstrumentFunctionArpeggio::processNote subnote
            // break");
            continue;
        }

        // create new arp-note

        // if(MM_SAFE(NotePlayHandle,1))
        {
            // Note subnote( _n->length(), 0, subnote_key,
            // _n->getVolume(), _n->getPanning(), _n->detuning() );
            Note subnote(/*MidiTime( 0 ), MidiTime( 0 ),*/
                         _n->length(), 0, subnote_key, _n->getVolume(),
                         _n->getPanning(), _n->detuning());

            // create sub-note-play-handle, only ptr to note is different
            // and is_arp_note=true
            NotePlayHandle* nph = NotePlayHandleManager::acquire(
                    _n->instrumentTrack(), frames_processed, gated_frames,
                    subnote, _n, -1, NotePlayHandle::OriginArpeggio,
                    _n->generation() + 1);
            Engine::mixer()->emit playHandleToAdd(nph);
        }

        // update counters
        frames_processed += arp_frames;
        cur_frame += arp_frames;
    }

    // make sure note is handled as arp-base-note, even
    // if we didn't add a sub-note so far
    // if(m_arpModeModel.value() != FreeMode)
    {
        if(_n->generation() == 0)
            _n->setMasterNote();
    }

    return true;
}

void InstrumentFunctionArpeggio::saveSettings(QDomDocument& _doc,
                                              QDomElement&  _this)
{
    m_enabledModel.saveSettings(_doc, _this, "arp-enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");
    m_arpModel.saveSettings(_doc, _this, "arp");
    m_arpRangeModel.saveSettings(_doc, _this, "arprange");
    m_arpCycleModel.saveSettings(_doc, _this, "arpcycle");
    m_arpSkipModel.saveSettings(_doc, _this, "arpskip");
    m_arpMissModel.saveSettings(_doc, _this, "arpmiss");
    m_arpTimeModel.saveSettings(_doc, _this, "arptime");
    m_arpGateModel.saveSettings(_doc, _this, "arpgate");
    m_arpDirectionModel.saveSettings(_doc, _this, "arpdir");
    m_arpModeModel.saveSettings(_doc, _this, "arpmode");
    m_arpBaseModel.saveSettings(_doc, _this, "base");
    m_arpRepeatModel.saveSettings(_doc, _this, "repeat");
}

void InstrumentFunctionArpeggio::loadSettings(const QDomElement& _this)
{
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");
    m_enabledModel.loadSettings(_this, "arp-enabled");
    m_arpModel.loadSettings(_this, "arp");
    m_arpRangeModel.loadSettings(_this, "arprange");
    m_arpCycleModel.loadSettings(_this, "arpcycle");
    m_arpSkipModel.loadSettings(_this, "arpskip");
    m_arpMissModel.loadSettings(_this, "arpmiss");
    m_arpTimeModel.loadSettings(_this, "arptime");
    m_arpGateModel.loadSettings(_this, "arpgate");
    m_arpDirectionModel.loadSettings(_this, "arpdir");
    /*
            // Keep compatibility with version 0.2.1 file format
            if( _this.hasAttribute( "arpsyncmode" ) )
            {
                    m_arpTimeKnob->setSyncMode(
                    ( tempoSyncKnob::tempoSyncMode ) _this.attribute(
                                                     "arpsyncmode"
       ).toInt()
       );
            }*/

    m_arpModeModel.loadSettings(_this, "arpmode");
    m_arpBaseModel.loadSettings(_this, "base");
    m_arpRepeatModel.loadSettings(_this, "repeat");
}

InstrumentFunctionView* InstrumentFunctionArpeggio::createView()
{
    return new InstrumentFunctionArpeggioView(this);
}

InstrumentFunctionNoteHumanizing::InstrumentFunctionNoteHumanizing(
        Model* _parent) :
      InstrumentFunction(_parent, tr("NoteHumanizing")),
      // m_enabledModel( false, this ),
      m_volumeRangeModel(0., 0., 100., 0.1, this, tr("Volume change")),
      m_panRangeModel(0., 0., 100., 0.1, this, tr("Pan change")),
      m_tuneRangeModel(0., 0., 100., 0.1, this, tr("Frequency change")),
      m_offsetRangeModel(0., 0., 100., 0.1, this, tr("Start delay")),
      m_shortenRangeModel(0., 0., 100., 0.1, this, tr("Shortening")),
      m_lengthenRangeModel(0., 0., 100., 0.1, this, tr("Lenghtening"))
{
}

InstrumentFunctionNoteHumanizing::~InstrumentFunctionNoteHumanizing()
{
}

bool InstrumentFunctionNoteHumanizing::processNote(NotePlayHandle* _n)
{
    if(!shouldProcessNote(_n))
        return true;
    if(_n->totalFramesPlayed() != 0 || _n->isReleased())
        return true;

    {
        real_t l = m_volumeRangeModel.value() / 100.;
        if(l > 0.)
        {
            real_t   r = fastrand01inc();
            volume_t o = _n->getVolume();  // 0..200
            volume_t n = bound(0., qRound(o * (1. - l * r)), 200.);
            // qInfo("NH: volume %f->%f",o,n);
            _n->setVolume(n);
        }
    }

    {
        real_t l = m_panRangeModel.value() / 100.;
        if(l > 0.)
        {
            real_t    r = fastrand01inc();
            panning_t o = _n->getPanning();  // -100..100
            panning_t n = bound(-100., round(o + 200. * l * (r - 0.5)), 100.);
            // qInfo("NH: panning %f->%f",o,n);
            _n->setPanning(n);
        }
    }

    {
        real_t l = m_tuneRangeModel.value() / 100.;
        if(l > 0.)
        {
            real_t r = fastrand01inc();
            real_t n = 12. * (l * (r - 0.5));
            // qInfo("NH: detune %f->%f", o, o + n);
            _n->addEffectDetune(n);
            //_n->setFrequencyUpdate();
        }
    }

    {
        real_t l = m_offsetRangeModel.value() / 100.;
        if(l > 0.)
        {
            real_t      r   = fastrand01inc();
            f_cnt_t     o   = _n->offset();  // ?
            const fpp_t fpt = Engine::framesPerTick()
                              * Engine::getSong()->ticksPerTact();
            const fpp_t fpp = Engine::mixer()->framesPerPeriod();
            f_cnt_t     n   = qRound(o + (fpt - 1 - o) * l * r);
            n = qBound(o, n, fpp - 1);  // tmp: must be inside the period
            // qInfo("NH: offset %d->%d",o,n);
            _n->setOffset(n);
        }
    }

    {
        real_t l = m_shortenRangeModel.value() / 100.;
        if(l > 0.)
        {
            real_t  r = fastrand01inc();
            f_cnt_t o = _n->frames();  // ?
            f_cnt_t n = qBound(1, qRound(o * (1. - l * r)), o);
            n         = qBound(1, n, o);
            // qInfo("NH: shorten %d->%d",o,n);
            _n->setFrames(n);
        }
    }

    {
        real_t l = m_lengthenRangeModel.value() / 100.;
        if(l > 0.)
        {
            real_t  r = fastrand01inc();
            f_cnt_t o = _n->frames();  // ?
            f_cnt_t n = qMax(1, qRound(o * (1. + l * r)));
            n         = qBound(o, n, 2 * o);
            // qInfo("NH: lengthen %d->%d",o,n);
            _n->setFrames(n);
        }
    }

    // qInfo("InstrumentFunctionNoteHumanizing::process Note");
    return true;
}

void InstrumentFunctionNoteHumanizing::saveSettings(QDomDocument& _doc,
                                                    QDomElement&  _this)
{
    m_enabledModel.saveSettings(_doc, _this, "enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");

    m_volumeRangeModel.saveSettings(_doc, _this, "volume");
    m_panRangeModel.saveSettings(_doc, _this, "pan");
    m_tuneRangeModel.saveSettings(_doc, _this, "tune");
    m_offsetRangeModel.saveSettings(_doc, _this, "offset");
    m_shortenRangeModel.saveSettings(_doc, _this, "shorten");
    m_lengthenRangeModel.saveSettings(_doc, _this, "lengthen");
}

void InstrumentFunctionNoteHumanizing::loadSettings(const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "enabled");
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");

    m_volumeRangeModel.loadSettings(_this, "volume");
    m_panRangeModel.loadSettings(_this, "pan");
    m_tuneRangeModel.loadSettings(_this, "tune");
    m_offsetRangeModel.loadSettings(_this, "offset");
    m_shortenRangeModel.loadSettings(_this, "shorten");
    m_lengthenRangeModel.loadSettings(_this, "lengthen");
}

InstrumentFunctionView* InstrumentFunctionNoteHumanizing::createView()
{
    return new InstrumentFunctionNoteHumanizingView(this);
}

InstrumentFunctionNoteDuplicatesRemoving::
        InstrumentFunctionNoteDuplicatesRemoving(Model* _parent) :
      InstrumentFunction(_parent, tr("NoteDuplicatesRemoving"))
{
}

InstrumentFunctionNoteDuplicatesRemoving::
        ~InstrumentFunctionNoteDuplicatesRemoving()
{
}

bool InstrumentFunctionNoteDuplicatesRemoving::processNote(NotePlayHandle* _n)
{
    if(!shouldProcessNote(_n))
        return true;

    // if( _n->totalFramesPlayed()==0 && _n->playedFrames()!=0 )
    //    qInfo("DuplicatesRemoving::processNote tpf=%d
    //    pf=%d",_n->totalFramesPlayed(),_n->playedFrames());

    if(_n->totalFramesPlayed() != 0 || _n->isReleased())
        return true;

    QMutexLocker locker(&m_mutex);
    // const real_t k=Engine::getSong()->getPlayPos().absoluteFrame();
    const int64_t k = (1000 * clock() / CLOCKS_PER_SEC);  // ms
    // qInfo("InstrumentFunctionNoteDuplicatesRemoving: cache k=%d v=%d
    // in=%d",
    //  k,_n->key(),m_cache.contains(k,_n->key()));

    // const real_t fpt=Engine::framesPerTick();
    int i = 0;
    for(const int64_t ck: m_cache)
    {
        if(ck + 150 < k)  // || ck>=k+150)
        {
            m_cache.remove(ck);
            i++;
            if(i > 8)
                break;
        }
    }

    if(m_cache.contains(k, _n->key()))
    {
        // qInfo("InstrumentFunctionNoteDuplicatesRemoving: HIT CACHE");
        _n->noteOff();  //???? kicker
        return false;
    }

    // qInfo("NoteDuplicatesRemoving: cache size=%d",m_cache.size());
    m_cache.insert(k, _n->key());
    return true;
}

void InstrumentFunctionNoteDuplicatesRemoving::saveSettings(
        QDomDocument& _doc, QDomElement& _this)
{
    m_enabledModel.saveSettings(_doc, _this, "enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");
}

void InstrumentFunctionNoteDuplicatesRemoving::loadSettings(
        const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "enabled");
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");
}

InstrumentFunctionView* InstrumentFunctionNoteDuplicatesRemoving::createView()
{
    return new InstrumentFunctionNoteDuplicatesRemovingView(this);
}

InstrumentFunctionNoteFiltering::InstrumentFunctionNoteFiltering(
        Model* _parent) :
      InstrumentFunction(_parent, tr("NoteFiltering")),
      // m_enabledModel( false, this ),
      m_configModel(this, tr("Filtering set"))
{
    for(int j = 0; j < 4; j++)
    {
        m_configModel.addItem(QString("%1").arg(j));
        m_actionModel[j] = new ComboBoxModel(this, tr("Action"));
        m_actionModel[j]->addItem(tr("Skip"));
        m_actionModel[j]->addItem(tr("Up"));
        m_actionModel[j]->addItem(tr("Down"));
        for(int i = 0; i < 12; ++i)
            m_noteSelectionModel[j][i]
                    = new BoolModel(true, this, Note::findKeyName(i));
    }
}

InstrumentFunctionNoteFiltering::~InstrumentFunctionNoteFiltering()
{
    for(int j = 0; j < 4; j++)
    {
        delete m_actionModel[j];
        for(int i = 0; i < 12; ++i)
            delete m_noteSelectionModel[j][i];
    }
}

bool InstrumentFunctionNoteFiltering::processNote(NotePlayHandle* _n)
{
    if(!shouldProcessNote(_n))
        return true;
    if(_n->totalFramesPlayed() != 0 || _n->isReleased())
        return true;

    const int j = m_configModel.value();

    {
        const int  k = _n->key() % 12;
        const bool v = m_noteSelectionModel[j][k]->value();
        // qInfo("InstrumentFunctionNoteFiltering::processNote k=%d
        // v=%d",k,v);
        if(v)
            return true;
    }

    const int action = m_actionModel[j]->value();

    if(action == 1)  // Up
    {
        for(int i = _n->key() + 1; i <= 127; i++)
        {
            const int  k = i % 12;
            const bool v = m_noteSelectionModel[j][k]->value();
            // qInfo("InstrumentFunctionNoteFiltering::processNote k=%d
            // v=%d",k,v);
            if(v)
            {
                _n->setKey(i);
                return true;
            }
        }
    }
    else if(action == 2)  // Down
    {
        for(int i = _n->key() - 1; i >= 0; i--)
        {
            const int  k = i % 12;
            const bool v = m_noteSelectionModel[j][k]->value();
            // qInfo("InstrumentFunctionNoteFiltering::processNote k=%d
            // v=%d",k,v);
            if(v)
            {
                _n->setKey(i);
                return true;
            }
        }
    }
    else
    {
        _n->setMasterNote();
    }

    return false;
}

void InstrumentFunctionNoteFiltering::saveSettings(QDomDocument& _doc,
                                                   QDomElement&  _this)
{
    m_enabledModel.saveSettings(_doc, _this, "enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");
    m_configModel.saveSettings(_doc, _this, "config");

    for(int j = 0; j < 4; j++)
    {
        m_actionModel[j]->saveSettings(_doc, _this, QString("c%1").arg(j));
        for(int i = 0; i < 12; ++i)
            m_noteSelectionModel[j][i]->saveSettings(
                    _doc, _this, QString("c%1n%2").arg(j).arg(i));
    }
}

void InstrumentFunctionNoteFiltering::loadSettings(const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "enabled");
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");
    m_configModel.loadSettings(_this, "config");

    for(int j = 0; j < 4; j++)
    {
        m_actionModel[j]->loadSettings(_this, QString("c%1").arg(j));
        for(int i = 0; i < 12; ++i)
            m_noteSelectionModel[j][i]->loadSettings(
                    _this, QString("c%1n%2").arg(j).arg(i));
    }
}

InstrumentFunctionView* InstrumentFunctionNoteFiltering::createView()
{
    return new InstrumentFunctionNoteFilteringView(this);
}

InstrumentFunctionNoteKeying::InstrumentFunctionNoteKeying(Model* _parent) :
      InstrumentFunction(_parent, tr("NoteKeying")),
      // m_enabledModel( false, this ),
      m_volumeRangeModel(0., -500., 500., 1., this, tr("Volume change")),
      m_volumeBaseModel(0., 0., 127., 1., this, tr("Volume base key")),
      m_volumeMinModel(0., 0., 200., 0.1, this, tr("Volume min")),
      m_volumeMaxModel(100., 0., 200., 0.1, this, tr("Volume max")),
      m_panRangeModel(0., -500., 500., 1., this, tr("Pan change")),
      m_panBaseModel(0., 0., 127., 1., this, tr("Pan base key")),
      m_panMinModel(0., -100., 100., 0.1, this, tr("Pan min")),
      m_panMaxModel(0., -100., 100., 0.1, this, tr("Pan max"))
{
}

InstrumentFunctionNoteKeying::~InstrumentFunctionNoteKeying()
{
}

bool InstrumentFunctionNoteKeying::processNote(NotePlayHandle* _n)
{
    if(!shouldProcessNote(_n))
        return true;
    if(_n->totalFramesPlayed() != 0 || _n->isReleased())
        return true;

    {
        real_t l = m_volumeRangeModel.value() / 100.;
        if(l != 0.)
        {
            real_t   r = bound(-1.,
                             (real_t(_n->key()) - m_volumeBaseModel.value())
                                     * 2. / NumKeys,
                             1.);
            volume_t o = _n->getVolume();  // 0..200
            volume_t n = qBound<volume_t>(m_volumeMinModel.value(),
                                          o + 100. * l * r,
                                          m_volumeMaxModel.value());
            n          = qBound<volume_t>(0., n, 200.);
            qInfo("NK: volume %f->%f", o, n);
            _n->setVolume(n);
        }
    }

    {
        real_t l = m_panRangeModel.value() / 100.;
        if(l != 0.)
        {
            real_t    r = bound(-1.,
                             (real_t(_n->key()) - m_panBaseModel.value()) * 2.
                                     / NumKeys,
                             1.);
            panning_t o = _n->getPanning();  // -100..100
            panning_t n = qBound<panning_t>(m_panMinModel.value(),
                                            o + 200. * l * r,
                                            m_panMaxModel.value());
            n           = qBound<volume_t>(-100., n, 100.);
            qInfo("NK: panning %f->%f", o, n);
            _n->setPanning(n);
        }
    }

    // qInfo("InstrumentFunctionNoteKeying::process Note");
    return true;
}

void InstrumentFunctionNoteKeying::saveSettings(QDomDocument& _doc,
                                                QDomElement&  _this)
{
    m_enabledModel.saveSettings(_doc, _this, "enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");
    m_volumeRangeModel.saveSettings(_doc, _this, "voldelta");
    m_volumeBaseModel.saveSettings(_doc, _this, "volbase");
    m_volumeMinModel.saveSettings(_doc, _this, "volmin");
    m_volumeMaxModel.saveSettings(_doc, _this, "volmax");
    m_panRangeModel.saveSettings(_doc, _this, "pandelta");
    m_panBaseModel.saveSettings(_doc, _this, "panbase");
    m_panMinModel.saveSettings(_doc, _this, "panmin");
    m_panMaxModel.saveSettings(_doc, _this, "panmax");
}

void InstrumentFunctionNoteKeying::loadSettings(const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "enabled");
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");
    m_volumeRangeModel.loadSettings(_this, "voldelta");
    m_volumeBaseModel.loadSettings(_this, "volbase");
    m_volumeMinModel.loadSettings(_this, "volmin");
    m_volumeMaxModel.loadSettings(_this, "volmax");
    m_panRangeModel.loadSettings(_this, "pandelta");
    m_panBaseModel.loadSettings(_this, "panbase");
    m_panMinModel.loadSettings(_this, "panmin");
    m_panMaxModel.loadSettings(_this, "panmax");
}

InstrumentFunctionView* InstrumentFunctionNoteKeying::createView()
{
    return new InstrumentFunctionNoteKeyingView(this);
}

InstrumentFunctionNoteOutting::InstrumentFunctionNoteOutting(Model* _parent) :
      InstrumentFunction(_parent, tr("NoteOutting")),
      // m_enabledModel( false, this ),
      m_volumeModel(0., 0., 200., 0.1, this, tr("Volume")),
      m_panModel(0., -100., 100., 0.1, this, tr("Pan")),
      m_keyModel(DefaultKey, 0., 127., 1., this, tr("Key")),
      m_noteModel(DefaultKey % 12, 0., 11., 1., this, tr("Note")),
      m_modValueModel(0., -1., 1., 0.001, this, tr("Modulation value")),
      m_modRefKeyModel(
              DefaultKey, 0., NumKeys - 1, 1., this, tr("Modulation key")),
      m_modAmountModel(0., -1., 1., 0.001, this, tr("Modulation amount")),
      m_modBaseModel(0., -1., 1., 0.001, this, tr("Modulation base"))
{
    m_modBaseModel.setStrictStepSize(true);

    m_volumeModel.setJournalling(false);
    m_panModel.setJournalling(false);
    m_keyModel.setJournalling(false);
    m_noteModel.setJournalling(false);
    m_modValueModel.setJournalling(false);

    m_volumeModel.setFrequentlyUpdated(true);
    m_panModel.setFrequentlyUpdated(true);
    m_keyModel.setFrequentlyUpdated(true);
    m_noteModel.setFrequentlyUpdated(true);
    m_modValueModel.setFrequentlyUpdated(true);
}

InstrumentFunctionNoteOutting::~InstrumentFunctionNoteOutting()
{
}

bool InstrumentFunctionNoteOutting::processNote(NotePlayHandle* _n)
{
    if(!shouldProcessNote(_n))
        return true;
    if(_n->totalFramesPlayed() != 0 || _n->isReleased())
        return true;

    m_volumeModel.setAutomatedValue(_n->getVolume());
    m_panModel.setAutomatedValue(_n->getPanning());
    m_keyModel.setAutomatedValue(_n->key());
    m_noteModel.setAutomatedValue(_n->key() % 12);
    m_modValueModel.setAutomatedValue(
            bound(-1.,
                  real_t(_n->key() - m_modRefKeyModel.value())
                                  * m_modAmountModel.value()
                          + m_modBaseModel.value(),
                  1.));

    return true;
}

void InstrumentFunctionNoteOutting::saveSettings(QDomDocument& _doc,
                                                 QDomElement&  _this)
{
    m_enabledModel.saveSettings(_doc, _this, "enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");

    m_modRefKeyModel.saveSettings(_doc, _this, "mod_refkey");
    m_modAmountModel.saveSettings(_doc, _this, "mod_amount");
    m_modBaseModel.saveSettings(_doc, _this, "mod_base");
}

void InstrumentFunctionNoteOutting::loadSettings(const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "enabled");
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");

    m_modRefKeyModel.loadSettings(_this, "mod_refkey");
    m_modAmountModel.loadSettings(_this, "mod_amount");
    m_modBaseModel.loadSettings(_this, "mod_base");
}

InstrumentFunctionView* InstrumentFunctionNoteOutting::createView()
{
    return new InstrumentFunctionNoteOuttingView(this);
}

InstrumentFunctionGlissando::InstrumentFunctionGlissando(Model* _parent) :
      InstrumentFunction(_parent, tr("Glissando")),
      m_gliTimeModel(75., 5., 500., 0.1, 500., this, tr("Glissando time")),
      m_gliGateModel(100., 1., 100., 1., this, tr("Glissando gate")),
      m_gliAttenuationModel(
              30., 0., 99., 1., this, tr("Glissando attenuation")),
      m_gliUpModeModel(this, tr("Glissando Up Mode")),
      m_gliDownModeModel(this, tr("Glissando Down Mode")), m_lastKey(-1),
      m_lastTime(-1)
{
    m_gliUpModeModel.addItem(tr("As previous key"));
    m_gliUpModeModel.addItem(tr("As next key"));
    m_gliUpModeModel.addItem(tr("As both or black"));
    m_gliUpModeModel.addItem(tr("As both or white"));
    m_gliUpModeModel.addItem(tr("All keys"));
    m_gliUpModeModel.addItem(tr("None"));
    m_gliUpModeModel.addItem(tr("Black keys"));
    m_gliUpModeModel.addItem(tr("White keys"));

    m_gliDownModeModel.addItem(tr("As previous key"));
    m_gliDownModeModel.addItem(tr("As next key"));
    m_gliDownModeModel.addItem(tr("As both or black"));
    m_gliDownModeModel.addItem(tr("As both or white"));
    m_gliDownModeModel.addItem(tr("All keys"));
    m_gliDownModeModel.addItem(tr("None"));
    m_gliDownModeModel.addItem(tr("Black keys"));
    m_gliDownModeModel.addItem(tr("White keys"));

    connect(Engine::getSong(), SIGNAL(playbackStateChanged()), this,
            SLOT(reset()));
}

InstrumentFunctionGlissando::~InstrumentFunctionGlissando()
{
}

void InstrumentFunctionGlissando::reset()
{
    if(!Engine::getSong()->isPlaying())
    {
        // qInfo("Glissando: reset()");
        m_lastKey  = -1;
        m_lastTime = -1;
    }
}

// static std::chrono::steady_clock::time_point s_startTime
//        = std::chrono::steady_clock::now();

bool InstrumentFunctionGlissando::processNote(NotePlayHandle* _n)
{
    if(!shouldProcessNote(_n))
        return true;

    if(_n->generation() >= 1)
        return true;

    if(_n->totalFramesPlayed() != 0)
    {
        // qInfo("Glissando: TFP>0 last=%ld cur=%ld\r", m_lastTime,
        // curTime); m_lastKey  = newKey; m_lastTime =
        // qMax(m_lastTime,curTime);
        return true;
    }

    if(_n->isReleased())
    {
        // qInfo("Glissando: RELEASED");
        // m_lastKey  = newKey;
        // m_lastTime = qMax(m_lastTime,curTime);
        return true;
    }

    const int newKey = _n->key();

    QMutexLocker locker(&m_mutex);

    // qInfo("\nNEW KEY IS %d", newKey);
    /*
    const int64_t curTime
            = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::steady_clock::now() - s_startTime)
                      .count();
    */
    const int64_t perTime
            = static_cast<int64_t>(Engine::mixer()->framesPerPeriod() * 1000.
                                   / Engine::mixer()->processingSampleRate());

    const int64_t curTime = Engine::getSong()->getMilliseconds();

    const int64_t frmTime = static_cast<int64_t>(
            _n->length() * Engine::framesPerTick() * 1000.
            / Engine::mixer()->processingSampleRate());
    const int64_t offTime = static_cast<int64_t>(
            _n->offset() * 1000. / Engine::mixer()->processingSampleRate());
    // qInfo("Glissando: INFO lk=%d nk=%d last=%ld cur=%ld frm=%ld off=%ld
    // "
    //      "per=%ld",
    //      m_lastKey, newKey, m_lastTime, curTime, frmTime, offTime,
    //      perTime);

    if(m_lastKey < 0 || m_lastKey > 127)
    {
        m_lastKey  = newKey;
        m_lastTime = curTime + frmTime + offTime;
        // qInfo("Glissando: KEY==%d last=%ld thread=%p", m_lastKey,
        // m_lastTime,
        //      QThread::currentThread());
        return true;
    }

    if(curTime + offTime + perTime + 10 < m_lastTime)
    {
        // qInfo("Glissando: SKIPPED cur=%ld last=%ld", curTime,
        // m_lastTime);
        return true;
    }

    if(curTime - m_lastTime >= 150)
    {
        m_lastKey  = newKey;
        m_lastTime = curTime + frmTime + offTime;
        // qInfo("Glissando: GAP600 last=%ld", m_lastTime);
        return true;
    }

    if(m_lastKey == newKey)
    {
        // m_lastTime = qMax(m_lastTime, curTime + frmTime + offTime);
        // qInfo("Glissando: LAST==NEW last=%ld", m_lastTime);
        return true;
    }

    if(Engine::mixer()->criticalXRuns())
    {
        m_lastKey  = newKey;
        m_lastTime = curTime + frmTime + offTime;
        // qInfo("Glissando: CRITICAL XRUNS last=%ld", m_lastTime);
        return true;
    }

    const int step = (newKey < m_lastKey ? -1 : 1);

    int mode = (step >= 0 ? m_gliUpModeModel.value()
                          : m_gliDownModeModel.value());

    if(mode == 5)
    {
        m_lastKey  = newKey;
        m_lastTime = curTime + frmTime + offTime;
        // qInfo("Glissando: NONE MODE last=%ld", m_lastTime);
        return true;
    }

    bool allowBlack  = true;
    bool allowWhite  = true;
    bool lastIsBlack = Piano::isBlackKey(m_lastKey);
    bool lastIsWhite = !lastIsBlack;
    bool nextIsBlack = Piano::isBlackKey(newKey);
    bool nextIsWhite = !nextIsBlack;

    switch(mode)
    {
        case 0:
            allowBlack = lastIsBlack;
            allowWhite = !allowBlack;
            break;
        case 1:
            allowBlack = nextIsBlack;
            allowWhite = !allowBlack;
            break;
        case 2:
            allowWhite = lastIsWhite && nextIsWhite;
            allowBlack = !allowWhite;
            break;
        case 3:
            allowBlack = lastIsBlack && nextIsBlack;
            allowWhite = !allowBlack;
            break;
        case 6:
            allowBlack = true;
            allowWhite = false;
            break;
        case 7:
            allowBlack = false;
            allowWhite = true;
            break;
    }

    // const int howmany = abs(m_lastKey - newKey);
    int howmany = 0;
    for(int key = m_lastKey + step; key != newKey; key += step)
    {
        if(key >= NumKeys || key < 0)
            continue;

        if(!allowBlack && Piano::isBlackKey(key))
            continue;
        if(!allowWhite && Piano::isWhiteKey(key))
            continue;

        howmany++;
    }

    if(howmany <= 1)
    {
        // qInfo("Glissando: HOWMANY<=1 (%d) lk=%d nk=%d step=%d
        // last=%ld",
        //       howmany, m_lastKey, newKey, step, m_lastTime);
        m_lastKey  = newKey;
        m_lastTime = curTime + frmTime + offTime;
        // qInfo("         : HOWMANY<=1 (%d) lk=%d nk=%d step=%d
        // last=%ld",
        //      howmany, m_lastKey, newKey, step, m_lastTime);
        return true;
    }

    // qInfo("Glissando: PROCEED many=%d", howmany);

    // number of frames that every note should be played
    f_cnt_t note_frames
            = (f_cnt_t)(m_gliTimeModel.value() / 1000.
                        * Engine::mixer()->processingSampleRate());

    if(howmany * note_frames > _n->length() * Engine::framesPerTick())
        note_frames = _n->length() * Engine::framesPerTick() / howmany;

    const f_cnt_t gated_frames
            = (f_cnt_t)(m_gliGateModel.value() * note_frames / 100.);

    if(gated_frames <= 6)
    {
        // qInfo("Glissando: GATED<=6");
        m_lastKey  = newKey;
        m_lastTime = curTime + frmTime + offTime;
        return true;
    }

    int pos_frame = _n->totalFramesPlayed() + note_frames - 1;

    f_cnt_t frames_processed = _n->noteOffset();

    for(int key = m_lastKey + step; key != newKey; key += step)
    {
        if(key >= NumKeys || key < 0)
            continue;

        if(!allowBlack && Piano::isBlackKey(key))
            continue;
        if(!allowWhite && Piano::isWhiteKey(key))
            continue;

        real_t a = m_gliAttenuationModel.value() / 100.;
        if(Piano::isBlackKey(key))
            a *= 1.05;
        a = bound(0., a, 0.99);

        if(!Engine::mixer()->criticalXRuns() && !_n->isTrackMuted())
        {
            Note subnote(_n->length(), 0, key, _n->getVolume() * (1. - a),
                         _n->getPanning(), _n->detuning());

            // create sub-note-play-handle, only ptr to note is different
            // and is_gli_note=true
            NotePlayHandle* nph = NotePlayHandleManager::acquire(
                    _n->instrumentTrack(), frames_processed, gated_frames,
                    subnote, _n, -1, NotePlayHandle::OriginGlissando,
                    _n->generation() + 1);
            Engine::mixer()->emit playHandleToAdd(nph);
        }

        // update counters
        frames_processed += note_frames;
        pos_frame += note_frames;
    }

    m_lastKey = newKey;

    const f_cnt_t total_frames = frames_processed - _n->offset();

    // const int64_t totTime = static_cast<int64_t>(
    //        total_frames * 1000. /
    //        Engine::mixer()->processingSampleRate());

    // const int64_t posTime = static_cast<int64_t>(
    //        pos_frame * 1000. /
    //        Engine::mixer()->processingSampleRate());

    // qInfo("Glissando : ++++++++> frm=%ld off=%ld pos=%ld tot=%ld",
    // frmTime,
    //      offTime, posTime, totTime);

    const int64_t endTime = curTime + frmTime + offTime;
    // qInfo("Glissando: NEW prev=%ld cur=%ld end=%ld", m_lastTime,
    // curTime,
    //      endTime);
    m_lastTime = endTime;

    if(!Engine::mixer()->criticalXRuns() && !_n->isTrackMuted())
    {
        // recreate the note
        Note note(_n->length(), _n->pos(), _n->key(), _n->getVolume(),
                  _n->getPanning(), _n->detuning());

        NotePlayHandle* nph = NotePlayHandleManager::acquire(
                _n->instrumentTrack(), frames_processed,
                _n->frames() - total_frames, note, NULL, -1,
                NotePlayHandle::OriginGlissando, _n->generation());
        Engine::mixer()->emit playHandleToAdd(nph);
    }

    return false;
}

void InstrumentFunctionGlissando::saveSettings(QDomDocument& _doc,
                                               QDomElement&  _this)
{
    m_enabledModel.saveSettings(_doc, _this, "enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");
    m_gliTimeModel.saveSettings(_doc, _this, "time");
    m_gliGateModel.saveSettings(_doc, _this, "gate");
    m_gliAttenuationModel.saveSettings(_doc, _this, "attenuation");
    m_gliUpModeModel.saveSettings(_doc, _this, "up_mode");
    m_gliDownModeModel.saveSettings(_doc, _this, "down_mode");
}

void InstrumentFunctionGlissando::loadSettings(const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "enabled");
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");
    m_gliTimeModel.loadSettings(_this, "time");
    m_gliGateModel.loadSettings(_this, "gate");
    m_gliAttenuationModel.loadSettings(_this, "attenuation");
    m_gliUpModeModel.loadSettings(_this, "up_mode");
    m_gliDownModeModel.loadSettings(_this, "down_mode");
}

InstrumentFunctionView* InstrumentFunctionGlissando::createView()
{
    return new InstrumentFunctionGlissandoView(this);
}

InstrumentFunctionNoteSustaining::InstrumentFunctionNoteSustaining(
        Model* _parent) :
      InstrumentFunction(_parent, tr("NoteSustaining")),
      // m_enabledModel( false, this ),
      m_lastKey(-1), m_lastTime(-1)
/*m_lastNPH(nullptr),
 m_volumeRangeModel(0., 0., 100., 0.1, this, tr("Volume change")),
 m_panRangeModel(0., 0., 100., 0.1, this, tr("Pan change")),
 m_tuneRangeModel(0., 0., 100., 0.1, this, tr("Frequency change")),
 m_offsetRangeModel(0., 0., 100., 0.1, this, tr("Start delay")),
 m_shortenRangeModel(1., 1., 100., 0.1, this, tr("Shortening")),
 m_volumeStepModel(1., 1., 100., 1., this, tr("Volume step")),
 m_panStepModel(1., 1., 100., 1., this, tr("Pan step")),
 m_tuneStepModel(1., 1., 100., 1., this, tr("Frequency step")),
 m_offsetStepModel(1., 1., 100., 1., this, tr("Start step")),
 m_shortenStepModel(1., 1., 100., 1., this, tr("Shortening step"))
                   */
{
    connect(Engine::getSong(), SIGNAL(playbackStateChanged()), this,
            SLOT(reset()));
}

InstrumentFunctionNoteSustaining::~InstrumentFunctionNoteSustaining()
{
}

void InstrumentFunctionNoteSustaining::reset()
{
    if(!Engine::getSong()->isPlaying())
    {
        // qInfo("Glissando: reset()");
        m_lastKey  = -1;
        m_lastTime = -1;
    }
}

bool InstrumentFunctionNoteSustaining::processNote(NotePlayHandle* _n)
{
    if(!shouldProcessNote(_n))
        return true;

    const int     newKey  = _n->key();
    const int64_t curTime = Engine::getSong()->getMilliseconds();

    QMutexLocker locker(&m_mutex);

    if(newKey == m_lastKey)
    {
        if(!_n->isReleased())
            _n->setFrames(_n->frames() + Engine::mixer()->framesPerPeriod());
        else
        {
            m_lastKey  = -1;
            m_lastTime = -1;
        }
    }
    else if(_n->totalFramesPlayed() == 0)
    {
        const int64_t offTime = static_cast<int64_t>(
                _n->offset() * 1000.
                / Engine::mixer()->processingSampleRate());

        m_lastKey  = newKey;
        m_lastTime = curTime + offTime;
    }

    // qInfo("InstrumentFunctionNoteSustaining::process Note %p f=%d", _n,
    //      _n->frames());
    return true;
}

void InstrumentFunctionNoteSustaining::saveSettings(QDomDocument& _doc,
                                                    QDomElement&  _this)
{
    m_enabledModel.saveSettings(_doc, _this, "enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");

    /*
    m_volumeRangeModel.saveSettings(_doc, _this, "volume");
    m_panRangeModel.saveSettings(_doc, _this, "pan");
    m_tuneRangeModel.saveSettings(_doc, _this, "tune");
    m_offsetRangeModel.saveSettings(_doc, _this, "offset");
    m_shortenRangeModel.saveSettings(_doc, _this, "shorten");

    m_volumeStepModel.saveSettings(_doc, _this, "volume_step");
    m_panStepModel.saveSettings(_doc, _this, "pan_step");
    m_tuneStepModel.saveSettings(_doc, _this, "tune_step");
    m_offsetStepModel.saveSettings(_doc, _this, "offset_step");
    m_shortenStepModel.saveSettings(_doc, _this, "shorten_step");
    */
}

void InstrumentFunctionNoteSustaining::loadSettings(const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "enabled");
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");

    /*
    m_volumeRangeModel.loadSettings(_this, "volume");
    m_panRangeModel.loadSettings(_this, "pan");
    m_tuneRangeModel.loadSettings(_this, "tune");
    m_offsetRangeModel.loadSettings(_this, "offset");
    m_shortenRangeModel.loadSettings(_this, "shorten");

    m_volumeStepModel.loadSettings(_this, "volume_step");
    m_panStepModel.loadSettings(_this, "pan_step");
    m_tuneStepModel.loadSettings(_this, "tune_step");
    m_offsetStepModel.loadSettings(_this, "offset_step");
    m_shortenStepModel.loadSettings(_this, "shorten_step");
    */
}

InstrumentFunctionView* InstrumentFunctionNoteSustaining::createView()
{
    return new InstrumentFunctionNoteSustainingView(this);
}

InstrumentFunctionNotePlaying::InstrumentFunctionNotePlaying(Model* _parent) :
      InstrumentFunction(_parent, tr("Playing")),
      m_gateModel(0., 0., 1., 1., this, tr("Gate")),
      m_keyModel(DefaultKey, 0., NumKeys, 1., this, tr("Key")),
      m_volModel(100., 0., 200., 0.1, this, tr("Volume")),
      m_panModel(0., -100., 100., 0.1, this, tr("Pan")),
      m_currentGeneration(0), m_currentNPH(nullptr)
{
    m_gateModel.setStrictStepSize(true);

    // connect(Engine::getSong(), SIGNAL(playbackStateChanged()), this,
    //        SLOT(reset()));
    connect(&m_gateModel, SIGNAL(dataChanged()), this, SLOT(onGateChanged()));
}

InstrumentFunctionNotePlaying::~InstrumentFunctionNotePlaying()
{
}

void InstrumentFunctionNotePlaying::reset()
{
    qInfo("InstrumentFunctionNotePlaying::reset");
    if(m_currentNPH != nullptr)
    {
        m_currentNPH->decrRefCount();
        m_currentNPH->noteOff();
        m_currentNPH = nullptr;
    }
}

bool InstrumentFunctionNotePlaying::processNote(NotePlayHandle* _n)
{
    return true;
}

void InstrumentFunctionNotePlaying::onGateChanged()
{
    qInfo("InstrumentFunctionNotePlaying::onGateChanged");

    reset();

    InstrumentTrack* track = dynamic_cast<InstrumentTrack*>(parent());
    if(track == nullptr || track->isMuted() || m_gateModel.value() < 0.5
       || Engine::mixer()->criticalXRuns())
        return;

    int  key = m_keyModel.value();
    Note subnote(192 * 128, 0, key, m_volModel.value(), m_panModel.value(),
                 nullptr);  // V,P,B

    const int mingen = m_minNoteGenerationModel.value();
    const int maxgen = m_maxNoteGenerationModel.value();
    m_currentGeneration++;
    if(m_currentGeneration > maxgen || m_currentGeneration < mingen)
        m_currentGeneration = mingen;

    NotePlayHandle* nph = NotePlayHandleManager::acquire(
            track, 0, 192 * 128 * Engine::framesPerTick(), subnote, nullptr,
            -1, NotePlayHandle::OriginPlaying, m_currentGeneration);

    nph->incrRefCount();
    m_currentNPH = nph;
    Engine::mixer()->emit playHandleToAdd(nph);
}

void InstrumentFunctionNotePlaying::saveSettings(QDomDocument& _doc,
                                                 QDomElement&  _this)
{
    m_enabledModel.saveSettings(_doc, _this, "enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");
}

void InstrumentFunctionNotePlaying::loadSettings(const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "enabled");
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");
}

InstrumentFunctionView* InstrumentFunctionNotePlaying::createView()
{
    return new InstrumentFunctionNotePlayingView(this);
}
