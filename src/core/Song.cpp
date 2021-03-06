/*
 * Song.cpp -
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

#include "Song.h"

#include "AutomationEditor.h"
#include "AutomationTrack.h"
#include "BBEditor.h"  // REQUIRED
#include "BBTrack.h"
#include "BBTrackContainer.h"
#include "ConfigManager.h"
#include "ControllerConnection.h"
#include "ControllerRackView.h"
#include "EnvelopeAndLfo.h"
#include "ExportFilter.h"
#include "ExportProjectDialog.h"
#include "FileDialog.h"
#include "FxMixer.h"
#include "FxMixerView.h"
#include "GuiApplication.h"
#include "ImportFilter.h"
#include "Instrument.h"
#include "MainWindow.h"
#include "Pattern.h"
#include "PeakController.h"
#include "PianoRoll.h"
#include "ProjectJournal.h"
#include "ProjectNotes.h"  // REQUIRED
#include "SongEditor.h"
#include "TextFloat.h"
#include "TimeLineWidget.h"
#include "VersionedSaveDialog.h"
#include "embed.h"
//#include "MemoryManagerArray.h"

#include <QCoreApplication>
#include <QTextStream>
//#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

#include <functional>

Song::Song() :
      TrackContainer(nullptr, tr("Song"), "song"),
      m_globalAutomationTrack(dynamic_cast<AutomationTrack*>(
              Track::create(Track::HiddenAutomationTrack, this))),
      m_tempoModel(
              DefaultTempo, MinTempo, MaxTempo, this, tr("Tempo"), "tempo"),
      m_timeSigModel(this), m_oldTicksPerTact(DefaultTicksPerTact),
      m_masterVolumeModel(
              100., 0., 200., 0.1, this, tr("Master volume"), "masterVolume"),
      m_masterPitchModel(
              0., -60., 60., 0.01, this, tr("Master pitch"), "masterPitch"),
      m_masterPanningModel(0.,
                           -100.,
                           100.,
                           0.1,
                           this,
                           tr("Master panning"),
                           "masterPanning"),
      m_metaData(), m_nLoadingTrack(0), m_fileName(), m_oldFileName(),
      m_modified(false), m_loadOnLaunch(true), m_recording(false),
      m_exporting(false), m_exportLoop(false), m_renderBetweenMarkers(false),
      m_playing(false), m_paused(false), m_loadingProject(false),
      m_isCancelled(false), m_savingProject(false), m_playMode(Mode_None),
      m_length(0), m_patternToPlay(nullptr), m_loopPattern(false),
      m_elapsedMilliSeconds(0), m_elapsedTicks(0), m_elapsedTacts(0)
{
    m_metaData.set("Created", QDateTime::currentDateTimeUtc().toString(
                                      "yyyy-MM-dd hh:mm:ss"));

    connect(&m_tempoModel, SIGNAL(dataChanged()), this, SLOT(setTempo()));
    connect(&m_tempoModel, SIGNAL(dataUnchanged()), this, SLOT(setTempo()));
    connect(&m_timeSigModel, SIGNAL(dataChanged()), this,
            SLOT(setTimeSignature()));

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(updateFramesPerTick()));

    connect(&m_masterVolumeModel, SIGNAL(dataChanged()), this,
            SLOT(masterVolumeChanged()));
    connect(&m_masterPanningModel, SIGNAL(dataChanged()), this,
            SLOT(masterPanningChanged()));

    setType(SongContainer);
}

Song::~Song()
{
    qInfo("Song::~Song 1");
    m_playing = false;
    MM_ACTIVE(false)
    qInfo("Song::~Song 2");
    // HAT deleted by the container
    // DELETE_HELPER(m_globalAutomationTrack)
    qInfo("Song::~Song 3");
}

void Song::masterVolumeChanged()
{
    Engine::mixer()->setMasterVolumeGain(m_masterVolumeModel.value() / 100.);
}

void Song::masterPanningChanged()
{
    Engine::mixer()->setMasterPanningGain(m_masterPanningModel.value()
                                          / 100.);
}

void Song::setTempo()
{
    const bpm_t tempo = (bpm_t)m_tempoModel.value();

    qInfo("Song::setTempo 1");
    Engine::mixer()->adjustTempo(tempo);
    qInfo("Song::setTempo 2");
    Engine::updateFramesPerTick();
    qInfo("Song::setTempo 3");
    m_vstSyncController.setTempo(tempo);
    qInfo("Song::setTempo 4");
    emit tempoChanged(tempo);
    qInfo("Song::setTempo 5");
}

void Song::setTimeSignature()
{
    const MeterModel& ts  = getTimeSigModel();
    const tick_t      tpt = MidiTime::ticksPerTact(ts);
    MidiTime::setTicksPerTact(tpt);
    MidiTime::setBeatsPerTact(MidiTime::beatsPerTact(ts));
    emit timeSignatureChanged(m_oldTicksPerTact, tpt);
    emit dataChanged();
    m_oldTicksPerTact = tpt;
    m_vstSyncController.setTimeSignature(ts.getNumerator(),
                                         ts.getDenominator());
}

void Song::savePos()
{
    TimeLineWidget* tl = m_playPos[m_playMode].m_timeLine;

    if(tl != nullptr)
    {
        tl->savePos(m_playPos[m_playMode]);
    }
}

void Song::processNextBuffer()
{
    // if not playing, nothing to do
    if(m_playing == false)
        return;

    Tracks trackList;
    int    tcoNum = -1;  // track content object number
    auto   bbTC   = Engine::getBBTrackContainer();

    // determine the list of tracks to play and the track content object
    // (TCO) number
    switch(m_playMode)
    {
        case Mode_PlaySong:
            trackList = tracks();
            // at song-start we have to reset the LFOs
            if(m_playPos[Mode_PlaySong] == 0)
                EnvelopeAndLfo::instances()->reset();
            break;

        case Mode_PlayBB:
            if(bbTC->lastUsedBeatIndex() >= 0)
            {
                tcoNum = bbTC->currentBB();
                trackList.append(BBTrack::findBBTrack(tcoNum));
            }
            break;

        case Mode_PlayPattern:
            if(m_patternToPlay != nullptr)
            {
                // tcoNum =
                // m_patternToPlay->track()->getTCONum(m_patternToPlay);
                trackList.append(m_patternToPlay->track());
            }
            break;

        case Mode_PlayAutomation:
            if(m_automationToPlay != nullptr)
            {
                // tcoNum = m_automationToPlay->track()->getTCONum(
                //        m_automationToPlay);
                bool all = false;
                // for(AutomatableModel* o: m_automationToPlay->objects())
                m_automationToPlay->objects().map([&trackList, &all](auto o) {
                    Model* p = o;
                    while(p != nullptr)
                    {
                        p = p->parentModel();

                        Track* t = dynamic_cast<Track*>(p);
                        if(t != nullptr)
                        {
                            // qWarning("AP +T %s",
                            // qPrintable(t->displayName()));
                            trackList.append(t);
                            break;
                        }

                        FxChannel* c = dynamic_cast<FxChannel*>(p);
                        if(c != nullptr)
                        {
                            // TO DO: add only relevant tracks
                            qWarning("AP +C %s",
                                     qPrintable(c->displayName()));
                            if(c->channelIndex() == 0)
                            {
                                all = true;
                                break;
                            }
                        }

                        /*
                        if(p != nullptr)
                        {
                            qWarning("AP ?? %s",
                                     qPrintable(p->displayName()));
                        }
                        */
                    }
                });

                if(all)
                    trackList = tracks();
                else
                    trackList.append(m_automationToPlay->track());
            }
            break;

        default:
            return;
    }

    // if we have no tracks to play, nothing to do
    if(trackList.empty())
        return;

    // check for looping-mode and act if necessary
    TimeLineWidget* tl        = m_playPos[m_playMode].m_timeLine;
    bool            checkLoop = (tl != nullptr) && (m_exporting == false)
                     && tl->loopPointsEnabled();

    if(tl != nullptr)
    {
        // int n =tl->currentLoop();
        int nn = tl->nextLoop();

        if(nn >= 0)
        {
            MidiTime& pos = m_playPos[m_playMode];
            // if looping-mode is enabled and we are outside of the looping
            // range, go to the beginning of the range
            if(pos >= tl->loopBegin(nn) && pos < tl->loopEnd(nn))
            {
                // qWarning("TLW: loop 1: pos=%d nlb=%d
                // nle=%d",pos.getTicks(),
                //	 tl->loopBegin(nn).getTicks(),tl->loopEnd(nn).getTicks());
                tl->setNextLoop(-1);
                tl->setCurrentLoop(nn);
                tl->toggleLoopPoints(
                        1);  // LoopPointStates::LoopPointsEnabled );
            }
        }

        if(checkLoop)
        {
            // if looping-mode is enabled and we are outside of the looping
            // range, go to the beginning of the range
            if(m_playPos[m_playMode] < tl->loopBegin()
               || m_playPos[m_playMode] >= tl->loopEnd())
            {
                qInfo("Loop begin 1: %d", tl->loopBegin().ticks());
                setToTime(tl->loopBegin());
                m_playPos[m_playMode].setTicks(tl->loopBegin().ticks());
                emit updateSampleTracks();
            }
        }
    }

    f_cnt_t      framesPlayed  = 0;
    const real_t framesPerTick = Engine::framesPerTick();

    while(framesPlayed < Engine::mixer()->framesPerPeriod())
    {
        m_vstSyncController.update();

        real_t currentFrame = m_playPos[m_playMode].currentFrame();
        // did we play a tick?
        if(currentFrame >= framesPerTick)
        {
            tick_t ticks = m_playPos[m_playMode].ticks()
                           + tick_t(currentFrame / framesPerTick);

            m_vstSyncController.setAbsolutePosition(ticks);

            // did we play a whole tact?
            if(ticks >= MidiTime::ticksPerTact())
            {
                // per default we just continue playing even if
                // there's no more stuff to play
                // (song-play-mode)
                int maxTact = m_playPos[m_playMode].getTact() + 2;

                // then decide whether to go over to next tact
                // or to loop back to first tact
                if(m_playMode == Mode_PlayBB)
                {
                    maxTact = bbTC->lengthOfCurrentBB();
                }
                else if(m_playMode == Mode_PlayPattern
                        && m_loopPattern == true && tl != nullptr
                        && tl->loopPointsEnabled() == false)
                {
                    maxTact = MidiTime(m_patternToPlay->unitLength())
                                      .getTact();
                    if(maxTact < 1)
                        maxTact = 1;
                }
                else if(m_playMode == Mode_PlayAutomation
                        && m_loopPattern == true && tl != nullptr
                        && tl->loopPointsEnabled() == false)
                {
                    maxTact = m_automationToPlay->unitLength()
                              / MidiTime::ticksPerTact();
                    if(maxTact < 1)
                        maxTact = 1;
                }

                // end of played object reached?
                if(m_playPos[m_playMode].getTact() + 1 >= maxTact)
                {
                    // then start from beginning and keep
                    // offset
                    ticks %= (maxTact * MidiTime::ticksPerTact());

                    // wrap milli second counter
                    setToTimeByTicks(ticks);

                    m_vstSyncController.setAbsolutePosition(ticks);
                }
            }
            m_playPos[m_playMode].setTicks(ticks);

            if(checkLoop)
            {
                m_vstSyncController.startCycle(tl->loopBegin().ticks(),
                                               tl->loopEnd().ticks());

                // if looping-mode is enabled and we have got
                // past the looping range, return to the
                // beginning of the range
                if(m_playPos[m_playMode] >= tl->loopEnd())
                {
                    // int n =tl->currentLoop();
                    int nn = tl->nextLoop();

                    if(nn >= 0)
                    {
                        tl->setNextLoop(-1);
                        tl->setCurrentLoop(nn);
                        tl->toggleLoopPoints(1);
                        // LoopPointStates::LoopPointsEnabled );
                    }
                    // else
                    {
                        m_playPos[m_playMode].setTicks(
                                tl->loopBegin().ticks());
                        setToTime(tl->loopBegin());
                    }
                }
                else if(m_playPos[m_playMode] == tl->loopEnd() - 1)
                {
                    emit updateSampleTracks();
                }
            }
            else
            {
                m_vstSyncController.stopCycle();
            }

            currentFrame = fmod(currentFrame, framesPerTick);
            m_playPos[m_playMode].setCurrentFrame(currentFrame);
        }

        f_cnt_t framesToPlay
                = Engine::mixer()->framesPerPeriod() - framesPlayed;

        f_cnt_t framesLeft = (f_cnt_t)framesPerTick - (f_cnt_t)currentFrame;
        // skip last frame fraction
        if(framesLeft == 0)
        {
            ++framesPlayed;
            m_playPos[m_playMode].setCurrentFrame(currentFrame + 1.);
            continue;
        }
        // do we have samples left in this tick but these are less
        // than samples we have to play?
        if(framesLeft < framesToPlay)
        {
            // then set framesToPlay to remaining samples, the
            // rest will be played in next loop
            framesToPlay = framesLeft;
        }

        if((f_cnt_t)currentFrame == 0)
        {
            // loop through all tracks and play them
            MidiTime  pos = m_playPos[m_playMode];
            PlayModes old = m_playMode;
            if(m_playMode == Mode_PlayAutomation)
            {
                pos += m_automationToPlay->startPosition();
                m_playPos[Mode_PlaySong].setTicks(pos.ticks());
                m_playMode = Mode_PlaySong;
                processAutomations(trackList, pos, framesToPlay);
            }
            else if(m_playMode == Mode_PlayPattern)
            {
                pos += m_patternToPlay->startPosition();
                m_playPos[Mode_PlaySong].setTicks(pos.ticks());
                m_playMode = Mode_PlaySong;
                processAutomations(trackList, pos, framesToPlay);
            }
            else
            {
                processAutomations(trackList, m_playPos[m_playMode],
                                   framesToPlay);
            }

            // qInfo("Song::play tl=%d pos=%s", trackList.size(),
            //      qPrintable(pos.toString()));
            for(int i = 0; i < trackList.size(); ++i)
                trackList[i]->play(pos, framesToPlay, framesPlayed, tcoNum);
            m_playMode = old;
        }

        // update frame-counters
        framesPlayed += framesToPlay;

        m_playPos[m_playMode].setCurrentFrame(framesToPlay + currentFrame);

        m_elapsedMilliSeconds += MidiTime::ticksToMilliseconds(
                framesToPlay / framesPerTick, getTempo());
        m_elapsedTacts = m_playPos[Mode_PlaySong].getTact();
        m_elapsedTicks
                = (m_playPos[Mode_PlaySong].getTicks() % ticksPerTact()) / 48;
    }
}

