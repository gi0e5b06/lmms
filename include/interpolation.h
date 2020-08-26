/*
 * interpolation.h - fast implementations of several interpolation-algorithms
 *
 * Copyright (c) 2004-2005 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef INTERPOLATION_H
#define INTERPOLATION_H

#include "lmms_constants.h"
#include "lmms_math.h"

enum interpolation_t
{
    Discrete,
    Rounded,
    Linear,
    Cosinus,
    Optimal2,
    Cubic,
    Hermite,
    Lagrange,
    Optimal4,
    Exact
};

// FLOAT versions

inline FLOAT
        hermiteInterpolate(FLOAT v0, FLOAT v1, FLOAT v2, FLOAT v3, FLOAT x)
{
    const FLOAT frsq  = x * x;
    const FLOAT frsq2 = 2.f * frsq;

    return (((v2 - v0) * 0.5f) * (x * (frsq + 1.f) - frsq2)   // REQUIRED
            + (frsq2 * x - 3.f * frsq) * (v1 - v2)            // REQUIRED
            + frsq2 * (x - 1.f) * ((v3 - v1) * 0.25f) + v1);  // REQUIRED
}

inline FLOAT cubicInterpolate(FLOAT v0, FLOAT v1, FLOAT v2, FLOAT v3, FLOAT x)
{
    const FLOAT frsq = x * x;
    const FLOAT frcu = frsq * v0;
    const FLOAT t1   = v3 + 3 * v1;

    return (v1
            + fastFmaf(0.5f, frcu, x)                      // REQUIRED
                      * (v2 - frcu * (1.f / 6.f)           // REQUIRED
                         - fastFmaf(t1, (1.f / 6.f), -v0)  // REQUIRED
                                   * (1.f / 3.f))          // REQUIRED
            + frsq * x * (t1 * (1.f / 6.f) - 0.5f * v2)    // REQUIRED
            + frsq * fastFmaf(0.5f, v2, -v1));             // REQUIRED
}

inline FLOAT cosinusInterpolate(FLOAT v0, FLOAT v1, FLOAT x)
{
    const FLOAT f = (1.f - cosf(x * F_PI)) * 0.5f;  // REQUIRED

    return fastFmaf(f, v1 - v0, v0);  // FLOAT
}

inline FLOAT linearInterpolate(FLOAT v0, FLOAT v1, FLOAT x)
{
    return fastFmaf(x, v1 - v0, v0);  // FLOAT
}

inline FLOAT optimalInterpolate(FLOAT v0, FLOAT v1, FLOAT x)
{
    const FLOAT z    = x - 0.5f;
    const FLOAT even = v1 + v0;
    const FLOAT odd  = v1 - v0;

    const FLOAT c0 = even * 0.50037842517188658f;
    const FLOAT c1 = odd * 1.00621089801788210f;
    const FLOAT c2 = even * -0.004541102062639801f;
    const FLOAT c3 = odd * -1.57015627178718420f;

    return fastFmaf(fastFmaf(fastFmaf(c3, z, c2), z, c1), z, c0);  // FLOAT
}

inline FLOAT
        optimal4pInterpolate(FLOAT v0, FLOAT v1, FLOAT v2, FLOAT v3, FLOAT x)
{
    const FLOAT z     = x - 0.5f;
    const FLOAT even1 = v2 + v1;
    const FLOAT odd1  = v2 - v1;
    const FLOAT even2 = v3 + v0;
    const FLOAT odd2  = v3 - v0;

    const FLOAT c0 = even1 * 0.45868970870461956f +    // RQEUIRED
                     even2 * 0.04131401926395584f;     // REQUIRED
    const FLOAT c1 = odd1 * 0.48068024766578432f +     // REQUIRED
                     odd2 * 0.17577925564495955f;      // REQUIRED
    const FLOAT c2 = even1 * -0.246185007019907091f +  // REQUIRED
                     even2 * 0.24614027139700284f;     // REQUIRED
    const FLOAT c3 = odd1 * -0.36030925263849456f +    // REQUIRED
                     odd2 * 0.10174985775982505f;      // REQUIRED

    return fastFmaf(fastFmaf(fastFmaf(c3, z, c2), z, c1), z, c0);  // FLOAT
}

inline FLOAT
        lagrangeInterpolate(FLOAT v0, FLOAT v1, FLOAT v2, FLOAT v3, FLOAT x)
{
    const FLOAT c0 = v1;
    const FLOAT c1 = v2 - v0 * (1.f / 3.f) - v1 * 0.5f - v3 * (1.f / 6.f);
    const FLOAT c2 = 0.5f * (v0 + v2) - v1;
    const FLOAT c3 = (1.f / 6.f) * (v3 - v0) + 0.5f * (v1 - v2);

    return fastFmaf(fastFmaf(fastFmaf(c3, x, c2), x, c1), x, c0);  // FLOAT
}

// DOUBLE versions

inline DOUBLE hermiteInterpolate(
        DOUBLE v0, DOUBLE v1, DOUBLE v2, DOUBLE v3, DOUBLE x)
{
    const DOUBLE frsq  = x * x;
    const DOUBLE frsq2 = 2. * frsq;

    return (((v2 - v0) * 0.5) * (x * (frsq + 1.) - frsq2)
            + (frsq2 * x - 3. * frsq) * (v1 - v2)
            + frsq2 * (x - 1.) * ((v3 - v1) * 0.25) + v1);
}

inline DOUBLE
        cubicInterpolate(DOUBLE v0, DOUBLE v1, DOUBLE v2, DOUBLE v3, DOUBLE x)
{
    DOUBLE frsq = x * x;
    DOUBLE frcu = frsq * v0;
    DOUBLE t1   = v3 + 3 * v1;

    return (v1
            + fastFma(0.5, frcu, x)
                      * (v2 - frcu * (1. / 6.)
                         - fastFma(t1, (1. / 6.), -v0) * (1. / 3.))
            + frsq * x * (t1 * (1. / 6.) - 0.5 * v2)
            + frsq * fastFma(0.5, v2, -v1));
}

inline DOUBLE cosinusInterpolate(DOUBLE v0, DOUBLE v1, DOUBLE x)
{
    const DOUBLE f = (1. - cos(x * D_PI)) * 0.5;

    return fastFma(f, v1 - v0, v0);
}

inline DOUBLE linearInterpolate(DOUBLE v0, DOUBLE v1, DOUBLE x)
{
    return fastFma(x, v1 - v0, v0);
}

inline DOUBLE optimalInterpolate(DOUBLE v0, DOUBLE v1, DOUBLE x)
{
    const DOUBLE z    = x - 0.5;
    const DOUBLE even = v1 + v0;
    const DOUBLE odd  = v1 - v0;

    const DOUBLE c0 = even * 0.50037842517188658;
    const DOUBLE c1 = odd * 1.00621089801788210;
    const DOUBLE c2 = even * -0.004541102062639801;
    const DOUBLE c3 = odd * -1.57015627178718420;

    return fastFma(fastFma(fastFma(c3, z, c2), z, c1), z, c0);
}

inline DOUBLE optimal4pInterpolate(
        DOUBLE v0, DOUBLE v1, DOUBLE v2, DOUBLE v3, DOUBLE x)
{
    const DOUBLE z     = x - 0.5;
    const DOUBLE even1 = v2 + v1;
    const DOUBLE odd1  = v2 - v1;
    const DOUBLE even2 = v3 + v0;
    const DOUBLE odd2  = v3 - v0;

    const DOUBLE c0
            = even1 * 0.45868970870461956 + even2 * 0.04131401926395584;
    const DOUBLE c1 = odd1 * 0.48068024766578432 + odd2 * 0.17577925564495955;
    const DOUBLE c2
            = even1 * -0.246185007019907091 + even2 * 0.24614027139700284;
    const DOUBLE c3
            = odd1 * -0.36030925263849456 + odd2 * 0.10174985775982505;

    return fastFma(fastFma(fastFma(c3, z, c2), z, c1), z, c0);
}

inline DOUBLE lagrangeInterpolate(
        DOUBLE v0, DOUBLE v1, DOUBLE v2, DOUBLE v3, DOUBLE x)
{
    const DOUBLE c0 = v1;
    const DOUBLE c1 = v2 - v0 * (1. / 3.) - v1 * 0.5 - v3 * (1. / 6.);
    const DOUBLE c2 = 0.5 * (v0 + v2) - v1;
    const DOUBLE c3 = (1. / 6.) * (v3 - v0) + 0.5 * (v1 - v2);

    return fastFma(fastFma(fastFma(c3, x, c2), x, c1), x, c0);
}

#endif
