#ifndef SAFE_LIST_H
#define SAFE_LIST_H

#include <QList>
#include <QMutex>

template <class T>
class SafeList
{

 public:
        void append(T _e)
        {
                QMutexLocker lock(&m_mutex);
                m_list.append(_e);
        }
        void clear()
        {
                QMutexLocker lock(&m_mutex);
                m_list.clear();
        }
        bool isEmpty() const
        {
                QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
                return m_list.isEmpty();
        }
        void prepend(T _e)
        {
                QMutexLocker lock(&m_mutex);
                m_list.append(_e);
        }
        void removeAll(T _e)
        {
                QMutexLocker lock(&m_mutex);
                m_list.removeAll(_e);
        }
        void removeOne(T _e)
        {
                QMutexLocker lock(&m_mutex);
                m_list.removeOne(_e);
        }
        int size() const
        {
                QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
                return m_list.size();
        }
        const QList<T> list() const
        {
                QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
                return m_list;
        }

 private:
        QMutex m_mutex;
        QList<T> m_list;
};

#endif
