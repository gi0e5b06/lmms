/*
 * MemoryManagerStandard.h - malloc/calloc/free
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

#ifndef MEMORY_MANAGER_STANDARD_H
#define MEMORY_MANAGER_STANDARD_H

#include "MemoryManager.h"
#include "export.h"

//#include <QtGlobal>

#include <cstdlib>
#include <cstring>

#ifndef MEMORY_MANAGER_CLASS
#define MEMORY_MANAGER_CLASS MemoryManagerStandard
#endif

class EXPORT MemoryManagerStandard
{
  public:
    static bool  init();
    static void  cleanup();
    static bool  safe(size_t size, const char* file, long line);
    static void* alloc(size_t size, const char* file, long line);
    static void  free(void* ptr, const char* file, long line);
    static void* alignedAlloc(size_t size, const char* file, long line);
    static void  alignedFree(void* ptr, const char* file, long line);
    static void  setActive(bool active){};

    // static int extend( int chunks );
    // returns index of created pool (for use by alloc)
};

#endif
