/*
 * InstrumentTrack.cpp - Track which provides arrangement of notes
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
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

#include "InstrumentTrack.h"

#include "AutomationPattern.h"
#include "BBTrack.h"
#include "CaptionMenu.h"
#include "ConfigManager.h"
#include "ControllerConnection.h"
#include "EffectChain.h"
#include "EffectChainView.h"
#include "FadeButton.h"
#include "FileBrowser.h"
#include "FileDialog.h"
#include "FxLineLcdSpinBox.h"
#include "FxMixer.h"
#include "FxMixerView.h"
#include "GroupBox.h"
#include "GuiApplication.h"
#include "Instrument.h"
#include "InstrumentFunctionView.h"
#include "InstrumentMidiIOView.h"
#include "InstrumentSoundShapingView.h"
#include "Knob.h"
#include "LcdSpinBox.h"
#include "LedCheckBox.h"
#include "LeftRightNav.h"
#include "MainWindow.h"
#include "MidiClient.h"
#include "MidiPortMenu.h"
#include "MixHelpers.h"
#include "Mixer.h"
#include "Pattern.h"
#include "PeripheralLaunchpadView.h"
#include "PeripheralPadsView.h"
#include "PianoView.h"
#include "PluginFactory.h"
#include "PluginView.h"
//#include "SamplePlayHandle.h"
#include "Backtrace.h"
#include "Scale.h"
#include "Song.h"
#include "StringPairDrag.h"
#include "TabWidget.h"
#include "TimeLineWidget.h"
#include "ToolTip.h"
#include "TrackContainerView.h"
#include "TrackLabelButton.h"
#include "embed.h"
#include "gui_templates.h"
#include "lmms_math.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QQueue>
#include <QSignalMapper>

const char* ITVOLHELP = QT_TRANSLATE_NOOP("InstrumentTrack",
                                          "With this knob you can set "
                                          "the volume of the opened "
                                          "channel.");

const int INSTRUMENT_WIDTH             = 250;
const int INSTRUMENT_HEIGHT            = INSTRUMENT_WIDTH;
const int PIANO_HEIGHT                 = 80;
const int INSTRUMENT_WINDOW_CACHE_SIZE = 8;

InstrumentTrack::InstrumentTrack(TrackContainer* tc) :
      Track(Track::InstrumentTrack, tc), MidiEventProcessor(),
      m_midiPort(tr("Instrument track"),
                 Engine::mixer()->midiClient(),
                 this,
                 this),
      m_notes(), m_sustainedNotes(true),
      m_midiNotesMutex("InstrumentTrack::m_midiNotesMutex", false),
      m_sustainPedalPressed(false), m_silentBuffersProcessed(false),
      /*m_previewMode(false),*/ m_scale(nullptr),
      m_rootModel(this, tr("Root"), "root"),
      m_modeModel(this, tr("Mode"), "mode"),
      m_baseNoteModel(0, 0, NumKeys - 1, this, tr("Base note"), "baseNote"),
      m_volumeEnabledModel(true, this, tr("Enabled Volume"), "volumeEnabled"),
      m_volumeModel(
              DefaultVolume, MinVolume, MaxVolume, 0.1, this, tr("Volume")),
      m_noteVolumeModel(DefaultVolume,
                        MinVolume,
                        MaxVolume,
                        0.1,
                        this,
                        tr("Note Volume"),
                        "noteVolume"),
      m_panningEnabledModel(
              true, this, tr("Enabled Panning"), "panningEnabled"),
      m_panningModel(DefaultPanning,
                     PanningLeft,
                     PanningRight,
                     0.1,
                     this,
                     tr("Panning"),
                     "panning"),
      m_notePanningModel(DefaultPanning,
                         PanningLeft,
                         PanningRight,
                         0.1,
                         this,
                         tr("Note Panning"),
                         "notePanning"),
      m_bendingEnabledModel(true, this, tr("Enabled bending")),
      m_bendingModel(DefaultPitch,
                     MinPitchDefault,
                     MaxPitchDefault,
                     1.,
                     this,
                     tr("Bending"),
                     "bending"),
      m_noteBendingModel(
              0., -48., 48., 0.5, this, tr("Note bending"), "noteBending"),
      m_bendingRangeModel(
              1, 1, 60, this, tr("Bending range"), "bendingRange"),
      m_useMasterPitchModel(true, this, tr("Master Pitch"), "masterPitch"),
      m_effectChannelModel(0, 0, 0, this, tr("FX channel"), "fxChannel"),
      m_audioPort(nullptr), m_instrument(nullptr), m_soundShaping(this),
      m_piano(this), m_envFilter1(nullptr), m_envFilter2(nullptr)
{
    m_audioPort.reset(new AudioPort(
            tr("Instrument track"), true, &m_volumeEnabledModel,
            &m_volumeModel, &m_panningEnabledModel, &m_panningModel,
            &m_bendingEnabledModel, &m_bendingModel, &m_mutedModel,
            &m_frozenModel, &m_clippingModel));
    Engine::mixer()->emit audioPortToAdd(m_audioPort);

    for(uint8_t i = 0; i < MidiControllerCount; i++)
    {
        m_midiCCModel[i] = new FloatModel(0., 0., 127., 1., this,
                                          QString("Midi CC %1").arg(i));
        m_midiCCModel[i]->setStrictStepSize(true);
        connect(m_midiCCModel[i], SIGNAL(dataChanged()), this,
                SLOT(updateMidiCC()));
    }

    setColor(QColor("#2BB34E"));
    setUseStyleColor(true);

    m_noteFunctions.append(new InstrumentFunctionNoteFiltering(this));
    m_noteFunctions.append(
            new InstrumentFunctionNoteDuplicatesRemoving(this));
    m_noteFunctions.append(new InstrumentFunctionNoteKeying(this));
    m_noteFunctions.append(new InstrumentFunctionNoteHumanizing(this));
    m_noteFunctions.append(new InstrumentFunctionGlissando(this));
    m_noteFunctions.append(new InstrumentFunctionNoteStacking(this));
    m_noteFunctions.append(new InstrumentFunctionArpeggio(this));
    m_noteFunctions.append(new InstrumentFunctionNoteSustaining(this));
    m_noteFunctions.append(new InstrumentFunctionNoteOutting(this));
    m_noteFunctions.append(new InstrumentFunctionNotePlaying(this));

    m_bendingModel.setCenterValue(DefaultPitch);
    m_panningModel.setCenterValue(DefaultPanning);
    m_baseNoteModel.setInitValue(DefaultKey);

    Note::fillRootModel(m_rootModel);
    m_rootModel.setInitValue(-1);
    connect(&m_rootModel, SIGNAL(dataChanged()), this,
            SLOT(onRootOrModeChanged()));

    ChordDef::fillModeModel(m_modeModel, true);
    m_modeModel.setInitValue(-1);
    connect(&m_modeModel, SIGNAL(dataChanged()), this,
            SLOT(onRootOrModeChanged()));

    m_effectChannelModel.setRange(0, Engine::fxMixer()->numChannels() - 1);

    for(int i = 0; i < NumMidiKeys; ++i)
    {
        m_notes[i]            = nullptr;
        m_runningMidiNotes[i] = 0;
    }

    // setName( tr( "Default preset" ) );
    setName(tr("Instrument track"));

    connect(&m_baseNoteModel, SIGNAL(dataChanged()), this,
            SLOT(updateBaseNote()));
    connect(&m_effectChannelModel, SIGNAL(dataChanged()), this,
            SLOT(updateEffectChannel()));

    connect(&m_volumeEnabledModel, SIGNAL(dataChanged()), this,
            SLOT(updateVolume()));
    connect(&m_volumeModel, SIGNAL(dataChanged()), this,
            SLOT(updateVolume()));
    connect(&m_noteVolumeModel, SIGNAL(dataChanged()), this,
            SLOT(updateVolume()));

    connect(&m_panningEnabledModel, SIGNAL(dataChanged()), this,
            SLOT(updatePanning()));
    connect(&m_panningModel, SIGNAL(dataChanged()), this,
            SLOT(updatePanning()));
    connect(&m_notePanningModel, SIGNAL(dataChanged()), this,
            SLOT(updatePanning()));

    connect(&m_bendingEnabledModel, SIGNAL(dataChanged()), this,
            SLOT(updateBending()));
    connect(&m_bendingModel, SIGNAL(dataChanged()), this,
            SLOT(updateBending()));
    connect(&m_noteBendingModel, SIGNAL(dataChanged()), this,
            SLOT(updateBending()));
    connect(&m_bendingRangeModel, SIGNAL(dataChanged()), this,
            SLOT(updateBendingRange()));

    connect(this, SIGNAL(instrumentChanged()), this, SIGNAL(iconChanged()));
}

InstrumentTrack::~InstrumentTrack()
{
    qInfo("InstrumentTrack::~InstrumentTrack [%s] START", qPrintable(name()));
    setMuted(true);

    /*
    if(m_instrument != nullptr)
    {
        lock();
        Engine::mixer()->emit playHandlesForInstrumentToRemove(m_instrument);
        qInfo("InstrumentTrack::~InstrumentTrack wait ForInstrument");
        QCoreApplication::sendPostedEvents();
        // QThread::yieldCurrentThread();
        Engine::mixer()->waitUntilNoPlayHandle(m_instrument);
        unlock();
    }
    */

    // kill all running notes
    silenceAllNotes();  // true

    qInfo("InstrumentTrack::~InstrumentTrack 2");
    // now we're save deleting the instrument
    Instrument* old = m_instrument;
    if(old != nullptr)
    {
        Engine::mixer()->emit playHandlesForInstrumentToRemove(old);
        qInfo("InstrumentTrack::~InstrumentTrack wait instrument");
        QCoreApplication::sendPostedEvents();
        QThread::yieldCurrentThread();
        Engine::mixer()->waitUntilNoPlayHandle(old);

        qInfo("~InstrumentTrack delete instrument START [%s]",
              qPrintable(name()));
        m_instrument = nullptr;
        delete old;
        qInfo("~InstrumentTrack delete instrument END [%s]",
              qPrintable(name()));
    }

    Engine::mixer()->audioPortToRemove(m_audioPort);

    for(uint8_t i = 0; i < MidiControllerCount; i++)
    {
        delete m_midiCCModel[i];
        m_midiCCModel[i] = nullptr;
    }

    qInfo("InstrumentTrack::~InstrumentTrack [%s] END", qPrintable(name()));

    qInfo("InstrumentTrack::~InstrumentTrack CHILDREN");
    for(QObject* o: children())
    {
        qInfo("  - child %p", o);
        qInfo("          '%s' '%s'", qPrintable(o->objectName()),
              typeid(o).name());
    }

    // m_audioPort.clear();
    // m_audioPort.reset(nullptr);
}

QString InstrumentTrack::defaultName() const
{
    QString r = instrumentName();
    if(r.isEmpty())
        r = tr("Instrument track");
    return r;
}

int InstrumentTrack::baseNote() const
{
    int mp = m_useMasterPitchModel.value() ? Engine::song()->masterPitch()
                                           : 0;

    return m_baseNoteModel.value() - mp;
}

