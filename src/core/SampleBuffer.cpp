/*
 * SampleBuffer.cpp - container-class SampleBuffer
 *
 * Copyright (c) 2017-2018 gi0e5b06 (on github.com)
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "base64.h"
#include "lmmsconfig.h"
//#include "debug.h"
#include "Backtrace.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QMessageBox>
#include <QMutexLocker>
#include <QPainter>

#include <sndfile.h>

#define OV_EXCLUDE_STATIC_CALLBACKS
#ifdef LMMS_HAVE_OGGVORBIS
#include <vorbis/vorbisfile.h>
#endif

#ifdef LMMS_HAVE_FLAC_STREAM_ENCODER_H
#include <FLAC/stream_encoder.h>
#endif

#ifdef LMMS_HAVE_FLAC_STREAM_DECODER_H
#include <FLAC/stream_decoder.h>
#endif

#include "ConfigManager.h"
#include "DrumSynth.h"
#include "Engine.h"
#include "FileDialog.h"
#include "GuiApplication.h"
#include "MixHelpers.h"
#include "Mixer.h"
#include "SampleBuffer.h"
#include "SampleRate.h"
#include "endian_handling.h"  // REQUIRED

SampleBuffer::SampleBuffer(const SampleBuffer& _other) :
      m_audioFile(_other.m_audioFile), m_origData(_other.m_origData),
      m_origFrames(_other.m_origFrames), m_mmapped(_other.m_mmapped),
      m_data(nullptr), m_frames(0), m_startFrame(_other.m_startFrame),
      m_endFrame(_other.m_endFrame),
      m_loopStartFrame(_other.m_loopStartFrame),
      m_loopEndFrame(_other.m_loopEndFrame),
      m_amplification(_other.m_amplification), m_reversed(_other.m_reversed),
      m_frequency(BaseFreq), m_sampleRate(Engine::mixer()->baseSampleRate())
{
    if(!m_mmapped)
    {
        m_origData   = NULL;
        m_origFrames = 0;
    }

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(sampleRateChanged()));
    update(true);
}

SampleBuffer::SampleBuffer(const QString& _audioFile,
                           bool           _isBase64Data,
                           bool           _sampleRateDependent) :
      m_audioFile((_isBase64Data == true) ? "" : _audioFile),
      m_origData(NULL), m_origFrames(0), m_mmapped(false), m_data(NULL),
      m_frames(0), m_startFrame(0), m_endFrame(0), m_loopStartFrame(0),
      m_loopEndFrame(0), m_amplification(1.), m_reversed(false),
      m_frequency(BaseFreq), m_sampleRate(Engine::mixer()->baseSampleRate())
{
    if(_isBase64Data == true)
    {
        loadFromBase64(_audioFile);
    }
    if(_sampleRateDependent)
    {
        connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
                SLOT(sampleRateChanged()));
    }
    update();
}

SampleBuffer::SampleBuffer(const sampleFrame* _data,
                           const f_cnt_t      _frames,
                           bool               _sampleRateDependent) :
      m_audioFile(""),
      m_origData(NULL), m_origFrames(0), m_mmapped(false), m_data(NULL),
      m_frames(0), m_startFrame(0), m_endFrame(0), m_loopStartFrame(0),
      m_loopEndFrame(0), m_amplification(1.), m_reversed(false),
      m_frequency(BaseFreq), m_sampleRate(Engine::mixer()->baseSampleRate())
{
    if(_frames > 0)
    {
        m_origData = MM_ALLOC(sampleFrame, _frames);
        memcpy(m_origData, _data, _frames * BYTES_PER_FRAME);
        m_origFrames = _frames;
    }
    if(_sampleRateDependent)
    {
        connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
                SLOT(sampleRateChanged()));
    }
    update();
}

SampleBuffer::SampleBuffer(const f_cnt_t _frames, bool _sampleRateDependent) :
      m_audioFile(""), m_origData(NULL), m_origFrames(0), m_mmapped(false),
      m_data(NULL), m_frames(0), m_startFrame(0), m_endFrame(0),
      m_loopStartFrame(0), m_loopEndFrame(0), m_amplification(1.),
      m_reversed(false), m_frequency(BaseFreq),
      m_sampleRate(Engine::mixer()->baseSampleRate())
{
    if(_frames > 0)
    {
        m_origData = MM_ALLOC(sampleFrame, _frames);
        memset(m_origData, 0, _frames * BYTES_PER_FRAME);
        m_origFrames = _frames;
    }
    if(_sampleRateDependent)
    {
        connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
                SLOT(sampleRateChanged()));
    }
    update();
}

SampleBuffer::~SampleBuffer()
{
    // if(!m_mmapped) qInfo("~SampleBuffer: FREE origData %p",m_origData);
    if(!m_mmapped)
        MM_FREE(m_origData);
    // if(m_origData!=m_data) qInfo("~SampleBuffer: FREE data %p",m_data);
    if(m_origData != m_data)
        MM_FREE(m_data);
}

void SampleBuffer::sampleRateChanged()
{
    update(true);
}

static QHash<QString, sampleFrame*> s_mmap_pointer;
static QHash<QString, QFile*>       s_mmap_file;

void SampleBuffer::clearMMap()
{
    s_mmap_pointer.clear();
    foreach(QFile* f, s_mmap_file)
    {
        f->close();
        delete f;
    }
    s_mmap_file.clear();
}

void SampleBuffer::update(bool _keepSettings)
{
    // qInfo("SampleBuffer::update");
    const bool lock = (m_data != NULL);
    if(lock)
    {
        Engine::mixer()->requestChangeInModel();
        m_varLock.lockForWrite();
        if(m_origData != m_data)
        {
            // qWarning("SampleBuffer::update m_data=%p",m_data);
            // BACKTRACE
            MM_FREE(m_data);
            m_data   = NULL;
            m_frames = 0;
        }
    }

    // File size and sample length limits
    const int fileSizeMax     = 1024;  // MB
    const int sampleLengthMax = 90;    // Minutes

    sample_rate_t samplerate = Engine::mixer()->baseSampleRate();
    QString       cchext
            = "."
              + rawStereoSuffix();  // QString(".f%1r%2").arg(DEFAULT_CHANNELS).arg(samplerate);
    QString filename;

    bool fileLoadError = false;
    if(m_audioFile.isEmpty() && m_origData != NULL && m_origFrames > 0)
    {
        // qInfo("SampleBuffer::update copy origData %p to data
        // %p",m_origData,m_data);
        if(m_data != NULL)
            qWarning("SampleBuffer::update m_data is not null");
        // TODO: reverse- and amplification-property is not covered
        // by following code...
        m_data = MM_ALLOC(sampleFrame, m_origFrames);
        memcpy(m_data, m_origData, m_origFrames * BYTES_PER_FRAME);
        m_frames = m_origFrames;
        if(_keepSettings == false)
        {
            m_loopStartFrame = m_startFrame = 0;
            m_loopEndFrame = m_endFrame = m_frames;
        }
    }
    else if(!m_audioFile.isEmpty()
            && ((filename = tryToMakeAbsolute(m_audioFile)).endsWith(cchext)
                || QFile(filename + cchext).exists())
            && QFile(filename).size() > 102400)
    {
        if(QFile(filename + cchext).exists())
            filename += cchext;
        qInfo("SampleBuffer: Trying cache %s", qPrintable(filename));

        /*
        if(n%BYTES_PER_FRAME!=0)
        {
                fileLoadError=true;
                qWarning("SampleBuffer: Invalid size %lld for
        %s",n,qPrintable(filename));
        }
        else
        */
        if(s_mmap_pointer.contains(filename))  // m_data = MM_ALLOC(
                                               // sampleFrame, m_origFrames );
        {
            quint64 n        = s_mmap_file.value(filename)->size();
            m_origData       = s_mmap_pointer.value(filename);
            m_origFrames     = n / BYTES_PER_FRAME;
            m_mmapped        = true;
            m_data           = m_origData;
            m_frames         = m_origFrames;
            m_loopStartFrame = m_startFrame = 0;
            m_loopEndFrame = m_endFrame = m_frames;
            qInfo("SampleBuffer: File %s already mapped (%d frames,%lld "
                  "bytes) %p",
                  qPrintable(filename), m_frames, n, m_data);
            // qInfo("--- testing #1 m_data[0][0]=%f",m_data[0][0]);
        }
        else
        {
            QFile* file = new QFile(filename);
            qint64 n    = file->size();
            file->open(QFile::ReadOnly);
            if(!(m_origData = (sampleFrame*)(file->map(0, n))))
            {
                delete file;
                m_origFrames  = 0;
                m_mmapped     = false;
                fileLoadError = true;
                qWarning("SampleBuffer: Fail to map %s",
                         qPrintable(filename));
            }
            else
            {
                s_mmap_file.insert(filename, file);
                s_mmap_pointer.insert(filename, m_origData);
                m_origFrames     = n / BYTES_PER_FRAME;
                m_mmapped        = true;
                m_data           = m_origData;
                m_frames         = m_origFrames;
                m_loopStartFrame = m_startFrame = 0;
                m_loopEndFrame = m_endFrame = m_frames;
                // file.unmap();
                qInfo("SampleBuffer: File %s mapped successfully (%d "
                      "frames,%lld bytes) %p",
                      qPrintable(filename), m_frames, n, m_data);
                // qInfo("--- testing #2 m_data[0][0]=%f",m_data[0][0]);
            }
        }
    }
    else if(!m_audioFile.isEmpty())
    {
        if(m_origData)
            qWarning("SampleBuffer already has data...");

        QString filename = tryToMakeAbsolute(m_audioFile);
#ifdef LMMS_BUILD_WIN32
        char* f = qstrdup(filename.toLocal8Bit().constData());
#else
        char* f = qstrdup(filename.toUtf8().constData());
#endif
        sampleS16_t* ibuf     = NULL;
        sample_t*    fbuf     = NULL;
        ch_cnt_t     channels = DEFAULT_CHANNELS;

        m_frames = 0;

        const QFileInfo fileInfo(filename);
        if(fileInfo.size() > fileSizeMax * 1024 * 1024)
        {
            fileLoadError = true;
        }
#ifdef LMMS_HAVE_MPG123
        else if(filename.endsWith(".mp3"))
        {
            // fileLoadError=true;
            m_frames = decodeSampleMPG123(f, fbuf, channels, samplerate);
        }
#endif
        else
        {
            SNDFILE* snd_file;
            SF_INFO  sf_info;
            sf_info.format = 0;
            if((snd_file = sf_open(f, SFM_READ, &sf_info)) != NULL)
            {
                f_cnt_t frames = sf_info.frames;
                int     rate   = sf_info.samplerate;
                if(frames / rate > sampleLengthMax * 60)
                {
                    fileLoadError = true;
                }
                sf_close(snd_file);
            }
        }

        if(!fileLoadError)
        {
            /*
#ifdef LMMS_HAVE_MPG123
            if( m_frames == 0 && fileInfo.suffix() == "mp3" )
            {
                    m_frames = decodeSampleMPG123( f, fbuf, channels,
samplerate );
            }
#endif
            */
#ifdef LMMS_HAVE_OGGVORBIS
            // workaround for a bug in libsndfile or our libsndfile decoder
            // causing some OGG files to be distorted -> try with OGG Vorbis
            // decoder first if filename extension matches "ogg"
            if(m_frames == 0 && fileInfo.suffix() == "ogg")
            {
                m_frames = decodeSampleOGGVorbis(f, ibuf, channels,
                                                 samplerate);
            }
#endif
            if(m_frames == 0)
            {
                m_frames = decodeSampleSF(f, fbuf, channels, samplerate);
            }
#ifdef LMMS_HAVE_OGGVORBIS
            if(m_frames == 0)
            {
                m_frames = decodeSampleOGGVorbis(f, ibuf, channels,
                                                 samplerate);
            }
#endif
            if(m_frames == 0)
            {
                m_frames = decodeSampleDS(f, ibuf, channels, samplerate);
            }

            delete[] f;
        }

        if(m_frames == 0 || fileLoadError)  // if still no frames, bail
        {
            // sample couldn't be decoded, create buffer containing
            // one sample-frame

            if(m_origData && !m_mmapped)
            {
                BACKTRACE
                qCritical("SampleBuffer::update in error origData=%p file=%s",
                          m_origData, qPrintable(m_audioFile));
                MM_FREE(m_origData);
            }
            // not needed
            m_origData   = NULL;
            m_origFrames = 0;

            m_data = MM_ALLOC(sampleFrame, 1);
            memset(m_data, 0, 1 * BYTES_PER_FRAME);
            m_frames         = 1;
            m_loopStartFrame = m_startFrame = 0;
            m_loopEndFrame = m_endFrame = 1;
        }
        else  // otherwise normalize sample rate
        {
            // qInfo("calling normalizeSampleRate this=%p
            // sr=%d",this,samplerate);
            normalizeSampleRate(samplerate, _keepSettings);

            if(!m_audioFile.isEmpty() && !filename.endsWith(cchext) &&
               //(m_frames>102400) &&             // only for big samples?
               QFileInfo(filename + cchext).isWritable())
            {
                qInfo("SampleBuffer: Write cache %s", qPrintable(filename));

                if(m_origData)
                    qFatal("SampleBuffer::update in write cache origData=%p",
                           m_origData);
                m_origData   = NULL;
                m_origFrames = 0;

                // QFile file(filename+cchext);//QDateTime
                // QFileInfo::lastModified() QFileInfo info(file);
                // if(info.exists()) file.unlink();
                writeCacheData(filename + cchext);
                /*
                if(!file.open(QIODevice::WriteOnly))
                        qCritical("SampleBuffer: Can not write
                %s",qPrintable(filename)); else
                {
                        if(m_origData) qFatal("SampleBuffer::update in write
                cache origData=%p",m_origData); m_origData=NULL;
                        m_origFrames=0;

                        QDataStream out(&file);
                        quint64 n=m_frames*BYTES_PER_FRAME;
                        quint64 w=out.writeRawData((const char*)m_data,n);
                        if(n!=w)
                                qWarning("SampleBuffer: Fail to fully write
                %s",qPrintable(filename)); else qInfo("SampleBuffer: Cache
                written %s",qPrintable(filename)); file.close();
                }
                */
            }
        }
    }
    else
    {
        // neither an audio-file nor a buffer to copy from, so create
        // buffer containing one sample-frame

        if(m_origData && !m_mmapped)
        {
            // qCritical("SampleBuffer::update #2 error
            // origData=%p",m_origData);
            MM_FREE(m_origData);
        }
        // not needed
        m_origData   = NULL;
        m_origFrames = 0;

        m_frames = 1;
        m_data   = MM_ALLOC(sampleFrame, m_frames);
        memset(m_data, 0, m_frames * BYTES_PER_FRAME);
        m_loopStartFrame = m_startFrame = 0;
        m_loopEndFrame = m_endFrame = 1;
    }

    if(lock)
    {
        m_varLock.unlock();
        Engine::mixer()->doneChangeInModel();
    }

    emit sampleUpdated();

    if(fileLoadError)
    {
        QString title   = tr("Fail to open file");
        QString message = tr("Audio files are limited to %1 MB "
                             "in size and %2 minutes of playing time")
                                  .arg(fileSizeMax)
                                  .arg(sampleLengthMax);
        if(gui)
        {
            QMessageBox::information(NULL, title, message, QMessageBox::Ok);
        }
        else
        {
            qWarning("%s", qPrintable(message));
            exit(EXIT_FAILURE);
        }
    }

    if(m_data)
    {
        prefetch(0);
        prefetch(m_startFrame);
        prefetch(m_loopStartFrame);
        prefetch(m_endFrame - 4096);
        prefetch(m_loopEndFrame - 4096);
    }
}

