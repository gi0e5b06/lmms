
#include "MemoryManagerArray.h"

#include "Backtrace.h"
#include "lmms_basics.h"  // REQUIRED

bool MemoryManagerArray::s_active = false;

/*
MemoryManagerArray MemoryManagerArray::S4   (  512,   4);
MemoryManagerArray MemoryManagerArray::S8   (   64,   8);
MemoryManagerArray MemoryManagerArray::S16  (  256,  16);
MemoryManagerArray MemoryManagerArray::S32  (  512,
32,"SampleBuffer::handleState"); MemoryManagerArray MemoryManagerArray::S80  (
4096,  80,"NotePlayHandle"); MemoryManagerArray MemoryManagerArray::S112 (
128, 112); MemoryManagerArray MemoryManagerArray::S128 (   64, 128);
MemoryManagerArray MemoryManagerArray::S192 (  256, 192);
//200 drumsynth
MemoryManagerArray MemoryManagerArray::S224 ( 4096, 224,"DetuningHelper");
MemoryManagerArray MemoryManagerArray::S264 ( 2048, 264,"NotePlayHandle");
MemoryManagerArray MemoryManagerArray::S480 ( 1024, 480,"BasicFilters");
MemoryManagerArray MemoryManagerArray::S496 (   64, 496);
MemoryManagerArray MemoryManagerArray::S512 ( 2048, 512,"tmp");
MemoryManagerArray MemoryManagerArray::S552 ( 4096, 552);
//968
MemoryManagerArray MemoryManagerArray::S1024(  256,1024);
MemoryManagerArray MemoryManagerArray::S1056(  768,1056);
MemoryManagerArray MemoryManagerArray::S1392(  128,1392);
MemoryManagerArray MemoryManagerArray::S2048(  256,2048,"Buffer");
MemoryManagerArray MemoryManagerArray::S2464(   64,2464);
//2760
//4096
MemoryManagerArray MemoryManagerArray::S4128( 256,4128);
*/

int                 L2[32768];
int                 P2[16];
int                 ASZ[16] = {128, 128, 128, 512, 512, 512, 512, 512,
               512, 1024, 512, 512, 512, 512, 512, 128};
MemoryManagerArray* MMA[16];

//#define MMA_STD_ALLOC(size) ::malloc(size)
#define MMA_STD_ALLOC(size) ::calloc(1, size)
#define MMA_STD_FREE(ptr) ::free(ptr)

#define C2ULI (unsigned long int)
bool MemoryManagerArray::init()
{
    s_active = false;
    for(int i = 0; i < 16; i++)
    {
        P2[i]  = 1 << i;
        MMA[i] = new MemoryManagerArray(ASZ[i], P2[i]);
    }
    int i = 0;
    int s = 1;
    for(int j = 0; j < 32768; j++)
    {
        if(j > s)
        {
            s *= 2;
            i++;
        }
        L2[j] = i;  // printf("i=%d j=%d s=%d\n",i,j,s);
    }
    return true;
}

void MemoryManagerArray::cleanup()
{
    s_active = false;
    for(int i = 0; i < 16; i++)
    {
        MemoryManagerArray* old = MMA[i];

        MMA[i] = nullptr;
        delete old;
    }
}

bool MemoryManagerArray::safe(size_t size, const char* file, long line)
{
    // qWarning("MemoryManagerArray::safe %lu %s:%ld",C2ULI size,file,line);

    /*
    if(s_active)
    {
            if(size<=   4) return !   S4.full();
            if(size<=   8) return !   S8.full();
            if(size<=  16) return !  S16.full();
            if(size<=  32) return !  S32.full();
            if(size<=  80) return !  S80.full();
            if(size<= 112) return ! S112.full();
            if(size<= 128) return ! S128.full();
            if(size<= 192) return ! S192.full();
            if(size<= 224) return ! S224.full();
            if(size<= 264) return ! S264.full();
            if(size<= 480) return ! S480.full();
            if(size<= 496) return ! S496.full();
            if(size<= 512) return ! S512.full();
            if(size<= 552) return ! S552.full();
            if(size<=1024) return !S1024.full();
            if(size<=1056) return !S1056.full();
            if(size<=1392) return !S1392.full();
            if(size<=2048) return !S2048.full();
            if(size<=2464) return !S2464.full();
            if(size<=4128) return !S4128.full();
    }
    */

    return true;
}

