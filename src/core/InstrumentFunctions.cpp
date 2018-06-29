/*
 * InstrumentFunctions.cpp - models for instrument-function-tab
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "InstrumentFunctions.h"

#include <time.h>

#include <QDomElement>

#include "Engine.h"
#include "InstrumentFunctionViews.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "PresetPreviewPlayHandle.h"
#include "Song.h"
#include "embed.h"
#include "lmms_math.h"

InstrumentFunction::InstrumentFunction(Model* _parent, QString _name) :
      Model(_parent, _name), m_enabledModel(false, this),
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
           {QT_TRANSLATE_NOOP("InstrumentFunctionNoteStacking",
                              "Phrygolydian"),
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
        const char* n, const ChordSemiTones& semi_tones) :
      m_name(InstrumentFunctionNoteStacking::tr(n))
{
    for(m_size = 0; m_size < MAX_CHORD_POLYPHONY; m_size++)
    {
        if(semi_tones[m_size] == -1)
        {
            break;
        }

        m_semiTones[m_size] = semi_tones[m_size];
    }
}

bool InstrumentFunctionNoteStacking::Chord::hasSemiTone(
        int8_t semi_tone) const
{
    for(int i = 0; i < size(); ++i)
    {
        if(semi_tone == m_semiTones[i])
        {
            return true;
        }
    }
    return false;
}

InstrumentFunctionNoteStacking::ChordTable::ChordTable() : QVector<Chord>()
{
    for(int i = 0;
        i < static_cast<int>(sizeof s_initTable / sizeof *s_initTable); i++)
    {
        push_back(Chord(s_initTable[i].m_name, s_initTable[i].m_semiTones));
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
      m_chordRangeModel(1.0f, 1.0f, 9.0f, 1.0f, this, tr("Chord range"))
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
                   || Engine::mixer()->criticalXRuns())
                {
                    // qInfo("InstrumentFunctionNoteStacking::processNote
                    // subnote break");
                    break;
                }

                // if(MM_SAFE(NotePlayHandle,1))
                {

                    // create copy of base-note
                    Note subnote(_n->length(), 0, subnote_key,
                                 _n->getVolume(), _n->getPanning(),
                                 _n->detuning());

                    // create sub-note-play-handle, only note is
                    // different
                    Engine::mixer()->addPlayHandle(
                            NotePlayHandleManager::acquire(
                                    _n->instrumentTrack(), _n->offset(),
                                    _n->frames(), subnote, _n, -1,
                                    NotePlayHandle::OriginNoteStacking,
                                    _n->generation() + 1));
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
      m_arpRangeModel(1.0f, 1.0f, 9.0f, 1.0f, this, tr("Arpeggio range")),
      m_arpCycleModel(0.0f, 0.0f, 6.0f, 1.0f, this, tr("Cycle steps")),
      m_arpSkipModel(0.0f, 0.0f, 100.0f, 1.0f, this, tr("Skip rate")),
      m_arpMissModel(0.0f, 0.0f, 100.0f, 1.0f, this, tr("Miss rate")),
      m_arpTimeModel(
              200.0f, 25.0f, 2000.0f, 1.0f, 2000, this, tr("Arpeggio time")),
      m_arpGateModel(100.0f, 1.0f, 200.0f, 1.0f, this, tr("Arpeggio gate")),
      m_arpDirectionModel(this, tr("Arpeggio direction")),
      m_arpModeModel(this, tr("Arpeggio mode"))
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

    if(  //_n->origin() == NotePlayHandle::OriginArpeggio ||
         //_n->origin() == NotePlayHandle::OriginNoteStacking ||
            (_n->isReleased()
             && _n->releaseFramesDone() >= _n->actualReleaseFramesToDo()))
    {
        return true;
    }

    // qInfo("InstrumentFunctionArpeggio::processNote n.key=%d in
    // g=%d",_n->key(),_n->generation()); if(_n->totalFramesPlayed()!=0 /*||
    // _n->isReleased()*/ ) return true;
    // qInfo("InstrumentFunctionArpeggio::processNote n.key=%d OK
    // g=%d",_n->key(),_n->generation());

    const int base_note_key = _n->key();
    const int selected_arp  = m_arpModel.value();

    ConstNotePlayHandleList cnphv
            = NotePlayHandle::nphsOfInstrumentTrack(_n->instrumentTrack());

    if(m_arpModeModel.value() != FreeMode && cnphv.size() == 0)
    {
        // maybe we're playing only a preset-preview-note?
        cnphv = PresetPreviewPlayHandle::nphsOfInstrumentTrack(
                _n->instrumentTrack());
        if(cnphv.size() == 0)
        {
            // still nothing found here, so lets return
            // return;
            cnphv.push_back(_n);
        }
    }

    const InstrumentFunctionNoteStacking::ChordTable& chord_table
            = InstrumentFunctionNoteStacking::ChordTable::getInstance();
    const int cur_chord_size = chord_table[selected_arp].size();
    const int range       = (int)(cur_chord_size * m_arpRangeModel.value());
    const int total_range = range * cnphv.size();

    // number of frames that every note should be played
    const f_cnt_t arp_frames
            = (f_cnt_t)(m_arpTimeModel.value() / 1000.0f
                        * Engine::mixer()->processingSampleRate());
    const f_cnt_t gated_frames
            = (f_cnt_t)(m_arpGateModel.value() * arp_frames / 100.0f);

    // used for calculating remaining frames for arp-note, we have to add
    // arp_frames-1, otherwise the first arp-note will not be setup
    // correctly... -> arp_frames frames silence at the start of every note!
    int cur_frame = ((m_arpModeModel.value() != FreeMode)
                             ? cnphv.first()->totalFramesPlayed()
                             : _n->totalFramesPlayed())
                    + arp_frames - 1;
    // used for loop
    f_cnt_t frames_processed = (m_arpModeModel.value() != FreeMode)
                                       ? cnphv.first()->noteOffset()
                                       : _n->noteOffset();

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
                      != (f_cnt_t)_n->index())
        {
            // Set master note if not playing arp note or it will play as an
            // ordinary note
            if(_n->generation() == 0)
                _n->setMasterNote();
            // update counters
            frames_processed += arp_frames;
            cur_frame += arp_frames;
            continue;
        }

        // Skip notes randomly
        if(m_arpSkipModel.value())
        {

            if(100 * ((float)rand() / (float)(RAND_MAX + 1.0f))
               < m_arpSkipModel.value())
            {
                // Set master note to prevent the note to extend over skipped
                // notes This may only be needed for lb302
                if(_n->generation() == 0)
                    _n->setMasterNote();
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
            if(100 * ((float)rand() / (float)(RAND_MAX + 1.0f))
               < m_arpMissModel.value())
            {
                dir = ArpDirRandom;
            }
        }

        int cur_arp_idx = 0;
        // process according to arpeggio-direction...
        if(dir == ArpDirUp)
        {
            cur_arp_idx = (cur_frame / arp_frames) % range;
        }
        else if(dir == ArpDirDown)
        {
            cur_arp_idx = range - (cur_frame / arp_frames) % range - 1;
        }
        else if(dir == ArpDirUpAndDown && range > 1)
        {
            // imagine, we had to play the arp once up and then
            // once down -> makes 2 * range possible notes...
            // because we don't play the lower and upper notes
            // twice, we have to subtract 2
            cur_arp_idx = (cur_frame / arp_frames) % (range * 2 - 2);
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
            cur_arp_idx = (cur_frame / arp_frames) % (range * 2 - 2);
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
            cur_arp_idx = (int)(range * ((float)rand() / (float)RAND_MAX));
        }

        // Cycle notes
        if(m_arpCycleModel.value() && dir != ArpDirRandom)
        {
            cur_arp_idx *= m_arpCycleModel.value() + 1;
            cur_arp_idx %= range;
        }

        // now calculate final key for our arp-note
        const int subnote_key
                = base_note_key
                  + (cur_arp_idx / cur_chord_size) * KeysPerOctave
                  + chord_table[selected_arp][cur_arp_idx % cur_chord_size];

        // range-checking
        if(subnote_key >= NumKeys || subnote_key < 0
           || Engine::mixer()->criticalXRuns())
        {
            // qInfo("InstrumentFunctionArpeggio::processNote subnote break");
            continue;
        }

        float vol_level = 1.0f;
        if(_n->isReleased())
        {
            vol_level = _n->volumeLevel(cur_frame + gated_frames);
        }

        // create new arp-note

        // if(MM_SAFE(NotePlayHandle,1))
        {
            // Note subnote( _n->length(), 0, subnote_key, _n->getVolume(),
            // _n->getPanning(), _n->detuning() );
            Note subnote(/*MidiTime( 0 ), MidiTime( 0 ),*/
                         _n->length(), 0, subnote_key,
                         (volume_t)qRound(_n->getVolume() * vol_level),
                         _n->getPanning(), _n->detuning());

            // create sub-note-play-handle, only ptr to note is different
            // and is_arp_note=true
            Engine::mixer()->addPlayHandle(NotePlayHandleManager::acquire(
                    _n->instrumentTrack(), frames_processed, gated_frames,
                    subnote, _n, -1, NotePlayHandle::OriginArpeggio,
                    _n->generation() + 1));
        }

        // update counters
        frames_processed += arp_frames;
        cur_frame += arp_frames;
    }

    // make sure note is handled as arp-base-note, even
    // if we didn't add a sub-note so far
    if(m_arpModeModel.value() != FreeMode)
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
                                                     "arpsyncmode" ).toInt()
       );
            }*/

    m_arpModeModel.loadSettings(_this, "arpmode");
}

