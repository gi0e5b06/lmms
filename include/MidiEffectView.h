/*
 * MidiEffectView.h - view-component for an effect
 *
 * Copyright (c) 2006-2007 Danny McRae <khjklujn/at/users.sourceforge.net>
 * Copyright (c) 2007-2010 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef MIDI_EFFECT_VIEW_H
#define MIDI_EFFECT_VIEW_H

//#include "AutomatableModel.h"
#include "MidiEffect.h"
#include "PluginView.h"

class QGroupBox;
class QLabel;
class QPushButton;
class QMdiSubWindow;

class MidiEffectControlDialog;
class Knob;
class LedCheckBox;
class TempoSyncKnob;


class MidiEffectView : public PluginView
{
	Q_OBJECT
public:
	MidiEffectView( MidiEffect * _model, QWidget * _parent );
	virtual ~MidiEffectView();

	inline MidiEffect * effect()
	{
		return castModel<MidiEffect>();
	}
	inline const MidiEffect * effect() const
	{
		return castModel<MidiEffect>();
	}


public slots:
	void editControls();
	void moveUp();
	void moveDown();
	void deletePlugin();
	void displayHelp();
	void closeEffects();


signals:
	void moveUp( MidiEffectView * _plugin );
	void moveDown( MidiEffectView * _plugin );
	void deletePlugin( MidiEffectView * _plugin );


protected:
	virtual void contextMenuEvent( QContextMenuEvent * _me );
	virtual void paintEvent( QPaintEvent * _pe );
	virtual void modelChanged();


private:
	QPixmap m_bg;
	LedCheckBox * m_bypass;
	Knob * m_wetDry;
	TempoSyncKnob * m_autoQuit;
	Knob * m_gate;
	QMdiSubWindow * m_subWindow;
	MidiEffectControlDialog * m_controlView;

} ;

#endif
