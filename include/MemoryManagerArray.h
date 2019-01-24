/*
 * MemoryManagerArray.h -
 *
 * Copyright (c) 2017-2019 gi0e5b06 (on github.com)
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

#ifndef MEMORY_MANAGER_ARRAY_H
#define MEMORY_MANAGER_ARRAY_H

//#include <stdlib.h>
//#include <string.h>

#include "Bitset.h"
#include "MemoryManager.h"
#include "export.h"

#include <QHash>
#include <QMutex>

#ifndef MEMORY_MANAGER_CLASS
#define MEMORY_MANAGER_CLASS MemoryManagerArray
#endif

class EXPORT MemoryManagerArray
{
  public:
    MemoryManagerArray(const int    nbe,
                       const size_t size,
                       const char*  ref = "");
    ~MemoryManagerArray();

    // bool  full();
    void* allocate(size_t size, const char* file, long line);
    bool  deallocate(void* ptr, const char* file, long line);

    static bool  init();
    static void  cleanup();
    static bool  safe(size_t size, const char* file, long line);
    static void* alloc(size_t size, const char* file, long line);
    static void  free(void* ptr, const char* file, long line);
    static void* alignedAlloc(size_t size, const char* file, long line);
    static void  alignedFree(void* ptr, const char* file, long line);
    static void  setActive(bool active);

  private:
    QMutex       m_mutex;
    const int    m_nbe;
    const size_t m_size;
    char*        m_data;

    int m_lastfree;
    int m_count;

    // info
    int                    m_max;
    unsigned long long int m_wasted;
    const char*            m_ref;

    // unsigned int available[1024]; // nbe<1024*32
    // bool*   m_available;
    Bitset m_available;

    QHash<size_t, long> m_stats;

    static bool s_active;

    /*
      static MemoryManagerArray S4, S8, S16, S32, S80, S112, S128, S192, S224,
      S264, S480, S496, S512, S552, S1024, S1056, S1392, S2048, S2464,
      S4128;

    bool bit(const unsigned int i) const;
    void set(const unsigned int i);
    void unset(const unsigned int i);
    int  nextSet(const unsigned int i) const;
    int  nextUnset(const unsigned int i) const;
    */
};

#endif
