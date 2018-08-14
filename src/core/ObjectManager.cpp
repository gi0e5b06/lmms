
#include "ObjectManager.h"

#include "Backtrace.h"
//#include "lmms_basics.h"

#define C2ULI (unsigned long int)

/*
template <class T>
bool ObjectManager<T>::init(const int _nbe, const char* _ref)
{
    s_singleton = new ObjectManager<T>(_nbe, _ref);
    return true;
}

template <class T>
void ObjectManager<T>::cleanup()
{
}

template <class T>
bool ObjectManager<T>::safe()
{
    return !s_singleton->full();
}

template <class T>
T* ObjectManager<T>::alloc()
{
    qInfo("ObjectManager::alloc %s", s_singleton->m_ref);
    return s_singleton->allocate();
}

template <class T>
void ObjectManager<T>::free(T* _ptr)
{
    if(!_ptr)
    {
        return;  // Null pointer deallocations are OK but do not need to be
                 // handled
    }

    qInfo("ObjectManager::free %s", s_singleton->m_ref);
    s_singleton->deallocate(_ptr);
}
*/

template <class T>
ObjectManager<T>::ObjectManager(const int _nbe, const char* _ref) :
      m_mutex(), m_nbe(_nbe), m_data(NULL), m_available(_nbe, true),
      m_lastfree(0), m_count(0), m_max(0), m_ref(_ref), //m_typeid(typeid(T)),
      m_size(sizeof(T))
{
    if(_nbe > 32 * 1024)
        qFatal("ObjectManager: too big %d (32768 elements max)", _nbe);
    if(_nbe * m_size > 32 * 1024 * 8192)
        qFatal("ObjectManager: too big %lu (268435456 bytes max)",
               C2ULI(_nbe * m_size));

    m_data = (char*)::calloc(_nbe, m_size);
    // memset(available,0xFF,sizeof(unsigned int)*1024);
}

template <class T>
ObjectManager<T>::~ObjectManager()
{
    qInfo("~ObjectManager %6lu : cnt=%6d : max=%6lu %s %s", C2ULI m_size,
          m_count, C2ULI m_max, (char*)(m_nbe == m_max ? "!!!" : "   "),
          m_ref);
    ::free(m_data);
}

template <class T>
bool ObjectManager<T>::full()
{
    return m_count >= m_nbe;
}

template <class T>
T* ObjectManager<T>::allocate()
{
    int i;
    m_mutex.lock();

    if(m_count >= m_nbe)
    {
        m_mutex.unlock();
        BACKTRACE
        qFatal("object manager full %d in %s", m_count, m_ref);
    }

    // development phase
    if((m_count >= (m_nbe * 90l) / 100) && (m_nbe >= 100)
       && ((m_count % (m_nbe / 100)) == 0))
        qWarning("object manager saturating: %d/%d in %s", m_count, m_nbe,
                 m_ref);

    if((m_lastfree >= 0) && (m_lastfree < m_nbe)
       && m_available.bit(m_lastfree))  // m_available[m_lastfree])
    {
        i = m_lastfree;
        m_available.unset(i);  // m_available[i]=false;
        m_count++;
        if(m_max < m_count)
            m_max++;
        m_lastfree = -1;
    }
    else
    {
        i = m_available.nextSet(0);  // from start
        if((i < 0) || (i >= m_nbe))
            i = m_nbe;
        else
        {
            m_available.unset(i);
            m_count++;
            if(m_max < m_count)
                m_max++;
        }
        /*
          for(i=0;i<m_nbe;i++)
          if(m_available[i])
          {
           m_available[i]=false;
           m_count++;
           if(m_max<m_count) m_max++;
           break;
           }
        */
    }

    if((i + 1 < m_nbe) && m_available.bit(i + 1))  // m_available[i+1])
        m_lastfree = i + 1;

    m_mutex.unlock();

    if(i >= m_nbe)
    {
        qFatal("object manager suprizingly full %d in %s", m_count, m_ref);
    }

    // qWarning("allocate n°%d %d/%d in %ld
    // %s#%ld",i,m_count,m_nbe,m_size,file,line);
    return m_data + i * m_size;
}

template <class T>
bool ObjectManager<T>::deallocate(T* _ptr)
{
    if(!_ptr)
        return true;

    if((_ptr < m_data) || (_ptr >= m_data + m_nbe * m_size))
    {
        // qWarning("free pointer out %p",_ptr);
        // MMA_STD_FREE(_ptr);
        return false;
    }

    size_t s = ((char*)_ptr) - m_data;
    if((s % m_size) != 0)
    {
        BACKTRACE
        qCritical("error: ObjectManager::free: invalid ptr in %s", m_ref);
    }

    const int i = s / m_size;
    m_mutex.lock();
    if((i < 0) || (i >= m_nbe))
    {
        BACKTRACE
        qCritical("error: i out of range: %d in %s", i, m_ref);
    }
    if(m_available.bit(i))  // m_available[i])
    {
        BACKTRACE
        qWarning("error: should be taken n°%d in %s", i, m_ref);
    }
    m_available.set(i);  // m_available[i]=true;
    m_lastfree = i;
    m_count--;
    if(m_count < 0)
    {
        BACKTRACE
        qWarning("error: negative count in %s: ", m_ref);
    }
    m_mutex.unlock();

    // qWarning("deallocate n°%d %d/%d in %lu %s#%ld",i,m_count,m_nbe,C2ULI
    // m_size,file,line);
    return true;
}
