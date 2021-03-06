/*
 * PaintManager.cpp -
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

#include "PaintManager.h"

#include "Engine.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "Mixer.h"
#include "PaintCacheable.h"

#include <QWidget>

PaintManager PaintManager::s_singleton;

PaintManager::PaintManager()
{
}

PaintManager::~PaintManager()
{
    while(!m_queue.empty())
        runQueue();
}

void PaintManager::runQueue()
{
    // qInfo("PaintManager::runQueue %d", m_queue.size());
    while(!m_queue.empty())
    {
        if(Engine::mixer()->warningXRuns())
            QThread::yieldCurrentThread();
        PaintCacheable* w = m_queue.dequeue();
        if(m_unique.remove(w))
        {
            QObject* o = dynamic_cast<QObject*>(w);
            if(o != nullptr)
                QObject::disconnect(o, SIGNAL(destroyed(QObject*)),
                                    &s_singleton,
                                    SLOT(onObjectDestroyed(QObject*)));
        }
        w->updateNow();
    }
}

void PaintManager::start(MainWindow* _mw)
{
    QObject::connect(_mw, SIGNAL(periodicUpdate()), &s_singleton,
                     SLOT(runQueue()));
}

void PaintManager::updateNow()
{
    s_singleton.runQueue();
}

void PaintManager::updateLater(PaintCacheable* _w)
{
    if(s_singleton.m_unique.contains(_w))
        return;

    s_singleton.m_unique.insert(_w, true);
    s_singleton.m_queue.enqueue(_w);
    // qInfo("PaintManager::updateLater %d", s_singleton.m_queue.size());

    QObject* o = dynamic_cast<QObject*>(_w);
    if(o != nullptr)
        QObject::connect(o, SIGNAL(destroyed(QObject*)), &s_singleton,
                         SLOT(onObjectDestroyed(QObject*)),
                         Qt::UniqueConnection);
}

void PaintManager::removeLater(PaintCacheable* _w)
{
    s_singleton.m_queue.removeAll(_w);
    if(s_singleton.m_unique.remove(_w))
    {
        QObject* o = dynamic_cast<QObject*>(_w);
        if(o != nullptr)
            QObject::disconnect(o, SIGNAL(destroyed(QObject*)), &s_singleton,
                                SLOT(onObjectDestroyed(QObject*)));
    }
}

void PaintManager::onObjectDestroyed(QObject* _o)
{
    qInfo("PaintManager::onObjectDestroyed");
    PaintCacheable* w = dynamic_cast<PaintCacheable*>(_o);
    removeLater(w);
}
