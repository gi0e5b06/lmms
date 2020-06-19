/*
 * Mixer.cpp - audio-device-independent mixer for LMMS
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

#include "Mixer.h"

#include "AudioPort.h"
#include "EnvelopeAndLfoParameters.h"
#include "FxMixer.h"
#include "MixerWorkerThread.h"
#include "Song.h"
#include "lmmsconfig.h"
//#include "ConfigManager.h"
#include "Configuration.h"
#include "SafeHash.h"
#include "SamplePlayHandle.h"

// platform-specific audio-interface-classes
#include "AudioAlsa.h"
#include "AudioAlsaGdx.h"
#include "AudioDummy.h"
#include "AudioJack.h"
#include "AudioOss.h"
#include "AudioPortAudio.h"
#include "AudioPulseAudio.h"
#include "AudioSdl.h"
#include "AudioSndio.h"
#include "AudioSoundIo.h"

// platform-specific midi-interface-classes
#include "Backtrace.h"
#include "BufferManager.h"
#include "MemoryHelper.h"
#include "MidiAlsaGdx.h"
#include "MidiAlsaRaw.h"
#include "MidiAlsaSeq.h"
#include "MidiApple.h"
#include "MidiDummy.h"
#include "MidiJack.h"
#include "MidiOss.h"
#include "MidiSndio.h"
#include "MidiWinMM.h"
#include "denormals.h"

#include <QCoreApplication>

#include <typeinfo>

/*
#define DEBUG_FIFO_CHECK                                            \
    if(QThread::currentThread() != m_fifoWriter)                      \
        qWarning("Warning: %s#%d: wrong thread (NOT FIFO)", __FILE__, \
                 __LINE__);
*/

static __thread bool s_renderingThread;

Mixer::Mixer(bool renderOnly) :
      m_renderOnly(renderOnly), m_audioPorts(true),
      m_framesPerPeriod(DEFAULT_BUFFER_SIZE),
      // m_periodCounter(0),
      m_inputBufferRead(0), m_inputBufferWrite(1), m_readBuf(nullptr),
      m_writeBuf(nullptr), m_displayRing(nullptr), m_workers(),
      m_numWorkers(QThread::idealThreadCount() * 2 - 1),  // tmp GDX
      m_playHandles(true), m_playHandlesToAdd(true),
      m_playHandlesToRemove(true),
      m_qualitySettings(qualitySettings::Mode_Draft), m_masterVolumeGain(1.),
      m_masterPanningGain(0.), m_isProcessing(false), m_audioDev(nullptr),
      m_oldAudioDev(nullptr), m_audioDevStartFailed(false),
      m_handleManager(nullptr), m_profiler(), m_metronomeActive(false),
      m_clearSignal(false), m_changesSignal(false), m_changes(0),
      m_doChangesMutex(QMutex::Recursive), m_waitingForWrite(false)
{
    for(int i = 0; i < 2; ++i)
    {
        m_inputBufferFrames[i] = 0;
        m_inputBufferSize[i]   = DEFAULT_BUFFER_SIZE * 100;
        m_inputBuffer[i]       = new sampleFrame[m_inputBufferSize[i]];
        memset(m_inputBuffer[i], 0,
               sizeof(sampleFrame) * m_inputBufferSize[i]);
        // BufferManager::clear( m_inputBuffer[i], m_inputBufferSize[i] );
    }

    // determine FIFO size and number of frames per period
    int fifoSize = 1;

    // if not only rendering (that is, using the GUI), load the buffer
    // size from user configuration
    if(renderOnly == false)
    {
        m_framesPerPeriod
                = (fpp_t)CONFIG_GET_INT("mixer.framesperaudiobuffer");
        //	( fpp_t ) ConfigManager::inst()->
        //		value( "mixer", "framesperaudiobuffer" ).toInt();

        // if the value read from user configuration is not set or
        // lower than the minimum allowed, use the default value and
        // save it to the configuration
        if(m_framesPerPeriod < MINIMUM_BUFFER_SIZE)
        {
            CONFIG_SET_INT("mixer.framesperaudiobuffer", DEFAULT_BUFFER_SIZE);
            // ConfigManager::inst()->setValue( "mixer",
            //			"framesperaudiobuffer",
            //			QString::number( DEFAULT_BUFFER_SIZE ) );
            m_framesPerPeriod = DEFAULT_BUFFER_SIZE;
        }
        else if(m_framesPerPeriod > DEFAULT_BUFFER_SIZE)
        {
            fifoSize          = m_framesPerPeriod / DEFAULT_BUFFER_SIZE;
            m_framesPerPeriod = DEFAULT_BUFFER_SIZE;
        }
    }

    // allocte the FIFO from the determined size
    m_fifo = new fifo(fifoSize);

    // now that framesPerPeriod is fixed initialize global BufferManager
    BufferManager::init(m_framesPerPeriod);

    for(int i = 0; i < 4; i++)  // 3
    {
        // m_readBuf = (surroundSampleFrame*)
        // MemoryHelper::alignedMalloc( m_framesPerPeriod * sizeof(
        // surroundSampleFrame ) );
        m_readBuf = MM_ALIGNED_ALLOC(surroundSampleFrame, m_framesPerPeriod);
        m_bufferPool.push_back(m_readBuf);
    }

    for(int i = 0; i < m_numWorkers + 1; ++i)
    {
        MixerWorkerThread* wt = new MixerWorkerThread(this);
        if(i < m_numWorkers)
        {
            wt->setObjectName(QString("mixerWorker%1").arg(i));
            wt->start(QThread::TimeCriticalPriority);
        }
        m_workers.push_back(wt);
    }

    m_poolDepth   = m_bufferPool.size() - 1;  // 2;
    m_readBuffer  = 0;
    m_writeBuffer = 1;
}

Mixer::~Mixer()
{
    runChangesInModel();

    qInfo("Mixer::~Mixer 1");
    // Engine::mixer()->clearAllPlayHandles();

    m_handleManager->quit();
    m_handleManager->wait(500);
    delete m_handleManager;
    m_handleManager = nullptr;

    qInfo("Mixer::~Mixer 2");

    for(int w = 0; w < m_numWorkers; ++w)
    {
        m_workers[w]->quit();
    }

    MixerWorkerThread::startAndWaitForJobs();

    for(int w = 0; w < m_numWorkers; ++w)
    {
        m_workers[w]->wait(500);
    }

    while(m_fifo->available())
    {
        delete[] m_fifo->read();
    }
    delete m_fifo;

    if(m_fifoWriter != nullptr)
    {
        m_fifoWriter->quit();
        m_fifoWriter->wait(500);
        m_fifoWriter = nullptr;
    }

    qInfo("Mixer::~Mixer 3");

    delete m_audioDev;

    qInfo("Mixer::~Mixer 4");

    delete m_midiClient;

    qInfo("Mixer::~Mixer 5");

    for(int i = 0; i < 3; i++)
    {
        // MemoryHelper::alignedFree( m_bufferPool[i] );
        MM_ALIGNED_FREE(m_bufferPool[i]);
    }

    for(int i = 0; i < 2; ++i)
    {
        delete[] m_inputBuffer[i];
    }

    if(m_displayRing != nullptr)
        delete m_displayRing;

    qInfo("Mixer::~Mixer 6");
}