void InstrumentTrack::processAudioBuffer(sampleFrame*    buf,
                                         const fpp_t     frames,
                                         NotePlayHandle* nph)
{
    const Song* song = Engine::song();

    // we must not play the sound if this InstrumentTrack is muted...
    if(/*tmp isMuted() ||*/
       (song->playMode() != Song::Mode_PlayPattern && nph != nullptr
        && nph->isTrackMuted())  // isBbTrackMuted())
       || m_instrument == nullptr)
    {
        memset(buf, 0, frames * BYTES_PER_FRAME);
        return;
    }

    // Test for silent input data if instrument provides a single stream only
    // (i.e. driven by InstrumentPlayHandle) We could do that in all other
    // cases as well but the overhead for silence test is bigger than what we
    // potentially save. While playing a note, a NotePlayHandle-driven
    // instrument will produce sound in 99 of 100 cases so that test would be
    // a waste of time.
    if(m_instrument->isSingleStreamed() && MixHelpers::isSilent(buf, frames))
    {
        // at least pass one silent buffer to allow
        if(m_silentBuffersProcessed)
        {
            // skip further processing
            memset(buf, 0, frames * BYTES_PER_FRAME);
            return;
        }
        m_silentBuffersProcessed = true;
    }
    else
    {
        m_silentBuffersProcessed = false;
    }

    // if effects "went to sleep" because there was no input, wake them up
    // now
    // m_audioPort.effects()->startRunning();

    // get volume knob data
    // static const float DefaultVolumeRatio = 1.0f / DefaultVolume;
    /*ValueBuffer * volBuf = m_volumeModel.valueBuffer();
    float v_scale = volBuf
            ? 1.0f
            : getVolume() * DefaultVolumeRatio;*/

    // instruments using instrument-play-handles will call this method
    // without any knowledge about notes, so they pass nullptr for n, which
    // is no problem for us since we just bypass the envelopes+LFOs
    if(nph != nullptr)  // && !m_instrument->isSingleStreamed())
    {
        const f_cnt_t offset = nph->noteOffset();
        m_soundShaping.processAudioBuffer(buf + offset, frames - offset, nph);
    }
    else
    {
        if(m_envFilter1 == nullptr)
            m_envFilter1 = new BasicFilters<>(
                    Engine::mixer()->processingSampleRate());
        if(m_envFilter2 == nullptr)
            m_envFilter2 = new BasicFilters<>(
                    Engine::mixer()->processingSampleRate());
        m_soundShaping.processAudioBuffer(
                buf + m_envOffset, frames - m_envOffset, m_envFilter1,
                m_envFilter2, m_envTotalFramesPlayed, m_envReleaseBegin,
                m_envLegato, m_envMarcato, m_envStaccato);
    }

    // if(nph != nullptr)
    {

        if(!m_instrument->isMidiBased() || m_volumeEnabledModel.value()
           || m_panningEnabledModel.value())
        {
            const volume_t vol
                    = m_volumeEnabledModel.value() ? qBound(
                              MinVolume,
                              nph == nullptr ? m_envVolume : nph->getVolume(),
                              MaxVolume)
                                                   : DefaultVolume;
            const panning_t pan
                    = m_panningEnabledModel.value()
                              ? qBound(PanningLeft,
                                       nph == nullptr ? m_envPanning
                                                      : nph->getPanning(),
                                       PanningRight)
                              : DefaultPanning;
            StereoGain sg = toStereoGain(pan, vol);

            const f_cnt_t offset
                    = nph == nullptr ? m_envOffset : nph->noteOffset();
            for(f_cnt_t f = offset; f < frames; ++f)
            {
                for(int c = 0; c < 2; ++c)
                {
                    buf[f][c] *= sg.gain[c];
                }
            }
        }
    }

    // if(MixHelpers::sanitize(buf,frames))
    //        qInfo("InstrumentTrack: sanitize done!!!");
    // if(MixHelpers::isClipping(buf,frames))
    //        setClipping(true);
}

/*
MidiEvent InstrumentTrack::applyMasterKey(const MidiEvent& event)
{
    MidiEvent copy(event);
    switch(event.type())
    {
        case MidiNoteOn:
        case MidiNoteOff:
        case MidiKeyPressure:
            copy.setKey(masterKey(event.key()));
            break;
        default:
            break;
    }
    return copy;
}
*/

void InstrumentTrack::removeMidiNote(const int _key, const f_cnt_t _offset)
{
    m_midiNotesMutex.lock();
    qInfo("IT::removeMidiNote key=%d", _key);
    NotePlayHandle* n = m_notes[_key];

    if(n != nullptr)  // || !n->isFinished())
    {
        // n->incrRefCount();
        PlayHandlePointer ph = n->pointer();
        m_notes[_key]        = nullptr;
        m_midiNotesMutex.unlock();
        if(isSustainPedalPressed() && n->origin() == n->OriginMidiInput)
        {
            m_sustainedNotes.append(n);
            m_sustainedPlayHandles.append(ph);
        }
        else
        {
            n->noteOff(_offset);
        }
    }
    else
        m_midiNotesMutex.unlock();
}

void InstrumentTrack::addMidiNote(const int      _key,
                                  const f_cnt_t  _offset,
                                  const volume_t _volume,
                                  const int      _channel)
{
    m_midiNotesMutex.lock();
    qInfo("IT::addMidiNote key=%d", _key);
    NotePlayHandle* n = m_notes[_key];

    if(n == nullptr)
    {
        n = NotePlayHandleManager::acquire(
                this, _offset, std::numeric_limits<f_cnt_t>::max() / 2,
                Note(MidiTime(), MidiTime(), _key, _volume), nullptr,
                _channel, NotePlayHandle::OriginMidiInput);
        // n->incrRefCount();
        m_notes[_key] = n;
        Engine::mixer()->emit playHandleToAdd(n->pointer());
        m_midiNotesMutex.unlock();
    }
    else
    {
        m_midiNotesMutex.unlock();
    }
}

void InstrumentTrack::processInEvent(const MidiEvent& event,
                                     const MidiTime&  time,
                                     f_cnt_t          offset)
{
    if(Engine::song()->isExporting())
        return;

    // return;  // TMP

    bool eventHandled = false;

    const int key = event.key() + baseNote() - DefaultKey;

    switch(event.type())
    {
        // we don't send MidiNoteOn, MidiNoteOff and MidiKeyPressure
        // events to instrument as NotePlayHandle will send them on its
        // own
        case MidiNoteOn:
            if(key >= 0 && key < NumMidiKeys)
            {
                if(event.velocity() > 0)
                {
                    qInfo("IT::pIE noteOn before");
                    // Engine::mixer()->requestChangeInModel();
                    addMidiNote(key, offset,
                                event.volume(midiPort()->baseVelocity()),
                                event.channel());
                    // Engine::mixer()->doneChangeInModel();
                    qInfo("IT::pIE noteOn after");
                }
            }
            eventHandled = true;
            break;

        case MidiNoteOff:
            if(key >= 0 && key < NumMidiKeys)
            {
                qInfo("IT::pIE noteOff before 0a");
                // Engine::mixer()->requestChangeInModel();
                removeMidiNote(key, offset);

                /*
                qInfo("IT::pIE noteOff before 0c");
                // Engine::mixer()->doneChangeInModel();
                // if(n != nullptr) n->lock();

                if(n != nullptr)
                {
                    // do actual note off and remove internal reference to
                    // NotePlayHandle (which itself will be deleted later
                    // automatically)
                    n->noteOff(offset);
                    n->decrRefCount();
                    if(isSustainPedalPressed()
                       && n->origin() == n->OriginMidiInput)
                        m_sustainedNotes.append(n);
                    else
                        n->decrRefCount();
                    // n->unlock();
                }
                */

                // Engine::mixer()->doneChangeInModel();
                qInfo("IT::pIE noteOff after");
            }
            eventHandled = true;
            break;

        case MidiKeyPressure:
            if(key >= 0 && key < NumMidiKeys)
            {
                qInfo("IT::pIE notePressure before 0a");
                m_midiNotesMutex.lock();
                // Engine::mixer()->requestChangeInModel();
                NotePlayHandle* n = m_notes[key];
                // if(n != nullptr) n->incrRefCount();
                // Engine::mixer()->doneChangeInModel();
                qInfo("IT::pIE notePressure after 1");

                if(n != nullptr)
                {
                    PlayHandlePointer ph = n->pointer();
                    m_midiNotesMutex.unlock();
                    // setVolume() calls processOutEvent() with
                    // MidiKeyPressure so the attached instrument will
                    // receive the event as well
                    n->setVolume(event.volume(midiPort()->baseVelocity()));
                    // n->decrRefCount();
                }
                else
                {
                    m_midiNotesMutex.unlock();
                }
            }
            eventHandled = true;
            break;

        case MidiPitchBend:
            // updatePitch() is connected to m_bendingModel::dataChanged()
            // which will send out MidiPitchBend events
            m_bendingModel.setValue(m_bendingModel.minValue()
                                    + event.midiPitchBend()
                                              * m_bendingModel.range()
                                              / MidiMaxPitchBend);
            // eventHandled = true;  // TMP?
            break;

        case MidiControlChange:
            if(event.controllerNumber() == MidiControllerSustain)
            {
                if(event.controllerValue() > MidiMaxControllerValue / 2)
                {
                    m_sustainPedalPressed = true;
                    // eventHandled          = true;  // TMP?
                }
                else if(isSustainPedalPressed())
                {
                    /*
                    for(NotePlayHandle* nph: m_sustainedNotes)
                    {
                        if(nph != nullptr)
                        {
                            if(nph->isReleased())
                            {
                                if(nph->origin() == nph->OriginMidiInput)
                                {
                                    nph->setLength(MidiTime(static_cast<
                                                            f_cnt_t>(
                                            nph->totalFramesPlayed()
                                            / Engine::framesPerTick())));
                                    midiNoteOff(*nph);
                                }
                            }
                            nph->decrRefCount();
                        }
                    }
                    m_sustainedNotes.clear();
                    */
                    m_sustainedNotes.map(
                            [this](NotePlayHandle* nph) {
                                if(nph != nullptr && !nph->isFinished())
                                {
                                    if(nph->isReleased())
                                    {
                                        if(nph->origin()
                                           == nph->OriginMidiInput)
                                        {
                                            nph->setLength(MidiTime(static_cast<
                                                                    f_cnt_t>(
                                                    nph->totalFramesPlayed()
                                                    / Engine::
                                                            framesPerTick())));
                                            midiNoteOff(*nph);
                                        }
                                    }
                                    // nph->decrRefCount();
                                    m_sustainedPlayHandles.removeOne(
                                            nph->pointer());
                                }
                            },
                            true);

                    m_sustainPedalPressed = false;
                    // eventHandled          = true;  // TMP?
                }
            }
            if(event.controllerNumber() == MidiControllerAllSoundOff
               || event.controllerNumber() == MidiControllerAllNotesOff
               || event.controllerNumber() == MidiControllerOmniOn
               || event.controllerNumber() == MidiControllerOmniOff
               || event.controllerNumber() == MidiControllerMonoOn
               || event.controllerNumber() == MidiControllerPolyOn)
            {
                silenceAllNotes();
                // eventHandled = true;  // TMP?
            }
            break;

        case MidiMetaEvent:
            // handle special cases such as note panning
            switch(event.metaEvent())
            {
                case MidiNotePanning:
                    NotePlayHandle* n;
                    m_midiNotesMutex.lock();
                    // Engine::mixer()->requestChangeInModel();
                    n = m_notes[key];
                    // if(n != nullptr) n->incrRefCount();
                    // Engine::mixer()->doneChangeInModel();

                    if(n != nullptr)
                    {
                        PlayHandlePointer ph = n->pointer();
                        m_midiNotesMutex.unlock();
                        eventHandled = true;
                        n->setPanning(event.panning());
                        // n->decrRefCount();
                    }
                    else
                    {
                        m_midiNotesMutex.unlock();
                    }

                    // eventHandled = true;  // TMP?
                    break;

                default:
                    qWarning(
                            "InstrumentTrack: unhandled MIDI meta event: "
                            "%i",
                            event.metaEvent());
                    // eventHandled = true;  // TMP?
                    break;
            }
            break;

        default:
            break;
    }

    if(eventHandled == false
       && instrument()->handleMidiEvent(event, time, offset) == false)
    {
        qWarning("InstrumentTrack: unhandled MIDI event %d", event.type());
    }
}

