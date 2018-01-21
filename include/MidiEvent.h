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

#include <cstdlib>
#include <QtGlobal>

#include "Midi.h"
#include "panning_constants.h"
#include "Pitch.h"
#include "volume.h"

class MidiEvent final
{
public:
	MidiEvent( MidiEventTypes type = MidiActiveSensing,
                   int8_t channel = 0,
                   int16_t param1 = 0,
                   int16_t param2 = 0,
                   const void* sourcePort = NULL ) :
		m_type( type ),
		m_metaEvent( MidiMetaInvalid ),
		m_channel( channel ),
		m_sysExData( NULL ),
		m_sourcePort( sourcePort )
	{
		m_data.m_param[0] = param1;
		m_data.m_param[1] = param2;
	}

	MidiEvent( MidiEventTypes type, const char* sysExData, int dataLen ) :
		m_type( type ),
		m_metaEvent( MidiMetaInvalid ),
		m_channel( 0 ),
		m_sysExData( sysExData ),
		m_sourcePort( NULL )
	{
		m_data.m_sysExDataLen = dataLen;
	}

	MidiEvent( const MidiEvent& other ) :
		m_type( other.m_type ),
		m_metaEvent( other.m_metaEvent ),
		m_channel( other.m_channel ),
		m_data( other.m_data ),
		m_sysExData( other.m_sysExData ),
		m_sourcePort( other.m_sourcePort )
	{
	}

	MidiEventTypes type() const
	{
		return m_type;
	}

	void setType( MidiEventTypes type )
	{
		m_type = type;
	}

	void setMetaEvent( MidiMetaEventType metaEvent )
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

	void setChannel( int8_t channel )
	{
                if(channel<0||channel>15)
                        qWarning("MidiEvent::setChannel invalid channel: %d",channel);
		m_channel = channel;
	}

	int16_t param( int i ) const
	{
		return m_data.m_param[i];
	}

	void setParam( int i, uint16_t value )
	{
                if(value<0||value>127)
                        qWarning("MidiEvent::setParam [%d] invalid value: %d",i,value);
		m_data.m_param[i] = value;
	}

	int16_t key() const
	{
		return param( 0 );
	}

	void setKey( int16_t key )
	{
                if(key<0||key>127)
                        qWarning("MidiEvent::setKey invalid key: %d",key);
		m_data.m_param[0] = key;
	}

	uint8_t velocity() const
	{
		return m_data.m_param[1] & 0x7F;
	}

	void setVelocity( int16_t velocity )
	{
                if(velocity<0||velocity>127)
                        qWarning("MidiEvent::setVelocity invalid velocity: %d",velocity);
		m_data.m_param[1] = velocity;
	}

	panning_t panning() const
	{
		return (panning_t) ( PanningLeft +
			( (float)( midiPanning() - MidiMinPanning ) ) / 
			( (float)( MidiMaxPanning - MidiMinPanning ) ) *
			( (float)( PanningRight - PanningLeft ) ) );
	}

	uint8_t midiPanning() const
	{
		return m_data.m_param[1];
	}

	volume_t volume( int _midiBaseVelocity ) const
	{
		return qBound(MinVolume,
                              (volume_t)( velocity() * DefaultVolume / _midiBaseVelocity),
                              MaxVolume);
	}

	const void* sourcePort() const
	{
		return m_sourcePort;
	}

	uint8_t controllerNumber() const
	{
		return param( 0 ) & 0x7F;
	}

	void setControllerNumber( uint8_t num )
	{
		setParam( 0, num );
	}

	uint8_t controllerValue() const
	{
		return param( 1 );
	}

	void setControllerValue( uint8_t value )
	{
		setParam( 1, value );
	}

	uint8_t program() const
	{
		return param( 0 );
	}

	uint8_t channelPressure() const
	{
		return param( 0 );
	}

        // -100..+100 center 0.
	float lmmsPitchBend() const
	{
                float r=midiPitchBend()-8192;
                     if(r<0) r=r/-8192.f*MinPitchDefault;
                else if(r>0) r=r/8191.f*MaxPitchDefault;
                return r;
	}

	void setLmmsPitchBend( float pitchBend )
	{
                uint16_t r=pitchBend-DefaultPitch;
                     if(r<0) r=r*-8192.f/MinPitchDefault;
                else if(r>0) r=r/8191.f*MaxPitchDefault;
                r=r+8192;
		setMidiPitchBend(r);
	}

        // 0..16383 center 8192
	uint16_t midiPitchBend() const
	{
		return ((param(1)&0x7F)<<7)|(param(0)&0x7F);
	}

	void setMidiPitchBend( uint16_t pitchBend )
	{
                if(pitchBend<0||pitchBend>16383)
                        qWarning("MidiEvent::setMidiPitchBend invalid pitchBend: %d",pitchBend);
                //( m_midiParseData.m_buffer[1] * 128 ) | m_midiParseData.m_buffer[0]
                if(pitchBend<    0) pitchBend=0;
                if(pitchBend>16383) pitchBend=16383;
                setParam(0,(pitchBend   )&0x7F);
                setParam(1,(pitchBend>>7)&0x7F);
	}

	void midiPitchBendLE(uint8_t* low_,uint8_t* high_) const
	{
		*low_ =param(0)&0x7F;
                *high_=param(1)&0x7F;
	}

	void setMidiPitchBendLE(uint8_t _low, uint8_t _high)
	{
                //( m_midiParseData.m_buffer[1] * 128 ) | m_midiParseData.m_buffer[0]
                setParam(0,_low &0x7F);
                setParam(1,_high&0x7F);
	}

	bool operator == (MidiEvent& b)
	{
		return
		(m_type==b.m_type) &&
		(m_metaEvent==b.m_metaEvent) &&
		(m_channel==b.m_channel) &&
		(m_data.m_sysExDataLen==b.m_data.m_sysExDataLen) &&
		(m_sysExData==b.m_sysExData) &&
		(m_sourcePort==b.m_sourcePort);
	}

private:
	MidiEventTypes m_type;		// MIDI event type
	MidiMetaEventType m_metaEvent;	// Meta event (mostly unused)
	int8_t m_channel;		// MIDI channel
	union
	{
		int16_t m_param[2];	// first/second parameter (key/velocity)
		uint8_t m_bytes[4];	// raw bytes
		int32_t m_sysExDataLen;	// len of m_sysExData
	} m_data;

	const char* m_sysExData;
	const void* m_sourcePort;

} ;

#endif