void Mixer::clearAllPlayHandles()
{
    if(QThread::currentThread() != thread())
        qWarning("Mixer::cleanHandles not in the main thread");

    qInfo("Mixer::clearAllPlayHandles 1");
    emit allPlayHandlesRemoval();

    bool again = true;
    while(again)
    {
        qInfo("Mixer::clearAllPlayHandles 2");
        QCoreApplication::sendPostedEvents();
        // qInfo("Mixer::clearAllPlayHandles 3");
        QThread::yieldCurrentThread();
        // qInfo("Mixer::clearAllPlayHandles 4");
        again = (m_playHandles.size() + m_playHandlesToAdd.size()
                         + m_playHandlesToRemove.size()
                 > 0);

        if(m_renderOnly)
            break;
    }
    qInfo("Mixer: all handles removed");
}

void Mixer::initDevices()
{
    bool success_ful = false;
    if(m_renderOnly)
    {
        m_audioDev       = new AudioDummy(success_ful, this);
        m_audioDevName   = AudioDummy::name();
        m_midiClient     = new MidiDummy;
        m_midiClientName = MidiDummy::name();
    }
    else
    {
        m_audioDev   = tryAudioDevices();
        m_midiClient = tryMidiClients();
    }

    if(m_displayRing == nullptr)
    {
        // m_displayBuf=MM_ALLOC(surroundSampleFrame, m_framesPerPeriod);
        // memset(m_displayBuf,0,sizeof(surroundSampleFrame)*m_framesPerPeriod);
        int periodsPerDisplayRefresh = qMax(
                1,
                int(ceil(real_t(processingSampleRate())
                         / real_t(m_framesPerPeriod
                                  / CONFIG_GET_INT("ui.framespersecond")))));
        m_displayRing
                = new Ring(m_framesPerPeriod * periodsPerDisplayRefresh);
    }
}

void Mixer::startProcessing(bool _needsFifo)
{
    if(m_handleManager == nullptr)
    {
        m_handleManager = new HandleManager(this);
        m_handleManager->start(QThread::HighPriority);
    }

    // bool _needsFifo = true;

    if(_needsFifo)
    {
        m_fifoWriter = new FifoWriter(this, m_fifo);
        m_fifoWriter->start(QThread::HighPriority);
    }
    else
    {
        m_fifoWriter = nullptr;
    }

    m_audioDev->startProcessing();

    m_isProcessing = true;
}

void Mixer::stopProcessing()
{
    m_isProcessing = false;

    if(m_fifoWriter != nullptr)
    {
        m_fifoWriter->finish();
        m_fifoWriter->wait();
        m_audioDev->stopProcessing();
        delete m_fifoWriter;
        m_fifoWriter = nullptr;
    }
    else
    {
        m_audioDev->stopProcessing();
    }
}

sample_rate_t Mixer::baseSampleRate() const
{
    return CONFIG_GET_INT("mixer.samplerate");
    /*
    ConfigManager* cm=ConfigManager::inst();

    sample_rate_t sr = cm!=nullptr
            ? qMax(44100,cm->value( "mixer", "samplerate" ).toInt())
            : 44100;

    // temporary reminder
    if(cm==nullptr) qWarning("       : in Mixer::baseSampleRate()");

    return sr;
    */
}

sample_rate_t Mixer::outputSampleRate() const
{
    return m_audioDev != nullptr ? m_audioDev->sampleRate()
                                 : baseSampleRate();
}

sample_rate_t Mixer::inputSampleRate() const
{
    return m_audioDev != nullptr ? m_audioDev->sampleRate()
                                 : baseSampleRate();
}

sample_rate_t Mixer::processingSampleRate() const
{
    return outputSampleRate() * m_qualitySettings.sampleRateMultiplier();
}

bool Mixer::warningXRuns() const
{
    return cpuLoad() >= 95 && Engine::getSong()->isExporting() == false;
}

bool Mixer::criticalXRuns() const
{
    return cpuLoad() >= 98 && Engine::getSong()->isExporting() == false;
}

void Mixer::pushInputFrames(sampleFrame* _ab, const f_cnt_t _frames)
{
    requestChangeInModel();

    f_cnt_t      frames = m_inputBufferFrames[m_inputBufferWrite];
    int          size   = m_inputBufferSize[m_inputBufferWrite];
    sampleFrame* buf    = m_inputBuffer[m_inputBufferWrite];

    if(frames + _frames > size)
    {
        size            = qMax(size * 2, frames + _frames);
        sampleFrame* ab = new sampleFrame[size];
        memset(ab, 0, sizeof(sampleFrame) * size);
        memcpy(ab, buf, frames * sizeof(sampleFrame));
        delete[] buf;

        m_inputBufferSize[m_inputBufferWrite] = size;
        m_inputBuffer[m_inputBufferWrite]     = ab;

        buf = ab;
    }

    memcpy(&buf[frames], _ab, _frames * sizeof(sampleFrame));
    m_inputBufferFrames[m_inputBufferWrite] += _frames;

    doneChangeInModel();
}

SampleBuffer* s_metronome1 = nullptr;
SampleBuffer* s_metronome2 = nullptr;