void InstrumentTrack::processOutEvent(const MidiEvent& event,
                                      const MidiTime&  time,
                                      f_cnt_t          offset)
{
    propagateMidiOutEvent(event, time, offset);

    // do nothing if we do not have an instrument instance (e.g. when
    // loading settings)
    if(m_instrument == nullptr)
        return;

    // const MidiEvent transposedEvent = applyMasterKey(event);
    // const int       key             = transposedEvent.key();
    const int key = event.key();

    switch(event.type())
    {
        case MidiNoteOn:
            if(key >= 0 && key < NumMidiKeys)
            {
                m_midiNotesMutex.lock();
                const int nboff         = m_runningMidiNotes[key];
                m_runningMidiNotes[key] = 1;
                m_midiNotesMutex.unlock();

                for(int i = 0; i < nboff; i++)
                    m_instrument->handleMidiEvent(
                            MidiEvent(MidiNoteOff,
                                      midiPort()->outputChannel() - 1,
                                      key - baseNote() + DefaultKey, 0),
                            time, offset);
                m_instrument->handleMidiEvent(
                        MidiEvent(MidiNoteOn, midiPort()->outputChannel() - 1,
                                  key - baseNote() + DefaultKey,
                                  event.velocity()),
                        time, offset);

                emit         newNote();
                m_piano.emit dataChanged();
            }

            // event.key() = original key
            // m_piano.setKeyState(event.key(), true);
            // m_piano.pressKey(event.key());
            break;

        case MidiNoteOff:
            if(key >= 0 && key < NumMidiKeys)
            {
                m_midiNotesMutex.lock();
                const int nboff         = m_runningMidiNotes[key];
                m_runningMidiNotes[key] = 0;
                m_midiNotesMutex.unlock();

                for(int i = 0; i < nboff; i++)
                    m_instrument->handleMidiEvent(
                            MidiEvent(MidiNoteOff,
                                      midiPort()->outputChannel() - 1,
                                      key - baseNote() + DefaultKey, 0),
                            time, offset);
                /*
                m_noteVolumeModel.setAutomatedValue(0.);
                m_notePanningModel.setAutomatedValue(0.);
                m_noteBendingModel.setAutomatedValue(0.);
                */
                // emit         endNote();
                m_piano.emit dataChanged();
            }

            // event.key() = original key
            // m_piano.setKeyState(event.key(), false);
            // m_piano.releaseKey(event.key());
            break;

        default:
            m_instrument->handleMidiEvent(event,  // transposedEvent,
                                          time, offset);
            break;
    }

    // if appropriate, midi-port does futher routing
    m_midiPort.processOutEvent(event, time);
}

void InstrumentTrack::silenceAllNotes(/*bool removeIPH*/)
{
    // qInfo("InstrumentTrack::silenceAllNotes 0a");
    // lock();
    // m_sustainedNotes.clear();
    m_sustainedNotes.map(
            [this](NotePlayHandle* nph) {
                if(nph != nullptr)
                {
                    const int key = nph->key();
                    // nph->decrRefCount();
                    PlayHandlePointer ph = nph->pointer();
                    nph->setFinished();
                    processOutEvent(MidiEvent(MidiNoteOff, -1, key, 0));
                    m_sustainedPlayHandles.removeOne(ph);
                }
            },
            true);
    // unlock();
    // qInfo("InstrumentTrack::silenceAllNotes 0b");

    // Engine::mixer()->requestChangeInModel();
    // qInfo("InstrumentTrack::silenceAllNotes 1a");
    for(int i = 0; i < NumMidiKeys; ++i)
    {
        if(m_notes[i] != nullptr || m_runningMidiNotes[i] != 0)
        {
            qInfo("InstrumentTrack: silence %d", i);
            processOutEvent(MidiEvent(MidiNoteOff, -1, i, 0));
        }
    }
    // qInfo("InstrumentTrack::silenceAllNotes 1b");
    m_midiNotesMutex.lock();
    // qInfo("InstrumentTrack::silenceAllNotes 1b1");
    for(int i = 0; i < NumMidiKeys; ++i)
    {
        NotePlayHandle* nph = m_notes[i];
        if(nph != nullptr || m_runningMidiNotes[i] != 0)
        {
            if(nph != nullptr)
            {
                // nph->resetRefCount();
                nph->setFinished();
            }

            qInfo("InstrumentTrack: silence %d", i);
            m_midiNotesMutex.unlock();
            processOutEvent(MidiEvent(MidiNoteOff, -1, i, 0));
            m_midiNotesMutex.lock();
        }
        m_notes[i]            = nullptr;
        m_runningMidiNotes[i] = 0;
    }
    m_midiNotesMutex.unlock();

    // qInfo("InstrumentTrack::silenceAllNotes 2");

    // lock();
    // qInfo("InstrumentTrack::silenceAllNotes 3b");
    m_processHandles.clear();
    // qInfo("InstrumentTrack::silenceAllNotes 3c");
    // unlock();
    // Engine::mixer()->doneChangeInModel();

    // qInfo("InstrumentTrack::silenceAllNotes 4");
    quint8 types = PlayHandle::TypeNotePlayHandle
                   | PlayHandle::TypePresetPreviewHandle;
    /*
    if(removeIPH)
    {
        qInfo("InstrumentTrack::silenceAllNotes removeIPH t=%s",
              qPrintable(name()));
        types |= PlayHandle::TypeInstrumentPlayHandle;
    }
    */

    // lock();
    Engine::mixer()->emit playHandlesOfTypesToRemove(this, types);
    // qInfo("InstrumentTrack::silenceAllNotes 5a");
    QCoreApplication::sendPostedEvents();
    QThread::yieldCurrentThread();
    // qInfo("InstrumentTrack::silenceAllNotes 5c");
    Engine::mixer()->waitUntilNoPlayHandle(this, types);
    // unlock();

    // qInfo("InstrumentTrack::silenceAllNotes 6");
}

void InstrumentTrack::onRootOrModeChanged()
{
    // TODO
}

f_cnt_t InstrumentTrack::beatLen(NotePlayHandle* _n) const
{
    if(m_instrument != nullptr)
    {
        const f_cnt_t len = m_instrument->beatLen(_n);
        if(len > 0)
        {
            return len;
        }
    }
    return m_soundShaping.envFrames();
}

void InstrumentTrack::playNote(NotePlayHandle* _nph,
                               sampleFrame*    _workingBuffer)
{
    /*
    // arpeggio- and chord-widget has to do its work -> adding sub-notes
    // for chords/arpeggios
    if(!m_noteFiltering         .processNote( _n )) return;
    if(!m_noteHumanizing        .processNote( _n )) return;
    if(!m_noteStacking          .processNote( _n )) return;
    if(!m_arpeggio              .processNote( _n )) return;
    if(!m_noteDuplicatesRemoving.processNote( _n )) return;
    */
    for(InstrumentFunction* f: m_noteFunctions)
        if(!f->processNote(_nph))
            return;

    if(_nph->isMasterNote() == false && m_instrument != nullptr)
    {
        // qInfo("InstrumentTrack::play n.key=%d
        // g=%d",_nph->key(),_nph->generation());
        // all is done, so now lets play the note!
        m_instrument->playNote(_nph, _workingBuffer);
    }
    // else qInfo("InstrumentTrack::play SKIP n.key=%d
    // g=%d",_nph->key(),_nph->generation());
}

QString InstrumentTrack::instrumentName() const
{
    return (m_instrument != nullptr) ? m_instrument->displayName()
                                     : QString::null;
}

void InstrumentTrack::deleteNotePluginData(NotePlayHandle* n)
{
    if(m_instrument != nullptr)
        m_instrument->deleteNotePluginData(n);
}

void InstrumentTrack::setName(const QString& _newName)
{
    // when changing name of track, also change name of those patterns,
    // which have the same name as the instrument-track
    // for(int i = 0; i < numOfTCOs(); ++i)
    for(Tile* t: getTCOs())
    {
        Pattern* p = dynamic_cast<Pattern*>(t);  // getTCO(i));
        if((p != nullptr && p->name() == name()) || p->name() == "")
            p->setName(_newName);
    }

    Track::setName(_newName);
    m_midiPort.setName(name());
    m_audioPort->setName(name());

    emit nameChanged();
}

void InstrumentTrack::toggleFrozen()
{
    // qInfo("InstrumentTrack::updateFrozenBuffer ap=%p",&m_audioPort);
    const Song*   song = Engine::song();
    const float   fpt  = Engine::framesPerTick();
    const f_cnt_t len  = song->ticksPerTact() * song->length() * fpt;
    m_audioPort->updateFrozenBuffer(len);
}

void InstrumentTrack::cleanFrozenBuffer()
{
    // qInfo("InstrumentTrack::cleanFrozenBuffer");
    const Song*   song = Engine::song();
    const float   fpt  = Engine::framesPerTick();
    const f_cnt_t len  = song->ticksPerTact() * song->length() * fpt;
    m_audioPort->cleanFrozenBuffer(len);
}

void InstrumentTrack::readFrozenBuffer()
{
    // qInfo("InstrumentTrack::readFrozenBuffer");
    m_audioPort->readFrozenBuffer(uuid());
}

void InstrumentTrack::writeFrozenBuffer()
{
    // qInfo("InstrumentTrack::writeFrozenBuffer");
    m_audioPort->writeFrozenBuffer(uuid());
}

bool InstrumentTrack::isKeyPressed(int _key) const
{
    return (_key >= 0 && _key < NumMidiKeys
            && (m_notes[_key] != nullptr || m_runningMidiNotes[_key] > 0));
}

void InstrumentTrack::updateBaseNote()
{
    /*
    for(NotePlayHandleList::Iterator it = m_processHandles.begin();
        it != m_processHandles.end(); ++it)
        (*it)->setFrequencyUpdate();
    */
    m_processHandles.map([](NotePlayHandle* nph) {
        if(!nph->isFinished())
            nph->setFrequencyUpdate();
    });
}

void InstrumentTrack::updateVolume()
{
    int v = 127;

    if(!m_volumeEnabledModel.value())
        v = midiVolume();
    // if(v != 127)
    //    qInfo("InstrumentTrack::updateVolume %d", v);
    /*
    processOutEvent(MidiEvent(MidiControlChange,
                              midiPort()->outputChannel() - 1,
                              MidiControllerMainVolume, v));
    */
    m_midiCCModel[MidiControllerMainVolume]->setValue(v);
}

void InstrumentTrack::updatePanning()
{
    int p = 64;

    if(!m_panningEnabledModel.value())
        p = midiPanning();
    // if(p != 64)
    //    qInfo("InstrumentTrack::updatePanning %d", p);
    /*
    processOutEvent(MidiEvent(MidiControlChange,
                              midiPort()->outputChannel() - 1,
                              MidiControllerPan, p));
    */
    m_midiCCModel[MidiControllerPan]->setValue(p);
}

void InstrumentTrack::updateMidiCC()
{
    QObject* o = sender();
    if(o == nullptr)
    {
        qInfo("InstrumentTrack::updateMidiCC no sender");
    }
    else
    {
        for(int i = 0; i < MidiControllerCount; i++)
            if(m_midiCCModel[i] == o)
            {
                updateMidiCC(i);
                break;
            }
    }
}

void InstrumentTrack::updateMidiCC(const uint8_t _cc)
{
    const uint8_t v = m_midiCCModel[_cc]->value();
    // qInfo("InstrumentTrack::updateMidiCC cc=%d v=%d", _cc, v);
    processOutEvent(MidiEvent(MidiControlChange,
                              midiPort()->outputChannel() - 1, _cc, v));
}

