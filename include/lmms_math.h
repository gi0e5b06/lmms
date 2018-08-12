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

#ifndef LMMS_MATH_H
#define LMMS_MATH_H

/*
#include <cmath>
#include <cstdint>

#include <QtCore/QtGlobal>
*/

#include <math.h>
#include <QtGlobal>

#include "lmms_constants.h"
#include "lmmsconfig.h"
using namespace std;

#if defined(LMMS_BUILD_WIN32) || defined(LMMS_BUILD_APPLE)   \
        || defined(LMMS_BUILD_HAIKU) || defined(__FreeBSD__) \
        || defined(__OpenBSD__)
#ifndef isnanf
#define isnanf(x) isnan(x)
#endif
#ifndef isinff
#define isinff(x) isinf(x)
#endif
#ifndef _isnanf
#define _isnanf(x) isnan(x)
#endif
#ifndef _isinff
#define _isinff(x) isinf(x)
#endif
#ifndef exp10
#define exp10(x) pow(10.0, x)
#endif
#ifndef exp10f
#define exp10f(x) powf(10.0f, x)
#endif
#endif

#ifdef __INTEL_COMPILER

static inline float absFraction(const float _x)
{
    return (_x - (_x >= 0.f ? floorf(_x) : floorf(_x) - 1.f));
}

static inline float fraction(const float _x)
{
    return (_x - floorf(_x));
}

#else

static inline float absFraction(const float _x)
{
    return (_x
            - (_x >= 0.f ? static_cast<int>(_x) : static_cast<int>(_x) - 1.f));
}

static inline float fraction(const float _x)
{
    return (_x - static_cast<int>(_x));
}

#if 0
// SSE3-version
static inline float absFraction( float _x )
{
	unsigned int tmp;
	asm(
		"fld %%st\n\t"
		"fisttp %1\n\t"
		"fild %1\n\t"
		"ftst\n\t"
		"sahf\n\t"
		"jae 1f\n\t"
		"fld1\n\t"
		"fsubrp %%st, %%st(1)\n\t"
	"1:\n\t"
		"fsubrp %%st, %%st(1)"
		: "+t"( _x ), "=m"( tmp )
		:
		: "st(1)", "cc" );
	return( _x );
}

static inline float absFraction( float _x )
{
	unsigned int tmp;
	asm(
		"fld %%st\n\t"
		"fisttp %1\n\t"
		"fild %1\n\t"
		"fsubrp %%st, %%st(1)"
		: "+t"( _x ), "=m"( tmp )
		:
		: "st(1)" );
	return( _x );
}
#endif

#endif

#define FAST_RAND_MAX 32767
static inline int fast_rand()
{
    static unsigned long next = 1;
    next                      = next * 1103515245 + 12345;
    return ((unsigned)(next / 65536) % 32768);
}

static inline double fastRand(double range)
{
    static const double fast_rand_ratio = 1.0 / FAST_RAND_MAX;
    return fast_rand() * range * fast_rand_ratio;
}

static inline float fastRandf(float range)
{
    static const float fast_rand_ratio = 1.0f / FAST_RAND_MAX;
    return fast_rand() * range * fast_rand_ratio;
}

static inline float fastrandf01inc()
{
    return ((float)fast_rand()) / ((float)FAST_RAND_MAX);
}

static inline float fastrandf01exc()
{
    return ((float)fast_rand()) / ((float)(FAST_RAND_MAX + 1));
}

//! @brief Takes advantage of fmal() function if present in hardware
static inline long double
        fastFmal(long double a, long double b, long double c)
{
#ifdef FP_FAST_FMAL
#ifdef __clang__
    return fma(a, b, c);
#else
    return fmal(a, b, c);
#endif
#else
    return a * b + c;
#endif
}

//! @brief Takes advantage of fmaf() function if present in hardware
static inline float fastFmaf(float a, float b, float c)
{
#ifdef FP_FAST_FMAF
#ifdef __clang__
    return fma(a, b, c);
#else
    return fmaf(a, b, c);
#endif
#else
    return a * b + c;
#endif
}

