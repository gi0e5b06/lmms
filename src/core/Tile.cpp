/*
 * Tile.cpp -
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

/** \file Tile.cpp
 *  \brief base class for tiles
 */

/*
 * \mainpage Tile classes
 * \section introduction Introduction
 * \todo fill this out
 */

#include "Tile.h"

#include "AutomationEditor.h"
#include "AutomationPattern.h"
//#include "AutomationTrack.h"
//#include "BBEditor.h"
//#include "BBTrack.h"
//#include "BBTrackContainer.h"
//#include "CaptionMenu.h"
#include "Clipboard.h"
//#include "ConfigManager.h"
#include "Engine.h"
#include "GuiApplication.h"
//#include "FxMixerView.h"
//#include "MainWindow.h"
//#include "Mixer.h"
//#include "Pattern.h"
//#include "PixmapButton.h"
//#include "ProjectJournal.h"
#include "Backtrace.h"
//#include "RenameDialog.h"
//#include "SampleTrack.h"
#include "Song.h"
//#include "SongEditor.h"
//#include "StringPairDrag.h"
//#include "TextFloat.h"
//#include "TimeLineWidget.h"
//#include "ToolTip.h"
//#include "debug.h"
//#include "embed.h"
//#include "gui_templates.h"

//#include <QColorDialog>
//#include <QHBoxLayout>
//#include <QMenu>
//#include <QMouseEvent>
//#include <QPainter>
//#include <QStyleOption>
//#include <QUuid>

#include <cassert>
#include <cmath>

/*! \brief Create a new Tile
 *
 *  Creates a new track content object for the given track.
 *
 * \param _track The track that will contain the new object
 */
Tile::Tile(Track*         _track,
           const QString& _displayName,
           const QString& _objectName) :
      Model(_track, _displayName, _objectName),
      m_steps(DefaultStepsPerTact), m_stepResolution(DefaultStepsPerTact),
      m_track(_track), m_name(QString::null), m_startPosition(), m_length(),
      m_mutedModel(false, this, tr("Mute"), "mute"),
      // m_soloModel(false, this, tr("Solo"),"solo"),
      m_autoResize(false), m_autoRepeat(false), m_color(128, 128, 128),
      m_useStyleColor(true), m_selectViewOnCreate(false)
{
    /*
    if(Engine::song()&&Engine::song()->isPlaying())
    {
            static bool s_once=false;
            if(!s_once)
            {
                    BACKTRACE
                    s_once=true;
                    qWarning("Tile::Tile
    alloc...");
            }
    }
    */

    Track* t = track();
    if(t != nullptr)
        t->addTCO(this);
    // else
    // qWarning("Tile::Tile no track???");

    setJournalling(false);
    movePosition(0);
    changeLength(0);
    setJournalling(true);
}

Tile::Tile(const Tile& _other) :
      Model(_other.m_track, _other.displayName()), m_steps(_other.m_steps),
      m_stepResolution(_other.m_stepResolution), m_track(_other.m_track),
      m_name(_other.m_name), m_startPosition(_other.m_startPosition),
      m_length(_other.m_length),
      m_mutedModel(_other.m_mutedModel.value(), this, tr("Mute")),
      m_autoResize(_other.m_autoResize), m_autoRepeat(_other.m_autoRepeat),
      m_color(_other.m_color), m_useStyleColor(_other.m_useStyleColor),
      m_selectViewOnCreate(false)
{
    Track* t = track();
    if(t != nullptr)
        t->addTCO(this);
}

/*! \brief Destroy a Tile
 *
 *  Destroys the given track content object.
 *
 */
Tile::~Tile()
{
    qInfo("Tile::~Tile 1");
    // emit destroyedTCO();
    qInfo("Tile::~Tile 2");
    // blockSignals(true);

    Track* t = track();
    if(t != nullptr)
        t->removeTCO(this);
    qInfo("Tile::~Tile 3");
}

QString Tile::defaultName() const
{
    return track()->name();
}

void Tile::saveSettings(QDomDocument& doc, QDomElement& element)
{
    element.setAttribute("name", name());
    element.setAttribute("autoresize", autoResize() ? 1 : 0);
    element.setAttribute("autorepeat", autoRepeat() ? 1 : 0);
    element.setAttribute("pos", startPosition());
    element.setAttribute("len", length());
    element.setAttribute("muted", isMuted());
    element.setAttribute("color", color().rgb());
    element.setAttribute("usestyle", useStyleColor() ? 1 : 0);
}

