/*
 * lmms_math.h - defines math functions
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

#include "lmms_math.h"

/*

#define FASTFUNC01_BODY(name, FFSZ)                           \
    float name##01_DATA [FFSZ + 1];                           \
    void  init_fast##name##01()                               \
    {                                                         \
        for(int i = FFSZ; i >= 0; --i)                        \
            name##01_DATA [i] = name(float(i) / float(FFSZ)); \
    }                                                         \
    float fast##name##01(const float x)                       \
    {                                                         \
        return name##01_DATA [int(x * float(FFSZ))];          \
    }                                                         \
    float linear##name##01(const float x)                     \
    {                                                         \
        const float j  = x * float(FFSZ);                     \
        const int   i  = int(j);                              \
        const float d  = j - i;                               \
        const float r0 = name##01_DATA [i];                   \
        const float r1 = name##01_DATA [i + 1];               \
        return r0 + d * (r1 - r0);                            \
    }


#define _QFAST 16
#define FFSZ1M 1048576*_QFAST/16
#define FFSZ100K 131072*_QFAST/16
#define FFSZ10K 16384*_QFAST/16
#define FFSZ1K 1024

FASTFUNC01_BODY(sqrtf, FFSZ10K)
FASTFUNC01_BODY(nsinf, FFSZ100K)
FASTFUNC01_BODY(trianglef, FFSZ1K)
FASTFUNC01_BODY(sawtoothf, FFSZ1K)
FASTFUNC01_BODY(squaref, FFSZ1K)
FASTFUNC01_BODY(harshsawf, FFSZ1K)
FASTFUNC01_BODY(peakexpf, FFSZ100K)
FASTFUNC01_BODY(randf, FFSZ100K)

*/
