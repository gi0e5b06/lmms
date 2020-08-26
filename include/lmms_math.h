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

#include "lmms_basics.h"
#include "lmms_constants.h"
#include "lmmsconfig.h"

#include <QtGlobal>

#include <cmath>

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
#define exp10(x) pow(10., x)
#endif
#ifndef exp10f
#define exp10f(x) powf(10.f, x)
#endif
#endif

#define absFraction positivefraction

static inline real_t positivefraction(const real_t _x)
{
#ifdef REAL_IS_FLOAT
    real_t r = _x - floorf(_x);
    if(r < 0.f)
        r++;
    if(r >= 1.f)
        r--;
#endif
#ifdef REAL_IS_DOUBLE
    real_t r = _x - floor(_x);
    if(r < 0.)
        r++;
    if(r >= 1.)
        r--;
#endif
    return r;
}

static inline real_t fraction(const real_t _x)
{
#ifdef REAL_IS_FLOAT
    return _x - floorf(_x);
#endif
#ifdef REAL_IS_DOUBLE
    return _x - floor(_x);
#endif
}

/*
static inline double absFraction(const double _x)
{
    return _x - floor(_x) + (_x >= 0. ? 0. : 1.);
}

static inline double fraction(const double _x)
{
    return _x - floor(_x);
}

//#ifdef __INTEL_COMPILER

static inline FLOAT absFraction(const FLOAT _x)
{
    //return _x - (_x >= 0.f ? floorf(_x) : floorf(_x) - 1.f);
    return _x - floorf(_x) + (_x >= 0.f ? 0.f : 1.f); // REQUIRED
}

static inline FLOAT fraction(const FLOAT _x)
{
    return _x - floorf(_x); // REQUIRED
}

#else

static inline FLOAT absFraction(const FLOAT _x)
{
    return (_x - (_x >= 0.f ? int(_x) : int(_x) - 1.f)); // REQUIRED
}

static inline FLOAT fraction(const FLOAT _x)
{
    return (_x - static_cast<int>(_x));
}

#if 0
// SSE3-version
static inline FLOAT absFraction( FLOAT _x )
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

static inline FLOAT absFraction( FLOAT _x )
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
*/

static inline real_t folding(const real_t _x)
{
#ifdef REAL_IS_FLOAT
    real_t r = _x * 0.5f;
    if(r >= 0.f)
    {
        r = fraction(r);
        if(r > 0.5f)
            r = 1.f - r;
    }
    else
    {
        r = fraction(-r);
        if(r > 0.5f)
            r = 1.f - r;
        r = -r;
    }
    return 2.f * r;
#endif
#ifdef REAL_IS_DOUBLE
    real_t r = _x * 0.5;
    if(r >= 0.)
    {
        r = fraction(r);
        if(r > 0.5)
            r = 1. - r;
    }
    else
    {
        r = fraction(-r);
        if(r > 0.5)
            r = 1. - r;
        r = -r;
    }
    return 2. * r;
#endif
}

static inline real_t
        bound(const real_t _min, const real_t _val, const real_t _max)
{
    real_t r = _val;
    if(r >= _max)
        r = _max;
    if(r <= _min)
        r = _min;
    return r;
}

static inline real_t
        roundat(const real_t _val, const real_t _where, const real_t _step)
{
    return abs(_val - _where) <= _step ? _where : _val;
}

#define FAST_RAND_MAX 32767
static inline int fastrandi()
{
    static unsigned long next = 1;

    next = next * 1103515245 + 12345;
    return ((unsigned)(next / 65536) % 32768);
}

static inline real_t fastrand(real_t range)
{
#ifdef REAL_IS_DOUBLE
    static const double fast_rand_ratio
            = 1. / static_cast<double>(FAST_RAND_MAX);
    return static_cast<double>(fastrandi()) * range * fast_rand_ratio;
#endif
#ifdef REAL_IS_FLOAT
    static const float fast_rand_ratio
            = 1.f / static_cast<float>(FAST_RAND_MAX);
    return static_cast<float>(fastrandi()) * range * fast_rand_ratio;
#endif
}

static inline real_t fastrand01inc()
{
    return real_t(fastrandi()) / real_t(FAST_RAND_MAX);
}

static inline real_t fastrand01exc()
{
    return real_t(fastrandi()) / real_t(FAST_RAND_MAX + 1);
}

