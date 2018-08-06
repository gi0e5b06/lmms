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

#include "PerfLog.h"

#include <QThread>

#ifdef LMMS_DEBUG_PERFLOG

QHash<QString, PerfLog::Entry> PerfLog::s_running;
QHash<QString, PerfLog::Cumul> PerfLog::s_cumulated;

PerfLog::Entry::Entry()
{
#ifdef LMMS_BUILD_LINUX
    c = times(&t);
    if(c == -1)
        qFatal("PerfLogEntry: init failed");
#endif
}

PerfLog::Cumul::Cumul() :
      ctreal(0.0f), ctuser(0.0f), ctsyst(0.0f), lastprint(0)
{
}

void PerfLog::begin(const QString& what)
{
    if(s_running.contains(what))
        qWarning("PerfLog::begin already %s", qPrintable(what));

    s_running.insert(what, Entry());
}

void PerfLog::end(const QString& what, const uint32_t milliseconds)
{
#ifdef LMMS_BUILD_LINUX
    static long clktck = 0l;
    if(!clktck)
        if((clktck = sysconf(_SC_CLK_TCK)) < 0)
            qFatal("PerfLog::end sysconf()");

    PerfLog::Entry e;
    PerfLog::Entry b = s_running.take(what);

    float treal = (e.c - b.c) / (double)clktck;
    float tuser = (e.t.tms_utime - b.t.tms_utime) / (double)clktck;
    float tsyst = (e.t.tms_stime - b.t.tms_stime) / (double)clktck;

    PerfLog::Cumul& c = s_cumulated[what];
    c.ctreal += treal;
    c.ctuser += tuser;
    c.ctsyst += tsyst;

    if(1000. * (e.c - c.lastprint) / (double)clktck >= milliseconds)
    {
        c.lastprint = e.c;
        //    "        | task | real  | user  | syst  | creal | cuser |
        //     csyst | thread"
        qInfo("PERFLOG | %20s | %8.3f | %8.3f | %8.3f | %8.3f | %8.3f | "
              "%8.3f | %20s",
              qPrintable(what), treal, tuser, tsyst, c.ctreal, c.ctuser,
              c.ctsyst, qPrintable(QThread::currentThread()->objectName()));
    }
#else
    s_running.take(what);
    qInfo("PERFLOG | %20s | n/a", qPrintable(what));
#endif
}

#endif
