/*
 * TrackContainer.cpp - implementation of base class for all trackcontainers
 *                      like Song-Editor, BB-Editor...
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

#include "TrackContainer.h"

#include "AutomationPattern.h"
#include "AutomationTrack.h"
#include "BBTrack.h"
#include "BBTrackContainer.h"
#include "GuiApplication.h"
#include "InstrumentTrack.h"
#include "MainWindow.h"
#include "Song.h"
#include "TextFloat.h"
#include "embed.h"

#include <QCoreApplication>
#include <QDomElement>
#include <QProgressDialog>
#include <QWriteLocker>

TrackContainer::TrackContainer(Model* _parent, const QString& _displayName) :
      Model(_parent, _displayName), JournallingObject(), m_tracksMutex(),
      m_tracks()
{
}

TrackContainer::~TrackContainer()
{
    qInfo("TrackContainer::~TrackContainer 1");
    clearAllTracks();
    qInfo("TrackContainer::~TrackContainer 2");
}

void TrackContainer::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    _this.setTagName(classNodeName());
    _this.setAttribute("type", nodeName());

    // save settings of each track
    m_tracksMutex.lockForRead();
    /*
    for(int i = 0; i < m_tracks.size(); ++i)
        m_tracks[i]->saveState(_doc, _this);
    */
    for(Track* t: m_tracks)
        t->saveState(_doc, _this);
    m_tracksMutex.unlock();
}

void TrackContainer::loadSettings(const QDomElement& _this)
{
    bool journalRestore = _this.parentNode().nodeName() == "journaldata";
    if(journalRestore)
        clearAllTracks();

    static QProgressDialog* pd       = nullptr;
    bool                    was_null = (pd == nullptr);
    if(gui && !journalRestore)
    {
        if(pd == nullptr)
        {
            pd = new QProgressDialog(
                    tr("Loading project..."), tr("Cancel"), 0,
                    Engine::getSong()->getLoadingTrackCount(),
                    gui->mainWindow());
            pd->setWindowModality(Qt::ApplicationModal);
            pd->setWindowTitle(tr("Please wait..."));
            if(gui->mainWindow() && gui->mainWindow()->isVisible())
                pd->show();
        }
    }

    QDomElement e = _this.firstChildElement("track");
    while(!e.isNull())
    {
        if(pd != nullptr)
        {
            pd->setValue(pd->value() + 1);
            QCoreApplication::instance()->processEvents(QEventLoop::AllEvents,
                                                        100);
            if(pd->wasCanceled())
            {
                if(gui)
                {
                    TextFloat::displayMessage(
                            tr("Loading cancelled"),
                            tr("Project loading was cancelled."),
                            embed::getIconPixmap("project_file", 24, 24),
                            2000);
                }
                Engine::getSong()->loadingCancelled();
                break;
            }
        }

        if(!e.attribute("metadata").toInt())
        {
            QString trackName
                    = e.hasAttribute("name")
                              ? e.attribute("name")
                              : e.firstChildElement().attribute("name");
            // qInfo("TrackContainer::loadSettings trackName=%s",
            //   qPrintable(trackName));

            if(pd != nullptr)
            {
                pd->setLabelText(
                        tr("Loading Track %1 (%2/Total %3)")
                                .arg(trackName)
                                .arg(pd->value() + 1)
                                .arg(Engine::getSong()
                                             ->getLoadingTrackCount()));
            }
            Track::create(e, this);
        }
        e = e.nextSiblingElement("track");
    }

    if(pd != nullptr)
    {
        if(was_null)
        {
            delete pd;
            pd = nullptr;
        }
    }
}

bool TrackContainer::hasTracks() const
{
    m_tracksMutex.lockForRead();
    bool r = m_tracks.size() > 0;
    m_tracksMutex.unlock();
    return r;
}

int TrackContainer::countTracks(Track::TrackType _tt) const
{
    int r = 0;
    m_tracksMutex.lockForRead();
    /*
    for(int i = 0; i < m_tracks.size(); ++i)
    {
        if(m_tracks[i]->type() == _tt || _tt == Track::NumTrackType)
            ++r;
    }
    */
    for(const Track* t: m_tracks)
        if(t->type() == _tt)
            r++;
    m_tracksMutex.unlock();
    return r;
}

void TrackContainer::addTrack(Track* _track)
{
    if(_track->type() != Track::HiddenAutomationTrack)
    {
        _track->lockTrack();
        m_tracksMutex.lockForWrite();
        if(m_tracks.contains(_track))
            qWarning("TrackContainer::addTrack already contains track");
        m_tracks.append(_track);
        m_tracksMutex.unlock();
        _track->unlockTrack();
        emit trackAdded(_track);
    }
}

