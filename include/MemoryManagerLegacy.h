/*
 * MemoryManagerLegacy.h - A lightweight, generic memory manager for LMMS
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

#ifndef MEMORY_MANAGER_LEGACY_H
#define MEMORY_MANAGER_LEGACY_H

#include <QVector>
#include <QMutex>
#include <QHash>
#include "MemoryHelper.h"
#include "export.h"
#include "MemoryManager.h"

#ifndef MEMORY_MANAGER_CLASS
#define MEMORY_MANAGER_CLASS MemoryManagerLegacy
#endif

class QReadWriteLock;

const int MM_CHUNK_SIZE = 64; // granularity of managed memory
const int MM_INITIAL_CHUNKS = 1024 * 1024; // how many chunks to allocate at startup - TODO: make configurable
const int MM_INCREMENT_CHUNKS = 16 * 1024; // min. amount of chunks to increment at a time

struct MemoryPool
{
	void * m_pool;
	char * m_free;
	int m_chunks;
	QMutex m_mutex;

	MemoryPool() :
		m_pool( NULL ),
		m_free( NULL ),
		m_chunks( 0 )
	{}

	MemoryPool( int chunks ) :
		m_chunks( chunks )
	{
		m_free = (char*) MemoryHelper::alignedMalloc( chunks );
		memset( m_free, 1, chunks );
	}

	MemoryPool( const MemoryPool & mp ) :
		m_pool( mp.m_pool ),
		m_free( mp.m_free ),
		m_chunks( mp.m_chunks ),
		m_mutex()
	{}

	MemoryPool & operator = ( const MemoryPool & mp )
	{
		m_pool = mp.m_pool;
		m_free = mp.m_free;
		m_chunks = mp.m_chunks;
		return *this;
	}

	void * getChunks( int chunksNeeded );
	void releaseChunks( void * ptr, int chunks );
};

struct PtrInfo
{
	int chunks;
	MemoryPool * memPool;
};

typedef QVector<MemoryPool> MemoryPoolVector;
typedef QHash<void*, PtrInfo> PointerInfoMap;

class EXPORT MemoryManagerLegacy
{
 public:
	static bool init();
	static void cleanup();
	static bool safe( size_t size , const char* file , long line );
	static void * alloc( size_t size, const char* file , long line );
	static void free( void * ptr, const char* file , long line );
	static void * alignedAlloc( size_t size , const char* file , long line);
	static void alignedFree( void * ptr , const char* file , long line);
	static void setActive(bool active) {};

 protected:
	static int extend( int chunks ); // returns index of created pool (for use by alloc)

 private:
	static MemoryPoolVector s_memoryPools;
	static QReadWriteLock s_poolMutex;

	static PointerInfoMap s_pointerInfo;
	static QMutex s_pointerMutex;
};

#endif
