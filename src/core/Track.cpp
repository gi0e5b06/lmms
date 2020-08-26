/*
 * Track.cpp - implementation of classes concerning tracks -> necessary for
 *             all track-like objects (beat/bassline, sample-track...)
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

/** \file Track.cpp
 *  \brief All classes concerning tracks and track-like objects
 */

/*
 * \mainpage Track classes
 * \section introduction Introduction
 * \todo fill this out
 */

#include "Track.h"

//#include "AutomationEditor.h"
//#include "AutomationPattern.h"
#include "AutomationTrack.h"
//#include "BBEditor.h"
#include "BBTrack.h"
#include "BBTrackContainer.h"
//#include "CaptionMenu.h"
//#include "Clipboard.h"
//#include "ConfigManager.h"
#include "Engine.h"
//#include "GuiApplication.h"
//#include "FxMixerView.h"
#include "InstrumentTrack.h"
//#include "MainWindow.h"
#include "Mixer.h"
//#include "Pattern.h"
//#include "PixmapButton.h"
//#include "ProjectJournal.h"
#include "Backtrace.h"
//#include "RenameDialog.h"
#include "SampleTrack.h"
#include "Song.h"
//#include "SongEditor.h"
#include "StringPairDrag.h"
//#include "TextFloat.h"
#include "TimeLineWidget.h"
//#include "ToolTip.h"
//#include "debug.h"
//#include "embed.h"
//#include "gui_templates.h"

#include <QCoreApplication>
//#include <QColorDialog>
//#include <QHBoxLayout>
//#include <QMenu>
//#include <QMouseEvent>
//#include <QPainter>
//#include <QStyleOption>
#include <QUuid>

#include <cassert>
#include <cmath>

// ===========================================================================
// track
// ===========================================================================

/*! \brief Create a new (empty) track object
 *
 *  The track object is the whole track, linking its contents, its
 *  automation, name, type, and so forth.
 *
 * \param type The type of track (Song Editor or Beat+Bassline Editor)
 * \param tc The track Container object to encapsulate in this track.
 *
 * \todo check the definitions of all the properties - are they OK?
 */
Track::Track(TrackType type, TrackContainer* tc) :
      Model(tc, "Track"), /*!< The track Model */
      m_frozenModel(false, this, tr("Frozen"), "frozen"),
      /*!< For controlling track freezing */
      m_clippingModel(false, this, tr("Clipping"), "clipping"),
      /*!< For showing track clipping alerts */
      m_mutedModel(false, this, tr("Mute"), "mute"),
      /*!< For controlling track muting */
      m_soloModel(false, this, tr("Solo"), "solo"),
      /*!< For controlling track soloing */
      m_loopEnabledModel(false, this, tr("Loop Enabled"), "loopEnabled"),
      m_currentLoopModel(0,
                         0,
                         TimeLineWidget::NB_LOOPS - 1,
                         this,
                         tr("Current Loop"),
                         "currentLoop"),
      m_trackContainer(tc),  /*!< The track container object */
      m_type(type),          /*!< The track type */
      m_name(QString::null), /*!< The track's name */
      m_color(Qt::white), m_useStyleColor(true),
      m_simpleSerializingMode(false),
      m_tiles(), /*!< The track content objects (segments) */
      m_processingLock("Track::m_processingLock", QMutex::Recursive, false)
{
    m_height = -1;

    connect(&m_soloModel, SIGNAL(dataChanged()), this, SLOT(toggleSolo()));
    connect(&m_frozenModel, SIGNAL(dataChanged()), this,
            SLOT(toggleFrozen()));

    m_trackContainer->addTrack(this);
}

/*! \brief Destroy this track
 *
 *  If the track container is a Beat+Bassline container, step through
 *  its list of tracks and remove us.
 *
 *  Then delete the Tile's contents, remove this track from
 *  the track container.
 *
 *  Finally step through this track's automation and forget all of them.
 */
Track::~Track()
{
    qInfo("Track::~Track 0 [%s]", qPrintable(name()));
    // setMuted(true);  // <-- DEBUG
    if(isSolo())
        setSolo(false);
    disconnect(&m_soloModel, SIGNAL(dataChanged()), this, SLOT(toggleSolo()));
    disconnect(&m_frozenModel, SIGNAL(dataChanged()), this,
               SLOT(toggleFrozen()));
    lockTrack();
    qInfo("Track::~Track 1");
    clearAllTrackPlayHandles();
    emit destroyedTrack();
    qInfo("Track::~Track 2");
    deleteTCOs();
    qInfo("Track::~Track 3");
    m_trackContainer->removeTrack(this);
    unlockTrack();
    qInfo("Track::~Track 4");
}

