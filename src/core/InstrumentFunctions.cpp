/*
 * InstrumentFunctions.cpp - models for instrument-function-tab
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

#include <QDomElement>

#include "InstrumentFunctions.h"

#include "lmms_math.h"
#include "embed.h"
#include "Engine.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "Song.h"
#include "PresetPreviewPlayHandle.h"


InstrumentFunction::InstrumentFunction( Model * _parent, QString _name ) :
	Model( _parent, _name ),
	m_enabledModel( false, this )
{
}



InstrumentFunctionNoteStacking::ChordTable::Init InstrumentFunctionNoteStacking::ChordTable::s_initTable[] =
{
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "octave" ), { 0, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Major" ), { 0, 4, 7, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Majb5" ), { 0, 4, 6, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "minor" ), { 0, 3, 7, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "minb5" ), { 0, 3, 6, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "sus2" ), { 0, 2, 7, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "sus4" ), { 0, 5, 7, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "aug" ), { 0, 4, 8, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "augsus4" ), { 0, 5, 8, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "tri" ), { 0, 3, 6, 9, -1 } },
	
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "6" ), { 0, 4, 7, 9, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "6sus4" ), { 0, 5, 7, 9, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "6add9" ), { 0, 4, 7, 9, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m6" ), { 0, 3, 7, 9, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m6add9" ), { 0, 3, 7, 9, 14, -1 } },

	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7" ), { 0, 4, 7, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7sus4" ), { 0, 5, 7, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7#5" ), { 0, 4, 8, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7b5" ), { 0, 4, 6, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7#9" ), { 0, 4, 7, 10, 15, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7b9" ), { 0, 4, 7, 10, 13, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7#5#9" ), { 0, 4, 8, 10, 15, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7#5b9" ), { 0, 4, 8, 10, 13, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7b5b9" ), { 0, 4, 6, 10, 13, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7add11" ), { 0, 4, 7, 10, 17, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7add13" ), { 0, 4, 7, 10, 21, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "7#11" ), { 0, 4, 7, 10, 18, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj7" ), { 0, 4, 7, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj7b5" ), { 0, 4, 6, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj7#5" ), { 0, 4, 8, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj7#11" ), { 0, 4, 7, 11, 18, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj7add13" ), { 0, 4, 7, 11, 21, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m7" ), { 0, 3, 7, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m7b5" ), { 0, 3, 6, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m7b9" ), { 0, 3, 7, 10, 13, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m7add11" ), { 0, 3, 7, 10, 17, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m7add13" ), { 0, 3, 7, 10, 21, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m-Maj7" ), { 0, 3, 7, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m-Maj7add11" ), { 0, 3, 7, 11, 17, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m-Maj7add13" ), { 0, 3, 7, 11, 21, -1 } },

	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "9" ), { 0, 4, 7, 10, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "9sus4" ), { 0, 5, 7, 10, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "add9" ), { 0, 4, 7, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "9#5" ), { 0, 4, 8, 10, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "9b5" ), { 0, 4, 6, 10, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "9#11" ), { 0, 4, 7, 10, 14, 18, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "9b13" ), { 0, 4, 7, 10, 14, 20, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj9" ), { 0, 4, 7, 11, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj9sus4" ), { 0, 5, 7, 11, 15, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj9#5" ), { 0, 4, 8, 11, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj9#11" ), { 0, 4, 7, 11, 14, 18, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m9" ), { 0, 3, 7, 10, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "madd9" ), { 0, 3, 7, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m9b5" ), { 0, 3, 6, 10, 14, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m9-Maj7" ), { 0, 3, 7, 11, 14, -1 } },

	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "11" ), { 0, 4, 7, 10, 14, 17, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "11b9" ), { 0, 4, 7, 10, 13, 17, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj11" ), { 0, 4, 7, 11, 14, 17, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m11" ), { 0, 3, 7, 10, 14, 17, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m-Maj11" ), { 0, 3, 7, 11, 14, 17, -1 } },

	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "13" ), { 0, 4, 7, 10, 14, 21, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "13#9" ), { 0, 4, 7, 10, 15, 21, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "13b9" ), { 0, 4, 7, 10, 13, 21, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "13b5b9" ), { 0, 4, 6, 10, 13, 21, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Maj13" ), { 0, 4, 7, 11, 14, 21, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m13" ), { 0, 3, 7, 10, 14, 21, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "m-Maj13" ), { 0, 3, 7, 11, 14, 21, -1 } },

	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Major" ), { 0, 2, 4, 5, 7, 9, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Harmonic minor" ), { 0, 2, 3, 5, 7, 8, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Melodic minor" ), { 0, 2, 3, 5, 7, 9, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Whole tone" ), { 0, 2, 4, 6, 8, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Diminished" ), { 0, 2, 3, 5, 6, 8, 9, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Major pentatonic" ), { 0, 2, 4, 7, 9, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Minor pentatonic" ), { 0, 3, 5, 7, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Jap in sen" ), { 0, 1, 5, 7, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Major bebop" ), { 0, 2, 4, 5, 7, 8, 9, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Dominant bebop" ), { 0, 2, 4, 5, 7, 9, 10, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Blues" ), { 0, 3, 5, 6, 7, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Arabic" ), { 0, 1, 4, 5, 7, 8, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Enigmatic" ), { 0, 1, 4, 6, 8, 10, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Neopolitan" ), { 0, 1, 3, 5, 7, 9, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Neopolitan minor" ), { 0, 1, 3, 5, 7, 8, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Hungarian minor" ), { 0, 2, 3, 6, 7, 8, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Dorian" ), { 0, 2, 3, 5, 7, 9, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Phrygolydian" ), { 0, 1, 3, 5, 7, 8, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Lydian" ), { 0, 2, 4, 6, 7, 9, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Mixolydian" ), { 0, 2, 4, 5, 7, 9, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Aeolian" ), { 0, 2, 3, 5, 7, 8, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Locrian" ), { 0, 1, 3, 5, 6, 8, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Minor" ), { 0, 2, 3, 5, 7, 8, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Chromatic" ), { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Half-Whole Diminished" ), { 0, 1, 3, 4, 6, 7, 9, 10, -1 } },
	
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "5" ), { 0, 7, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Phrygian dominant" ), { 0, 1, 4, 5, 7, 8, 10, -1 } },
	{ QT_TRANSLATE_NOOP( "InstrumentFunctionNoteStacking", "Persian" ), { 0, 1, 4, 5, 6, 8, 11, -1 } }
} ;




InstrumentFunctionNoteStacking::Chord::Chord( const char * n, const ChordSemiTones & semi_tones ) :
	m_name( InstrumentFunctionNoteStacking::tr( n ) )
{
	for( m_size = 0; m_size < MAX_CHORD_POLYPHONY; m_size++ )
	{
		if( semi_tones[m_size] == -1 )
		{
			break;
		}

		m_semiTones[m_size] = semi_tones[m_size];
	}
}




bool InstrumentFunctionNoteStacking::Chord::hasSemiTone( int8_t semi_tone ) const
{
	for( int i = 0; i < size(); ++i )
	{
		if( semi_tone == m_semiTones[i] )
		{
			return true;
		}
	}
	return false;
}




InstrumentFunctionNoteStacking::ChordTable::ChordTable() :
	QVector<Chord>()
{
	for( int i = 0;
		i < static_cast<int>( sizeof s_initTable / sizeof *s_initTable );
		i++ )
	{
		push_back( Chord( s_initTable[i].m_name, s_initTable[i].m_semiTones ) );
	}
}




const InstrumentFunctionNoteStacking::Chord & InstrumentFunctionNoteStacking::ChordTable::getByName( const QString & name, bool is_scale ) const
{
	for( int i = 0; i < size(); i++ )
	{
		if( at( i ).getName() == name && is_scale == at( i ).isScale() )
			return at( i );
	}

	static Chord empty;
	return empty;
}




InstrumentFunctionNoteStacking::InstrumentFunctionNoteStacking( Model * _parent ) :
	InstrumentFunction( _parent, tr( "Chords" ) ),
	//m_enabledModel( false, this ),
	m_chordsModel( this, tr( "Chord type" ) ),
	m_chordRangeModel( 1.0f, 1.0f, 9.0f, 1.0f, this, tr( "Chord range" ) )
{
	const ChordTable & chord_table = ChordTable::getInstance();
	for( int i = 0; i < chord_table.size(); ++i )
	{
		m_chordsModel.addItem( chord_table[i].getName() );
	}
}




InstrumentFunctionNoteStacking::~InstrumentFunctionNoteStacking()
{
}




void InstrumentFunctionNoteStacking::processNote( NotePlayHandle * _n )
{
	const int base_note_key = _n->key();
	const ChordTable & chord_table = ChordTable::getInstance();
	// we add chord-subnotes to note if either note is a base-note and
	// arpeggio is not used or note is part of an arpeggio
	// at the same time we only add sub-notes if nothing of the note was
	// played yet, because otherwise we would add chord-subnotes every
	// time an audio-buffer is rendered...
	if( ( _n->origin() == NotePlayHandle::OriginArpeggio ||
	      ( _n->hasParent() == false && _n->instrumentTrack()->isArpeggioEnabled() == false ) ) &&
	    _n->totalFramesPlayed() == 0 &&
	    m_enabledModel.value() == true && ! _n->isReleased() )
	{
		// then insert sub-notes for chord
		const int selected_chord = m_chordsModel.value();

		for( int octave_cnt = 0; octave_cnt < m_chordRangeModel.value(); ++octave_cnt )
		{
			const int sub_note_key_base = base_note_key + octave_cnt * KeysPerOctave;

			// process all notes in the chord
			for( int i = 0; i < chord_table[selected_chord].size(); ++i )
			{
				// add interval to sub-note-key
				const int sub_note_key = sub_note_key_base + (int) chord_table[selected_chord][i];
				// maybe we're out of range -> let's get outta
				// here!
				if( sub_note_key > NumKeys )
				{
					break;
				}

				//if(MM_SAFE(NotePlayHandle,1))
				{

					// create copy of base-note
					Note note_copy( _n->length(), 0, sub_note_key, _n->getVolume(), _n->getPanning(), _n->detuning() );

					// create sub-note-play-handle, only note is
					// different
					Engine::mixer()->addPlayHandle
						(NotePlayHandleManager::acquire( _n->instrumentTrack(), _n->offset(),
										 _n->frames(), note_copy,
										 _n, -1, NotePlayHandle::OriginNoteStacking ) );
				}
			}
		}
	}
}




void InstrumentFunctionNoteStacking::saveSettings( QDomDocument & _doc, QDomElement & _this )
{
	m_enabledModel.saveSettings( _doc, _this, "chord-enabled" );
	m_chordsModel.saveSettings( _doc, _this, "chord" );
	m_chordRangeModel.saveSettings( _doc, _this, "chordrange" );
}




void InstrumentFunctionNoteStacking::loadSettings( const QDomElement & _this )
{
	m_enabledModel.loadSettings( _this, "chord-enabled" );
	m_chordsModel.loadSettings( _this, "chord" );
	m_chordRangeModel.loadSettings( _this, "chordrange" );
}







InstrumentFunctionArpeggio::InstrumentFunctionArpeggio( Model * _parent ) :
	InstrumentFunction( _parent, tr( "Arpeggio" ) ),
	//m_enabledModel( false ),
	m_arpModel( this, tr( "Arpeggio type" ) ),
	m_arpRangeModel( 1.0f, 1.0f, 9.0f, 1.0f, this, tr( "Arpeggio range" ) ),
	m_arpCycleModel( 0.0f, 0.0f, 6.0f, 1.0f, this, tr( "Cycle steps" ) ),
	m_arpSkipModel( 0.0f, 0.0f, 100.0f, 1.0f, this, tr( "Skip rate" ) ),
	m_arpMissModel( 0.0f, 0.0f, 100.0f, 1.0f, this, tr( "Miss rate" ) ),
	m_arpTimeModel( 200.0f, 25.0f, 2000.0f, 1.0f, 2000, this, tr( "Arpeggio time" ) ),
	m_arpGateModel( 100.0f, 1.0f, 200.0f, 1.0f, this, tr( "Arpeggio gate" ) ),
	m_arpDirectionModel( this, tr( "Arpeggio direction" ) ),
	m_arpModeModel( this, tr( "Arpeggio mode" ) )
{
	const InstrumentFunctionNoteStacking::ChordTable & chord_table = InstrumentFunctionNoteStacking::ChordTable::getInstance();
	for( int i = 0; i < chord_table.size(); ++i )
	{
		m_arpModel.addItem( chord_table[i].getName() );
	}

	m_arpDirectionModel.addItem( tr( "Up" ), new PixmapLoader( "arp_up" ) );
	m_arpDirectionModel.addItem( tr( "Down" ), new PixmapLoader( "arp_down" ) );
	m_arpDirectionModel.addItem( tr( "Up and down" ), new PixmapLoader( "arp_up_and_down" ) );
	m_arpDirectionModel.addItem( tr( "Down and up" ), new PixmapLoader( "arp_up_and_down" ) );
	m_arpDirectionModel.addItem( tr( "Random" ), new PixmapLoader( "arp_random" ) );
	m_arpDirectionModel.setInitValue( ArpDirUp );

	m_arpModeModel.addItem( tr( "Free" ), new PixmapLoader( "arp_free" ) );
	m_arpModeModel.addItem( tr( "Sort" ), new PixmapLoader( "arp_sort" ) );
	m_arpModeModel.addItem( tr( "Sync" ), new PixmapLoader( "arp_sync" ) );
}




InstrumentFunctionArpeggio::~InstrumentFunctionArpeggio()
{
}




void InstrumentFunctionArpeggio::processNote( NotePlayHandle * _n )
{
	const int base_note_key = _n->key();
	if( _n->origin() == NotePlayHandle::OriginArpeggio ||
	    _n->origin() == NotePlayHandle::OriginNoteStacking ||
	    !m_enabledModel.value() ||
	    ( _n->isReleased() && _n->releaseFramesDone() >= _n->actualReleaseFramesToDo() ) )
	{
		return;
	}


	const int selected_arp = m_arpModel.value();

	ConstNotePlayHandleList cnphv = NotePlayHandle::nphsOfInstrumentTrack( _n->instrumentTrack() );

	if( m_arpModeModel.value() != FreeMode && cnphv.size() == 0 )
	{
		// maybe we're playing only a preset-preview-note?
		cnphv = PresetPreviewPlayHandle::nphsOfInstrumentTrack( _n->instrumentTrack() );
		if( cnphv.size() == 0 )
		{
			// still nothing found here, so lets return
			//return;
			cnphv.push_back( _n );
		}
	}

	const InstrumentFunctionNoteStacking::ChordTable & chord_table = InstrumentFunctionNoteStacking::ChordTable::getInstance();
	const int cur_chord_size = chord_table[selected_arp].size();
	const int range = (int)( cur_chord_size * m_arpRangeModel.value() );
	const int total_range = range * cnphv.size();

	// number of frames that every note should be played
	const f_cnt_t arp_frames = (f_cnt_t)( m_arpTimeModel.value() / 1000.0f * Engine::mixer()->processingSampleRate() );
	const f_cnt_t gated_frames = (f_cnt_t)( m_arpGateModel.value() * arp_frames / 100.0f );

	// used for calculating remaining frames for arp-note, we have to add
	// arp_frames-1, otherwise the first arp-note will not be setup
	// correctly... -> arp_frames frames silence at the start of every note!
	int cur_frame = ( ( m_arpModeModel.value() != FreeMode ) ?
						cnphv.first()->totalFramesPlayed() :
						_n->totalFramesPlayed() ) + arp_frames - 1;
	// used for loop
	f_cnt_t frames_processed = ( m_arpModeModel.value() != FreeMode ) ? cnphv.first()->noteOffset() : _n->noteOffset();

	while( frames_processed < Engine::mixer()->framesPerPeriod() )
	{
		const f_cnt_t remaining_frames_for_cur_arp = arp_frames - ( cur_frame % arp_frames );
		// does current arp-note fill whole audio-buffer?
		if( remaining_frames_for_cur_arp > Engine::mixer()->framesPerPeriod() )
		{
			// then we don't have to do something!
			break;
		}

		frames_processed += remaining_frames_for_cur_arp;

		// in sorted mode: is it our turn or do we have to be quiet for
		// now?
		if( m_arpModeModel.value() == SortMode &&
				( ( cur_frame / arp_frames ) % total_range ) / range != (f_cnt_t) _n->index() )
		{
			// Set master note if not playing arp note or it will play as an ordinary note
			_n->setMasterNote();
			// update counters
			frames_processed += arp_frames;
			cur_frame += arp_frames;
			continue;
		}

		// Skip notes randomly
		if( m_arpSkipModel.value() )
		{

			if( 100 * ( (float) rand() / (float)( RAND_MAX + 1.0f ) ) < m_arpSkipModel.value() )
			{
				// Set master note to prevent the note to extend over skipped notes
				// This may only be needed for lb302
				_n->setMasterNote();
				// update counters
				frames_processed += arp_frames;
				cur_frame += arp_frames;
				continue;
			}
		}

		int dir = m_arpDirectionModel.value();

		// Miss notes randomly. We intercept int dir and abuse it
		// after need.  :)

		if( m_arpMissModel.value() )
		{
			if( 100 * ( (float) rand() / (float)( RAND_MAX + 1.0f ) ) < m_arpMissModel.value() )
			{
				dir = ArpDirRandom;
			}
		}

		int cur_arp_idx = 0;
		// process according to arpeggio-direction...
		if( dir == ArpDirUp )
		{
			cur_arp_idx = ( cur_frame / arp_frames ) % range;
		}
		else if( dir == ArpDirDown )
		{
			cur_arp_idx = range - ( cur_frame / arp_frames ) %
								range - 1;
		}
		else if( dir == ArpDirUpAndDown && range > 1 )
		{
			// imagine, we had to play the arp once up and then
			// once down -> makes 2 * range possible notes...
			// because we don't play the lower and upper notes
			// twice, we have to subtract 2
			cur_arp_idx = ( cur_frame / arp_frames ) % ( range * 2 - 2 );
			// if greater than range, we have to play down...
			// looks like the code for arp_dir==DOWN... :)
			if( cur_arp_idx >= range )
			{
				cur_arp_idx = range - cur_arp_idx % ( range - 1 ) - 1;
			}
		}
		else if( dir == ArpDirDownAndUp && range > 1 )
		{
			// copied from ArpDirUpAndDown above
			cur_arp_idx = ( cur_frame / arp_frames ) % ( range * 2 - 2 );
			// if greater than range, we have to play down...
			// looks like the code for arp_dir==DOWN... :)
			if( cur_arp_idx >= range )
			{
				cur_arp_idx = range - cur_arp_idx % ( range - 1 ) - 1;
			}
			// inverts direction
			cur_arp_idx = range - cur_arp_idx - 1;
		}
		else if( dir == ArpDirRandom )
		{
			// just pick a random chord-index
			cur_arp_idx = (int)( range * ( (float) rand() / (float) RAND_MAX ) );
		}

		// Cycle notes
		if( m_arpCycleModel.value() && dir != ArpDirRandom )
		{
			cur_arp_idx *= m_arpCycleModel.value() + 1;
			cur_arp_idx %= range;
		}

		// now calculate final key for our arp-note
		const int sub_note_key = base_note_key + (cur_arp_idx / cur_chord_size ) *
							KeysPerOctave + chord_table[selected_arp][cur_arp_idx % cur_chord_size];

		// range-checking
		if( sub_note_key >= NumKeys ||
			sub_note_key < 0 ||
			Engine::mixer()->criticalXRuns() )
		{
			continue;
		}

		float vol_level = 1.0f;
		if( _n->isReleased() )
		{
			vol_level = _n->volumeLevel( cur_frame + gated_frames );
		}

		// create new arp-note

		//if(MM_SAFE(NotePlayHandle,1))
		{
			// create sub-note-play-handle, only ptr to note is different
			// and is_arp_note=true
			Engine::mixer()->addPlayHandle
				(NotePlayHandleManager::acquire( _n->instrumentTrack(),
								 frames_processed,
								 gated_frames,
								 Note( MidiTime( 0 ), MidiTime( 0 ), sub_note_key,
								       (volume_t) qRound( _n->getVolume() * vol_level ),
								       _n->getPanning(), _n->detuning() ),
								 _n, -1, NotePlayHandle::OriginArpeggio )
						       );
		}

		// update counters
		frames_processed += arp_frames;
		cur_frame += arp_frames;
	}

	// make sure note is handled as arp-base-note, even
	// if we didn't add a sub-note so far
	if( m_arpModeModel.value() != FreeMode )
	{
		_n->setMasterNote();
	}
}




void InstrumentFunctionArpeggio::saveSettings( QDomDocument & _doc, QDomElement & _this )
{
	m_enabledModel.saveSettings( _doc, _this, "arp-enabled" );
	m_arpModel.saveSettings( _doc, _this, "arp" );
	m_arpRangeModel.saveSettings( _doc, _this, "arprange" );
	m_arpCycleModel.saveSettings( _doc, _this, "arpcycle" );
	m_arpSkipModel.saveSettings( _doc, _this, "arpskip" );
	m_arpMissModel.saveSettings( _doc, _this, "arpmiss" );
	m_arpTimeModel.saveSettings( _doc, _this, "arptime" );
	m_arpGateModel.saveSettings( _doc, _this, "arpgate" );
	m_arpDirectionModel.saveSettings( _doc, _this, "arpdir" );

	m_arpModeModel.saveSettings( _doc, _this, "arpmode" );
}




void InstrumentFunctionArpeggio::loadSettings( const QDomElement & _this )
{
	m_enabledModel.loadSettings( _this, "arp-enabled" );
	m_arpModel.loadSettings( _this, "arp" );
	m_arpRangeModel.loadSettings( _this, "arprange" );
	m_arpCycleModel.loadSettings( _this, "arpcycle" );
	m_arpSkipModel.loadSettings( _this, "arpskip" );
	m_arpMissModel.loadSettings( _this, "arpmiss" );
	m_arpTimeModel.loadSettings( _this, "arptime" );
	m_arpGateModel.loadSettings( _this, "arpgate" );
	m_arpDirectionModel.loadSettings( _this, "arpdir" );
/*
	// Keep compatibility with version 0.2.1 file format
	if( _this.hasAttribute( "arpsyncmode" ) )
	{
	 	m_arpTimeKnob->setSyncMode( 
 		( tempoSyncKnob::tempoSyncMode ) _this.attribute(
 						 "arpsyncmode" ).toInt() );
	}*/

	m_arpModeModel.loadSettings( _this, "arpmode" );
}