// obsolete
static inline FLOAT fastrandf01inc()
{
    return static_cast<FLOAT>(fastrandi())
           / static_cast<FLOAT>(FAST_RAND_MAX);
}

// obsolete
static inline FLOAT fastrandf01exc()
{
    return static_cast<FLOAT>(fastrandi())
           / static_cast<FLOAT>(FAST_RAND_MAX + 1);
}

#define fastFma fma
#define fastFmaf fmaf

/*
static inline FLOAT fma(FLOAT a, FLOAT b, FLOAT c)
{
    return fmaf(a, b, c); // REQUIRED
}
*/

/*
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
static inline FLOAT fastFmaf(FLOAT a, FLOAT b, FLOAT c)
{
#ifdef FP_FAST_FMAF
#ifdef __clang__
    return fma(a, b, c);
#else
    return fmaf(a, b, c); // REQUIRED
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
*/

static inline real_t fastexp(real_t a)
{
    return expf(a); // REQUIRED
}

static inline real_t fastexp2(real_t a)
{
    return exp2f(a);
}

// https://en.wikipedia.org/wiki/Fast_inverse_square_root
// not tested/used yet
/*
static inline real_t fastrsqrt(real_t x)
{
    const float x2         = static_cast<float>(x) * 0.5f;  // REQUIRED
    const float threehalfs = 1.5f;                          // REQUIRED

    union {
        float    f; // REQUIRED
        uint32_t i;
    } conv = {x};  // member 'f' set to value of 'number'.
    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= (threehalfs - (x2 * conv.f * conv.f));
    return conv.f;
}
*/

/*
#define fastPow fastpow

static inline FLOAT fastPowf(FLOAT a, FLOAT b)
{
   return fastPow(static_cast<double>(a), static_cast<double>(b));
}
*/

static inline real_t fastpow(real_t a, real_t b)
{
    return powf(a, b);  // REQUIRED
}

// http://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
// gives strange results
/*
static inline real_t fastpow(real_t a, real_t b)
{
    union {
        DOUBLE  d;
        int32_t x[2];
    } u    = {a};
    u.x[1] = static_cast<int32_t>(b * (u.x[1] - 1072632447) + 1072632447);
    u.x[0] = 0;
    return u.d;
}
*/

// sinc function
static inline real_t sinc(real_t _x)
{
#ifdef REAL_IS_FLOAT
    return _x == 0.f ? 1.f : sinf(F_PI * _x) / (F_PI * _x);
#endif
#ifdef REAL_IS_DOUBLE
    return _x == 0. ? 1. : sin(D_PI * _x) / (D_PI * _x);
#endif
}

//! @brief Exponential function that deals with negative bases
static inline real_t signedpow(real_t v, real_t e)
{
#ifdef REAL_IS_FLOAT
    return v < 0.f ? powf(-v, e) * -1.f : powf(v, e);  // REQUIRED
#endif
#ifdef REAL_IS_DOUBLE
    return v < 0. ? pow(-v, e) * -1. : pow(v, e);
#endif
}

// needed temporary for Nes
static inline FLOAT signedpowf(FLOAT v, FLOAT e)
{
    return v < 0.f ? powf(-v, e) * -1.f : powf(v, e);  // REQUIRED
}

/*
static inline double signedPow(double v, double e)
{
    return v < 0 ? pow(-v, e) * -1. : pow(v, e);
}

static inline FLOAT signedPowf(FLOAT v, FLOAT e)
{
    return v < 0 ? powf(-v, e) * -1.f : powf(v, e); // REQUIRED
}
*/

static inline double logToLinearScale(double min, double max, double value)
{
    if(min < 0)
    {
        const double mmax   = qMax(qAbs(min), qAbs(max));
        const double val    = value * (max - min) + min;
        const double result = signedpow(val / mmax, D_E) * mmax;
        return isnan(result) ? 0 : result;
    }
    double result = pow(value, D_E) * (max - min) + min;
    return isnan(result) ? 0 : result;
}

static inline double linearToLogScale(double min, double max, double value)
{
    static const double EXP          = 1. / D_E;
    const double        valueLimited = qBound(min, value, max);
    const double        val          = (valueLimited - min) / (max - min);
    if(min < 0)
    {
        const double mmax   = qMax(qAbs(min), qAbs(max));
        const double result = signedpow(valueLimited / mmax, EXP) * mmax;
        return isnan(result) ? 0 : result;
    }
    double result = pow(val, EXP) * (max - min) + min;
    return isnan(result) ? 0 : result;
}

