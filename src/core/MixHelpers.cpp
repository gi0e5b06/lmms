/*
 * MixHelpers.cpp - helper functions for mixing buffers
 *
 * Copyright (c) 2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "MixHelpers.h"

#include "ValueBuffer.h"
#include "lmms_math.h"  // REQUIRED

namespace MixHelpers
{

/*! \brief Function for applying MIXOP on all sample frames */
template <typename MIXOP>
static inline void run(sampleFrame*       dst,
                       const sampleFrame* src,
                       int                frames,
                       const MIXOP&       OP)
{
    for(int i = 0; i < frames; ++i)
    {
        OP(dst[i], src[i]);
    }
}

/*! \brief Function for applying MIXOP on all sample frames - split source */
template <typename MIXOP>
static inline void run(sampleFrame*    dst,
                       const sample_t* srcLeft,
                       const sample_t* srcRight,
                       int             frames,
                       const MIXOP&    OP)
{
    for(int i = 0; i < frames; ++i)
    {
        const sampleFrame src = {srcLeft[i], srcRight[i]};
        OP(dst[i], src);
    }
}

int16_t convertToS16(FLOAT _v)
{
    if(_v >= 1.f)  // FLOAT
        return I_S16_MULTIPLIER;
    else if(_v <= -1.f)  // FLOAT
        return -I_S16_MULTIPLIER - 1;
    else if(_v >= 0.f)  // FLOAT
        _v *= F_S16_MULTIPLIER;
    else
        _v *= F_S16_MULTIPLIER + 1.f;  // FLOAT
    return _v;
}

int32_t convertToS32(FLOAT _v)
{
    if(_v >= 1.f)  // FLOAT
        return I_S32_MULTIPLIER;
    else if(_v <= -1.f)  // FLOAT
        return -I_S32_MULTIPLIER - 1;
    else if(_v >= 0.f)  // FLOAT
        _v *= F_S32_MULTIPLIER;
    else
        _v *= F_S32_MULTIPLIER + 1.f;  // FLOAT
    return _v;
}

int64_t convertToS64(FLOAT _v)
{
    if(_v >= 1.f)  // FLOAT
        return I_S64_MULTIPLIER;
    else if(_v <= -1.f)  // FLOAT
        return -I_S64_MULTIPLIER - 1;
    else if(_v >= 0.f)  // FLOAT
        _v *= F_S64_MULTIPLIER;
    else
        _v *= F_S64_MULTIPLIER + 1.f;  // FLOAT
    return _v;
}

int16_t convertToS16(double _v)
{
    if(_v >= 1.)
        return I_S16_MULTIPLIER;
    else if(_v <= -1.)
        return -I_S16_MULTIPLIER - 1;
    else if(_v >= 0.)
        _v *= D_S16_MULTIPLIER;
    else
        _v *= D_S16_MULTIPLIER + 1.;
    return _v;
}

int32_t convertToS32(double _v)
{
    if(_v >= 1.)
        return I_S32_MULTIPLIER;
    else if(_v <= -1.)
        return -I_S32_MULTIPLIER - 1;
    else if(_v >= 0.)
        _v *= D_S32_MULTIPLIER;
    else
        _v *= D_S32_MULTIPLIER + 1.;
    return _v;
}

int64_t convertToS64(double _v)
{
    if(_v >= 1.)
        return I_S64_MULTIPLIER;
    else if(_v <= -1.)
        return -I_S64_MULTIPLIER - 1;
    else if(_v >= 0.)
        _v *= D_S64_MULTIPLIER;
    else
        _v *= D_S64_MULTIPLIER + 1.;
    return _v;
}

sample_t convertFromS16(int16_t _v)
{
#ifdef REAL_IS_DOUBLE
    return sample_t(_v)
           / (_v >= 0 ? D_S16_MULTIPLIER : D_S16_MULTIPLIER + 1.);
#else
    return sample_t(_v)
           / (_v >= 0 ? F_S16_MULTIPLIER : F_S16_MULTIPLIER + 1.);
#endif
}

sample_t convertFromS32(int32_t _v)
{
#ifdef REAL_IS_DOUBLE
    return sample_t(_v)
           / (_v >= 0 ? D_S32_MULTIPLIER : D_S32_MULTIPLIER + 1.);
#else
    return sample_t(_v)
           / (_v >= 0 ? F_S32_MULTIPLIER : F_S32_MULTIPLIER + 1.);
#endif
}

sample_t convertFromS64(int64_t _v)
{
#ifdef REAL_IS_DOUBLE
    return sample_t(_v)
           / (_v >= 0 ? D_S64_MULTIPLIER : D_S64_MULTIPLIER + 1.);
#else
    return sample_t(_v)
           / (_v >= 0 ? F_S64_MULTIPLIER : F_S64_MULTIPLIER + 1.);
#endif
}

