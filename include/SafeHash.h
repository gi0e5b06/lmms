#ifndef SAFE_HASH_H
#define SAFE_HASH_H

#include <QHash>
#include <QMutex>

template <class T,class U>
class SafeHash
{

 public:
        void clear()
        {
                QMutexLocker lock(&m_mutex);
                m_hash.clear();
        }
        bool contains(T _k) const
        {
                QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
                return m_hash.contains(_k);
        }
        bool isEmpty() const
        {
                QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
                return m_hash.isEmpty();
        }
        void insert(T _k,U _v)
        {
                QMutexLocker lock(&m_mutex);
                m_hash.insert(_k,_v);
        }
        /*
        void removeAll(T _e)
        {
                QMutexLocker lock(&m_mutex);
                m_hash.removeAll(_e);
        }
        void removeOne(T _e)
        {
                QMutexLocker lock(&m_mutex);
                m_hash.removeOne(_e);
        }
        */
        int size() const
        {
                QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
                return m_hash.size();
        }
        const QHash<T,U> hash() const
        {
                QMutexLocker lock(const_cast<QMutex*>(&m_mutex));
                return m_hash;
        }

 private:
        QMutex m_mutex;
        QHash<T,U> m_hash;
};

#endif