void Song::processAutomations(const TrackList& tracklist,
                              MidiTime         timeStart,
                              fpp_t)
{
    // AutomatedValueMap values;

    QSet<const AutomatableModel*> recordedModels;

    TrackContainer* container = this;
    int             tcoNum    = -1;

    switch(m_playMode)
    {
        case Mode_PlaySong:
            break;
        case Mode_PlayBB:
        {
            Q_ASSERT(tracklist.size() == 1);
            Q_ASSERT(tracklist.at(0)->type() == Track::BBTrack);
            auto bbTrack = qobject_cast<BBTrack*>(tracklist.at(0));
            auto bbTC    = Engine::getBBTrackContainer();
            container    = bbTC;
            tcoNum       = bbTrack->ownBBTrackIndex();
        }
        break;
        case Mode_PlayAutomation:
            break;
        default:
            return;
    }

    // values = container->automatedValuesAt(timeStart, tcoNum);
    m_automatedValues.clear();
    container->automatedValuesAt(timeStart, tcoNum, m_automatedValues);
    // qInfo("automated values: %d",m_automatedValues.size());

    const Tracks& tracks = container->tracks();

    Tiles tcos;
    for(const Track* track: tracks)
    {
        if((track->type() == Track::AutomationTrack) && !track->isMuted())
            track->getTCOsInRange(tcos, 0, timeStart);
    }

    // Process recording
    for(Tile* tco: tcos)
    {
        auto p = dynamic_cast<AutomationPattern*>(tco);
        if(!p)
            continue;

        MidiTime relTime = timeStart - p->startPosition();
        if(p->isRecording() && relTime >= 0 && relTime < p->length())
        {
            const AutomatableModel* recordedModel = p->firstObject();
            // const real_t v1=recordedModel->controllerValue(0,true);
            const real_t v2 = recordedModel->value<real_t>();
            // qInfo("Song record %f %f",v1,v2);
            p->emit recordValue(relTime, v2);

            recordedModels << recordedModel;
        }
    }

    // Apply values
    for(auto it = m_automatedValues.begin(); it != m_automatedValues.end();
        it++)  // values
    {
        auto m = it.key();
        if(!m)
            continue;
        if(!recordedModels.contains(m))
        {
            m->setAutomatedValue(it.value());
        }
    }
}

