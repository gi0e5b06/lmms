/*
 * Scale.h -
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
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

#ifndef SCALE_H
#define SCALE_H

#include "ComboBoxModel.h"
#include "MemoryManager.h"
//#include "fft_helpers.h"
//#include "pitch.h"
#include "lmms_basics.h"
//#include "lmms_math.h"

class Scale
{
    MM_OPERATORS

  public:
    Scale(const QString& _name,
          const int      _bank,
          const int      _index,
          const real_t   _baseFreq      = 440.,
          const real_t   _baseKey       = 69.,
          const real_t   _octaveFactor  = 2.,
          const real_t   _octaveKeys    = 12.,
          const real_t   _bendingFactor = 2.);
    Scale(const QString& _name,
          const int      _bank,
          const int      _index,
          const QString& _file);
    /*
    Scale(const QString& _name,
          const int      _bank,
          const int      _index,
          scale_t        _data);
    */
    virtual ~Scale();

    // convenient f() for later (curve, waveform)
    // x must be between 0. and 1.
    virtual real_t f(const real_t _x) const final;

    // x must be between 0. and 1.
    virtual real_t tune(const real_t _x) const final;

    virtual real_t bending(const real_t _x) const final;

    // k should be between 0. and 127.
    virtual real_t frequency(const real_t _key,
                             const real_t _cents) const final;

    inline const QString& name() const
    {
        return m_name;
    }

    inline const int bank() const
    {
        return m_bank;
    }

    inline const int index() const
    {
        return m_index;
    }

    static const Scale* get(const int _bank, const int _index);

    static void fillBankModel(ComboBoxModel& _model);
    static void fillIndexModel(ComboBoxModel& _model, const int _bank);

    // Standard waves
    static const int ET12_BANK  = 0;
    static const int ET12_INDEX = 0;
    static const int MIN_BANK   = 0;
    static const int MIN_INDEX  = 0;
    static const int MAX_BANK   = 127;
    static const int MAX_INDEX  = 127;

    static const Scale ET12;

    class Set
    {
      public:
        Set();
        const Scale* get(const int _bank, const int _index);
        void         set(const int _bank, const int _index, const Scale* _s);
        void         fillBankModel(ComboBoxModel& _model);
        void         fillIndexModel(ComboBoxModel& _model, const int _bank);

      private:
        QString      m_bankNames[MAX_BANK - MIN_BANK + 1];
        const Scale* m_stock[MAX_BANK - MIN_BANK + 1]
                            [MAX_INDEX - MIN_INDEX + 1];
    };

    static Set SCALES;

  protected:
    /*
    Scale(const QString& _name,
          const int      _bank,
          const int      _index,
          const real_t    _baseFreq      = 440.,
          const real_t    _baseKey       = 69.,
          const real_t    _octaveFactor  = 2.,
          const real_t    _octaveKeys    = 12.,
          const real_t    _bendingFactor = 2.);
    */

    void rebuild();
    void build();

    bool    m_built;
    QString m_name;
    int     m_bank;
    int     m_index;

    real_t m_baseFrequency;  // 440.
    real_t m_baseKey;        // 69.
    real_t m_octaveFactor;   // 2.
    real_t m_octaveKeys;     // 12.
    real_t m_bendingFactor;  // 2.

    QString m_file;
    real_t* m_data;
    int     m_size;  // size of the data -1
};

#endif
