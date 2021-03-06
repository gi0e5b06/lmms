/*
 * AudioDevice.cpp - base-class for audio-devices used by LMMS-mixer
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "AudioDevice.h"

#include "ConfigManager.h"
#include "Engine.h"
#include "MixHelpers.h"
#include "Mixer.h"
#include "SampleRate.h"
#include "Song.h"
#include "debug.h"  // REQUIRED

#include <cstring>

AudioDevice::AudioDevice(const ch_cnt_t _channels, Mixer* _mixer) :
      m_supportsCapture(false), m_sampleRate(_mixer->processingSampleRate()),
      m_channels(_channels), m_mixer(_mixer),
      m_buffer(new surroundSampleFrame[mixer()->framesPerPeriod()])
{
    qWarning("AudioDevice::AudioDevice");

    int error;
    m_srcState
            = src_new(mixer()->currentQualitySettings().libsrcInterpolation(),
                      SURROUND_CHANNELS, &error);
    if(m_srcState == nullptr)
        qCritical("AudioDevice: src_new() failed error=%d", error);
}

AudioDevice::~AudioDevice()
{
    src_delete(m_srcState);
    delete[] m_buffer;

    m_devMutex.tryLock();
    unlock();
}

void AudioDevice::processNextBuffer()
{
    const fpp_t frames = getNextBuffer(m_buffer);

    if(frames)
    {
        //, mixer()->masterGain());
        writeBuffer(m_buffer, frames);
    }
    else
    {
        m_inProcess = false;
    }
}

fpp_t AudioDevice::getNextBuffer(surroundSampleFrame* _ab)
{
    fpp_t                      frames = mixer()->framesPerPeriod();
    const surroundSampleFrame* b      = mixer()->nextBuffer();

    if(b == nullptr)
    {
        qWarning("AudioDevice::getNextBuffer no data from mixer?");
        return 0;
    }

    // make sure, no other thread is accessing device
    lock();

    // resample if necessary
    if(mixer()->processingSampleRate() != m_sampleRate)
    {
        resample(b, frames, _ab, mixer()->processingSampleRate(),
                 m_sampleRate);
        /*
        frames = (frames * double(m_sampleRate))
                 / double(mixer()->processingSampleRate());
        */
    }
    else
    {
        memcpy(_ab, b, frames * sizeof(surroundSampleFrame));
    }

    // release lock
    unlock();

    if(mixer()->hasFifoWriter())
    {
        delete[] b;
    }

    if(!Engine::getSong()->isExporting())
        applyMasterAdjustments(_ab, frames);

    return frames;
}

void AudioDevice::stopProcessing()
{
    if(mixer()->hasFifoWriter())
    {
        while(m_inProcess)
        {
            processNextBuffer();
        }
    }
}

void AudioDevice::stopProcessingThread(QThread* thread)
{
    if(!thread->wait(30000))
    {
        fprintf(stderr, "Terminating audio device thread\n");
        thread->terminate();
        if(!thread->wait(1000))
        {
            fprintf(stderr, "Thread not terminated yet\n");
        }
    }
}

void AudioDevice::applyQualitySettings()
{
    src_delete(m_srcState);

    int error;
    if((m_srcState
        = src_new(mixer()->currentQualitySettings().libsrcInterpolation(),
                  SURROUND_CHANNELS, &error))
       == NULL)
    {
        qCritical("Error: src_new() failed in audio_device.cpp!");
    }
}

void AudioDevice::applyMasterAdjustments(surroundSampleFrame* _ab,
                                         fpp_t                _frames)
{
    SurroundGain ggg
            = toSurroundGain(PanningRight * mixer()->masterPanningGain(),
                             MaxVolume * mixer()->masterVolumeGain());

    for(fpp_t f = _frames - 1; f >= 0; --f)
        for(ch_cnt_t ch = SURROUND_CHANNELS - 1; ch >= 0; --ch)
            _ab[f][ch] *= ggg.gain[ch];
}

void AudioDevice::registerPort(AudioPort*)
{
}

void AudioDevice::unregisterPort(AudioPort* _port)
{
}

void AudioDevice::renamePort(AudioPort*)
{
}

void AudioDevice::resample(const surroundSampleFrame* _srcBuf,
                           const fpp_t                _frames,
                           surroundSampleFrame*       _dstBuf,
                           const sample_rate_t        _srcSR,
                           const sample_rate_t        _dstSR)
{
    const double frqRatio = double(_dstSR) / double(_srcSR);

    SampleRate::resample(_srcBuf, _dstBuf, _frames, _frames, frqRatio);
    /*
#ifdef REAL_IS_FLOAT
{
    if(m_srcState == NULL)
        return;
    m_srcData.input_frames  = _frames;
    m_srcData.output_frames = _frames;
    m_srcData.data_in       = _src[0];
    m_srcData.data_out      = _dst[0];
    m_srcData.src_ratio     = double(_dst_sr) / double(_src_sr);
    m_srcData.end_of_input  = 0;
    int error;
    if((error = src_process(m_srcState, &m_srcData)))
    {
        qFatal("AudioDevice::resample(): error while resampling: %s",
               src_strerror(error));
    }
}
#endif
#ifdef REAL_IS_DOUBLE
{
    // resample64
    qFatal("AudioDevice::resample");
}
#endif
    */
}