const surroundSampleFrame* Mixer::renderNextBuffer()
{
    QThread::yieldCurrentThread();

    m_profiler.startPeriod();

    s_renderingThread = true;

    // static PlayPos last_metro_pos = -1;

    Song* song = Engine::getSong();

    Song::PlayModes currentPlayMode = song->playMode();
    PlayPos         p               = song->getPlayPos(currentPlayMode);

    bool playModeSupportsMetronome
            = currentPlayMode == Song::Mode_PlaySong
              || currentPlayMode == Song::Mode_PlayBB
              || currentPlayMode == Song::Mode_PlayPattern
              || currentPlayMode == Song::Mode_PlayAutomation;

    if(playModeSupportsMetronome && m_metronomeActive && !song->isExporting()
       &&
       // p != last_metro_pos &&
       // Stop crash with metronome if empty project
       Engine::getSong()->hasTracks())  // countTracks())
    {
        if(!s_metronome1)
            s_metronome1 = sharedObject::ref(
                    new SampleBuffer("misc/metronome01.ogg"));
        if(!s_metronome2)
            s_metronome2 = sharedObject::ref(
                    new SampleBuffer("misc/metronome02.ogg"));

        const tick_t ticksPerTact = MidiTime::ticksPerTact();
        const tick_t ticksPerBeat
                = ticksPerTact / song->getTimeSigModel().getNumerator();
        const real_t framesPerTick   = Engine::framesPerTick();
        const fpp_t  framesPerPeriod = Engine::mixer()->framesPerPeriod();

        const real_t fptact = framesPerTick * ticksPerTact;
        const real_t fpbeat = framesPerTick * ticksPerBeat;
        const real_t curfa  = p.absoluteFrame();
        for(int k = 0; k < framesPerPeriod; k++)
        {
            const real_t metf = curfa + k;
            if(fmod(metf, fptact) < 1.)
            {
                SamplePlayHandle* sph = new SamplePlayHandle(s_metronome2);
                sph->setOffset(k);
                // sph->setFrames(qMin(sph->frames(),framesPerBeat));
                addPlayHandle1(sph);
                // k+=framesPerTick-1;
            }
            else if(fmod(metf, fpbeat) < 1.)
            {
                SamplePlayHandle* sph = new SamplePlayHandle(s_metronome1);
                sph->setOffset(k);
                // sph->setFrames(qMin(sph->frames(),framesPerBeat));
                addPlayHandle1(sph);
                // k+=framesPerTick-1;
            }
            // else k++;
        }
        // last_metro_f=curf;
    }

    // swap buffer
    m_inputBufferWrite = (m_inputBufferWrite + 1) % 2;
    m_inputBufferRead  = (m_inputBufferRead + 1) % 2;

    // clear new write buffer
    m_inputBufferFrames[m_inputBufferWrite] = 0;

    /*
    if(m_clearSignal)
    {
        m_clearSignal = false;
        clearInternal();
    }
    */

    // remove all play-handles that have to be deleted and delete
    // them if they still exist...
    // maybe this algorithm could be optimized...
    /* GDX
    ConstPlayHandleList::Iterator it_rem = m_playHandlesToRemove.begin();
    while(it_rem != m_playHandlesToRemove.end())
    {
        PlayHandleList::Iterator it
                = qFind(m_playHandles.begin(), m_playHandles.end(), *it_rem);

        if(it != m_playHandles.end())
        {
            (*it)->audioPort()->removePlayHandle((*it));
            if((*it)->type() == PlayHandle::TypeNotePlayHandle)
            {
                // TMP
                NotePlayHandle* nph = dynamic_cast<NotePlayHandle*>(*it);
                if(nph == nullptr)
                {
                    BACKTRACE
                    qWarning("Mixer::renderNextBuffer nph==null");
                }
                else
                {
                    NotePlayHandleManager::release((NotePlayHandle*)*it);
                }
            }
            else
            {
                delete *it;
            }

            m_playHandles.erase(it);
        }

        it_rem = m_playHandlesToRemove.erase(it_rem);
    }
*/

    // rotate buffers
    m_writeBuffer = (m_writeBuffer + 1) % m_poolDepth;
    m_readBuffer  = (m_readBuffer + 1) % m_poolDepth;

    m_writeBuf = m_bufferPool[m_writeBuffer];
    m_readBuf  = m_bufferPool[m_readBuffer];

    // clear last audio-buffer
    memset(m_writeBuf, 0, m_framesPerPeriod * sizeof(surroundSampleFrame));

    // prepare master mix (clear internal buffers etc.)
    FxMixer* fxMixer = Engine::fxMixer();
    fxMixer->prepareMasterMix();

    // create play-handles for new notes, samples etc.
    song->processNextBuffer();

    requestChangeInModel();

    // add all play-handles that have to be added
    // while(!m_playHandlesToAdd.isEmpty())
    //   m_playHandles.append(m_playHandlesToAdd.takeLast());
    m_playHandlesToAdd.map(
            [this](PlayHandle* ph) { this->m_playHandles.appendUnique(ph); },
            true);

    m_playHandles.map([this](PlayHandle* ph) {
        bool removed = true;
        if(!this->m_clearSignal && !ph->isFinished())
            removed = false;
        if(this->m_clearSignal
           && (ph->type() == PlayHandle::TypeInstrumentPlayHandle
               || ph->type() == PlayHandle::TypePresetPreviewHandle))
            removed = ph->isFinished();  // false;
        if(removed)
            // this->emit playHandleToRemove(ph);
            this->m_playHandlesToRemove.appendUnique(ph);
    });
    // m_clearSignal = false;

    m_playHandlesToRemove.map(
            [this](PlayHandle* ph) {
                this->m_playHandles.removeAll(ph);
                ph->setFinished();
                // this->emit playHandleToRemove(ph);
            },
            true);

    // doneChangeInModel();

    // STAGE 1: run and render all play handles
    MixerWorkerThread::fillJobQueue<PlayHandle*>(m_playHandles);
    MixerWorkerThread::startAndWaitForJobs();

    // requestChangeInModel();

    // removed all play handles which are done
    m_playHandles.map([this](PlayHandle* ph) {
        bool removed = true;
        if(!this->m_clearSignal && !ph->isFinished())
            removed = false;
        if(this->m_clearSignal
           && (ph->type() == PlayHandle::TypeInstrumentPlayHandle
               || ph->type() == PlayHandle::TypePresetPreviewHandle))
            removed = ph->isFinished();
        if(removed)
            // this->emit playHandleToRemove(ph);
            this->m_playHandlesToRemove.appendUnique(ph);
    });
    m_clearSignal = false;
    m_playHandlesToRemove.map(
            [this](PlayHandle* ph) {
                this->m_playHandles.removeAll(ph);
                ph->setFinished();
                // this->emit playHandleToRemove(ph);
            },
            true);

    // doneChangeInModel();
    // requestChangeInModel();

    // STAGE 2: process effects of all instrument- and sampletracks
    MixerWorkerThread::fillJobQueue<AudioPort*>(m_audioPorts);
    MixerWorkerThread::startAndWaitForJobs();

    doneChangeInModel();

    // STAGE 3: do master mix in FX mixer
    fxMixer->masterMix(m_writeBuf);

    /*
    if(m_periodCounter%periodsPerDisplayRefresh==0)
    {
            memcpy(m_displayBuf,m_writeBuf,m_framesPerPeriod *
    sizeof(surroundSampleFrame)); emit
    nextSurroundDisplayBuffer(m_displayBuf);
    }
    */
    // emit nextAudioBuffer( m_readBuf );
    m_displayRing->write(m_readBuf, m_framesPerPeriod);

    runChangesInModel();

    // and trigger LFOs
    EnvelopeAndLfoParameters::instances()->trigger();
    Controller::triggerFrameCounter();
    AutomatableModel::incrementPeriodCounter();

    s_renderingThread = false;

    m_profiler.finishPeriod(processingSampleRate(), m_framesPerPeriod);

    /*
    qInfo("Mixer: running=%d toAdd=%d toRemove=%d",
          m_playHandles.size(),
          m_playHandlesToAdd.size(),
          m_playHandlesToRemove.size());
    */

    return m_readBuf;
}

