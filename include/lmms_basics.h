/*
 * lmms_basics.h - typedefs for common types that are used in the whole app
 *
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef LMMS_BASICS_H
#define LMMS_BASICS_H

#include "lmmsconfig.h"

#ifdef LMMS_HAVE_STDINT_H
#include <cstdint>
#endif

//#include <limits>

using namespace std;

#define REAL_IS_DOUBLE
#undef REAL_IS_FLOAT
typedef double real_t;

//#define REAL_IS_FLOAT
//#undef REAL_IS_DOUBLE
// typedef float real_t;

typedef float  FLOAT;
typedef double DOUBLE;

typedef int32_t tact_t;
typedef int32_t tick_t;

typedef real_t volume_t;     // uint8_t (0 .. 200)
typedef real_t pitch_t;      // int16_t
typedef real_t panning_t;    // int8_t  (-100 .. 100)
typedef real_t frequency_t;  //

typedef real_t control_t;  // (0 .. 1)

typedef real_t  sample_t;     // standard sample everywhere (-1 .. 1)
typedef float   sampleF32_t;  // 32-bit float sample
typedef int16_t sampleS16_t;  // 16-bit int sample

typedef int32_t sample_rate_t;  // sample rate
typedef int16_t fpp_t;          // frames per period (0 .. 16384)
typedef int32_t f_cnt_t;        // standard frame-count
typedef int8_t  ch_cnt_t;       // channel count (0 .. SURROUND_CHANNELS)
typedef int16_t bpm_t;          // tempo (MIN_BPM .. MAX_BPM)
typedef int16_t bitrate_t;      // bitrate in kbps
typedef int16_t fx_ch_t;        // FX-channel (0 .. MAX_EFFECT_CHANNEL)

typedef uint32_t jo_id_t;  // (unique) ID of a journalling object

// use for improved branch prediction
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define SILENCE (1.E-10)

const ch_cnt_t DEFAULT_CHANNELS = 2;

#ifdef LMMS_BUILD_WIN32
#define LADSPA_PATH_SEPARATOR ';'
#define LV2_PATH_SEPARATOR ';'
#else
#define LADSPA_PATH_SEPARATOR ':'
#define LV2_PATH_SEPARATOR ':'
#endif

#ifdef LMMS_BUILD_APPLE
#define UI_ALT_KEY "⌥"
#define UI_CTRL_KEY "⌘"
#define UI_SHIFT_KEY "shift"
#else
#define UI_ALT_KEY "Alt"
#define UI_CTRL_KEY "Ctrl"
#define UI_SHIFT_KEY "⇧"
#endif

typedef sample_t sampleFrame[DEFAULT_CHANNELS];

#define MM_ALIGN_SIZE 16
//#if __GNUC__
// typedef sample_t sampleFrameA[DEFAULT_CHANNELS]
// __attribute__((__aligned__(ALIGN_SIZE))); #endif

#define LMMS_DISABLE_SURROUND
#ifndef LMMS_DISABLE_SURROUND
const ch_cnt_t   SURROUND_CHANNELS = 4;
typedef sample_t surroundSampleFrame[SURROUND_CHANNELS];
#else
const ch_cnt_t      SURROUND_CHANNELS = DEFAULT_CHANNELS;
typedef sampleFrame surroundSampleFrame;
#endif

#define STRINGIFY(s) STR(s)
#define STR(PN) #PN

#define DELETE_HELPER(x)     \
    {                        \
        qInfo("delete " #x); \
        auto tmp = x;        \
        x        = nullptr;  \
        if(tmp != nullptr)   \
            delete x;        \
    }

#endif
