/*
 * Mutex.h -
 *
 * Copyright (c) 2019-2020 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef MUTEX_H
#define MUTEX_H

#include "Backtrace.h"

#include <QMutex>
#include <QString>
#include <QThread>

class Mutex : public QMutex
{
  public:
    Mutex(const QString&        _name,
          QMutex::RecursionMode _mode = NonRecursive,
          bool                  _info = false) :
          QMutex(_mode),
          m_name(_name), m_info(_info), m_locked(0)
    {
    }

    Mutex(const QString&        _name,
          bool                  _info,
          QMutex::RecursionMode _mode = NonRecursive) :
          QMutex(NonRecursive),
          m_name(_name), m_info(_info), m_locked(0)
    {
    }

    // virtual ~Mutex();
    // bool isRecursive() const;

    virtual void lock()
    {
        if(m_info)
            qInfo("Mutex::lock %s before", qPrintable(m_name));
        if(m_locked > 0 && !isRecursive()
           && m_thread == QThread::currentThread())
        {
            BACKTRACE
            qWarning("Mutex::lock %s is already locked (%d,%s)",
                     qPrintable(m_name), m_locked,
                     qPrintable(m_thread->objectName()));
        }
        QMutex::lock();
        m_locked++;
        m_thread = QThread::currentThread();
        if(m_info)
            qInfo("Mutex::lock %s after", qPrintable(m_name));
    }

    virtual bool tryLock(int _timeout = 0)
    {
        if(m_info)
            qInfo("Mutex::tryLock %s before", qPrintable(m_name));
        bool r = QMutex::tryLock(_timeout);
        if(r)
        {
            m_locked++;
            m_thread = QThread::currentThread();
            if(m_locked > 1 && !isRecursive())
            {
                BACKTRACE
                qWarning("Mutex::tryLock %s is already locked",
                         qPrintable(m_name));
            }
        }
        if(m_info)
            qInfo("Mutex::tryLock %s after", qPrintable(m_name));
        return r;
    }

    virtual bool try_lock()
    {
        return tryLock();
    }

    virtual void unlock()
    {
        if(m_info)
            qInfo("Mutex::unlock %s before", qPrintable(m_name));
        if(m_locked < 1)
        {
            BACKTRACE
            qWarning("Mutex::unlock %s is not locked", qPrintable(m_name));
        }
        m_thread = nullptr;
        m_locked--;
        QMutex::unlock();
        if(m_info)
            qInfo("Mutex::unlock %s after", qPrintable(m_name));
    }

  private:
    QString  m_name;
    bool     m_info;
    int      m_locked;
    QThread* m_thread;
};

#endif