void InstrumentTrack::updateBending()
{
    updateBaseNote();

    int b = 8192;
    if(!m_bendingEnabledModel.value())
        b = midiBending();
    // if(b != 8192)
    //    qInfo("InstrumentTrack::updateBending %d", b);
    processOutEvent(MidiEvent(MidiPitchBend, midiPort()->outputChannel() - 1,
                              b & 0x7F, (b >> 7) & 0x7F));
}

void InstrumentTrack::updateBendingRange()
{
    const int r = m_bendingRangeModel.value();
    m_bendingModel.setRange(MinPitchDefault * r, MaxPitchDefault * r);

    processOutEvent(MidiEvent(MidiControlChange,
                              midiPort()->outputChannel() - 1,
                              MidiControllerRegisteredParameterNumberLSB,
                              MidiPitchBendSensitivityRPN & 0x7F));
    processOutEvent(MidiEvent(MidiControlChange,
                              midiPort()->outputChannel() - 1,
                              MidiControllerRegisteredParameterNumberMSB,
                              (MidiPitchBendSensitivityRPN >> 8) & 0x7F));
    processOutEvent(MidiEvent(MidiControlChange,
                              midiPort()->outputChannel() - 1,
                              MidiControllerDataEntry, midiBendingRange()));
}

void InstrumentTrack::updateEffectChannel()
{
    m_audioPort->setNextFxChannel(m_effectChannelModel.value());
}

/*
int InstrumentTrack::masterKey(int _midiKey) const
{
    int key = _midiKey - (baseNote() - DefaultKey);
    //if(key < 0) key = 12 + key % 12;
    //if(key > 127) key = 120 + key % 12;
    return qBound(0, key, NumMidiKeys - 1);
}
*/

void InstrumentTrack::removeMidiPortNode(DataFile& _dataFile)
{
    QDomNodeList n = _dataFile.elementsByTagName("midiport");
    n.item(0).parentNode().removeChild(n.item(0));
}

int InstrumentTrack::midiVolume() const
{
    const real_t p = (m_volumeModel.value() / 100.)
                     * (m_noteVolumeModel.value() / 100.);
    int r = MidiMinVolume
            + (MidiMaxVolume - MidiMinVolume) * bound(0., p, 1.);
    // qInfo("InstrumentTrack::midiVolume %f %d", p, r);
    return r;
}

int InstrumentTrack::midiPanning() const
{
    // qInfo("InstrumentTrack::midiPanning models instr=%f note=%f",
    //  m_panningModel.value(), m_notePanningModel.value());

    real_t p = (m_panningModel.value() + m_notePanningModel.value()) / 100.;
    p        = 1. - (1. + p) / 2.;
    int r    = MidiMinPanning
            + (MidiMaxPanning - MidiMinPanning) * bound(0., p, 1.);
    // qInfo("InstrumentTrack::midiPanning %f %d", p, r);
    return r;
}

int InstrumentTrack::midiBending() const
{
    const real_t p
            = ((m_bendingModel.value() + (m_bendingModel.range() + 1) / 2.)
               + 100. * m_noteBendingModel.value())
              / m_bendingModel.range();
    int r = MidiMinPitchBend
            + (MidiMaxPitchBend - MidiMinPitchBend) * bound(0., p, 1.);
    // qInfo("InstrumentTrack::midiBending %f %d",p,r);
    return r;
}

bool InstrumentTrack::play(const MidiTime& _start,
                           const fpp_t     _frames,
                           const f_cnt_t   _offset,
                           int             _bb)
{
    if(m_instrument == nullptr || !tryLockTrack())
        return false;

    const Song*  song = Engine::song();
    const real_t FPT  = Engine::framesPerTick();

    // qInfo("InstrumentTrack::play
    // exporting=%d",Engine::song()->isExporting());
    if(isFrozen() && (song->playMode() == Song::Mode_PlaySong)
       && song->isPlaying() && !Engine::song()->isExporting())
    {
        // const f_cnt_t fstart=_start.getTicks()*FPT;
        // qInfo("InstrumentTrack::play FROZEN f=%d",fstart);
        // silenceAllNotes(true);
        unlockTrack();
        return true;
    }

    MidiTime   start = _start;
    Tiles      tcos;
    ::BBTrack* bb_track = nullptr;
    if(_bb >= 0)
    {
        Tile* tco = tileForBB(_bb);  // getTCO(_bb);
        tcos.append(tco);
        if(trackContainer() == (TrackContainer*)Engine::getBBTrackContainer())
            bb_track = BBTrack::findBBTrack(_bb);
    }
    else
    {
        int n = currentLoop();
        if(song->playMode() == Song::Mode_PlaySong)
        {
            TimeLineWidget* tl = song->getPlayPos().m_timeLine;
            if(tl != nullptr)
                start = tl->adjustTime(start, n);
        }
        getTCOsInRange(tcos, start,
                       start + tick_t(ceilf(float(_frames) / FPT)));
    }

    // Handle automation: detuning
    /*
    for(NotePlayHandleList::Iterator it = m_processHandles.begin();
        it != m_processHandles.end(); ++it)
        (*it)->processMidiTime(start);
    */
    m_processHandles.map([start](NotePlayHandle* nph) {
        if(!nph->isFinished())
            nph->processMidiTime(start);
    });

    if(tcos.size() == 0)
    {
        unlockTrack();
        return false;
    }

    bool played_a_note = false;  // will be return variable

    if(!isMuted())  // test with isMuted GDX
        for(Tiles::Iterator it = tcos.begin(); it != tcos.end(); ++it)
        {
            Pattern* p = qobject_cast<Pattern*>(*it);
            // everything which is not a pattern or muted won't be played
            if(p == nullptr || p->isMuted())
                continue;

            tick_t ul = p->unitLength();
            if(ul == 0)
                continue;

            // MidiTime
            tick_t cur_start = start;
            if(_bb < 0)
                cur_start -= p->startPosition();

            tick_t real_start = cur_start;
            if(p->autoRepeat())
                cur_start %= ul;

            // get all notes from the given pattern...
            const Notes& notes = p->notes();
            // ...and set our index to zero
            Notes::ConstIterator nit = notes.begin();

            // very effective algorithm for playing notes that are
            // posated within the current sample-frame

            if(cur_start > 0)
            {
                // skip notes which are posated before start-tact
                while(nit != notes.end() && (*nit)->pos() < cur_start)
                    ++nit;
            }

            Note* cur_note;
            while(nit != notes.end() && (cur_note = *nit)->pos() == cur_start)
            {
                if(real_start >= p->length())
                {
                    ++nit;
                    continue;
                }

                const f_cnt_t note_frames = cur_note->length().frames(FPT);

                NotePlayHandle* nph = NotePlayHandleManager::acquire(
                        this, _offset, note_frames, *cur_note);
                nph->setBBTrack(bb_track);

                /*
                qInfo("IT rs=%d cs=%d np=%d nl=%d ne=%d pl=%d", real_start,
                      cur_start, nph->pos().getTicks(),
                      nph->length().getTicks(),
                      (nph->pos() + nph->length()).getTicks(),
                      p->length().getTicks());
                */
                if(real_start + nph->length() > p->length())
                {
                    nph->setLength(p->length() - real_start);
                    nph->setFrames(
                            MidiTime(p->length() - real_start).frames(FPT));
                }

                // are we playing global song?
                if(_bb < 0)
                {
                    // then set song-global offset of pattern in order to
                    // properly perform the note detuning
                    nph->setSongGlobalParentOffset(p->startPosition());
                }

                // Engine::mixer()->addPlayHandle(nph);
                Engine::mixer()->emit playHandleToAdd(nph->pointer());
                played_a_note = true;
                ++nit;
            }
        }

    unlockTrack();
    return played_a_note;
}

Tile* InstrumentTrack::createTCO()
{
    return new Pattern(this);
}

TrackView* InstrumentTrack::createView(TrackContainerView* tcv)
{
    return new InstrumentTrackView(this, tcv);
}

void InstrumentTrack::saveTrackSpecificSettings(QDomDocument& doc,
                                                QDomElement&  thisElement)
{
    m_volumeModel.saveSettings(doc, thisElement, "vol");
    m_panningModel.saveSettings(doc, thisElement, "pan");
    m_bendingModel.saveSettings(doc, thisElement, "pitch");
    m_bendingRangeModel.saveSettings(doc, thisElement, "pitchrange");

    m_volumeEnabledModel.saveSettings(doc, thisElement, "cc_volume");
    m_panningEnabledModel.saveSettings(doc, thisElement, "cc_panning");
    m_bendingEnabledModel.saveSettings(doc, thisElement, "cc_bending");

    m_effectChannelModel.saveSettings(doc, thisElement, "fxch");
    m_baseNoteModel.saveSettings(doc, thisElement, "basenote");
    m_useMasterPitchModel.saveSettings(doc, thisElement, "usemasterpitch");

    if(m_scale != nullptr)
        thisElement.setAttribute("scale",
                                 m_scale->bank() * 1000 + m_scale->index());

    m_rootModel.saveSettings(doc, thisElement, "root");
    m_modeModel.saveSettings(doc, thisElement, "mode");

    if(m_instrument != nullptr)
    {
        QDomElement i = doc.createElement("instrument");
        i.setAttribute("name", m_instrument->descriptor()->name());
        m_instrument->saveState(doc, i);
        thisElement.appendChild(i);
    }

    m_soundShaping.saveState(doc, thisElement);

    /*
      m_noteFiltering         .saveState( doc, thisElement );
      m_noteHumanizing        .saveState( doc, thisElement );
      m_noteStacking          .saveState( doc, thisElement );
      m_arpeggio              .saveState( doc, thisElement );
      m_noteDuplicatesRemoving.saveState( doc, thisElement );
    */
    for(InstrumentFunction* f: m_noteFunctions)
        f->saveState(doc, thisElement);

    m_midiPort.saveState(doc, thisElement);
    m_audioPort->effects()->saveState(doc, thisElement);
}