InstrumentFunctionView* InstrumentFunctionArpeggio::createView()
{
    return new InstrumentFunctionArpeggioView(this);
}

InstrumentFunctionNoteHumanizing::InstrumentFunctionNoteHumanizing(
        Model* _parent) :
      InstrumentFunction(_parent, tr("NoteHumanizing")),
      // m_enabledModel( false, this ),
      m_volumeRangeModel(0.0f, 0.0f, 100.0f, 0.1f, this, tr("Volume change")),
      m_panRangeModel(0.0f, 0.0f, 100.0f, 0.1f, this, tr("Pan change")),
      m_tuneRangeModel(
              0.0f, 0.0f, 100.0f, 0.1f, this, tr("Frequency change")),
      m_offsetRangeModel(0.0f, 0.0f, 100.0f, 0.1f, this, tr("Start delay")),
      m_shortenRangeModel(0.0f, 0.0f, 100.0f, 0.1f, this, tr("Shortening"))
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
        float l = m_volumeRangeModel.value() / 100.f;
        if(l > 0.f)
        {
            float    r = fastrandf01inc();
            volume_t o = _n->getVolume();  // 0..200
            volume_t n = qBound(0, qRound(o * (1.f - l * r)), 200);
            // qInfo("NH: volume %d->%d",o,n);
            _n->setVolume(n);
        }
    }

    {
        float l = m_panRangeModel.value() / 100.f;
        if(l > 0.f)
        {
            float     r = fastrandf01inc();
            panning_t o = _n->getPanning();  // -100..100
            panning_t n
                    = qBound(-100, qRound(o + 200.f * l * (r - 0.5f)), 100);
            // qInfo("NH: panning %d->%d",o,n);
            _n->setPanning(n);
        }
    }

    {
        float l = m_tuneRangeModel.value() / 100.f;
        if(l > 0.f)
        {
            float r = fastrandf01inc();
            float o = _n->baseDetune();
            float n = o + 12.f * (l * (r - 0.5f));
            // qInfo("NH: detune %f->%f",o,n);
            _n->setBaseDetune(n);
            _n->setFrequencyUpdate();
        }
    }

    {
        float l = m_offsetRangeModel.value() / 100.f;
        if(l > 0.f)
        {
            float       r   = fastrandf01inc();
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
        float l = m_shortenRangeModel.value() / 100.f;
        if(l > 0.f)
        {
            float   r = fastrandf01inc();
            f_cnt_t o = _n->frames();  // ?
            f_cnt_t n = qBound(1, qRound(o * (1.f - l * r)), o);
            // qInfo("NH: shorten %d->%d",o,n);
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

    static QMutex mtx;
    mtx.lock();
    // const float k=Engine::getSong()->getPlayPos().absoluteFrame();
    const int k = (1000 * clock() / CLOCKS_PER_SEC);  // ms
    // qInfo("InstrumentFunctionNoteDuplicatesRemoving: cache k=%d v=%d
    // in=%d",
    //  k,_n->key(),m_cache.contains(k,_n->key()));

    // const float fpt=Engine::framesPerTick();
    int i = 0;
    foreach(const int ck, m_cache)
    {
        if(ck < k - 150)  // || ck>=k+150)
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
        mtx.unlock();
        _n->noteOff();  //???? kicker
        return false;
    }

    // qInfo("NoteDuplicatesRemoving: cache size=%d",m_cache.size());
    m_cache.insert(k, _n->key());
    mtx.unlock();
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
      m_configModel(this, tr("Configuration"))
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

    _n->setMasterNote();

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
      m_volumeRangeModel(
              0.0f, -500.0f, 500.0f, 1.0f, this, tr("Volume change")),
      m_volumeBaseModel(0.0f, 0.0f, 127.0f, 1.f, this, tr("Volume base key")),
      m_volumeMinModel(0.0f, 0.0f, 200.0f, 0.1f, this, tr("Volume min")),
      m_volumeMaxModel(100.0f, 0.0f, 200.0f, 0.1f, this, tr("Volume max")),
      m_panRangeModel(0.0f, -500.0f, 500.0f, 1.0f, this, tr("Pan change")),
      m_panBaseModel(0.0f, -1.0f, 1.0f, 0.01f, this, tr("Pan base")),
      m_panMinModel(0.0f, -100.0f, 100.0f, 0.1f, this, tr("Pan min")),
      m_panMaxModel(0.0f, -100.0f, 100.0f, 0.1f, this, tr("Pan max"))
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
        float l = m_volumeRangeModel.value() / 100.f;
        if(l != 0.f)
        {
            float    r = qBound(-1.0f,
                             (1.0f * _n->key() - m_volumeBaseModel.value())
                                     * 2.f / NumKeys,
                             1.0f);
            volume_t o = _n->getVolume();  // 0..200
            volume_t n = qBound(m_volumeMinModel.value(), o + 100.f * l * r,
                                m_volumeMaxModel.value());
            n          = qBound(0.f, n, 200.f);
            qInfo("NK: volume %f->%f", o, n);
            _n->setVolume(n);
        }
    }

    {
        float l = m_panRangeModel.value() / 100.f;
        if(l != 0.f)
        {
            float     r = qBound(-1.0f,
                             (1.0f * _n->key() - m_panBaseModel.value()) * 2.f
                                     / NumKeys,
                             1.0f);
            panning_t o = _n->getPanning();  // -100..100
            panning_t n = qBound(m_panMinModel.value(), o + 200.f * l * r,
                                 m_panMaxModel.value());
            n           = qBound(-100.f, n, 100.f);
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
      m_keyModel(DefaultKey, 0.0f, 127.0f, 1.0f, this, tr("Key")),
      m_volumeModel(0.0f, 0.0f, 200.0f, 0.1f, this, tr("Volume")),
      m_panModel(0.0f, -100.0f, 100.0f, 0.1f, this, tr("Pan"))
{
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

    m_keyModel.setValue(_n->key());
    m_volumeModel.setValue(_n->getVolume());
    m_panModel.setValue(_n->getPanning());

    // qInfo("InstrumentFunctionNoteOutting::process Note");
    return true;
}

void InstrumentFunctionNoteOutting::saveSettings(QDomDocument& _doc,
                                                 QDomElement&  _this)
{
    m_enabledModel.saveSettings(_doc, _this, "enabled");
    m_minNoteGenerationModel.saveSettings(_doc, _this, "mingen");
    m_maxNoteGenerationModel.saveSettings(_doc, _this, "maxgen");
}

void InstrumentFunctionNoteOutting::loadSettings(const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "enabled");
    m_minNoteGenerationModel.loadSettings(_this, "mingen");
    m_maxNoteGenerationModel.loadSettings(_this, "maxgen");
}

InstrumentFunctionView* InstrumentFunctionNoteOutting::createView()
{
    return new InstrumentFunctionNoteOuttingView(this);
}
