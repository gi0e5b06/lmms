/*
 * Mixer.h - audio-device-independent mixer for LMMS
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

#ifndef MIXER_H
#define MIXER_H

#include "lmms_basics.h"

#include <QMutex>
#include <QThread>
#include <QVector>
#include <QWaitCondition>

#include <samplerate.h>
//#include "LocklessList.h"
#include "Configuration.h"
#include "MemoryManager.h"
#include "MixerProfiler.h"
#include "Note.h"
#include "NotePlayHandle.h"
#include "Ring.h"
#include "fifo_buffer.h"

class AudioDevice;
class MidiClient;
class AudioPort;
class NotePlayHandle;

const fpp_t MINIMUM_BUFFER_SIZE = 32;
const fpp_t DEFAULT_BUFFER_SIZE = 256;

const int BYTES_PER_SAMPLE         = sizeof(sample_t);
const int BYTES_PER_S16_SAMPLE     = sizeof(sampleS16_t);
const int BYTES_PER_FRAME          = sizeof(sampleFrame);
const int BYTES_PER_SURROUND_FRAME = sizeof(surroundSampleFrame);

// const real_t S16_MULTIPLIER = powf(2,15)-1.;
// const real_t S32_MULTIPLIER = powf(2,31)-1.;
// const real_t S64_MULTIPLIER = powf(2,63)-1.;

const int32_t FREQUENCIES[12] = {11025, 16000, 22050, 24000, 32000,  44100,
                                 48000, 64000, 88200, 96000, 176400, 192000};

const real_t  BaseFreq   = 440.;
const Keys    BaseKey    = Key_A;
const Octaves BaseOctave = DefaultOctave;

#include "PlayHandle.h"

class MixerWorkerThread;

class EXPORT Mixer : public QObject
{
    MM_OPERATORS
    Q_OBJECT
  public:
    struct qualitySettings
    {
        enum Mode
        {
            Mode_Draft,
            Mode_HighQuality,
            Mode_FinalMix
        };

        enum Interpolation
        {
            Interpolation_Linear,
            Interpolation_SincFastest,
            Interpolation_SincMedium,
            Interpolation_SincBest
        };

        enum Oversampling
        {
            Oversampling_None,
            Oversampling_2x,
            Oversampling_4x,
            Oversampling_8x
        };

        Interpolation interpolation;
        Oversampling  oversampling;

        qualitySettings(Mode _m)
        {
            switch(_m)
            {
                case Mode_Draft:
                    interpolation = Interpolation_Linear;
                    oversampling  = Oversampling_None;
                    break;
                case Mode_HighQuality:
                    interpolation = Interpolation_SincFastest;
                    oversampling  = Oversampling_2x;
                    break;
                case Mode_FinalMix:
                    interpolation = Interpolation_SincBest;
                    oversampling  = Oversampling_8x;
                    break;
            }
        }

        qualitySettings(Interpolation _i, Oversampling _o) :
              interpolation(_i), oversampling(_o)
        {
        }

        int sampleRateMultiplier() const
        {
            switch(oversampling)
            {
                case Oversampling_None:
                    return 1;
                case Oversampling_2x:
                    return 2;
                case Oversampling_4x:
                    return 4;
                case Oversampling_8x:
                    return 8;
            }
            return 1;
        }

        int libsrcInterpolation() const
        {
            switch(interpolation)
            {
                case Interpolation_Linear:
                    return SRC_ZERO_ORDER_HOLD;
                case Interpolation_SincFastest:
                    return SRC_SINC_FASTEST;
                case Interpolation_SincMedium:
                    return SRC_SINC_MEDIUM_QUALITY;
                case Interpolation_SincBest:
                    return SRC_SINC_BEST_QUALITY;
            }
            return SRC_LINEAR;
        }
    };

    void initDevices();
    void clear();
    void clearPlayHandlesToAdd();

    // audio-device-stuff

    // Returns the current audio device's name. This is not necessarily
    // the user's preferred audio device, in case you were thinking that.
    inline const QString& audioDevName() const
    {
        return m_audioDevName;
    }
    inline bool audioDevStartFailed() const
    {
        return m_audioDevStartFailed;
    }

    void setAudioDevice(AudioDevice* _dev);
    void setAudioDevice(AudioDevice*                  _dev,
                        const struct qualitySettings& _qs,
                        bool                          _needs_fifo);
    void storeAudioDevice();
    void restoreAudioDevice();

    inline AudioDevice* audioDev()
    {
        return m_audioDev;
    }

    // audio-port-stuff
    void addAudioPort(AudioPort* _port);
    void removeAudioPort(AudioPort* _port);

    // MIDI-client-stuff
    inline const QString& midiClientName() const
    {
        return m_midiClientName;
    }

    inline MidiClient* midiClient()
    {
        return m_midiClient;
    }

    /*
    inline PlayHandleList& playHandles()
    {
        return m_playHandles;
    }
    */

    void removePlayHandlesOfTypes(const Track* _track, const quint8 types);

    ConstNotePlayHandleList  // QList<const NotePlayHandle*>
            nphsOfTrack(const Track* _track, bool _all = false);

    void adjustTempo(const bpm_t _tempo);

    // methods providing information for other classes
    inline fpp_t framesPerPeriod() const
    {
        return m_framesPerPeriod;
    }

    MixerProfiler& profiler()
    {
        return m_profiler;
    }

    int cpuLoad() const
    {
        return m_profiler.cpuLoad();
    }

    const qualitySettings& currentQualitySettings() const
    {
        return m_qualitySettings;
    }

    sample_rate_t baseSampleRate() const;
    sample_rate_t outputSampleRate() const;
    sample_rate_t inputSampleRate() const;
    sample_rate_t processingSampleRate() const;

    inline real_t masterVolumeGain() const
    {
        return m_masterVolumeGain;
    }

    inline void setMasterVolumeGain(const real_t _gain)
    {
        m_masterVolumeGain = _gain;
    }

    inline real_t masterPanningGain() const
    {
        return m_masterPanningGain;
    }

    inline void setMasterPanningGain(const real_t _gain)
    {
        m_masterPanningGain = _gain;
    }

    void applyMasterAdjustments(surroundSampleFrame* _ab, fpp_t frames);

    static inline sample_t clip(const sample_t _s)
    {
        if(_s > 1.)
        {
            return 1.;
        }
        else if(_s < -1.)
        {
            return -1.;
        }
        return _s;
    }

    void getPeakValues(const sampleFrame* _ab,
                       const f_cnt_t      _frames,
                       real_t&            peakLeft,
                       real_t&            peakRight) const;