//! @brief Scales @value from linear to logarithmic.
//! Value should be within [0,1]
static inline FLOAT logToLinearScale(FLOAT min, FLOAT max, FLOAT value)
{
    if(min < 0)
    {
        const FLOAT mmax   = qMax(qAbs(min), qAbs(max));
        const FLOAT val    = value * (max - min) + min;
        const FLOAT result = signedpowf(val / mmax, F_E) * mmax;
        return isnan(result) ? 0 : result;
    }
    FLOAT result = powf(value, F_E) * (max - min) + min;
    return isnan(result) ? 0 : result;
}

//! @brief Scales value from logarithmic to linear. Value should be in min-max
//! range.
static inline FLOAT linearToLogScale(FLOAT min, FLOAT max, FLOAT value)
{
    static const FLOAT EXP          = 1.0f / F_E;
    const FLOAT        valueLimited = qBound(min, value, max);
    const FLOAT        val          = (valueLimited - min) / (max - min);
    if(min < 0)
    {
        const FLOAT mmax   = qMax(qAbs(min), qAbs(max));
        const FLOAT result = signedpowf(valueLimited / mmax, EXP) * mmax;
        return isnan(result) ? 0 : result;
    }
    FLOAT result = powf(val, EXP) * (max - min) + min;
    return isnan(result) ? 0 : result;
}

//! @brief Converts linear amplitude (0-1.0) to dBFS scale. Handles zeroes as
//! -inf.
//! @param amp Linear amplitude, where 1.0 = 0dBFS.
//! @return Amplitude in dBFS. -inf for 0 amplitude.
static inline real_t safeAmpToDbfs(real_t amp)
{
#ifdef REAL_IS_FLOAT
    // return amp == 0.0f ? -INFINITY : log10f(amp) * 20.0f;
    return amp <= 0.0f ? -INFINITY : log10f(amp) * 20.0f;
#endif
#ifdef REAL_IS_DOUBLE
    return amp <= 0. ? -std::numeric_limits<real_t>::infinity()
                     : log10(amp) * 20.;
#endif
}

//! @brief Converts dBFS-scale to linear amplitude with 0dBFS = 1.0. Handles
//! infinity as zero.
//! @param dbfs The dBFS value to convert: all infinites are treated as -inf
//! and result in 0
//! @return Linear amplitude
static inline real_t safeDbfsToAmp(real_t dbfs)
{
#ifdef REAL_IS_FLOAT
    return isinf(dbfs) ? 0.0f : exp10f(dbfs * 0.05f);
#endif
#ifdef REAL_IS_DOUBLE
    return isinf(dbfs) ? 0. : exp10(dbfs * 0.05);
#endif
}

//! @brief Converts linear amplitude (>0-1.0) to dBFS scale.
//! @param amp Linear amplitude, where 1.0 = 0dBFS. ** Must be larger than
//! zero! **
//! @return Amplitude in dBFS.
static inline real_t ampToDbfs(real_t amp)
{
#ifdef REAL_IS_FLOAT
    return log10f(amp) * 20.f;
#endif
#ifdef REAL_IS_DOUBLE
    return log10(amp) * 20.;
#endif
}

//! @brief Converts dBFS-scale to linear amplitude with 0dBFS = 1.0
//! @param dbfs The dBFS value to convert. ** Must be a real number - not
//! inf/nan! **
//! @return Linear amplitude
static inline real_t dbfsToAmp(real_t dbfs)
{
#ifdef REAL_IS_FLOAT
    return exp10f(dbfs * 0.05f);
#endif
#ifdef REAL_IS_DOUBLE
    return exp10(dbfs * 0.05);
#endif
}

//! returns 1.0f if val >= 0.0f, -1.0 else
static inline real_t sign(real_t val)
{
#ifdef REAL_IS_FLOAT
    return val >= 0.f ? 1.f : -1.f;
#endif
#ifdef REAL_IS_DOUBLE
    return val >= 0. ? 1. : -1.;
#endif
}