QString Track::objectName() const
{
    QString r = QObject::objectName();
    if(r.isEmpty() || r == "track")
        r = QString("track%1").arg(trackIndex() + 1);
    return r;
}

bool Track::isFixed() const
{
    return m_trackContainer->isFixed();
}

QColor Track::color() const
{
    return m_color;
}

void Track::setColor(const QColor& _c)
{
    m_color = _c;
}

bool Track::useStyleColor() const
{
    return m_useStyleColor;
}

void Track::setUseStyleColor(bool b)
{
    m_useStyleColor = b;
}

int Track::currentLoop() const
{
    if(m_loopEnabledModel.value())
        return m_currentLoopModel.value();
    else
        return -1;
}

void Track::setCurrentLoop(int _loop)
{
    if(_loop < 0 || _loop >= TimeLineWidget::NB_LOOPS)
    {
        m_loopEnabledModel.setValue(false);
        return;
    }

    if(m_loopEnabledModel.value() && m_currentLoopModel.value() == _loop)
    {
        m_loopEnabledModel.setValue(false);
    }
    else
    {
        m_loopEnabledModel.setValue(true);
        m_currentLoopModel.setValue(_loop);
    }

    qInfo("Track: loop=%d enabled=%d", m_currentLoopModel.value(),
          m_loopEnabledModel.value());
}

/*
void Track::selectSubloop(const MidiTime& _pos)
{
    qWarning("Track::selectSubloop %d %s",_pos.getTicks(),qPrintable(name()));
}
*/

/*! \brief Create a track based on the given track type and container.
 *
 *  \param tt The type of track to create
 *  \param tc The track container to attach to
 */
Track* Track::create(TrackType tt, TrackContainer* tc)
{
    Engine::mixer()->requestChangeInModel();

    Track* t = nullptr;

    switch(tt)
    {
        case InstrumentTrack:
            t = new ::InstrumentTrack(tc);
            break;
        case BBTrack:
            t = new ::BBTrack(tc);
            break;
        case SampleTrack:
            t = new ::SampleTrack(tc);
            break;
            //		case EVENT_TRACK:
            //		case VIDEO_TRACK:
        case AutomationTrack:
            t = new ::AutomationTrack(tc);
            break;
        case HiddenAutomationTrack:
            t = new ::AutomationTrack(tc, true);
            break;
        default:
            break;
    }

    auto bbTC = Engine::getBBTrackContainer();
    if(tc == bbTC && t != nullptr)
        bbTC->updateAfterInnerTrackAdd(t);

    Engine::mixer()->doneChangeInModel();

    return t;
}

/*! \brief Create a track inside TrackContainer from track type in a
 * QDomElement and restore state from XML
 *
 *  \param element The QDomElement containing the type of track to create
 *  \param tc The track container to attach to
 */
Track* Track::create(const QDomElement& element, TrackContainer* tc)
{
    Engine::mixer()->requestChangeInModel();

    Track* t = create(
            static_cast<TrackType>(element.attribute("type").toInt()), tc);

    if(t != nullptr)
        t->restoreState(element);

    Engine::mixer()->doneChangeInModel();

    return t;
}

/*! \brief Clone a track from this track
 *
 */
Track* Track::clone()
{
    QDomDocument doc;
    QDomElement  parent = doc.createElement("clone");
    saveState(doc, parent);
    Track* r = create(parent.firstChild().toElement(), m_trackContainer);
    r->resetUuid();  // m_uuid = "";
    return r;
}

/*! \brief Save this track's settings to file
 *
 *  We save the track type and its muted state and solo state, then append the
 * track- specific settings.  Then we iterate through the Tiles
 *  and save all their states in turn.
 *
 *  \param doc The QDomDocument to use to save
 *  \param element The The QDomElement to save into
 *  \todo Does this accurately describe the parameters?  I think not!?
 *  \todo Save the track height
 */