void InstrumentTrack::loadTrackSpecificSettings(
        const QDomElement& thisElement)
{
    silenceAllNotes();  // true

    lockTrack();

    m_volumeModel.loadSettings(thisElement, "vol");
    m_panningModel.loadSettings(thisElement, "pan");
    m_bendingRangeModel.loadSettings(thisElement, "pitchrange");
    m_bendingModel.loadSettings(thisElement, "pitch");

    m_volumeEnabledModel.loadSettings(thisElement, "cc_volume");
    m_panningEnabledModel.loadSettings(thisElement, "cc_panning");
    m_bendingEnabledModel.loadSettings(thisElement, "cc_bending");

    m_effectChannelModel.setRange(0, Engine::fxMixer()->numChannels() - 1);
    // if(!m_previewMode)
    m_effectChannelModel.loadSettings(thisElement, "fxch");

    m_baseNoteModel.loadSettings(thisElement, "basenote");
    m_useMasterPitchModel.loadSettings(thisElement, "usemasterpitch");

    if(thisElement.hasAttribute("scale"))
    {
        qulonglong bi = thisElement.attribute("scale").toULongLong();
        setScale(Scale::get(bi / 1000, bi % 1000));
    }

    m_rootModel.loadSettings(thisElement, "root");
    m_modeModel.loadSettings(thisElement, "mode");

    // clear effect-chain just in case we load an old preset without
    // FX-data
    m_audioPort->effects()->clear();

    QStringList errors;

    QDomNode node = thisElement.firstChild();
    while(!node.isNull())
    {
        // qInfo("IT: nodename=%s", qPrintable(node.nodeName()));

        if(node.isElement())
        {
            bool found = false;
            for(InstrumentFunction* f: m_noteFunctions)
                if(f->nodeName() == node.nodeName())
                {
                    f->restoreState(node.toElement());
                    found = true;
                    break;
                }
            if(found)
            {
                node = node.nextSibling();
                continue;
            }

            if(m_soundShaping.nodeName() == node.nodeName())
            {
                // qInfo("InstrumentTrack: restore sound shaping before");
                m_soundShaping.restoreState(node.toElement());
                // qInfo("InstrumentTrack: restore sound shaping after");
            }
            /*
            else if( m_noteFiltering.nodeName() == node.nodeName() )
            {
                    m_noteFiltering.restoreState( node.toElement() );
            }
            else if( m_noteHumanizing.nodeName() == node.nodeName() )
            {
                    m_noteHumanizing.restoreState( node.toElement() );
            }
            else if( m_noteStacking.nodeName() == node.nodeName() )
            {
                    m_noteStacking.restoreState( node.toElement() );
            }
            else if( m_arpeggio.nodeName() == node.nodeName() )
            {
                    m_arpeggio.restoreState( node.toElement() );
            }
            else if( m_noteDuplicatesRemoving.nodeName() ==
            node.nodeName() )
            {
                    m_noteDuplicatesRemoving.restoreState(
            node.toElement() );
            }
            */
            else if(m_midiPort.nodeName() == node.nodeName())
            {
                m_midiPort.restoreState(node.toElement());
            }
            else if(m_audioPort->effects()->nodeName() == node.nodeName())
            {
                m_audioPort->effects()->restoreState(node.toElement());
            }
            else if(node.nodeName() == "instrument")
            {
                delete m_instrument;
                m_instrument  = nullptr;
                QString iname = node.toElement().attribute("name");
                m_instrument  = Instrument::instantiate(iname, this);
                if(m_instrument != nullptr)
                {
                    QDomElement ielem = node.firstChildElement();
                    if(ielem.tagName() == "freeboy")
                        ielem.setTagName("papu");
                    // qInfo("InstrumentTrack: restore state before");
                    m_instrument->restoreState(ielem);
                    // qInfo("InstrumentTrack: restore state after 1");
                    emit instrumentChanged();
                    // qInfo("InstrumentTrack: restore state after 2");
                }
                else
                {
                    qWarning(
                            "InstrumentTrack::loadTrackSpecificSettings"
                            " fail to load '%s'",
                            qPrintable(iname));
                }
            }
            // compat code - if node-name doesn't match any known
            // one, we assume that it is an instrument-plugin
            // which we'll try to load
            else if(AutomationPattern::classNodeName() != node.nodeName()
                    && ControllerConnection::classNodeName()
                               != node.nodeName()
                    && !node.toElement().hasAttribute("id"))
            {
                /*
                  delete m_instrument;
                  m_instrument = nullptr;
                  m_instrument = Instrument::instantiate(node.nodeName(),
                  this); if(m_instrument->nodeName() == node.nodeName())
                  {
                  m_instrument->restoreState(node.toElement());
                  }
                  emit instrumentChanged();
                */

                qWarning(
                        "Notice: unknown tag '%s': trying loading as a "
                        "plugin",
                        qPrintable(node.nodeName()));

                Plugin* p = Plugin::instantiate(node.nodeName(), this, this,
                                                false);
                if(dynamic_cast<Instrument*>(p) != nullptr
                   && p->nodeName() == node.nodeName())
                {
                    // qInfo("InstrumentTrack: restore state after 3a");
                    delete m_instrument;
                    m_instrument = dynamic_cast<Instrument*>(p);
                    // qInfo("InstrumentTrack: restore state after 3b");
                    m_instrument->restoreState(node.toElement());
                    // qInfo("InstrumentTrack: restore state after 3c");
                    emit instrumentChanged();
                    // qInfo("InstrumentTrack: restore state after 3d");
                }
                else if(dynamic_cast<InstrumentFunction*>(p) != nullptr
                        && p->nodeName() == node.nodeName())
                {
                    InstrumentFunction* f
                            = dynamic_cast<InstrumentFunction*>(p);
                    f->restoreState(node.toElement());
                    m_noteFunctions.append(f);
                    emit instrumentFunctionAdded(f);
                }
                else
                {
                    errors.append(node.nodeName());
                    delete p;
                }
            }
        }
        node = node.nextSibling();
    }

    // qInfo("InstrumentTrack: restore state after 4");
    updateBendingRange();
    unlockTrack();
    // BACKTRACE
    // qInfo("InstrumentTrack: restore state after 5");

    if(!errors.isEmpty())
    {
        errors.removeDuplicates();
        QString s = errors.join("\n");
        if(gui)
        {
            QMessageBox::information(
                    nullptr, tr("Plugins not found"),
                    tr("Some plugins were not found:\n%1").arg(s),
                    QMessageBox::Ok | QMessageBox::Default);
        }
        else
        {
            qWarning("Some plugins were not found:\n%s", qPrintable(s));
        }
    }
}

/*
void InstrumentTrack::setPreviewMode(const bool value)
{
    m_previewMode = value;
}
*/

Instrument* InstrumentTrack::loadInstrument(const QString& _pluginName)
{
    silenceAllNotes();  // true

    lockTrack();
    delete m_instrument;
    qInfo("InstrumentTrack::loadInstrument loading '%s'",
          qPrintable(_pluginName));
    m_instrument = Instrument::instantiate(_pluginName, this);
    unlockTrack();
    // setName( m_instrument->displayName() );
    setName(defaultName());

    emit instrumentChanged();

    const bool nmb = m_instrument == nullptr || !m_instrument->isMidiBased();
    m_volumeEnabledModel.setValue(nmb);
    m_panningEnabledModel.setValue(nmb);

    return m_instrument;
}

const Scale* InstrumentTrack::scale() const
{
    return m_scale;
}

void InstrumentTrack::setScale(const Scale* _scale)
{
    if(m_scale != _scale)
        m_scale = _scale;
}

// #### ITV:

// QQueue<InstrumentTrackWindow*> InstrumentTrackView::s_windowCache;

InstrumentTrackView::InstrumentTrackView(InstrumentTrack*    _it,
                                         TrackContainerView* _tcv) :
      TrackView(_it, _tcv),
      m_window(nullptr), m_lastPos(-1, -1)
{
    setAcceptDrops(true);
    setFixedHeight(32);

    qInfo("InstrumentTrackView::InstrumentTrackView 1");
    m_tlb = new TrackLabelButton(this, getTrackSettingsWidget());
    m_tlb->setCheckable(true);
    m_tlb->setIcon(embed::getIcon("instrument_track"));
    m_tlb->move(3, 1);
    m_tlb->show();

    connect(m_tlb, SIGNAL(toggled(bool)), this,
            SLOT(toggleInstrumentWindow(bool)));

    connect(_it, SIGNAL(nameChanged()), m_tlb, SLOT(update()));

    // creation of widgets for track-settings-widget
    int widgetWidth;
    if(ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt())
        widgetWidth = DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT;
    else
        widgetWidth = DEFAULT_SETTINGS_WIDGET_WIDTH;

    qInfo("InstrumentTrackView::InstrumentTrackView 2");
    m_volumeKnob
            = new VolumeKnob(getTrackSettingsWidget());  //, tr("Volume"));
    // m_volumeKnob->setVolumeKnob(true);
    m_volumeKnob->setModel(&_it->m_volumeModel);
    // m_volumeKnob->setHintText(tr("Volume:"), "%");
    m_volumeKnob->setText("");
    m_volumeKnob->move(widgetWidth - 2 * 29, 3);  // 24,2
    m_volumeKnob->show();
    m_volumeKnob->setWhatsThis(tr(ITVOLHELP));

    m_panningKnob = new Knob(getTrackSettingsWidget(), tr("Panning"));
    m_panningKnob->setModel(&_it->m_panningModel);
    m_panningKnob->setHintText(tr("Panning:"), "%");
    m_panningKnob->move(widgetWidth - 29, 3);  // 24,2
    m_panningKnob->setPointColor(Qt::magenta);
    m_panningKnob->show();

    qInfo("InstrumentTrackView::InstrumentTrackView 3");
    m_midiMenu = new QMenu(tr("MIDI"), this);

    // sequenced MIDI?
    if(!Engine::mixer()->midiClient()->isRaw())
    {
        m_midiInputAction = m_midiMenu->addMenu(
                new MidiPortMenu(&_it->m_midiPort, MidiPort::Input));
        m_midiOutputAction = m_midiMenu->addMenu(
                new MidiPortMenu(&_it->m_midiPort, MidiPort::Output));
    }
    else
    {
        m_midiInputAction  = m_midiMenu->addAction("");
        m_midiOutputAction = m_midiMenu->addAction("");
        m_midiInputAction->setCheckable(true);
        m_midiOutputAction->setCheckable(true);
        connect(m_midiInputAction, SIGNAL(changed()), this,
                SLOT(midiInSelected()));
        connect(m_midiOutputAction, SIGNAL(changed()), this,
                SLOT(midiOutSelected()));
        connect(&_it->m_midiPort, SIGNAL(modeChanged()), this,
                SLOT(midiConfigChanged()));
    }

    m_midiInputAction->setText(tr("Input"));
    m_midiOutputAction->setText(tr("Output"));

    qInfo("InstrumentTrackView::InstrumentTrackView 4");
    m_activityIndicator
            = new FadeButton(QApplication::palette().color(
                                     QPalette::Active, QPalette::Background),
                             QApplication::palette().color(
                                     QPalette::Active, QPalette::BrightText),
                             getTrackSettingsWidget());
    m_activityIndicator->setGeometry(widgetWidth - 2 * 29 - 11, 2, 8,
                                     28);  // 24
    m_activityIndicator->show();
    connect(m_activityIndicator, SIGNAL(pressed()), this,
            SLOT(activityIndicatorPressed()));
    connect(m_activityIndicator, SIGNAL(released()), this,
            SLOT(activityIndicatorReleased()));
    connect(_it, SIGNAL(newNote()), m_activityIndicator, SLOT(activate()));
    connect(&_it->m_mutedModel, SIGNAL(dataChanged()), this,
            SLOT(muteChanged()));

    qInfo("InstrumentTrackView::InstrumentTrackView 5");
    // setModel(_it);
}

InstrumentTrackView::~InstrumentTrackView()
{
    qInfo("InstrumentTrackView::~InstrumentTrackView START");
    disconnect(m_activityIndicator, SIGNAL(pressed()), this,
               SLOT(activityIndicatorPressed()));
    disconnect(m_activityIndicator, SIGNAL(released()), this,
               SLOT(activityIndicatorReleased()));

    InstrumentTrack* m = model();
    if(m != nullptr)
    {
        disconnect(m, SIGNAL(newNote()), m_activityIndicator,
                   SLOT(activate()));
        disconnect(&m->m_mutedModel, SIGNAL(dataChanged()), this,
                   SLOT(muteChanged()));
    }

    freeInstrumentTrackWindow();
    /*
    qInfo("InstrumentTrackView::~InstrumentTrackView 1");
    InstrumentTrack* m = model();
    if(m == nullptr)
        qCritical("InstrumentTrackView::~InstrumentTrackView null model");
    else
    {
        // delete m->m_midiPort.m_readablePortsMenu;
        // delete m->m_midiPort.m_writablePortsMenu;
    }
    */
    qInfo("InstrumentTrackView::~InstrumentTrackView END");
}

InstrumentTrackWindow* InstrumentTrackView::topLevelInstrumentTrackWindow()
{
    InstrumentTrackWindow* itw = nullptr;
    for(const QMdiSubWindow* sw:
        gui->mainWindow()->workspace()->subWindowList(
                QMdiArea::ActivationHistoryOrder))
    {
        itw = dynamic_cast<InstrumentTrackWindow*>(sw->widget());
        if(sw->isVisible() && itw != nullptr)
            break;
    }

    return itw;
}

