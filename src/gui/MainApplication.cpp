/*
 * MainApplication.cpp - Main QApplication handler
 *
 * Copyright (c) 2017-2017 Tres Finocchiaro <tres.finocchiaro/at/gmail.com>
 *
 * This file is part of LMMS - http://lmms.io
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
#include "MainApplication.h"

//#include <QDebug>
#include "Engine.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "Song.h"

#include <QFileOpenEvent>

MainApplication::MainApplication(int& argc, char** argv) :
      QApplication(argc, argv), m_queuedFile()
{
}

bool MainApplication::event(QEvent* event)
{
    Song* song = Engine::song();

    switch(event->type())
    {
        case QEvent::FileOpen:
        {
            QFileOpenEvent* fileEvent = static_cast<QFileOpenEvent*>(event);
            // Handle the project file
            m_queuedFile = fileEvent->file();
            if(song != nullptr)
            {
                if(gui->mainWindow()->mayChangeProject(true))
                {
                    qInfo("Notice: Loading file: %s",
                          qPrintable(m_queuedFile));
                    song->loadProject(m_queuedFile);
                }
            }
            else
            {
                qInfo("Queuing file: %s", qPrintable(m_queuedFile));
            }
            return true;
        }
        default:
            return QApplication::event(event);
    }
}

bool MainApplication::notify(QObject* receiver, QEvent* event)
{
    Song* song = Engine::song();

    switch(event->type())
    {
        case QEvent::Gesture:
        case QEvent::KeyPress:
        case QEvent::MouseButtonPress:
        case QEvent::Shortcut:
        case QEvent::TouchBegin:
        case QEvent::Wheel:
            if(song != nullptr)
            {
                // qInfo("MainApplication: event %d", event->type());
                song->userWorking();
            }
        default:
            break;
    }

    return QApplication::notify(receiver, event);
}

#ifdef LMMS_BUILD_WIN32
bool MainApplication::winEventFilter(MSG* msg, long* result)
{
    switch(msg->message)
    {
        case WM_STYLECHANGING:
            if(msg->wParam == GWL_EXSTYLE)
            {
                // Prevent plugins making the main window transparent
                STYLESTRUCT* style
                        = reinterpret_cast<STYLESTRUCT*>(msg->lParam);
                if(!(style->styleOld & WS_EX_LAYERED))
                {
                    style->styleNew &= ~WS_EX_LAYERED;
                }
                *result = 0;
                return true;
            }
            return false;
        default:
            return false;
    }
}
#endif