std::pair<MidiTime, MidiTime> Song::getExportEndpoints() const
{
    if(m_renderBetweenMarkers)
    {
        return std::pair<MidiTime, MidiTime>(
                m_playPos[Mode_PlaySong].m_timeLine->loopBegin(),
                m_playPos[Mode_PlaySong].m_timeLine->loopEnd());
    }
    else if(m_exportLoop)
    {
        return std::pair<MidiTime, MidiTime>(MidiTime(0, 0),
                                             MidiTime(m_length, 0));
    }
    else
    {
        // if not exporting as a loop, we leave one bar of padding at the end
        // of the song to accomodate reverb, etc.
        return std::pair<MidiTime, MidiTime>(MidiTime(0, 0),
                                             MidiTime(m_length + 1, 0));
    }
}

void Song::playSong()
{
    m_recording = false;

    if(isStopped() == false)
    {
        stop();
    }

    m_playMode = Mode_PlaySong;
    m_playing  = true;
    m_paused   = false;
    MM_ACTIVE(true)

    // qWarning("Playing song...");
    m_vstSyncController.setPlaybackState(true);
    savePos();
    emit playbackStateChanged();
}

void Song::record()
{
    m_recording = true;
    // TODO: Implement
}

void Song::playAndRecord()
{
    playSong();
    m_recording = true;
}

void Song::playBB()
{
    if(!isStopped())
        stop();

    m_playMode = Mode_PlayBB;
    m_playing  = true;
    m_paused   = false;
    MM_ACTIVE(true)

    m_vstSyncController.setPlaybackState(true);
    savePos();
    emit playbackStateChanged();
}

void Song::playPattern(const Pattern* _patternToPlay, bool _loop)
{
    if(!isStopped())
        stop();

    m_patternToPlay = _patternToPlay;
    m_loopPattern   = _loop;

    if(m_patternToPlay != nullptr)
    {
        m_playMode = Mode_PlayPattern;
        m_playing  = true;
        m_paused   = false;
        MM_ACTIVE(true)
    }

    m_vstSyncController.setPlaybackState(true);
    savePos();
    emit playbackStateChanged();
}

void Song::playAutomation(const AutomationPattern* _automationToPlay,
                          bool                     _loop)
{
    if(!isStopped())
        stop();

    m_automationToPlay = _automationToPlay;
    m_loopPattern      = _loop;

    if(m_automationToPlay != nullptr)
    {
        m_playMode = Mode_PlayAutomation;
        m_playing  = true;
        m_paused   = false;
        MM_ACTIVE(true)
    }

    m_vstSyncController.setPlaybackState(true);
    savePos();
    emit playbackStateChanged();
}

void Song::updateLength()
{
    m_length = 0;
    m_tracksMutex.lockForRead();
    for(TrackList::const_iterator it = tracks().begin(); it != tracks().end();
        ++it)
    {
        if(Engine::getSong()->isExporting() && (*it)->isMuted())
        {
            continue;
        }

        const tact_t cur = (*it)->length();
        if(cur > m_length)
        {
            m_length = cur;
        }
    }
    m_tracksMutex.unlock();

    emit lengthChanged(m_length);
}

void Song::setPlayPos(tick_t ticks, PlayModes playMode)
{
    tick_t ticksFromPlayMode = m_playPos[playMode].getTicks();
    m_elapsedTicks += ticksFromPlayMode - ticks;
    m_elapsedMilliSeconds += MidiTime::ticksToMilliseconds(
            ticks - ticksFromPlayMode, getTempo());
    m_playPos[playMode].setTicks(ticks);
    m_playPos[playMode].setCurrentFrame(0.);

    // send a signal if playposition changes during playback
    if(isPlaying())
    {
        emit playbackPositionChanged();
        emit updateSampleTracks();
    }
}

void Song::togglePause()
{
    if(m_paused == true)
    {
        m_playing = true;
        m_paused  = false;
        MM_ACTIVE(true)
    }
    else
    {
        m_playing = false;
        m_paused  = true;
        MM_ACTIVE(false)
    }

    m_vstSyncController.setPlaybackState(m_playing);

    emit playbackStateChanged();
}

void Song::stop()
{
    // do not stop/reset things again if we're stopped already
    if(m_playMode == Mode_None)
    {
        // return;
    }

    TimeLineWidget* tl = m_playPos[m_playMode].m_timeLine;
    m_paused           = false;
    m_recording        = true;

    if(tl != nullptr)
    {

        switch(tl->behaviourAtStop())
        {
            case TimeLineWidget::BackToZero:
                // m_playPos[m_playMode].setTicks( 0 );
                // m_elapsedMilliSeconds = 0;
                Engine::transport()->transportLocate(0);
                if(gui && gui->songWindow()
                   && (tl->autoScroll() == TimeLineWidget::AutoScrollEnabled))
                {
                    gui->songWindow()->m_editor->updatePosition(0);
                }
                break;

            case TimeLineWidget::BackToStart:
                if(tl->savedPos() >= 0)
                {
                    // m_playPos[m_playMode].setTicks(
                    // tl->savedPos().getTicks() ); setToTime(tl->savedPos());
                    Engine::transport()->transportLocate(
                            tl->savedPos().frames(Engine::framesPerTick()));
                    if(gui && gui->songWindow()
                       && (tl->autoScroll()
                           == TimeLineWidget::AutoScrollEnabled))
                    {
                        gui->songWindow()->m_editor->updatePosition(
                                MidiTime(tl->savedPos().getTicks()));
                    }
                    tl->savePos(-1);
                }
                break;

            case TimeLineWidget::KeepStopPosition:
            default:
                break;
        }
    }
    else
    {
        m_playPos[m_playMode].setTicks(0);
        m_elapsedMilliSeconds = 0;
    }
    m_playing = false;
    MM_ACTIVE(false)

    m_playPos[m_playMode].setCurrentFrame(0);

    m_vstSyncController.setPlaybackState(m_exporting);
    m_vstSyncController.setAbsolutePosition(m_playPos[m_playMode].getTicks());

    // remove all note-play-handles that are active
    Engine::mixer()->clear();

    m_playMode = Mode_None;

    emit playbackStateChanged();
}

void Song::startExport()
{
    stop();
    if(m_renderBetweenMarkers)
    {
        m_playPos[Mode_PlaySong].setTicks(
                m_playPos[Mode_PlaySong].m_timeLine->loopBegin().getTicks());
    }
    else
    {
        m_playPos[Mode_PlaySong].setTicks(0);
    }

    playSong();

    m_exporting = true;

    m_vstSyncController.setPlaybackState(true);
}

void Song::stopExport()
{
    stop();
    m_exporting  = false;
    m_exportLoop = false;

    m_vstSyncController.setPlaybackState(m_playing);
}

void Song::insertBar()
{
    m_tracksMutex.lockForRead();
    for(TrackList::const_iterator it = tracks().begin(); it != tracks().end();
        ++it)
    {
        (*it)->insertTact(m_playPos[Mode_PlaySong]);
    }
    m_tracksMutex.unlock();
}