void TrackContainer::removeTrack(Track* _track)
{
    // need a read locker to ensure that m_tracks doesn't change after reading
    // index.
    //   After checking that index != -1, we need to upgrade the lock to a
    //   write locker before changing m_tracks. But since Qt offers no
    //   function to promote a read lock to a write lock, we must start with
    //   the write locker.
    /*
    QWriteLocker lockTracksAccess(&m_tracksMutex);
    int          index = m_tracks.indexOf(_track);
    if(index != -1)
    {
        // If the track is solo, all other tracks are muted. Change this
        // before removing the solo track:
        if(_track->isSolo())
        {
            _track->setSolo(false);
        }
        m_tracks.remove(index);
        lockTracksAccess.unlock();

        if(Engine::getSong())
        {
            Engine::getSong()->setModified();
        }
    }
    */

    if(_track->trackContainer() == this)
    {
        if(_track->isSolo())
        {
            qInfo("TrackContainer::removeTrack before solo");
            _track->setSolo(false);
            qInfo("TrackContainer::removeTrack after solo");
        }

        m_tracksMutex.lockForWrite();
        int nbt = m_tracks.removeAll(_track);
        if(nbt > 1)
            qCritical("TrackContainer::removeTrack more than once");
        bool modified = (nbt > 0);
        m_tracksMutex.unlock();

        if(modified)
        {
            Song* song = Engine::getSong();
            if(song != nullptr)
                song->setModified();
        }
    }
    else
        qWarning(
                "TrackContainer::removeTrack trying to remove a track that "
                "doesn't belong to this container");
}

void TrackContainer::updateAfterTrackAdd()
{
}

void TrackContainer::clearAllTracks()
{
    qInfo("TrackContainer::clearAllTracks START");
    Tracks tl = m_tracks;
    for(Track* t: tl)  // while(!m_tracks.isEmpty())
    {
        // qInfo("TrackContainer::clearAllTracks last");
        // Track* t = m_tracks.last();
        qInfo("TrackContainer::clearAllTracks delete track [%s]",
              qPrintable(t->name()));
        if(t->type() == Track::HiddenAutomationTrack)
            qInfo(" *** is HiddenAutomationTrack");
        delete t;
        qInfo("TrackContainer::clearAllTracks after delete");
    }
    qInfo("TrackContainer::clearAllTracks END");
}

bool TrackContainer::isEmpty() const
{
    /*
    for(Tracks::const_iterator it = m_tracks.begin(); it != m_tracks.end();
        ++it)
    {
        if(!(*it)->getTCOs().isEmpty())
        {
            return false;
        }
    }
    return true;
    */

    for(const Track* t: m_tracks)
        if(!t->getTCOs().isEmpty())
            return false;

    return true;
}

// AutomatedValueMap
void TrackContainer::automatedValuesAt(MidiTime           time,
                                       int                tcoNum,
                                       AutomatedValueMap& _map) const
{
    // return
    automatedValuesFromTracks(tracks(), time, tcoNum, _map);
}

// AutomatedValueMap
void TrackContainer::automatedValuesFromTracks(const Tracks&      tracks,
                                               MidiTime           time,
                                               int                tcoNum,
                                               AutomatedValueMap& _map)
{
    Tiles tcos;

    for(Track* track: tracks)
    {
        if(track->isMuted())
            continue;

        switch(track->type())
        {
            case Track::AutomationTrack:
            case Track::HiddenAutomationTrack:
            case Track::BBTrack:
                if(tcoNum < 0)
                {
                    track->getTCOsInRange(tcos, 0, time);
                }
                else
                {
                    Q_ASSERT(track->numOfTCOs() > tcoNum);
                    tcos << track->getTCO(tcoNum);
                }
            default:
                break;
        }
    }

    // AutomatedValueMap valueMap;

    // Q_ASSERT(std::is_sorted(tcos.begin(), tcos.end(),
    // Tile::comparePosition));

    if(!std::is_sorted(tcos.begin(), tcos.end(), Tile::lessThan))
    {
        qCritical(
                "Error: fail assert: is_sorted(tcos.begin(), tcos.end()) "
                "%s#%d",
                __FILE__, __LINE__);
        return;  // valueMap;
    }

    for(Tile* tco: tcos)
    {
        if(tco->isMuted() || tco->startPosition() > time
           || tco->endPosition() < time)
            continue;

        if(auto* p = dynamic_cast<AutomationPattern*>(tco))
        {
            if(!p->hasAutomation())
            {
                continue;
            }
            MidiTime relTime = time - p->startPosition();
            /*
            if (! p->autoResize()) {
                    relTime = qMin(relTime, p->length());
            }
            */
            if(p->isFixed())
                relTime = relTime % p->length();
            else if(relTime >= p->length())
                continue;

            // real_t value = p->valueAt(relTime);
            // for(AutomatableModel* model: p->objects())
            p->automatedValuesAt(relTime, _map);
        }
        else if(auto* bb = dynamic_cast<BBTCO*>(tco))
        {
            auto bbIndex = dynamic_cast<class BBTrack*>(bb->track())->index();
            auto bbContainer = Engine::getBBTrackContainer();
            MidiTime bbTime  = time - tco->startPosition();
            // bbTime = std::min(bbTime, tco->length());
            bbTime = bbTime % tco->length();
            bbTime = bbTime
                     % (bbContainer->lengthOfBB(bbIndex)
                        * MidiTime::ticksPerTact());

            bbContainer->automatedValuesAt(bbTime, bbIndex, _map);
            /*
            auto bbValues = bbContainer->automatedValuesAt(bbTime, bbIndex);
            for (auto it=bbValues.begin(); it != bbValues.end(); it++)
            {
                    // override old values, bb track with the highest index
            takes precedence
                    //valueMap[it.key()] = it.value();
                    //valueMap.insert(it.key(),it.value());
                    _map.insert(it.key(),it.value());
            }
            */
        }
        else
        {
            continue;
        }
    }

    return;  // valueMap;
}

