/*
 * CPULoadWidget.h - widget for displaying CPU-load (partly based on
 *                    Hydrogen's CPU-load-widget)
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

#ifndef CPULOAD_WIDGET_H
#define CPULOAD_WIDGET_H

//#include "lmms_basics.h"

//#include "PaintCacheable.h"
#include "Widget.h"

#include <QPixmap>
#include <QTimer>
//#include <QWidget>

class CPULoadWidget
      : public Widget
      , public virtual PaintCacheable
{
    Q_OBJECT

  public:
    CPULoadWidget(QWidget* _parent, const bool _bigger);
    virtual ~CPULoadWidget();

  protected:
    virtual void drawWidget(QPainter& _p);
    //virtual void paintEvent(QPaintEvent* _pe);

    // interfaces
    //using PaintCacheable::update;
    //virtual void updateNow() { QWidget::update(); }

  protected slots:
    void refresh();

  private:
    bool   m_bigger;
    int    m_currentLoad;
    //bool   m_changed;
    //QTimer m_updateTimer;

    QPixmap m_background;
    QPixmap m_foreground;
    QPixmap m_leds;
    //QPixmap m_cache;
};

#endif
