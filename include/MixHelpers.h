/*
 * MixHelpers.h - helper functions for mixing buffers
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

#ifndef MIX_HELPERS_H
#define MIX_HELPERS_H

#include "lmms_basics.h"

class ValueBuffer;
namespace MixHelpers
{

const int16_t I_S16_MULTIPLIER = 32767;
const int32_t I_S32_MULTIPLIER = 2147483647;
const int64_t I_S64_MULTIPLIER = 9223372036854775807;

const FLOAT F_S16_MULTIPLIER = 32767.f;
const FLOAT F_S32_MULTIPLIER = 2147483647.f;
const FLOAT F_S64_MULTIPLIER = 9223372036854775807.f;

const double D_S16_MULTIPLIER = 32767.;
const double D_S32_MULTIPLIER = 2147483647.;
const double D_S64_MULTIPLIER = 9223372036854775807.;

int16_t convertToS16(FLOAT _v);
int32_t convertToS32(FLOAT _v);
int64_t convertToS64(FLOAT _v);
int16_t convertToS16(double _v);
int32_t convertToS32(double _v);
int64_t convertToS64(double _v);

sample_t convertFromS16(int16_t _v);
sample_t convertFromS32(int32_t _v);
sample_t convertFromS64(int64_t _v);

bool isSilent(const sampleFrame* _src, const f_cnt_t _frames);
bool isClipping(const sampleFrame* _src, const f_cnt_t _frames);

bool sanitize(sampleFrame* _src, const f_cnt_t _frames);
bool unclip(sampleFrame* _src, const f_cnt_t _frames);

void addMultiplied(sampleFrame*       _dst,
                   const sampleFrame* _src,
                   const ValueBuffer* _coeffSrcBuf,
                   const f_cnt_t      _frames);

/*! \brief Add samples from src to dst */
void add(sampleFrame* dst, const sampleFrame* src, int frames);

/*! \brief Add sanitized samples from src to dst */
void addSanitized(sampleFrame*       _dst,
                  const sampleFrame* _src,
                  f_cnt_t            _frames);

/*! \brief Add samples from src multiplied by coeffSrc to dst */
void addMultiplied(sampleFrame*       dst,
                   const sampleFrame* src,
                   real_t             coeffSrc,
                   int                frames);

/*! \brief Add samples from src multiplied by coeffSrc to dst, swap inputs */
void addSwappedMultiplied(sampleFrame*       dst,
                          const sampleFrame* src,
                          real_t             coeffSrc,
                          int                frames);

/*! \brief Add samples from src multiplied by coeffSrc and coeffSrcBuf to dst
 */
void addMultipliedByBuffer(sampleFrame*       dst,
                           const sampleFrame* src,
                           real_t             coeffSrc,
                           ValueBuffer*       coeffSrcBuf,
                           int                frames);

/*! \brief Add samples from src multiplied by coeffSrc and coeffSrcBuf to dst
 */
void addMultipliedByBuffers(sampleFrame*       dst,
                            const sampleFrame* src,
                            ValueBuffer*       coeffSrcBuf1,
                            ValueBuffer*       coeffSrcBuf2,
                            int                frames);

/*! \brief Same as addMultiplied, but sanitize output (strip out infs/nans) */
void addSanitizedMultiplied(sampleFrame*       dst,
                            const sampleFrame* src,
                            real_t             coeffSrc,
                            int                frames);

/*! \brief Add samples from src multiplied by coeffSrc and coeffSrcBuf to dst
 * - sanitized version */
void addSanitizedMultipliedByBuffer(sampleFrame*       dst,
                                    const sampleFrame* src,
                                    real_t             coeffSrc,
                                    ValueBuffer*       coeffSrcBuf,
                                    int                frames);

/*! \brief Add samples from src multiplied by coeffSrc and coeffSrcBuf to dst
 * - sanitized version */
void addSanitizedMultipliedByBuffers(sampleFrame*       dst,
                                     const sampleFrame* src,
                                     ValueBuffer*       coeffSrcBuf1,
                                     ValueBuffer*       coeffSrcBuf2,
                                     int                frames);

/*! \brief Add samples from src multiplied by coeffSrcLeft/coeffSrcRight to
 * dst */
void addMultipliedStereo(sampleFrame*       dst,
                         const sampleFrame* src,
                         real_t             coeffSrcLeft,
                         real_t             coeffSrcRight,
                         int                frames);

/*! \brief Multiply dst by coeffDst and add samples from src multiplied by
 * coeffSrc */
void multiplyAndAddMultiplied(sampleFrame*       dst,
                              const sampleFrame* src,
                              real_t             coeffDst,
                              real_t             coeffSrc,
                              int                frames);

/*! \brief Multiply dst by coeffDst and add samples from srcLeft/srcRight
 * multiplied by coeffSrc */
void multiplyAndAddMultipliedJoined(sampleFrame*    dst,
                                    const sample_t* srcLeft,
                                    const sample_t* srcRight,
                                    real_t          coeffDst,
                                    real_t          coeffSrc,
                                    int             frames);

}  // namespace MixHelpers

#endif