void SampleBuffer::convertFromS16(sampleS16_t*& _ibuf,
                                  f_cnt_t       _frames,
                                  int           _channels)
{
    m_frames = _frames;
    m_data   = MM_ALLOC(sampleFrame, m_frames);

    // following code transforms S16 samples into
    // F32 samples and does amplifying & reversing
    // const real_t fac = 1. / MixHelpers::F_S16_MULTIPLIER; // 1 /
    // S16_MULTIPLIER;
    const int ch = (_channels > 1) ? 1 : 0;

    // if reversing is on, we also reverse when
    // scaling
    if(m_reversed)
    {
        int idx = (_frames - 1) * _channels;
        for(f_cnt_t frame = 0; frame < _frames; ++frame)
        {
            if(ch == 0)
            {
                const sample_t s0 = MixHelpers::convertFromS16(_ibuf[idx]);
                //_ibuf[idx] * fac;
                m_data[frame][0] = s0;
                m_data[frame][1] = s0;
            }
            else
            {
                const sample_t s0
                        = MixHelpers::convertFromS16(_ibuf[idx + 0]);
                //_ibuf[idx  ] * fac;
                const sample_t s1
                        = MixHelpers::convertFromS16(_ibuf[idx + 1]);
                //_ibuf[idx+1] * fac;
                m_data[frame][0] = s0;
                m_data[frame][1] = s1;
            }
            idx -= _channels;
        }
    }
    else
    {
        int idx = 0;
        for(f_cnt_t frame = 0; frame < _frames; ++frame)
        {
            if(ch == 0)
            {
                const sample_t s0 = MixHelpers::convertFromS16(_ibuf[idx]);
                //_ibuf[idx] * fac;
                m_data[frame][0] = s0;
                m_data[frame][1] = s0;
            }
            else
            {
                const sample_t s0
                        = MixHelpers::convertFromS16(_ibuf[idx + 0]);
                //_ibuf[idx  ] * fac;
                const sample_t s1
                        = MixHelpers::convertFromS16(_ibuf[idx + 1]);
                //_ibuf[idx+1] * fac;
                m_data[frame][0] = s0;
                m_data[frame][1] = s1;
            }
            idx += _channels;
        }
    }

    delete[] _ibuf;
}

void SampleBuffer::directFloatWrite(sample_t*& _fbuf,
                                    f_cnt_t    _frames,
                                    int        _channels)

