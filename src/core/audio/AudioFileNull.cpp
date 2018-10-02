/*
 * AudioFileNull.cpp - audio-device writing sound data without encoding.
 *                    32-bit float at the given sample rate.
 *                    filename should be: *.f<channels>r<samplerate>
 *                    endianness should be machine-like (no change).
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

#include "AudioFileNull.h"
//#include "endian_handling.h"
#include "Mixer.h"

AudioFileDevice* AudioFileNull::getInst(const QString&        outputFilename,
                                        const OutputSettings& outputSettings,
                                        const ch_cnt_t        channels,
                                        Mixer*                mixer,
                                        bool&                 successful)
{
    if(!outputFilename.isEmpty())
        qFatal("AudioFileNull: output filename given: %s",
               qPrintable(outputFilename));

    AudioFileNull* r = new AudioFileNull(outputSettings, channels, successful,
                                         // outputFilename,
                                         mixer);
    /*
    r->initOutputFile();
    r->openOutputFile();
    successful = r->outputFileOpened() && r->startEncoding();
    */
    successful = true;

    return r;
}

AudioFileNull::AudioFileNull(OutputSettings const& outputSettings,
                             const ch_cnt_t        channels,
                             bool&                 successful,
                             // const QString&        file,
                             Mixer* mixer) :
      AudioFileDevice(outputSettings, channels, "", mixer)
{
    successful = true;
}

AudioFileNull::~AudioFileNull()
{
    // finishEncoding();
}

void AudioFileNull::initOutputFile()
{
}

void AudioFileNull::openOutputFile()
{
}

bool AudioFileNull::outputFileOpened() const
{
    return true;
}

void AudioFileNull::closeOutputFile()
{
}

int AudioFileNull::writeData(const void* data, int len)
{
    return len;
}

/*
bool AudioFileNull::startEncoding()
{
        return true;
}

void AudioFileNull::writeBuffer(const surroundSampleFrame* _ab,
                                const fpp_t                _frames )
{
}

void AudioFileNull::finishEncoding()
{
}
*/
