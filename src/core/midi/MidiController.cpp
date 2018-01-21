/*
 * MidiController.cpp - implementation of class midi-controller which handles
 *                      MIDI control change messages
 *
 * Copyright (c) 2008 Paul Giblock <drfaygo/at/gmail.com>
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

#include <QDomElement>
#include <QObject>

#include "lmms_math.h"
#include "Song.h"
#include "Mixer.h"
#include "MidiClient.h"
#include "MidiController.h"


MidiController::MidiController( Model * _parent ) :
	Controller( Controller::MidiController, _parent, tr( "MIDI Controller" ) ),
	MidiEventProcessor(),
	m_midiPort( tr( "unnamed_midi_controller" ),
			Engine::mixer()->midiClient(), this, this, MidiPort::Input ),
	m_lastValue( 0.0f ),
	m_previousValue( 0.0f ),
	m_switch( true )
{
	setSampleExact( true );
	connect( &m_midiPort, SIGNAL( modeChanged() ),
			this, SLOT( updateName() ) );
}




MidiController::~MidiController()
{
}




void MidiController::updateValueBuffer()
{
	if( m_previousValue != m_lastValue )
	{
		m_valueBuffer.interpolate( m_previousValue, m_lastValue );
		m_previousValue = m_lastValue;
	}
	else
	{
		m_valueBuffer.fill( m_lastValue );
	}
	m_bufferLastUpdated = s_periods;
}


void MidiController::updateName()
{
        int type = m_midiPort.widgetType();
        if(type == 6)
                setName( QString("MIDI ch%1 pitchbend").
                         arg( m_midiPort.inputChannel() ) );
        else
                setName( QString("MIDI ch%1 ctrl%2").
                         arg( m_midiPort.inputChannel() ).
                         arg( m_midiPort.inputController() ) );
}




void MidiController::processInEvent( const MidiEvent& event, const MidiTime& time, f_cnt_t offset )
{
	unsigned char controllerNum = event.controllerNumber();

        int type = m_midiPort.widgetType();
        if(type <= 0 || type >= 7) return;
        if(type == 6 && event.type() != MidiPitchBend) return;
        if(type != 6 && event.type() == MidiPitchBend) return;

	if( (m_midiPort.inputController() == controllerNum + 1 ||
             event.type() == MidiPitchBend) &&
	    ( m_midiPort.inputChannel() == event.channel() + 1 ||
	      m_midiPort.inputChannel() == 0 ) )
	{
		float val=0.f;

		if(event.type() == MidiControlChange)
			val=event.controllerValue();
		else
		if(event.type() == MidiNoteOn)
			val=event.velocity();
		else
		if(event.type() == MidiNoteOff)
			val=event.velocity();
		else
                if(event.type() == MidiPitchBend)
                        val=127.f*event.midiPitchBend()/16383.f;
                else
			// Don't care - maybe add special cases for mod later
			qWarning("MidiController: in event Default");

		if(type == 1) //Open Relay Button
		{
			     if(event.type() == MidiNoteOn ) val=0.f;
			else if(event.type() == MidiNoteOff) ;//val=127;
		}
		else
		if(type == 2) //Close Relay Button
		{
			     if(event.type() == MidiNoteOn ) ;//val=127;
			else if(event.type() == MidiNoteOff) val=0.f;
		}
		else
		if(type == 3) //Switch
		{
			if(event.type() == MidiNoteOn)
			{
				val=( m_switch ? val : 0.f );
				m_switch=(m_switch ? 0 : 1.f);
			}
			else return;
		}

		{
			float base =m_midiPort.baseInputValue();
			float slope=m_midiPort.slopeInputValue();
			float delta=m_midiPort.deltaInputValue();
			val=base+slope*(val-delta);

                        float vmin=m_midiPort.minInputValue();
                        float vmax=m_midiPort.maxInputValue();
			float step=m_midiPort.stepInputValue();
			if(step>1.f) val=vmin+fmodf(val-vmin,step);
			if(val<vmin) val=vmin;
			if(val>vmax) val=vmax;

			if(val<  0.f) val=  0.f; //just for safety
			if(val>127.f) val=127.f; //just for safety

			m_previousValue = m_lastValue;
			m_lastValue = val/127.0f;
                        //qInfo("MidiController: last=%f prev=%f",m_lastValue,m_previousValue);
		}

		if(m_previousValue != m_lastValue)
		{
			if(type<=3) m_previousValue=m_lastValue;
			updateValueBuffer();
			emit valueChanged();
		}
		else
		{
			//qWarning("MidiController: emit dataUnchanged");
			//emit dataUnchanged();
		}
	}
}




void MidiController::subscribeReadablePorts( const MidiPort::Map & _map )
{
	for( MidiPort::Map::ConstIterator it = _map.constBegin();
						it != _map.constEnd(); ++it )
	{
		m_midiPort.subscribeReadablePort( it.key(), *it );
	}
}




void MidiController::saveSettings( QDomDocument & _doc, QDomElement & _this )
{
	Controller::saveSettings( _doc, _this );
	m_midiPort.saveSettings( _doc, _this );

}




void MidiController::loadSettings( const QDomElement & _this )
{
	Controller::loadSettings( _this );

	m_midiPort.loadSettings( _this );

	updateName();
}




QString MidiController::nodeName() const
{
	return( "Midicontroller" );
}




ControllerDialog * MidiController::createDialog( QWidget * _parent )
{
	return NULL;
}