void Mixer::clear()
{
    // qInfo("Mixer::clear");
    m_clearSignal = true;
}

void Mixer::clearPlayHandlesToAdd()
{
    requestChangeInModel();
    m_playHandlesToAdd.map(
            [this](PlayHandle* ph) {
                ph->setFinished();
                // this->emit playHandleToRemove(ph);
            },
            true);
    doneChangeInModel();
}

// removes all play-handles. this is necessary, when the song is stopped ->
// all remaining notes etc. would be played until their end
/*
void Mixer::clearInternal()
{
    // TODO: m_midiClient->noteOffAll();

    // qInfo("Mixer::clearInternal");
    // requestChangeInModel();
    for(PlayHandle* ph: m_playHandles)
        if(ph->type() != PlayHandle::TypeInstrumentPlayHandle)
            m_playHandlesToRemove.append(ph);
    // doneChangeInModel();

    **
for(PlayHandleList::Iterator it = m_playHandles.begin();
    it != m_playHandles.end(); ++it)
{
    // we must not delete instrument-play-handles as they exist
    // during the whole lifetime of an instrument
    if((*it)->type() != PlayHandle::TypeInstrumentPlayHandle)
    {
        m_playHandlesToRemove.push_back(*it);
    }
}
    **
}
*/

void Mixer::getPeakValues(const sampleFrame* _ab,
                          const f_cnt_t      _frames,
                          real_t&            peakLeft,
                          real_t&            peakRight) const
{
    peakLeft  = 0.;
    peakRight = 0.;
    for(f_cnt_t f = _frames - 1; f >= 0; --f)
    {
        const real_t absLeft  = qAbs(_ab[f][0]);
        const real_t absRight = qAbs(_ab[f][1]);
        if(absLeft > peakLeft)
            peakLeft = absLeft;
        if(absRight > peakRight)
            peakRight = absRight;
    }
}

#ifndef LMMS_DISABLE_SURROUND
void Mixer::getPeakValues(const surroundSampleFrame* _ab,
                          const f_cnt_t              _frames,
                          real_t&                    peakLeft,
                          real_t&                    peakRight) const
{
    peakLeft  = 0.;
    peakRight = 0.;
    for(f_cnt_t f = _frames - 1; f >= 0; --f)
    {
        const real_t absLeft  = qAbs(_ab[f][0]);
        const real_t absRight = qAbs(_ab[f][1]);
        if(absLeft > peakLeft)
            peakLeft = absLeft;
        if(absRight > peakRight)
            peakRight = absRight;
    }
}
#endif

const surroundSampleFrame* Mixer::nextBuffer()
{
    if(CONFIG_GET_BOOL("midi.mtc_enabled"))
    {
        MidiClient* m = midiClient();
        if(m != nullptr)
            m->sendMTC();
    }

    return hasFifoWriter() ? m_fifo->read() : renderNextBuffer();
}

void Mixer::changeQuality(const struct qualitySettings& _qs)
{
    // don't delete the audio-device
    stopProcessing();

    m_qualitySettings = _qs;
    m_audioDev->applyQualitySettings();

    emit sampleRateChanged();
    emit qualitySettingsChanged();

    startProcessing();
}

void Mixer::setAudioDevice(AudioDevice* _dev)
{
    stopProcessing();

    // qInfo("Mixer::setAudioDevice p=%p", _dev);

    if(_dev == nullptr)
    {
        qWarning(
                "Warning: param _dev == nullptr in "
                "Mixer::setAudioDevice(...). "
                "Trying any working audio-device");
        m_audioDev = tryAudioDevices();
    }
    else
    {
        m_audioDev = _dev;
    }

    emit sampleRateChanged();

    startProcessing();
}

void Mixer::setAudioDevice(AudioDevice*                  _dev,
                           const struct qualitySettings& _qs,
                           bool                          _needsFifo)
{
    // don't delete the audio-device
    stopProcessing();

    m_qualitySettings = _qs;

    // qInfo("Mixer::setAudioDevice p=%p", _dev);

    if(_dev == nullptr)
    {
        qWarning(
                "Warning: param _dev == nullptr in "
                "Mixer::setAudioDevice(...). "
                "Trying any working audio-device");
        m_audioDev = tryAudioDevices();
    }
    else
    {
        m_audioDev = _dev;
    }

    emit qualitySettingsChanged();
    emit sampleRateChanged();

    startProcessing(_needsFifo);
}

void Mixer::storeAudioDevice()
{
    if(!m_oldAudioDev)
    {
        m_oldAudioDev = m_audioDev;
    }
}

void Mixer::restoreAudioDevice()
{
    if(m_oldAudioDev != nullptr)
    {
        stopProcessing();
        delete m_audioDev;

        m_audioDev = m_oldAudioDev;
        emit sampleRateChanged();

        m_oldAudioDev = nullptr;
        startProcessing();
    }
}

void Mixer::addAudioPort(AudioPort* _port)
{
    // qInfo("Mixer::addAudioPort");
    requestChangeInModel();
    m_audioPorts.appendUnique(_port);
    doneChangeInModel();
}

void Mixer::removeAudioPort(AudioPort* _port)
{
    // qInfo("Mixer::removeAudioPort 1");
    requestChangeInModel();
    /*
    QVector<AudioPort*>::Iterator it
            = qFind(m_audioPorts.begin(), m_audioPorts.end(), _port);
    if(it != m_audioPorts.end())
    {
        m_audioPorts.erase(it);
    }
    */
    // qInfo("Mixer::removeAudioPort 2");
    m_audioPorts.removeAll(_port);
    // qInfo("Mixer::removeAudioPort 3");
    doneChangeInModel();
    // qInfo("Mixer::removeAudioPort 4");
}