InstrumentFunctionNoteHumanizing::InstrumentFunctionNoteHumanizing( Model * _parent ) :
	InstrumentFunction( _parent, tr( "NoteHumanizing" ) ),
	//m_enabledModel( false, this ),
	m_volumeRangeModel( 0.0f, 0.0f, 100.0f, 0.1f, this, tr( "Volume decrease" ) ),
	m_panRangeModel( 0.0f, 0.0f, 100.0f, 0.1f, this, tr( "Pan change" ) ),
	m_tuneRangeModel( 0.0f, 0.0f, 100.0f, 0.1f, this, tr( "Frequency change" ) ),
	m_offsetRangeModel( 0.0f, 0.0f, 100.0f, 0.1f, this, tr( "Start delay" ) ),
	m_shortenRangeModel( 0.0f, 0.0f, 100.0f, 0.1f, this, tr( "Shortening" ) )
{
}




InstrumentFunctionNoteHumanizing::~InstrumentFunctionNoteHumanizing()
{
}




void InstrumentFunctionNoteHumanizing::processNote( NotePlayHandle * _n )
{
	if( _n->totalFramesPlayed() == 0 &&
	    m_enabledModel.value() == true && ! _n->isReleased() )
	{
		{
			float l=m_volumeRangeModel.value()/100.f;
			if(l>0.f)
			{
				float r=fastrandf01inc();
				volume_t o=_n->getVolume(); // 0..200
				volume_t n=qBound(0,qRound(o*(1.f-l*r)),200);
				//qInfo("NH: volume %d->%d",o,n);
				_n->setVolume(n);
			}
		}

		{
			float l=m_panRangeModel.value()/100.f;
			if(l>0.f)
			{
				float r=fastrandf01inc();
				panning_t o=_n->getPanning(); // -100..100
				panning_t n=qBound(-100,qRound(o+200.f*l*(r-0.5f)),100);
				//qInfo("NH: panning %d->%d",o,n);
				_n->setPanning(n);
			}
		}

		{
			float l=m_tuneRangeModel.value()/100.f;
			if(l>0.f)
			{
				float r=fastrandf01inc();
				float o=_n->baseDetune();
				float n=o+12.f*(l*(r-0.5f));
				//qInfo("NH: detune %f->%f",o,n);
				_n->setBaseDetune(n);
				_n->setFrequencyUpdate();
			}
		}

		{
			float l=m_offsetRangeModel.value()/100.f;
			if(l>0.f)
			{
				float   r=fastrandf01inc();
				f_cnt_t o=_n->offset(); // ?
				const fpp_t fpt = Engine::framesPerTick()*Engine::getSong()->ticksPerTact();
				const fpp_t fpp = Engine::mixer()->framesPerPeriod();
				f_cnt_t n=qRound(o+(fpt-1-o)*l*r);
				n=qBound(o,n,fpp-1); // tmp: must be inside the period
				//qInfo("NH: offset %d->%d",o,n);
				_n->setOffset(n);
			}
		}

		{
			float l=m_shortenRangeModel.value()/100.f;
			if(l>0.f)
			{
				float   r=fastrandf01inc();
				f_cnt_t o=_n->frames(); // ?
				f_cnt_t n=qBound(1,qRound(o*(1.f-l*r)),o);
				qInfo("NH: shorten %d->%d",o,n);
				_n->setFrames(n);
			}
		}
	}
}




void InstrumentFunctionNoteHumanizing::saveSettings( QDomDocument & _doc, QDomElement & _this )
{
	m_enabledModel     .saveSettings( _doc, _this, "enabled" );
	m_volumeRangeModel .saveSettings( _doc, _this, "volume" );
	m_panRangeModel    .saveSettings( _doc, _this, "pan" );
	m_tuneRangeModel   .saveSettings( _doc, _this, "tune" );
	m_offsetRangeModel .saveSettings( _doc, _this, "offset" );
	m_shortenRangeModel.saveSettings( _doc, _this, "shorten" );
}




void InstrumentFunctionNoteHumanizing::loadSettings( const QDomElement & _this )
{
	m_enabledModel     .loadSettings( _this, "enabled" );
	m_volumeRangeModel .loadSettings( _this, "volume" );
	m_panRangeModel    .loadSettings( _this, "pan" );
	m_tuneRangeModel   .loadSettings( _this, "tune" );
	m_offsetRangeModel .loadSettings( _this, "offset" );
	m_shortenRangeModel.loadSettings( _this, "shorten" );
}
