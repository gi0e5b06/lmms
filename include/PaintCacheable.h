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

#ifndef PAINT_CACHEABLE_H
#define PAINT_CACHEABLE_H

//#include <QMutex>
#include <QPainter>
//#include <QSharedPointer>

class QImage;

class PaintCacheable
{

  public:
    PaintCacheable();
    virtual ~PaintCacheable();

    bool      paintCache(QPainter& _p);
    bool      isCacheValid();
    void      resizeCache(int _w, int _h);
    QPainter& beginCache();
    void      endCache();

  public slots:
    void invalidateCache();
    // virtual void refresh();
    virtual void update();
    virtual void updateNow() = 0;

  private:
    volatile bool m_valid;
    QImage*       m_cache;
    QPainter*     m_painter;
};

#endif
