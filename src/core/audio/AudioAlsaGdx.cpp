/*
 * AudioAlsaGdx.cpp - device-class which implements ALSA-PCM-output
 *
 * Copyright (c) 2018      gi0e5b06        <on github.com>
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

//#include <QComboBox>
//#include <QLineEdit>

#include "AudioAlsaGdx.h"

#ifdef LMMS_HAVE_ALSA

#include "ConfigManager.h"
#include "Engine.h"
#include "Mixer.h"
#include "endian_handling.h"
//#include "gui_templates.h"
#include "templates.h"

AudioAlsaGdx::AudioAlsaGdx(bool& _success_ful, Mixer* _mixer) :
      AudioDevice(tLimit<ch_cnt_t>(ConfigManager::inst()
                                           ->value("audioalsagdx", "channels")
                                           .toInt(),
                                   DEFAULT_CHANNELS,
                                   SURROUND_CHANNELS),
                  _mixer),
      m_outHandle(NULL), m_hwParams(NULL), m_swParams(NULL),
      m_convertEndian(false)
{
    qWarning("AudioAlsaGdx::AudioAlsaGdx");

    setObjectName("alsaGdxAudio");

    _success_ful = false;

    if(setenv("PULSE_ALSA_HOOK_CONF", "/dev/null", 0))
    {
        qCritical(
                "AudioAlsaGdx: could not avoid possible "
                "interception by PulseAudio");
    }

    int err;

    if((err = snd_pcm_open(&m_outHandle, probeDevice().toLatin1().constData(),
                           SND_PCM_STREAM_PLAYBACK, 0))
       < 0)
    {
        qCritical("AudioAlsaGdx: playback open error: %s", snd_strerror(err));
        return;
    }

    snd_pcm_hw_params_malloc(&m_hwParams);
    snd_pcm_sw_params_malloc(&m_swParams);

    if((err = setHWParams(channels(), SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        qCritical("AudioAlsaGdx: setting of hwparams failed: %s",
                  snd_strerror(err));
        return;
    }

    if((err = setSWParams()) < 0)
    {
        qCritical("AudioAlsaGdx: setting of swparams failed: %s",
                  snd_strerror(err));
        return;
    }

    // set FD_CLOEXEC flag for all file descriptors so forked processes
    // do not inherit them
    struct pollfd* ufds;
    int            count = snd_pcm_poll_descriptors_count(m_outHandle);
    ufds                 = new pollfd[count];
    snd_pcm_poll_descriptors(m_outHandle, ufds, count);
    for(int i = 0; i < qMax(3, count); ++i)
    {
        const int fd       = (i >= count) ? ufds[0].fd + i : ufds[i].fd;
        int       oldflags = fcntl(fd, F_GETFD, 0);
        if(oldflags < 0)
            continue;
        oldflags |= FD_CLOEXEC;
        fcntl(fd, F_SETFD, oldflags);
    }
    delete[] ufds;
    _success_ful = true;
}

AudioAlsaGdx::~AudioAlsaGdx()
{
    stopProcessing();
    if(m_outHandle != NULL)
    {
        snd_pcm_close(m_outHandle);
    }

    if(m_hwParams != NULL)
    {
        snd_pcm_hw_params_free(m_hwParams);
    }

    if(m_swParams != NULL)
    {
        snd_pcm_sw_params_free(m_swParams);
    }
}

QString AudioAlsaGdx::probeDevice()
{
    QString dev = ConfigManager::inst()->value("audioalsagdx", "device");
    if(dev == "")
    {
        if(getenv("AUDIODEV") != NULL)
        {
            return getenv("AUDIODEV");
        }
        return "default";
    }
    return dev;
}

/**
 * @brief Creates a list of all available devices.
 *
 * Uses the hints API of ALSA to collect all devices. This also includes plug
 * devices. The reason to collect these and not the raw hardware devices
 * (e.g. hw:0,0) is that hardware devices often have a very limited number of
 * supported formats, etc. Plugs on the other hand are software components
 * that map all types of formats and inputs to the hardware and therefore they
 * are much more flexible and more what we want.
 *
 * Further helpful info http://jan.newmarch.name/LinuxSound/Sampled/Alsa/.
 *
 * @return A collection of devices found on the system.
 */
