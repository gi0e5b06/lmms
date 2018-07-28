
#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include <stdlib.h>
#include <string.h>
#include <typeinfo>

//#include <QHash>
#include <QMutex>

#include "Bitset.h"
#include "ObjectManager.h"
#include "export.h"

template <class T>
class EXPORT ObjectManager
{
  public:
    // static T* acquire(...);
    // static void release(T* _ptr);

  protected:
    /*
static bool init(const int _nbe, const char* _ref = "");
static void cleanup();
static bool safe();
static T*   alloc();
static void free(T* _ptr);
    */

    ObjectManager(const int _nbe, const char* _ref);
    ~ObjectManager();

    bool full();
    T*   allocate();
    bool deallocate(T* _ptr);

  private:
    //static ObjectManager<T>* s_singleton;

    QMutex       m_mutex;
    const int    m_nbe;
    const size_t m_size;
    char*        m_data;
    Bitset       m_available;

    int m_lastfree;
    int m_count;

    // info
    int                   m_max;
    const char*           m_ref;
    //const std::type_info& m_typeid;
};

#endif