/*! \brief Function for detecting silence - returns true if found */
bool isSilent(const sampleFrame* _src, const f_cnt_t _frames)
{
    for(f_cnt_t f = _frames - 1; f >= 0; f -= 4)  //--f
    {
        if(abs(_src[f][0]) >= SILENCE || abs(_src[f][1]) >= SILENCE)
            return false;
    }

    return true;
}

/*! \brief Function for detecting clipping - returns true if found */
bool isClipping(const sampleFrame* _src, const f_cnt_t _frames)
{
    for(f_cnt_t f = _frames - 1; f >= 0; --f)
    {
        if(abs(_src[f][0]) > 1. || abs(_src[f][1]) > 1.)
            return true;
    }
    return false;
}

/*! \brief Function for sanitizing a buffer of infs/nans - returns true if
 * modified */
bool sanitize(sampleFrame* _src, const f_cnt_t _frames)
{
    bool found = false;
    for(f_cnt_t f = _frames - 1; f >= 0; --f)
    {
        const sample_t s0 = _src[f][0];
        if(isinf(s0) || isnan(s0))
        {
            _src[f][0] = 0.;
            found      = true;
        }
        else if(abs(s0) <= SILENCE)
        {
            _src[f][0] = 0.;
        }

        const sample_t s1 = _src[f][1];
        if(isinf(s1) || isnan(s1))
        {
            _src[f][1] = 0.;
            found      = true;
        }
        else if(abs(s1) <= SILENCE)
        {
            _src[f][1] = 0.;
        }
    }
    return found;
}

/*! \brief Function for unclipping a buffer - returns true if modified */
bool unclip(sampleFrame* _src, const f_cnt_t _frames)
{
    bool found = false;
    for(f_cnt_t f = _frames - 1; f >= 0; --f)
    {
        if(_src[f][0] < -1.)
        {
            _src[f][0] = -1.;
            found      = true;
        }
        else if(_src[f][0] > 1.)
        {
            _src[f][0] = 1.;
            found      = true;
        }

        if(_src[f][1] < -1.)
        {
            _src[f][1] = -1.;
            found      = true;
        }
        else if(_src[f][1] > 1.)
        {
            _src[f][1] = 1.;
            found      = true;
        }
    }
    return found;
}

void addMultiplied(sampleFrame*       _dst,
                   const sampleFrame* _src,
                   const ValueBuffer* _coeffSrcBuf,
                   const f_cnt_t      _frames)
{
    // const real_t* const values = _coeffSrcBuf->values();
    for(f_cnt_t f = _frames - 1; f >= 0; --f)
    {
        _dst[f][0] += _src[f][0] * _coeffSrcBuf->value(f);  // values[f];
        _dst[f][1] += _src[f][1] * _coeffSrcBuf->value(f);  // values[f];
    }
}

struct AddOp
{
    void operator()(sampleFrame& dst, const sampleFrame& src) const
    {
        dst[0] += src[0];
        dst[1] += src[1];
    }
};

void add(sampleFrame* dst, const sampleFrame* src, int frames)
{
    run<>(dst, src, frames, AddOp());
}

struct AddMultipliedOp
{
    AddMultipliedOp(real_t coeff) : m_coeff(coeff)
    {
    }

    void operator()(sampleFrame& dst, const sampleFrame& src) const
    {
        dst[0] += src[0] * m_coeff;
        dst[1] += src[1] * m_coeff;
    }

    const real_t m_coeff;
};

void addMultiplied(sampleFrame*       dst,
                   const sampleFrame* src,
                   real_t             coeffSrc,
                   int                frames)
{
    run<>(dst, src, frames, AddMultipliedOp(coeffSrc));
}

struct AddSwappedMultipliedOp
{
    AddSwappedMultipliedOp(real_t coeff) : m_coeff(coeff)
    {
    }

    void operator()(sampleFrame& dst, const sampleFrame& src) const
    {
        dst[0] += src[1] * m_coeff;
        dst[1] += src[0] * m_coeff;
    }

    const real_t m_coeff;
};

void addSwappedMultiplied(sampleFrame*       dst,
                          const sampleFrame* src,
                          real_t             coeffSrc,
                          int                frames)
{
    run<>(dst, src, frames, AddSwappedMultipliedOp(coeffSrc));
}

void addMultipliedByBuffer(sampleFrame*       dst,
                           const sampleFrame* src,
                           real_t             coeffSrc,
                           const ValueBuffer* coeffSrcBuf,
                           int                frames)
{
    // const real_t* const values = coeffSrcBuf->values();
    for(int f = 0; f < frames; ++f)
    {
        const real_t c = coeffSrc * coeffSrcBuf->value(f);  // s[f];
        dst[f][0] += src[f][0] * c;
        dst[f][1] += src[f][1] * c;
    }
}

void addMultipliedByBuffers(sampleFrame*       dst,
                            const sampleFrame* src,
                            const ValueBuffer* coeffSrcBuf1,
                            const ValueBuffer* coeffSrcBuf2,
                            int                frames)
{
    // const real_t* const values = _coeffSrcBuf->values();
    for(int f = 0; f < frames; ++f)
    {
        const real_t c = coeffSrcBuf1->value(f)     // s()[f]
                         * coeffSrcBuf2->value(f);  // s()[f];
        dst[f][0] += src[f][0] * c;
        dst[f][1] += src[f][1] * c;
    }
}

