/*
 * AudioFileRaw.cpp - audio-device writing sound data without encoding.
 *                    32-bit float at the given sample rate.
 *                    filename should be: *.f<channels>r<samplerate>
 *                    endianness should be machine-like (no change).
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

#include "AudioFileRaw.h"
//#include "endian_handling.h"
#include "Mixer.h"

AudioFileDevice* AudioFileRaw::getInst(const QString&        outputFilename,
                                       const OutputSettings& outputSettings,
                                       const ch_cnt_t        channels,
                                       Mixer*                mixer,
                                       bool&                 successful)
{
    AudioFileRaw* r = new AudioFileRaw(outputSettings, channels, successful,
                                       outputFilename, mixer);
    r->initOutputFile();
    r->openOutputFile();

    successful = r->outputFileOpened() && r->startEncoding();

    return r;
}

AudioFileRaw::AudioFileRaw(OutputSettings const& outputSettings,
                           const ch_cnt_t        channels,
                           bool&                 successful,
                           const QString&        file,
                           Mixer*                mixer) :
      AudioFileDevice(outputSettings, channels, file, mixer)
{
}

AudioFileRaw::~AudioFileRaw()
{
    finishEncoding();
}

bool AudioFileRaw::startEncoding()
{
    Q_ASSERT(getOutputSettings().getBitDepth() == OutputSettings::Depth_F32);

    m_fh = fopen(
#ifdef LMMS_BUILD_WIN32
            outputFile().toLocal8Bit().constData(),
#else
            outputFile().toUtf8().constData(),
#endif
            "wb");
    return m_fh != NULL;
}

void AudioFileRaw::writeBuffer(const surroundSampleFrame* _ab,
                               const fpp_t                _frames)
{
    Q_ASSERT(getOutputSettings().getBitDepth() == OutputSettings::Depth_F32);

    fwrite(_ab, sizeof(surroundSampleFrame), _frames, m_fh);
}

void AudioFileRaw::finishEncoding()
{
    if(m_fh)
    {
        fclose(m_fh);
        m_fh = NULL;
    }
}