void Song::removeBar()
{
    m_tracksMutex.lockForRead();
    for(TrackList::const_iterator it = tracks().begin(); it != tracks().end();
        ++it)
    {
        (*it)->removeTact(m_playPos[Mode_PlaySong]
                          + MidiTime::ticksPerTact());
    }
    m_tracksMutex.unlock();
}

void Song::addInstrumentTrack()
{
    (void)Track::create(Track::InstrumentTrack, this);
}

void Song::addBBTrack()
{
    Track* t = Track::create(Track::BBTrack, this);
    Engine::getBBTrackContainer()->setCurrentBB(
            dynamic_cast<BBTrack*>(t)->ownBBTrackIndex());
}

void Song::addSampleTrack()
{
    (void)Track::create(Track::SampleTrack, this);
}

void Song::addAutomationTrack()
{
    (void)Track::create(Track::AutomationTrack, this);
}

bpm_t Song::getTempo()
{
    return (bpm_t)m_tempoModel.value();
}

AutomationPattern* Song::tempoAutomationPattern()
{
    return AutomationPattern::globalAutomationPattern(&m_tempoModel);
}

AutomatedValueMap Song::automatedValuesAt(MidiTime time, int tcoNum) const
{
    // return
    // TrackContainer::automatedValuesFromTracks(TrackList{m_globalAutomationTrack}
    // << tracks(), time, tcoNum);
    AutomatedValueMap r;
    TrackContainer::automatedValuesFromTrack(m_globalAutomationTrack, time,
                                             tcoNum, r);
    for(Track* t: tracks())
        TrackContainer::automatedValuesFromTrack(t, time, tcoNum, r);
    return r;
}

void Song::clearProject()
{
    Engine::projectJournal()->setJournalling(false);

    if(m_playing)
        stop();

    for(int i = 0; i < Mode_Count; i++)
        setPlayPos(0, (PlayModes)i);

    qInfo("Song::clearProject 1");

    // Engine::mixer()->clearAllPlayHandles();

    qInfo("Song::clearProject 2a");

    // Engine::mixer()->requestChangeInModel();

    if(gui && gui->bbWindow())
    {
        gui->bbWindow()->trackContainerView()->clearAllTracks();
    }

    qInfo("Song::clearProject 2b");

    if(gui && gui->songWindow())
    {
        gui->songWindow()->m_editor->clearAllTracks();
    }

    qInfo("Song::clearProject 2c");

    if(gui && gui->fxMixerView())
    {
        gui->fxMixerView()->clear();
    }

    // Engine::mixer()->doneChangeInModel();
    QCoreApplication::sendPostedEvents();
    // Engine::mixer()->requestChangeInModel();

    qInfo("Song::clearProject 3a");
    Engine::getBBTrackContainer()->clearAllTracks();
    qInfo("Song::clearProject 3b");
    clearAllTracks();
    qInfo("Song::clearProject 3c");
    clearSongMetaData();
    qInfo("Song::clearProject 4");

    /*
    Engine::mixer()->doneChangeInModel();
    QCoreApplication::sendPostedEvents();
    Engine::mixer()->requestChangeInModel();
    */

    qInfo("Song::clearProject 5");

    Engine::fxMixer()->clear();
    QCoreApplication::sendPostedEvents();

    qInfo("Song::clearProject 6");

    if(gui && gui->automationWindow())
    {
        gui->automationWindow()->reset();
    }

    qInfo("Song::clearProject 7");

    if(gui && gui->pianoRollWindow())
    {
        gui->pianoRollWindow()->reset();
    }

    qInfo("Song::clearProject 8");

    m_tempoModel.reset();
    m_masterVolumeModel.reset();
    m_masterPitchModel.reset();
    m_masterPanningModel.reset();
    m_timeSigModel.reset();

    qInfo("Song::clearProject 9");

    AutomationPattern::globalAutomationPattern(&m_tempoModel)->clear();
    AutomationPattern::globalAutomationPattern(&m_masterVolumeModel)->clear();
    AutomationPattern::globalAutomationPattern(&m_masterPitchModel)->clear();
    AutomationPattern::globalAutomationPattern(&m_masterPanningModel)
            ->clear();

    // Engine::mixer()->doneChangeInModel();

    qInfo("Song::clearProject 10");

    if(gui && gui->getProjectNotes())
    {
        gui->getProjectNotes()->clear();
    }

    removeAllControllers();

    SampleBuffer::clearMMap();

    qInfo("Song::clearProject 11");

    emit dataChanged();

    qInfo("Song::clearProject 12");

    Engine::projectJournal()->clearJournal();
    Engine::projectJournal()->setJournalling(true);

    qInfo("Song::clearProject 13");

    // InstrumentTrackView::cleanupWindowCache();
    // qInfo("Song::clearProject 14");
}

// create new file
void Song::createNewProject()
{
    QString defaultTemplate
            = ConfigManager::inst()->userTemplateDir() + "default.mpt";

    if(QFile::exists(defaultTemplate))
    {
        createNewProjectFromTemplate(defaultTemplate);
        return;
    }

    defaultTemplate = ConfigManager::inst()->factoryProjectsDir()
                      + "templates/default.mpt";
    if(QFile::exists(defaultTemplate))
    {
        createNewProjectFromTemplate(defaultTemplate);
        return;
    }

    m_loadingProject = true;

    clearProject();

    Engine::projectJournal()->setJournalling(false);

    m_fileName = m_oldFileName = "";

    Track* t;
    t = Track::create(Track::InstrumentTrack, this);
    dynamic_cast<InstrumentTrack*>(t)->loadInstrument("tripleoscillator");
    t = Track::create(Track::InstrumentTrack, Engine::getBBTrackContainer());
    dynamic_cast<InstrumentTrack*>(t)->loadInstrument("kicker");
    Track::create(Track::SampleTrack, this);
    Track::create(Track::BBTrack, this);
    Track::create(Track::AutomationTrack, this);

    m_tempoModel.setInitValue(DefaultTempo);
    m_timeSigModel.reset();
    m_masterVolumeModel.setInitValue(100);
    m_masterPitchModel.setInitValue(0);
    m_masterPanningModel.setInitValue(0);

    QCoreApplication::instance()->processEvents();

    m_loadingProject = false;

    Engine::projectJournal()->clearJournal();
    Engine::projectJournal()->setJournalling(true);

    QCoreApplication::sendPostedEvents();

    m_modified     = false;
    m_loadOnLaunch = false;

    if(gui->mainWindow() != nullptr)
        gui->mainWindow()->resetWindowTitle();
}

void Song::createNewProjectFromTemplate(const QString& templ)
{
    loadProject(templ);
    m_metaData.set("Created", QDateTime::currentDateTimeUtc().toString(
                                      "yyyy-MM-dd hh:mm:ss"));

    // clear file-name so that user doesn't overwrite template when
    // saving...
    m_fileName = m_oldFileName = "";
    // update window title
    m_loadOnLaunch = false;

    if(gui->mainWindow())
        gui->mainWindow()->resetWindowTitle();
}

