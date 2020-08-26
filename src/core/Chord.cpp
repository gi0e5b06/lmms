/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Chord.cpp -
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
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

#include "Chord.h"

// static const Chord s_null = {-1, "null", {-1,-1}, false};
// Chords::Init Chords::s_initTable[]

static const ChordDef NULL_CHORDDEF = {-1, "null", {-1}, false};

static const QVector<ChordDef> CHORDDEFS
        = {{0, "octave", {0}, false},
           {95, "3", {0, 4}, false},
           {92, "5", {0, 7}, false},

           {1, "Major", {0, 4, 7}, false},
           {2, "Majb5", {0, 4, 6}, false},
           {3, "minor", {0, 3, 7}, false},
           {4, "minb5", {0, 3, 6}, false},
           {5, "sus2", {0, 2, 7}, false},
           {6, "sus4", {0, 5, 7}, false},
           {7, "aug", {0, 4, 8}, false},
           {8, "augsus4", {0, 5, 8}, false},

           {9, "tri", {0, 3, 6, 9}, false},

           {10, "6", {0, 4, 7, 9}, false},
           {11, "6sus4", {0, 5, 7, 9}, false},
           {12, "6add9", {0, 4, 7, 9, 14}, false},
           {13, "m6", {0, 3, 7, 9}, false},
           {14, "m6add9", {0, 3, 7, 9, 14}, false},

           {15, "7", {0, 4, 7, 10}, false},
           {16, "7sus4", {0, 5, 7, 10}, false},
           {17, "7#5", {0, 4, 8, 10}, false},
           {18, "7b5", {0, 4, 6, 10}, false},
           {19, "7#9", {0, 4, 7, 10, 15}, false},
           {20, "7b9", {0, 4, 7, 10, 13}, false},
           {21, "7#5#9", {0, 4, 8, 10, 15}, false},
           {22, "7#5b9", {0, 4, 8, 10, 13}, false},
           {23, "7b5b9", {0, 4, 6, 10, 13}, false},
           {24, "7add11", {0, 4, 7, 10, 17}, false},
           {25, "7add13", {0, 4, 7, 10, 21}, false},
           {26, "7#11", {0, 4, 7, 10, 18}, false},
           {27, "Maj7", {0, 4, 7, 11}, false},
           {28, "Maj7b5", {0, 4, 6, 11}, false},
           {29, "Maj7#5", {0, 4, 8, 11}, false},
           {30, "Maj7#11", {0, 4, 7, 11, 18}, false},
           {31, "Maj7add13", {0, 4, 7, 11, 21}, false},
           {32, "m7", {0, 3, 7, 10}, false},
           {33, "m7b5", {0, 3, 6, 10}, false},
           {34, "m7b9", {0, 3, 7, 10, 13}, false},
           {35, "m7add11", {0, 3, 7, 10, 17}, false},
           {36, "m7add13", {0, 3, 7, 10, 21}, false},
           {37, "m-Maj7", {0, 3, 7, 11}, false},
           {38, "m-Maj7add11", {0, 3, 7, 11, 17}, false},
           {39, "m-Maj7add13", {0, 3, 7, 11, 21}, false},

           {40, "9", {0, 4, 7, 10, 14}, false},
           {41, "9sus4", {0, 5, 7, 10, 14}, false},
           {42, "add9", {0, 4, 7, 14}, false},
           {43, "9#5", {0, 4, 8, 10, 14}, false},
           {44, "9b5", {0, 4, 6, 10, 14}, false},
           {45, "9#11", {0, 4, 7, 10, 14, 18}, false},
           {46, "9b13", {0, 4, 7, 10, 14, 20}, false},
           {47, "Maj9", {0, 4, 7, 11, 14}, false},
           {48, "Maj9sus4", {0, 5, 7, 11, 15}, false},
           {49, "Maj9#5", {0, 4, 8, 11, 14}, false},
           {50, "Maj9#11", {0, 4, 7, 11, 14, 18}, false},
           {51, "m9", {0, 3, 7, 10, 14}, false},
           {52, "madd9", {0, 3, 7, 14}, false},
           {53, "m9b5", {0, 3, 6, 10, 14}, false},
           {54, "m9-Maj7", {0, 3, 7, 11, 14}, false},

           {55, "11", {0, 4, 7, 10, 14, 17}, false},
           {56, "11b9", {0, 4, 7, 10, 13, 17}, false},
           {57, "Maj11", {0, 4, 7, 11, 14, 17}, false},
           {58, "m11", {0, 3, 7, 10, 14, 17}, false},
           {59, "m-Maj11", {0, 3, 7, 11, 14, 17}, false},

           {60, "13", {0, 4, 7, 10, 14, 21}, false},
           {61, "13#9", {0, 4, 7, 10, 15, 21}, false},
           {62, "13b9", {0, 4, 7, 10, 13, 21}, false},
           {63, "13b5b9", {0, 4, 6, 10, 13, 21}, false},
           {64, "Maj13", {0, 4, 7, 11, 14, 21}, false},
           {65, "m13", {0, 3, 7, 10, 14, 21}, false},
           {66, "m-Maj13", {0, 3, 7, 11, 14, 21}, false},

           // Pentatonic
           {72, "Major pentatonic", {0, 2, 4, 7, 9}, true},
           {73, "Minor pentatonic", {0, 3, 5, 7, 10}, true},
           {99, "Japanese", {0, 2, 3, 7, 8}, true},     // 21414
           {74, "Jap in sen", {0, 1, 5, 7, 10}, true},  // 14232
           {100, "Yo", {0, 2, 5, 7, 9}, true},

           // Hexatonic
           {70, "Whole tone", {0, 2, 4, 6, 8, 10}, true},
           {77, "Blues", {0, 3, 5, 6, 7, 10}, true},

           // Heptatonic
           {67, "Major mode", {0, 2, 4, 5, 7, 9, 11}, true},
           {89, "Minor mode", {0, 2, 3, 5, 7, 8, 10}, true},
           {68, "Harmonic minor", {0, 2, 3, 5, 7, 8, 11}, true},
           {69, "Melodic minor", {0, 2, 3, 5, 7, 9, 11}, true},
           {98, "Double harmonic", {0, 1, 4, 5, 7, 8, 11}, true},
           {79, "Enigmatic", {0, 1, 4, 6, 8, 10, 11}, true},
           {87, "Aeolian", {0, 2, 3, 5, 7, 8, 10}, true},
           {83, "Dorian", {0, 2, 3, 5, 7, 9, 10}, true},
           {88, "Locrian", {0, 1, 3, 5, 6, 8, 10}, true},
           {85, "Lydian", {0, 2, 4, 6, 7, 9, 11}, true},
           {86, "Mixolydian", {0, 2, 4, 5, 7, 9, 10}, true},
           {80, "Neopolitan", {0, 1, 3, 5, 7, 9, 11}, true},
           {81, "Neopolitan minor", {0, 1, 3, 5, 7, 8, 11}, true},
           {84, "Phrygian", {0, 1, 3, 5, 7, 8, 10}, true},
           {93, "Phrygian dominant", {0, 1, 4, 5, 7, 8, 10}, true},
           {78, "Arabic", {0, 1, 4, 5, 7, 8, 11}, true},
           {82, "Hungarian minor", {0, 2, 3, 6, 7, 8, 11}, true},
           {96, "Hungarian major", {0, 3, 4, 6, 7, 9, 10}, true},
           {97, "Ukrainian dorian", {0, 2, 3, 5, 6, 8, 9}, true},
           {94, "Persian", {0, 1, 4, 5, 6, 8, 11}, true},

           // Octotonic
           {75, "Major bebop", {0, 2, 4, 5, 7, 8, 9, 11}, true},
           {76, "Dominant bebop", {0, 2, 4, 5, 7, 9, 10, 11}, true},
           {71, "Diminished", {0, 2, 3, 5, 6, 8, 9, 11}, true},
           {91, "Half-Whole Diminished", {0, 1, 3, 4, 6, 7, 9, 10}, true},

           // Dodecatonic
           {90, "Chromatic", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}, true}};