void* MemoryManagerArray::alloc(size_t size, const char* file, long line)
{
    // qWarning("MemoryManagerArray::alloc %lu",C2ULI size);

    if(s_active && size < 32768)
    {
        int i = L2[size];
        return MMA[i]->allocate(size, file, line);
        /*
        if(size<=   4) return    S4.allocate(size,file,line);
        if(size<=   8) return    S8.allocate(size,file,line);
        if(size<=  16) return   S16.allocate(size,file,line);
        if(size<=  32) return   S32.allocate(size,file,line);
        if(size<=  80) return   S80.allocate(size,file,line);
        if(size<= 112) return  S112.allocate(size,file,line);
        if(size<= 128) return  S128.allocate(size,file,line);
        if(size<= 192) return  S192.allocate(size,file,line);
        if(size<= 224) return  S224.allocate(size,file,line);
        if(size<= 264) return  S264.allocate(size,file,line);
        if(size<= 480) return  S480.allocate(size,file,line);
        if(size<= 496) return  S496.allocate(size,file,line);
        if(size<= 512) return  S512.allocate(size,file,line);
        if(size<= 552) return  S552.allocate(size,file,line);
        if(size<=1024) return S1024.allocate(size,file,line);
        if(size<=1056) return S1056.allocate(size,file,line);
        if(size<=1392) return S1392.allocate(size,file,line);
        if(size<=2048) return S2048.allocate(size,file,line);
        if(size<=2464) return S2464.allocate(size,file,line);
        if(size<=4128) return S4128.allocate(size,file,line);
        */
    }

    void* r = MMA_STD_ALLOC(size);
    // if(s_active) qWarning("std malloc %ld %p %s#%ld",size,r,file,line);
    return r;
}

void MemoryManagerArray::free(void* ptr, const char* file, long line)
{
    if(!ptr)
    {
        return;  // Null pointer deallocations are OK but do not need to be
                 // handled
    }

    for(int i = 0; i < 16; i++)
        if(MMA[i] != nullptr && MMA[i]->deallocate(ptr, file, line))
            return;

    /*
    if(!   S4.deallocate(ptr,file,line))
    if(!   S8.deallocate(ptr,file,line))
    if(!  S16.deallocate(ptr,file,line))
    if(!  S32.deallocate(ptr,file,line))
    if(!  S80.deallocate(ptr,file,line))
    if(! S112.deallocate(ptr,file,line))
    if(! S128.deallocate(ptr,file,line))
    if(! S192.deallocate(ptr,file,line))
    if(! S224.deallocate(ptr,file,line))
    if(! S264.deallocate(ptr,file,line))
    if(! S480.deallocate(ptr,file,line))
    if(! S496.deallocate(ptr,file,line))
    if(! S512.deallocate(ptr,file,line))
    if(! S552.deallocate(ptr,file,line))
    if(!S1024.deallocate(ptr,file,line))
    if(!S1056.deallocate(ptr,file,line))
    if(!S1392.deallocate(ptr,file,line))
    if(!S2048.deallocate(ptr,file,line))
    if(!S2464.deallocate(ptr,file,line))
    if(!S4128.deallocate(ptr,file,line))
    */
    {
        // if(s_active) qWarning("std free %p in %s#%ld",ptr,file,line);
        MMA_STD_FREE(ptr);
    }
}

void* MemoryManagerArray::alignedAlloc(size_t      size,
                                       const char* file,
                                       long        line)
{
    char *    ptr, *ptr2, *aligned_ptr;
    const int align_mask = MM_ALIGN_SIZE - 1;

    // ptr = static_cast<char*>( malloc( size + MM_ALIGN_SIZE + sizeof( int )
    // ) );
    ptr = static_cast<char*>(MemoryManagerArray::alloc(
            size + MM_ALIGN_SIZE + sizeof(int), file, line));

    if(ptr == NULL)
        return NULL;

    ptr2        = ptr + sizeof(int);
    aligned_ptr = ptr2 + (MM_ALIGN_SIZE - ((size_t)ptr2 & align_mask));

    ptr2          = aligned_ptr - sizeof(int);
    *((int*)ptr2) = (int)(aligned_ptr - ptr);

    return aligned_ptr;
}

void MemoryManagerArray::alignedFree(void* ptr, const char* file, long line)
{
    if(ptr)
    {
        int* ptr2 = static_cast<int*>(ptr) - 1;
        ptr       = static_cast<char*>(ptr) - *ptr2;
        MemoryManagerArray::free(ptr, file, line);
    }
}

void MemoryManagerArray::setActive(bool active)
{
    s_active = active;
}

MemoryManagerArray::MemoryManagerArray(const int    nbe,
                                       const size_t size,
                                       const char*  ref) :
      m_mutex(QMutex::Recursive),
      m_nbe(nbe), m_size(size), m_data(NULL), m_lastfree(0), m_count(0),
      m_max(0), m_wasted(0), m_ref(ref), m_available(nbe, true)
{
    if(nbe > 32 * 1024)
        qFatal("MemoryManagerArray: too big %d (32768 elements max)", nbe);
    if(nbe * size > 32 * 1024 * 8192)
        qFatal("MemoryManagerArray: too big %lu (268435456 bytes max)",
               C2ULI(nbe * size));

    m_data = (char*)::calloc(nbe, size);
    // memset(available,0xFF,sizeof(unsigned int)*1024);
}

