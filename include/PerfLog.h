/*
 * PerfLog.h - Small performance logger
 *
 * Copyright (c) 2017 gi0e5b06
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

#ifndef PERFLOG_H
#define PERFLOG_H

#include "lmmsconfig.h"

#ifdef LMMS_BUILD_LINUX
#include <sys/times.h>
#include <unistd.h>
#endif

#ifdef LMMS_DEBUG_PERFLOG

#include <QHash>
#include <QString>

class PerfLog
{
  public:
    static void begin(const QString& what);
    static void end(const QString& what, const uint32_t milliseconds = 0);

  private:
    class Entry
    {
      public:
#ifdef LMMS_BUILD_LINUX
        clock_t c;
        tms     t;
#endif
        Entry();
    };

    class Cumul
    {
      public:
        float   ctreal;
        float   ctuser;
        float   ctsyst;
        clock_t lastprint;
        Cumul();
    };

    static QHash<QString, PerfLog::Entry> s_running;
    static QHash<QString, PerfLog::Cumul> s_cumulated;
};

#define PL_BEGIN(w) PerfLog::begin(w);
#define PL_END(w) PerfLog::end(w);
#define qTrace(...) qInfo( __VA_ARGS__ )

#else

#define PL_BEGIN(w)
#define PL_END(w)
#define qTrace(...)

#endif

#endif
