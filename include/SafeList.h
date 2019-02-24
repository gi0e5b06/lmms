#ifndef SAFE_LIST_H
#define SAFE_LIST_H

#include <QList>
#include <QMutex>
//#include <functional>

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
    /*
    void prepend(T _e)
    {
        QMutexLocker lock(&m_mutex);
        m_list.append(_e);
    }
    */
    int removeAll(T _e)
    {
        QMutexLocker lock(&m_mutex);
        return m_list.removeAll(_e);
    }
    bool removeOne(T _e)
    {
        QMutexLocker lock(&m_mutex);
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
    void map(const std::function<void(T)>& _f)
    {
        QMutexLocker lock(&m_mutex);
        for(T e: m_list)
            _f(e);
    }

  private:
    QMutex     m_mutex;
    QVector<T> m_list;
};

#endif
