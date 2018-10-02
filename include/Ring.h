/*
 * RingBuffer.h - an effective and flexible implementation of a ringbuffer for
 * LMMS
 *
 * Copyright (c) 2014 Vesa Kivimäki
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

#ifndef RING_H
#define RING_H

#include "MemoryManager.h"
#include "lmms_basics.h"
//#include "lmms_math.h"

#include <QObject>

#include <cmath>

class EXPORT Ring : public QObject
{
    Q_OBJECT
    MM_OPERATORS

  public:
    enum OpMode
    {
        Nop,
        Put,
        Add,
        PutMul,
        AddMul
    };

    Ring(f_cnt_t _size);
    Ring(real_t _duration);
    virtual ~Ring();

    void reset();

    sample_t value(f_cnt_t _offset, ch_cnt_t _ch);
    sample_t value(f_cnt_t _offset, ch_cnt_t _ch, f_cnt_t _pos);

    const real_t maxLevel() const
    {
        return m_maxLevel;
    }

    const bool isFrozen() const
    {
        return m_frozen;
    }

    void setFrozen(bool _b)
    {
        m_frozen = _b;
    }

    const f_cnt_t size() const
    {
        return m_size;
    }
    const int duration() const
    {
        return m_duration;
    }
    void changeSize(f_cnt_t _size);
    void changeDuration(real_t _duration);

    void rewind(f_cnt_t _frames);
    void forward(f_cnt_t _frames);
    void rewind(real_t _ms);
    void forward(real_t _ms);

    bool isSampleRateAware();
    void setSampleRateAware(bool _b);

    void read(sampleFrame& _dst) const;
    void read(sampleFrame* _dst, const f_cnt_t _length) const;
    void read(sampleFrame*  _dst,
              const f_cnt_t _length,
              const f_cnt_t _pos) const;
    void read(sampleFrame*  _dst,
              const f_cnt_t _length,
              const real_t  _pos) const;

    void write(const sampleFrame& _src,
               const OpMode       _op     = Put,
               const real_t       _factor = 1.f);
    void write(const sampleFrame* _src,
               const f_cnt_t      _length,
               const OpMode       _op     = Put,
               const real_t       _factor = 1.f);
    void write(const sampleFrame* _src,
               const f_cnt_t      _length,
               const f_cnt_t      _offset,
               const OpMode       _op     = Put,
               const real_t       _factor = 1.f);
    void write(const sampleFrame* _src,
               const f_cnt_t      _length,
               const real_t       _offset,
               const OpMode       _op     = Put,
               const real_t       _factor = 1.f);

  protected slots:
    void updateSamplerate();

  protected:
    inline f_cnt_t msToFrames(const real_t _ms) const
    {
        return static_cast<f_cnt_t>(
                ceilf(_ms * (real_t)m_sampleRate * 0.001f));
    }

    inline real_t framesToMs(const f_cnt_t _size) const
    {
        return real_t(_size) * 1000.f / (real_t)m_sampleRate;
    }

    inline f_cnt_t relpos(const f_cnt_t _offset) const
    {
        return abspos(m_position + _offset);
    }

    inline f_cnt_t relpos(const real_t _offset) const
    {
        return relpos(msToFrames(_offset));
    }

    /*inline*/ f_cnt_t abspos(const f_cnt_t _pos) const
    {
        f_cnt_t r = _pos;
        Q_ASSERT(m_size > 0);
        if(r < 0)
            r = (r % m_size) + m_size;
        r %= m_size;

        Q_ASSERT(r >= 0);
        Q_ASSERT(r < m_size);
        return r;
    }

    inline f_cnt_t abspos(const real_t _pos) const
    {
        return abspos(msToFrames(_pos));
    }

    // const fpp_t m_fpp;
    real_t           m_maxLevel;
    volatile bool    m_frozen;
    bool             m_sampleRateAware;
    sample_rate_t    m_sampleRate;
    f_cnt_t          m_size;
    real_t           m_duration;
    sampleFrame*     m_buffer;
    volatile f_cnt_t m_position;
};
#endif
