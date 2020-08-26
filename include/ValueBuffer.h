/*
 * ValueBuffer.h - a container class for passing buffers of model values
 * around
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef VALUE_BUFFER_H
#define VALUE_BUFFER_H

//#include <vector>
#include "MemoryManager.h"
#include "export.h"
#include "lmms_basics.h"

class EXPORT ValueBuffer  //: public std::vector<real_t>
{
    MM_OPERATORS

  public:
    ValueBuffer(const ValueBuffer* _vb);
    ValueBuffer(const ValueBuffer& _vb);
    ValueBuffer(int _length);
    virtual ~ValueBuffer();

    INLINE real_t value(int _i) const
    {
        Q_ASSERT(_i >= 0 && _i < m_len);
        return m_data[_i];  // % m_len];
    }

    /*
    INLINE real_t const * const values() const
    {
        return m_data;
    }
    */

    INLINE real_t* values() const
    {
        return m_data;
    }

    INLINE int length() const
    {
        return m_len;
    }

    INLINE void set(int _i, real_t _v)
    {
        Q_ASSERT(_i >= 0 && _i < m_len);
        m_data[_i] = _v;
    }

    INLINE void clear()
    {
        memset(m_data, 0, sizeof(real_t) * m_len);
    }

    INLINE void lock() const
    {
        const_cast<QMutex*>(&m_mutex)->lock();
    }

    INLINE void unlock() const
    {
        const_cast<QMutex*>(&m_mutex)->unlock();
    }

    INLINE long period() const
    {
        return m_period;
    }

    INLINE void setPeriod(long _period)
    {
        m_period = _period;
    }

    void copyFrom(const ValueBuffer* const _vb);
    void fill(real_t _value);
    void interpolate(real_t _start, real_t _end);

  private:
    QMutex  m_mutex;
    int     m_len;
    real_t* m_data;
    long    m_period;
};

#endif