void Track::saveSettings(QDomDocument& doc, QDomElement& element)
{
    if(!m_simpleSerializingMode)
        element.setTagName("track");

    element.setAttribute("type", type());
    element.setAttribute("name", name());
    // element.setAttribute("muted", isMuted());
    // element.setAttribute("solo", isSolo());
    // element.setAttribute("frozen", isFrozen());
    m_mutedModel.saveSettings(doc, element, "muted");
    m_soloModel.saveSettings(doc, element, "solo");
    m_frozenModel.saveSettings(doc, element, "frozen");

    element.setAttribute("color", color().rgb());
    element.setAttribute("usestyle", useStyleColor() ? 1 : 0);

    if(m_height >= MINIMAL_TRACK_HEIGHT && m_height != DEFAULT_TRACK_HEIGHT)
        element.setAttribute("trackheight", m_height);

    if(hasUuid())                              //! m_uuid.isEmpty())
        element.setAttribute("uuid", uuid());  // m_uuid);

    QDomElement tsDe = doc.createElement(nodeName());
    // let actual track (InstrumentTrack, bbTrack, sampleTrack etc.) save
    // its settings
    element.appendChild(tsDe);
    saveTrackSpecificSettings(doc, tsDe);

    if(m_simpleSerializingMode)
    {
        m_simpleSerializingMode = false;
        return;
    }

    // now save settings of all TCO's
    rearrangeAllTiles();
    for(Tile* tile: m_tiles)
        if(tile!=nullptr)
            tile->saveState(doc, element);

    // for(Tiles::const_iterator it = m_tiles.begin(); it != m_tiles.end();
    // ++it)
    //    (*it)->saveState(doc, element);
}

/*! \brief Load the settings from a file
 *
 *  We load the track's type and muted state and solo state, then clear out
 * our current Tile.
 *
 *  Then we step through the QDomElement's children and load the
 *  track-specific settings and Tiles states from it
 *  one at a time.
 *
 *  \param element the QDomElement to load track settings from
 *  \todo Load the track height.
 */
void Track::loadSettings(const QDomElement& element)
{
    if(element.attribute("type").toInt() != type())
    {
        qWarning(
                "Track::loadSettings Current track-type does not"
                " match track-type of settings-node!");
    }

    QString name = "";
    if(element.hasAttribute("name"))
    {
        name = element.attribute("name");
    }
    else
    {
        QDomElement e = element.firstChildElement();
        if(!e.isNull() && e.hasAttribute("name"))
            name = e.attribute("name");
    }
    m_name = name;
    // setName(name);

    // setMuted(element.attribute("muted").toInt());
    // setSolo(element.attribute("solo").toInt());
    // setFrozen(element.attribute("frozen").toInt());
    m_mutedModel.loadSettings(element, "muted");
    m_soloModel.loadSettings(element, "solo");
    m_frozenModel.loadSettings(element, "frozen");

    if(element.hasAttribute("color"))
        setColor(QColor(element.attribute("color").toUInt()));

    if(element.hasAttribute("usestyle"))
        setUseStyleColor(element.attribute("usestyle").toUInt() == 1);

    if(m_simpleSerializingMode)
    {
        QDomNode node = element.firstChild();
        while(!node.isNull())
        {
            if(node.isElement() && node.nodeName() == nodeName())
            {
                loadTrackSpecificSettings(node.toElement());
                break;
            }
            node = node.nextSibling();
        }
        m_simpleSerializingMode = false;
        return;
    }

    deleteTCOs();
    /*
    while(!m_tiles.empty())
    {
        delete m_tiles.front();
        // m_tiles.erase( m_tiles.begin() );
    }
    */

    QDomNode node = element.firstChild();
    while(!node.isNull())
    {
        if(node.isElement())
        {
            if(node.nodeName() == nodeName())
            {
                loadTrackSpecificSettings(node.toElement());
            }
            else if(node.nodeName() != "muted" && node.nodeName() != "solo"
                    && node.nodeName() != "frozen"
                    && !node.toElement().attribute("metadata").toInt())
            {
                Tile* tco = createTCO();
                tco->restoreState(node.toElement());
                saveJournallingState(false);
                restoreJournallingState();
            }
        }
        node = node.nextSibling();
    }

    if(element.hasAttribute("trackheight"))
    {
        int storedHeight = element.attribute("trackheight").toInt();
        qWarning("Track: storedH=%d", storedHeight);
        if(storedHeight >= MINIMAL_TRACK_HEIGHT)
            m_height = storedHeight;
        else
            m_height = MINIMAL_TRACK_HEIGHT;
    }
    else
    {
        m_height = DEFAULT_TRACK_HEIGHT;
    }
    // setFixedHeight(m_height);

    if(element.hasAttribute("uuid"))
    {
        if(hasUuid())  //! m_uuid.isEmpty())
            qWarning("Track::loadSettings this track already has an UUID");
        // m_uuid = element.attribute("uuid");
        setUuid(element.attribute("uuid"));
        if(isFrozen())
            readFrozenBuffer();
    }

    // if(!isFrozen()) cleanFrozenBuffer();
    rearrangeAllTiles();

    // auto bbTC = Engine::getBBTrackContainer();
    // if(trackContainer() == bbTC)  // && t != nullptr)
    //    bbTC->fixIncorrectPositions();
}

