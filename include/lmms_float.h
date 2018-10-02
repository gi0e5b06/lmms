/*
 * lmms_math.h - defines math functions
 *
 * Copyright (c) 2004-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef LMMS_FLOAT_H
#define LMMS_FLOAT_H

#include "lmms_math.h"

#ifdef REAL_IS_DOUBLE

static inline real_t cbrtf(const real_t _x)
{
        return cbrt(_x);
}

static inline real_t sqrtf(const real_t _x)
{
        return sqrt(_x);
}


#endif

// fast approximation of square root (not used)
/*
static inline float fastSqrt( float n )
{
        union
        {
                int32_t i;
                float f;
        } u;
        u.f = n;
        u.i = ( u.i + ( 127 << 23 ) ) >> 1;
        return u.f;
}
*/

#endif