void Mixer::addPlayHandle1(PlayHandle* _ph)
{
    if(_ph == nullptr)
    {
        BACKTRACE
        qWarning("Mixer::addPlayHandle(nullptr)");
        return;  // false;
    }

    if(QThread::currentThread() != m_handleManager)
    {
        // BACKTRACE
        qWarning("Warning: %s#%d: addPlayHandle wrong thread (NOT HM)",
                 __FILE__, __LINE__);
    }

    // requestChangeInModel();

    _ph->audioPort()->addPlayHandle(_ph);

    if(_ph->isFinished())
    {
        BACKTRACE
        qWarning("Mixer::addPlayHandle ph is finished");
        _ph->setFinished();
    }
    else if(criticalXRuns()
            && ((_ph->type() & PlayHandle::TypeNotePlayHandle)
                || (_ph->type() & PlayHandle::TypeSamplePlayHandle)))
    {
        // if(m_playHandles.contains(_ph))
        // m_playHandlesToRemove.appendUnique(_ph);
        _ph->setFinished();
        // emit playHandleToRemove(_ph);
    }
    else
    {
        if(m_playHandles.contains(_ph) || m_playHandlesToAdd.contains(_ph))
        {
            qWarning("Mixer::addPlayHandleInternal ph already in");
        }
        else
        {
            m_playHandlesToAdd.appendUnique(_ph);
        }
    }

    // doneChangeInModel();
}

void Mixer::removePlayHandle1(PlayHandle* _ph)
{
    if(_ph == nullptr)
    {
        BACKTRACE
        qWarning("Mixer::removePlayHandle(nullptr)");
        return;
    }

    if(!_ph->isFinished())
    {
        qWarning("Mixer::removePlayHandle ph not finished");
        // return;
    }

    // requestChangeInModel();

    if(_ph->type() == PlayHandle::TypeNotePlayHandle)
    {
        NotePlayHandle* nph = dynamic_cast<NotePlayHandle*>(_ph);
        nph->finishSubNotes();
    }

    deletePlayHandle1(_ph);

    // doneChangeInModel();
}

static SafeHash<QString, bool> s_deleteTracker;

void Mixer::deletePlayHandle1(PlayHandle* _ph)
{
    if(_ph == nullptr)
    {
        qWarning("Mixer::deletePlayHandleInternal(nullptr)");
        return;
    }

    if(QThread::currentThread() != m_handleManager)
    {
        BACKTRACE
        qWarning(
                "Warning: %s#%d: deletePlayHandleInternal wrong thread (NOT "
                "HM)",
                __FILE__, __LINE__);
    }

    // TMP GDX
    if(_ph->audioPort() == nullptr)
    {
        BACKTRACE
        qWarning("Mixer::removePlayHandleUnchecked null audio port");
    }

    int bx1 = m_playHandlesToAdd.removeAll(_ph, false);
    int bx2 = m_playHandles.removeAll(_ph, false);
    int bx3 = m_playHandlesToRemove.removeAll(_ph, false);
    if(bx1 > 0 && bx2 > 0)
    {
        BACKTRACE
        qWarning("Mixer::removePlayHandleUnchecked both add + reg");
    }
    if(bx3 > 0 && bx2 > 0)
    {
        BACKTRACE
        qWarning("Mixer::removePlayHandleUnchecked both rem + reg");
    }
    if(bx1 > 0 && bx3 > 0)
    {
        BACKTRACE
        qWarning("Mixer::removePlayHandleUnchecked both add + rem");
    }

    /*
    if(m_playHandles.contains(_ph))
        qWarning("Mixer::deletePlayHandleInternal _ph in current list");
    if(m_playHandlesToAdd.contains(_ph))
        qWarning("Mixer::deletePlayHandleInternal _ph in to-add list");
    if(m_playHandlesToRemove.contains(_ph))
        qWarning("Mixer::deletePlayHandleInternal _ph in to-remove list");
    */

    /*
    if(_ph->type() == PlayHandle::TypeNotePlayHandle)
    {
        NotePlayHandle* nph = dynamic_cast<NotePlayHandle*>(_ph);
        nph->requestSubNotesToRemove();
    }
    */

    if(_ph->type() == 2)
    {
        qInfo("iph->audioPort()->removePlayHandle(iph);");
        qInfo("audioPort=%p", _ph->audioPort());
    }

    _ph->audioPort()->removePlayHandle(_ph);
    if(_ph->type() == 2)
    {
        qInfo("... after iph->audioPort()->removePlayHandle(iph)");
    }

    // if(bx1 + bx2 + bx3 > 0)
    {
        if(_ph->type() == PlayHandle::TypeNotePlayHandle)
        {
            // TMP
            NotePlayHandle* nph = dynamic_cast<NotePlayHandle*>(_ph);
            if(nph == nullptr)
            {
                BACKTRACE
                qWarning("Mixer::deletePlayHandleInternal nph==null");
            }
            else
            {
                NotePlayHandleManager::release(nph);
                emit playHandleDeleted(_ph);
            }
        }
        else
        {
            if(s_deleteTracker.contains(_ph->m_debug_uuid))
            {
                BACKTRACE
                qCritical(
                        "Mixer::deletePlayHandleInternal %p was "
                        "already released (%s)",
                        _ph, typeid(_ph).name());
                return;
            }
            s_deleteTracker.insert(_ph->m_debug_uuid, true);

            if(_ph->type() == 2)
                qInfo("   deleting IPH");
            delete _ph;
            emit playHandleDeleted(_ph);
        }
    }
    // else qWarning("Mixer::deletePlayHandle1 nph not found in lists");
}

void Mixer::removePlayHandlesOfTypes1(const Track* _track,
                                      const quint8 _types)
{
    if(_track == nullptr)
    {
        qWarning("Mixer::removePlayHandlesOfTypes track is null");
        return;
    }

    if(_types == 0)
    {
        qWarning("Mixer::removePlayHandlesOfTypes types is 0");
        return;
    }

    // qInfo("Mixer::removePlayHandlesOfTypes START %p %d", _track, _types);
    SafeList<PlayHandle*> r(false);
    requestChangeInModel();

    m_playHandles.map([&r, _types, _track](PlayHandle* ph) {
        if((ph->type() & _types) && ph->isFromTrack(_track))
            r.appendUnique(ph);
    });
    m_playHandlesToAdd.map([&r, _types, _track](PlayHandle* ph) {
        if((ph->type() & _types) && ph->isFromTrack(_track))
            r.appendUnique(ph);
    });
    m_playHandlesToRemove.map([&r, _types, _track](PlayHandle* ph) {
        if((ph->type() & _types) && ph->isFromTrack(_track))
            r.appendUnique(ph);
    });

    if(_types == 2)
        qInfo("remove PHOT IPH size=%d", r.size());
    if(_types == 0xFF)
        qInfo("remove PHOT ALL size=%d", r.size());

    r.map(
            [this](PlayHandle* ph) {
                if(ph->type() == 2)
                    qInfo("   set finished to iph");
                ph->resetRefCount();
                ph->setFinished();
                // this->removePlayHandle(ph);
            },
            true);

    doneChangeInModel();
    // qInfo("Mixer::removePlayHandlesOfTypes END %p %d", _track, _types);
}