void Tile::loadSettings(const QDomElement& element)
{
    if(element.hasAttribute("name"))
        setName(element.attribute("name"));

    if(element.hasAttribute("pos"))
        if(element.attribute("pos").toInt() >= 0)
            movePosition(element.attribute("pos").toInt());

    if(element.hasAttribute("len"))
        changeLength(element.attribute("len").toInt());

    if(element.hasAttribute("autoresize"))
        setAutoResize(element.attribute("autoresize").toInt());

    if(element.hasAttribute("autorepeat"))
        setAutoRepeat(element.attribute("autorepeat").toInt());

    if(element.hasAttribute("muted"))
        if(element.attribute("muted").toInt() != isMuted())
            toggleMute();

    if(element.hasAttribute("color"))
        setColor(QColor(element.attribute("color").toUInt()));

    if(element.hasAttribute("usestyle"))
        setUseStyleColor(element.attribute("usestyle").toUInt() == 1);
}

bool Tile::isFixed() const
{
    return track()->isFixed();
}

QColor Tile::color() const
{
    return m_color;
}

void Tile::setColor(const QColor& _c)
{
    m_color = _c;
}

bool Tile::useStyleColor() const
{
    return m_useStyleColor;
}

void Tile::setUseStyleColor(bool _b)
{
    m_useStyleColor = _b;
}

Tile* Tile::previousTile() const
{
    return adjacentTileByOffset(-1);
}

Tile* Tile::nextTile() const
{
    return adjacentTileByOffset(1);
}

Tile* Tile::adjacentTileByOffset(int offset) const
{
    m_track->lockTrack();
    m_track->rearrangeAllTiles();
    Tiles tcos   = m_track->getTCOs();
    int   tcoNum = m_track->getTCONum(this);
    Tile* r = /*dynamic_cast<Tile*>*/ (tcos.value(tcoNum + offset, nullptr));
    m_track->unlockTrack();
    return r;
}

/*! \brief Move this Tile's position in time
 *
 *  If the track content object has moved, update its position.  We
 *  also add a journal entry for undo and update the display.
 *
 * \param _pos The new position of the track content object.
 */
void Tile::movePosition(const MidiTime& pos)
{
    if(m_startPosition != pos)
    {
        m_startPosition = pos;
        Engine::song()->updateLength();
        emit positionChanged();
    }
}

/*! \brief Change the length of this Tile
 *
 *  If the track content object's length has chaanged, update it.  We
 *  also add a journal entry for undo and update the display.
 *
 * \param _length The new length of the track content object.
 */
void Tile::changeLength(const MidiTime& _length)
{
    // qInfo("Tile::changeLength #1");

    real_t nom          = Engine::song()->getTimeSigModel().getNumerator();
    real_t den          = Engine::song()->getTimeSigModel().getDenominator();
    int    ticksPerTact = DefaultTicksPerTact * (nom / den);

    tick_t len = qMax(static_cast<tick_t>(_length), ticksPerTact / 32);

    if(m_length != len)
    {
        m_length = len;
        m_steps  = len * m_stepResolution * stepsPerTact()
                  / MidiTime::ticksPerTact() / 16;
        // qInfo("Tile::changeLength #2");
        Engine::song()->updateLength();
        // qInfo("Tile::changeLength #3");
        emit lengthChanged();
    }
}

void Tile::updateLength()
{
    updateLength(length());
}

void Tile::updateLength(tick_t _len)
{
    if(isFixed())
    {
        if(m_stepResolution == 0)
            m_stepResolution = DefaultStepsPerTact;
        if(m_steps == 0)
            m_steps = m_stepResolution;
        _len = m_steps * MidiTime::ticksPerTact() * 16 / m_stepResolution
               / stepsPerTact();
    }

    changeLength(_len);
    // updateBBTrack();
}

void Tile::resizeLeft(const MidiTime& pos, const MidiTime& len)
{
    movePosition(pos);
    changeLength(len);
}

void Tile::resizeRight(const MidiTime& pos, const MidiTime& len)
{
    movePosition(pos);
    changeLength(len);
}

step_t Tile::stepsPerTact() const
{
    int steps = MidiTime::ticksPerTact() * DefaultStepsPerTact
                / DefaultTicksPerTact;
    return qMax(1, steps);
}

MidiTime Tile::stepPosition(step_t _step) const
{
    return _step * 16. / m_stepResolution * MidiTime::ticksPerTact()
           / stepsPerTact();
}

