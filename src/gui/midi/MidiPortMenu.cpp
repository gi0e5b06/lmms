/*
 * MidiPortMenu.cpp - a menu for subscribing a MidiPort to several external
 *                      MIDI ports
 *
 * Copyright (c) 2008-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "MidiPortMenu.h"

#include "gui_templates.h"

MidiPortMenu::MidiPortMenu(MidiPort* _model, MidiPort::Modes _mode) :
      ModelView(_model, this), m_mode(_mode)
{
    // setFont( pointSize<9>( font() ) );
    connect(this, SIGNAL(triggered(QAction*)), this,
            SLOT(activatedPort(QAction*)));
}

MidiPortMenu::MidiPortMenu(MidiPort::Modes _mode) :
      ModelView(this), m_mode(_mode)
{
    // setFont( pointSize<9>( font() ) );
    connect(this, SIGNAL(triggered(QAction*)), this,
            SLOT(activatedPort(QAction*)));
}

MidiPortMenu::~MidiPortMenu()
{
}

void MidiPortMenu::doConnections()
{
    MidiPort* m = model();//castModel<MidiPort>();
    if(m != nullptr)
    {
        ModelView::doConnections();
        if(m_mode == MidiPort::Input)
            connect(m, SIGNAL(readablePortsChanged()), this,
                    SLOT(updateMenu()));
        if(m_mode == MidiPort::Output)
            connect(m, SIGNAL(writablePortsChanged()), this,
                    SLOT(updateMenu()));
    }
}

void MidiPortMenu::undoConnections()
{
    MidiPort* m = model();//castModel<MidiPort>();
    if(m != nullptr)
    {
        if(m_mode == MidiPort::Input)
            disconnect(m, SIGNAL(readablePortsChanged()), this,
                       SLOT(updateMenu()));
        if(m_mode == MidiPort::Output)
            disconnect(m, SIGNAL(writablePortsChanged()), this,
                       SLOT(updateMenu()));
        ModelView::undoConnections();
    }
}

void MidiPortMenu::modelChanged()
{
    updateMenu();
}

void MidiPortMenu::activatedPort(QAction* _item)
{
    if(m_mode == MidiPort::Input)
        model()->subscribeReadablePort(_item->text(), _item->isChecked());
    if(m_mode == MidiPort::Output)
        model()->subscribeWritablePort(_item->text(), _item->isChecked());
}

void MidiPortMenu::updateMenu()
{
    MidiPort* mp = model();  // castModel<MidiPort>();

    const MidiPort::Map& map = (m_mode == MidiPort::Input)
                                       ? mp->readablePorts()
                                       : mp->writablePorts();
    clear();
    for(MidiPort::Map::ConstIterator it = map.begin(); it != map.end(); ++it)
    {
        QAction* a = addAction(it.key());
        a->setCheckable(true);
        a->setChecked(it.value());
    }
}