void AudioDevice::swapEndian(void*       _buf,
                             const fpp_t _frames,
                             const int   _frameSize)
{
    unsigned char* b = static_cast<unsigned char*>(_buf);
    unsigned char  tmp[_frameSize];
    for(fpp_t f = 0; f < _frames; ++f)
    {
        for(int i = 0; i < _frameSize; i++)
            tmp[_frameSize - 1 - i] = b[i];
        memcpy(b, tmp, _frameSize);
        b += _frameSize;
    }
}

void AudioDevice::clearBuffer(void*       _outbuf,
                              const fpp_t _frames,
                              const int   _frameSize)
{
    assert(_outbuf != NULL);
    int size = _frames * channels() * _frameSize;
    memset(_outbuf, 0, size);
}

int AudioDevice::convertToS16(const surroundSampleFrame* _ab,
                              const fpp_t                _frames,
                              sampleS16_t*               _output_buffer,
                              const bool                 _convert_endian)
{
    int size = _frames * channels() * sizeof(sampleS16_t);

    for(fpp_t frame = 0; frame < _frames; ++frame)
    {
        for(ch_cnt_t chnl = 0; chnl < channels(); ++chnl)
        {
            (_output_buffer + frame * channels())[chnl]
                    = MixHelpers::convertToS16(_ab[frame][chnl]);
        }
    }

    if(_convert_endian)
        swapEndian(_output_buffer, size, sizeof(sampleS16_t));

    return size;
}

void AudioDevice::clearS16Buffer(sampleS16_t* _outbuf, const fpp_t _frames)
{
    clearBuffer(_outbuf, _frames, sizeof(sampleS16_t));
}

int AudioDevice::convertToF32(const surroundSampleFrame* _ab,
                              const fpp_t                _frames,
                              FLOAT*                     _output_buffer,
                              const bool                 _convert_endian)
{
    int size = _frames * channels() * sizeof(FLOAT);

#ifdef REAL_IS_FLOAT
    memcpy(_output_buffer, _ab, size);
#endif
#ifdef REAL_IS_DOUBLE
    for(fpp_t frame = 0; frame < _frames; ++frame)
    {
        for(ch_cnt_t chnl = 0; chnl < channels(); ++chnl)
        {
            (_output_buffer + frame * channels())[chnl]
                    = Mixer::clip(_ab[frame][chnl]);
        }
    }
#endif

    if(_convert_endian)
        swapEndian(_output_buffer, size, sizeof(FLOAT));

    return size;
}

void AudioDevice::clearF32Buffer(FLOAT* _outbuf, const fpp_t _frames)
{
    clearBuffer(_outbuf, _frames, sizeof(FLOAT));
}

int AudioDevice::convertToF64(const surroundSampleFrame* _ab,
                              const fpp_t                _frames,
                              double*                    _output_buffer,
                              const bool                 _convert_endian)
{
    int size = _frames * channels() * sizeof(double);

#ifdef REAL_IS_DOUBLE
    memcpy(_output_buffer, _ab, size);
#endif
#ifdef REAL_IS_FLOAT
    for(fpp_t frame = 0; frame < _frames; ++frame)
    {
        for(ch_cnt_t chnl = 0; chnl < channels(); ++chnl)
        {
            (_output_buffer + frame * channels())[chnl]
                    = Mixer::clip(_ab[frame][chnl]);
        }
    }
#endif

    if(_convert_endian)
        swapEndian(_output_buffer, size, sizeof(double));

    return size;
}

void AudioDevice::clearF64Buffer(double* _outbuf, const fpp_t _frames)
{
    clearBuffer(_outbuf, _frames, sizeof(double));
}

int AudioDevice::convertToS32(const surroundSampleFrame* _ab,
                              const fpp_t                _frames,
                              int32_t*                   _output_buffer,
                              const bool                 _convert_endian)
{
    int size = _frames * channels() * sizeof(int32_t);

    for(fpp_t frame = 0; frame < _frames; ++frame)
    {
        for(ch_cnt_t chnl = 0; chnl < channels(); ++chnl)
        {
            (_output_buffer + frame * channels())[chnl]
                    = MixHelpers::convertToS32(_ab[frame][chnl]);
        }
    }

    if(_convert_endian)
        swapEndian(_output_buffer, size, sizeof(int32_t));

    return _frames * channels() * sizeof(int32_t);
}

void AudioDevice::clearS32Buffer(int32_t* _outbuf, const fpp_t _frames)
{
    clearBuffer(_outbuf, _frames, sizeof(int32_t));
}

bool AudioDevice::hqAudio() const
{
    return ConfigManager::inst()->value("mixer", "hqaudio").toInt();
}