// load given song
void Song::loadProject(const QString& fileName)
{
    if(m_loadingProject)
        qWarning("Song::loadProject() called when another load is running");

    if(m_savingProject)
        qWarning("Song::loadProject() called when a save is running");

    m_loadingProject = true;

    Engine::projectJournal()->setJournalling(false);

    m_oldFileName = m_fileName;
    m_fileName    = fileName;

    DataFile dataFile(m_fileName);
    // if file could not be opened, head-node is null and we create
    // new project
    if(dataFile.head().isNull())
    {
        if(m_loadOnLaunch)
            createNewProject();
        m_fileName = m_oldFileName;
        return;
    }

    m_oldFileName = m_fileName;

    clearProject();

    clearErrors();

    DataFile::LocaleHelper localeHelper(DataFile::LocaleHelper::ModeLoad);

    Engine::mixer()->requestChangeInModel();

    // get the header information from the DOM
    m_tempoModel.loadSettings(dataFile.head(), "bpm");
    m_timeSigModel.loadSettings(dataFile.head(), "timesig");
    m_masterVolumeModel.loadSettings(dataFile.head(), "mastervol");
    m_masterPitchModel.loadSettings(dataFile.head(), "masterpitch");
    m_masterPanningModel.loadSettings(dataFile.head(), "masterpanning");

    if(m_metaData.load(dataFile, dataFile.head()))
        emit metaDataChanged();

    if(m_playPos[Mode_PlaySong].m_timeLine)
    {
        // reset loop-point-state
        m_playPos[Mode_PlaySong].m_timeLine->toggleLoopPoints(0);
    }

    if(!dataFile.content().firstChildElement("track").isNull())
    {
        m_globalAutomationTrack->restoreState(
                dataFile.content().firstChildElement("track"));
    }

    // Backward compatibility for LMMS <= 0.4.15
    PeakController::initGetControllerBySetting();

    // Load mixer first to be able to set the correct range for FX channels
    {
        QDomNode node = dataFile.content().firstChildElement(
                Engine::fxMixer()->nodeName());
        if(!node.isNull())
        {
            Engine::fxMixer()->restoreState(node.toElement());
            if(gui)
            {
                // refresh FxMixerView
                gui->fxMixerView()->refreshDisplay();
            }
        }
    }

    {
        QDomNodeList tclist
                = dataFile.content().elementsByTagName("trackcontainer");
        const int n     = tclist.count();
        m_nLoadingTrack = 0;
        for(int i = 0; i < n; ++i)
        {
            QDomNode nd = tclist.at(i).firstChild();
            while(!nd.isNull())
            {
                if(nd.isElement() && nd.nodeName() == "track")
                {
                    ++m_nLoadingTrack;
                    if(nd.toElement().attribute("type").toInt()
                       == Track::BBTrack)
                    {
                        // n ???
                        m_nLoadingTrack
                                += nd.toElement()
                                           .elementsByTagName("bbtrack")
                                           .at(0)
                                           .toElement()
                                           .firstChildElement()
                                           .childNodes()
                                           .count();
                    }
                }
                nd = nd.nextSibling();
            }
        }
    }

    // load the different sections
    {
        QDomNode node = dataFile.content().firstChild();

        while(!node.isNull() && !isCancelled())
        {
            if(node.isElement())
            {
                // qInfo("Song::loadProject tag %s start",
                //   qPrintable(node.nodeName()));

                if(node.nodeName() == "trackcontainer")
                {
                    // qInfo("Song::loadProject trackcontainer song start");
                    ((JournallingObject*)(this))
                            ->restoreState(node.toElement());
                    // qInfo("Song::loadProject trackcontainer song end");
                }
                else if(node.nodeName() == "controllers")
                {
                    restoreControllerStates(node.toElement());
                }
                else if(gui)
                {
                    if(node.nodeName()
                       == gui->getControllerRackView()->nodeName())
                    {
                        gui->getControllerRackView()->restoreState(
                                node.toElement());
                    }
                    else if(node.nodeName()
                            == gui->pianoRollWindow()->nodeName())
                    {
                        gui->pianoRollWindow()->restoreState(
                                node.toElement());
                    }
                    else if(node.nodeName()
                            == gui->automationWindow()->m_editor->nodeName())
                    {
                        gui->automationWindow()->m_editor->restoreState(
                                node.toElement());
                    }
                    else if(node.nodeName()
                            == gui->getProjectNotes()->nodeName())
                    {
                        gui->getProjectNotes()
                                ->SerializingObject::restoreState(
                                        node.toElement());
                    }
                    else if(node.nodeName()
                            == m_playPos[Mode_PlaySong]
                                       .m_timeLine->nodeName())
                    {
                        m_playPos[Mode_PlaySong].m_timeLine->restoreState(
                                node.toElement());
                    }
                }

                // qInfo("Song::loadProject tag %s end",
                //   qPrintable(node.nodeName()));
            }
            node = node.nextSibling();
        }
    }

    if(gui)
    {
        if(gui->pianoRollWindow()->currentPattern() == nullptr)
        {
            // gui->pianoRollWindow()->parentWidget()->hide();
        }

        if(gui->automationWindow()->m_editor->currentPattern() == nullptr)
        {
            // gui->automationWindow()->hide();
        }
    }

    // qInfo("Song::loadProject 0");
    // quirk for fixing projects with broken positions of TCOs inside
    // BB-tracks
    Engine::getBBTrackContainer()->fixIncorrectPositions();

    // qInfo("Song::loadProject 1");
    // Connect controller links to their controllers
    // now that everything is loaded
    ControllerConnection::finalizeConnections();

    // Remove dummy controllers that were added for correct connections
    m_controllers.erase(
            std::remove_if(m_controllers.begin(), m_controllers.end(),
                           [](Controller* c) {
                               return c->type()
                                      == Controller::DummyController;
                           }),
            m_controllers.end());

    // qInfo("Song::loadProject 2");
    // resolve all IDs so that autoModels are automated
    AutomationPattern::resolveAllIDs();

    // qInfo("Song::loadProject 3");
    Engine::mixer()->doneChangeInModel();

    // qInfo("Song::loadProject 4");
    ConfigManager::inst()->addRecentlyOpenedProject(fileName);

    Engine::projectJournal()->clearJournal();
    Engine::projectJournal()->setJournalling(true);
    emit projectLoaded();

    // qInfo("Song::loadProject 5");
    if(isCancelled())
    {
        m_isCancelled = false;
        createNewProject();
        return;
    }

    if(hasErrors())
    {
        if(gui)
        {
            QMessageBox::warning(nullptr, tr("LMMS Error report"),
                                 errorSummary(), QMessageBox::Ok);
        }
        else
        {
            QTextStream(stderr) << Engine::getSong()->errorSummary() << endl;
        }
    }

    // qInfo("Song::loadProject 6");
    m_loadingProject = false;
    m_modified       = false;
    m_loadOnLaunch   = false;

    if(gui && gui->mainWindow())
    {
        gui->mainWindow()->resetWindowTitle();
        // qInfo("Song::loadProject 7");
        gui->songWindow()->m_editor->realignTracks();
        // qInfo("Song::loadProject 8");
        gui->bbWindow()->trackContainerView()->realignTracks();
        // qInfo("Song::loadProject 9");
    }

    // qInfo("Song::loadProject 10");
}

void Song::buildProjectDataFile(DataFile& dataFile)
{
    if(m_savingProject)
    {
        qWarning(
                "Song::buildProjectDataFile() called when another save is "
                "running");
    }

    if(m_loadingProject)
    {
        qWarning(
                "Song::buildProjectDataFile() called when a load is running");
    }

    m_savingProject = true;

    DataFile::LocaleHelper localeHelper(DataFile::LocaleHelper::ModeSave);

    m_tempoModel.saveSettings(dataFile, dataFile.head(), "bpm");
    m_timeSigModel.saveSettings(dataFile, dataFile.head(), "timesig");
    m_masterVolumeModel.saveSettings(dataFile, dataFile.head(), "mastervol");
    m_masterPitchModel.saveSettings(dataFile, dataFile.head(), "masterpitch");
    m_masterPanningModel.saveSettings(dataFile, dataFile.head(),
                                      "masterpanning");

    m_metaData.save(dataFile, dataFile.head());

    saveState(dataFile, dataFile.content());

    m_globalAutomationTrack->saveState(dataFile, dataFile.content());
    Engine::fxMixer()->saveState(dataFile, dataFile.content());
    if(gui)
    {
        gui->getControllerRackView()->saveState(dataFile, dataFile.content());
        gui->pianoRollWindow()->saveState(dataFile, dataFile.content());
        gui->automationWindow()->m_editor->saveState(dataFile,
                                                     dataFile.content());
        gui->getProjectNotes()->SerializingObject::saveState(
                dataFile, dataFile.content());
        m_playPos[Mode_PlaySong].m_timeLine->saveState(dataFile,
                                                       dataFile.content());
    }

    saveControllerStates(dataFile, dataFile.content());

    m_savingProject = false;
}