{
    m_frames = _frames;
    m_data   = MM_ALLOC(sampleFrame, m_frames);

    const int ch = (_channels > 1) ? 1 : 0;

    // if reversing is on, we also reverse when
    // scaling
    if(m_reversed)
    {
        int idx = (_frames - 1) * _channels;
        for(f_cnt_t frame = 0; frame < _frames; ++frame)
        {
            m_data[frame][0] = _fbuf[idx + 0];
            m_data[frame][1] = _fbuf[idx + ch];
            idx -= _channels;
        }
    }
    else
    {
        int idx = 0;
        for(f_cnt_t frame = 0; frame < _frames; ++frame)
        {
            m_data[frame][0] = _fbuf[idx + 0];
            m_data[frame][1] = _fbuf[idx + ch];
            idx += _channels;
        }
    }

    delete[] _fbuf;
}

void SampleBuffer::normalizeSampleRate(const sample_rate_t _srcSR,
                                       bool                _keepSettings)
{
    // do samplerate-conversion to our default-samplerate
    if(m_data && m_frames > 0 && _srcSR != Engine::mixer()->baseSampleRate())
    {
        // if(m_origData!=m_data)
        // qWarning("SampleBuffer::normalize %s",qPrintable(m_audioFile));
        // qWarning("SampleBuffer::normalize before m_data=%p m_frames=%d
        // sr=%d mixersr=%d",
        //  m_data,m_frames,_srcSR,Engine::mixer()->baseSampleRate());
        /*
          SampleBuffer*
          resampled=resample(_srcSR,Engine::mixer()->baseSampleRate());
          if(m_origData!=m_data) MM_FREE( m_data );
          m_frames = resampled->frames();
          m_data = MM_ALLOC( sampleFrame, m_frames );
          memcpy( m_data, resampled->data(), m_frames*BYTES_PER_FRAME);
          delete resampled;
        */
        resample(_srcSR, Engine::mixer()->baseSampleRate());
        // qWarning("SampleBuffer::normalize after m_data=%p
        // m_frames=%d",m_data,m_frames);
    }

    if(_keepSettings == false)
    {
        // update frame-variables
        m_loopStartFrame = m_startFrame = 0;
        m_loopEndFrame = m_endFrame = m_frames;
    }
}

f_cnt_t SampleBuffer::decodeSampleSF(const char*    _f,
                                     sample_t*&     _buf,
                                     ch_cnt_t&      _channels,
                                     sample_rate_t& _samplerate)
{
    SNDFILE* snd_file;
    SF_INFO  sf_info;
    sf_info.format = 0;
    f_cnt_t frames = 0;
    bool    sf_rr  = false;

    if((snd_file = sf_open(_f, SFM_READ, &sf_info)) != NULL)
    {
        frames = sf_info.frames;

        f_cnt_t nbsamples = sf_info.channels * frames;
        _buf              = new sample_t[nbsamples];

#ifdef REAL_IS_FLOAT
        sf_rr = sf_read_float(snd_file, _buf, sf_info.channels * frames);
#endif
#ifdef REAL_IS_DOUBLE
        FLOAT* tmpbuf = new float[nbsamples];
        sf_rr = sf_read_float(snd_file, tmpbuf, sf_info.channels * frames);
        for(f_cnt_t i = nbsamples - 1; i >= 0; --i)
            _buf[i] = tmpbuf[i];
        delete tmpbuf;
#endif

        if(sf_rr < sf_info.channels * frames)
        {
#ifdef DEBUG_LMMS
            qDebug("SampleBuffer::decodeSampleSF(): could not read"
                   " sample %s: %s",
                   _f, sf_strerror(NULL));
#endif
        }
        _channels   = sf_info.channels;
        _samplerate = sf_info.samplerate;

        sf_close(snd_file);
    }
    else
    {
#ifdef DEBUG_LMMS
        qDebug("SampleBuffer::decodeSampleSF(): could not load "
               "sample %s: %s",
               _f, sf_strerror(NULL));
#endif
    }
    // write down either directly or convert i->f depending on file type

    if(frames > 0 && _buf != NULL)
    {
        directFloatWrite(_buf, frames, _channels);
    }

    return frames;
}

#ifdef LMMS_HAVE_OGGVORBIS

// callback-functions for reading ogg-file

size_t qfileReadCallback(void* _ptr, size_t _size, size_t _n, void* _udata)
{
    return static_cast<QFile*>(_udata)->read((char*)_ptr, _size * _n);
}

int qfileSeekCallback(void* _udata, ogg_int64_t _offset, int _whence)
{
    QFile* f = static_cast<QFile*>(_udata);

    if(_whence == SEEK_CUR)
    {
        f->seek(f->pos() + _offset);
    }
    else if(_whence == SEEK_END)
    {
        f->seek(f->size() + _offset);
    }
    else
    {
        f->seek(_offset);
    }
    return 0;
}

int qfileCloseCallback(void* _udata)
{
    delete static_cast<QFile*>(_udata);
    return 0;
}

long qfileTellCallback(void* _udata)
{
    return static_cast<QFile*>(_udata)->pos();
}

f_cnt_t SampleBuffer::decodeSampleOGGVorbis(const char*    _f,
                                            sampleS16_t*&  _buf,
                                            ch_cnt_t&      _channels,
                                            sample_rate_t& _samplerate)
{
    static ov_callbacks callbacks = {qfileReadCallback, qfileSeekCallback,
                                     qfileCloseCallback, qfileTellCallback};

    OggVorbis_File vf;

    f_cnt_t frames = 0;

    QFile* f = new QFile(_f);
    if(f->open(QFile::ReadOnly) == false)
    {
        delete f;
        return 0;
    }

    int err = ov_open_callbacks(f, &vf, NULL, 0, callbacks);

    if(err < 0)
    {
        switch(err)
        {
            case OV_EREAD:
                qWarning(
                        "SampleBuffer::decodeSampleOGGVorbis():"
                        " media read error");
                break;
            case OV_ENOTVORBIS:
                /*
                  qWarning( "SampleBuffer::decodeSampleOGGVorbis():"
                  " not an Ogg Vorbis file" );
                */
                break;
            case OV_EVERSION:
                qWarning(
                        "SampleBuffer::decodeSampleOGGVorbis():"
                        " vorbis version mismatch");
                break;
            case OV_EBADHEADER:
                qWarning(
                        "SampleBuffer::decodeSampleOGGVorbis():"
                        " invalid Vorbis bitstream header");
                break;
            case OV_EFAULT:
                qWarning(
                        "SampleBuffer::decodeSampleOgg(): "
                        "internal logic fault");
                break;
        }
        delete f;
        return 0;
    }

    ov_pcm_seek(&vf, 0);

    _channels   = ov_info(&vf, -1)->channels;
    _samplerate = ov_info(&vf, -1)->rate;

    ogg_int64_t total = ov_pcm_total(&vf, -1);

    _buf            = new sampleS16_t[total * _channels];
    int  bitstream  = 0;
    long bytes_read = 0;

    do
    {
        bytes_read = ov_read(
                &vf, (char*)&_buf[frames * _channels],
                (total - frames) * _channels * sizeof(sampleS16_t),
                isLittleEndian() ? 0 : 1, sizeof(sampleS16_t), 1, &bitstream);
        if(bytes_read < 0)
        {
            break;
        }
        frames += bytes_read / (_channels * sizeof(sampleS16_t));
    } while(bytes_read != 0 && bitstream == 0);

    ov_clear(&vf);
    // if buffer isn't empty, convert it to F32 and write it down

    if(frames > 0 && _buf != NULL)
    {
        convertFromS16(_buf, frames, _channels);
    }

    return frames;
}
#endif

f_cnt_t SampleBuffer::decodeSampleDS(const char*    _f,
                                     sampleS16_t*&  _buf,
                                     ch_cnt_t&      _channels,
                                     sample_rate_t& _samplerate)
{
    DrumSynth ds;
    f_cnt_t   frames = ds.GetDSFileSamples(_f, _buf, _channels, _samplerate);

    if(frames > 0 && _buf != NULL)
    {
        convertFromS16(_buf, frames, _channels);
    }

    return frames;
}

static sample_t tmp_prefetch_for_mmapped_files = 0.;

void SampleBuffer::prefetch(f_cnt_t _index)
{
    if(!m_data)
        return;
    // prefetch. should be done in a separate thread
    // 4096=typical OS page size / sizeof(sample_t) / nbch
    for(int j = 1; j < 9; j++)
    {
        int i = qMax(0, qMin(m_frames - 1, _index + j * 512));
        ::tmp_prefetch_for_mmapped_files = m_data[i][1];
    }
}

