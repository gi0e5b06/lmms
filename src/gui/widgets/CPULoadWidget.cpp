/*
 * CPULoadWidget.cpp - widget for displaying CPU-load (partly based on
 *                      Hydrogen's CPU-load-widget)
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

#include "CPULoadWidget.h"

#include "Engine.h"
#include "Mixer.h"
#include "embed.h"

#include <QPainter>

CPULoadWidget::CPULoadWidget(QWidget* _parent, const bool _bigger) :
      Widget(_parent), m_bigger(_bigger), m_currentLoad(0)
      //, m_changed(true), m_updateTimer()
{
    if(m_bigger)
    {
        m_background = embed::getPixmap("cpuload_bigger_bg");
        m_foreground = embed::getPixmap("cpuload_bigger_fg");
        m_leds       = embed::getPixmap("cpuload_bigger_leds");
    }
    else
    {
        m_background = embed::getPixmap("cpuload_bg");
        m_leds       = embed::getPixmap("cpuload_leds");
    }

    const int w = m_background.width();
    const int h = m_background.height();

    //m_cache = QPixmap(w, h);
    setFixedSize(w, h);
    //resizeCache(w,h);
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    //connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateCpuLoad()));
    //m_updateTimer.start(1000 / 8);
}

CPULoadWidget::~CPULoadWidget()
{
}

void CPULoadWidget::drawWidget(QPainter& _p)
{
    _p.setRenderHints(QPainter::Antialiasing, false);
    _p.drawPixmap(0, 0, m_background);

    if(m_bigger)
    {
        int w = (m_leds.width() * m_currentLoad / 300) * 3;
        if(w > 0)
            _p.drawPixmap(0, 0, m_leds, 0, 0, w, m_leds.height());
    }
    else
    {
        // as load-indicator consists of small 2-pixel wide leds with
        // 1 pixel spacing, we have to make sure, only whole leds are
        // shown which we achieve by the following formula
        int w = (m_leds.width() * m_currentLoad / 300) * 3;
        if(w > 0)
            _p.drawPixmap(23, 3, m_leds, 0, 0, w, m_leds.height());
    }

    if(m_bigger)
        _p.drawPixmap(0, 0, m_foreground);
}

/*
void CPULoadWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    if(paintCache(p))
        return;

    QPainter& pc = beginCache();
    drawWidget(pc);
    endCache();

    paintCache(p);
}
*/

void CPULoadWidget::refresh()
{
    // smooth load-values a bit
    // int new_load = ( m_currentLoad + Engine::mixer()->cpuLoad() ) / 2;

    // int new_load = qMax<int>((m_currentLoad*9)/10,
    //                         Engine::mixer()->cpuLoad());

    int new_load = Engine::mixer()->cpuLoad();
    if(new_load != m_currentLoad)
    {
        m_currentLoad = new_load;
        invalidateCache();
        update();
    }
}
