/*
 * PaintCacheable.h -
 *
 * Copyright (c) 2018 gi0e5b06
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

#ifndef PAINT_MANAGER_H
#define PAINT_MANAGER_H

//#include "PaintCacheable.h"

//#include <QBasicTimer>
#include <QHash>
#include <QObject>
#include <QQueue>
//#include <QWeakPointer>

class PaintCacheable;
class MainWindow;

class PaintManager : public QObject
{
    Q_OBJECT

  public:
    static void updateNow();
    static void updateLater(PaintCacheable* _w);
    static void removeLater(PaintCacheable* _w);

  public slots:
    void runQueue();

  signals:
    void periodicUpdate();

  private:
    PaintManager();
    virtual ~PaintManager();

    QQueue<PaintCacheable*>      m_queue;
    QHash<PaintCacheable*, bool> m_unique;

    static PaintManager s_singleton;
    static void         start(MainWindow* _mw);

    friend class MainWindow;
};

#endif