void Mixer::removePlayHandlesForInstrument1(const Instrument* _instrument)
{
    if(_instrument == nullptr)
    {
        qWarning("Mixer::removePlayHandlesForInstrument1 instrument is null");
        return;
    }

    // qInfo("Mixer::removePlayHandlesForInstrument1 START %p %d", _track,
    // _types);
    SafeList<PlayHandle*> r(false);
    requestChangeInModel();

    m_playHandles.map([&r, _instrument](PlayHandle* ph) {
        if(ph->isFromInstrument(_instrument))
            r.appendUnique(ph);
    });
    m_playHandlesToAdd.map([&r, _instrument](PlayHandle* ph) {
        if(ph->isFromInstrument(_instrument))
            r.appendUnique(ph);
    });
    m_playHandlesToRemove.map([&r, _instrument](PlayHandle* ph) {
        if(ph->isFromInstrument(_instrument))
            r.appendUnique(ph);
    });

    r.map(
            [this](PlayHandle* ph) {
                if(ph->type() == 2)
                    qInfo("Mixer::removePlayHandlesForInstrument1 set "
                          "finished to iph");
                ph->resetRefCount();
                ph->setFinished();
                // this->removePlayHandle(ph);
            },
            true);

    doneChangeInModel();
    // qInfo("Mixer::removePlayHandlesForInstrument1 END %p %d", _track,
    // _types);
}

void Mixer::removeAllPlayHandles1()
{
    qInfo("Mixer::removeAllPlayHandles1 1");

    SafeList<PlayHandle*> r(false);
    requestChangeInModel();

    m_playHandles.map([&r](PlayHandle* ph) { r.appendUnique(ph); });
    m_playHandlesToAdd.map([&r](PlayHandle* ph) { r.appendUnique(ph); });
    m_playHandlesToRemove.map([&r](PlayHandle* ph) { r.appendUnique(ph); });

    r.map(
            [this](PlayHandle* ph) {
                ph->resetRefCount();
                ph->setFinished();
            },
            true);

    doneChangeInModel();

    qInfo("Mixer::removeAllPlayHandles1 2");
}

ConstNotePlayHandleList Mixer::nphsOfTrack(const Track* _track, bool _all)
{
    ConstNotePlayHandleList cnphv;

    if(_track == nullptr)
    {
        qWarning("Mixer::nphsOfTrack track is null");
        return cnphv;
    }

    // requestChangeInModel();
    /*
    for(PlayHandle* ph: m_playHandles)
    {
        if(!(ph->type() & PlayHandle::TypeNotePlayHandle)
           || !ph->isFromTrack(_track))
            continue;
        const NotePlayHandle* nph = dynamic_cast<const
    NotePlayHandle*>(ph); if(((nph->isReleased() == false &&
    nph->hasParent() == false)
            || _all == true))
            cnphv.append(nph);
    }
    */
    m_playHandles.map([&cnphv, _track, _all](PlayHandle* ph) {
        if((ph->type() & PlayHandle::TypeNotePlayHandle)
           && ph->isFromTrack(_track))
        {
            const NotePlayHandle* nph
                    = dynamic_cast<const NotePlayHandle*>(ph);
            if(((nph->isReleased() == false && nph->hasParent() == false)
                || _all == true))
                cnphv.append(nph);
        }
    });
    // doneChangeInModel();
    return cnphv;
}

void Mixer::waitUntilNoPlayHandle(const Track* _track, const quint8 _types)
{
    if(_track == nullptr)
    {
        qWarning("Mixer::waitUntilNoPlayHandle track is null");
        return;
    }

    if(_types == 0)
    {
        qWarning("Mixer::waitUntilNoPlayHandle types is 0");
        return;
    }

    bool                  again = true;
    SafeList<PlayHandle*> r(false);
    while(again)
    {
        // requestChangeInModel();

        m_playHandles.map([&r, _types, _track](PlayHandle* ph) {
            if((ph->type() & _types) && ph->isFromTrack(_track))
                r.appendUnique(ph);
        });
        m_playHandlesToAdd.map([&r, _types, _track](PlayHandle* ph) {
            if((ph->type() & _types) && ph->isFromTrack(_track))
                r.appendUnique(ph);
        });
        m_playHandlesToRemove.map([&r, _types, _track](PlayHandle* ph) {
            if((ph->type() & _types) && ph->isFromTrack(_track))
                r.appendUnique(ph);
        });

        again = !r.isEmpty();

        r.map([](PlayHandle* ph) {
            qInfo("wait track ph type=%d isFinished=%d", ph->type(),
                  ph->isFinished());
            ph->resetRefCount();  // force ?
            ph->setFinished();
        });

        // doneChangeInModel();

        if(again)
        {
            qInfo("Mixer::waitUntilNoPlayHandle(Track) still %d playHandles",
                  r.size());
            r.clear();
            QCoreApplication::sendPostedEvents();
            QThread::yieldCurrentThread();
            if(m_renderOnly)
                break;
        }
    }
}

void Mixer::waitUntilNoPlayHandle(const Instrument* _instrument)
{
    if(_instrument == nullptr)
    {
        qWarning("Mixer::waitUntilNoPlayHandle instrument is null");
        return;
    }

    bool                  again = true;
    SafeList<PlayHandle*> r(false);
    while(again)
    {
        // requestChangeInModel();

        m_playHandles.map([&r, _instrument](PlayHandle* ph) {
            if(ph->isFromInstrument(_instrument))
                r.appendUnique(ph);
        });
        m_playHandlesToAdd.map([&r, _instrument](PlayHandle* ph) {
            if(ph->isFromInstrument(_instrument))
                r.appendUnique(ph);
        });
        m_playHandlesToRemove.map([&r, _instrument](PlayHandle* ph) {
            if(ph->isFromInstrument(_instrument))
                r.appendUnique(ph);
        });

        again = !r.isEmpty();

        r.map([](PlayHandle* ph) {
            qInfo("wait instrument ph type=%d isFinished=%d", ph->type(),
                  ph->isFinished());
            ph->resetRefCount();  // force?
            ph->setFinished();
        });

        // doneChangeInModel();

        if(again)
        {
            qInfo("Mixer::waitUntilNoPlayHandle(Instrument) still %d "
                  "playHandles",
                  r.size());
            r.clear();
            QCoreApplication::sendPostedEvents();
            QThread::yieldCurrentThread();
            if(m_renderOnly)
                break;
        }
    }
}