/*! \brief Create and assign a new FX Channel for this track */
void InstrumentTrackView::createFxLine()
{
    InstrumentTrack* m = model();

    int channelIndex = gui->fxMixerView()->addNewChannel();
    Engine::fxMixer()->effectChannel(channelIndex)->name() = m->name();
    assignFxLine(channelIndex);
}

/*! \brief Assign a specific FX Channel for this track */
void InstrumentTrackView::assignFxLine(int channelIndex)
{
    // qInfo("InstrumentTrackView::assignFxLine %d",channelIndex);
    InstrumentTrack* m = model();
    m->effectChannelModel()->setRange(0,
                                      Engine::fxMixer()->numChannels() - 1);
    m->effectChannelModel()->setValue(channelIndex);
    gui->fxMixerView()->setCurrentLine(channelIndex);
}

// TODO: Add windows to free list on freeInstrumentTrackWindow.
// But, don't nullptr m_window or disconnect signals.  This will allow
// windows that are being show/hidden frequently to stay connected.
void InstrumentTrackView::freeInstrumentTrackWindow()
{
    if(m_window != nullptr)
    {
        m_lastPos = m_window->parentWidget()->pos();
        /*
        if(ConfigManager::inst()
                   ->value("ui", "oneinstrumenttrackwindow")
                   .toInt()
           || s_windowCache.count() < INSTRUMENT_WINDOW_CACHE_SIZE)
        {
            model()->setHook(nullptr);
            m_window->setInstrumentTrackView(nullptr);
            m_window->parentWidget()->hide();
            // m_window->setModel(
            //	engine::dummyTrackContainer()->
            //			dummyInstrumentTrack() );
            m_window->updateInstrumentView();
            s_windowCache << m_window;
        }
        else
        */
        {
            delete m_window;
        }

        m_window = nullptr;
    }
}

/*
void InstrumentTrackView::cleanupWindowCache()
{
    while(!s_windowCache.isEmpty())
    {
        delete s_windowCache.dequeue();
    }
}
*/

InstrumentTrackWindow* InstrumentTrackView::instrumentTrackWindow()
{
    if(m_window != nullptr)
    {
    }
    /*
    else if(!s_windowCache.isEmpty())
    {
        m_window = s_windowCache.dequeue();

        m_window->setInstrumentTrackView(this);
        m_window->setModel(model());
        m_window->updateInstrumentView();
        model()->setHook(m_window);

        if(ConfigManager::inst()
                   ->value("ui", "oneinstrumenttrackwindow")
                   .toInt())
        {
            s_windowCache << m_window;
        }
        else if(m_lastPos.x() > 0 || m_lastPos.y() > 0)
        {
            m_window->parentWidget()->move(m_lastPos);
        }
    }
    */
    else
    {
        m_window = new InstrumentTrackWindow(this);
        /*
        if(ConfigManager::inst()
                   ->value("ui", "oneinstrumenttrackwindow")
                   .toInt())
        {
            // first time, an InstrumentTrackWindow is opened
            s_windowCache << m_window;
        }
        */
    }

    return m_window;
}

void InstrumentTrackView::paintEvent(QPaintEvent* _pe)
{
    TrackView::paintEvent(_pe);
}

void InstrumentTrackView::resizeEvent(QResizeEvent* re)
{
    TrackView::resizeEvent(re);
    /*
      I don't how to vertical align the content to the top
      m_tlb->setFixedHeight(height()-2);
    */
}

void InstrumentTrackView::dragEnterEvent(QDragEnterEvent* _dee)
{
    instrumentTrackWindow()->dragEnterEvent(_dee);
    if(!_dee->isAccepted())
        TrackView::dragEnterEvent(_dee);
}

void InstrumentTrackView::dropEvent(QDropEvent* _de)
{
    instrumentTrackWindow()->dropEvent(_de);
    if(!_de->isAccepted())
        TrackView::dropEvent(_de);
}

void InstrumentTrackView::toggleInstrumentWindow(bool _on)
{
    instrumentTrackWindow()->toggleVisibility(_on);

    if(!_on)
        freeInstrumentTrackWindow();
}

void InstrumentTrackView::activityIndicatorPressed()
{
    InstrumentTrack* m = model();
    m->processInEvent(
            MidiEvent(MidiNoteOn, 0, DefaultKey, MidiDefaultVelocity));
}

void InstrumentTrackView::activityIndicatorReleased()
{
    InstrumentTrack* m = model();
    m->processInEvent(MidiEvent(MidiNoteOff, 0, DefaultKey, 0));
}

void InstrumentTrackView::midiInSelected()
{
    InstrumentTrack* m = model();
    m->m_midiPort.setReadable(m_midiInputAction->isChecked());
}

void InstrumentTrackView::midiOutSelected()
{
    InstrumentTrack* m = model();
    m->m_midiPort.setWritable(m_midiOutputAction->isChecked());
}

void InstrumentTrackView::midiConfigChanged()
{
    InstrumentTrack* m = model();
    m_midiInputAction->setChecked(m->m_midiPort.isReadable());
    m_midiOutputAction->setChecked(m->m_midiPort.isWritable());
}

void InstrumentTrackView::muteChanged()
{
    InstrumentTrack* m = model();

    if(m != nullptr && m->m_mutedModel.value())
    {
        m_activityIndicator->setActiveColor(QApplication::palette().color(
                QPalette::Active, QPalette::Highlight));
    }
    else
    {
        m_activityIndicator->setActiveColor(QApplication::palette().color(
                QPalette::Active, QPalette::BrightText));
    }
}

QMenu* InstrumentTrackView::createAudioInputMenu()
{
    QString title = tr("Audio Input (not implemented)");
    //.arg( channelIndex ).arg( fxChannel->name() );
    QMenu* fxMenu = new QMenu(title);
    fxMenu->setEnabled(false);
    return fxMenu;
}

QMenu* InstrumentTrackView::createAudioOutputMenu()
{
    InstrumentTrack* m = model();

    int        channelIndex = m->effectChannelModel()->value();
    FxChannel* fxChannel    = Engine::fxMixer()->effectChannel(channelIndex);

    QString title = tr("Audio Output (%1:%2)")
                            .arg(channelIndex)
                            .arg(fxChannel->name());
    QMenu* fxMenu = new QMenu(title);

    QSignalMapper* fxMenuSignalMapper = new QSignalMapper(fxMenu);

    QString newFxLabel = tr("Assign to new channel");
    fxMenu->addAction(newFxLabel, this, SLOT(createFxLine()));
    // fxMenu->addSeparator();
    for(int i = 0; i < Engine::fxMixer()->numChannels(); ++i)
    {
        FxChannel* ch = Engine::fxMixer()->effectChannel(i);

        QString label
                = tr("Mixer %1:%2").arg(ch->channelIndex()).arg(ch->name());
        QAction* action
                = fxMenu->addAction(label, fxMenuSignalMapper, SLOT(map()));
        action->setEnabled(ch != fxChannel);
        fxMenuSignalMapper->setMapping(action, ch->channelIndex());
        if(ch->channelIndex() != i)
            qWarning(
                    "InstrumentTrackView::createAudioOutputMenu "
                    "suspicious "
                    "ch: %d %d",
                    ch->channelIndex(), i);
    }

    connect(fxMenuSignalMapper, SIGNAL(mapped(int)), this,
            SLOT(assignFxLine(int)));

    return fxMenu;
}

QMenu* InstrumentTrackView::createMidiInputMenu()
{
    InstrumentTrack* m = model();
    // InstrumentTrack* it = dynamic_cast<InstrumentTrack*>(track());

    // sequenced MIDI?
    if(!Engine::mixer()->midiClient()->isRaw())
    {
        MidiPortMenu* r = new MidiPortMenu(&m->m_midiPort, MidiPort::Input);
        r->setTitle(tr("MIDI Input"));
        r->updateMenu();
        return r;
    }

    return new QMenu("N/A");
}

QMenu* InstrumentTrackView::createMidiOutputMenu()
{
    InstrumentTrack* m = model();
    // InstrumentTrack* it = dynamic_cast<InstrumentTrack*>(track());

    // sequenced MIDI?
    if(!Engine::mixer()->midiClient()->isRaw())
    {
        MidiPortMenu* r = new MidiPortMenu(&m->m_midiPort, MidiPort::Output);
        r->setTitle(tr("MIDI Output"));
        r->updateMenu();
        return r;
    }

    return new QMenu("N/A");
}

QMenu* InstrumentTrackView::createFxMenu(QString title, QString newFxLabel)
{
    InstrumentTrack* m = model();

    int        channelIndex = m->effectChannelModel()->value();
    FxChannel* fxChannel    = Engine::fxMixer()->effectChannel(channelIndex);

    // If title allows interpolation, pass channel index and name
    if(title.contains("%2"))
        title = title.arg(channelIndex).arg(fxChannel->name());

    QMenu*         fxMenu             = new QMenu(title);
    QSignalMapper* fxMenuSignalMapper = new QSignalMapper(fxMenu);

    fxMenu->addAction(newFxLabel, this, SLOT(createFxLine()));
    fxMenu->addSeparator();

    for(int i = 0; i < Engine::fxMixer()->numChannels(); ++i)
    {
        FxChannel* ch = Engine::fxMixer()->effectChannel(i);

        if(ch != fxChannel)
        {
            QString label
                    = tr("FX %1: %2").arg(ch->channelIndex()).arg(ch->name());
            QAction* action = fxMenu->addAction(label, fxMenuSignalMapper,
                                                SLOT(map()));
            fxMenuSignalMapper->setMapping(action, ch->channelIndex());
        }
    }

    connect(fxMenuSignalMapper, SIGNAL(mapped(int)), this,
            SLOT(assignFxLine(int)));

    return fxMenu;
}

void InstrumentTrackView::addSpecificMenu(QMenu* _cm, bool _enabled)
{
    _cm->addMenu(createAudioInputMenu());
    _cm->addMenu(createAudioOutputMenu());
    _cm->addMenu(createMidiInputMenu());
    _cm->addMenu(createMidiOutputMenu());

    // Old MIDI menu
    // _cm->addSeparator();
    // _cm->addMenu(trackView->midiMenu());
}