bool SampleBuffer::play(sampleFrame*      _ab,
                        handleState*      _state,
                        const fpp_t       _frames,
                        const frequency_t _freq,
                        const LoopMode    _loopmode)
{
    f_cnt_t startFrame     = m_startFrame;
    f_cnt_t endFrame       = m_endFrame;
    f_cnt_t loopStartFrame = m_loopStartFrame;
    f_cnt_t loopEndFrame   = m_loopEndFrame;

    if(endFrame == 0 || _frames == 0)
    {
        return false;
    }

    // variable for determining if we should currently be playing backwards in
    // a ping-pong loop
    bool is_backwards = _state->isBackwards();

    const double freq_factor = (double)_freq / (double)m_frequency
                               * m_sampleRate
                               / Engine::mixer()->processingSampleRate();

    // calculate how many frames we have in requested pitch
    const f_cnt_t total_frames_for_current_pitch
            = static_cast<f_cnt_t>((endFrame - startFrame) / freq_factor);

    if(total_frames_for_current_pitch == 0)
    {
        return false;
    }

    // this holds the index of the first frame to play
    f_cnt_t play_frame = qMax(_state->m_frameIndex, startFrame);

    if(_loopmode == LoopOff)
    {
        if(play_frame >= endFrame
           || (endFrame - play_frame) / freq_factor <= 0)  //==0
        {
            // the sample is done being played
            return false;
        }
    }
    else if(_loopmode == LoopOn)
    {
        play_frame = getLoopedIndex(play_frame, loopStartFrame, loopEndFrame);
    }
    else
    {
        play_frame
                = getPingPongIndex(play_frame, loopStartFrame, loopEndFrame);
    }

    f_cnt_t fragment_size = (f_cnt_t)(_frames * freq_factor)
                            + MARGIN[_state->interpolationMode()];

    if(m_mmapped)
        prefetch(play_frame);

    sampleFrame* tmp = nullptr;

    // check whether we have to change pitch...
    if(freq_factor != 1. || _state->m_varyingPitch)
    {
        if(m_mmapped)  // undo mmap
        {
            qInfo("SampleBuffer::play unmmap data");
            // Engine::mixer()->requestChangeInModel();
            m_varLock.lockForWrite();
            m_mmapped    = false;
            m_origData   = MM_ALLOC(sampleFrame, m_frames);
            m_origFrames = m_frames;
            memcpy(m_origData, m_data, m_frames * BYTES_PER_FRAME);
            m_data = MM_ALLOC(sampleFrame, m_frames);
            memcpy(m_data, m_origData, m_frames * BYTES_PER_FRAME);
            m_varLock.unlock();
            // Engine::mixer()->doneChangeInModel();
        }

        f_cnt_t input_frames_used       = 0;
        f_cnt_t output_frames_generated = 0;
        /*
        static QHash<QString,QPair<sampleFrame*,int>> cache;
        QString key("%1_%2_%3_%4_%5_%6_%7_%8_%9");
        key=key.arg(play_frame).arg(fragment_size).arg(_loopmode).arg(is_backwards).arg(loopStartFrame)
                .arg(loopEndFrame).arg(endFrame).arg(freq_factor).arg(_frames);
        if(cache.contains(key))
        {
                qInfo("SampleBuffer::play use resample cache");
                QPair<sampleFrame*,int> v=cache.value(key);
                memcpy(_ab[0],v.first,_frames*BYTES_PER_FRAME);
                input_frames_used=v.second;
        }
        else
        */

        sampleFrame* srcBuf = getSampleFragment(
                play_frame, fragment_size, _loopmode, &tmp, &is_backwards,
                loopStartFrame, loopEndFrame, endFrame);

        SampleRate::resample(srcBuf, _ab, fragment_size, _frames,
                             1. / freq_factor, 10, input_frames_used,
                             output_frames_generated,
                             _state->m_resamplingData);

        /*
#ifdef REAL_IS_FLOAT
        {
            SRC_DATA src_data;
            // Generate output
            src_data.data_in = getSampleFragment(
                    play_frame, fragment_size, _loopmode, &tmp, &is_backwards,
                    loopStartFrame, loopEndFrame, endFrame)[0];
            src_data.data_out      = _ab[0];
            src_data.input_frames  = fragment_size;
            src_data.output_frames = _frames;
            src_data.src_ratio     = 1. / freq_factor;
            src_data.end_of_input  = 0;
            int error = src_process(_state->m_resamplingData, &src_data);
            if(error)
            {
                qWarning("SampleBuffer: error while resampling: %s",
                         src_strerror(error));
            }
            else if(src_data.output_frames_gen > _frames)
            {
                qWarning("SampleBuffer: not enough frames: %ld / %d",
                         src_data.output_frames_gen, _frames);
            }

            input_frames_used = src_data.input_frames_used;
        }
#endif
#ifdef REAL_IS_DOUBLE
        // TODO
        qFatal("TODO SampleBuffer");
#endif
          */
        /*
          else
          {
                  qInfo("cache key=%s
          size=%d",qPrintable(key),cache.size()); if(cache.size()>510)
                          foreach(const QString& k,cache.keys())
                          {
                                  QPair<sampleFrame*,int> v=cache.value(k);
                                  MM_FREE(v.first);
                                  cache.remove(k);
                          }
                  sampleFrame* v=MM_ALLOC(sampleFrame,_frames);
                  memcpy(v,_ab[0],_frames*BYTES_PER_FRAME);
                  cache.insert(key,QPair<sampleFrame*,int>(v,src_data.input_frames_used));
          }
        */

        // Advance
        switch(_loopmode)
        {
            case LoopOff:
                play_frame += input_frames_used;
                break;
            case LoopOn:
                play_frame += input_frames_used;
                play_frame = getLoopedIndex(play_frame, loopStartFrame,
                                            loopEndFrame);
                break;
            case LoopPingPong:
            {
                f_cnt_t left = input_frames_used;
                if(_state->isBackwards())
                {
                    play_frame -= input_frames_used;
                    if(play_frame < loopStartFrame)
                    {
                        left -= (loopStartFrame - play_frame);
                        play_frame = loopStartFrame;
                    }
                    else
                        left = 0;
                }
                play_frame += left;
                play_frame = getPingPongIndex(play_frame, loopStartFrame,
                                              loopEndFrame);
                break;
            }
        }
    }
    else
    {
        // we don't have to pitch, so we just copy the sample-data
        // as is into pitched-copy-buffer

        // Generate output
        sampleFrame* pos = getSampleFragment(
                play_frame, _frames, _loopmode, &tmp, &is_backwards,
                loopStartFrame, loopEndFrame, endFrame);
        /*
        qWarning("SampleBuffer::play m_data=%p m_origData=%p m_mmapped=%d\n"
                 "                   m_frames=%d m_origFrames=%d\n"
                 "                   _ab=%p pos=%p tmp=%p play_frame=%d\n"
                 "                   frames=%d lstart=%d lend=%d send=%d",
                 m_data,m_origData,m_mmapped,
                 m_frames,m_origFrames,
                 _ab,pos,tmp, play_frame,
                 _frames, loopStartFrame, loopEndFrame, endFrame );
        qWarning("pos[0][0]=%f",pos[0][0]);
        qWarning("_ab[0][0]=%f",_ab[0][0]);
        */
        memcpy(_ab, pos, _frames * BYTES_PER_FRAME);
        /*
memcpy( _ab,
getSampleFragment( play_frame, _frames, _loopmode, &tmp, &is_backwards,
loopStartFrame, loopEndFrame, endFrame ),
_frames * BYTES_PER_FRAME );
        */
        // Advance
        switch(_loopmode)
        {
            case LoopOff:
                play_frame += _frames;
                break;
            case LoopOn:
                play_frame += _frames;
                play_frame = getLoopedIndex(play_frame, loopStartFrame,
                                            loopEndFrame);
                break;
            case LoopPingPong:
            {
                f_cnt_t left = _frames;
                if(_state->isBackwards())
                {
                    play_frame -= _frames;
                    if(play_frame < loopStartFrame)
                    {
                        left -= (loopStartFrame - play_frame);
                        play_frame = loopStartFrame;
                    }
                    else
                        left = 0;
                }
                play_frame += left;
                play_frame = getPingPongIndex(play_frame, loopStartFrame,
                                              loopEndFrame);
                break;
            }
        }
    }

    if(tmp != nullptr)
    {
        // qWarning("SampleBuffer::play FREE tmp %p",tmp);
        MM_FREE(tmp);
        tmp = nullptr;
    }

    _state->setBackwards(is_backwards);
    _state->setFrameIndex(play_frame);

    if(m_amplification != 1.)
        for(fpp_t i = 0; i < _frames; ++i)
        {
            _ab[i][0] *= m_amplification;
            _ab[i][1] *= m_amplification;
        }

    return true;
}

