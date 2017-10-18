/*
 * MemoryManager.h - A lightweight, generic memory manager for LMMS
 *
 * Copyright (c) 2014 Vesa Kivimäki
 * Copyright (c) 2007-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
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

#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

//#include <QtCore/QVector>
//#include <QtCore/QMutex>
//#include <QtCore/QHash>
//#include "MemoryHelper.h"
#include <string.h>
#include "export.h"

#define MEMORY_MANAGER_CLASS MemoryManagerArray
#include "MemoryManagerArray.h"

#define MM_INIT MEMORY_MANAGER_CLASS::init();
#define MM_CLEANUP MEMORY_MANAGER_CLASS::cleanup();

#define MM_OPERATORS \
public: 	 \
static void * operator new ( size_t size ) \
{			 \
	return MEMORY_MANAGER_CLASS::alloc( size , __FILE__ , __LINE__ );	\
}			 \
static void * operator new[] ( size_t size ) \
{			 \
	return MEMORY_MANAGER_CLASS::alloc( size , __FILE__ , __LINE__ ); \
}			 \
static void operator delete ( void * ptr ) \
{			 \
	MEMORY_MANAGER_CLASS::free( ptr , __FILE__ , __LINE__ ); \
}			 \
static void operator delete[] ( void * ptr ) \
{			 \
	MEMORY_MANAGER_CLASS::free( ptr , __FILE__ , __LINE__ ); \
}

// for use in cases where overriding new/delete isn't a possibility
#define MM_ALLOC( type, count ) (type*) MEMORY_MANAGER_CLASS::alloc( sizeof( type ) * count , __FILE__ , __LINE__ )
// and just for symmetry...
#define MM_FREE( ptr ) MEMORY_MANAGER_CLASS::free( ptr , __FILE__ , __LINE__ )
// checking for room
#define MM_SAFE( type, count ) MEMORY_MANAGER_CLASS::safe( sizeof( type ) * count , __FILE__ , __LINE__ )
// for use in cases where overriding new/delete isn't a possibility
#define MM_ALIGNED_ALLOC( type, count ) (type*) MEMORY_MANAGER_CLASS::alignedAlloc( sizeof( type ) * count , __FILE__ , __LINE__ )
// and just for symmetry...
#define MM_ALIGNED_FREE( ptr ) MEMORY_MANAGER_CLASS::alignedFree( ptr , __FILE__ , __LINE__ )



// for debugging purposes

#define MM_OPERATORS_DEBUG		 \
public: 						 \
static void * operator new ( size_t size ) \
{								 \
	qDebug( "MM_OPERATORS_DEBUG: new called for %d bytes", size ); \
	return MEMORY_MANAGER_CLASS::alloc( size , __FILE__ , __LINE__ ); \
}								 \
static void * operator new[] ( size_t size ) \
{								 \
	qDebug( "MM_OPERATORS_DEBUG: new[] called for %d bytes", size ); \
	return MEMORY_MANAGER_CLASS::alloc( size , __FILE__ , __LINE__ ); \
}								 \
static void operator delete ( void * ptr ) \
{								 \
	qDebug( "MM_OPERATORS_DEBUG: delete called for %p", ptr ); \
	MEMORY_MANAGER_CLASS::free( ptr , __FILE__ , __LINE__ ); \
}								 \
static void operator delete[] ( void * ptr ) \
{								 \
	qDebug( "MM_OPERATORS_DEBUG: delete[] called for %p", ptr ); \
	MEMORY_MANAGER_CLASS::free( ptr , __FILE__ , __LINE__ ); \
}


#endif
