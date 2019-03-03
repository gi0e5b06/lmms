/*
 * MidiClient.cpp - base-class for MIDI-clients like ALSA-sequencer-client
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * This file partly contains code from Fluidsynth, Peter Hanappe
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

//#include <QObject>

#include "MidiClient.h"

#include "MidiPort.h"
#include "Note.h"

MidiClient::MidiClient()
{
}

MidiClient::~MidiClient()
{
    // TODO: noteOffAll(); / clear all ports
    for(MidiPort* port: m_midiPorts)
        port->invalidateClient();
}

void MidiClient::applyPortMode(MidiPort*)
{
}

void MidiClient::applyPortName(MidiPort*)
{
}

void MidiClient::addPort(MidiPort* port)
{
    // m_midiPorts.push_back( port );
    if(m_midiPorts.contains(port))
        qWarning("MidiClient::addPort MidiPort already registered");
    m_midiPorts.append(port);
}

void MidiClient::removePort(MidiPort* port)
{
    if(!port)
    {
        return;
    }

    /*
    QVector<MidiPort *>::Iterator it =
            qFind( m_midiPorts.begin(), m_midiPorts.end(), port );
    if( it != m_midiPorts.end() )
    {
            m_midiPorts.erase( it );
    }
    */
    m_midiPorts.removeOne(port);
}

void MidiClient::subscribeReadablePort(MidiPort*, const QString&, bool)
{
}

void MidiClient::subscribeWritablePort(MidiPort*, const QString&, bool)
{
}

void MidiClient::processOutEventOnAllPorts(const MidiEvent& _me,
                                           const MidiTime&  _time)
{
    for(MidiPort* port: m_midiPorts)
        processOutEvent(_me, _time, port);
}

void MidiClient::sendMTC()
{
    Song* song = Engine::getSong();
    if(song != nullptr)
    {
        int    rr = CONFIG_GET_INT("midi.mtc_video_fps");
        double vfps;
        switch(rr)
        {
            case 0:
                vfps = 24.;
                break;
            case 1:
                vfps = 25.;
                break;
            case 3:
                vfps = 30.;
                break;
            default:
                rr   = 2;
                vfps = 29.97;
                break;
        }

        /*
        if(vfps != 30.f)
            tt = tt * (vfps / 30.f);  // REQUIRED
        */

        int     tt = song->getMilliseconds();
        uint8_t ff = ((tt % 1000) * vfps) / 1000;
        uint8_t ss = (tt / 1000) % 60;
        uint8_t mm = (tt / 60000) % 60;
        uint8_t hh = (tt / 3600000) % 24;

        static int previous = -1;
        const int  present  = ((hh * 60 + mm) * 60 + ss) * 100 + ff;
        static int part     = 0;
        if(previous != present)
        {
            if((previous + 1 != present)
               && (ff == 0 && previous / 100 + 1 != present / 100))
            {
                qInfo("Full MTC %02d:%02d:%02d.%02d", hh, mm, ss, ff);
                uint8_t bytes[10] = {0xF0, 0x7F, 0x7F, 0x01, 0x01,
                                     hh,   mm,   ss,   ff,   0xF7};
                for(MidiPort* port: m_midiPorts)
                    if(port->mode() == MidiPort::Output
                       || port->mode() == MidiPort::Duplex)
                        sendBytes(bytes, 10, 0, port);
                part = 0;
            }
            int maxi = ((!song->isPlaying() && part == 0) ? 8 : 1);
            for(int i = 0; i < maxi; i++)
            {
                qInfo("Part %d MTC %02d:%02d:%02d.%02d (%f vfps)", part, hh,
                      mm, ss, ff, vfps);
                uint8_t data = part << 4;
                switch(part)
                {
                    case 0:
                        data |= ff % 16;
                        break;
                    case 1:
                        data |= ff / 16;
                        break;
                    case 2:
                        data |= ss % 16;
                        break;
                    case 3:
                        data |= ss / 16;
                        break;
                    case 4:
                        data |= mm % 16;
                        break;
                    case 5:
                        data |= mm / 16;
                        break;
                    case 6:
                        data |= hh % 16;
                        break;
                    case 7:
                        data |= (rr < 1) | hh / 16;
                        break;
                }
                uint8_t bytes[2] = {0xF1, data};
                for(MidiPort* port: m_midiPorts)
                    if(port->mode() == MidiPort::Output
                       || port->mode() == MidiPort::Duplex)
                        sendBytes(bytes, 2, 0, port);
                part++;
                if(part == 8)
                    part = 0;
            }
        }
        previous = present;
    }
}

