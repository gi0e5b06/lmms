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

class EXPORT ValueBuffer  //: public std::vector<float>
{
    MM_OPERATORS

  public:
    ValueBuffer(const ValueBuffer* _vb);
    ValueBuffer(int _length);
    ~ValueBuffer();

    inline float value(int _offset) const
    {
        return m_data[_offset % m_len];
    }

    inline const float* values() const
    {
        return m_data;
    }

    inline float* values()
    {
        return m_data;
    }

    inline int length() const
    {
        return m_len;
    }

    inline void set(int _i, float _v)
    {
        m_data[_i] = _v;
    }

    inline void clear()
    {
        memset(m_data, 0, sizeof(float) * m_len);
    }

    void copyFrom(const ValueBuffer* _vb);
    void fill(float _value);
    void interpolate(float _start, float _end);

  private:
    int    m_len;
    float* m_data;
};

#endif