//! @brief Takes advantage of fma() function if present in hardware
static inline double fastFma(double a, double b, double c)
{
#ifdef FP_FAST_FMA
    return fma(a, b, c);
#else
    return a * b + c;
#endif
}

// source:
// http://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
static inline double fastPow(double a, double b)
{
    union {
        double  d;
        int32_t x[2];
    } u    = {a};
    u.x[1] = static_cast<int32_t>(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}

// sinc function
static inline double sinc(double _x)
{
    return _x == 0.0 ? 1.0 : sin(F_PI * _x) / (F_PI * _x);
}

//! @brief Exponential function that deals with negative bases
static inline float signedPowf(float v, float e)
{
    return v < 0 ? powf(-v, e) * -1.0f : powf(v, e);
}

//! @brief Scales @value from linear to logarithmic.
//! Value should be within [0,1]
static inline float logToLinearScale(float min, float max, float value)
{
    if(min < 0)
    {
        const float mmax   = qMax(qAbs(min), qAbs(max));
        const float val    = value * (max - min) + min;
        float       result = signedPowf(val / mmax, F_E) * mmax;
        return isnan(result) ? 0 : result;
    }
    float result = powf(value, F_E) * (max - min) + min;
    return isnan(result) ? 0 : result;
}

//! @brief Scales value from logarithmic to linear. Value should be in min-max
//! range.
static inline float linearToLogScale(float min, float max, float value)
{
    static const float EXP          = 1.0f / F_E;
    const float        valueLimited = qBound(min, value, max);
    const float        val          = (valueLimited - min) / (max - min);
    if(min < 0)
    {
        const float mmax   = qMax(qAbs(min), qAbs(max));
        float       result = signedPowf(valueLimited / mmax, EXP) * mmax;
        return isnan(result) ? 0 : result;
    }
    float result = powf(val, EXP) * (max - min) + min;
    return isnan(result) ? 0 : result;
}

//! @brief Converts linear amplitude (0-1.0) to dBFS scale. Handles zeroes as
//! -inf.
//! @param amp Linear amplitude, where 1.0 = 0dBFS.
//! @return Amplitude in dBFS. -inf for 0 amplitude.
static inline float safeAmpToDbfs(float amp)
{
    return amp == 0.0f ? -INFINITY : log10f(amp) * 20.0f;
}

//! @brief Converts dBFS-scale to linear amplitude with 0dBFS = 1.0. Handles
//! infinity as zero.
//! @param dbfs The dBFS value to convert: all infinites are treated as -inf
//! and result in 0
//! @return Linear amplitude
static inline float safeDbfsToAmp(float dbfs)
{
    return isinf(dbfs) ? 0.0f : exp10(dbfs * 0.05f);
}

//! @brief Converts linear amplitude (>0-1.0) to dBFS scale.
//! @param amp Linear amplitude, where 1.0 = 0dBFS. ** Must be larger than
//! zero! **
//! @return Amplitude in dBFS.
static inline float ampToDbfs(float amp)
{
    return log10f(amp) * 20.0f;
}

//! @brief Converts dBFS-scale to linear amplitude with 0dBFS = 1.0
//! @param dbfs The dBFS value to convert. ** Must be a real number - not
//! inf/nan! **
//! @return Linear amplitude
static inline float dbfsToAmp(float dbfs)
{
    return exp10(dbfs * 0.05f);
}

//! returns 1.0f if val >= 0.0f, -1.0 else
static inline float sign(float val)
{
    return val >= 0.0f ? 1.0f : -1.0f;
}

//! if val >= 0.0f, returns sqrtf(val), else: -sqrtf(-val)
static inline float sqrt_neg(float val)
{
    return sqrtf(fabs(val)) * sign(val);
}

//! returns value furthest from zero
template <class T>
static inline T absMax(T a, T b)
{
    return qAbs<T>(a) > qAbs<T>(b) ? a : b;
}

//! returns value nearest to zero
template <class T>
static inline T absMin(T a, T b)
{
    return qAbs<T>(a) < qAbs<T>(b) ? a : b;
}

static inline float phasef(const float _x)
{
    float i;
    return modf(_x, &i);
}

static inline float sqf(const float _x)
{
    return _x * _x;
}

static inline float cbf(const float _x)
{
    return _x * _x * _x;
}

static inline float nsinf(const float _x)
{
    return sinf(_x * F_2PI);
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

static inline float sawtoothf(const float x)
{
    return -1.f + x * 2.f;
}

static inline float sawtooth0pf(const float x)
{
    if(x < 0.5f)
        return 2.f * x;
    else
        return -1.f + 2.f * (x - 0.5f);
}

static inline float squaref(const float x)
{
    return (x < 0.5f) ? 1.f : -1.f;
}

static inline float harshsawf(const float x)
{
    if(x < 0.5f)
        return -1.f + x * 4.f;
    else
        return 1.f - 2.f * x;
}

static inline float harshsaw0pf(const float x)
{
    if(x < 0.25f)
        return 4.f * x;
    else if(x < 0.75f)
        return 0.50f - 2.f * x;
    else
        return 4.f * x - 4.f;
}

static inline float sqpeakf(const float x)
{
    float p = x;
    if(p > 0.5f)
        p = 1.0f - p;
    return -1.0f + 8.0f * p * p;
}

static inline float sqpeak0pf(const float x)
{
    return sqpeakf(phasef(x + 0.353553f));
}

static inline float cbpeakf(const float x)
{
    float p = x;
    if(p > 0.5f)
        p = 1.0f - p;
    return -1.0f + 16.0f * p * p * p;
}

static inline float cbpeak0pf(const float x)
{
    return cbpeakf(phasef(x + 0.603150f));
}

static inline float randf(const float)
{
    return 2.f * fastrandf01inc() - 1.f;
}

static inline float pulsef(const float x)
{
    return x < 0.5f ? 1.f : 0.f;
}

static inline float pulse0pf(const float x)
{
    if(x < 0.25f)
        return 0.f;
    else if(x < 0.75f)
        return 1.f;
    else
        return 0.f;
}

static inline float moogsawf(const float x)
{
    return 3.41333f * x * x * x - 7.36 * x * x + 5.22667 * x - 1.f;
}

static inline float moogsaw0pf(const float x)
{
    const float p = phasef(x + 0.301297f);
    return 3.41333f * p * p * p - 7.36 * p * p + 5.22667 * p - 1.f;
}

static inline float moogsquaref(const float x)
{
    if(x < 0.5f)
        return 1.f / exp(2.5f * x);
    else
        return - (1.f / exp(2.5f * (x - 0.5f)));
}

static inline float expsawf(const float x)
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

static inline float minus1f(const float x)
{
    return -1.f;
}

static inline float minus05f(const float x)
{
    return -0.5f;
}

static inline float zerof(const float x)
{
    return 0.f;
}

static inline float plus05f(const float x)
{
    return 0.5f;
}

static inline float plus1f(const float x)
{
    return 1.f;
}

static inline float nexpf(const float x)
{
    return expm1(x) /*(exp(x) - 1.f)*/
           / 1.718281828459f;
}

static inline float nlogf(const float x)
{
    return log(x * 1.718281828459f + 1.f);
}

// nerf erf()
// nexp2 exp2()

/*
#define FASTFUNC01_HEADER(name)          \
    void  init_fast##name##01();         \
    float fast##name##01(const float x); \
    float linear##name##01(const float x);

FASTFUNC01_HEADER(sqrtf)
FASTFUNC01_HEADER(nsinf)
FASTFUNC01_HEADER(trianglef)
FASTFUNC01_HEADER(sawtoothf)
FASTFUNC01_HEADER(squaref)
FASTFUNC01_HEADER(harshsawf)
FASTFUNC01_HEADER(peakexpf)
FASTFUNC01_HEADER(randf)
*/

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