// only save current song as _filename and do nothing else
bool Song::saveProjectFile(const QString& _fileName)
{
    if(m_metaData.get("Created").isEmpty())
    {
        QFileInfo fi(_fileName);
        QDateTime d = fi.birthTime();
        if(!d.isValid())
            d = fi.metadataChangeTime();
        if(!d.isValid())
            d = fi.lastModified();
        if(d.isValid())
            m_metaData.set("Created", d.toString("yyyy-MM-dd hh:mm:ss"));
    }

    DataFile dataFile(DataFile::SongProject);
    buildProjectDataFile(dataFile);

    bool r = dataFile.writeFile(_fileName);
    if(r && (_fileName == m_fileName))
    {
        dataFile.writeFile(projectDir() + QDir::separator() + "project"
                           + QDir::separator() + "current.mmp");
        dataFile.writeFile(projectDir() + QDir::separator() + "project"
                           + QDir::separator() + "backup" + QDir::separator()
                           + QDate::currentDate().toString("yyyyMMdd")
                           + ".mmpz");
    }
    return r;
}

// save current song and update the gui
bool Song::guiSaveProject()
{
    // strange code, why dataFile? GDX
    DataFile dataFile(DataFile::SongProject);
    m_fileName = dataFile.nameWithExtension(m_fileName);

    createProjectTree();
    if(saveProjectFile(m_fileName) && gui != nullptr)
    {
        TextFloat::displayMessage(
                tr("Project saved"),
                tr("The project %1 is now saved.").arg(m_fileName),
                embed::getIconPixmap("project_save", 24, 24), 2000);
        ConfigManager::inst()->addRecentlyOpenedProject(m_fileName);
        m_modified = false;
        gui->mainWindow()->resetWindowTitle();
    }
    else if(gui != nullptr)
    {
        TextFloat::displayMessage(
                tr("Project NOT saved."),
                tr("The project %1 was not saved!").arg(m_fileName),
                embed::getIconPixmap("error"), 4000);
        return false;
    }

    return true;
}

// save current song in given filename
bool Song::guiSaveProjectAs(const QString& _file_name)
{
    QString o     = m_oldFileName;
    m_oldFileName = m_fileName;
    m_fileName    = _file_name;
    if(guiSaveProject() == false)
    {
        m_fileName    = m_oldFileName;
        m_oldFileName = o;
        return false;
    }
    m_oldFileName = m_fileName;
    return true;
}

void Song::importProject()
{
    createProjectTree();

    FileDialog ofd(nullptr, tr("Import file"),
                   ConfigManager::inst()->userProjectsDir(),
                   tr("MIDI sequences") + " (*.mid *.midi *.rmi);;"
                           + tr("Hydrogen projects") + " (*.h2song);;"
                           + tr("Stems") + " (*.stem.mp4);;"
                           + tr("All file types") + " (*.*)");

    ofd.setFileMode(FileDialog::ExistingFile);
    if(ofd.exec() == QDialog::Accepted && !ofd.selectedFiles().isEmpty())
    {
        ImportFilter::import(ofd.selectedFiles()[0], this);
    }
    m_loadOnLaunch = false;
}

void Song::saveControllerStates(QDomDocument& doc, QDomElement& element)
{
    // save settings of controllers
    QDomElement controllersNode = doc.createElement("controllers");
    element.appendChild(controllersNode);
    for(int i = 0; i < m_controllers.size(); ++i)
    {
        m_controllers[i]->saveState(doc, controllersNode);
    }
}

void Song::restoreControllerStates(const QDomElement& element)
{
    QDomNode node = element.firstChild();
    while(!node.isNull() && !isCancelled())
    {
        Controller* c = Controller::create(node.toElement(), this);
        if(c)
        {
            addController(c);
        }
        else
        {
            // Fix indices to ensure correct connections
            m_controllers.append(
                    Controller::create(Controller::DummyController, this));
        }

        node = node.nextSibling();
    }
}

void Song::removeAllControllers()
{
    while(m_controllers.size() != 0)
    {
        removeController(m_controllers.at(0));
    }

    m_controllers.clear();
}

QString Song::projectDir()
{
    QString fname = m_fileName;
    if(fname.isEmpty())
    {
        qWarning("Song::projectDir no project filename");
        fname = ConfigManager::inst()->userProjectsDir() + QDir::separator()
                + tr("untitled") + ".mmp";
    }

    qInfo("Song::projectDir filename is %s", qPrintable(fname));
    QFileInfo fi(fname);
    QString   fs = fi.suffix().toLower();
    qInfo("Song::projectDir suffix is %s", qPrintable(fs));
    if((fs != "mmp") && (fs != "mmpz"))
    {
        qWarning("Song::projectDir invalid project suffix: %s",
                 qPrintable(fi.suffix()));
        fs = "mmp";
        fname += "." + fs;
        fi = QFileInfo(fname);
    }

    QString pname = fi.completeBaseName();
    qInfo("Song::projectDir pname is %s", qPrintable(pname));
    QString pdir = fi.absolutePath() + QDir::separator() + ".."
                   + QDir::separator() + "balls" + QDir::separator()
                   + pname;  // ~/lmms/balls/<prj>
    pdir = QFileInfo(pdir).absoluteFilePath();
    qInfo("Song::projectDir pdir is %s", qPrintable(pdir));
    return pdir;
}

bool Song::createProjectTree()
{
    QString r = projectDir();
    qInfo("Song::createProjectTree %s", qPrintable(r));
    QDir pdir(r);
    // pdir.mkpath("backup"  );
    pdir.mkpath("channels");
    pdir.mkpath("images");
    pdir.mkpath("midi");
    pdir.mkpath("project");
    pdir.mkpath("samples");
    pdir.mkpath("song");
    pdir.mkpath("stems");
    pdir.mkpath("tracks");
    pdir.mkpath("videos");
    pdir.mkpath("waveforms");

    pdir.mkpath(QString("channels") + QDir::separator() + "frozen");
    pdir.mkpath(QString("channels") + QDir::separator() + "rendered");
    pdir.mkpath(QString("midi") + QDir::separator() + "exported");
    pdir.mkpath(QString("project") + QDir::separator() + "backup");
    pdir.mkpath(QString("project") + QDir::separator() + "exported");
    pdir.mkpath(QString("song") + QDir::separator() + "rendered");
    pdir.mkpath(QString("stems") + QDir::separator() + "imported");
    pdir.mkpath(QString("stems") + QDir::separator() + "exported");
    pdir.mkpath(QString("tracks") + QDir::separator() + "frozen");
    pdir.mkpath(QString("tracks") + QDir::separator() + "rendered");
    pdir.mkpath(QString("videos") + QDir::separator() + "rendered");
    pdir.mkpath(QString("samples") + QDir::separator() + "used");
    pdir.mkpath(QString("waveforms") + QDir::separator() + "used");

    return true;
}

void Song::freeze()
{
    FxMixer* fxMixer = Engine::fxMixer();

    for(Track* t: tracks())
    {
        t->setFrozen(false);
        t->cleanFrozenBuffer();
    }

    for(FxChannel* ch: fxMixer->channels())
    {
        ch->setFrozen(false);
        ch->cleanFrozenBuffer();
    }

    ExportProjectDialog epd("/tmp/tmp." + SampleBuffer::rawStereoSuffix(),
                            gui->mainWindow(), false);
    epd.setWindowTitle(tr("Freeze tracks"));
    epd.fileFormatCB->setCurrentIndex(5);
    epd.bitrateCB->setCurrentIndex(0);
    epd.depthCB->setCurrentIndex(OutputSettings::Depth_F32);  // 4
    epd.stereoModeComboBox->setCurrentIndex(
            OutputSettings::StereoMode_Stereo);  // 0
    epd.interpolationCB->setCurrentIndex(2);
    epd.oversamplingCB->setCurrentIndex(0);
    epd.checkBoxVariableBitRate->setChecked(false);
    epd.exportLoopCB->setChecked(false);
    epd.renderMarkersCB->setChecked(false);
    epd.peakNormalizeCB->setChecked(false);

    switch(Engine::mixer()->baseSampleRate())
    {
        case 48000:
            epd.samplerateCB->setCurrentIndex(1);
            break;
        case 88200:
            epd.samplerateCB->setCurrentIndex(2);
            break;
        case 96000:
            epd.samplerateCB->setCurrentIndex(3);
            break;
        case 192000:
            epd.samplerateCB->setCurrentIndex(4);
            break;
        default:
            epd.samplerateCB->setCurrentIndex(0);
            break;
    }

    if(epd.exec() == QDialog::Accepted)
    {

        for(FxChannel* ch: fxMixer->channels())
        {
            ch->setFrozen(true);
            ch->cleanFrozenBuffer();
        }

        for(Track* t: tracks())
        {
            t->setFrozen(true);
            t->writeFrozenBuffer();
        }
    }
}

