/*
 * SampleRate.cpp -
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

#include "SampleRate.h"

#include "MemoryManager.h"

//#include <samplerate.h>

/*
  SRC_SINC_BEST_QUALITY   = 0,
  SRC_SINC_MEDIUM_QUALITY = 1,
  SRC_SINC_FASTEST        = 2,
  SRC_ZERO_ORDER_HOLD     = 3,
  SRC_LINEAR              = 4,
*/

bool SampleRate::resample(const sampleFrame* srcBuf,
                          sampleFrame*       dstBuf,
                          const f_cnt_t      srcSize,
                          const f_cnt_t      dstSize,
                          const double       frqRatio)
{
    f_cnt_t ifu, ofg;  // not used
    return resample(srcBuf, dstBuf, srcSize, dstSize, frqRatio, 10, ifu, ofg,
                    nullptr);
}

bool SampleRate::resample(const sampleFrame* _srcBuf,
                          sampleFrame*       _dstBuf,
                          const f_cnt_t      srcSize,
                          const f_cnt_t      dstSize,
                          const double       frqRatio,
                          const uint8_t      quality,  // 0-10
                          f_cnt_t&           inputFramesUsed,
                          f_cnt_t&           outputFramesGenerated,
                          SRC_STATE*         srcState)
{
#ifdef REAL_IS_DOUBLE
    FLOAT* srcBuf = MM_ALLOC(FLOAT, (srcSize * DEFAULT_CHANNELS));
    FLOAT* dstBuf = MM_ALLOC(FLOAT, (dstSize * DEFAULT_CHANNELS));
    for(f_cnt_t f = 0; f < srcSize; f++)
        for(ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ch++)
            srcBuf[DEFAULT_CHANNELS * f + ch] = _srcBuf[f][ch];
#endif
#ifdef REAL_IS_FLOAT
    const FLOAT* srcBuf = _srcBuf[0];
    FLOAT* dstBuf = _dstBuf[0];
#endif

    {
        int  error    = 0;
        bool statenew = false;

        if(srcState == nullptr)
        {
            int converter = 0;
            switch(quality)
            {
                case 0:
                case 1:
                    converter = SRC_ZERO_ORDER_HOLD;
                    break;
                case 2:
                case 3:
                    converter = SRC_LINEAR;
                    break;
                case 4:
                case 5:
                    converter = SRC_SINC_FASTEST;
                    break;
                case 6:
                case 7:
                    converter = SRC_SINC_MEDIUM_QUALITY;
                    break;
                default:
                    converter = SRC_SINC_BEST_QUALITY;
                    break;
            }

            srcState = src_new(converter, DEFAULT_CHANNELS, &error);
            statenew = true;
        }

        if(error != 0)
        {
            qCritical("SampleRate: error while initializing: %s",
                      src_strerror(error));
            return false;
        }

        if(srcState == nullptr)
            return false;

        SRC_DATA src_data;
        src_data.data_in       = &srcBuf[0];
        src_data.data_out      = &dstBuf[0];
        src_data.input_frames  = srcSize;
        src_data.output_frames = dstSize;
        src_data.src_ratio     = frqRatio;
        src_data.end_of_input  = 0;

        // We don't need to lock this assuming that we're only outputting the
        // samples in one thread
        error = src_process(srcState, &src_data);

        inputFramesUsed       = src_data.input_frames_used;
        outputFramesGenerated = src_data.output_frames_gen;

        if(statenew)
            srcState = src_delete(srcState);

        if(error != 0)
        {
            qCritical("SampleRate: error while resampling: %s",
                      src_strerror(error));
        }

        if(srcSize != 0 && src_data.output_frames_gen == 0)
        {
            qCritical("SampleRate: could not resample, no frames generated");
            qCritical("srcSize=%d inputFrames=%d frqRatio=%f",srcSize,inputFramesUsed,frqRatio);
            return false;
        }

        if(src_data.output_frames_gen > 0
           && src_data.output_frames_gen > dstSize)
        {
            qCritical(
                    "SampleRate: too many frames, wanted: %d, generated: %ld",
                    dstSize, src_data.output_frames_gen);
            return false;
        }
        /*
        if(src_data.output_frames_gen > 0
           && src_data.output_frames_gen < dstSize)
        {
            qCritical(
                    "SampleRate: not enough frames, wanted: %d, generated: "
                    "%ld",
                    dstSize, src_data.output_frames_gen);
            return false;
        }
        */
    }

    /*
    {
        real_t xstep = 1. / frqRatio;
        //qInfo("%d %d %f %f",srcSize,dstSize,frqRatio,xstep);
        real_t x;
        if(xstep < 1.)
        {
            x = real_t(srcSize - 1) * xstep;
            for(f_cnt_t f = dstSize - 1; f >= 0.; f--)
            {
                for(ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ch++)
                    dstBuf[f][ch] = srcBuf[f_cnt_t(x)][ch];
                x -= xstep;
            }
        }
        else
        {
            x = 0.;
            for(f_cnt_t f = 0; f < dstSize; f++)
            {
                for(ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ch++)
                    dstBuf[f][ch] = srcBuf[f_cnt_t(x)][ch];
                x += xstep;
            }
        }
        inputFramesUsed       = srcSize;
        outputFramesGenerated = dstSize;
    }
    */

#ifdef REAL_IS_DOUBLE
    for(f_cnt_t f = 0; f < dstSize; f++)
        for(ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ch++)
            _dstBuf[f][ch] = dstBuf[DEFAULT_CHANNELS * f + ch];
    MM_FREE(srcBuf);
    MM_FREE(dstBuf);
#endif

    return true;
}