// #### ITW:
InstrumentTrackWindow::InstrumentTrackWindow(InstrumentTrackView* _itv) :
      QWidget(), ModelView(_itv->model(), this),
      // m_track(_itv->model()),
      m_itv(_itv), m_instrumentView(nullptr)
{
    allowNullModel(true);
    allowModelChange(true);
    setAcceptDrops(true);

    InstrumentTrack* m = model();

    // init own layout + widgets
    setFocusPolicy(Qt::StrongFocus);
    QVBoxLayout* vlayout = new QVBoxLayout(this);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);

    TabWidget* generalSettingsWidget
            = new TabWidget(tr("GENERAL SETTINGS"), this);

    QVBoxLayout* generalSettingsLayout
            = new QVBoxLayout(generalSettingsWidget);

    generalSettingsLayout->setContentsMargins(6, 18, 6, 6);
    generalSettingsLayout->setSpacing(6);

    QWidget* nameAndChangeTrackWidget = new QWidget(generalSettingsWidget);
    QHBoxLayout* nameAndChangeTrackLayout
            = new QHBoxLayout(nameAndChangeTrackWidget);
    nameAndChangeTrackLayout->setContentsMargins(0, 0, 0, 0);
    nameAndChangeTrackLayout->setSpacing(6);

    // setup line edit for changing instrument track name
    m_nameLineEdit = new QLineEdit();
    m_nameLineEdit->setFont(pointSize<9>(m_nameLineEdit->font()));
    connect(m_nameLineEdit, SIGNAL(textChanged(const QString&)), this,
            SLOT(textChanged(const QString&)));

    m_nameLineEdit->setSizePolicy(
            QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    nameAndChangeTrackLayout->addWidget(m_nameLineEdit);

    // set up left/right arrows for changing instrument
    /*
    m_leftRightNav = new LeftRightNav(this);
    connect(m_leftRightNav, SIGNAL(onNavLeft()), this,
            SLOT(viewPrevInstrument()));
    connect(m_leftRightNav, SIGNAL(onNavRight()), this,
            SLOT(viewNextInstrument()));
    m_leftRightNav->setWhatsThis(
            tr("Use these controls to view and edit the next/previous
    track " "in the song editor."));

    // m_leftRightNav->setShortcuts();
    nameAndChangeTrackLayout->addWidget(m_leftRightNav);
    nameAndChangeTrackLayout->setAlignment(m_leftRightNav, Qt::AlignTop);
    */

    generalSettingsLayout->addWidget(nameAndChangeTrackWidget);

    QGridLayout* basicControlsLayout = new QGridLayout;
    basicControlsLayout->setHorizontalSpacing(3);
    basicControlsLayout->setVerticalSpacing(0);
    basicControlsLayout->setContentsMargins(0, 0, 0, 0);

    // set up volume knob
    // m_volumeKnob = new Knob(nullptr, tr("Instrument volume"));
    m_volumeKnob = new VolumeKnob(nullptr);
    // m_volumeKnob->setVolumeKnob(true);
    // m_volumeKnob->setText(tr("VOL"));
    // m_volumeKnob->setHintText(tr("Volume:"), "%");
    m_volumeKnob->setWhatsThis(tr(ITVOLHELP));

    // QLabel *label = new QLabel( tr( "VOL" ), this );
    // label->setStyleSheet( labelStyleSheet );
    // basicControlsLayout->addWidget( label, 1, 0);
    // basicControlsLayout->setAlignment( label, labelAlignment );

    // set up panning knob
    m_panningKnob = new Knob(nullptr, tr("Panning"));
    m_panningKnob->setText(tr("PAN"));
    m_panningKnob->setHintText(tr("Panning:"), "");
    m_panningKnob->setPointColor(Qt::magenta);

    // label = new QLabel( tr( "PAN" ), this );
    // label->setStyleSheet( labelStyleSheet );
    // basicControlsLayout->addWidget( label, 1, 1);
    // basicControlsLayout->setAlignment( label, labelAlignment );

    // set up pitch knob
    m_bendingKnob = new Knob(nullptr, tr("Pitch"));
    m_bendingKnob->setPointColor(Qt::cyan);
    m_bendingKnob->setHintText(tr("Pitch:"), " " + tr("cents"));
    m_bendingKnob->setText(tr("PITCH"));

    // m_pitchLabel = new QLabel( tr( "PITCH" ), this );
    // m_pitchLabel->setStyleSheet( labelStyleSheet );
    // basicControlsLayout->addWidget( m_pitchLabel, 1, 3);
    // basicControlsLayout->setAlignment( m_pitchLabel, labelAlignment );

    // set up pitch range knob
    m_bendingRangeSpinBox = new LcdSpinBox(2, "19cyan", nullptr,
                                           tr("Pitch range (semitones)"));
    m_bendingRangeSpinBox->setText(tr("RANGE"));

    // m_bendingRangeLabel = new QLabel( tr( "RANGE" ), this );
    // m_bendingRangeLabel->setStyleSheet( labelStyleSheet );
    // basicControlsLayout->addWidget( m_bendingRangeLabel, 1, 4);
    // basicControlsLayout->setAlignment( m_bendingRangeLabel,
    // labelAlignment
    // );

    // setup spinbox for selecting FX-channel
    m_effectChannelNumber
            = new FxLineLcdSpinBox(2, nullptr, tr("FX channel"));
    m_effectChannelNumber->setText(tr("FX"));

    // label = new QLabel( tr( "FX" ), this );
    // label->setStyleSheet( labelStyleSheet );
    // basicControlsLayout->addWidget( label, 1, 6);
    // basicControlsLayout->setAlignment( label, labelAlignment );

    QPushButton* saveSettingsBtn
            = new QPushButton(embed::getIcon("project_save"), QString());
    saveSettingsBtn->setMinimumSize(30, 30);  // 32, 32 );

    connect(saveSettingsBtn, SIGNAL(clicked()), this,
            SLOT(saveSettingsBtnClicked()));

    ToolTip::add(saveSettingsBtn, tr("Save current instrument track "
                                     "settings in a preset file"));
    saveSettingsBtn->setWhatsThis(
            tr("Click here, if you want to save current instrument track "
               "settings in a preset file. "
               "Later you can load this preset by double-clicking it in the "
               "preset-browser."));

    // QString labelStyleSheet = "font-size: 6pt;";
    // Qt::Alignment labelAlignment = Qt::AlignHCenter | Qt::AlignTop;
    Qt::Alignment widgetAlignment
            = Qt::AlignHCenter | Qt::AlignBottom;  // Center;

    basicControlsLayout->addWidget(m_volumeKnob, 0, 0, widgetAlignment);
    basicControlsLayout->addWidget(m_panningKnob, 0, 1, widgetAlignment);
    basicControlsLayout->addWidget(m_bendingKnob, 0, 2, widgetAlignment);
    basicControlsLayout->addWidget(m_bendingRangeSpinBox, 0, 3,
                                   widgetAlignment);
    basicControlsLayout->addWidget(m_effectChannelNumber, 0, 4,
                                   widgetAlignment);
    basicControlsLayout->setColumnStretch(5, 1);
    basicControlsLayout->addWidget(saveSettingsBtn, 0, 6, widgetAlignment);

    // label = new QLabel( tr( "SAVE" ), this );
    // label->setStyleSheet( labelStyleSheet );
    // basicControlsLayout->addWidget( label, 1, 7);
    // basicControlsLayout->setAlignment( label, labelAlignment );

    generalSettingsLayout->addLayout(basicControlsLayout);

    m_tabWidget = new TabWidget("", this, true);
    m_tabWidget->setFixedHeight(INSTRUMENT_HEIGHT + GRAPHIC_TAB_HEIGHT - 4);

    // create tab-widgets
    m_ssView
            = new InstrumentSoundShapingView(&m->m_soundShaping, m_tabWidget);

    // FUNC tab
    QScrollArea* saFunc = new QScrollArea(m_tabWidget);
    saFunc->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    saFunc->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    saFunc->setFrameStyle(QFrame::NoFrame);

    QWidget*     instrumentFunctions = new QWidget(saFunc);
    QVBoxLayout* instrumentFunctionsLayout
            = new QVBoxLayout(instrumentFunctions);
    instrumentFunctionsLayout->setContentsMargins(2, 3, 2, 5);
    // instrumentFunctionsLayout->addStrut( 230 );

    for(InstrumentFunction* f: m->m_noteFunctions)
    {
        InstrumentFunctionView* v = f->createView();
        m_noteFunctionViews.append(v);
        v->setMinimumWidth(230);
        instrumentFunctionsLayout->addWidget(v);
    }
    /*
    m_noteFilteringView = new InstrumentFunctionNoteFilteringView(
    &m->m_noteFiltering ); m_noteHumanizingView = new
    InstrumentFunctionNoteHumanizingView( &m->m_noteHumanizing );
    m_noteStackingView = new InstrumentFunctionNoteStackingView(
    &m->m_noteStacking ); m_arpeggioView = new
    InstrumentFunctionArpeggioView( &m->m_arpeggio );
    m_noteDuplicatesRemovingView = new
    InstrumentFunctionNoteDuplicatesRemovingView(
    &m->m_noteDuplicatesRemoving );

    m_noteFilteringView->setMinimumWidth(230);
    m_noteHumanizingView->setMinimumWidth(230);
    m_noteStackingView->setMinimumWidth(230);
    m_arpeggioView->setMinimumWidth(230);
    m_noteDuplicatesRemovingView->setMinimumWidth(230);

    instrumentFunctionsLayout->addWidget( m_noteFilteringView );
    instrumentFunctionsLayout->addWidget( m_noteHumanizingView );
    instrumentFunctionsLayout->addWidget( m_noteStackingView );
    instrumentFunctionsLayout->addWidget( m_arpeggioView );
    instrumentFunctionsLayout->addWidget( m_noteDuplicatesRemovingView );
    instrumentFunctionsLayout->addStretch();
    */

    saFunc->setWidget(instrumentFunctions);

    // MIDI tab
    m_midiView = new InstrumentMidiIOView(m, m_tabWidget);

    // FX tab
    m_effectView
            = new EffectChainView(m->m_audioPort->effects(), m_tabWidget);

    // MISC tab
    m_miscView = new InstrumentMiscView(m, m_tabWidget);

    m_tabWidget->addTab(m_ssView, tr("Envelope, filter & LFO"), "env_lfo_tab",
                        1);
    m_tabWidget->addTab(saFunc /*instrumentFunctions*/, tr("Note effects"),
                        "func_tab", 2);
    m_tabWidget->addTab(m_effectView, tr("Effects"), "fx_tab", 3);
    m_tabWidget->addTab(m_midiView, tr("MIDI settings"), "midi_tab", 4);
    m_tabWidget->addTab(m_miscView, tr("Miscellaneous"), "misc_tab", 5);

    vlayout->addWidget(generalSettingsWidget);
    vlayout->addWidget(m_tabWidget);

    // setup piano-widget
    m_peripheralView = new PianoView(&m->m_piano, this);
    // m_peripheralView = new PeripheralPadsView(this);
    m_peripheralView->setFixedSize(INSTRUMENT_WIDTH, PIANO_HEIGHT);
    // m_peripheralView->setModel(&m->m_piano);
    vlayout->addWidget(m_peripheralView);

    // setModel(_itv->model());

    modelChanged();
    updateInstrumentView();

    // setFixedWidth(INSTRUMENT_WIDTH);
    // resize(sizeHint());
    resize(250, 500);

    /*
    QMdiSubWindow * win = gui->mainWindow()->addWindowedWidget( this );
    Qt::WindowFlags flags = win->windowFlags();
    flags |= Qt::MSWindowsFixedSizeDialogHint;
    flags &= ~Qt::WindowMaximizeButtonHint;
    win->setWindowFlags( flags );
    */
    SubWindow* win
            = SubWindow::putWidgetOnWorkspace(this, false, false, false);
    win->setWindowIcon(embed::getIcon("instrument_track"));
    // win->setFixedSize( win->size() );
    win->hide();

    // Hide the Size and Maximize options from the system menu
    // since the dialog size is fixed.
    // QMenu * systemMenu = win->systemMenu();
    // systemMenu->actions().at( 2 )->setVisible( false ); // Size
    // systemMenu->actions().at( 4 )->setVisible( false ); // Maximize
}

InstrumentTrackWindow::~InstrumentTrackWindow()
{
    // InstrumentTrackView::s_windowCache.removeAll(this);

    DELETE_HELPER(m_peripheralView)
    DELETE_HELPER(m_instrumentView)

    if(gui->mainWindow()->workspace())
    {
        parentWidget()->hide();
        parentWidget()->deleteLater();
    }
}

void InstrumentTrackWindow::setInstrumentTrackView(InstrumentTrackView* view)
{
    if(m_itv && view)
        m_itv->m_tlb->setChecked(false);

    m_itv = view;
}