sampleFrame* SampleBuffer::getSampleFragment(f_cnt_t       _index,
                                             f_cnt_t       _frames,
                                             LoopMode      _loopmode,
                                             sampleFrame** _tmp,
                                             bool*         _backwards,
                                             f_cnt_t       _loopstart,
                                             f_cnt_t       _loopend,
                                             f_cnt_t       _end) const
{
    if(_loopend < _loopstart)
        qSwap(_loopstart, _loopend);
    _index     = qBound(0, _index, m_frames - 1);
    _end       = qBound(1, _end, m_frames);
    _loopstart = qBound(0, _loopstart, _loopend - 1);
    _loopend   = qBound(_loopstart + 1, _loopend, _end);
    _loopstart = qBound(0, _loopstart, m_frames - 1);
    _loopend   = qBound(1, _loopend, m_frames);

    if(_loopmode == LoopOff)
    {
        if(_index + _frames <= _end)
        {
            return m_data + _index;
        }
    }
    else if(_loopmode == LoopOn)
    {
        if(_index + _frames <= _loopend)
        {
            return m_data + _index;
        }
    }
    else  // PingPong
    {
        if(!*_backwards && _index + _frames < _loopend)
        {
            return m_data + _index;
        }
    }

    *_tmp = MM_ALLOC(sampleFrame, _frames);

    if(_loopmode == LoopOff)
    {
        f_cnt_t available = _end - _index;
        memcpy(*_tmp, m_data + _index, available * BYTES_PER_FRAME);
        memset(*_tmp + available, 0, (_frames - available) * BYTES_PER_FRAME);
    }
    else if(_loopmode == LoopOn)
    {
        _index         = qBound(_loopstart, _index, _loopend - 1);
        f_cnt_t copied = qMin(_frames, _loopend - _index);
        memcpy(*_tmp, m_data + _index, copied * BYTES_PER_FRAME);
        f_cnt_t loop_frames = _loopend - _loopstart;
        while(copied < _frames)
        {
            f_cnt_t todo = qMin(_frames - copied, loop_frames);
            memcpy(*_tmp + copied, m_data + _loopstart,
                   todo * BYTES_PER_FRAME);
            copied += todo;
        }
    }
    else
    {
        _index            = qBound(_loopstart, _index, _loopend - 1);
        f_cnt_t pos       = _index;
        bool    backwards = pos < _loopstart ? false : *_backwards;
        f_cnt_t copied    = 0;

        if(backwards)
        {
            copied = qMin(_frames, pos - _loopstart);
            if(copied > 0)
            {
                for(int i = 0; i < copied; i++)
                {
                    (*_tmp)[i][0] = m_data[pos - i][0];
                    (*_tmp)[i][1] = m_data[pos - i][1];
                }
                pos -= copied;
            }
            else
                copied = 0;
            if(pos == _loopstart)
                backwards = false;
        }
        else
        {
            copied = qMin(_frames, _loopend - pos);
            if(copied > 0)
            {
                memcpy(*_tmp, m_data + pos, copied * BYTES_PER_FRAME);
                pos += copied;
            }
            else
                copied = 0;
            if(pos == _loopend)
                backwards = true;
        }

        while(copied < _frames)
        {
            if(backwards)
            {
                f_cnt_t todo = qMin(_frames - copied, pos - _loopstart);
                if(todo > 0)
                {
                    for(int i = 0; i < todo; i++)
                    {
                        (*_tmp)[copied + i][0] = m_data[pos - i][0];
                        (*_tmp)[copied + i][1] = m_data[pos - i][1];
                    }
                    pos -= todo;
                    copied += todo;
                }
                else
                    todo = 0;
                if(pos <= _loopstart)
                    backwards = false;
            }
            else
            {
                f_cnt_t todo = qMin(_frames - copied, _loopend - pos);
                if(todo > 0)
                {
                    memcpy(*_tmp + copied, m_data + pos,
                           todo * BYTES_PER_FRAME);
                    pos += todo;
                    copied += todo;
                }
                else
                    todo = 0;
                if(pos >= _loopend)
                    backwards = true;
            }
        }
        *_backwards = backwards;
    }

    return *_tmp;
}

f_cnt_t SampleBuffer::getLoopedIndex(f_cnt_t _index,
                                     f_cnt_t _startf,
                                     f_cnt_t _endf) const
{
    if(_index < _endf || _endf <= _startf)
        return _index;

    return _startf + (_index - _startf) % (_endf - _startf);
}

f_cnt_t SampleBuffer::getPingPongIndex(f_cnt_t _index,
                                       f_cnt_t _startf,
                                       f_cnt_t _endf) const
{
    if(_index < _endf || _endf <= _startf)
        return _index;

    const f_cnt_t looplen = _endf - _startf;
    const f_cnt_t looppos = (_index - _endf) % (looplen * 2);

    return (looppos < looplen) ? _endf - looppos
                               : _startf + (looppos - looplen);
}

f_cnt_t SampleBuffer::findClosestZero(f_cnt_t _index)
{
    sample_t vval = m_data[_index][0];
    if(vval == 0.)
        return _index;

    bool vsign = signbit(vval);

    f_cnt_t left = _index - 1;
    while((left > 0) && (signbit(m_data[left][0]) == vsign))
        left--;
    left++;

    f_cnt_t right = _index + 1;
    while((right < m_frames - 1) && (signbit(m_data[right][0]) == vsign))
        right++;
    right--;

    if(right - _index > _index - left)
        return left;
    return right;
}

// fully rewriten. gi0e5b06
void SampleBuffer::visualize(QPainter&    _p,
                             const QRect& _r,
                             const QRect& _clip,
                             f_cnt_t      _from,
                             f_cnt_t      _to)
{
    if(!m_data)
        return;
    if(m_frames == 0)
        return;

    if(_from > _to)
        qSwap(_from, _to);
    _from = qBound(0, _from, m_frames);
    _to   = qBound(0, _to, m_frames);

    if(_from == _to)
        return;

    //_p.setClipRect( _clip );
    const int xr = _r.x();
    const int yr = _r.y();
    const int wr = _r.width();
    const int hr = _r.height();

    int nbp = qMin<int>(wr, _to - _from);
    if(nbp <= 1)
        return;
    if(nbp > 10000)
        nbp = 10000;

    QPointF* lc = new QPointF[nbp];
    QPointF* rc = new QPointF[nbp];

    for(int i = 0; i < nbp; i++)
    {
        FLOAT   tp    = FLOAT(i) / FLOAT(nbp);
        f_cnt_t frame = _from + (_to - _from) * tp;
        FLOAT   xp    = xr + FLOAT(wr - 1) / FLOAT(nbp - 1) * FLOAT(i);
        FLOAT   ypl   = yr
                    + FLOAT(hr - 1) / 2.
                              * (1. + m_amplification * m_data[frame][0]);
        FLOAT ypr = yr
                    + FLOAT(hr - 1) / 2.
                              * (1. + m_amplification * m_data[frame][1]);
        lc[i] = QPointF(xp, ypl);
        rc[i] = QPointF(xp, ypr);
    }

    //_p.setRenderHint(QPainter::Antialiasing);
    _p.setPen(Qt::black);
    _p.drawPolyline(rc, nbp);
    _p.setPen(Qt::white);
    _p.drawPolyline(lc, nbp);
    delete[] lc;
    delete[] rc;
}

/*
void SampleBuffer::visualize( QPainter & _p, const QRect & _dr,
                              const QRect & _clip, f_cnt_t _from_frame,
f_cnt_t _to_frame )
{
        const bool focus_on_range =
                _to_frame <= m_frames &&
                0 <= _from_frame &&
                _from_frame < _to_frame;

        //_p.setClipRect( _clip );
        const int w = _dr.width();
        const int h = _dr.height();

        const int yb = h / 2 + _dr.y();
        const FLOAT y_space = h*0.5f;
        const int nb_frames = focus_on_range ? _to_frame - _from_frame :
m_frames;

        const int fpp = tLimit<int>( nb_frames / w, 1, 20 );
        QPointF * l = new QPointF[nb_frames / fpp + 1];
        QPointF * r = new QPointF[nb_frames / fpp + 1];
        int n = 0;
        const int xb = _dr.x();
        const int first = focus_on_range ? _from_frame : 0;
        const int last = focus_on_range ? _to_frame : m_frames;
        for( int frame = first; frame < last; frame += fpp )
        {
                l[n] = QPointF( xb + ( (frame - first) * double( w ) /
nb_frames ), ( yb - ( m_data[frame][0] * y_space * m_amplification ) ) ); r[n]
= QPointF( xb + ( (frame - first) * double( w ) / nb_frames ), ( yb - (
m_data[frame][1] * y_space * m_amplification ) ) );
                ++n;
        }
        _p.setRenderHint( QPainter::Antialiasing );
        _p.drawPolyline( l, nb_frames / fpp );
        _p.drawPolyline( r, nb_frames / fpp );
        delete[] l;
        delete[] r;
}
*/

const QString SampleBuffer::rawStereoSuffix()
{
    // qInfo("SampleBuffer::rawStereoSuffix");
    return QString("f%1s%2")
            .arg(DEFAULT_CHANNELS)
            .arg(Engine::mixer()->baseSampleRate());
}

const QString SampleBuffer::rawSurroundSuffix()
{
    // qInfo("SampleBuffer::rawSurroundSuffix");
    return QString("f%1s%2")
            .arg(SURROUND_CHANNELS)
            .arg(Engine::mixer()->processingSampleRate());
}

QString SampleBuffer::selectAudioFile(const QString& _file)
{
    FileDialog ofd(NULL, tr("Open audio file"));

    QString dir;
    QString file;
    if(!_file.isNull() && _file != "")
    {
        if(QFileInfo(_file).isRelative())
        {
            QString g = ConfigManager::inst()->userSamplesDir() + _file;
            if(QFileInfo(g).exists())
                file = g;
            else
                file = ConfigManager::inst()->factorySamplesDir() + _file;
        }
        dir = QFileInfo(file).absolutePath();
    }
    else
    {
        dir = ConfigManager::inst()->userSamplesDir();
    }
    // change dir to position of previously opened file
    ofd.setDirectory(dir);
    ofd.setFileMode(FileDialog::ExistingFiles);

    // set filters
    QStringList types;
    types << tr("All Audio-Files (*.wav *.ogg *.ds *.flac *.mp3 *.spx *.voc "
                "*.aif *.aiff *.au *.%1 *.%2)")
                     .arg(rawStereoSuffix())
                     .arg(rawSurroundSuffix())
          << tr("Wave-Files (*.wav)") << tr("OGG-Files (*.ogg)")
          << tr("DrumSynth-Files (*.ds)") << tr("FLAC-Files (*.flac)")
          << tr("SPEEX-Files (*.spx)")
          << tr("MP3-Files (*.mp3)")
          //<< tr( "MIDI-Files (*.mid)" )
          << tr("VOC-Files (*.voc)") << tr("AIFF-Files (*.aif *.aiff)")
          << tr("AU-Files (*.au)")
          << tr("Stereo RAW-Files (*.%1)").arg(rawStereoSuffix())
#ifndef LMMS_DISABLE_SURROUND
          << tr("Surround RAW-Files (*.%1)").arg(rawSurroundSuffix())
#endif
            //<< tr( "MOD-Files (*.mod)" )
            ;
    ofd.setNameFilters(types);
    if(!file.isEmpty())
    {
        // select previously opened file
        ofd.selectFile(file);  // QFileInfo( m_audioFile ).fileName() );
    }

    if(ofd.exec() == QDialog::Accepted)
    {
        if(ofd.selectedFiles().isEmpty())
        {
            return QString::null;
        }
        return tryToMakeRelative(ofd.selectedFiles()[0]);
    }

    return QString::null;
}