void Song::exportProjectChannels()
{
    qWarning("Song::exportProjectChannels not implemented yet");
    // exportProject( true );
}

void Song::exportProjectTracks()
{
    exportProject(true);
}

void Song::exportProject(bool multiExport)
{
    /*
    if( isEmpty() )
    {
            QMessageBox::information( gui->mainWindow(),
                            tr( "Empty project" ),
                            tr( "This project is empty so exporting makes "
                                    "no sense. Please put some items into "
                                    "Song Editor first!" ) );
            return;
    }
    */

    FileDialog efd(gui->mainWindow());

    if(multiExport)
    {
        efd.setFileMode(FileDialog::Directory);
        efd.setWindowTitle(
                tr("Select directory for writing exported tracks..."));
        if(!m_fileName.isEmpty())
        {
            createProjectTree();
            // QString rtdp= QFileInfo( m_fileName ).absolutePath();
            QString rtdp = projectDir() + QDir::separator() + "tracks"
                           + QDir::separator() + "rendered";
            efd.setDirectory(rtdp);
        }
    }
    else
    {
        efd.setFileMode(FileDialog::AnyFile);
        int         idx = 0;
        QStringList types;
        while(ProjectRenderer::fileEncodeDevices(idx).m_fileFormat
              != ProjectRenderer::NumFileFormats)
        {
            if(ProjectRenderer::fileEncodeDevices(idx).isAvailable())
            {
                types << /*tr*/ (ProjectRenderer::fileEncodeDevices(idx)
                                         .m_description);
            }
            ++idx;
        }
        efd.setNameFilters(types);
        QString baseFileName;
        if(!m_fileName.isEmpty())
        {
            createProjectTree();
            // QString rpdp= QFileInfo( m_fileName ).absolutePath();
            QString rpdp = projectDir() + QDir::separator() + "song"
                           + QDir::separator() + "rendered";
            efd.setDirectory(rpdp);
            baseFileName = QFileInfo(m_fileName).completeBaseName();
        }
        else
        {
            efd.setDirectory(ConfigManager::inst()->userProjectsDir());
            baseFileName = tr("untitled");
        }
        efd.selectFile(baseFileName
                       + ProjectRenderer::fileEncodeDevices(0).m_extension);
        efd.setWindowTitle(tr("Select file for project-export..."));
    }

    QString suffix = "wav";
    efd.setDefaultSuffix(suffix);
    efd.setAcceptMode(FileDialog::AcceptSave);

    if(efd.exec() == QDialog::Accepted && !efd.selectedFiles().isEmpty()
       && !efd.selectedFiles()[0].isEmpty())
    {

        QString exportFileName = efd.selectedFiles()[0];
        if(!multiExport)
        {
            int stx = efd.selectedNameFilter().indexOf("(*.");
            int etx = efd.selectedNameFilter().indexOf(")");

            if(stx > 0 && etx > stx)
            {
                // Get first extension from selected dropdown.
                // i.e. ".wav" from "WAV-File (*.wav), Dummy-File (*.dum)"
                suffix = efd.selectedNameFilter()
                                 .mid(stx + 2, etx - stx - 2)
                                 .split(" ")[0]
                                 .trimmed();
                exportFileName.remove("." + suffix, Qt::CaseInsensitive);
                if(efd.selectedFiles()[0].endsWith(suffix))
                {
                    if(VersionedSaveDialog::fileExistsQuery(
                               exportFileName + suffix, tr("Save project")))
                    {
                        exportFileName += suffix;
                    }
                }
            }
        }

        ExportProjectDialog epd(exportFileName, gui->mainWindow(),
                                multiExport);
        epd.exec();
    }
}

void Song::exportProjectMidi()
{
    /*
    if( isEmpty() )
    {
            QMessageBox::information( gui->mainWindow(),
                            tr( "Empty project" ),
                            tr( "This project is empty so exporting makes "
                                    "no sense. Please put some items into "
                                    "Song Editor first!" ) );
            return;
    }
    */

    FileDialog efd(gui->mainWindow());

    efd.setFileMode(FileDialog::AnyFile);

    QStringList types;
    types << tr("MIDI File (*.mid)");
    efd.setNameFilters(types);
    QString baseFileName;
    if(!m_fileName.isEmpty())
    {
        efd.setDirectory(QFileInfo(m_fileName).absolutePath());
        baseFileName = QFileInfo(m_fileName).completeBaseName();

        createProjectTree();
        // QString rmdp= QFileInfo( m_fileName ).absolutePath();
        QString rmdp = projectDir() + QDir::separator() + "midi"
                       + QDir::separator() + "exported";
        efd.setDirectory(rmdp);
        baseFileName = QFileInfo(m_fileName).completeBaseName();
    }
    else
    {
        efd.setDirectory(ConfigManager::inst()->userProjectsDir());
        baseFileName = tr("untitled");
    }
    efd.selectFile(baseFileName + ".mid");
    efd.setDefaultSuffix("mid");
    efd.setWindowTitle(tr("Select file for project-export..."));

    efd.setAcceptMode(FileDialog::AcceptSave);

    if(efd.exec() == QDialog::Accepted && !efd.selectedFiles().isEmpty()
       && !efd.selectedFiles()[0].isEmpty())
    {
        const QString suffix = ".mid";

        QString export_filename = efd.selectedFiles()[0];
        if(!export_filename.endsWith(suffix))
            export_filename += suffix;

        // NOTE start midi export

        // instantiate midi export plugin
        /*
          Tracks tracks;
          Tracks tracks_BB;
          tracks = Engine::getSong()->tracks();
          tracks_BB = Engine::getBBTrackContainer()->tracks();
        */
        ExportFilter* exf = dynamic_cast<ExportFilter*>(
                Plugin::instantiate("midiexport", nullptr, nullptr));
        if(exf == nullptr)
        {
            qWarning("Warning: Failed to load midi export filter");
            return;
        }
        // exf->tryExport(tracks, tracks_BB, getTempo(),
        // m_masterPitchModel.value(), export_filename);
        exf->proceed(export_filename);
    }
}