MidiClientRaw::MidiClientRaw()
{
}

MidiClientRaw::~MidiClientRaw()
{
}

void MidiClientRaw::parseData(const unsigned char c)
{
    /*********************************************************************/
    /* 'Process' system real-time messages                               */
    /*********************************************************************/
    /* There are not too many real-time messages that are of interest here.
     * They can occur anywhere, even in the middle of a noteon message!
     * Real-time range: 0xF8 .. 0xFF
     * Note: Real-time does not affect (running) status.
     */
    if(c >= 0xF8)
    {
        if(c == MidiSystemReset)
        {
            m_midiParseData.m_midiEvent.setType(MidiSystemReset);
            m_midiParseData.m_status = 0;
            processParsedEvent();
        }
        return;
    }

    /*********************************************************************/
    /* 'Process' system common messages (again, just skip them)          */
    /*********************************************************************/
    /* There are no system common messages that are of interest here.
     * System common range: 0xF0 .. 0xF7
     */
    if(c > 0xF0)
    {
        /* MIDI spec say: To ignore a non-real-time message, just discard all
         * data up to the next status byte.  And our parser will ignore data
         * that is received without a valid status.
         * Note: system common cancels running status. */
        m_midiParseData.m_status = 0;
        return;
    }

    /*********************************************************************/
    /* Process voice category messages:                                  */
    /*********************************************************************/
    /* Now that we have handled realtime and system common messages, only
     * voice messages are left.
     * Only a status byte has bit # 7 set.
     * So no matter the status of the parser (in case we have lost sync),
     * as soon as a byte >= 0x80 comes in, we are dealing with a status byte
     * and start a new event.
     */
    if(c & 0x80)
    {
        m_midiParseData.m_channel = c & 0x0F;
        m_midiParseData.m_status  = c & 0xF0;
        /* The event consumes x bytes of data
                                (subtract 1 for the status byte) */
        m_midiParseData.m_bytesTotal
                = eventLength(m_midiParseData.m_status) - 1;
        /* of which we have read 0 at this time. */
        m_midiParseData.m_bytes = 0;
        return;
    }

    /*********************************************************************/
    /* Process data                                                      */
    /*********************************************************************/
    /* If we made it this far, then the received char belongs to the data
     * of the last event. */
    if(m_midiParseData.m_status == 0)
    {
        /* We are not interested in the event currently received.
                                                Discard the data. */
        return;
    }

    /* Store the first couple of bytes */
    if(m_midiParseData.m_bytes < RAW_MIDI_PARSE_BUF_SIZE)
    {
        m_midiParseData.m_buffer[m_midiParseData.m_bytes] = c;
    }
    ++m_midiParseData.m_bytes;

    /* Do we still need more data to get this event complete? */
    if(m_midiParseData.m_bytes < m_midiParseData.m_bytesTotal)
    {
        return;
    }

    /*********************************************************************/
    /* Send the event                                                    */
    /*********************************************************************/
    /* The event is ready-to-go.  About 'running status':
     *
     * The MIDI protocol has a built-in compression mechanism. If several
     * similar events are sent in-a-row, for example note-ons, then the
     * event type is only sent once. For this case, the last event type
     * (status) is remembered.
     * We simply keep the status as it is, just reset the parameter counter.
     * If another status byte comes in, it will overwrite the status.
     */
    m_midiParseData.m_midiEvent.setType(
            static_cast<MidiEventTypes>(m_midiParseData.m_status));
    m_midiParseData.m_midiEvent.setChannel(m_midiParseData.m_channel);
    m_midiParseData.m_bytes = 0; /* Related to running status! */
    switch(m_midiParseData.m_midiEvent.type())
    {
        case MidiNoteOff:
        case MidiNoteOn:
        case MidiKeyPressure:
        case MidiProgramChange:
        case MidiChannelPressure:
            m_midiParseData.m_midiEvent.setKey(m_midiParseData.m_buffer[0]
                                               - KeysPerOctave);
            m_midiParseData.m_midiEvent.setVelocity(
                    m_midiParseData.m_buffer[1]);
            break;

        case MidiControlChange:
            m_midiParseData.m_midiEvent.setControllerNumber(
                    m_midiParseData.m_buffer[0]);
            m_midiParseData.m_midiEvent.setControllerValue(
                    m_midiParseData.m_buffer[1]);
            break;

        case MidiPitchBend:
            // Pitch-bend is transmitted with 14-bit precision.
            // Note: '|' does here the same as '+' (no common bits),
            // but might be faster
            // m_midiParseData.m_midiEvent.setMidiPitchBend( (
            // m_midiParseData.m_buffer[1] * 128 ) |
            // m_midiParseData.m_buffer[0] );
            m_midiParseData.m_midiEvent.setMidiPitchBendLE(
                    m_midiParseData.m_buffer[0], m_midiParseData.m_buffer[1]);
            // qInfo("MidiClient::parseDara %d %d pitchbend=%d",
            //    m_midiParseData.m_buffer[0],m_midiParseData.m_buffer[1],
            //    m_midiParseData.m_midiEvent.midiPitchBend());
            break;

        default:
            // Unlikely
            return;
    }

    processParsedEvent();
}