QString SampleBuffer::openAudioFile() const
{
    return selectAudioFile(m_audioFile);
}

QString SampleBuffer::openAndSetAudioFile()
{
    QString fileName = openAudioFile();
    if(!fileName.isEmpty())
        setAudioFile(fileName);
    return fileName;
}

QString SampleBuffer::openAndSetWaveformFile()
{
    if(m_audioFile.isEmpty())
    {
        m_audioFile = ConfigManager::inst()->factorySamplesDir()
                      + "waveforms/10saw.flac";
    }

    QString fileName = openAudioFile();

    if(!fileName.isEmpty())
    {
        setAudioFile(fileName);
    }
    else
    {
        m_audioFile = "";
    }

    return fileName;
}

#undef LMMS_HAVE_FLAC_STREAM_ENCODER_H /* not yet... */
#undef LMMS_HAVE_FLAC_STREAM_DECODER_H

#ifdef LMMS_HAVE_FLAC_STREAM_ENCODER_H
FLAC__StreamEncoderWriteStatus
        flacStreamEncoderWriteCallback(const FLAC__StreamEncoder*
                                       /*_encoder*/,
                                       const FLAC__byte _buffer[],
                                       unsigned int /* _samples*/,
                                       unsigned int _bytes,
                                       unsigned int /* _current_frame*/,
                                       void* _client_data)
{
    /*	if( _bytes == 0 )
            {
                    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
            }*/
    return (static_cast<QBuffer*>(_client_data)
                    ->write((const char*)_buffer, _bytes)
            == (int)_bytes)
                   ? FLAC__STREAM_ENCODER_WRITE_STATUS_OK
                   : FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
}

void flacStreamEncoderMetadataCallback(const FLAC__StreamEncoder*,
                                       const FLAC__StreamMetadata* _metadata,
                                       void* _client_data)
{
    QBuffer* b = static_cast<QBuffer*>(_client_data);
    b->seek(0);
    b->write((const char*)_metadata, sizeof(*_metadata));
}

#endif

QString& SampleBuffer::toBase64(QString& _dst) const
{
#ifdef LMMS_HAVE_FLAC_STREAM_ENCODER_H
    const f_cnt_t FRAMES_PER_BUF = 1152;

    FLAC__StreamEncoder* flac_enc = FLAC__stream_encoder_new();
    FLAC__stream_encoder_set_channels(flac_enc, DEFAULT_CHANNELS);
    FLAC__stream_encoder_set_blocksize(flac_enc, FRAMES_PER_BUF);
    /*	FLAC__stream_encoder_set_do_exhaustive_model_search( flac_enc, true );
            FLAC__stream_encoder_set_do_mid_side_stereo( flac_enc, true );*/
    FLAC__stream_encoder_set_sample_rate(flac_enc,
                                         Engine::mixer()->sampleRate());
    QBuffer ba_writer;
    ba_writer.open(QBuffer::WriteOnly);

    FLAC__stream_encoder_set_write_callback(flac_enc,
                                            flacStreamEncoderWriteCallback);
    FLAC__stream_encoder_set_metadata_callback(
            flac_enc, flacStreamEncoderMetadataCallback);
    FLAC__stream_encoder_set_client_data(flac_enc, &ba_writer);
    if(FLAC__stream_encoder_init(flac_enc) != FLAC__STREAM_ENCODER_OK)
    {
        qWarning("error within FLAC__stream_encoder_init()!");
    }
    f_cnt_t frame_cnt = 0;
    while(frame_cnt < m_frames)
    {
        f_cnt_t remaining
                = qMin<f_cnt_t>(FRAMES_PER_BUF, m_frames - frame_cnt);
        FLAC__int32 buf[FRAMES_PER_BUF * DEFAULT_CHANNELS];
        for(f_cnt_t f = 0; f < remaining; ++f)
        {
            for(ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ++ch)
            {
                buf[f * DEFAULT_CHANNELS + ch] = (FLAC__int32)
                        //( Mixer::clip( m_data[f+frame_cnt][ch] ) *
                        //  S16_MULTIPLIER )
                        (MixHelpers::convertToS16(m_data[f + frame_cnt][ch]));
            }
        }
        FLAC__stream_encoder_process_interleaved(flac_enc, buf, remaining);
        frame_cnt += remaining;
    }
    FLAC__stream_encoder_finish(flac_enc);
    FLAC__stream_encoder_delete(flac_enc);
    // qInfo("SampleBufer::toBase64 %d %d", frame_cnt, (int)ba_writer.size()
    // );
    ba_writer.close();

    _dst = base64::encodeChars(ba_writer.buffer().data(),
                               ba_writer.buffer().size());

#else /* LMMS_HAVE_FLAC_STREAM_ENCODER_H */

    // base64::encode((const char*)m_data, m_frames * sizeof(sampleFrame),
    // _dst);
#ifdef REAL_IS_FLOAT
    _dst = base64::encodeFloats((const FLOAT*)m_data, m_frames * DEFAULT_CHANNELS);
#endif
#ifdef REAL_IS_DOUBLE
    _dst = base64::encodeDoublesAsFloats((const double*)m_data, m_frames * DEFAULT_CHANNELS);
#endif

#endif /* LMMS_HAVE_FLAC_STREAM_ENCODER_H */

    return _dst;
}

void SampleBuffer::resample(const sample_rate_t _srcSR,
                            const sample_rate_t _dstSR)
{
    const double  frq_ratio  = double(_dstSR) / double(_srcSR);
    const f_cnt_t dst_frames = static_cast<f_cnt_t>(m_frames * frq_ratio);
    sampleFrame*  dst_data   = MM_ALLOC(sampleFrame, dst_frames);
    f_cnt_t       input_frames_used       = 0;
    f_cnt_t       output_frames_generated = 0;

    bool success = SampleRate::resample(
            m_data, dst_data, m_frames, dst_frames, frq_ratio, 10,
            input_frames_used, output_frames_generated, nullptr);
    /*
    int          error;

#ifdef REAL_IS_FLOAT
    {
        // yeah, libsamplerate, let's rock with sinc-interpolation!
        SRC_STATE* state;
        if((state
            = src_new(SRC_SINC_MEDIUM_QUALITY, DEFAULT_CHANNELS, &error))
           != NULL)
        {
            SRC_DATA src_data;
            src_data.end_of_input  = 1;
            src_data.data_in       = m_data[0];    // data[0];
            src_data.data_out      = dst_data[0];  // dst_buf[0];
            src_data.input_frames  = m_frames;
            src_data.output_frames = dst_frames;
            src_data.src_ratio     = frq_ratio;
            if((error = src_process(state, &src_data)))
            {
                qFatal("SampleBuffer::resample(): error while resampling: %s",
                       src_strerror(error));
            }
            src_delete(state);
        }
        else
        {
            qWarning("Error: src_new() failed in sample_buffer.cpp!");
        }
    }
#endif
#ifdef REAL_IS_DOUBLE
    {
        // double
        real_t src_step = 1. / frq_ratio;
        real_t x         = real_t(dst_frames - 1) * src_step;
        for(f_cnt_t f = dst_frames - 1; f >= 0.; --f)
        {
            for(ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ch++)
                dst_data[f][ch] = m_data[f_cnt_t(x)][ch];
            x -= src_step;
        }
        error = 0;
    }
#endif

    if(!error)
    */

    if(success)
    {
        if(m_data != m_origData)
            MM_FREE(m_data);
        m_data   = dst_data;
        m_frames = output_frames_generated;
    }
    else
        MM_FREE(dst_data);
}