void addSanitized(sampleFrame*       _dst,
                  const sampleFrame* _src,
                  const f_cnt_t      _frames)
{
    for(f_cnt_t f = 0; f < _frames; ++f)
    {
        _dst[f][0]
                += (isinf(_src[f][0]) || isnan(_src[f][0])) ? 0. : _src[f][0];
        _dst[f][1]
                += (isinf(_src[f][1]) || isnan(_src[f][1])) ? 0. : _src[f][1];
    }
}

void addSanitizedMultipliedByBuffer(sampleFrame*       dst,
                                    const sampleFrame* src,
                                    real_t             coeffSrc,
                                    const ValueBuffer* coeffSrcBuf,
                                    int                frames)
{
    for(int f = 0; f < frames; ++f)
    {
        const real_t c = coeffSrc * coeffSrcBuf->value(f);  // s[f];
        dst[f][0] += (isinf(src[f][0]) || isnan(src[f][0])) ? 0.
                                                            : src[f][0] * c;
        dst[f][1] += (isinf(src[f][1]) || isnan(src[f][1])) ? 0.
                                                            : src[f][1] * c;
    }
}

void addSanitizedMultipliedByBuffers(sampleFrame*       dst,
                                     const sampleFrame* src,
                                     const ValueBuffer* coeffSrcBuf1,
                                     const ValueBuffer* coeffSrcBuf2,
                                     int                frames)
{
    for(int f = 0; f < frames; ++f)
    {
        const real_t c = coeffSrcBuf1->value(f)     // s()[f]
                         * coeffSrcBuf2->value(f);  // s()[f];
        dst[f][0] += (isinf(src[f][0]) || isnan(src[f][0])) ? 0.
                                                            : src[f][0] * c;
        dst[f][1] += (isinf(src[f][1]) || isnan(src[f][1])) ? 0.
                                                            : src[f][1] * c;
    }
}

struct AddSanitizedMultipliedOp
{
    AddSanitizedMultipliedOp(real_t coeff) : m_coeff(coeff)
    {
    }

    void operator()(sampleFrame& dst, const sampleFrame& src) const
    {
        dst[0] += (isinf(src[0]) || isnan(src[0])) ? 0. : src[0] * m_coeff;
        dst[1] += (isinf(src[1]) || isnan(src[1])) ? 0. : src[1] * m_coeff;
    }

    const real_t m_coeff;
};

void addSanitizedMultiplied(sampleFrame*       dst,
                            const sampleFrame* src,
                            real_t             coeffSrc,
                            int                frames)
{
    run<>(dst, src, frames, AddSanitizedMultipliedOp(coeffSrc));
}

struct AddMultipliedStereoOp
{
    AddMultipliedStereoOp(real_t coeffLeft, real_t coeffRight)
    {
        m_coeffs[0] = coeffLeft;
        m_coeffs[1] = coeffRight;
    }

    void operator()(sampleFrame& dst, const sampleFrame& src) const
    {
        dst[0] += src[0] * m_coeffs[0];
        dst[1] += src[1] * m_coeffs[1];
    }

    real_t m_coeffs[2];
};

void addMultipliedStereo(sampleFrame*       dst,
                         const sampleFrame* src,
                         real_t             coeffSrcLeft,
                         real_t             coeffSrcRight,
                         int                frames)
{

    run<>(dst, src, frames,
          AddMultipliedStereoOp(coeffSrcLeft, coeffSrcRight));
}

struct MultiplyAndAddMultipliedOp
{
    MultiplyAndAddMultipliedOp(real_t coeffDst, real_t coeffSrc)
    {
        m_coeffs[0] = coeffDst;
        m_coeffs[1] = coeffSrc;
    }

    void operator()(sampleFrame& dst, const sampleFrame& src) const
    {
        dst[0] = dst[0] * m_coeffs[0] + src[0] * m_coeffs[1];
        dst[1] = dst[1] * m_coeffs[0] + src[1] * m_coeffs[1];
    }

    real_t m_coeffs[2];
};

void multiplyAndAddMultiplied(sampleFrame*       dst,
                              const sampleFrame* src,
                              real_t             coeffDst,
                              real_t             coeffSrc,
                              int                frames)
{
    run<>(dst, src, frames, MultiplyAndAddMultipliedOp(coeffDst, coeffSrc));
}

void multiplyAndAddMultipliedJoined(sampleFrame*    dst,
                                    const sample_t* srcLeft,
                                    const sample_t* srcRight,
                                    real_t          coeffDst,
                                    real_t          coeffSrc,
                                    int             frames)
{
    run<>(dst, srcLeft, srcRight, frames,
          MultiplyAndAddMultipliedOp(coeffDst, coeffSrc));
}

}  // namespace MixHelpers
