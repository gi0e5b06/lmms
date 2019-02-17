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

real_t exptrif(const real_t x)
{
    if(x < 0.25f)
        return expm1(8.f * x) * 0.15651764274967f;
    else if(x < 0.50f)
        return expm1(4.f - 8.f * x) * 0.15651764274967f;
    else if(x < 0.75f)
        return -expm1(8.f * x - 4.f) * 0.15651764274967f;
    else
        return -expm1(8.f - 8.f * x) * 0.15651764274967f;
}

real_t nexpf(const real_t x)
{
    return expm1(x) /*(exp(x) - 1.f)*/
           / 1.718281828459f;
}

real_t nlogf(const real_t x)
{
    return log(x * 1.718281828459f + 1.f);
}

real_t nexp2f(const real_t x)
{
        return exp2(x) - 1.;
}

real_t nexp2rampf(const real_t x)
{
    return (exp2(x) - 1.) * 2. - 1.;
}

real_t nerf(const real_t x)
{
    return erf(6. * (x - 0.5))
           / 0.99997790950300136092465663750772364437580108642578125;
}
real_t fibonacci1(const real_t x)
{
    int n = int(13. * x) - 1;
    int a = 0;
    int b = 1;
    if(n == -1)
        return 0;
    if(n == 0)
        return 1. / 144.;
    int c = 0;
    while(n > 0)
    {
        n--;
        c = a + b;
        a = b;
        b = c;
    }
    return c / 144.;
}

real_t fibonacci2(const real_t xmax)
{
    double x = 0.;
    int    a = 0;
    int    b = 1;
    int    c = 0;
    int    r = 0;
    while(x < xmax)
    {
        c = a + b;
        a = b;
        b = c;
        x += c / 377.;
        r = 1 - r;
    }
    return r;
}