void SampleBuffer::retune(  // const sample_rate_t _srcSR,
        const double _semitones)
{
    update(true);  // Test?
    if(_semitones == 0.)
        return;

    const double  frq_ratio  = exp2(_semitones / -12.);
    const f_cnt_t dst_frames = static_cast<f_cnt_t>(m_frames * frq_ratio);
    sampleFrame*  dst_data   = MM_ALLOC(sampleFrame, dst_frames);
    f_cnt_t       input_frames_used       = 0;
    f_cnt_t       output_frames_generated = 0;

    bool success = SampleRate::resample(
            m_data, dst_data, m_frames, dst_frames, frq_ratio, 10,
            input_frames_used, output_frames_generated, nullptr);

    /*
    int           error;

#ifdef REAL_IS_FLOAT
    {
        // yeah, libsamplerate, let's rock with sinc-interpolation!
        SRC_STATE* state;
        if((state
            = src_new(SRC_SINC_MEDIUM_QUALITY, DEFAULT_CHANNELS, &error))
           != NULL)
        {
            SRC_DATA src_data;
            src_data.end_of_input  = 1;
            src_data.data_in       = m_data[0];    // data[0];
            src_data.data_out      = dst_data[0];  // dst_buf[0];
            src_data.input_frames  = m_frames;
            src_data.output_frames = dst_frames;
            src_data.src_ratio     = frq_ratio;
            if((error = src_process(state, &src_data)))
            {
                qWarning("SampleBuffer: error while retuning: %s",
                         src_strerror(error));
            }
            src_delete(state);
        }
        else
        {
            qWarning("Error: src_new() failed in sample_buffer.cpp!");
        }
    }
#endif
#ifdef REAL_IS_DOUBLE
    {
        // double
        real_t src_ratio = 1. / frq_ratio;
        real_t x         = real_t(dst_frames - 1) * src_ratio;
        for(f_cnt_t f = dst_frames - 1; f >= 0.; --f)
        {
            for(ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ch++)
                dst_data[f][ch] = m_data[f_cnt_t(x)][ch];
            x -= src_ratio;
        }
        error = 0;
    }
#endif

    if(!error)
    */

    if(success)
    {
        if(m_data != m_origData)
            MM_FREE(m_data);
        m_data   = dst_data;
        m_frames = output_frames_generated;

        // TODO: adjust points with ratio
    }
    else
        MM_FREE(dst_data);
}

void SampleBuffer::setAudioFile(const QString& _audioFile)
{
    m_audioFile = tryToMakeRelative(_audioFile);
    update();
}

#ifdef LMMS_HAVE_MPG123

//#include <out123.h>
//#include <mpg123.h>
//#include <stdio.h>
//#include <strings.h>

void SampleBuffer::cleanupMPG123(void* mh)
{
    /* It's really to late for error checks here;-) */
    mpg123_close((mpg123_handle*)mh);
    mpg123_delete((mpg123_handle*)mh);
    mpg123_exit();
}

f_cnt_t SampleBuffer::decodeSampleMPG123(const char*    _infile,
                                         sample_t*&     buf_,
                                         ch_cnt_t&      channels_,
                                         sample_rate_t& samplerate_)
{
    mpg123_handle* mh = NULL;
    // char *driver = NULL;
    // char *outfile = NULL;
    // const char *encname;
    unsigned char* buffer      = NULL;
    size_t         buffer_size = 0;
    size_t         done        = 0;
    int            channels    = 0;
    int            encoding    = 0;
    int            framesize   = 1;
    long           rate        = 0;
    int            err         = MPG123_OK;
    off_t          samples     = 0;

    err = mpg123_init();
    if(err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL)
    {
        qCritical("libmpg123: setup goes wrong: %s",
                  mpg123_plain_strerror(err));
        cleanupMPG123(mh);
        return -1;
    }

    mpg123_format_none(mh);
    mpg123_format(mh, 48000, MPG123_STEREO, MPG123_ENC_FLOAT_32);
    mpg123_format(mh, 44100, MPG123_STEREO, MPG123_ENC_FLOAT_32);
    mpg123_format(mh, 32000, MPG123_STEREO, MPG123_ENC_FLOAT_32);
    mpg123_format(mh, 24000, MPG123_STEREO, MPG123_ENC_FLOAT_32);
    mpg123_format(mh, 22050, MPG123_STEREO, MPG123_ENC_FLOAT_32);
    mpg123_format(mh, 16000, MPG123_STEREO, MPG123_ENC_FLOAT_32);
    mpg123_format(mh, 12000, MPG123_STEREO, MPG123_ENC_FLOAT_32);
    mpg123_format(mh, 11025, MPG123_STEREO, MPG123_ENC_FLOAT_32);
    mpg123_format(mh, 8000, MPG123_STEREO, MPG123_ENC_FLOAT_32);

    mpg123_param(mh, MPG123_VERBOSE, 2, 2.);
    mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_FORCE_FLOAT, 0.);
    mpg123_param(mh, MPG123_ADD_FLAGS, MPG123_FORCE_STEREO | MPG123_FUZZY,
                 0.);

    /* Let mpg123 work with the file, that excludes MPG123_NEED_MORE
       messages. */
    if(mpg123_open(mh, _infile) != MPG123_OK
       /* Peek into track and get first output format. */
       || mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK)
    {
        qWarning("libmpg123: could not open: %s", mpg123_strerror(mh));
        cleanupMPG123(mh);
        return -1;
    }

    // qInfo("libmpg123: rate=%ld, channels=%d, encoding=%d",
    //      rate,channels,encoding);

    // encoding=MPG123_ENC_FLOAT_32;//MPG123_FORCE_STEREO|MPG123_FORCE_FLOAT;
    // mpg123_param(mh, MPG123_OUTSCALE, 1, 1.);
    // rate=m_sampleRate;
    // channels=2;
    // encoding=1;

    /* Ensure that this output format will not change
       (it might, when we allow it). */
    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);

    framesize = mpg123_encsize(encoding);
    // channels*sizeof(f loat);
    // framesize=sizeof(sample_t)=sizeof(f loat)=4

    // qInfo("libmpg123: rate=%ld, channels=%d, encoding=%d,
    // framesize=%d",
    //      rate,channels,encoding,framesize);

    QByteArray b;
    /* Buffer could be almost any size here, mpg123_outblock() is just
       some recommendation. The size should be a multiple of the PCM
       frame size. */
    buffer_size = mpg123_outblock(mh);
    buffer      = MM_ALLOC(unsigned char, buffer_size);

    size_t played = 0;
    do
    {
        err = mpg123_read(mh, buffer, buffer_size, &done);
        b.append((const char*)buffer, done);
        played += done;
        // qInfo("libmpg123: done=%ld played=%ld",done,played);

        samples += done / framesize;
        /* We are not in feeder mode, so MPG123_OK, MPG123_ERR and
           MPG123_NEW_FORMAT are the only possibilities.
           We do not handle a new format, MPG123_DONE is the end... so
           abort on anything not MPG123_OK. */
    } while(done && err == MPG123_OK);

    MM_FREE(buffer);

    if(err != MPG123_DONE)
        qWarning("Warning: Decoding ended prematurely because: %s",
                 err == MPG123_ERR ? mpg123_strerror(mh)
                                   : mpg123_plain_strerror(err));

    // qInfo("libmpg123: %li samples", (long)samples);
    cleanupMPG123(mh);

    buf_ = new sample_t[samples];
    memcpy(buf_, b.constData(), samples * sizeof(sample_t));
    directFloatWrite(buf_, samples / channels, channels);

    channels_   = channels;
    samplerate_ = rate;
    return samples / channels;
}

#endif

#ifdef LMMS_HAVE_FLAC_STREAM_DECODER_H

struct flacStreamDecoderClientData
{
    QBuffer* read_buffer;
    QBuffer* write_buffer;
};

FLAC__StreamDecoderReadStatus
        flacStreamDecoderReadCallback(const FLAC__StreamDecoder*
                                      /*_decoder*/,
                                      FLAC__byte*   _buffer,
                                      unsigned int* _bytes,
                                      void*         _client_data)
{
    int res = static_cast<flacStreamDecoderClientData*>(_client_data)
                      ->read_buffer->read((char*)_buffer, *_bytes);

    if(res > 0)
    {
        *_bytes = res;
        return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }
    *_bytes = 0;
    return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
}

