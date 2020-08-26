/*
 * TimeDisplayWidget.cpp - widget for displaying current playback time
 *
 * Copyright (c) 2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "TimeDisplayWidget.h"

#include "Engine.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "Song.h"
#include "ToolTip.h"

#include <QMouseEvent>

TimeDisplayWidget::TimeDisplayWidget() :
      QWidget(), m_displayMode(MinutesSeconds), m_spinBoxesLayout(this),
      m_majorLCD(4, this), m_minorLCD(2, this), m_milliSecondsLCD(3, this)
{
    m_spinBoxesLayout.setSpacing(0);
    m_spinBoxesLayout.setMargin(0);
    m_spinBoxesLayout.addWidget(&m_majorLCD);
    m_spinBoxesLayout.addWidget(&m_minorLCD);
    m_spinBoxesLayout.addWidget(&m_milliSecondsLCD);

    setMaximumHeight(32);

    ToolTip::add(this, tr("click to change time units"));

    // update labels of LCD spinboxes
    setDisplayMode(m_displayMode);

    connect(gui->mainWindow(), SIGNAL(periodicUpdate()), this,
            SLOT(updateTime()));
}

TimeDisplayWidget::~TimeDisplayWidget()
{
}

void TimeDisplayWidget::setDisplayMode(DisplayMode displayMode)
{
    m_displayMode = displayMode;

    switch(m_displayMode)
    {
        case MinutesSeconds:
            m_majorLCD.setText(tr("MIN"));
            m_minorLCD.setText(tr("SEC"));
            m_milliSecondsLCD.setText(tr("MSEC"));
            break;

        case BarsTicks:
            m_majorLCD.setText(tr("BAR"));
            m_minorLCD.setText(tr("BEAT"));
            m_milliSecondsLCD.setText(tr("TICK"));
            break;

        default:
            break;
    }
}

void TimeDisplayWidget::updateTime()
{
    Song* s  = Engine::getSong();
    int   ms = s->getMilliseconds();

    switch(m_displayMode)
    {
        case MinutesSeconds:
        {
            int mins = ms / 60000;
            int secs = (ms / 1000) % 60;
            m_majorLCD.setValue(mins);
            m_minorLCD.setValue(secs);
            m_milliSecondsLCD.setValue(ms % 1000);
        }
        break;

        case BarsTicks:
        {
            int TPT  = s->ticksPerTact();
            int NUM  = s->getTimeSigModel().getNumerator();
            int tick = (ms * s->getTempo() * (DefaultTicksPerTact / 4))
                       / 60000;
            m_majorLCD.setValue((tick / TPT) + 1);
            m_minorLCD.setValue((tick % TPT) / (TPT / NUM) + 1);
            m_milliSecondsLCD.setValue((tick % TPT) % (TPT / NUM));
        }
        break;

        default:
            break;
    }
}

void TimeDisplayWidget::mousePressEvent(QMouseEvent* mouseEvent)
{
    if(mouseEvent->button() == Qt::LeftButton)
    {
        if(m_displayMode == MinutesSeconds)
        {
            setDisplayMode(BarsTicks);
        }
        else
        {
            setDisplayMode(MinutesSeconds);
        }
    }
}
