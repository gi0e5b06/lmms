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

static inline float trianglef(const float x)
{
    if(x < 0.25f)
        return x * 4.f;
    else if(x < 0.75f)
        return 2.f - x * 4.f;
    else
        return x * 4.f - 4.f;
}

static inline float sawf(const float x)
{
    return -1.f + x * 2.f;
}

static inline float squaref(const float x)
{
    return (x > 0.5f) ? -1.f : 1.f;
}

static inline float moogsawf(const float x)
{
    if(x < 0.5f)
        return -1.f + x * 4.f;
    else
        return 1.f - 2.f * x;
}

static inline float normexpf(const float x)
{
    if(x < 0.5f)
        return -1.f + 8.f * x * x;
    else
        return -1.f + 8.f * (1.f - x) * (1.f - x);
}

static inline float randf(const float)
{
        return 2.f*fastrandf01inc()-1.f;
}


#define _QFAST 16
#define FFSZ1M 1048576*_QFAST/16
#define FFSZ100K 131072*_QFAST/16
#define FFSZ10K 16384*_QFAST/16
#define FFSZ1K 1024

FASTFUNC01_BODY(sqrtf, FFSZ10K)
FASTFUNC01_BODY(normsinf, FFSZ100K)
FASTFUNC01_BODY(trianglef, FFSZ1K)
FASTFUNC01_BODY(sawf, FFSZ1K)
FASTFUNC01_BODY(squaref, FFSZ1K)
FASTFUNC01_BODY(moogsawf, FFSZ1K)
FASTFUNC01_BODY(normexpf, FFSZ100K)
FASTFUNC01_BODY(randf, FFSZ100K)
