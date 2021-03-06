/*
 * MidiEvent.h - MidiEvent class
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef MIDI_EVENT_H
#define MIDI_EVENT_H

//#include <QMessageLogger>

#include "Midi.h"
#include "panning.h"
#include "pitch.h"
#include "volume.h"

#include <cstdio>
#include <cstdlib>

#ifdef qWarning
#define WARNING1(f, v1) qWarning(f, v1)
#define WARNING2(f, v1, v2) qWarning(f, v1, v2)
#else
#define WARNING1(f, v1) fprintf(stderr, f, v1);
#define WARNING2(f, v1, v2) fprintf(stderr, f, v1, v2);
#endif

class MidiEvent final
{
  public:
    MidiEvent(MidiEventTypes type       = MidiActiveSensing,
              int8_t         channel    = 0,
              int16_t        param1     = 0,
              int16_t        param2     = 0,
              const void*    sourcePort = nullptr) :
          m_type(type),
          m_metaEvent(MidiMetaInvalid), m_channel(channel),
          m_sysExData(nullptr), m_sourcePort(sourcePort)
    {
        m_data.m_param[0] = param1;
        m_data.m_param[1] = param2;
    }

    MidiEvent(MidiEventTypes type, const char* sysExData, int dataLen) :
          m_type(type), m_metaEvent(MidiMetaInvalid), m_channel(0),
          m_sysExData(sysExData), m_sourcePort(nullptr)
    {
        m_data.m_sysExDataLen = dataLen;
    }

    MidiEvent(const MidiEvent& other) :
          m_type(other.m_type), m_metaEvent(other.m_metaEvent),
          m_channel(other.m_channel), m_data(other.m_data),
          m_sysExData(other.m_sysExData), m_sourcePort(other.m_sourcePort)
    {
    }

    MidiEventTypes type() const
    {
        return m_type;
    }

    void setType(MidiEventTypes type)
    {
        m_type = type;
    }

    void setMetaEvent(MidiMetaEventType metaEvent)
    {
        m_metaEvent = metaEvent;
    }

    MidiMetaEventType metaEvent() const
    {
        return m_metaEvent;
    }

    int8_t channel() const
    {
        return m_channel;
    }

    void setChannel(int8_t channel)
    {
        if(channel < 0 || channel > 15)
        {
            WARNING1("MidiEvent::setChannel invalid channel: %d", channel);
            channel = 0;
        }
        m_channel = channel;
    }

    int16_t param(int i) const
    {
        return m_data.m_param[i];
    }

    void setParam(int i, uint16_t value)
    {
        if(value < 0 || value > 127)
            WARNING2("MidiEvent::setParam [%d] invalid value: %d", i, value);
        if(value < 0)
            value = 0;
        if(value > 127)
            value = 127;
        m_data.m_param[i] = value;
    }

    uint8_t byte(int i) const
    {
        return m_data.m_bytes[i];
    }

    void setByte(int i, uint16_t value)
    {
        if(value < 0 || value > 127)
            WARNING2("MidiEvent::setByte [%d] invalid value: %d", i, value);
        if(value < 0)
            value = 0;
        if(value > 127)
            value = 127;
        m_data.m_bytes[i] = value;
    }

    int16_t key() const
    {
        return param(0);
    }

    void setKey(int16_t key)
    {
        if(key < 0 || key > 127)
            WARNING1("MidiEvent::setKey invalid key: %d", key);
        if(key < 0)
            key = 0;
        if(key > 127)
            key = 127;
        m_data.m_param[0] = key;
    }

    uint8_t velocity() const
    {
        return m_data.m_param[1] & 0x7F;
    }

    void setVelocity(int16_t velocity)
    {
        if(velocity < 0 || velocity > 127)
            WARNING1("MidiEvent::setVelocity invalid velocity: %d", velocity);
        if(velocity < 0)
            velocity = 0;
        if(velocity > 127)
            velocity = 127;
        m_data.m_param[1] = velocity;
    }

    panning_t panning() const
    {
        return PanningLeft
               + (real_t(midiPanning() - MidiMinPanning))
                         / (real_t(MidiMaxPanning - MidiMinPanning))
                         * (real_t(PanningRight - PanningLeft));
    }

    uint8_t midiPanning() const
    {
        return m_data.m_param[1];
    }

    volume_t volume(int _midiBaseVelocity) const
    {
        // no qBound because of RemotePlugin
        volume_t r
                = (volume_t)(velocity() * DefaultVolume / _midiBaseVelocity);
        if(r < MinVolume)
            r = MinVolume;
        if(r > MaxVolume)
            r = MaxVolume;
        return r;
    }

    const void* sourcePort() const
    {
        return m_sourcePort;
    }

    uint8_t controllerNumber() const
    {
        return param(0) & 0x7F;
    }

    void setControllerNumber(uint8_t num)
    {
        setParam(0, num);
    }

    uint8_t controllerValue() const
    {
        return param(1);
    }

    void setControllerValue(uint8_t value)
    {
        setParam(1, value);
    }

    uint8_t program() const
    {
        return param(0);
    }

    uint8_t channelPressure() const
    {
        return param(0);
    }

    // 0..16383 center 8192
    uint16_t midiPitchBend() const
    {
        return ((param(1) & 0x7F) << 7) | (param(0) & 0x7F);
    }

    void setMidiPitchBend(uint16_t pitchBend)
    {
        if(pitchBend < 0 || pitchBend > 16383)
            WARNING1("MidiEvent::setMidiPitchBend invalid pitchBend: %d",
                     pitchBend);
        if(pitchBend < 0)
            pitchBend = 0;
        if(pitchBend > 16383)
            pitchBend = 16383;
        setParam(0, (pitchBend)&0x7F);
        setParam(1, (pitchBend >> 7) & 0x7F);
    }

    void midiPitchBendLE(uint8_t* low_, uint8_t* high_) const
    {
        *low_  = param(0) & 0x7F;
        *high_ = param(1) & 0x7F;
    }

    void setMidiPitchBendLE(uint8_t _low, uint8_t _high)
    {
        //( m_midiParseData.m_buffer[1] * 128 ) | m_midiParseData.m_buffer[0]
        setParam(0, _low & 0x7F);
        setParam(1, _high & 0x7F);
    }

    bool operator==(MidiEvent& b)
    {
        return (m_type == b.m_type) && (m_metaEvent == b.m_metaEvent)
               && (m_channel == b.m_channel)
               && (m_data.m_sysExDataLen == b.m_data.m_sysExDataLen)
               && (m_sysExData == b.m_sysExData)
               && (m_sourcePort == b.m_sourcePort);
    }

  private:
    MidiEventTypes    m_type;       // MIDI event type
    MidiMetaEventType m_metaEvent;  // Meta event (mostly unused)
    int8_t            m_channel;    // MIDI channel
    union {
        int16_t m_param[2];      // first/second parameter (key/velocity)
        uint8_t m_bytes[4];      // raw bytes
        int32_t m_sysExDataLen;  // len of m_sysExData
    } m_data;

    const char* m_sysExData;
    const void* m_sourcePort;
};

#endif