void InstrumentTrackWindow::modelChanged()
{
    InstrumentTrack* m = model();  // castModel<InstrumentTrack>();

    m_nameLineEdit->setText(m->name());

    m->disconnect(SIGNAL(nameChanged()), this);
    m->disconnect(SIGNAL(instrumentChanged()), this);

    connect(m, SIGNAL(nameChanged()), this, SLOT(updateName()));
    connect(m, SIGNAL(instrumentChanged()), this,
            SLOT(updateInstrumentView()));

    m_volumeKnob->setModel(&m->m_volumeModel);
    m_panningKnob->setModel(&m->m_panningModel);
    m_effectChannelNumber->setModel(&m->m_effectChannelModel);
    m_peripheralView->setModel(&m->m_piano);

    m_ssView->setModel(&m->m_soundShaping);

    for(int i = 0; i < m->m_noteFunctions.size(); i++)
        m_noteFunctionViews[i]->setModel(m->m_noteFunctions[i]);

    m_midiView->setModel(&m->m_midiPort);
    m_effectView->setModel(m->m_audioPort->effects());
    m_miscView->pitchGroupBox()->ledButton()->setModel(
            &m->m_useMasterPitchModel);
    updateName();
}

void InstrumentTrackWindow::saveSettingsBtnClicked()
{
    InstrumentTrack* m = model();

    FileDialog sfd(this, tr("Save preset"), "",
                   tr("XML preset file (*.xpf)"));

    QString presetRoot = ConfigManager::inst()->userPresetsDir();
    if(!QDir(presetRoot).exists())
    {
        QDir().mkdir(presetRoot);
    }
    if(!QDir(presetRoot + m->instrumentName()).exists())
    {
        QDir(presetRoot).mkdir(m->instrumentName());
    }

    sfd.setAcceptMode(FileDialog::AcceptSave);
    sfd.setDirectory(presetRoot + m->instrumentName());
    sfd.setFileMode(FileDialog::AnyFile);
    QString fname = m->name();
    sfd.selectFile(fname.remove(QRegExp("[^a-zA-Z0-9_\\-\\d\\s]")));
    sfd.setDefaultSuffix("xpf");

    if(sfd.exec() == QDialog::Accepted && !sfd.selectedFiles().isEmpty()
       && !sfd.selectedFiles().first().isEmpty())
    {
        DataFile::LocaleHelper localeHelper(DataFile::LocaleHelper::ModeSave);
        DataFile               dataFile(DataFile::InstrumentTrackSettings);

        m->setSimpleSerializing();
        m->saveSettings(dataFile, dataFile.content());
        QString f = sfd.selectedFiles()[0];
        dataFile.writeFile(f);
    }
}

void InstrumentTrackWindow::updateName()
{
    InstrumentTrack* m = model();
    setWindowTitle(m->name().length() > 25 ? (m->name().left(24) + "...")
                                           : m->name());

    if(m_nameLineEdit->text() != m->name())
        m_nameLineEdit->setText(m->name());
}

void InstrumentTrackWindow::updateInstrumentView()
{
    delete m_instrumentView;

    InstrumentTrack* m = model();
    qInfo("InstrumentTrackWindow::updateInstrumentView 1 m=%p", m);
    if(m->m_instrument != nullptr)
    {
        m_instrumentView = m->m_instrument->createView(m_tabWidget);
        qInfo("InstrumentTrackWindow::updateInstrumentView 2 miv=%p",
              m_instrumentView);
        m_tabWidget->addTab(m_instrumentView, tr("Plugin"), "plugin_tab", 0);
        m_tabWidget->setActiveTab(0);
        m_ssView->setFunctionsHidden(m->m_instrument->isSingleStreamed());

        // Get the instrument window to refresh
        m_nameLineEdit->setText(m->name());
        if(m->instrument() && m->instrument()->isBendable())
        {
            if(m_bendingKnob->model() != &m->m_bendingModel)
                m_bendingKnob->setModel(&m->m_bendingModel);
            if(m_bendingRangeSpinBox->model() != &m->m_bendingRangeModel)
                m_bendingRangeSpinBox->setModel(&m->m_bendingRangeModel);
            m_bendingKnob->show();
            // m_pitchLabel->show();
            m_bendingRangeSpinBox->show();
            // m_bendingRangeLabel->show();
        }
        else
        {
            m_bendingKnob->hide();
            // m_pitchLabel->hide();
            if(!m_bendingKnob->model()->isDefaultConstructed())
                m_bendingKnob->setModel(new RealModel(0., 0., 0., 0.01,
                                                      nullptr, tr("Bending"),
                                                      "bending", true));
            m_bendingRangeSpinBox->hide();
            // m_bendingRangeLabel->hide();
        }
        updateName();

        // Get the text on the trackButton to change
        m->emit dataChanged();
    }
}

void InstrumentTrackWindow::textChanged(const QString& _newName)
{
    model()->setName(_newName);
    Engine::song()->setModified();
}

void InstrumentTrackWindow::toggleVisibility(bool _on)
{
    if(_on)
    {
        show();
        parentWidget()->show();
        parentWidget()->raise();
    }
    else
    {
        parentWidget()->hide();
    }
}

void InstrumentTrackWindow::closeEvent(QCloseEvent* _ce)
{
    _ce->ignore();

    if(gui->mainWindow()->workspace())
        parentWidget()->hide();
    else
        hide();

    m_itv->m_tlb->setFocus();
    m_itv->m_tlb->setChecked(false);
}

void InstrumentTrackWindow::focusInEvent(QFocusEvent*)
{
    m_peripheralView->setFocus();
}

void InstrumentTrackWindow::dragEnterEvent(QDragEnterEvent* _dee)
{
    StringPairDrag::processDragEnterEvent(
            _dee, "instrument,presetfile,pluginpresetfile");
}

void InstrumentTrackWindow::dropEvent(QDropEvent* _de)
{
    // qInfo("InstrumentTrackWindow::dropEvent event=%p", event);

    QString type  = StringPairDrag::decodeKey(_de);
    QString value = StringPairDrag::decodeValue(_de);

    InstrumentTrack* m = model();

    // qInfo("InstrumentTrackWindow::dropEvent #2");
    if(type == "instrument")
    {
        // qInfo("InstrumentTrackWindow::dropEvent #instrument");

        m->loadInstrument(value);

        Engine::song()->setModified();
        _de->accept();
        setFocus();
    }
    else if(type == "presetfile")
    {
        // qInfo("InstrumentTrackWindow::dropEvent #preset");

        DataFile dataFile(value);
        InstrumentTrack::removeMidiPortNode(dataFile);
        m->setSimpleSerializing();
        m->loadSettings(dataFile.content().toElement());

        Engine::song()->setModified();
        _de->accept();
        setFocus();
    }
    else if(type == "pluginpresetfile")
    {
        // qInfo("InstrumentTrackWindow::dropEvent #plugin");

        const QString    ext = FileItem::extension(value);
        InstrumentTrack* m   = model();
        Instrument*      i   = m->instrument();

        // qInfo("InstrumentTrackWindow::dropEvent i=%p", i);

        if((i == nullptr) || !i->descriptor()->supportsFileType(ext))
        {
            i = m->loadInstrument(
                    pluginFactory->pluginSupportingExtension(ext).name());
        }

        if(i != nullptr)
        {
            i->loadFile(value);
            Engine::song()->setModified();
        }

        _de->accept();
        setFocus();
    }
}

void InstrumentTrackWindow::saveSettings(QDomDocument& doc,
                                         QDomElement&  thisElement)
{
    thisElement.setAttribute("tab", m_tabWidget->activeTab());
    MainWindow::saveWidgetState(this, thisElement);
}

void InstrumentTrackWindow::loadSettings(const QDomElement& thisElement)
{
    m_tabWidget->setActiveTab(thisElement.attribute("tab").toInt());
    MainWindow::restoreWidgetState(this, thisElement);
    if(isVisible())
        m_itv->m_tlb->setChecked(true);
}

/*
void InstrumentTrackWindow::viewInstrumentInDirection(int d)
{
    // helper routine for viewNextInstrument, viewPrevInstrument
    // d=-1 to view the previous instrument,
    // d=+1 to view the next instrument

    const QList<TrackView*>& trackViews
            = m_itv->trackContainerView()->trackViews();
    int idxOfMe = trackViews.indexOf(m_itv);

    // search for the next InstrumentTrackView (i.e. skip AutomationViews,
    // etc) sometimes, the next InstrumentTrackView may already be open,
in
    // which case
    //   replace our window contents with the *next* closed Instrument
Track
    //   and give focus to the InstrumentTrackView we skipped.
    int                  idxOfNext    = idxOfMe;
    InstrumentTrackView* newView      = nullptr;
    InstrumentTrackView* bringToFront = nullptr;
    do
    {
        idxOfNext = (idxOfNext + d + trackViews.size()) %
trackViews.size(); newView   =
dynamic_cast<InstrumentTrackView*>(trackViews[idxOfNext]);
        // the window that should be brought to focus is the FIRST
        // InstrumentTrackView that comes after us
        if(bringToFront == nullptr && newView != nullptr)
        {
            bringToFront = newView;
        }
        // if the next instrument doesn't have an active window, then exit
        // loop & load that one into our window.
        if(newView != nullptr && !newView->m_tlb->isChecked())
        {
            break;
        }
    } while(idxOfNext != idxOfMe);

    // avoid reloading the window if there is only one instrument, as that
    // will just change the active tab
    if(idxOfNext != idxOfMe)
    {
        // save current window pos and then hide the window by unchecking
its
        // button in the track list
        QPoint curPos = parentWidget()->pos();
        m_itv->m_tlb->setChecked(false);

        // enable the new window by checking its track list button &
moving it
        // to where our window just was
        newView->m_tlb->setChecked(true);
        newView->instrumentTrackWindow()->parentWidget()->move(curPos);

        // scroll the SongEditor/BB-editor to make sure the new trackview
        // label is visible
        bringToFront->trackContainerView()->scrollToTrackView(bringToFront);
    }
    bringToFront->instrumentTrackWindow()->setFocus();
}

void InstrumentTrackWindow::viewNextInstrument()
{
    viewInstrumentInDirection(+1);
}
void InstrumentTrackWindow::viewPrevInstrument()
{
    viewInstrumentInDirection(-1);
}
*/

void InstrumentTrackWindow::switchToLaunchpad()
{
    PeripheralView* old = m_peripheralView;
    m_peripheralView = new PeripheralLaunchpadView(&model()->m_piano, this);
    m_peripheralView->setFixedSize(INSTRUMENT_WIDTH, PIANO_HEIGHT);
    // m_peripheralView->setModel(&m->m_piano);
    layout()->addWidget(m_peripheralView);
    if(old != nullptr)
    {
        layout()->removeWidget(old);
        delete old;
    }
    update();
}

void InstrumentTrackWindow::switchToPads()
{
    PeripheralView* old = m_peripheralView;
    m_peripheralView    = new PeripheralPadsView(&model()->m_piano, this);
    m_peripheralView->setFixedSize(INSTRUMENT_WIDTH, PIANO_HEIGHT);
    // m_peripheralView->setModel(&m->m_piano);
    layout()->addWidget(m_peripheralView);
    if(old != nullptr)
    {
        layout()->removeWidget(old);
        delete old;
    }
    update();
}

void InstrumentTrackWindow::switchToPiano()
{
    PeripheralView* old = m_peripheralView;
    m_peripheralView    = new PianoView(&model()->m_piano, this);
    m_peripheralView->setFixedSize(INSTRUMENT_WIDTH, PIANO_HEIGHT);
    // m_peripheralView->setModel(&m->m_piano);
    layout()->addWidget(m_peripheralView);
    if(old != nullptr)
    {
        layout()->removeWidget(old);
        delete old;
    }
    update();
}

//#include "InstrumentTrack.moc"