AudioAlsaGdx::DeviceInfoCollection AudioAlsaGdx::getAvailableDevices()
{
    DeviceInfoCollection deviceInfos;

    char** hints;

    /* Enumerate sound devices */
    int err = snd_device_name_hint(-1, "pcm", (void***)&hints);
    if(err != 0)
    {
        return deviceInfos;
    }

    char** n = hints;
    while(*n != NULL)
    {
        char* name        = snd_device_name_get_hint(*n, "NAME");
        char* description = snd_device_name_get_hint(*n, "DESC");

        if(name != 0 && description != 0)
        {
            deviceInfos.push_back(
                    DeviceInfo(QString(name), QString(description)));
        }

        free(name);
        free(description);

        n++;
    }

    // Free the hint buffer
    snd_device_name_free_hint((void**)hints);

    return deviceInfos;
}

int AudioAlsaGdx::handleError(int _err)
{
    if(_err == -EPIPE)
    {
        // under-run
        _err = snd_pcm_prepare(m_outHandle);
        if(_err < 0)
            qCritical(
                    "AudioAlsaGdx: can not recover from underrun,"
                    " prepare failed: %s",
                    snd_strerror(_err));
        return 0;
    }
#ifdef ESTRPIPE
    else if(_err == -ESTRPIPE)
    {
        while((_err = snd_pcm_resume(m_outHandle)) == -EAGAIN)
        {
            sleep(1);  // wait until the suspend flag
                       // is released
        }

        if(_err < 0)
        {
            _err = snd_pcm_prepare(m_outHandle);
            if(_err < 0)
                qCritical(
                        "AudioAlsaGdx: can not recover from"
                        "suspend, prepare failed: %s",
                        snd_strerror(_err));
        }
        return 0;
    }
#endif
    return _err;
}

void AudioAlsaGdx::startProcessing()
{
    if(!isRunning())
    {
        start(QThread::TimeCriticalPriority);  // QThread::HighPriority );
    }
}

void AudioAlsaGdx::stopProcessing()
{
    stopProcessingThread(this);
}

