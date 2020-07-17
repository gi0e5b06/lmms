/*
 * PresetPreviewPlayHandle.cpp - implementation of class
 * PresetPreviewPlayHandle
 *
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

#include "PresetPreviewPlayHandle.h"

#include "Engine.h"
#include "Instrument.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "Mutex.h"
#include "PluginFactory.h"
#include "ProjectJournal.h"
#include "TrackContainer.h"

#include <QFileInfo>
#include <QTimer>

// invisible track-container which is needed as parent for preview-channels
class PreviewTrackContainer : public TrackContainer
{
  public:
    PreviewTrackContainer() :
          TrackContainer(nullptr, tr("Preview track container"), "previewTrackContainer"),
          m_previewInstrumentTrack(nullptr), m_previewNPH(nullptr),
          m_dataMutex("PreviewTrackContainer::m_dataMutex",
                      QMutex::Recursive,
                      false)
    {
        setJournalling(false);
        m_previewInstrumentTrack = dynamic_cast<InstrumentTrack*>(
                Track::create(Track::InstrumentTrack, this));
        m_previewInstrumentTrack->setJournalling(false);
        // m_previewInstrumentTrack->setPreviewMode(true);
    }

    virtual ~PreviewTrackContainer()
    {
    }

    virtual QString nodeName() const
    {
        return "previewtrackcontainer";
    }

    InstrumentTrack* previewInstrumentTrack()
    {
        return m_previewInstrumentTrack;
    }

    NotePlayHandle* previewNote()
    {
        return m_previewNPH;
    }

    void setPreviewNote(NotePlayHandle* _note)
    {
        m_previewNPH = _note;
    }

    void lockData()
    {
        m_dataMutex.lock();
    }

    void unlockData()
    {
        m_dataMutex.unlock();
    }

    bool isPreviewing()
    {
        bool ret = !m_dataMutex.tryLock();
        // qWarning("PresetPreviewPlayHandle isPreviewing=%d",ret);
        if(ret == false)
        {
            m_dataMutex.unlock();
        }
        return ret;
    }

  private:
    InstrumentTrack* m_previewInstrumentTrack;
    NotePlayHandle*  m_previewNPH;
    Mutex            m_dataMutex;

    friend class PresetPreviewPlayHandle;
};

PreviewTrackContainer* PresetPreviewPlayHandle::s_previewTC;

PresetPreviewPlayHandle::PresetPreviewPlayHandle(const QString& _preset_file,
                                                 bool      _load_by_plugin,
                                                 DataFile* dataFile) :
      PlayHandle(TypePresetPreviewHandle),
      m_previewNPH(nullptr)
{
    qInfo("PresetPreviewPlayHandle::PresetPreviewPlayHandle 0");
    s_previewTC->lockData();

    setUsesBuffer(false);

    /*
    NotePlayHandle* pn = s_previewTC->previewNote();
    if(pn != nullptr)
    {
        qInfo("PresetPreviewPlayHandle::PresetPreviewPlayHandle 1");
        pn->noteOff();
        qInfo("PresetPreviewPlayHandle::PresetPreviewPlayHandle 2");
        pn->mute();
        qInfo("PresetPreviewPlayHandle::PresetPreviewPlayHandle 3");
        // Engine::mixer()->removePlayHandle(s_previewTC->previewNote());
        s_previewTC->setPreviewNote(nullptr);
    }
    */

    const bool j = Engine::projectJournal()->isJournalling();
    Engine::projectJournal()->setJournalling(false);

    if(_load_by_plugin)
    {
        Instrument*   i = s_previewTC->previewInstrumentTrack()->instrument();
        const QString ext = QFileInfo(_preset_file).suffix().toLower();

        if(i == nullptr || !i->descriptor()->supportsFileType(ext))
            i = s_previewTC->previewInstrumentTrack()->loadInstrument(
                    pluginFactory->pluginSupportingExtension(ext).name());

        if(i != nullptr)
            i->loadFile(_preset_file);
    }
    else
    {
        bool dataFileCreated = false;
        if(dataFile == nullptr)
        {
            dataFile        = new DataFile(_preset_file);
            dataFileCreated = true;
        }

        // vestige previews are bug prone; fallback on 3xosc with volume of 0
        // without an instrument in preview track, it will segfault
        if(dataFile->content().elementsByTagName("vestige").length() == 0)
        {
            qInfo("PresetPreviewPlayHandle::PresetPreviewPlayHandle 4");
            s_previewTC->previewInstrumentTrack()->loadTrackSpecificSettings(
                    dataFile->content().firstChild().toElement());
            qInfo("PresetPreviewPlayHandle::PresetPreviewPlayHandle 5");
        }
        else
        {
            qInfo("PresetPreviewPlayHandle: Vestige->3osc");
            s_previewTC->previewInstrumentTrack()->loadInstrument(
                    "tripleoscillator");
            s_previewTC->previewInstrumentTrack()->setVolume(0);
        }

        if(dataFileCreated)
            delete dataFile;
    }
    dataFile = 0;
    qInfo("PresetPreviewPlayHandle::PresetPreviewPlayHandle 6");

    // make sure, our preset-preview-track does not appear in any MIDI-
    // devices list, so just disable receiving/sending MIDI-events at all
    s_previewTC->previewInstrumentTrack()->midiPort()->setMode(
            MidiPort::Disabled);

    // setAudioPort(s_previewTC->previewInstrumentTrack()->audioPort());

    if(s_previewTC->previewInstrumentTrack()->getVolume() > DefaultVolume)
        s_previewTC->previewInstrumentTrack()->setVolume(DefaultVolume);

    if(!Engine::mixer()->criticalXRuns()
       && !s_previewTC->previewInstrumentTrack()->isMuted())
    {
        connect(Engine::mixer(), SIGNAL(playHandleDeleted(PlayHandlePointer)),
                this, SLOT(onPlayHandleDeleted(PlayHandlePointer)));

        // create note-play-handle for it
        m_previewNPH = NotePlayHandleManager::acquire(
                s_previewTC->previewInstrumentTrack(), 0,
                std::numeric_limits<f_cnt_t>::max() / 2,
                Note(0, 0, DefaultKey, 100));

        qInfo("PresetPreviewPlayHandle::PresetPreviewPlayHandle 7");
        s_previewTC->setPreviewNote(m_previewNPH);
        Engine::mixer()->emit playHandleToAdd(m_previewNPH->pointer());
    }

    qInfo("PresetPreviewPlayHandle::PresetPreviewPlayHandle 8");
    s_previewTC->unlockData();
    Engine::projectJournal()->setJournalling(j);
}