/*! \brief Add another Tile into this track
 *
 *  \param tco The Tile to attach to this track.
 */
void Track::addTCO(Tile* _tco)
{
    // qInfo("Track::addTCO tco=%p 'emit tileAdded'", _tco);
    if(!m_tiles.contains(_tco))
        m_tiles.append(_tco);
    else
        qInfo("Track::addTCO tco was already added");

    emit tileAdded(_tco);

    // return _tco;  // just for convenience
}

/*! \brief Remove a given Tile from this track
 *
 *  \param tco The Tile to remove from this track.
 */
void Track::removeTCO(Tile* _tco)
{
    /*
      Tiles::iterator it = qFind(m_tiles.begin(),
      m_tiles.end(), tco);
      if(it != m_tiles.end())
      {
      m_tiles.erase(it);
      if(Engine::song())
      {
      Engine::song()->updateLength();
      Engine::song()->setModified();
      }
      }
    */
    if(m_tiles.removeOne(_tco))
    {
        Song* song = Engine::song();
        if(song != nullptr)
        {
            song->updateLength();
            song->setModified();
        }

        // tmp test
        if(m_tiles.contains(_tco))
            qInfo("Track::removeTCO() tco was more than one");
    }
    else
        qInfo("Track::removeTCO() tco was not there");
}

/*! \brief Remove all TCOs from this track */
void Track::deleteTCOs()
{
    while(!m_tiles.isEmpty())
        delete m_tiles.first();
    // tile will remove itself
}

/*! \brief Return the number of Tiles we contain
 *
 *  \return the number of Tiles we currently contain.
 */
int Track::numOfTCOs() const
{
    // BACKTRACE
    return m_tiles.size();
}

/*! \brief Get a Tile by number
 *
 *  If the TCO number is less than our TCO array size then fetch that
 *  numbered object from the array.  Otherwise we warn the user that
 *  we've somehow requested a TCO that is too large, and create a new
 *  TCO for them.
 *  \param tcoNum The number of the Tile to fetch.
 *  \return the given Tile or a new one if out of range.
 *  \todo reject TCO numbers less than zero.
 *  \todo if we create a TCO here, should we somehow attach it to the
 *     track?
 */
Tile* Track::getTCO(int tcoNum) const
{
    if(tcoNum < m_tiles.size())
        return m_tiles[tcoNum];

    BACKTRACE
    qCritical(
            "Track::getTCO( %d ): "
            "TCO %d doesn't exist\n",
            tcoNum, tcoNum);
    return nullptr;  // createTCO( tcoNum * MidiTime::ticksPerTact() );
}

Tile* Track::tileForBB(int _bb) const
{
    MidiTime pos(_bb, 0);
    for(Tile* tile: m_tiles)
    {
        if(tile == nullptr)
            continue;
        if(tile->startPosition() == pos)
            return tile;
    }

    // BACKTRACE
    // qCritical("Track::tileForBB(%d): not found", _bb);
    return nullptr;
}

/*! \brief Determine the given Tile's number in our array.
 *
 *  \param tco The Tile to search for.
 *  \return its number in our array.
 */
int Track::getTCONum(const Tile* tco) const
{
    //	for( int i = 0; i < getTrackContentWidget()->numOfTCOs(); ++i )
    Tiles::iterator it = const_cast<Tiles::iterator>(
            qFind(m_tiles.begin(), m_tiles.end(), tco));
    if(it != m_tiles.end())
    {
        /*
          if( getTCO( i ) == _tco )
          {
          return i;
          }
        */
        return it - m_tiles.begin();
    }
    qCritical("Track::getTCONum(...): TCO not found!\n");
    return 0;
}