FLAC__StreamDecoderWriteStatus flacStreamDecoderWriteCallback(
        const FLAC__StreamDecoder* /*_decoder*/,
        const FLAC__Frame*       _frame,
        const FLAC__int32* const _buffer[],
        void*                    _client_data)
{
    if(_frame->header.channels != 2)
    {
        qWarning("flacStreamDecoderWriteCallback: channels != 2");
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    if(_frame->header.bits_per_sample != 16)
    {
        qWarning("flacStreamDecoderWriteCallback: bits_per_sample != 16");
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    const f_cnt_t frames = _frame->header.blocksize;
    for(f_cnt_t frame = 0; frame < frames; ++frame)
    {
        // sampleFrame sframe = { _buffer[0][frame] /
        // MixHelpers::F_S16_MULTIPLIER, _buffer[1][frame]
        // /
        // MixHelpers::F_S16_MULTIPLIER } ;
        sampleFrame sframe = {MixHelpers::convertFromS16(_buffer[0][frame]),
                              MixHelpers::convertFromS16(_buffer[1][frame])};
        static_cast<flacStreamDecoderClientData*>(_client_data)
                ->write_buffer->write((const char*)sframe, sizeof(sframe));
    }
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void flacStreamDecoderMetadataCallback(const FLAC__StreamDecoder*,
                                       const FLAC__StreamMetadata*,
                                       void* /*_client_data*/)
{
    qWarning(
            "flacStreamDecoderMetadataCallback: stream decoder metadata "
            "callback");
    /*	QBuffer * b = static_cast<QBuffer *>( _client_data );
            b->seek( 0 );
            b->write( (const char *) _metadata, sizeof( *_metadata ) );*/
}

void flacStreamDecoderErrorCallback(const FLAC__StreamDecoder*,
                                    FLAC__StreamDecoderErrorStatus _status,
                                    void* /*_client_data*/)
{
    qWarning("flacStreamDecoderErrorCallback: error callback! %d", _status);
    // what to do now??
}

#endif

void SampleBuffer::loadFromBase64(const QString& _data)
{
#ifdef LMMS_HAVE_FLAC_STREAM_DECODER_H
    char* dst   = NULL;
    int   dsize = base64::decodeChars(_data, &dst);

    QByteArray orig_data = QByteArray::fromRawData(dst, dsize);
    QBuffer    ba_reader(&orig_data);
    ba_reader.open(QBuffer::ReadOnly);

    QBuffer ba_writer;
    ba_writer.open(QBuffer::WriteOnly);

    flacStreamDecoderClientData cdata = {&ba_reader, &ba_writer};

    FLAC__StreamDecoder* flac_dec = FLAC__stream_decoder_new();

    FLAC__stream_decoder_set_read_callback(flac_dec,
                                           flacStreamDecoderReadCallback);
    FLAC__stream_decoder_set_write_callback(flac_dec,
                                            flacStreamDecoderWriteCallback);
    FLAC__stream_decoder_set_error_callback(flac_dec,
                                            flacStreamDecoderErrorCallback);
    FLAC__stream_decoder_set_metadata_callback(
            flac_dec, flacStreamDecoderMetadataCallback);
    FLAC__stream_decoder_set_client_data(flac_dec, &cdata);

    FLAC__stream_decoder_init(flac_dec);

    FLAC__stream_decoder_process_until_end_of_stream(flac_dec);

    FLAC__stream_decoder_finish(flac_dec);
    FLAC__stream_decoder_delete(flac_dec);

    ba_reader.close();

    orig_data = ba_writer.buffer();
    // qInfo("SampleBuffer::loadFromBase64 %d", (int) orig_data.size() );

    if(orig_data.size() % BYTES_PER_FRAME != 0)  // osize/BYTES
    {
        qCritical("SampleBuffer::loadFromBase64#1 invalid data");
        // BACKTRACE
    }
    m_origFrames = orig_data.size() / BYTES_PER_FRAME;
    if(!m_mmapped)
        MM_FREE(m_origData);
    else
        m_mmapped = false;
    m_origData = MM_ALLOC(sampleFrame, m_origFrames);
    memcpy(m_origData, orig_data.data(), orig_data.size());

#else /* LMMS_HAVE_FLAC_STREAM_DECODER_H */

#ifdef REAL_IS_FLOAT
    FLOAT* dst = NULL;
    int dsize = base64::decodeFloats(_data, &dst);
#endif
#ifdef REAL_IS_DOUBLE
    double* dst = NULL;
    int dsize = base64::decodeFloatsAsDoubles(_data, &dst);
#endif

    if(dsize % BYTES_PER_FRAME != 0)  // dsize/BYTES
    {
        qCritical("SampleBuffer::loadFromBase64#2 invalid data");
        // BACKTRACE
    }
    m_origFrames = dsize / BYTES_PER_FRAME;
    if(!m_mmapped)
        MM_FREE(m_origData);
    else
        m_mmapped = false;
    m_origData = MM_ALLOC(sampleFrame, m_origFrames);
    memcpy(m_origData, dst, dsize);

#endif

    delete[] dst;

    m_audioFile = QString();
    update();
}

void SampleBuffer::setStartFrame(f_cnt_t _f)
{
    if(_f < 0 || _f >= m_frames)
        _f = 0;
    m_startFrame = _f;
}

void SampleBuffer::setEndFrame(f_cnt_t _f)
{
    if(_f <= 0 || _f > m_frames)
        _f = m_frames;
    m_endFrame = _f;
}

void SampleBuffer::setLoopStartFrame(f_cnt_t _f)
{
    if(_f >= m_endFrame)
        _f = m_endFrame - 1;
    if(_f < m_startFrame)
        _f = m_startFrame;
    m_loopStartFrame = _f;
}

void SampleBuffer::setLoopEndFrame(f_cnt_t _f)
{
    if(_f <= m_startFrame)
        _f = m_startFrame + 1;
    if(_f > m_endFrame)
        _f = m_endFrame;
    m_loopEndFrame = _f;
}

void SampleBuffer::setAllPointFrames(f_cnt_t _start,
                                     f_cnt_t _end,
                                     f_cnt_t _loopstart,
                                     f_cnt_t _loopend)
{
    setStartFrame(_start);
    setEndFrame(_end);
    setLoopStartFrame(_loopstart);
    setLoopEndFrame(_loopend);
}

void SampleBuffer::setAmplification(real_t _a)
{
    m_amplification = _a;
    emit sampleUpdated();
}

void SampleBuffer::setReversed(bool _on)
{
    m_reversed = _on;
    update(true);
}

void SampleBuffer::getDataFrame(f_cnt_t _f, sample_t& ch0_, sample_t& ch1_)
{
    ch0_ = 0.;
    ch1_ = 0.;
    if(_f < 0 || _f >= m_origFrames)
    {
        // qWarning("SampleBuffer::getDataFrame invalid frame _f=%d",_f);
        return;
    }
    if(m_origData == NULL)
    {
        qWarning("SampleBuffer::getDataFrame m_data is null");
        return;
    }
    ch0_ = m_origData[_f][0];
    ch1_ = m_origData[_f][1];

    if(_f == 1000)
        qInfo("SampleBuffer::getDataFrame f=%d ch0=%f ch1=%f", _f, ch0_,
              ch1_);
}

void SampleBuffer::setDataFrame(f_cnt_t _f, sample_t _ch0, sample_t _ch1)
{
    if(_f < 0 || _f >= m_origFrames)
    {
        // qWarning("SampleBuffer::setDataFrame invalid frame _f=%d",_f);
        return;
    }
    if(m_origData == NULL)
    {
        qWarning("SampleBuffer::setDataFrame m_data is null");
        return;
    }
    if(m_mmapped)
    {
        qWarning("SampleBuffer::setDataFrame sample is mmapped");
        return;
    }
    m_origData[_f][0] = _ch0;
    m_origData[_f][1] = _ch1;
    if(_f == 1000)
        qInfo("SampleBuffer::setDataFrame f=%d ch0=%f ch1=%f", _f, _ch0,
              _ch1);
}

void SampleBuffer::writeCacheData(QString _fileName) const
{
    qInfo("SampleBuffer: Write cache %s", qPrintable(_fileName));
    QFile file(_fileName);
    if(!file.open(QIODevice::WriteOnly))
        qCritical("SampleBuffer: Can not write %s", qPrintable(_fileName));
    else
    {
        // if(m_origData) qFatal("SampleBuffer::update in write cache
        // origData=%p",m_origData); m_origData=NULL; m_origFrames=0;

        QDataStream out(&file);
        quint64     n = m_frames * BYTES_PER_FRAME;
        quint64     w = out.writeRawData((const char*)m_data, n);
        if(n != w)
            qWarning("SampleBuffer: Fail to fully write %s",
                     qPrintable(_fileName));
        else
            qInfo("SampleBuffer: Cache written %s", qPrintable(_fileName));
        file.close();
    }
}

QString SampleBuffer::tryToMakeRelative(const QString& file)
{
    if(QFileInfo(file).isRelative() == false)
    {
        // Normalize the path
        QString f(QDir::cleanPath(file));

        // First, look in factory samples
        // Isolate "samples/" from "data:/samples/"
        QString samplesSuffix
                = ConfigManager::inst()->factorySamplesDir().mid(
                        ConfigManager::inst()->dataDir().length());

        // Iterate over all valid "data:/" searchPaths
        for(const QString& path : QDir::searchPaths("data"))
        {
            QString samplesPath = QDir::cleanPath(path + samplesSuffix) + "/";
            if(f.startsWith(samplesPath))
            {
                return QString(f).mid(samplesPath.length());
            }
        }

        // Next, look in user samples
        QString usd = ConfigManager::inst()->userSamplesDir();
        usd.replace(QDir::separator(), '/');
        if(f.startsWith(usd))
        {
            return QString(f).mid(usd.length());
        }
    }
    return file;
}

QString SampleBuffer::tryToMakeAbsolute(const QString& file)
{
    QFileInfo f(file);

    if(f.isRelative())
    {
        f = QFileInfo(ConfigManager::inst()->userSamplesDir() + file);

        if(!f.exists())
        {
            f = QFileInfo(ConfigManager::inst()->factorySamplesDir() + file);
        }
    }

    if(f.exists())
    {
        return f.absoluteFilePath();
    }
    return file;
}

SampleBuffer::handleState::handleState(bool _varying_pitch,
                                       int  interpolation_mode) :
      m_frameIndex(0),
      m_varyingPitch(_varying_pitch), m_isBackwards(false)
{
    int error;
    m_interpolationMode = interpolation_mode;

    if((m_resamplingData
        = src_new(interpolation_mode, DEFAULT_CHANNELS, &error))
       == NULL)
    {
        qWarning("SampleBuffer::handleState src_new() failed");
    }
}

SampleBuffer::handleState::~handleState()
{
    src_delete(m_resamplingData);
}
