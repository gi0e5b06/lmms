/*
 * Piano.h - declaration of class Piano
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

#ifndef PIANO_H
#define PIANO_H

//#include "InstrumentTrack.h"
#include "Model.h"
#include "Note.h"

#include <QMutex>
#include <QPointer>

class InstrumentTrack;
class MidiEventProcessor;

class Piano : public Model
{
  public:
    enum KeyTypes
    {
        WhiteKey,
        BlackKey
    };

    Piano(InstrumentTrack* track);
    virtual ~Piano();

    void reset();
    bool isKeyPressed(int key) const;
    // void pressKey(int key);
    // void releaseKey(int key);

    void handleKeyPress(int key, uint8_t midiVelocity = -1);
    void handleKeyRelease(int key);
    void handleKeyPressure(int key, uint8_t midiVelocity);
    void handleKeyPanning(int key, uint8_t midiPanning);

    InstrumentTrack* instrumentTrack() const
    {
        return m_instrumentTrack;
    }

    /*
    MidiEventProcessor* midiEventProcessor() const
    {
        return m_midiEvProc;
    }

    void lock()
    {
        m_mutex.lock();
    }

    void unlock()
    {
        m_mutex.unlock();
    }
    */

    static bool isWhiteKey(int key);
    static bool isBlackKey(int key);

  private:
    /*
    static bool isValidKey(int key)
    {
        return key >= 0 && key < NumKeys;
    }
    */

    // QMutex              m_mutex;
    QPointer<InstrumentTrack> m_instrumentTrack;
    // MidiEventProcessor* m_midiEvProc;
    // int                 m_pressedKeys[NumKeys];
};

#endif