/*! \brief Retrieve a list of Tiles that fall within a period.
 *
 *  Here we're interested in a range of Tiles that intersect
 *  the given time period.
 *
 *  We return the TCOs we find in order by time, earliest TCOs first.
 *
 *  \param tcoV The list to contain the found Tiles.
 *  \param start The MIDI start time of the range.
 *  \param end   The MIDI endi time of the range.
 */
void Track::getTCOsInRange(Tiles&          tcoV,
                           const MidiTime& start,
                           const MidiTime& end) const
{
    for(Tile* tco: m_tiles)
    {
        if(tco == nullptr)
            continue;

        int s = tco->startPosition();
        int e = tco->endPosition();
        if((s < end) && (e > start))  // >=
        {
            // TCO is within given range
            // Insert sorted by TCO's position
            tcoV.insert(std::upper_bound(tcoV.begin(), tcoV.end(), tco,
                                         Tile::lessThan),
                        tco);
        }
    }
}

/*! \brief Swap the position of two Tiles.
 *
 *  First, we arrange to swap the positions of the two TCOs in the
 *  Tiles list.  Then we swap their start times as well.
 *
 *  \param tcoNum1 The first Tile to swap.
 *  \param tcoNum2 The second Tile to swap.
 */
/*
void Track::swapPositionOfTCOs(int tcoNum1, int tcoNum2)
{
    qSwap(m_tiles[tcoNum1], m_tiles[tcoNum2]);

    const MidiTime pos = m_tiles[tcoNum1]->startPosition();

    m_tiles[tcoNum1]->movePosition(m_tiles[tcoNum2]->startPosition());
    m_tiles[tcoNum2]->movePosition(pos);
}
*/

void Track::createTCOsForBB(int bb)
{
    Tile* tco = tileForBB(bb);
    if(tco != nullptr)
        return;

    qInfo("Track::createTCOsForBB bb=%d track=%s", bb, qPrintable(name()));
    saveJournallingState(false);
    MidiTime position = MidiTime(bb, 0);
    tco               = createTCO();
    tco->movePosition(position);
    restoreJournallingState();
    // tco->changeLength(MidiTime(1, 0));

    /*
    int i = 0;
    for(Tile* tile: m_tiles)
    {
        if(tile == nullptr)
        {
            MidiTime position = MidiTime(i, 0);
            Tile*    tco      = createTCO(position);
            tco->movePosition(position);
            tco->changeLength(MidiTime(1, 0));
        }
        i++;
    }
    m_tiles.removeAll(nullptr);
    while(numOfTCOs() < bb + 1)
    {
        MidiTime position = MidiTime(numOfTCOs(), 0);
        Tile*    tco      = createTCO(position);
        tco->movePosition(position);
        tco->changeLength(MidiTime(1, 0));
    }
    */
}

void Track::deleteTCOsForBB(int _bb)
{
    qInfo("Track::deleteTCOsForBB bb=%d track=%s", _bb, qPrintable(name()));
    MidiTime position = MidiTime(_bb, 0);
    bool     ok       = false;
    for(Tile* tco: getTCOs())
        if(tco->startPosition() == position)
        {
            delete tco;
            ok = true;
        }
    if(!ok)
        qInfo("Track::deleteTCOsForBB tile not found at tact %d", _bb);
}

void Track::moveTCOsForBB(int _fromBB, int _toBB)
{
    qInfo("Track::moveTCOsForBB %d -> %d", _fromBB, _toBB);
    MidiTime fromPos = MidiTime(_fromBB, 0);
    MidiTime toPos   = MidiTime(_toBB, 0);
    int      ok      = 0;
    for(Tile* tco: getTCOs())
        if(tco->startPosition() == fromPos)
        {
            tco->movePosition(toPos);
            ok++;
        }
    if(ok == 0)
        qInfo("Track::moveTCOsForBB tile not found at tact %d", _fromBB);
    else if(ok > 1)
        qInfo("Track::moveTCOsForBB too many tiles found at tact %d",
              _fromBB);
}

