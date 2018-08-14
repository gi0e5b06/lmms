/*
 * PeripheralView.cpp - implementation of peripheral-widget used in instrument-track-window
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

/** \file piano.cpp
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

#include "PeripheralView.h"

#include "AutomatableModelView.h"
#include "CaptionMenu.h"
#include "InstrumentTrack.h"
#include "Piano.h"

#include "embed.h"
//#include "gui_templates.h"

#include <QCursor>
#include <QKeyEvent>
//#include <QPainter>
//#include <QVBoxLayout>

#include <cmath>

/*! \brief Create a new peripheral display view
 *
 *  \param _parent the parent instrument plugin window
 */
PeripheralView::PeripheralView( QWidget * _parent ) :
	QWidget( _parent ),       /*!< Our parent */
	ModelView( NULL, this ),  /*!< Our view Model */
        m_piano( NULL )           /*!< Our piano Model */

{
}




/*! \brief Destroy this peripheral display view
 *
 */
PeripheralView::~PeripheralView()
{
}


/*! \brief Map a keyboard key being pressed to a note in our keyboard view
 *
 *  \param _k The keyboard scan code of the key being pressed.
 *  \todo check the scan codes for ',' = c, 'L' = c#, '.' = d, ':' = d#,
 *     '/' = d, '[' = f', '=' = f'#, ']' = g' - Paul's additions
 */
int PeripheralView::getKeyFromKeyEvent( QKeyEvent * _ke )
{
#ifdef LMMS_BUILD_APPLE
	const int k = _ke->nativeVirtualKey();
#else
	const int k = _ke->nativeScanCode();
#endif

#ifdef LMMS_BUILD_WIN32
	switch( k )
	{
		case 44: return 0; // Z  = C
		case 31: return 1; // S  = C#
		case 45: return 2; // X  = D
		case 32: return 3; // D  = D#
		case 46: return 4; // C  = E
		case 47: return 5; // V  = F
		case 34: return 6; // G  = F#
		case 48: return 7; // B  = G
		case 35: return 8; // H  = G#
		case 49: return 9; // N  = A
		case 36: return 10; // J = A#
		case 50: return 11; // M = B
		case 51: return 12; // , = c
		case 38: return 13; // L = c#
		case 52: return 14; // . = d
		case 39: return 15; // ; = d#
		//case 86: return 16; // / = e
		case 53: return 16; // / = e
		case 16: return 12; // Q = c
		case 3: return 13; // 2 = c#
		case 17: return 14; // W = d
		case 4: return 15; // 3 = d#
		case 18: return 16; // E = e
		case 19: return 17; // R = f
		case 6: return 18; // 5 = f#
		case 20: return 19; // T = g
		case 7: return 20; // 6 = g#
		case 21: return 21; // Y = a
		case 8: return 22; // 7 = a#
		case 22: return 23; // U = b
		case 23: return 24; // I = c'
		case 10: return 25; // 9 = c'#
		case 24: return 26; // O = d'
		case 11: return 27; // 0 = d'#
		case 25: return 28; // P = e'
		case 26: return 29; // [
		case 13: return 30; // =
		case 27: return 31; // ]
	}
#endif
#if defined(LMMS_BUILD_LINUX) || defined(LMMS_BUILD_OPENBSD)
	switch( k )
	{
		case 52: return 0; // Z  = C
		case 39: return 1; // S  = C#
		case 53: return 2; // X  = D
		case 40: return 3; // D  = D#
		case 54: return 4; // C  = E
		case 55: return 5; // V  = F
		case 42: return 6; // G  = F#
		case 56: return 7; // B  = G
		case 43: return 8; // H  = G#
		case 57: return 9; // N  = A
		case 44: return 10; // J = A#
		case 58: return 11; // M = B
		case 59: return 12; // , = c
		case 46: return 13; // L = c#
		case 60: return 14; // . = d
		case 47: return 15; // ; = d#
		case 61: return 16; // / = e
		case 24: return 12; // Q = c
		case 11: return 13; // 2 = c#
		case 25: return 14; // W = d
		case 12: return 15; // 3 = d#
		case 26: return 16; // E = e
		case 27: return 17; // R = f
		case 14: return 18; // 5 = f#
		case 28: return 19; // T = g
		case 15: return 20; // 6 = g#
		case 29: return 21; // Y = a
		case 16: return 22; // 7 = a#
		case 30: return 23; // U = b
		case 31: return 24; // I = c'
		case 18: return 25; // 9 = c'#
		case 32: return 26; // O = d'
		case 19: return 27; // 0 = d'#
		case 33: return 28; // P = e'
		case 34: return 29; // [
		case 21: return 30; // =
		case 35: return 31; // ]
	}
#endif
#ifdef LMMS_BUILD_APPLE
	switch( k )
	{
		case 6: return 0; // Z  = C
		case 1: return 1; // S  = C#
		case 7: return 2; // X  = D
		case 2: return 3; // D  = D#
		case 8: return 4; // C  = E
		case 9: return 5; // V  = F
		case 5: return 6; // G  = F#
		case 11: return 7; // B  = G
		case 4: return 8; // H  = G#
		case 45: return 9; // N  = A
		case 38: return 10; // J = A#
		case 46: return 11; // M = B
		case 43: return 12; // , = c
		case 37: return 13; // L = c#
		case 47: return 14; // . = d
		case 41: return 15; // ; = d#
		case 44: return 16; // / = e
		case 12: return 12; // Q = c
		case 19: return 13; // 2 = c#
		case 13: return 14; // W = d
		case 20: return 15; // 3 = d#
		case 14: return 16; // E = e
		case 15: return 17; // R = f
		case 23: return 18; // 5 = f#
		case 17: return 19; // T = g
		case 22: return 20; // 6 = g#
		case 16: return 21; // Y = a
		case 26: return 22; // 7 = a#
		case 32: return 23; // U = b
		case 34: return 24; // I = c'
		case 25: return 25; // 9 = c'#
		case 31: return 26; // O = d'
		case 29: return 27; // 0 = d'#
		case 35: return 28; // P = e'
	}
#endif

	return -100;
}


