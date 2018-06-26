/*
 * AudioFileRaw.h - audio-device writing sound data without encoding.
 *                  32-bit float at the given sample rate.
 *                  filename should be: *.f<channels>r<samplerate>
 *                  endianness should be machine-like (no change).
 *
 * Copyright (c) 2018 gi0e5b06
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

#ifndef AUDIO_FILE_RAW_H
#define AUDIO_FILE_RAW_H

#include <stdio.h>

#include "AudioFileDevice.h"
#include "lmmsconfig.h"

class AudioFileRaw : public AudioFileDevice
{
  public:
    virtual ~AudioFileRaw();

    static AudioFileDevice* getInst(const QString&        _outputFileName,
                                    const OutputSettings& _outputSettings,
                                    const ch_cnt_t        _channels,
                                    Mixer*                _mixer,
                                    bool&                 _successful);

  protected:
    AudioFileRaw(const OutputSettings& _outputSettings,
                 const ch_cnt_t        _channels,
                 bool&                 _successful,
                 const QString&        _file,
                 Mixer*                _mixer);

    virtual void writeBuffer(const surroundSampleFrame* _ab,
                             const fpp_t                _frames,
                             float                      _masterGain);
    /*override*/

  private:
    bool startEncoding();
    void finishEncoding();

    FILE* m_fh;
};

#endif