void Mixer::adjustTempo(const bpm_t _tempo)
{
    // requestChangeInModel();
    // for(PlayHandle* ph: m_playHandles)
    m_playHandles.map([_tempo](PlayHandle* ph) {
        NotePlayHandle* nph = dynamic_cast<NotePlayHandle*>(ph);
        if(nph != nullptr && !nph->isReleased())
        {
            nph->lock();
            nph->resize(_tempo);
            nph->unlock();
        }
    });
    // doneChangeInModel();
}

void Mixer::requestChangeInModel()
{
    if(s_renderingThread)
        return;

    m_changesMutex.lock();
    m_changes++;
    /*
    if(m_changes > 1)
    {
        qWarning("Mixer::requestChangeInModel changes=%d", m_changes);
        BACKTRACE
    }
    */
    m_changesMutex.unlock();

    m_doChangesMutex.lock();

    m_waitChangesMutex.lock();
    if(m_isProcessing && !m_waitingForWrite && !m_changesSignal)
    {
        m_changesSignal = true;
        m_changesRequestCondition.wait(&m_waitChangesMutex);
    }
    m_waitChangesMutex.unlock();
}

void Mixer::doneChangeInModel()
{
    if(s_renderingThread)
        return;

    m_changesMutex.lock();
    /*
    if(m_changes > 1)
    {
        qWarning("Mixer::doneChangeInModel changes=%d", m_changes);
        BACKTRACE
    }
    */
    bool moreChanges = --m_changes;
    m_changesMutex.unlock();

    if(!moreChanges)
    {
        m_changesSignal = false;
        m_changesMixerCondition.wakeOne();
    }
    m_doChangesMutex.unlock();
}

void Mixer::runChangesInModel()
{
    if(m_changesSignal)
    {
        m_waitChangesMutex.lock();
        m_changesRequestCondition.wakeOne();
        m_changesMixerCondition.wait(&m_waitChangesMutex);
        m_waitChangesMutex.unlock();
    }
}

AudioDevice* Mixer::tryAudioDevices()
{
    bool         success_ful = false;
    AudioDevice* dev         = nullptr;
    QString      dev_name    = CONFIG_GET_STRING("mixer.audiodev");
    // ConfigManager::inst()->value( "mixer", "audiodev" );

    m_audioDevStartFailed = false;
    // BACKTRACE

#ifdef LMMS_HAVE_ALSA
    // qInfo("Mixer::tryAudioDevices trying device '%s'",
    // qPrintable(dev_name));

    if(dev_name == AudioAlsaGdx::name() || dev_name == "")
    {
        // qInfo("Mixer::tryAudioDevices #2");

        dev = new AudioAlsaGdx(success_ful, this);
        if(success_ful)
        {
            // qInfo("Mixer::tryAudioDevices #3");

            m_audioDevName = AudioAlsaGdx::name();
            return dev;
        }
        delete dev;
    }

    if(dev_name == AudioAlsa::name() || dev_name == "")
    {
        dev = new AudioAlsa(success_ful, this);
        if(success_ful)
        {
            // qInfo("Mixer::tryAudioDevices #4");
            m_audioDevName = AudioAlsa::name();
            return dev;
        }
        delete dev;
    }
#endif

#ifdef LMMS_HAVE_SDL
    if(dev_name == AudioSdl::name() || dev_name == "")
    {
        dev = new AudioSdl(success_ful, this);
        if(success_ful)
        {
            m_audioDevName = AudioSdl::name();
            return dev;
        }
        delete dev;
    }
#endif

#ifdef LMMS_HAVE_PULSEAUDIO
    if(dev_name == AudioPulseAudio::name() || dev_name == "")
    {
        dev = new AudioPulseAudio(success_ful, this);
        if(success_ful)
        {
            m_audioDevName = AudioPulseAudio::name();
            return dev;
        }
        delete dev;
    }
#endif

#ifdef LMMS_HAVE_OSS
    if(dev_name == AudioOss::name() || dev_name == "")
    {
        dev = new AudioOss(success_ful, this);
        if(success_ful)
        {
            m_audioDevName = AudioOss::name();
            return dev;
        }
        delete dev;
    }
#endif

#ifdef LMMS_HAVE_SNDIO
    if(dev_name == AudioSndio::name() || dev_name == "")
    {
        dev = new AudioSndio(success_ful, this);
        if(success_ful)
        {
            m_audioDevName = AudioSndio::name();
            return dev;
        }
        delete dev;
    }
#endif

#ifdef LMMS_HAVE_JACK
    if(dev_name == AudioJack::name() || dev_name == "")
    {
        dev = new AudioJack(success_ful, this);
        if(success_ful)
        {
            m_audioDevName = AudioJack::name();
            return dev;
        }
        delete dev;
    }
#endif

#ifdef LMMS_HAVE_PORTAUDIO
    if(dev_name == AudioPortAudio::name() || dev_name == "")
    {
        dev = new AudioPortAudio(success_ful, this);
        if(success_ful)
        {
            m_audioDevName = AudioPortAudio::name();
            return dev;
        }
        delete dev;
    }
#endif

#ifdef LMMS_HAVE_SOUNDIO
    if(dev_name == AudioSoundIo::name() || dev_name == "")
    {
        dev = new AudioSoundIo(success_ful, this);
        if(success_ful)
        {
            m_audioDevName = AudioSoundIo::name();
            return dev;
        }
        delete dev;
    }
#endif

    // add more device-classes here...
    // dev = new audioXXXX( SAMPLE_RATES[m_qualityLevel], success_ful,
    // this ); if( sucess_ful )
    //{
    //	return dev;
    //}
    // delete dev

    if(dev_name != AudioDummy::name())
    {
        qWarning(
                "Warning: No audio-driver working - falling back to "
                "dummy-audio-"
                "driver\nYou can render your songs and listen to the "
                "output "
                "files...\n");

        m_audioDevStartFailed = true;
    }

    m_audioDevName = AudioDummy::name();

    return new AudioDummy(success_ful, this);
}