PresetPreviewPlayHandle::~PresetPreviewPlayHandle()
{
    s_previewTC->lockData();

    NotePlayHandle* pn = s_previewTC->previewNote();
    if(pn != nullptr && pn == m_previewNPH)
        s_previewTC->setPreviewNote(nullptr);

    if(m_previewNPH != nullptr)
    {
        qInfo("PresetPreviewPlayHandle::~PresetPreviewPlayHandle 0");
        if(!m_previewNPH->isFinished())
            m_previewNPH->noteOff();
        // qInfo("PresetPreviewPlayHandle::~PresetPreviewPlayHandle 1a");
        // m_previewNPH->mute();
        qInfo("PresetPreviewPlayHandle::~PresetPreviewPlayHandle 1");
        // m_previewNPH->setFinished();
        // Engine::mixer()->emit playHandleToRemove(m_previewNPH);
        qInfo("PresetPreviewPlayHandle::~PresetPreviewPlayHandle 2");
        m_previewNPH = nullptr;
    }

    s_previewTC->unlockData();
    BACKTRACE
}

void PresetPreviewPlayHandle::enterMixer()
{
    // setAudioPort(s_previewTC->previewInstrumentTrack()->audioPort());
    s_previewTC->previewInstrumentTrack()->audioPort()->addPlayHandle(
            pointer());
}

void PresetPreviewPlayHandle::exitMixer()
{
    s_previewTC->previewInstrumentTrack()->audioPort()->removePlayHandle(
            pointer());
}

void PresetPreviewPlayHandle::onPlayHandleDeleted(PlayHandlePointer handle)
{
    // qInfo("PresetPreviewPlayHandle::onPlayHandleDeleted");
    s_previewTC->lockData();
    if(m_previewNPH != nullptr)
    {
        if(m_previewNPH == handle)
        {
            qInfo("PresetPreviewPlayHandle::onPlayHandleDeleted");
            s_previewTC->setPreviewNote(nullptr);
            m_previewNPH = nullptr;
        }
    }
    s_previewTC->unlockData();
}

void PresetPreviewPlayHandle::noteOff(const f_cnt_t offset)
{
    s_previewTC->lockData();
    if(m_previewNPH != nullptr)
    {
        qInfo("PresetPreviewPlayHandle::noteOff");
        m_previewNPH->noteOff(offset);
    }
    s_previewTC->unlockData();
}

void PresetPreviewPlayHandle::play(sampleFrame* _working_buffer)
{
    // Do nothing; the preview instrument is played by m_previewNPH, which
    // has been added to the mixer
}

bool PresetPreviewPlayHandle::isFinished() const
{
    bool r = m_finished || m_previewNPH == nullptr || m_previewNPH->isMuted();
    qInfo("PresetPreviewPlayHandle::isFinished r=%d", r);
    return r;
}

bool PresetPreviewPlayHandle::isFromTrack(const Track* _track) const
{
    return s_previewTC != nullptr && _track != nullptr
           && s_previewTC->previewInstrumentTrack() == _track;
}

bool PresetPreviewPlayHandle::isFromInstrument(
        const Instrument* _instrument) const
{
    return false;
}

void PresetPreviewPlayHandle::init()
{
    if(s_previewTC == nullptr)
        s_previewTC = new PreviewTrackContainer;
}

void PresetPreviewPlayHandle::cleanup()
{
    DELETE_HELPER(s_previewTC);
    /*
    if(s_previewTC != nullptr)
    {
        delete s_previewTC;
        s_previewTC = nullptr;
    }
    */
}

ConstNotePlayHandles PresetPreviewPlayHandle::nphsOfInstrumentTrack(
        const InstrumentTrack* _it)
{
    ConstNotePlayHandles cnphv;
    s_previewTC->lockData();
    if(s_previewTC->previewNote() != nullptr
       && s_previewTC->previewNote()->instrumentTrack() == _it)
        cnphv.append(s_previewTC->previewNote());
    s_previewTC->unlockData();
    return cnphv;

    // return Engine::mixer()->nphsOfTrack(_it);
}

bool PresetPreviewPlayHandle::isPreviewing()
{
    return s_previewTC != nullptr && s_previewTC->isPreviewing();
}
