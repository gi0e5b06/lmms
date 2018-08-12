/*
 * MidiEffectChain.h - class for processing and effects chain
 *
 * Copyright (c) 2006-2008 Danny McRae <khjklujn/at/users.sourceforge.net>
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef MIDI_EFFECT_CHAIN_H
#define MIDI_EFFECT_CHAIN_H

#include "MidiEffect.h"
#include "MidiTime.h"
#include "Model.h"
#include "SerializingObject.h"

class MidiEffect;
class MidiEvent;

class EXPORT MidiEffectChain : public Model, public SerializingObject
{
	Q_OBJECT
public:
	MidiEffectChain( Model * _parent );
	virtual ~MidiEffectChain();

	virtual void saveSettings( QDomDocument & _doc, QDomElement & _parent );
	virtual void loadSettings( const QDomElement & _this );

	inline virtual QString nodeName() const
	{
		return "fxchain";
	}

	void appendEffect( MidiEffect * _effect );
	void removeEffect( MidiEffect * _effect );
	void moveDown( MidiEffect * _effect );
	void moveUp( MidiEffect * _effect );

	virtual bool processEvents(QList<MidiEvent>& events,
				   const MidiTime&   time   = MidiTime(),
				   f_cnt_t           offset = 0 );

	/*
	bool processAudioBuffer( sampleFrame * _buf, const fpp_t _frames, bool hasInputNoise );
	*/
	void startRunning();

	void clear();

	void setEnabled( bool _on )
	{
		m_enabledModel.setValue( _on );
	}


private:
	typedef QVector<MidiEffect *> MidiEffectList;
	MidiEffectList m_effects;

	BoolModel m_enabledModel;


	friend class MidiEffectRackView;


signals:
	void aboutToClear();

} ;

#endif