ChordDef::ChordDef(int                       _index,
                   const char*               _name,
                   const ChordDef::Semitones _semitones,
                   bool                      _mode) :
      m_index(_index),
      m_name(QObject::tr(_name, "InstrumentFunctionNoteStacking")),
      m_semitones(_semitones), m_mode(_mode)
{
}

bool ChordDef::hasSemitone(int8_t _semitone) const
{
    return m_semitones.contains(_semitone);
    /*
    for(int i = 0; i < m_semitones.size(); ++i)
        if(_semitone == m_semitones.at(i))
            return true;

    return false;
    */
}

/*
const ChordDef& ChordDef::atNum(int _num)
{
    return CHORDDEFS.at(_num);
}

int ChordDef::numOf(const ChordDef& _chord)
{
    for(int i=minNum();i<=maxNum();i++)
        if(CHORDDEFS.at(i).index()==_chord.index())
            return i;
    return -1;
}
*/

int ChordDef::minIndex()
{
    return 0;
}

int ChordDef::maxIndex()
{
    return 100;
}

void ChordDef::map(const std::function<void(const ChordDef&)>& _f)
{
    for(const ChordDef& c: CHORDDEFS)
        _f(c);
}

const ChordDef& ChordDef::findByName(const QString& _name)
{
    for(int i = 0; i < CHORDDEFS.size(); i++)
    {
        const ChordDef& r = CHORDDEFS.at(i);
        if(_name == r.name())
            return r;
    }

    return NULL_CHORDDEF;
}

const ChordDef& ChordDef::findByIndex(int _index)
{
    if(_index >= 3)
    {
        const ChordDef& r = CHORDDEFS.at(_index - 2);
        if(_index == r.index())
            return r;
    }

    for(int i = 0; i < CHORDDEFS.size(); i++)
    {
        const ChordDef& r = CHORDDEFS.at(i);
        if(_index == r.index())
            return r;
    }

    return NULL_CHORDDEF;
}

void ChordDef::fillAllModel(ComboBoxModel& _model, bool _none)
{
    _model.setDisplayName(_model.tr("Chords and Modes"));
    _model.clear();
    if(_none)
        _model.addItem(-1, _model.tr("None"));
    for(const ChordDef& c: CHORDDEFS)
        _model.addItem(c.index(), c.name());
}

void ChordDef::fillChordModel(ComboBoxModel& _model, bool _none)
{
    _model.setDisplayName(_model.tr("Chords"));
    _model.clear();
    if(_none)
        _model.addItem(-1, _model.tr("No chord"));
    for(const ChordDef& c: CHORDDEFS)
        if(!c.isMode())
            _model.addItem(c.index(), c.name());
}

void ChordDef::fillModeModel(ComboBoxModel& _model, bool _none)
{
    _model.setDisplayName(_model.tr("Modes"));
    _model.clear();
    if(_none)
        _model.addItem(-1, _model.tr("No mode"));
    for(const ChordDef& c: CHORDDEFS)
        if(c.isMode())
            _model.addItem(c.index(), c.name());
}