//! if val >= 0.0f, returns sqrtf(val), else: -sqrtf(-val)
#define sqrt_neg sqrtneg
static inline real_t sqrtneg(real_t val)
{
#ifdef REAL_IS_FLOAT
    return sqrtf(fabs(val)) * sign(val);
#endif
#ifdef REAL_IS_DOUBLE
    return sqrt(abs(val)) * sign(val);
#endif
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

static inline real_t phasef(const real_t _x)
{
    real_t i;
#ifdef REAL_IS_DOUBLE
    return modf(_x, &i);  // REQUIRED
#endif
#ifdef REAL_IS_FLOAT
    return modf(_x, &i);  // REQUIRED
#endif
}

static inline real_t identityf(const real_t x)
{
    return x;
}

static inline real_t sqf(const real_t _x)
{
    return _x * _x;
}

static inline real_t cbf(const real_t _x)
{
    return _x * _x * _x;
}

static inline real_t ninvrampf(const real_t x)
{
    return 3. - 4. / (1. + x);
}

static inline real_t ninvdistf(const real_t x)
{
    return 2. / (1. + x) - 1.;
}

static inline real_t cornerrampf(const real_t x)
{
    // -(1-(2*x)**7)/(1+(2*x)**7)
    real_t y = pow(2. * x, 7);
    return -(1. - y) / (1. + y);
}

static inline real_t cornerpeakf(const real_t x)
{
    // -1-(1-(4*x)**6) / (1+(4*x)**6)-(1-(4*(1-x))**6) / (1+(4*(1-x))**6)
    real_t y0 = pow(4. * x, 6);
    real_t y1 = pow(4. * (1. - x), 6);
    return -1. - (1. - y0) / (1. + y0) - (1. - y1) / (1. + y1);
}

static inline real_t cornerdistf(const real_t x)
{
    // (1-x**8) / (1+x**8)
    real_t y = pow(x, 8);
    return (1. - y) / (1. + y);
}

static inline real_t profilpeakf(const real_t x)
{
    // cos(2*pi*x+pi)+10*x*(1-x)*sin(7*pi*x)/8
    return cos(2. * R_PI * x + R_PI)
           + 10. * x * (1. - x) * sin(7. * R_PI * x) / 8.;
}

static inline real_t nsinf(const real_t x)
{
#ifdef REAL_IS_DOUBLE
    return sin(x * D_2PI);
#endif
#ifdef REAL_IS_FLOAT
    return sinf(x * F_2PI);  // REQUIRED
#endif
}

static inline real_t trianglef(const real_t x)
{
    if(x < 0.25)
        return x * 4.;
    else if(x < 0.75)
        return 2. - x * 4.;
    else
        return x * 4. - 4.;
}

static inline real_t sawtoothf(const real_t x)
{
    return 1. - x * 2.;
}

static inline real_t sawtooth0pf(const real_t x)
{
    if(x < 0.5)
        return -2. * x;
    else
        return 1. - 2. * (x - 0.5);
}

static inline real_t rampf(const real_t x)
{
    return -1. + x * 2.;
}

static inline real_t ramp0pf(const real_t x)
{
    if(x < 0.5)
        return 2. * x;
    else
        return -1. + 2. * (x - 0.5);
}

static inline real_t squaref(const real_t x)
{
    return (x < 0.5) ? 1. : -1.;
}

static inline real_t harshrampf(const real_t x)
{
    if(x < 0.5)
        return -1. + x * 4.;
    else
        return 1. - 2. * x;
}

static inline real_t harshramp0pf(const real_t x)
{
    if(x < 0.25)
        return 4. * x;
    else if(x < 0.75)
        return 0.50 - 2. * x;
    else
        return 4. * x - 4.;
}

static inline real_t nsin2f(const real_t x)
{
#ifdef REAL_IS_DOUBLE
    return sin((x / 2. + 0.75) * D_2PI);
#endif
#ifdef REAL_IS_FLOAT
    return sinf((x / 2.f + 0.75f) * F_2PI);
#endif
}

static inline real_t nsin4f(const real_t x)
{
#ifdef REAL_IS_DOUBLE
    return 2. * sin(x * D_PI_2) - 1.;
#endif
#ifdef REAL_IS_FLOAT
    return 2f. * sinf(x * F_PI_2) - 1.f;
#endif
}

static inline real_t ncosf(const real_t x)
{
#ifdef REAL_IS_DOUBLE
    return (cos(x * D_2PI) + 1.) / 2.;
#endif
#ifdef REAL_IS_FLOAT
    return (cosf(x * F_2PI) + 1.f) / 2.f;
#endif
}

static inline real_t ncos2f(const real_t x)
{
#ifdef REAL_IS_DOUBLE
    return (cos(x * D_PI) + 1.) / 2.;
#endif
#ifdef REAL_IS_FLOAT
    return (cosf(x * F_PI) + 1.f) / 2.f;
#endif
}

static inline real_t ncos4f(const real_t x)
{
#ifdef REAL_IS_DOUBLE
    return cos(x * D_PI_2);
#endif
#ifdef REAL_IS_FLOAT
    return cosf(x * F_PI_2);  // REQUIRED
#endif
}

static inline real_t complementf(const real_t x)
{
    return 1 - x;
}

static inline real_t ngaussf(const real_t _x)
{
#ifdef REAL_IS_DOUBLE
    return (exp(-_x * _x)
            - 0.367879441355257019719005029401159845292568206787109375)
           / 0.632120558644742924769843739341013133525848388671875;
#endif
#ifdef REAL_IS_FLOAT
    return (expf(-_x * _x)
            - 0.367879441355257019719005029401159845292568206787109375f)
           / 0.632120558644742924769843739341013133525848388671875f;
#endif
}

static inline real_t sharpgaussf(const real_t _x)
{
#ifdef REAL_IS_DOUBLE
    return (exp(-7. * _x * _x)
            - 0.00091188196874393361544830494125335462740622460842132568359375)
           / 0.9990881180312560783107755923992954194545745849609375;
#endif
#ifdef REAL_IS_FLOAT
    return (expf(-7.f * _x * _x)
            - 0.00091188196874393361544830494125335462740622460842132568359375f)
           / 0.9990881180312560783107755923992954194545745849609375f;
#endif
}

static inline real_t octaviusrampf(const real_t x)
{
    real_t x2 = x * x;
    real_t x3 = x2 * x;
    real_t y = (x < 0.225) ? (-223.2124952380952 * x3 + 103.3947428571429 * x2
                              - 12.99710476190476 * x + 0.5)
                           : (4.634282931882935 * x3 - 7.651001253561262 * x2
                              + 4.391827775187776 * x - 0.37510945350945);
    return 2. * y - 1.;
}

static inline real_t octaviusramp0pf(const real_t x)
{
    return octaviusrampf(phasef(x + 0.566454));  // REQUIRED
}

static inline real_t sqpeakf(const real_t x)
{
    real_t p = x;
    if(p > 0.5)
        p = 1. - p;
    return -1. + 8. * p * p;
}

static inline real_t sqpeak0pf(const real_t x)
{
    return sqpeakf(phasef(x + 0.353553));  // REQUIRED
}

static inline real_t cbpeakf(const real_t x)
{
    real_t p = x;
    if(p > 0.5)
        p = 1. - p;
    return -1. + 16. * p * p * p;
}

static inline real_t cbpeak0pf(const real_t x)
{
    return cbpeakf(phasef(x + 0.603150));  // REQUIRED
}

static inline real_t randf(const real_t)
{
    return 2. * fastrand01inc() - 1.;
}

static inline real_t pulsef(const real_t x)
{
    return x < 0.5 ? 1. : 0.;
}

static inline real_t pulse0pf(const real_t x)
{
    if(x < 0.25)
        return 0.;
    else if(x < 0.75)
        return 1.;
    else
        return 0.;
}

static inline real_t moogrampf(const real_t x)
{
    return 3.41333 * x * x * x - 7.36 * x * x + 5.22667 * x - 1.;
}

static inline real_t moogramp0pf(const real_t x)
{
    const real_t p = phasef(x + 0.301297);  // REQUIRED
    return 3.41333 * p * p * p - 7.36 * p * p + 5.22667 * p - 1.;
}

static inline real_t moogsquaref(const real_t x)
{
    if(x < 0.5)
        return 1. / exp(2.5 * x);
    else
        return -(1. / exp(2.5 * (x - 0.5)));
}

real_t exptrif(const real_t x);

static inline real_t minus1f(const real_t x)
{
    return -1.;
}

static inline real_t minus05f(const real_t x)
{
    return -0.5;
}

static inline real_t zerof(const real_t x)
{
    return 0.;
}

static inline real_t plus05f(const real_t x)
{
    return 0.5;
}

static inline real_t plus1f(const real_t x)
{
    return 1.;
}

real_t nexpf(const real_t x);
real_t nlogf(const real_t x);

real_t nexp2f(const real_t x);
real_t nexp2rampf(const real_t x);

real_t nerf(const real_t x);

real_t fibonacci1(const real_t x);
real_t fibonacci2(const real_t x);

#endif
