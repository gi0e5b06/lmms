/*
 * PaintCacheable.cpp -
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

#include "PaintCacheable.h"

#include "PaintManager.h"

#include <QWidget>

PaintCacheable::PaintCacheable() :
      m_valid(false), m_cache(NULL), m_painter(NULL)
{
}

PaintCacheable::~PaintCacheable()
{
    PaintManager::removeLater(this);
    if(m_painter)
        delete m_painter;
    if(m_cache)
        delete m_cache;
}

bool PaintCacheable::paintCache(QPainter& _p)
{
    if(m_valid)
        _p.drawImage(0, 0, *m_cache);

    return m_valid;
}

bool PaintCacheable::isCacheValid()
{
    return m_valid;
}

void PaintCacheable::invalidateCache()
{
    m_valid = false;
}

void PaintCacheable::resizeCache(int _w, int _h)
{
    if(!m_cache || _w != m_cache->width() || _h != m_cache->height())
    {
        m_valid = false;

        // qInfo("PaintCacheable::resizeCache");
        QImage*   oldc = m_cache;
        QPainter* oldp = m_painter;

        m_cache   = new QImage(_w, _h, QImage::Format_ARGB32);
        m_painter = new QPainter();

        if(oldp)
            delete oldp;
        if(oldc)
            delete oldc;

        if(m_valid)
            qWarning("PaintCacheable::resizeCache");
    }
}

QPainter& PaintCacheable::beginCache()
{
    m_valid = false;
    m_cache->fill(qRgba(0, 0, 0, 0));
    m_painter->begin(m_cache);
    m_painter->setRenderHints(QPainter::Antialiasing, false);
    return *m_painter;
}

void PaintCacheable::endCache()
{
    m_painter->end();
    if(m_valid)
        qWarning("PaintCacheable::endCache");
    m_valid = true;
}

void PaintCacheable::update()
{
    invalidateCache();
    // qInfo("PaintCacheable::update");
    if(m_cache && m_painter)
        PaintManager::updateLater(this);
    else
        updateNow();
}

/*
void PaintCacheable::refresh()
{
    invalidateCache();
    update();
}
*/