void Song::exportProjectVideoLine()
{
    /*
    if( isEmpty() )
    {
            QMessageBox::information( gui->mainWindow(),
                            tr( "Empty project" ),
                            tr( "This project is empty so exporting makes "
                                    "no sense. Please put some items into "
                                    "Song Editor first!" ) );
            return;
    }
    */

    FileDialog efd(gui->mainWindow());

    efd.setFileMode(FileDialog::AnyFile);

    QStringList types;
    types << tr("MP4 File (*.mp4)");
    efd.setNameFilters(types);
    QString baseFileName;
    if(!m_fileName.isEmpty())
    {
        createProjectTree();
        // QString rvldp= QFileInfo( m_fileName ).absolutePath();
        QString rvldp = projectDir() + QDir::separator() + "video"
                        + QDir::separator() + "rendered";
        efd.setDirectory(rvldp);
        baseFileName = "line";  // QFileInfo( m_fileName ).completeBaseName();
    }
    else
    {
        efd.setDirectory(ConfigManager::inst()->userProjectsDir());
        baseFileName = tr("untitled");
    }
    efd.selectFile(baseFileName + ".mp4");
    efd.setDefaultSuffix("mp4");
    efd.setWindowTitle(tr("Select file for export..."));

    efd.setAcceptMode(FileDialog::AcceptSave);

    if(efd.exec() == QDialog::Accepted && !efd.selectedFiles().isEmpty()
       && !efd.selectedFiles()[0].isEmpty())
    {
        const QString suffix = ".mp4";

        QString export_filename = efd.selectedFiles()[0];
        if(!export_filename.endsWith(suffix))
            export_filename += suffix;

        // instantiate videoline export plugin
        /*
        Tracks tracks;
        Tracks tracks_BB;
        QVector<QPair<tick_t,tick_t>> loops;

        tracks = Engine::getSong()->tracks();
        tracks_BB = Engine::getBBTrackContainer()->tracks();

        TimeLineWidget * tl = m_playPos[m_playMode].m_timeLine;
        if(tl)
        {
                for(int l=0;l<tl->NB_LOOPS;l++)
                  loops << QPair<tick_t,tick_t>(tl->loopBegin(l).getTicks(),
                                                tl->loopEnd(l).getTicks());
        }
        */
        ExportFilter* exf = dynamic_cast<ExportFilter*>(
                Plugin::instantiate("videolineexport", nullptr, nullptr));
        if(exf == nullptr)
        {
            qWarning("Warning: Failed to load videoline export filter");
            return;
        }
        // exf->tryExport(tracks, tracks_BB, getTempo(), loops,
        // export_filename);
        exf->proceed(export_filename);
    }
}

void Song::exportProjectVideoWave()
{
    /*
    if( isEmpty() )
    {
            QMessageBox::information( gui->mainWindow(),
                            tr( "Empty project" ),
                            tr( "This project is empty so exporting makes "
                                    "no sense. Please put some items into "
                                    "Song Editor first!" ) );
            return;
    }
    */

    FileDialog efd(gui->mainWindow());

    efd.setFileMode(FileDialog::AnyFile);

    QStringList types;
    types << tr("MP4 File (*.mp4)");
    efd.setNameFilters(types);
    QString baseFileName;
    if(!m_fileName.isEmpty())
    {
        createProjectTree();
        // QString rvldp= QFileInfo( m_fileName ).absolutePath();
        QString rvldp = projectDir() + QDir::separator() + "videos"
                        + QDir::separator() + "rendered";
        efd.setDirectory(rvldp);
        baseFileName = "wave";  // QFileInfo( m_fileName ).completeBaseName();
    }
    else
    {
        efd.setDirectory(ConfigManager::inst()->userProjectsDir());
        baseFileName = tr("untitled");
    }
    efd.selectFile(baseFileName + ".mp4");
    efd.setDefaultSuffix("mp4");
    efd.setWindowTitle(tr("Select file for export..."));

    efd.setAcceptMode(FileDialog::AcceptSave);

    if(efd.exec() == QDialog::Accepted && !efd.selectedFiles().isEmpty()
       && !efd.selectedFiles()[0].isEmpty())
    {
        const QString suffix = ".mp4";

        QString export_filename = efd.selectedFiles()[0];
        if(!export_filename.endsWith(suffix))
            export_filename += suffix;

        // instantiate videowave export plugin
        ExportFilter* exf = dynamic_cast<ExportFilter*>(
                Plugin::instantiate("videowaveexport", nullptr, nullptr));
        if(exf == nullptr)
        {
            qWarning("Warning: Failed to load videowave export filter");
            return;
        }
        exf->proceed(export_filename);
    }
}

void Song::exportProjectFormat12()
{
    FileDialog efd(gui->mainWindow());

    efd.setFileMode(FileDialog::AnyFile);

    QStringList types;
    types << tr("MMP File (*.mmp)");
    efd.setNameFilters(types);
    QString baseFileName;
    if(!m_fileName.isEmpty())
    {
        createProjectTree();
        // QString rvldp= QFileInfo( m_fileName ).absolutePath();
        QString rvldp = projectDir() + QDir::separator() + "project"
                        + QDir::separator() + "exported";
        efd.setDirectory(rvldp);
        baseFileName = QFileInfo(m_fileName).completeBaseName() + "_12";
    }
    else
    {
        efd.setDirectory(ConfigManager::inst()->userProjectsDir());
        baseFileName = tr("untitled") + "_12";
    }
    efd.selectFile(baseFileName + ".mmp");
    efd.setDefaultSuffix("mmp");
    efd.setWindowTitle(tr("Select file for export..."));

    efd.setAcceptMode(FileDialog::AcceptSave);

    if(efd.exec() == QDialog::Accepted && !efd.selectedFiles().isEmpty()
       && !efd.selectedFiles()[0].isEmpty())
    {
        const QString suffix = ".mmp";

        QString export_filename = efd.selectedFiles()[0];
        if(!export_filename.endsWith(suffix))
            export_filename += suffix;

        // instantiate oldformat export plugin
        ExportFilter* exf = dynamic_cast<ExportFilter*>(
                Plugin::instantiate("format12export", nullptr, nullptr));
        if(exf == nullptr)
        {
            qWarning("Warning: Failed to load format12 export filter");
            return;
        }
        exf->proceed(export_filename);
    }
}

void Song::updateFramesPerTick()
{
    Engine::updateFramesPerTick();
}

void Song::setModified()
{
    if(!m_loadingProject)
    {
        m_modified = true;
        m_metaData.set("Modified", QDateTime::currentDateTimeUtc().toString(
                                           "yyyy-MM-dd hh:mm:ss"));

        if(gui != nullptr && gui->mainWindow()
           && QThread::currentThread() == gui->mainWindow()->thread())
            gui->mainWindow()->resetWindowTitle();
    }
}

void Song::addController(Controller* controller)
{
    if(controller && !m_controllers.contains(controller))
    {
        m_controllers.append(controller);
        emit controllerAdded(controller);

        this->setModified();
    }
}

void Song::removeController(Controller* controller)
{
    int index = m_controllers.indexOf(controller);
    if(index != -1)
    {
        m_controllers.remove(index);

        emit controllerRemoved(controller);
        delete controller;

        this->setModified();
    }
}

void Song::clearErrors()
{
    m_errors.clear();
}

void Song::collectError(const QString error)
{
    m_errors.append(error);
}

bool Song::hasErrors()
{
    return (m_errors.length() > 0);
}

QString Song::errorSummary()
{
    QString errors = m_errors.join("\n") + '\n';

    errors.prepend("\n\n");
    errors.prepend(tr("The following errors occured while loading: "));

    return errors;
}

QString Song::songMetaData(const QString& k)
{
    return m_metaData.get(k);
}

void Song::setSongMetaData(const QString& k, const QString& v)
{
    if(m_metaData.set(k, v))
    {
        setModified();
        emit metaDataChanged(k, v);
    }
}

void Song::clearSongMetaData()
{
    if(m_metaData.clear())
    {
        setModified();
        emit metaDataChanged();
    }
}

void Song::userWorking()
{
    static qint64 last = 0;

    qint64 now = QDateTime::currentSecsSinceEpoch();
    qint64 d   = qMin<qint64>(30, now - last);
    last       = now;
    if(d > 0)
    {
        qint64 t = d + m_metaData.get("CumulativeWorkTime").toULongLong();
        m_metaData.set("CumulativeWorkTime", QString("%1").arg(t));
        // qInfo("CumulativeWorkTime %lld %lld", d, t);
    }
}

f_cnt_t Song::transportPosition()
{
    return currentFrame();
}

void Song::transportStart()
{
    if(!isPlaying())
    {
        if(isPaused())
            togglePause();
        else
            playSong();
    }
}

void Song::transportStop()
{
    if(isPlaying())
    {
        togglePause();
    }
}

void Song::transportLocate(f_cnt_t _frame)
{
    if(currentFrame() != _frame)
    {
        tick_t   t = _frame / Engine::framesPerTick();
        PlayPos& p = getPlayPos(playMode());
        p.setTicks(t);
        p.setCurrentFrame(_frame - t * Engine::framesPerTick());
        setToTime(p);
    }
}
