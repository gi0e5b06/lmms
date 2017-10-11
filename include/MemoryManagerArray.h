
#ifndef MEMORY_MANAGER_ARRAY_H
#define MEMORY_MANAGER_ARRAY_H

#include <stdlib.h>
#include <QtCore/QMutex>

class MemoryManagerArray
{
public:
	MemoryManagerArray(int nbe,size_t size);
	~MemoryManagerArray();

	bool full();
	void * allocate( size_t size , const char* file , long line);
	bool deallocate( void * ptr , const char* file , long line);

	static bool active;
	static MemoryManagerArray S4,S8,S16,S32,S80,S112,S128,S192,S224,
		S256,S480,S496,S512,S552,S1024,S2048,S2464,S4128;

	static bool init();
	static void cleanup();
	static bool safe(size_t size , const char* file , long line);
	static void * alloc(size_t size , const char* file , long line);
	static void free(void * ptr , const char* file , long line);

private:
	QMutex  m_mutex;
	int     m_nbe;
	size_t  m_size;
	char*   m_data;
	bool*   m_available;

	int     m_lastfree;
	int     m_count;
	int     m_max;
};

#endif
