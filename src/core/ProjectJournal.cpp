/*
 * ProjectJournal.cpp - implementation of ProjectJournal
 *
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "ProjectJournal.h"

#include "AutomationPattern.h"
#include "Engine.h"
#include "JournallingObject.h"
#include "Song.h"

#include <cstdlib>

static const int EO_ID_MSB = 1 << 23;

const int ProjectJournal::MAX_UNDO_STATES
        = 100;  // TODO: make this configurable in settings

ProjectJournal::ProjectJournal() :
      m_joIDs(), m_undoCheckPoints(), m_redoCheckPoints(),
      m_journalling(false)
{
}

ProjectJournal::~ProjectJournal()
{
    qInfo("ProjectJournal::~ProjectJournal");
}

void ProjectJournal::undo()
{
    while(!m_undoCheckPoints.isEmpty())
    {
        CheckPoint         c = m_undoCheckPoints.pop();
        JournallingObject* o = m_joIDs[c.joID];

        if(o != nullptr)
        {
            DataFile curState(DataFile::JournalData);
            o->saveState(curState, curState.content());
            m_redoCheckPoints.push(CheckPoint(c.joID, curState));

            bool prev = isJournalling();
            setJournalling(false);
            o->restoreState(c.data.content().firstChildElement());
            setJournalling(prev);
            Engine::song()->setModified();
            break;
        }
    }
}

void ProjectJournal::redo()
{
    while(!m_redoCheckPoints.isEmpty())
    {
        CheckPoint         c = m_redoCheckPoints.pop();
        JournallingObject* o = m_joIDs[c.joID];

        if(o != nullptr)
        {
            DataFile curState(DataFile::JournalData);
            o->saveState(curState, curState.content());
            m_undoCheckPoints.push(CheckPoint(c.joID, curState));

            bool prev = isJournalling();
            setJournalling(false);
            o->restoreState(c.data.content().firstChildElement());
            setJournalling(prev);
            Engine::song()->setModified();
            break;
        }
    }
}

bool ProjectJournal::canUndo() const
{
    return !m_undoCheckPoints.isEmpty();
}

bool ProjectJournal::canRedo() const
{
    return !m_redoCheckPoints.isEmpty();
}

void ProjectJournal::addJournalCheckPoint(JournallingObject* o)
{
    if(o == nullptr)
    {
        BACKTRACE
        return;
    }

    if(isJournalling())
    {
        /*
        for(auto id: m_joIDs.keys(nullptr))
            m_joIDs.remove(id);
        AutomationPattern::resolveAllIDs();
        qInfo("ProjectJournal::addJournalCheckPoint id=%d nb=%d", o->id(),
              m_joIDs.size());
        */

        m_redoCheckPoints.clear();

        DataFile dataFile(DataFile::JournalData);
        o->saveState(dataFile, dataFile.content());

        m_undoCheckPoints.push(CheckPoint(o->id(), dataFile));
        if(m_undoCheckPoints.size() > MAX_UNDO_STATES)
            m_undoCheckPoints.remove(0, m_undoCheckPoints.size()
                                                - MAX_UNDO_STATES);
    }
}

jo_id_t ProjectJournal::allocID(JournallingObject* _obj)
{
    jo_id_t id;
    for(jo_id_t tid = rand();
        m_joIDs.contains(id = ((tid % EO_ID_MSB) | EO_ID_MSB)); tid++)
    {
    }

    m_joIDs.insert(id, _obj);
    if(id == 2379397)
        qInfo("ProjectJournal::allocID %d %p", id, _obj);
    if(_obj == nullptr)
        BACKTRACE

    return id;
}

void ProjectJournal::reallocID(const jo_id_t _id, JournallingObject* _obj)
{
    if(_id == 2379397)
        qInfo("ProjectJournal::reallocID %d %p", _id, _obj);

    if(_obj == nullptr)
    {
        BACKTRACE
        return;
    }

    //	if( m_joIDs.contains( _id ) )
    m_joIDs.insert(_id, _obj);
}

void ProjectJournal::freeID(const jo_id_t _id)
{
    JournallingObject* o = m_joIDs.value(_id);
    if(_id == 2379397)
        qInfo("ProjectJournal::freeID %d %p", _id, o);

    if(o == nullptr)
    {
        BACKTRACE
        return;
    }

    //	if( m_joIDs.contains( _id ) )
    m_joIDs.insert(_id, nullptr);
}

jo_id_t ProjectJournal::idToSave(jo_id_t id)
{
    if(id == 2379397)
        qInfo("ProjectJournal::idToSave %d -> %d", id, id & ~EO_ID_MSB);
    return (id % EO_ID_MSB);  // & ~EO_ID_MSB;
}

jo_id_t ProjectJournal::idFromSave(jo_id_t id)
{
    if(id == 2379397)
        qInfo("ProjectJournal::idFromSave %d -> %d", id, id | EO_ID_MSB);
    return (id % EO_ID_MSB) | EO_ID_MSB;
}

void ProjectJournal::clearJournal()
{
    m_undoCheckPoints.clear();
    m_redoCheckPoints.clear();

    /*
    for(JoIdMap::Iterator it = m_joIDs.begin(); it != m_joIDs.end();)
    {
        if(it.value() == nullptr)
        {
            it = m_joIDs.erase(it);
        }
        else
        {
            ++it;
        }
    }
    */
    for(auto id: m_joIDs.keys(nullptr))
        m_joIDs.remove(id);
}

void ProjectJournal::stopAllJournalling()
{
    /*
    for(JoIdMap::Iterator it = m_joIDs.begin(); it != m_joIDs.end(); ++it)
        if(it.value() != nullptr)
            it.value()->setJournalling(false);
    */
    for(auto o: m_joIDs.values())
        if(o != nullptr)
            o->setJournalling(false);

    setJournalling(false);
}