#ifndef LMMS_DISABLE_SURROUND
    void getPeakValues(const surroundSampleFrame* _ab,
                       const f_cnt_t              _frames,
                       real_t&                    peakLeft,
                       real_t&                    peakRight) const;
#endif

    bool warningXRuns() const;
    bool criticalXRuns() const;

    inline bool hasFifoWriter() const
    {
        return m_fifoWriter != NULL;
    }

    void pushInputFrames(sampleFrame* _ab, const f_cnt_t _frames);

    inline const sampleFrame* inputBuffer()
    {
        return m_inputBuffer[m_inputBufferRead];
    }

    inline f_cnt_t inputBufferFrames() const
    {
        return m_inputBufferFrames[m_inputBufferRead];
    }

    inline const surroundSampleFrame* nextBuffer()
    {
        return hasFifoWriter() ? m_fifo->read() : renderNextBuffer();
    }

    void changeQuality(const struct qualitySettings& _qs);

    inline bool isMetronomeActive() const
    {
        return m_metronomeActive;
    }
    inline void setMetronomeActive(bool value = true)
    {
        m_metronomeActive = value;
    }

    void requestChangeInModel();
    void doneChangeInModel();

    Ring* displayRing()
    {
        return m_displayRing;
    }

  public slots:
    // play-handle stuff
    bool addPlayHandle(PlayHandle* handle);
    void removePlayHandle(PlayHandle* handle);

  signals:
    void qualitySettingsChanged();
    void sampleRateChanged();
    // void nextDisplayBuffer( const surroundSampleFrame * buffer );
    void playHandleDeleted(PlayHandle* handle);

  private:
    typedef fifoBuffer<surroundSampleFrame*> fifo;

    class fifoWriter : public QThread
    {
      public:
        fifoWriter(Mixer* _mixer, fifo* _fifo);

        void finish();

      private:
        Mixer*        m_mixer;
        fifo*         m_fifo;
        volatile bool m_writing;

        virtual void run();

        void write(surroundSampleFrame* buffer);
    };

    Mixer(bool renderOnly);
    virtual ~Mixer();

    void startProcessing(bool _needs_fifo = true);
    void stopProcessing();

    AudioDevice* tryAudioDevices();
    MidiClient*  tryMidiClients();

    const surroundSampleFrame* renderNextBuffer();

    // void clearInternal();
    bool addPlayHandleInternal(PlayHandle* _ph);
    void removePlayHandleInternal(PlayHandle* _ph);
    void removePlayHandleUnchecked(PlayHandle* _ph);
    void deletePlayHandleInternal(PlayHandle* _ph);
    void runChangesInModel();

    bool m_renderOnly;

    QVector<AudioPort*> m_audioPorts;

    fpp_t m_framesPerPeriod;
    // long  m_periodCounter;

    sampleFrame* m_inputBuffer[2];
    f_cnt_t      m_inputBufferFrames[2];
    f_cnt_t      m_inputBufferSize[2];
    int          m_inputBufferRead;
    int          m_inputBufferWrite;

    surroundSampleFrame* m_readBuf;
    surroundSampleFrame* m_writeBuf;

    // int m_periodsPerDisplayRefresh;
    Ring* m_displayRing;

    QVector<surroundSampleFrame*> m_bufferPool;
    int                           m_readBuffer;
    int                           m_writeBuffer;
    int                           m_poolDepth;

    // worker thread stuff
    QVector<MixerWorkerThread*> m_workers;
    int                         m_numWorkers;

    // playhandle stuff
    PlayHandleList m_playHandles;
    PlayHandleList m_playHandlesToAdd;
    PlayHandleList m_playHandlesToRemove; /*Const*/
    // place where new playhandles are added temporarily
    // LocklessList<PlayHandle *> m_newPlayHandles;

    struct qualitySettings m_qualitySettings;
    real_t                 m_masterVolumeGain;
    real_t                 m_masterPanningGain;

    bool m_isProcessing;

    // audio device stuff
    AudioDevice* m_audioDev;
    AudioDevice* m_oldAudioDev;
    QString      m_audioDevName;
    bool         m_audioDevStartFailed;

    // MIDI device stuff
    MidiClient* m_midiClient;
    QString     m_midiClientName;

    // FIFO stuff
    fifo*       m_fifo;
    fifoWriter* m_fifoWriter;

    MixerProfiler m_profiler;

    bool m_metronomeActive;

    bool m_clearSignal;

    bool           m_changesSignal;
    unsigned int   m_changes;
    QMutex         m_changesMutex;
    QMutex         m_doChangesMutex;
    QMutex         m_waitChangesMutex;
    QWaitCondition m_changesMixerCondition;
    QWaitCondition m_changesRequestCondition;

    bool m_waitingForWrite;

    friend class LmmsCore;
    friend class MixerWorkerThread;
    friend class ProjectRenderer;
};

#endif
