/*
 * SafeList.h -
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

#ifndef SAFE_LIST_H
#define SAFE_LIST_H

#include "Backtrace.h"

#include <QList>
#include <QMutex>

#include <functional>

template <class T>
class SafeList
{
  public:
    SafeList(bool _checkUnicity = true)
    {
    }

    SafeList(const SafeList<T>& _other) :
          m_unicity(_other.m_unicity), m_list(_other.m_list)
    {
    }

    void append(T _e)
    {
        QMutexLocker lock(&m_mutex);
        if(m_unicity && m_list.contains(_e))
        {
            BACKTRACE
            qCritical("SafeList::append already contains that element");
            return;
        }
        m_list.append(_e);
    }

    void appendUnique(T _e)
    {
        QMutexLocker lock(&m_mutex);
        if(!m_list.contains(_e))
            m_list.append(_e);
    }

    void clear()
    {
        QMutexLocker lock(&m_mutex);
        m_list.clear();
    }

    bool contains(const T& _e) const
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        return m_list.contains(_e);
    }

    int indexOf(const T& _e) const
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        return m_list.indexOf(_e);
    }

    bool isEmpty() const
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        return m_list.isEmpty();
    }

    /*
    void prepend(T _e)
    {
        QMutexLocker lock(&m_mutex);
        m_list.append(_e);
    }
    */

    int removeAll(const T& _e, bool _check = true)
    {
        QMutexLocker lock(&m_mutex);
        if(_check && !m_list.contains(_e))
        {
            BACKTRACE
            qCritical("SafeList::removeAll doesn't contain that element");
            return 0;
        }
        return m_unicity ? (m_list.removeOne(_e) ? 1 : 0)
                         : m_list.removeAll(_e);
    }

    bool removeOne(const T& _e, bool _check = true)
    {
        QMutexLocker lock(&m_mutex);
        if(_check && !m_list.contains(_e))
        {
            BACKTRACE
            qCritical("SafeList::removeOne doesn't contain that element");
            return false;
        }
        return m_list.removeOne(_e);
    }

    bool moveUp(const T& _e, bool _check = true)
    {
        QMutexLocker lock(&m_mutex);
        if(_check && !m_list.contains(_e))
        {
            BACKTRACE
            qCritical("SafeList::moveUp doesn't contain that element");
            return false;
        }

        if(_e == m_list.first())
            return false;

        typename QVector<T>::Iterator it
                = qFind(m_list.begin(), m_list.end(), _e);

        it = m_list.erase(it);
        m_list.insert(it - 1, _e);
        return true;
    }

    bool moveDown(const T& _e, bool _check = true)
    {
        QMutexLocker lock(&m_mutex);
        if(_check && !m_list.contains(_e))
        {
            BACKTRACE
            qCritical("SafeList::moveDown doesn't contain that element");
            return false;
        }

        if(_e == m_list.last())
            return false;

        typename QVector<T>::Iterator it
                = qFind(m_list.begin(), m_list.end(), _e);

        it = m_list.erase(it);
        m_list.insert(it + 1, _e);
        return true;
    }

    bool moveTop(const T& _e, bool _check = true)
    {
        QMutexLocker lock(&m_mutex);
        if(_check && !m_list.contains(_e))
        {
            BACKTRACE
            qCritical("SafeList::moveTop doesn't contain that element");
            return false;
        }

        if(_e == m_list.first())
            return false;

        m_list.removeOne(_e);
        m_list.prepend(_e);
        return true;
    }

    bool moveBottom(const T& _e, bool _check = true)
    {
        QMutexLocker lock(&m_mutex);
        if(_check && !m_list.contains(_e))
        {
            BACKTRACE
            qCritical("SafeList::moveBottom doesn't contain that element");
            return false;
        }

        if(_e == m_list.last())
            return false;

        m_list.removeOne(_e);
        m_list.append(_e);
        return true;
    }

    int size() const
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        return m_list.size();
    }

    T takeFirst()
    {
        QMutexLocker lock(&m_mutex);
        if(m_list.isEmpty())
        {
            BACKTRACE
            qCritical("SafeList::takeLast list is empty");
        }
        return m_list.takeFirst();
    }

    T takeLast()
    {
        QMutexLocker lock(&m_mutex);
        if(m_list.isEmpty())
        {
            BACKTRACE
            qCritical("SafeList::takeLast list is empty");
        }
        return m_list.takeLast();
    }

    T first()
    {
        QMutexLocker lock(&m_mutex);
        if(m_list.isEmpty())
        {
            BACKTRACE
            qCritical("SafeList::first list is empty");
        }
        return m_list.first();
    }

    const T first() const
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        if(m_list.isEmpty())
        {
            BACKTRACE
            qCritical("SafeList::first list is empty");
        }
        return m_list.first();
    }

    T last()
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        if(m_list.isEmpty())
        {
            BACKTRACE
            qCritical("SafeList::last list is empty");
        }
        return m_list.last();
    }

    const T last() const
    {
        QMutexLocker lock(&m_mutex);
        if(m_list.isEmpty())
        {
            BACKTRACE
            qCritical("SafeList::last list is empty");
        }
        return m_list.last();
    }

    /*
    const QList<T> list() const
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        return m_list;
    }
    */

    /*
    void map(void _f(T))
    {
        QMutexLocker lock(&m_mutex);
        for(T e: m_list)
            _f(e);
    }
    */

    void map(const std::function<void(T&)>& _f, bool _clear = false)
    {
        QMutexLocker lock(&m_mutex);
        for(T& e: m_list)
            _f(e);
        if(_clear)
            m_list.clear();
    }

    void map(const std::function<void(const T&)>& _f) const
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        for(const T& e: m_list)
            _f(e);
    }

    void filter(const std::function<bool(const T&)>& _f)
    {
        QMutexLocker lock(&m_mutex);
        for(int i = m_list.size() - 1; i >= 0; i--)  // const T& e: m_list)
            if(_f(m_list.at(i)))
                m_list.removeAt(i);
    }

  private:
    QMutex     m_mutex;
    bool       m_unicity;
    QVector<T> m_list;
};

#endif
