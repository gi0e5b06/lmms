/*
 * MemoryManagerStandard.h - malloc/calloc/free
 *
 * Copyright (c) 2017
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#ifndef MEMORY_MANAGER_STANDARD_H
#define MEMORY_MANAGER_STANDARD_H

#include <stdlib.h>
#include "export.h"

class EXPORT MemoryManagerStandard
{
 public:
	static bool  safe(size_t size , const char* file , long line);
	static void* alloc(size_t size , const char* file , long line);
	static void  free(void* ptr , const char* file , long line);
	static bool  init();
	static void  cleanup();
	//static int extend( int chunks ); // returns index of created pool (for use by alloc)
};

#endif