MidiClient* Mixer::tryMidiClients()
{
    QString client_name = CONFIG_GET_STRING("mixer.mididev");
    // ConfigManager::inst()->value( "mixer","mididev" );

#ifdef LMMS_HAVE_ALSA
    if(client_name == MidiAlsaGdx::name() || client_name == "")
    {
        MidiAlsaGdx* malsag = new MidiAlsaGdx();
        if(malsag->isRunning())
        {
            m_midiClientName = MidiAlsaGdx::name();
            return malsag;
        }
        delete malsag;
    }

    if(client_name == MidiAlsaSeq::name() || client_name == "")
    {
        MidiAlsaSeq* malsas = new MidiAlsaSeq();
        if(malsas->isRunning())
        {
            m_midiClientName = MidiAlsaSeq::name();
            return malsas;
        }
        delete malsas;
    }

    if(client_name == MidiAlsaRaw::name() || client_name == "")
    {
        MidiAlsaRaw* malsar = new MidiAlsaRaw();
        if(malsar->isRunning())
        {
            m_midiClientName = MidiAlsaRaw::name();
            return malsar;
        }
        delete malsar;
    }
#endif

#ifdef LMMS_HAVE_JACK
    if(client_name == MidiJack::name() || client_name == "")
    {
        MidiJack* mjack = new MidiJack();
        if(mjack->isRunning())
        {
            m_midiClientName = MidiJack::name();
            return mjack;
        }
        delete mjack;
    }
#endif

#ifdef LMMS_HAVE_OSS
    if(client_name == MidiOss::name() || client_name == "")
    {
        MidiOss* moss = new MidiOss();
        if(moss->isRunning())
        {
            m_midiClientName = MidiOss::name();
            return moss;
        }
        delete moss;
    }
#endif

#ifdef LMMS_HAVE_SNDIO
    if(client_name == MidiSndio::name() || client_name == "")
    {
        MidiSndio* msndio = new MidiSndio();
        if(msndio->isRunning())
        {
            m_midiClientName = MidiSndio::name();
            return msndio;
        }
        delete msndio;
    }
#endif

#ifdef LMMS_BUILD_WIN32
    if(client_name == MidiWinMM::name() || client_name == "")
    {
        MidiWinMM* mwmm = new MidiWinMM();
        // if( moss->isRunning() )
        {
            m_midiClientName = MidiWinMM::name();
            return mwmm;
        }
        delete mwmm;
    }
#endif

#ifdef LMMS_BUILD_APPLE
    printf("trying midi apple...\n");
    if(client_name == MidiApple::name() || client_name == "")
    {
        MidiApple* mapple = new MidiApple();
        m_midiClientName  = MidiApple::name();
        printf("Returning midi apple\n");
        return mapple;
    }

    qWarning("Error: MIDI apple didn't work: client_name=%s",
             qPrintable(client_name));
#endif

    qWarning(
            "Error: Couldn't create MIDI-client, neither with ALSA "
            "nor with "
            "OSS. Will use dummy-MIDI-client.");

    m_midiClientName = MidiDummy::name();

    return new MidiDummy;
}

FifoWriter::FifoWriter(Mixer* mixer, fifo* _fifo) :
      m_mixer(mixer), m_fifo(_fifo), m_writing(true)
{
    setObjectName("mixer:fifowriter");
}

void FifoWriter::finish()
{
    m_writing = false;
}

void FifoWriter::run()
{
    // qInfo("FifoWriter::run");
    disable_denormals();

#if 0
#ifdef LMMS_BUILD_LINUX
#ifdef LMMS_HAVE_SCHED_H
	cpu_set_t mask;
	CPU_ZERO( &mask );
	CPU_SET( 0, &mask );
	sched_setaffinity( 0, sizeof( mask ), &mask );
#endif
#endif
#endif

    const fpp_t frames = m_mixer->framesPerPeriod();
    while(m_writing)
    {
        surroundSampleFrame*       buffer = new surroundSampleFrame[frames];
        const surroundSampleFrame* b      = m_mixer->renderNextBuffer();
        memcpy(buffer, b, frames * sizeof(surroundSampleFrame));
        write(buffer);
    }

    write(nullptr);
}

void FifoWriter::write(surroundSampleFrame* buffer)
{
    m_mixer->m_waitChangesMutex.lock();
    m_mixer->m_waitingForWrite = true;
    m_mixer->m_waitChangesMutex.unlock();
    m_mixer->runChangesInModel();

    m_fifo->write(buffer);

    m_mixer->m_doChangesMutex.lock();
    m_mixer->m_waitingForWrite = false;
    m_mixer->m_doChangesMutex.unlock();
}

HandleManager::HandleManager(Mixer* _mixer) : m_mixer(_mixer)
{
    qInfo("HandleManager::HandleManager mixer=%p", m_mixer);
    setObjectName("mixer:hm");
    moveToThread(this);

    connect(m_mixer, SIGNAL(playHandleToAdd(PlayHandle*)), this,
            SLOT(addPlayHandleHM(PlayHandle*)), Qt::QueuedConnection);
    connect(m_mixer, SIGNAL(playHandleToRemove(PlayHandle*)), this,
            SLOT(removePlayHandleHM(PlayHandle*)), Qt::QueuedConnection);
    connect(m_mixer,
            SIGNAL(playHandlesOfTypesToRemove(const Track*, const quint8)),
            this,
            SLOT(removePlayHandlesOfTypesHM(const Track*, const quint8)),
            Qt::QueuedConnection);
    connect(m_mixer,
            SIGNAL(playHandlesForInstrumentToRemove(const Instrument*)), this,
            SLOT(removePlayHandlesForInstrumentHM(const Instrument*)),
            Qt::QueuedConnection);
    connect(m_mixer, SIGNAL(allPlayHandlesRemoval()), this,
            SLOT(removeAllPlayHandlesHM()), Qt::QueuedConnection);
}

HandleManager::~HandleManager()
{
    qInfo("HandleManager::~HandleManager");
}

void HandleManager::addPlayHandleHM(PlayHandle* _handle)
{
    // qInfo("HandleManager::addPlayHandleHM");
    if(QThread::currentThread() != this)
        qInfo("HandleManager::addPlayHandleHM STRANGE");

    m_mixer->addPlayHandle1(_handle);
}

void HandleManager::removePlayHandleHM(PlayHandle* _handle)
{
    // qInfo("HandleManager::removePlayHandleHM");
    if(QThread::currentThread() != this)
        qInfo("HandleManager::removePlayHandleHM STRANGE");

    m_mixer->removePlayHandle1(_handle);
}

void HandleManager::removePlayHandlesOfTypesHM(const Track* _track,
                                               const quint8 _types)
{
    // qInfo("HandleManager::removePlayHandlesOfTypesHM");
    if(QThread::currentThread() != this)
        qInfo("HandleManager::removePlayHandlesOfTypesHM STRANGE");

    m_mixer->removePlayHandlesOfTypes1(_track, _types);
}

void HandleManager::removePlayHandlesForInstrumentHM(
        const Instrument* _instrument)
{
    // qInfo("HandleManager::removePlayHandlesForInstrumentHM");
    if(QThread::currentThread() != this)
        qInfo("HandleManager::removePlayHandlesForInstrumentHM STRANGE");

    m_mixer->removePlayHandlesForInstrument1(_instrument);
}

void HandleManager::removeAllPlayHandlesHM()
{
    qInfo("HandleManager::removeAllPlayHandlesHM");
    if(QThread::currentThread() != this)
        qInfo("HandleManager::removeAllPlayHandlesHM STRANGE");

    m_mixer->removeAllPlayHandles1();
}
