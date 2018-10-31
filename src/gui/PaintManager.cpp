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

#include "GuiApplication.h"
#include "MainWindow.h"
#include "PaintCacheable.h"
#include "Engine.h"
#include "Mixer.h"

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
    int n=0;
    while(!m_queue.empty())
    {
        QThread::yieldCurrentThread();
        if(n<20 && Engine::mixer()->warningXRuns())
        {
                n++;
                QThread::msleep(20);
                continue;
        }
        PaintCacheable* w = m_queue.dequeue();
        m_unique.remove(w);
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
}

void PaintManager::removeLater(PaintCacheable* _w)
{
    s_singleton.m_queue.removeAll(_w);
}