void Track::fixOverlappingTCOs()
{
    if(m_trackContainer != nullptr
       && m_trackContainer->type() == TrackContainer::BBContainer)
    {
        BACKTRACE
        qWarning("Track::fixOverlappingTCOs don't call on BBTC.");
        return;
    }

    MidiTime pos(0);
    // const int n = numOfTCOs();
    // for(int i = 0; i < n; ++i)
    for(Tile* tile: m_tiles)
    {
        // Tile* tile = getTCO(i);
        if(tile->startPosition() < pos)
            tile->movePosition(pos);
        pos = tile->endPosition();
    }
}

/*! \brief Move all the Tiles after a certain time later by one
 * bar.
 *
 *  \param pos The time at which we want to insert the bar.
 *  \todo if we stepped through this list last to first, and the list was
 *    in ascending order by TCO time, once we hit a TCO that was earlier
 *    than the insert time, we could fall out of the loop early.
 */
void Track::insertTact(const MidiTime& pos)
{
    // we'll increase the position of every TCO, positioned behind pos, by
    // one tact
    for(Tiles::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
    {
        if((*it)->startPosition() >= pos)
        {
            (*it)->movePosition((*it)->startPosition()
                                + MidiTime::ticksPerTact());
        }
    }
}

/*! \brief Move all the Tiles after a certain time earlier by
 * one bar.
 *
 *  \param pos The time at which we want to remove the bar.
 */
void Track::removeTact(const MidiTime& pos)
{
    // we'll decrease the position of every TCO, positioned behind pos, by
    // one tact
    for(Tiles::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
    {
        if((*it)->startPosition() >= pos)
        {
            (*it)->movePosition(qMax((*it)->startPosition().getTicks()
                                             - MidiTime::ticksPerTact(),
                                     0));
        }
    }
}

/*! \brief Return the length of the entire track in bars
 *
 *  We step through our list of TCOs and determine their end position,
 *  keeping track of the latest time found in ticks.  Then we return
 *  that in bars by dividing by the number of ticks per bar.
 */
tact_t Track::length() const
{
    // find last end-position
    tick_t last = 0;
    for(Tiles::const_iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
    {
        if(Engine::song()->isExporting() && (*it)->isMuted())
        {
            continue;
        }

        const tick_t cur = (*it)->endPosition();
        if(cur > last)
        {
            last = cur;
        }
    }

    return last / MidiTime::ticksPerTact();
}

/*! \brief Invert the track's solo state.
 *
 *  We have to go through all the tracks determining if any other track
 *  is already soloed.  Then we have to save the mute state of all tracks,
 *  and set our mute state to on and all the others to off.
 */
void Track::toggleSolo()
{
    qInfo("Track::toggleSolo");
    const Tracks& tl = m_trackContainer->tracks();

    bool soloBefore = false;
    for(Track* t: tl)
    {
        if(t != this)
        {
            if(t->m_soloModel.value())
            {
                soloBefore = true;
                break;
            }
        }
    }

    const bool solo = m_soloModel.value();
    for(Track* t: tl)
    {
        if(solo)
        {
            // save mute-state in case no track was solo before
            if(!soloBefore)
                t->m_mutedBeforeSolo = t->isMuted();

            t->setMuted(t == this ? false : true);
            if(t != this)
                t->m_soloModel.setValue(false);
        }
        else if(!soloBefore)
        {
            t->setMuted(t->m_mutedBeforeSolo);
        }
    }
}

void Track::toggleFrozen()
{
    // qInfo("Track::toggleFrozen");
}

void Track::cleanFrozenBuffer()
{
}

void Track::readFrozenBuffer()
{
}

void Track::writeFrozenBuffer()
{
}

void Track::clearAllTrackPlayHandles()
{
    if(QThread::currentThread() != thread())
        qWarning("Mixer::cleanHandles not in the main thread");

    if(Engine::mixer() != nullptr)
    {
        quint8 types = 0xFF;

        Engine::mixer()->emit playHandlesOfTypesToRemove(this, types);
        QCoreApplication::sendPostedEvents();
        QThread::yieldCurrentThread();
        Engine::mixer()->waitUntilNoPlayHandle(this, types);
    }
    else
        qInfo("Track::clearAllTrackPlayHandles no mixer");
}

void Track::rearrangeAllTiles()
{
    // sort notes by start time
    qSort(m_tiles.begin(), m_tiles.end(), Tile::lessThan);
}

int Track::trackIndex() const
{
    const TrackContainer* tc = trackContainer();
    if(tc == nullptr)
        return -1;
    return tc->tracks().indexOf(const_cast<Track*>(this));
}