/*! \brief Register a change to this piano display view
 *
 */
void PeripheralView::modelChanged()
{
	m_piano = castModel<Piano>();
        //qInfo("PeripheralView::modelChanged piano=%p",m_piano);
	if( m_piano != NULL )
        {
                connect( m_piano->instrumentTrack()->baseNoteModel(),
                         SIGNAL( dataChanged() ),
                         this, SLOT( update() ) );
                connect( m_piano,
                         SIGNAL( dataChanged() ),
                         this, SLOT( update() ) );
                connect( m_piano,
                         SIGNAL( propertiesChanged() ),
                         this, SLOT( update() ) );
        }
        ModelView::modelChanged();
}


/*! \brief Handle a context menu selection on the peripheral display view
 *
 *  \param _me the ContextMenuEvent to handle.
 *  \todo Is this right, or does this create the context menu?
 */
void PeripheralView::contextMenuEvent( QContextMenuEvent * _me )
{
	CaptionMenu menu( tr( "Base Key" ) );

	AutomatableModelView amv( m_piano->instrumentTrack()->baseNoteModel(), &menu );
	amv.addDefaultActions( &menu );

	menu.addSeparator();
	menu.addAction( embed::getIconPixmap( "midi_tab" ),//"piano_view" ),
                         AutomatableModel::tr( "&Piano View" ),
                         parent(), SLOT( switchToPiano() ) );
	menu.addAction( embed::getIconPixmap( "bb_track" ),//"launchpad_view" ),
                         AutomatableModel::tr( "&Launchpad View" ),
                         parent(), SLOT( switchToLaunchpad() ) );
	menu.addAction( embed::getIconPixmap( "bb_track" ),//"pads_view" ),
                         AutomatableModel::tr( "P&ads View" ),
                         parent(), SLOT( switchToPads() ) );

	menu.exec( QCursor::pos() );
}
