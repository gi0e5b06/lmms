
#include "MemoryManagerStandard.h"
#include "MemoryHelper.h"

bool MemoryManagerStandard::init()
{
	return true;
}

void MemoryManagerStandard::cleanup()
{
}

bool MemoryManagerStandard::safe( size_t size , const char* file , long line)
{
	return true;
}

void * MemoryManagerStandard::alloc( size_t size , const char* file , long line)
{
	return ::malloc(size);//::calloc(1,size);
}

void MemoryManagerStandard::free( void * ptr , const char* file , long line)
{
	if( !ptr )
	{
		return; // Null pointer deallocations are OK but do not need to be handled
	}

	::free(ptr);
}

void * MemoryManagerStandard::alignedAlloc( size_t size , const char* file , long line)
{
	return MemoryHelper::alignedMalloc(size);
}

void MemoryManagerStandard::alignedFree( void * ptr , const char* file , long line)
{
	MemoryHelper::alignedFree(ptr);
}
