/*
 * debug.h - header file to be included for debugging purposes
 *
 * Copyright (c) 2004-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef DEBUG_H
#define DEBUG_H

//#include <cstdio>
#include "lmmsconfig.h"

#include <QDebug>
#include <QObject>
#include <QThread>

#ifndef qInfo
#define qInfo qWarning
#endif

#ifndef qNotice
#define qNotice qWarning
#endif

// set whether debug-stuff (like messages on the console, asserts and other
// additional range-checkings) should be compiled

#ifdef LMMS_DEBUG

#include <cassert>

#define DEBUG_THREAD_CHECK                   \
    if(QThread::currentThread() != thread()) \
        qWarning("Warning: %s#%d: wrong thread", __FILE__, __LINE__);

#define DEBUG_THREAD_PRINT                                    \
    qInfo("THREAD: CT=%s P=%d %s#%d",                         \
          qPrintable(QThread::currentThread()->objectName()), \
          QThread::currentThread()->priority(), __FILE__, __LINE__);

#define DEBUG_CONNECT(sender, signal, receiver, slot)                    \
    {                                                                    \
        qInfo("DEBUG_CONNECT from %p '%s' to %p '%s'", sender,           \
              STRINGIFY(signal), receiver, STRINGIFY(slot));             \
        QMetaObject::Connection c                                        \
                = QObject::connect(sender, signal, receiver, slot);      \
        if(!c)                                                           \
            qt_assert_x(Q_FUNC_INFO, "CHECKED_CONNECT failed", __FILE__, \
                        __LINE__);                                       \
    }

#define DEBUG_DISCONNECT(sender, signal, receiver, slot)          \
    {                                                             \
        qInfo("DEBUG_DISCONNECT from %p '%s' to %p '%s'", sender, \
              STRINGIFY(signal), receiver, STRINGIFY(slot));      \
        QObject::disconnect(sender, signal, receiver, slot);      \
    }

#else

#ifndef assert
#define assert(x) ((void)(x))
#endif

#define DEBUG_THREAD_CHECK

#define DEBUG_THREAD_PRINT

#define DEBUG_CONNECT(sender, signal, receiver, slot) \
    connect(sender, signal, receiver, slot);

#define DEBUG_DISCONNECT(sender, signal, receiver, slot) \
    disconnect(sender, signal, receiver, slot);

#endif  // LMMS_DEBUG

#endif  // DEBUG_H
