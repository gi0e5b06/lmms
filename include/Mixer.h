/*
 * Mixer.h - audio-device-independent mixer for LMMS
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef MIXER_H
#define MIXER_H

//#include "LocklessList.h"
#include "AudioPort.h"
#include "Configuration.h"
#include "MemoryManager.h"
#include "MixerProfiler.h"
#include "Note.h"
#include "NotePlayHandle.h"
#include "Ring.h"
#include "fifo_buffer.h"
#include "lmms_basics.h"

//#include <QMutex>
#include <QThread>
#include <QVector>
#include <QWaitCondition>

#include <samplerate.h>

class AudioDevice;
class MidiClient;
// class AudioPort;
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

const int32_t FREQUENCIES[15]
        = {8000,  11025, 16000, 22050,  24000,  32000,  44100, 48000,
           64000, 88200, 96000, 176400, 192000, 352800, 384000};

const real_t  BaseFreq   = 440.;
const Keys    BaseKey    = Key_A;
const Octaves BaseOctave = DefaultOctave;

#include "PlayHandle.h"

class Mixer;
class MixerWorkerThread;

typedef fifoBuffer<surroundSampleFrame*> fifo;

class HandleManager : public QThread
{
    Q_OBJECT

  public:
    HandleManager(Mixer* _mixer);
    virtual ~HandleManager();

  public slots:
    void addAudioPortHM(AudioPortPointer port);
    void removeAudioPortHM(AudioPortPointer port);

    void addPlayHandleHM(PlayHandlePointer handle);
    void removePlayHandleHM(PlayHandlePointer handle);

    void removePlayHandlesOfTypesHM(const Track* _track, const quint8 _types);
    void removePlayHandlesForInstrumentHM(const Instrument* _instrument);
    void removeAllPlayHandlesHM();

  private:
    Mixer* m_mixer;
};

class FifoWriter : public QThread
{
    Q_OBJECT

  public:
    FifoWriter(Mixer* _mixer, fifo* _fifo);

    void finish();

  private:
    Mixer*        m_mixer;
    fifo*         m_fifo;
    volatile bool m_writing;

    virtual void run();

    void write(surroundSampleFrame* buffer);
};

class EXPORT Mixer : public QObject
{
    Q_OBJECT
    MM_OPERATORS

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
    void clearAllPlayHandles();

    // audio-device-stuff

    // Returns the current audio device's name. This is not necessarily
    // the user's preferred audio device, in case you were thinking that.
    INLINE const QString& audioDevName() const
    {
        return m_audioDevName;
    }
    INLINE bool audioDevStartFailed() const
    {
        return m_audioDevStartFailed;
    }

    void setAudioDevice(AudioDevice* _dev);
    void setAudioDevice(AudioDevice*                  _dev,
                        const struct qualitySettings& _qs,
                        bool                          _needsFifo);
    void storeAudioDevice();
    void restoreAudioDevice();

    INLINE AudioDevice* audioDev()
    {
        return m_audioDev;
    }

    // MIDI-client-stuff
    INLINE const QString& midiClientName() const
    {
        return m_midiClientName;
    }

    INLINE MidiClient* midiClient()
    {
        return m_midiClient;
    }

    /*
    INLINE PlayHandleList& playHandles()
    {
        return m_playHandles;
    }
    */

    ConstNotePlayHandles nphsOfTrack(const Track* _track, bool _all = false);

    void waitUntilNoPlayHandle(const Track* _track, const quint8 _types);
    void waitUntilNoPlayHandle(const Instrument* _instrument);

    void adjustTempo(const bpm_t _tempo);

    // methods providing information for other classes
    INLINE fpp_t framesPerPeriod() const
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

    INLINE real_t masterVolumeGain() const
    {
        return m_masterVolumeGain;
    }

    INLINE void setMasterVolumeGain(const real_t _gain)
    {
        m_masterVolumeGain = _gain;
    }

    INLINE real_t masterPanningGain() const
    {
        return m_masterPanningGain;
    }

    INLINE void setMasterPanningGain(const real_t _gain)
    {
        m_masterPanningGain = _gain;
    }

    void applyMasterAdjustments(surroundSampleFrame* _ab, fpp_t frames);

    static INLINE sample_t clip(const sample_t _s)
    {
        if(_s > 1.)
            return 1.;
        else if(_s < -1.)
            return -1.;
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

    INLINE bool hasFifoWriter() const
    {
        return m_fifoWriter != nullptr;
    }

    void pushInputFrames(sampleFrame* _ab, const f_cnt_t _frames);

    INLINE const sampleFrame* inputBuffer()
    {
        return m_inputBuffer[m_inputBufferRead];
    }

    INLINE f_cnt_t inputBufferFrames() const
    {
        return m_inputBufferFrames[m_inputBufferRead];
    }

    const surroundSampleFrame* nextBuffer();

    void changeQuality(const struct qualitySettings& _qs);

    INLINE bool isMetronomeActive() const
    {
        return m_metronomeActive;
    }

    INLINE void setMetronomeActive(bool value = true)
    {
        m_metronomeActive = value;
    }

    void requestChangeInModel();
    void doneChangeInModel();

    Ring* displayRing()
    {
        return m_displayRing;
    }

    bool isHM()
    {
        return QThread::currentThread() == m_handleManager;
    }

  signals:
    void qualitySettingsChanged();
    void sampleRateChanged();
    // void nextDisplayBuffer( const surroundSampleFrame * buffer );

    void audioPortToAdd(AudioPortPointer port);
    void audioPortToRemove(AudioPortPointer port);

    void playHandleToAdd(PlayHandlePointer handle);
    void playHandleToRemove(PlayHandlePointer handle);
    void playHandleDeleted(PlayHandlePointer handle);
    void playHandlesOfTypesToRemove(const Track* _track, const quint8 _types);
    void playHandlesForInstrumentToRemove(const Instrument* _instrument);
    void allPlayHandlesRemoval();

  private:
    Mixer(bool renderOnly);
    virtual ~Mixer();

    void startProcessing(bool _needsFifo = true);
    void stopProcessing();

    AudioDevice* tryAudioDevices();
    MidiClient*  tryMidiClients();

    const surroundSampleFrame* renderNextBuffer();

    void addAudioPort1(AudioPortPointer port);
    void removeAudioPort1(AudioPortPointer port);

    void addPlayHandle1(PlayHandlePointer handle);
    void removePlayHandle1(PlayHandlePointer handle);
    void deletePlayHandle1(PlayHandlePointer _ph);
    void removePlayHandlesOfTypes1(const Track* _track, const quint8 types);
    void removePlayHandlesForInstrument1(const Instrument* _instrument);
    void removeAllPlayHandles1();

    void runChangesInModel();

    bool       m_renderOnly;
    AudioPorts m_audioPorts;
    fpp_t      m_framesPerPeriod;
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
    FifoWriter* m_fifoWriter;

    HandleManager* m_handleManager;
    MixerProfiler  m_profiler;
    bool           m_metronomeActive;
    bool           m_clearSignal;

    bool           m_changesSignal;
    unsigned int   m_changes;
    Mutex          m_changesMutex;
    Mutex          m_doChangesMutex;
    Mutex          m_waitChangesMutex;
    QWaitCondition m_changesMixerCondition;
    QWaitCondition m_changesRequestCondition;

    bool m_waitingForWrite;

    friend class LmmsCore;
    friend class MixerWorkerThread;
    friend class ProjectRenderer;
    friend class HandleManager;
    friend class FifoWriter;
};

#endif
