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
    SafeList(bool _checkUnicity)
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

    bool contains(T _e) const
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        return m_list.contains(_e);
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

    int removeAll(T _e, bool _check = true)
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

    bool removeOne(T _e, bool _check = true)
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

    int size() const
    {
        QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
        return m_list.size();
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

    void map(const std::function<void(T)>& _f, bool _clear = false)
    {
        QMutexLocker lock(&m_mutex);
        for(T e: m_list)
            _f(e);
        if(_clear)
            m_list.clear();
    }

    void filter(const std::function<bool(T)>& _f)
    {
        QMutexLocker lock(&m_mutex);
        for(T e: m_list)
            if(_f(e))
                m_list.remove(e);
    }

  private:
    QMutex     m_mutex;
    bool       m_unicity;
    QVector<T> m_list;
};

#endif
