/*
 * lmms_float.h - defines missing math functions for double
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
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

#ifndef LMMS_FLOAT_H
#define LMMS_FLOAT_H

#include "lmms_math.h" // REQUIRED

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