void AudioAlsaGdx::applyQualitySettings()
{
    if(hqAudio())
    {
        setSampleRate(Engine::mixer()->processingSampleRate());

        if(m_outHandle != NULL)
        {
            snd_pcm_close(m_outHandle);
        }

        int err;
        if((err
            = snd_pcm_open(&m_outHandle, probeDevice().toLatin1().constData(),
                           SND_PCM_STREAM_PLAYBACK, 0))
           < 0)
        {
            qCritical("AudioAlsaGdx: open error: %s", snd_strerror(err));
            return;
        }

        if((err = setHWParams(channels(), SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
        {
            qCritical(
                    "AudioAlsaGdx: "
                    "setting of hwparams failed: %s",
                    snd_strerror(err));
            return;
        }
        if((err = setSWParams()) < 0)
        {
            qCritical(
                    "AudioAlsaGdx: "
                    "setting of swparams failed: %s",
                    snd_strerror(err));
            return;
        }
    }

    AudioDevice::applyQualitySettings();
}

void AudioAlsaGdx::run()
{
    if(m_pcmFormat == "S16_LE" || m_pcmFormat == "S16_BE")
        runS16();
    else if(m_pcmFormat == "FLOAT_LE" || m_pcmFormat == "FLOAT_BE")
        runF32();
    else if(m_pcmFormat == "S32_LE" || m_pcmFormat == "S32_BE")
        runS32();
}

void AudioAlsaGdx::runS16()
{
    surroundSampleFrame* temp
            = new surroundSampleFrame[mixer()->framesPerPeriod()];
    int pcmbuf_size = m_periodSize * channels();

    sampleS16_t* outbuf
            = new sampleS16_t[mixer()->framesPerPeriod() * channels()];
    sampleS16_t* pcmbuf = new sampleS16_t[m_periodSize * channels()];

    int outbuf_size = mixer()->framesPerPeriod() * channels();
    int outbuf_pos  = 0;

    bool quit = false;
    while(quit == false)
    {
        sampleS16_t* ptr = pcmbuf;
        int          len = pcmbuf_size;
        while(len)
        {
            if(outbuf_pos == 0)
            {
                // frames depend on the sample rate
                const fpp_t frames = getNextBuffer(temp);
                if(!frames)
                {
                    quit = true;
                    memset(ptr, 0, len * sizeof(sampleS16_t));
                    break;
                }
                outbuf_size = frames * channels();

                convertToS16(temp, frames, outbuf, m_convertEndian);
            }
            int min_len = qMin(len, outbuf_size - outbuf_pos);
            memcpy(ptr, outbuf + outbuf_pos, min_len * sizeof(sampleS16_t));
            ptr += min_len;
            len -= min_len;
            outbuf_pos += min_len;
            outbuf_pos %= outbuf_size;
        }

        f_cnt_t frames = m_periodSize;
        ptr            = pcmbuf;

        while(frames)
        {
            int err = snd_pcm_writei(m_outHandle, ptr, frames);

            if(err == -EAGAIN)
            {
                continue;
            }

            if(err < 0)
            {
                if(handleError(err) < 0)
                {
                    qWarning("AudioAlsaGdx: write error: %s",
                             snd_strerror(err));
                }
                break;  // skip this buffer
            }
            ptr += err * channels();
            frames -= err;
        }
    }

    delete[] outbuf;
    delete[] pcmbuf;
    delete[] temp;
}

void AudioAlsaGdx::runF32()
{

    surroundSampleFrame* temp
            = new surroundSampleFrame[mixer()->framesPerPeriod()];
    int pcmbuf_size = m_periodSize * channels();

    float* outbuf = new float[mixer()->framesPerPeriod() * channels()];
    float* pcmbuf = new float[m_periodSize * channels()];

    int outbuf_size = mixer()->framesPerPeriod() * channels();
    int outbuf_pos  = 0;

    bool quit = false;
    while(quit == false)
    {
        float* ptr = pcmbuf;
        int    len = pcmbuf_size;
        while(len)
        {
            if(outbuf_pos == 0)
            {
                // frames depend on the sample rate
                const fpp_t frames = getNextBuffer(temp);
                if(!frames)
                {
                    quit = true;
                    memset(ptr, 0, len * sizeof(float));
                    break;
                }
                outbuf_size = frames * channels();

                convertToF32(temp, frames, outbuf, m_convertEndian);
            }
            int min_len = qMin(len, outbuf_size - outbuf_pos);
            memcpy(ptr, outbuf + outbuf_pos, min_len * sizeof(float));
            ptr += min_len;
            len -= min_len;
            outbuf_pos += min_len;
            outbuf_pos %= outbuf_size;
        }

        f_cnt_t frames = m_periodSize;
        ptr            = pcmbuf;

        while(frames)
        {
            int err = snd_pcm_writei(m_outHandle, ptr, frames);

            if(err == -EAGAIN)
            {
                continue;
            }

            if(err < 0)
            {
                if(handleError(err) < 0)
                {
                    qWarning("AudioAlsaGdx: write error: %s",
                             snd_strerror(err));
                }
                break;  // skip this buffer
            }
            ptr += err * channels();
            frames -= err;
        }
    }

    delete[] outbuf;
    delete[] pcmbuf;
    delete[] temp;
}

void AudioAlsaGdx::runS32()
{
    surroundSampleFrame* temp
            = new surroundSampleFrame[mixer()->framesPerPeriod()];
    int pcmbuf_size = m_periodSize * channels();

    int32_t* outbuf = new int32_t[mixer()->framesPerPeriod() * channels()];
    int32_t* pcmbuf = new int32_t[m_periodSize * channels()];

    int outbuf_size = mixer()->framesPerPeriod() * channels();
    int outbuf_pos  = 0;

    bool quit = false;
    while(quit == false)
    {
        int32_t* ptr = pcmbuf;
        int      len = pcmbuf_size;
        while(len)
        {
            if(outbuf_pos == 0)
            {
                // frames depend on the sample rate
                const fpp_t frames = getNextBuffer(temp);
                if(!frames)
                {
                    quit = true;
                    memset(ptr, 0, len * sizeof(int32_t));
                    break;
                }
                outbuf_size = frames * channels();

                convertToS32(temp, frames, outbuf, m_convertEndian);
            }
            int min_len = qMin(len, outbuf_size - outbuf_pos);
            memcpy(ptr, outbuf + outbuf_pos, min_len * sizeof(int32_t));
            ptr += min_len;
            len -= min_len;
            outbuf_pos += min_len;
            outbuf_pos %= outbuf_size;
        }

        f_cnt_t frames = m_periodSize;
        ptr            = pcmbuf;

        while(frames)
        {
            int err = snd_pcm_writei(m_outHandle, ptr, frames);

            if(err == -EAGAIN)
            {
                continue;
            }

            if(err < 0)
            {
                if(handleError(err) < 0)
                {
                    qWarning("AudioAlsaGdx: write error: %s",
                             snd_strerror(err));
                }
                break;  // skip this buffer
            }
            ptr += err * channels();
            frames -= err;
        }
    }

    delete[] outbuf;
    delete[] pcmbuf;
    delete[] temp;
}

int AudioAlsaGdx::setHWParams(const ch_cnt_t   _channels,
                              snd_pcm_access_t _access)
{
    int err, dir;

    // choose all parameters
    if((err = snd_pcm_hw_params_any(m_outHandle, m_hwParams)) < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "broken configuration for playback: "
                "no configurations available: %s",
                snd_strerror(err));
        return err;
    }

    // set the interleaved read/write format
    if((err = snd_pcm_hw_params_set_access(m_outHandle, m_hwParams, _access))
       < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "access type not available for playback: %s",
                snd_strerror(err));
        return err;
    }

    // set the count of channels
    if((err
        = snd_pcm_hw_params_set_channels(m_outHandle, m_hwParams, _channels))
       < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "channel count (%i) not available for playbacks: %s"
                "\n(Does your soundcard not support surround?)",
                _channels, snd_strerror(err));
        return err;
    }

    // set the sample format
    /*
    for(int val = 0; val <= SND_PCM_FORMAT_LAST; val++)
    {
        if(0
           == snd_pcm_hw_params_test_format(m_outHandle, m_hwParams,
                                            (snd_pcm_format_t)val))
            if(snd_pcm_format_name((snd_pcm_format_t)val) != NULL)
                qWarning("***  %s (%s) available",
                         snd_pcm_format_name((snd_pcm_format_t)val),
                         snd_pcm_format_description((snd_pcm_format_t)val));
    }
    */

    if(snd_pcm_hw_params_test_format(m_outHandle, m_hwParams,
                                     SND_PCM_FORMAT_FLOAT_LE)
               == 0
       && (err = snd_pcm_hw_params_set_format(m_outHandle, m_hwParams,
                                              SND_PCM_FORMAT_FLOAT_LE))
                  == 0)
    {
        m_pcmFormat     = "FLOAT_LE";
        m_convertEndian = !isLittleEndian();
    }
    else if(snd_pcm_hw_params_test_format(m_outHandle, m_hwParams,
                                          SND_PCM_FORMAT_FLOAT_BE)
                    == 0
            && (err = snd_pcm_hw_params_set_format(m_outHandle, m_hwParams,
                                                   SND_PCM_FORMAT_FLOAT_BE))
                       == 0)
    {
        m_pcmFormat     = "FLOAT_BE";
        m_convertEndian = isLittleEndian();
    }
    else if(snd_pcm_hw_params_test_format(m_outHandle, m_hwParams,
                                          SND_PCM_FORMAT_S32_LE)
                    == 0
            && (err = snd_pcm_hw_params_set_format(m_outHandle, m_hwParams,
                                                   SND_PCM_FORMAT_S32_LE))
                       == 0)
    {
        m_pcmFormat     = "S32_LE";
        m_convertEndian = !isLittleEndian();
    }
    /*
    else if((err = snd_pcm_hw_params_set_format(m_outHandle, m_hwParams,
                                                SND_PCM_FORMAT_S32_BE))
            == 0)
    {
        m_pcmFormat     = "S32_BE";
        m_convertEndian = isLittleEndian();
    }
    */
    else if(snd_pcm_hw_params_test_format(m_outHandle, m_hwParams,
                                          SND_PCM_FORMAT_S16_LE)
                    == 0
            && (err = snd_pcm_hw_params_set_format(m_outHandle, m_hwParams,
                                                   SND_PCM_FORMAT_S16_LE))
                       == 0)
    {
        m_pcmFormat     = "S16_LE";
        m_convertEndian = !isLittleEndian();
    }
    else if(snd_pcm_hw_params_test_format(m_outHandle, m_hwParams,
                                          SND_PCM_FORMAT_S16_BE)
                    == 0
            && (err = snd_pcm_hw_params_set_format(m_outHandle, m_hwParams,
                                                   SND_PCM_FORMAT_S16_BE))
                       == 0)
    {
        m_pcmFormat     = "S16_BE";
        m_convertEndian = isLittleEndian();
    }
    else
    {
        qCritical(
                "AudioAlsaGdx: "
                "pcm format not found for playback: %s",
                snd_strerror(err));
        return err;
    }

    qWarning("AudioAlsaGdx: card set to %s", qPrintable(m_pcmFormat));

    // set the sample rate
    if((err = snd_pcm_hw_params_set_rate(m_outHandle, m_hwParams,
                                         sampleRate(), 0))
       == 0)
    {
        qWarning("AudioAlsaGdx: card set to %d Hz", sampleRate());
    }
    else if((err = snd_pcm_hw_params_set_rate(m_outHandle, m_hwParams,
                                              mixer()->baseSampleRate(), 0))
            == 0)
    {
        qWarning("AudioAlsaGdx: card set to %d Hz",
                 mixer()->baseSampleRate());
    }
    else
    {
        qCritical(
                "AudioAlsaGdx: "
                "could not set sample rate: %s",
                snd_strerror(err));
        return err;
    }

    m_periodSize = mixer()->framesPerPeriod();

    dir = 0;
    err = snd_pcm_hw_params_set_period_size_near(m_outHandle, m_hwParams,
                                                 &m_periodSize, &dir);
    if(err < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to set period size %lu for playback: %s",
                m_periodSize, snd_strerror(err));
        return err;
    }
    dir = 0;
    err = snd_pcm_hw_params_get_period_size(m_hwParams, &m_periodSize, &dir);
    if(err < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to get period size for playback: %s",
                snd_strerror(err));
    }

    m_bufferSize = m_periodSize * 2;

    // dir = 0;
    err = snd_pcm_hw_params_set_buffer_size_near(m_outHandle, m_hwParams,
                                                 &m_bufferSize);
    if(err < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to set buffer size %lu for playback: %s",
                m_bufferSize, snd_strerror(err));
        return err;
    }

    err = snd_pcm_hw_params_get_buffer_size(m_hwParams, &m_bufferSize);
    if(err < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to get buffer size for playback: %s",
                snd_strerror(err));
        return err;
    }

    if(2 * m_periodSize > m_bufferSize)
    {
        qCritical(
                "AudioAlsaGdx: "
                "buffer to small, could not use");
        return err;
    }

    // write the parameters to device
    err = snd_pcm_hw_params(m_outHandle, m_hwParams);
    if(err < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to set hw params for playback: %s",
                snd_strerror(err));
        return err;
    }

    return 0;  // all ok
}