int Tile::stepResolution() const
{
    return m_stepResolution;
}

void Tile::setStepResolution(int _res)
{
    if(_res > 0)
    {
        int old = m_stepResolution;
        if(_res != old)
        {
            m_steps          = qMax(1, (m_steps * _res / old));
            m_stepResolution = _res;
            updateLength();
            emit dataChanged();
        }
    }
}

void Tile::addBarSteps()
{
    m_steps += stepsPerTact();
    updateLength();
    emit dataChanged();
    Engine::song()->setModified();
}

void Tile::addBeatSteps()
{
    m_steps += stepsPerTact()
               / Engine::song()->getTimeSigModel().getNumerator();
    updateLength();
    emit dataChanged();
    Engine::song()->setModified();
}

void Tile::addOneStep()
{
    m_steps++;  //= stepsPerTact();
    updateLength();
    emit dataChanged();
    Engine::song()->setModified();
}

void Tile::removeBarSteps()
{
    int n = stepsPerTact();
    if(n < m_steps)
    {
        /*
        for( int i = m_steps - n; i < m_steps; ++i )
        {
                setStep( i, false );
        }
        */
        m_steps -= n;
        updateLength();
        emit dataChanged();
        Engine::song()->setModified();
    }
}

void Tile::removeBeatSteps()
{
    int n = stepsPerTact() / Engine::song()->getTimeSigModel().getNumerator();
    if(n < m_steps)
    {
        /*
        for( int i = m_steps - n; i < m_steps; ++i )
        {
                setStep( i, false );
        }
        */
        m_steps -= n;
        updateLength();
        emit dataChanged();
        Engine::song()->setModified();
    }
}

void Tile::removeOneStep()
{
    int n = 1;
    if(n < m_steps)
    {
        /*
        for( int i = m_steps - n; i < m_steps; ++i )
        {
                setStep( i, false );
        }
        */
        m_steps -= n;
        updateLength();
        emit dataChanged();
        Engine::song()->setModified();
    }
}

void Tile::rotateOneStepLeft()
{
    rotate(-MidiTime::ticksPerTact() / stepsPerTact());
}

void Tile::rotateOneStepRight()
{
    rotate(MidiTime::ticksPerTact() / stepsPerTact());
}

void Tile::rotateOneBeatLeft()
{
    rotate(-MidiTime::ticksPerTact()
           / Engine::song()->getTimeSigModel().getNumerator());
}

void Tile::rotateOneBeatRight()
{
    rotate(MidiTime::ticksPerTact()
           / Engine::song()->getTimeSigModel().getNumerator());
}

void Tile::rotateOneBarLeft()
{
    rotate(-MidiTime::ticksPerTact());
}

void Tile::rotateOneBarRight()
{
    rotate(MidiTime::ticksPerTact());
}

void Tile::splitAfterEveryBar()
{
    splitEvery(MidiTime::ticksPerTact());
}

void Tile::splitAfterEveryFourthBar()
{
    splitEvery(4 * MidiTime::ticksPerTact());
}

/*
bool Tile::comparePosition(const Tile* a,
                                         const Tile* b)
{
    return a->startPosition() < b->startPosition();
}
*/

/*! \brief Copy this Tile to the clipboard.
 *
 *  Copies this track content object to the clipboard.
 */
void Tile::copy()
{
    Clipboard::copy(this);
}

/*! \brief Pastes this Tile into a track.
 *
 *  Pastes this track content object into a track.
 *
 * \param _je The journal entry to undo
 */
void Tile::paste()
{
    if(Clipboard::has(nodeName()))
    {
        const MidiTime pos = startPosition();
        Clipboard::paste(this);
        movePosition(pos);
    }
    /*
    if( Clipboard::getContent( nodeName() ) != nullptr )
    {
            const MidiTime pos = startPosition();
            restoreState( *( Clipboard::getContent( nodeName() ) ) );
            movePosition( pos );
    }
    */
    AutomationPattern::resolveAllIDs();
    gui->automationWindow()->m_editor->updateAfterPatternChange();
}

/*! \brief Mutes this Tile
 *
 *  Restore the previous state of this track content object.  This will
 *  restore the position or the length of the track content object
 *  depending on what was changed.
 *
 * \param _je The journal entry to undo
 */
void Tile::toggleMute()
{
    m_mutedModel.setValue(!m_mutedModel.value());
    emit dataChanged();
}
