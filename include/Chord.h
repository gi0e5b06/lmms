/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Chord.h -
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

#ifndef CHORD_H
#define CHORD_H

#include "ComboBoxModel.h"
//#include "MemoryManager.h"
//#include "fft_helpers.h"
//#include "pitch.h"
//#include "lmms_basics.h"
//#include "lmms_math.h"

class ChordDef
{
  public:
    typedef QVector<int8_t> Semitones;

    ChordDef(int                       _index,
             const char*               _name,
             const ChordDef::Semitones _semitones,
             bool                      _mode);

    int size() const
    {
        return m_semitones.size();
    }

    bool isMode() const
    {
        return m_mode;
    }

    bool isEmpty() const
    {
        return size() == 0;
    }

    bool hasSemitone(int8_t _semitone) const;

    int8_t last() const
    {
        return m_semitones[size() - 1];
    }

    int index() const
    {
        return m_index;
    }

    const QString& name() const
    {
        return m_name;
    }

    // obsolete
    const QString& getName() const
    {
        return name();
    }

    int8_t operator[](int _n) const
    {
        if(_n < 0 || _n >= size())
        {
            qWarning("ChordDef: invalid index");
            return 0;
        }

        return m_semitones.at(_n);
    }

    /*
    static const ChordDef& atNum(int _num);
    static int             numOf(const ChordDef& _chord);
    */
    static int minIndex();
    static int maxIndex();
    static void map(const std::function<void(const ChordDef&)>& _f);

    static const ChordDef& findByName(const QString& _name);
    static const ChordDef& findByIndex(int _index);

    static void fillAllModel(ComboBoxModel& _model, bool _none = false);
    static void fillChordModel(ComboBoxModel& _model, bool _none = false);
    static void fillModeModel(ComboBoxModel& _model, bool _none = false);

  private:
    int                 m_index;
    QString             m_name;
    ChordDef::Semitones m_semitones;
    bool                m_mode;
};

#endif