int AudioAlsaGdx::setSWParams()
{
    int err;

    // get the current swparams
    if((err = snd_pcm_sw_params_current(m_outHandle, m_swParams)) < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to determine current swparams "
                "for playback: %s",
                snd_strerror(err));
        return err;
    }

    // start the transfer when a period is full
    if((err = snd_pcm_sw_params_set_start_threshold(m_outHandle, m_swParams,
                                                    m_periodSize))
       < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to set start threshold mode for playback: %s",
                snd_strerror(err));
        return err;
    }

    // allow the transfer when at least m_periodSize samples can be
    // processed
    if((err = snd_pcm_sw_params_set_avail_min(m_outHandle, m_swParams,
                                              m_periodSize))
       < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to set avail min for playback: %s",
                snd_strerror(err));
        return err;
    }

    // align all transfers to 1 sample

#if SND_LIB_VERSION < ((1 << 16) | (0) | 16)
    if((err = snd_pcm_sw_params_set_xfer_align(m_outHandle, m_swParams, 1))
       < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to set transfer align for playback: %s",
                snd_strerror(err));
        return err;
    }
#endif

    // write the parameters to the playback device
    if((err = snd_pcm_sw_params(m_outHandle, m_swParams)) < 0)
    {
        qCritical(
                "AudioAlsaGdx: "
                "unable to set sw params for playback: %s",
                snd_strerror(err));
        return err;
    }

    return 0;  // all ok
}

#endif
