/*
 * DummyMidiEffect.h - effect used as fallback if an effect couldn't be loaded
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

#ifndef DUMMY_MIDI_EFFECT_H
#define DUMMY_MIDI_EFFECT_H

#include "MidiEffect.h"
#include "MidiEffectControls.h"
#include "MidiEffectControlDialog.h"


class DummyMidiEffectControlDialog : public MidiEffectControlDialog
{
public:
	DummyMidiEffectControlDialog( MidiEffectControls * _controls ) :
		MidiEffectControlDialog( _controls )
	{
	}

} ;


class DummyMidiEffectControls : public MidiEffectControls
{
public:
	DummyMidiEffectControls( MidiEffect * _eff ) :
		MidiEffectControls( _eff )
	{
	}

	virtual ~DummyMidiEffectControls()
	{
	}

	virtual int controlCount()
	{
		return 0;
	}

	virtual void saveSettings( QDomDocument &, QDomElement & )
	{
	}

	virtual void loadSettings( const QDomElement & )
	{
	}

	virtual QString nodeName() const
	{
		return "DummyControls";
	}

	virtual MidiEffectControlDialog * createView()
	{
		return new DummyMidiEffectControlDialog( this );
	}
} ;



class DummyMidiEffect : public MidiEffect
{
	Q_OBJECT
public:
	DummyMidiEffect( Model * _parent, const QDomElement& originalPluginData ) :
		MidiEffect( NULL, _parent, NULL ),
		m_controls( this ),
		m_originalPluginData( originalPluginData )
	{
		setName();
	}

	virtual ~DummyMidiEffect()
	{
	}

	virtual MidiEffectControls * controls()
	{
		return &m_controls;
	}

	inline virtual bool processEvents(QList<MidiEvent>& events,
					  const MidiTime&  time   = MidiTime(),
					  f_cnt_t          offset = 0 )
	{
		return true;
	}

	const QDomElement& originalPluginData() const
	{
		return m_originalPluginData;
	}



private:
	DummyMidiEffectControls m_controls;
	const QDomElement m_originalPluginData;

	// Parse the display name from the dom
	virtual void setName()
	{
		QDomNodeList keys = originalPluginData().elementsByTagName( "key" );
		for( int i = 0; !keys.item( i ).isNull(); ++i )
		{
			QDomNodeList attributes = keys.item( i ).toElement().elementsByTagName( "attribute" );
			for( int j = 0; !attributes.item( j ).isNull(); ++j )
			{
				QDomElement attribute = attributes.item( j ).toElement();
				if( attribute.hasAttribute( "value" ) )
				{
					QString name = tr("NOT FOUND") + " (" + attribute.attribute( "value" ) + ")";
					setDisplayName(name);
					return;
				}

			}

		}
	}
} ;


#endif
