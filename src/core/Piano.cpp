/*
 * Piano.cpp - implementation of piano-widget used in instrument-track-window
 *             for testing + according model class
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

/** \file Piano.cpp
 *  \brief A piano keyboard to play notes on in the instrument plugin window.
 */

/*
 * \mainpage Instrument plugin keyboard display classes
 *
 * \section introduction Introduction
 *
 * \todo fill this out
 * \todo write isWhite inline function and replace throughout
 */

#include "Piano.h"

#include "InstrumentTrack.h"

#include <QMutexLocker>

/*! The black / white order of keys as they appear on the keyboard.
 */
static const Piano::KeyTypes KEY_ORDER[] = {
        //	C                CIS              D                DIS
        Piano::WhiteKey, Piano::BlackKey, Piano::WhiteKey, Piano::BlackKey,
        //	E                F                FIS              G
        Piano::WhiteKey, Piano::WhiteKey, Piano::BlackKey, Piano::WhiteKey,
        //	GIS              A                AIS              B
        Piano::BlackKey, Piano::WhiteKey, Piano::BlackKey, Piano::WhiteKey};

/*! \brief Create a new keyboard display
 *
 *  \param _it the InstrumentTrack window to attach to
 */
Piano::Piano(InstrumentTrack* _track) :
      Model(_track, "Piano", false), /*!< base class ctor */
      // m_mutex(QMutex::Recursive),
      m_instrumentTrack(_track)
// m_midiEvProc(_track) /*!< the InstrumentTrack Model */
{
    reset();
}

/*! \brief Destroy this new keyboard display
 *
 */
Piano::~Piano()
{
    qInfo("Piano::~Piano");
    m_instrumentTrack = nullptr;
    // m_midiEvProc      = nullptr;
}

void Piano::reset()
{
    /*
    {
        QMutexLocker lock(&m_mutex);
        for(int i = 0; i < NumKeys; ++i)
            m_pressedKeys[i] = 0;
    }
    */
    for(int i = 0; i < NumKeys; ++i)
        // if(i < NumMidiKeys)
        // m_midiEvProc->processInEvent(MidiEvent(MidiNoteOff, -1, i, 0));
        if(isKeyPressed(i))
            handleKeyRelease(i);

    emit dataChanged();
}

/*! \brief Turn a key on or off
 *
 *  \param key the key number to change
 *  \param state the state to set the key to
 */
/*
void Piano::setKeyState(int key, bool state)
{
    bool changed = false;
    if(isValidKey(key))
    {
        QMutexLocker lock(&m_mutex);
        // if(m_pressedKeys[key] != state)
        if(state)
        {
            m_pressedKeys[key] = 1;  //++;
            changed            = (m_pressedKeys[key] == 1);
        }
        else if(m_pressedKeys[key] > 0)
        {
            m_pressedKeys[key]--;
            changed = (m_pressedKeys[key] == 0);
        }
    }
    // if(changed)
    Q_UNUSED(changed)
    emit dataChanged();
}
*/

/*
void Piano::pressKey(int key)
{
   bool changed = false;
   if(isValidKey(key))
   {
       QMutexLocker lock(&m_mutex);
       m_pressedKeys[key]++;
       if(m_pressedKeys[key] == 1)
           changed = true;
   }
   if(changed)
       emit dataChanged();
}

void Piano::releaseKey(int key)
{
   bool changed = false;
   if(isValidKey(key))
   {
       QMutexLocker lock(&m_mutex);
       if(m_pressedKeys[key] > 0)
       {
           m_pressedKeys[key]--;
           if(m_pressedKeys[key] == 0)
               changed = true;
       }
   }
   // if(changed)
   Q_UNUSED(changed)
   emit dataChanged();
}
*/

bool Piano::isKeyPressed(int _key) const
{
    /*
QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
return isValidKey(_key) ? (m_pressedKeys[_key] > 0) : false;
    */

    return m_instrumentTrack->isKeyPressed(_key);
}

/*! \brief Handle a note being pressed on our keyboard display
 *
 *  \param key the key being pressed
 */
void Piano::handleKeyPress(int key, uint8_t midiVelocity)
{
    /*
if(!isValidKey(key))
    return;

if(midiVelocity == -1)
    midiVelocity = m_instrumentTrack->midiPort()->baseVelocity();

bool changed = false;
{
    QMutexLocker lock(&m_mutex);
    m_pressedKeys[key]++;
    if(m_pressedKeys[key] == 1)
        changed = true;
}
if(changed)
{
    if(key < NumMidiKeys)
        m_midiEvProc->processInEvent(
                MidiEvent(MidiNoteOn, -1, key, midiVelocity));
    emit dataChanged();
}
    */
    // key -= m_instrumentTrack->baseNote() - DefaultKey;
    m_instrumentTrack->processInEvent(
            MidiEvent(MidiNoteOn, -1, key, midiVelocity));
    // emit dataChanged();
}

/*! \brief Handle a note being released on our keyboard display
 *
 *  \param key the key being releassed
 */
void Piano::handleKeyRelease(int key)
{
    /*
    if(!isValidKey(key))
        return;

    bool changed = false;
    {
        QMutexLocker lock(&m_mutex);
        if(m_pressedKeys[key] > 0)
        {
            m_pressedKeys[key]--;
            if(m_pressedKeys[key] == 0)
                changed = true;
        }
    }
    // if(changed)
    Q_UNUSED(changed)
    {
        if(key < NumMidiKeys)
            m_midiEvProc->processInEvent(MidiEvent(MidiNoteOff, -1, key, 0));
        emit dataChanged();
    }
    */

    m_instrumentTrack->processInEvent(MidiEvent(MidiNoteOff, -1, key, 0));
    // emit dataChanged();
}

void Piano::handleKeyPressure(int key, uint8_t midiVelocity)
{
    /*
   if(!isValidKey(key) || !isKeyPressed(key))
       return;

   if(midiVelocity == -1)
       midiVelocity = m_instrumentTrack->midiPort()->baseVelocity();

   if(key < NumMidiKeys)
       midiEventProcessor()->processInEvent(
               MidiEvent(MidiKeyPressure, -1, key, midiVelocity));
   */

    if(isKeyPressed(key))
    {
        m_instrumentTrack->processInEvent(
                MidiEvent(MidiKeyPressure, -1, key, midiVelocity));
    }
}

void Piano::handleKeyPanning(int key, uint8_t midiPanning)
{
    if(isKeyPressed(key))
    {
        MidiEvent event(MidiMetaEvent, -1, key, midiPanning);
        event.setMetaEvent(MidiNotePanning);
        m_instrumentTrack->processInEvent(event);
    }
}

bool Piano::isBlackKey(int key)
{
    int keyCode = key % KeysPerOctave;

    return KEY_ORDER[keyCode] == Piano::BlackKey;
}

bool Piano::isWhiteKey(int key)
{
    return !isBlackKey(key);
}