void TrackContainer::automatedValuesFromTrack(const Track*       _track,
                                              MidiTime           time,
                                              int                tcoNum,
                                              AutomatedValueMap& _map)
{
    if(_track->isMuted())
        return;

    Tiles tcos;

    switch(_track->type())
    {
        case Track::AutomationTrack:
        case Track::HiddenAutomationTrack:
        case Track::BBTrack:
            if(tcoNum < 0)
            {
                _track->getTCOsInRange(tcos, 0, time);
            }
            else
            {
                Q_ASSERT(_track->numOfTCOs() > tcoNum);
                tcos << _track->getTCO(tcoNum);
            }
        default:
            break;
    }

    // AutomatedValueMap valueMap;

    // Q_ASSERT(std::is_sorted(tcos.begin(), tcos.end(),
    // Tile::comparePosition));

    if(!std::is_sorted(tcos.begin(), tcos.end(), Tile::lessThan))
    {
        qCritical(
                "Error: fail assert: is_sorted(tcos.begin(), tcos.end()) "
                "%s#%d",
                __FILE__, __LINE__);
        return;
    }

    for(Tile* tco: tcos)
    {
        if(tco->isMuted() || tco->startPosition() > time)
        {
            continue;
        }

        if(auto* p = dynamic_cast<AutomationPattern*>(tco))
        {
            if(!p->hasAutomation())
            {
                continue;
            }
            MidiTime relTime = time - p->startPosition();
            /*
            if (! p->autoResize()) {
                    relTime = qMin(relTime, p->length());
            }
            */
            if(p->isFixed())
                relTime = relTime % p->length();
            else if(relTime >= p->length())
                continue;

            // real_t value = p->valueAt(relTime);
            // for(AutomatableModel* model: p->objects())
            // _map.insert(model, value);
            p->automatedValuesAt(relTime, _map);
        }
        else if(auto* bb = dynamic_cast<BBTCO*>(tco))
        {
            auto bbIndex = dynamic_cast<class BBTrack*>(bb->track())->index();
            auto bbContainer = Engine::getBBTrackContainer();
            MidiTime bbTime  = time - tco->startPosition();
            // bbTime = std::min(bbTime, tco->length());
            bbTime = bbTime % tco->length();
            bbTime = bbTime
                     % (bbContainer->lengthOfBB(bbIndex)
                        * MidiTime::ticksPerTact());

            bbContainer->automatedValuesAt(bbTime, bbIndex, _map);
            /*
            auto bbValues = bbContainer->automatedValuesAt(bbTime,
            bbIndex); for (auto it=bbValues.begin(); it != bbValues.end();
            it++)
            {
                    // override old values, bb track with the highest
            index takes precedence
                    //valueMap[it.key()] = it.value();
                    _map.insert(it.key(),it.value());
            }
            */
        }
        else
        {
            continue;
        }
    }
}

DummyTrackContainer::DummyTrackContainer() :
      TrackContainer(nullptr, "Dummy track container"),
      m_dummyInstrumentTrack(nullptr)
{
    setJournalling(false);
    m_dummyInstrumentTrack = dynamic_cast<InstrumentTrack*>(
            Track::create(Track::InstrumentTrack, this));
    m_dummyInstrumentTrack->setJournalling(false);
}

DummyTrackContainer::~DummyTrackContainer()
{
    qInfo("DummyTrackContainer::~DummyTrackContainer");
}
