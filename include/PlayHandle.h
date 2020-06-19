/*
 * PlayHandle.h - base class PlayHandle - core of rendering engine
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

#ifndef PLAY_HANDLE_H
#define PLAY_HANDLE_H

#include "MemoryManager.h"
#include "Mutex.h"
#include "SafeList.h"
#include "ThreadableJob.h"
#include "lmms_basics.h"

#include <QList>

class QThread;

class Track;
class Instrument;
class AudioPort;

class PlayHandle : public ThreadableJob
{
    MM_OPERATORS

  public:
    QString m_debug_uuid;

    enum Types
    {
        TypeNotePlayHandle       = 0x01,
        TypeInstrumentPlayHandle = 0x02,
        TypeSamplePlayHandle     = 0x04,
        TypePresetPreviewHandle  = 0x08
    };
    typedef Types Type;

    enum
    {
        MaxNumber = 1024
    };

    virtual ~PlayHandle();

    Type type() const
    {
        return m_type;
    }

    inline AudioPort* audioPort() const
    {
        return m_audioPort;
    }

    inline void setAudioPort(AudioPort* port)
    {
        m_audioPort = port;
    }

    // required for ThreadableJob
    virtual void doProcessing();

    virtual bool requiresProcessing() const
    {
        return !isFinished();
    }

    void lock()
    {
        m_processingLock.lock();
    }

    void unlock()
    {
        m_processingLock.unlock();
    }

    bool tryLock()
    {
        return m_processingLock.tryLock();
    }

    virtual void play(sampleFrame* buffer)                             = 0;
    virtual bool isFromTrack(const Track* _track) const                = 0;
    virtual bool isFromInstrument(const Instrument* _instrument) const = 0;
    virtual bool isFinished() const                                    = 0;
    virtual void setFinished() final;

    // returns the frameoffset at the start of the playhandle,
    // ie. how many empty frames should be inserted at the start of
    // the first period
    virtual f_cnt_t offset() const final
    {
        return m_offset;
    }

    virtual void setOffset(f_cnt_t _offset) final
    {
        m_offset = _offset;
    }

    virtual bool usesBuffer() const final
    {
        return m_usesBuffer;
    }

    virtual void setUsesBuffer(const bool b) final
    {
        m_usesBuffer = b;
    }

    virtual void releaseBuffer() final;

    virtual sampleFrame* buffer() final;

    virtual void incrRefCount() final
    {
        m_refCount++;
    }

    virtual void decrRefCount() final
    {
        if(m_refCount > 0)
            m_refCount--;
        else
            qWarning("Warning: playHandle negative refCount");
    }

    virtual void resetRefCount() final
    {
        m_refCount = 0;
    }

  protected:
    PlayHandle(const Type type, f_cnt_t offset = 0);

    /*
    PlayHandle& operator=(PlayHandle& p)
    {
        m_type       = p.m_type;
        m_offset     = p.m_offset;
        m_usesBuffer = p.m_usesBuffer;
        m_audioPort  = p.m_audioPort;
        return *this;
    }
    */

    AudioPort* m_audioPort;
    bool       m_finished;

  private:
    Type         m_type;
    f_cnt_t      m_offset;
    Mutex        m_processingLock;
    bool         m_usesBuffer;
    sampleFrame* m_playHandleBuffer;
    bool         m_bufferReleased;
    int          m_refCount;
};

// typedef QList<PlayHandle*>       PlayHandleList;
// typedef QList<const PlayHandle*> ConstPlayHandleList;
typedef SafeList<PlayHandle*>       PlayHandleList;
typedef SafeList<const PlayHandle*> ConstPlayHandleList;

#endif
