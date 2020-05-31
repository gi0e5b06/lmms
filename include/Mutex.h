/*
 * Mutex.h -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include <QMutex>
#include <QString>

class Mutex : public QMutex
{
  public:
    Mutex(const QString&        _name,
          QMutex::RecursionMode _mode = NonRecursive) :
          QMutex(_mode),
          m_name(_name)
    {
    }

    // virtual ~Mutex();
    // bool isRecursive() const;

    virtual void lock()
    {
        qInfo("Mutex::lock %s before", qPrintable(m_name));
        QMutex::lock();
        qInfo("Mutex::lock %s after", qPrintable(m_name));
    }

    virtual bool tryLock(int _timeout = 0)
    {
        qInfo("Mutex::tryLock %s before", qPrintable(m_name));
        bool r=QMutex::tryLock(_timeout);
        qInfo("Mutex::tryLock %s before", qPrintable(m_name));
        return r;
    }

    virtual bool try_lock()
    {
        qInfo("Mutex::try_lock %s before", qPrintable(m_name));
        bool r=QMutex::try_lock();
        qInfo("Mutex::try_lock %s after", qPrintable(m_name));
        return r;
    }

    virtual void unlock()
    {
        qInfo("Mutex::unlock %s before", qPrintable(m_name));
        QMutex::unlock();
        qInfo("Mutex::unlock %s after", qPrintable(m_name));
    }

  private:
    QString m_name;
};

#endif
