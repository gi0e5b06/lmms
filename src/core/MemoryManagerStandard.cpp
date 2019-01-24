/*
 * MemoryManagerStandard.cpp -
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

#include "MemoryManagerStandard.h"

#include "MemoryHelper.h"

bool MemoryManagerStandard::init()
{
    return true;
}

void MemoryManagerStandard::cleanup()
{
}

bool MemoryManagerStandard::safe(size_t size, const char* file, long line)
{
    return true;
}

void* MemoryManagerStandard::alloc(size_t size, const char* file, long line)
{
    return ::malloc(size);  //::calloc(1,size);
}

void MemoryManagerStandard::free(void* ptr, const char* file, long line)
{
    if(!ptr)
    {
        return;  // Null pointer deallocations are OK but do not need to be
                 // handled
    }

    ::free(ptr);
}

void* MemoryManagerStandard::alignedAlloc(size_t      size,
                                          const char* file,
                                          long        line)
{
    return MemoryHelper::alignedMalloc(size);
}

void MemoryManagerStandard::alignedFree(void*       ptr,
                                        const char* file,
                                        long        line)
{
    MemoryHelper::alignedFree(ptr);
}
