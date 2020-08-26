/*
 * Note.h - declaration of class note which contains all informations about a
 *          note + definitions of several constants and enums
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

#ifndef NOTE_H
#define NOTE_H

#include "ComboBoxModel.h"
#include "MidiTime.h"
#include "SerializingObject.h"
#include "panning.h"
#include "volume.h"

//#include <QVector>

class DetuningHelper;

enum Keys
{
    Key_C   = 0,
    Key_CIS = 1,
    Key_DES = 1,
    Key_D   = 2,
    Key_DIS = 3,
    Key_ES  = 3,
    Key_E   = 4,
    Key_FES = 4,
    Key_F   = 5,
    Key_FIS = 6,
    Key_GES = 6,
    Key_G   = 7,
    Key_GIS = 8,
    Key_AS  = 8,
    Key_A   = 9,
    Key_AIS = 10,
    Key_B   = 10,
    Key_H   = 11
};

enum Octaves
{
    Octave_M1 = 0,
    Octave_0,
    Octave_1,
    Octave_2,
    Octave_3,
    Octave_4,
    DefaultOctave = Octave_4,
    Octave_5,
    Octave_6,
    Octave_7,
    Octave_8,
    Octave_9,
    Octave_10
    // , NumOctaves
};

const int WhiteKeysPerOctave = 7;
const int BlackKeysPerOctave = 5;
const int KeysPerOctave      = WhiteKeysPerOctave + BlackKeysPerOctave;
const int NumMidiKeys        = 128;
const int NumKeys            = 140;  // NumOctaves * KeysPerOctave;
const int DefaultKey = DefaultOctave * KeysPerOctave + Key_A;  // A4 69

const float MaxDetuning = 4 * 12.0f;

class EXPORT Note : public SerializingObject
{
  public:
    Note(const MidiTime& length   = MidiTime(0),
         const MidiTime& pos      = MidiTime(0),
         int             key      = DefaultKey,
         volume_t        volume   = DefaultVolume,
         panning_t       panning  = DefaultPanning,
         DetuningHelper* detuning = nullptr);
    Note(const Note& note);
    virtual ~Note();

    // used by GUI

    INLINE void setSelected(const bool selected)
    {
        m_selected = selected;
    }

    INLINE void setOldKey(const int oldKey)
    {
        m_oldKey = oldKey;
    }

    INLINE void setOldPos(const MidiTime& oldPos)
    {
        m_oldPos = oldPos;
    }

    INLINE void setOldLength(const MidiTime& oldLength)
    {
        m_oldLength = oldLength;
    }

    INLINE void setIsPlaying(const bool isPlaying)
    {
        m_isPlaying = isPlaying;
    }

    void setLength(const MidiTime& length);
    void setPos(const MidiTime& pos);
    void setKey(const int key);
    void quantizeLength(const int qGrid);
    void quantizePos(const int qGrid);

    virtual void setVolume(volume_t _volume);
    virtual void setPanning(panning_t _panning);
    virtual void setProbability(real_t _probability);
    virtual void setLegato(bool _legato);
    virtual void setMarcato(bool _marcato);
    virtual void setStaccato(bool _staccato);

    // function to compare two notes - must be called explictly when
    // using qSort.
    static bool lessThan(const Note* a, const Note* b)
    {
        const tick_t pa = a->pos().ticks();
        const tick_t pb = b->pos().ticks();
        if(pa != pb)
            return pa < pb;
        const int ka = a->key();
        const int kb = b->key();
        if(ka != kb)
            return ka < kb;
        const tick_t la = a->length().ticks();
        const tick_t lb = b->length().ticks();
        if(la != lb)
            return la < lb;
        return a < b;
    }

    INLINE bool selected() const
    {
        return m_selected;
    }

    INLINE int oldKey() const
    {
        return m_oldKey;
    }

    INLINE MidiTime oldPos() const
    {
        return m_oldPos;
    }

    INLINE MidiTime oldLength() const
    {
        return m_oldLength;
    }

    INLINE bool isPlaying() const
    {
        return m_isPlaying;
    }

    INLINE MidiTime endPos() const
    {
        return pos() + qMax<tick_t>(1, m_length);
    }

    INLINE const MidiTime& length() const
    {
        return m_length;
    }

    INLINE const MidiTime& pos() const
    {
        return m_pos;
    }

    // useless and confusing
    /*
    INLINE MidiTime pos(MidiTime basePos) const
    {
        // const int bp = basePos;
        return m_pos - basePos;  // bp;
    }
    */

    INLINE int key() const
    {
        return m_key;
    }

    INLINE volume_t getVolume() const
    {
        return m_volume;
    }

    uint8_t midiVelocity(int midiBaseVelocity) const
    {
        return qMin(MidiMaxVelocity, (uint8_t)(getVolume() * midiBaseVelocity
                                               / DefaultVolume));
    }

    INLINE panning_t getPanning() const
    {
        return m_panning;
    }

    uint8_t midiPanning() const
    {
        return panningToMidi(getPanning());
    }

    INLINE bool probability() const
    {
        return m_probability;
    }

    INLINE bool legato() const
    {
        return m_legato;
    }

    INLINE bool marcato() const
    {
        return m_marcato;
    }

    INLINE bool staccato() const
    {
        return m_staccato;
    }

    static INLINE const QString classNodeName()
    {
        return "note";
    }

    virtual QString nodeName() const
    {
        return classNodeName();
    }

    static MidiTime quantized(const MidiTime& m, const int qGrid);

    DetuningHelper* detuning() const
    {
        return m_detuning;
    }

    bool hasDetuningInfo() const;
    bool withinRange(tick_t tickStart, tick_t tickEnd) const;

    void createDetuning();

    static void    fillRootModel(ComboBoxModel& _model, bool _none = false);
    static int     findKeyNum(QString& _name);  // ex. 70
    static QString findKeyName(int _num);       // ex. A#4
    static QString findNoteName(int _num);      // ex. A#

  protected:
    virtual void saveSettings(QDomDocument& doc, QDomElement& parent);
    virtual void loadSettings(const QDomElement& _this);

  private:
    static void buildKeyTables();

    // for piano roll editing
    bool     m_selected;
    int      m_oldKey;
    MidiTime m_oldPos;
    MidiTime m_oldLength;
    bool     m_isPlaying;

    int             m_key;
    volume_t        m_volume;
    panning_t       m_panning;
    real_t          m_probability;
    bool            m_legato;
    bool            m_marcato;
    bool            m_staccato;
    MidiTime        m_length;
    MidiTime        m_pos;
    DetuningHelper* m_detuning;
};

// typedef QVector<Note*> NoteVector;
typedef QVector<Note*> Notes;

typedef QVector<const Note*> Chord;
typedef QVector<Chord>       Chords;

#endif
