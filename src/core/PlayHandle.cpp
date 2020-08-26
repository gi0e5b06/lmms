/*
 * PlayHandle.cpp - base class PlayHandle - core of rendering engine
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

#include "PlayHandle.h"

#include "BufferManager.h"
#include "Engine.h"
#include "Mixer.h"

#include <QThread>
//#include <QDebug>
#include <QUuid>

#include <iterator>

PlayHandle::PlayHandle(const Type type, f_cnt_t offset) :
      // m_audioPort(nullptr),
      m_finished(false), m_type(type), m_offset(offset),
      m_processingLock(
              "PlayHandle::m_processingLock", QMutex::Recursive, false),
      m_usesBuffer(true), m_playHandleBuffer(nullptr),
      //m_bufferReleased(true),
      //m_refCount(0),
      m_pointer(nullptr)
{
    m_pointer = new PlayHandlePointer(this);
    // (((PlayHandlePointer*)m_pointer)->reset(this);
    // BufferManager::acquire()),
    m_debug_uuid = QUuid::createUuid().toString();
}

PlayHandle::~PlayHandle()
{
    //m_pointer->reset(nullptr);
    //delete ((PlayHandlePointer*)m_pointer);
}

void PlayHandle::setFinished()
{
    lock();
    if(/*m_refCount <= 0 &&*/ !m_finished)
    {
        releaseBuffer();
        m_finished = true;
        Engine::mixer()->emit playHandleToRemove(pointer());
    }
    unlock();
}

void PlayHandle::doProcessing()
{
    lock();
    if(m_usesBuffer)
    {
        if(m_playHandleBuffer == nullptr)
        {
            m_playHandleBuffer = BufferManager::acquire();
            //BufferManager::clear(m_playHandleBuffer); tmp needed?
        }
        play(m_playHandleBuffer);  // buffer());
    }
    else
    {
        play(nullptr);
    }
    unlock();
}

void PlayHandle::releaseBuffer()
{
    lock();
    if(m_playHandleBuffer != nullptr)
    {
        BufferManager::release(m_playHandleBuffer);
        m_playHandleBuffer = nullptr;
    }
    unlock();
}

sampleFrame* PlayHandle::buffer()
{
    return /*m_bufferReleased ? nullptr :*/
        m_playHandleBuffer;
    // reinterpret_cast<sampleFrame*>(m_playHandleBuffer);
}