void MidiClientRaw::processParsedEvent()
{
    /*
    for(int i = 0; i < m_midiPorts.size(); ++i)
    {
        m_midiPorts[i]->processInEvent(m_midiParseData.m_midiEvent);
    }
    */
    for(MidiPort* port: m_midiPorts)
        port->processInEvent(m_midiParseData.m_midiEvent);
}

void MidiClientRaw::processOutEvent(const MidiEvent& event,
                                    const MidiTime&,
                                    const MidiPort* port)
{
    // TODO: also evaluate _time and queue event if necessary
    switch(event.type())
    {
        case MidiNoteOn:
        case MidiNoteOff:
        case MidiKeyPressure:
            sendByte(event.type() | event.channel());
            sendByte(event.key());  // + KeysPerOctave);
            sendByte(event.velocity());
            break;

        default:
            qWarning("MidiClientRaw: unhandled MIDI-event %d\n",
                     (int)event.type());
            break;
    }
}

void MidiClientRaw::sendBytes(const uint8_t*  _bytes,
                              const int       _size,
                              const MidiTime& _time,
                              const MidiPort* _port)
{
    for(int i = 0; i < _size; i++)
        sendByte(_bytes[i]);
}

// Taken from Nagano Daisuke's USB-MIDI driver
static const unsigned char REMAINS_F0F6[] = {
        0, /* 0xF0 */
        2, /* 0XF1 */
        3, /* 0XF2 */
        2, /* 0XF3 */
        2, /* 0XF4 (Undefined by MIDI Spec, and subject to change) */
        2, /* 0XF5 (Undefined by MIDI Spec, and subject to change) */
        1  /* 0XF6 */
};

static const unsigned char REMAINS_80E0[] = {
        3, /* 0x8X Note Off */
        3, /* 0x9X Note On */
        3, /* 0xAX Poly-key pressure */
        3, /* 0xBX Control Change */
        2, /* 0xCX Program Change */
        2, /* 0xDX Channel pressure */
        3  /* 0xEX PitchBend Change */
};

// Returns the length of the MIDI message starting with _event.
// Taken from Nagano Daisuke's USB-MIDI driver
int MidiClientRaw::eventLength(const unsigned char event)
{
    if(event < 0xF0)
    {
        return REMAINS_80E0[((event - 0x80) >> 4) & 0x0F];
    }
    else if(event < 0xF7)
    {
        return REMAINS_F0F6[event - 0xF0];
    }
    return 1;
}