MemoryManagerArray::~MemoryManagerArray()
{
    qWarning(
            "~MemoryManagerArray %6lu : cnt=%6d : max=%6lu %s wasted=%6lu %s",
            C2ULI m_size, m_count, C2ULI m_max,
            (char*)(m_nbe == m_max ? "!!!" : "   "), C2ULI m_wasted, m_ref);
    ::free(m_data);
    //::free(m_available);

    QHashIterator<size_t, long> i(m_stats);
    while(i.hasNext())
    {
        i.next();
        qWarning("                    %6lu : bytes=%6lu cnt=%6ld",
                 C2ULI m_size, C2ULI i.key(), i.value());
    }
}

bool MemoryManagerArray::full()
{
    return m_count >= m_nbe;
}

void* MemoryManagerArray::allocate(size_t size, const char* file, long line)
{
    if(size > m_size)  //!=
    {
        void* r = MMA_STD_ALLOC(size);
        qWarning("invalid size %lu %p in %lu: %s#%ld", C2ULI size, r,
                 C2ULI m_size, file, line);
        return r;
    }

    int i;
    m_mutex.lock();

    if(m_count >= m_nbe)
    {
        m_mutex.unlock();
        void* r = MMA_STD_ALLOC(size);
        BACKTRACE
        qWarning("block %lu full %d (asking %lu bytes): %s#%ld", C2ULI m_size,
                 m_count, C2ULI size, file, line);
        return r;
    }

    // development phase
    if(size < m_size)
    {
        m_wasted += (m_size - size);
        m_stats[size] += 1;
        // qWarning("MemoryManagerArray::sup-allocate %lu %lu",C2ULI
        // size,C2ULI m_size);
    }

    // development phase
    if((m_count >= (m_nbe * 90l) / 100) && (m_nbe >= 100)
       && ((m_count % (m_nbe / 100)) == 0))
        qWarning("block %lu saturating %d (asking %lu bytes): %s#%ld",
                 C2ULI m_size, m_count, C2ULI size, file, line);

    if((m_lastfree >= 0) && (m_lastfree < m_nbe)
       && m_available.bit(m_lastfree))  // m_available[m_lastfree])
    {
        i = m_lastfree;
        m_available.unset(i);  // m_available[i]=false;
        m_count++;
        if(m_max < m_count)
            m_max++;
        m_lastfree = -1;
    }
    else
    {
        i = m_available.nextSet(0);  // from start
        if((i < 0) || (i >= m_nbe))
            i = m_nbe;
        else
        {
            m_available.unset(i);
            m_count++;
            if(m_max < m_count)
                m_max++;
        }
        /*
          for(i=0;i<m_nbe;i++)
          if(m_available[i])
          {
           m_available[i]=false;
           m_count++;
           if(m_max<m_count) m_max++;
           break;
           }
        */
    }

    if((i + 1 < m_nbe) && m_available.bit(i + 1))  // m_available[i+1])
        m_lastfree = i + 1;

    m_mutex.unlock();

    if(i >= m_nbe)
    {
        qWarning("block %lu suprizingly full %d %s#%ld", C2ULI m_size,
                 m_count, file, line);
        return MMA_STD_ALLOC(size);
    }

    // qWarning("allocate n°%d %d/%d in %ld
    // %s#%ld",i,m_count,m_nbe,m_size,file,line);
    return m_data + i * m_size;
}

bool MemoryManagerArray::deallocate(void* ptr, const char* file, long line)
{
    if((ptr < m_data) || (ptr >= m_data + m_nbe * m_size))
    {
        // qWarning("free pointer out %p",ptr);
        // MMA_STD_FREE(ptr);
        return false;
    }

    size_t s = ((char*)ptr) - m_data;
    if((s % m_size) != 0)
    {
        BACKTRACE
        qCritical("error: MemoryManagerArray::free: invalid ptr %s#%ld", file,
                  line);
    }

    const int i = s / m_size;
    m_mutex.lock();
    if((i < 0) || (i >= m_nbe))
    {
        BACKTRACE
        qCritical("error: i out of range: %d in %lu: %s#%ld", i, C2ULI m_size,
                  file, line);
    }
    if(m_available.bit(i))  // m_available[i])
    {
        BACKTRACE
        qWarning("error: should be taken n°%d in %lu: %s#%ld", i,
                 C2ULI m_size, file, line);
    }
    m_available.set(i);  // m_available[i]=true;
    m_lastfree = i;
    m_count--;
    if(m_count < 0)
    {
        BACKTRACE
        qWarning("error: negative count in %lu: %s#%ld", C2ULI m_size, file,
                 line);
    }
    m_mutex.unlock();

    // qWarning("deallocate n°%d %d/%d in %lu %s#%ld",i,m_count,m_nbe,C2ULI
    // m_size,file,line);
    return true;
}
