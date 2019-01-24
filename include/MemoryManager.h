/*
 * MemoryManager.h -
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

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "MemoryManagerArray.h"
//#include "MemoryManagerLegacy.h"
//#include "MemoryManagerStandard.h"

#define MM_INIT MEMORY_MANAGER_CLASS::init();
#define MM_CLEANUP MEMORY_MANAGER_CLASS::cleanup();
#define MM_ACTIVE(b) MEMORY_MANAGER_CLASS::setActive(b);

#define MM_OPERATORS                                                  \
  public:                                                             \
    static void* operator new(size_t size)                            \
    {                                                                 \
        return MEMORY_MANAGER_CLASS::alloc(size, __FILE__, __LINE__); \
    }                                                                 \
    static void* operator new[](size_t size)                          \
    {                                                                 \
        return MEMORY_MANAGER_CLASS::alloc(size, __FILE__, __LINE__); \
    }                                                                 \
    static void operator delete(void* ptr)                            \
    {                                                                 \
        MEMORY_MANAGER_CLASS::free(ptr, __FILE__, __LINE__);          \
    }                                                                 \
    static void operator delete[](void* ptr)                          \
    {                                                                 \
        MEMORY_MANAGER_CLASS::free(ptr, __FILE__, __LINE__);          \
    }

// for use in cases where overriding new/delete isn't a possibility
#define MM_ALLOC(type, count)                                          \
    (type*)MEMORY_MANAGER_CLASS::alloc(sizeof(type) * count, __FILE__, \
                                       __LINE__)
// and just for symmetry...
#define MM_FREE(ptr) MEMORY_MANAGER_CLASS::free(ptr, __FILE__, __LINE__)
// checking for room
#define MM_SAFE(type, count) \
    MEMORY_MANAGER_CLASS::safe(sizeof(type) * count, __FILE__, __LINE__)
// for use in cases where overriding new/delete isn't a possibility
#define MM_ALIGNED_ALLOC(type, count)                               \
    (type*)MEMORY_MANAGER_CLASS::alignedAlloc(sizeof(type) * count, \
                                              __FILE__, __LINE__)
// and just for symmetry...
#define MM_ALIGNED_FREE(ptr) \
    MEMORY_MANAGER_CLASS::alignedFree(ptr, __FILE__, __LINE__)

// for debugging purposes

#define MM_OPERATORS_DEBUG                                             \
  public:                                                              \
    static void* operator new(size_t size)                             \
    {                                                                  \
        qDebug("MM_OPERATORS_DEBUG: new called for %d bytes", size);   \
        return MEMORY_MANAGER_CLASS::alloc(size, __FILE__, __LINE__);  \
    }                                                                  \
    static void* operator new[](size_t size)                           \
    {                                                                  \
        qDebug("MM_OPERATORS_DEBUG: new[] called for %d bytes", size); \
        return MEMORY_MANAGER_CLASS::alloc(size, __FILE__, __LINE__);  \
    }                                                                  \
    static void operator delete(void* ptr)                             \
    {                                                                  \
        qDebug("MM_OPERATORS_DEBUG: delete called for %p", ptr);       \
        MEMORY_MANAGER_CLASS::free(ptr, __FILE__, __LINE__);           \
    }                                                                  \
    static void operator delete[](void* ptr)                           \
    {                                                                  \
        qDebug("MM_OPERATORS_DEBUG: delete[] called for %p", ptr);     \
        MEMORY_MANAGER_CLASS::free(ptr, __FILE__, __LINE__);           \
    }

#endif
