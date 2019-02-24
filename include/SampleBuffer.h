/*
 * SampleBuffer.h - container-class SampleBuffer
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
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

#ifndef SAMPLE_BUFFER_H
#define SAMPLE_BUFFER_H

#ifdef LMMS_HAVE_MPG123
#include <mpg123.h>
#endif

#include "export.h"
#include "interpolation.h"
#include "lmms_basics.h"
//#include "lmms_math.h"
#include "MemoryManager.h"
#include "shared_object.h"

#include <QMutex>
#include <QObject>
#include <QReadWriteLock>

//#include <samplerate.h>

class QPainter;
class QRect;

class EXPORT SampleBuffer : public QObject, public sharedObject
{
    Q_OBJECT
    MM_OPERATORS

  public:
    enum LoopMode
    {
        LoopOff,
        LoopOn,
        LoopPingPong
    };

    class EXPORT HandleState
    {
        MM_OPERATORS

      public:
        HandleState(f_cnt_t _startIndex   = -1,
                    bool    _varyingPitch = false,
                    int     _quality      = 10,
                    bool    _isBackwards  = false);
        virtual ~HandleState();

        const f_cnt_t frameIndex() const
        {
            return m_frameIndex;
        }

        void setFrameIndex(f_cnt_t _index)
        {
            m_frameIndex = _index;
        }

        bool isBackwards() const
        {
            return m_isBackwards;
        }

        void setBackwards(bool _backwards)
        {
            m_isBackwards = _backwards;
        }

        int quality() const
        {
            return m_quality;
        }

      private:
        f_cnt_t    m_frameIndex;
        const bool m_varyingPitch;
        int        m_quality;
        bool       m_isBackwards;
        // SRC_STATE* m_resamplingData;

        friend class SampleBuffer;
    };

    // constructor which either loads sample _audioFile or decodes
    // base64-data out of string
    SampleBuffer(const SampleBuffer& _other);
    SampleBuffer(const QString& _audioFile,
                 bool           _isBase64Data        = false,
                 bool           _sampleRateDependent = true);
    SampleBuffer(const sampleFrame* _data,
                 const f_cnt_t      _frames,
                 bool               _sampleRateDependent = true);
    SampleBuffer(const f_cnt_t _frames = 0, bool _sampleRateDependent = true);

    virtual ~SampleBuffer();

    bool play(sampleFrame*      _ab,
              HandleState*      _state,
              const fpp_t       _frames,
              const frequency_t _freq,
              const LoopMode    _loopmode        = LoopOff,
              const bool        _released        = false,
              const LoopMode    _releaseLoopmode = LoopOff);

    void visualize(QPainter& _p, const QRect& _dr);
    void visualize(QPainter&    _p,
                   const QRect& _dr,
                   const QRect& _clip,
                   f_cnt_t      _from_frame,
                   f_cnt_t      _to_frame);

    inline const QString& audioFile() const
    {
        return m_audioFile;
    }

    inline f_cnt_t startFrame() const
    {
        return m_startFrame;
    }

    inline f_cnt_t endFrame() const
    {
        return m_endFrame;
    }

    inline f_cnt_t loopStartFrame() const
    {
        return m_loopStartFrame;
    }

    inline f_cnt_t loopEndFrame() const
    {
        return m_loopEndFrame;
    }

    void setLoopStartFrame(f_cnt_t _start);
    void setLoopEndFrame(f_cnt_t _end);
    void setAllPointFrames(f_cnt_t _start,
                           f_cnt_t _end,
                           f_cnt_t _loopstart,
                           f_cnt_t _loopend);

    inline f_cnt_t frames() const
    {
        return m_frames;
    }

    inline real_t amplification() const
    {
        return m_amplification;
    }

    inline bool reversed() const
    {
        return m_reversed;
    }

    inline frequency_t frequency() const
    {
        return m_frequency;
    }

    sample_rate_t sampleRate() const
    {
        return m_sampleRate;
    }

    int sampleLength() const
    {
        return DOUBLE(m_endFrame - m_startFrame) / m_sampleRate * 1000.;
    }

    inline void setFrequency(frequency_t _freq)
    {
        m_frequency = _freq;
    }

    inline void setSampleRate(sample_rate_t _rate)
    {
        m_sampleRate = _rate;
    }

    inline const sampleFrame* data() const
    {
        return m_data;
    }

    f_cnt_t findClosestZero(f_cnt_t _index);

    QString openAudioFile() const;
    QString openAndSetAudioFile();
    QString openAndSetWaveformFile();

    QString& toBase64(QString& _dst) const;

    // protect calls from the GUI to this function with dataReadLock() and
    // dataUnlock()
    // SampleBuffer *
    void resample(const sample_rate_t _src_sr, const sample_rate_t _dst_sr);
    void retune(const DOUBLE _semitones);
    void stretch(const DOUBLE _factor);
    void predelay(const real_t _t);
    void postdelay(const real_t _t);

    void normalizeSampleRate(const sample_rate_t _src_sr,
                             bool                _keep_settings = false);

    // protect calls from the GUI to this function with dataReadLock() and
    // dataUnlock(), out of loops for efficiency
    sample_t userWaveSample(const real_t _sample) const;

    void dataReadLock()
    {
        m_varLock.lockForRead();
    }

    void dataUnlock()
    {
        m_varLock.unlock();
    }

    static const QString rawStereoSuffix();    // typically f2r44100
    static const QString rawSurroundSuffix();  // not used, f4r48000
    static QString selectAudioFile(const QString& _file = QString::null);
    static QString tryToMakeRelative(const QString& _file);
    static QString tryToMakeAbsolute(const QString& _file);
    static void    clearMMap();

  public slots:
    void setAudioFile(const QString& _audioFile);
    void loadFromBase64(const QString& _data);
    void setStartFrame(f_cnt_t _f);
    void setEndFrame(f_cnt_t _f);
    void setAmplification(real_t _a);
    void setReversed(bool _on);
    void setStretching(real_t _v);
    void setPredelay(real_t _t);
    void setPostdelay(real_t _t);
    void sampleRateChanged();

  signals:
    void sampleUpdated();

  private:
    f_cnt_t nextFrame(const f_cnt_t  _currentFrame,
                      const LoopMode _loopMode,
                      bool&          isBackwards_);

    real_t nextStretchedFrame(const real_t   _currentFrame,
                              const LoopMode _loopMode,
                              bool&          isBackwards_,
                              const real_t   _step);

    void update(bool _keep_settings = false);
    void prefetch(f_cnt_t _index);

    void getDataFrame(f_cnt_t _f, sample_t& ch0_, sample_t& ch1_);
    void setDataFrame(f_cnt_t _f, sample_t _ch0, sample_t _ch1);
    void writeCacheData(QString _fileName) const;

    void convertFromS16(sampleS16_t*& _ibuf, f_cnt_t _frames, int _channels);
    void directFloatWrite(sample_t*& _fbuf, f_cnt_t _frames, int _channels);

    f_cnt_t decodeSampleSF(const char*    _f,
                           sample_t*&     _buf,
                           ch_cnt_t&      _channels,
                           sample_rate_t& _sample_rate);
#ifdef LMMS_HAVE_MPG123
    void    cleanupMPG123(void* mh);
    f_cnt_t decodeSampleMPG123(const char*    _f,
                               sample_t*&     _buf,
                               ch_cnt_t&      _channels,
                               sample_rate_t& _samplerate);
#endif

#ifdef LMMS_HAVE_OGGVORBIS
    f_cnt_t decodeSampleOGGVorbis(const char*    _f,
                                  sampleS16_t*&  _buf,
                                  ch_cnt_t&      _channels,
                                  sample_rate_t& _sample_rate);
#endif
    f_cnt_t decodeSampleDS(const char*    _f,
                           sampleS16_t*&  _buf,
                           ch_cnt_t&      _channels,
                           sample_rate_t& _sample_rate);

    sampleFrame* getSampleFragment(f_cnt_t       _index,
                                   f_cnt_t       _frames,
                                   LoopMode      _loopmode,
                                   sampleFrame** _tmp,
                                   bool*         _backwards,
                                   f_cnt_t       _loopstart,
                                   f_cnt_t       _loopend,
                                   f_cnt_t       _end) const;
    f_cnt_t      getLoopedIndex(f_cnt_t _index,
                                f_cnt_t _startf,
                                f_cnt_t _endf) const;
    f_cnt_t      getPingPongIndex(f_cnt_t _index,
                                  f_cnt_t _startf,
                                  f_cnt_t _endf) const;

    QString        m_audioFile;
    sampleFrame*   m_origData;
    f_cnt_t        m_origFrames;
    bool           m_mmapped;
    sampleFrame*   m_data;
    f_cnt_t        m_frames;
    f_cnt_t        m_startFrame;
    f_cnt_t        m_endFrame;
    f_cnt_t        m_loopStartFrame;
    f_cnt_t        m_loopEndFrame;
    real_t         m_stretching;
    real_t         m_predelay;   // ms
    real_t         m_postdelay;  // ms
    real_t         m_amplification;
    bool           m_reversed;
    frequency_t    m_frequency;
    sample_rate_t  m_sampleRate;
    QReadWriteLock m_varLock;

    friend class AudioPort;
    friend class FxChannel;
};

#endif
