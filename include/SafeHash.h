/*
 * SafeHash.h -
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
