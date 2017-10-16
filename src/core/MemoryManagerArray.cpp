
#include "MemoryManagerArray.h"

bool MemoryManagerArray::active=false;

MemoryManagerArray MemoryManagerArray::S4   (  512,   4);
MemoryManagerArray MemoryManagerArray::S8   (   64,   8);
MemoryManagerArray MemoryManagerArray::S16  (  256,  16);
MemoryManagerArray MemoryManagerArray::S32  (  512,  32,"SampleBuffer::handleState");
MemoryManagerArray MemoryManagerArray::S80  ( 2048,  80,"NotePlayHandle");
MemoryManagerArray MemoryManagerArray::S112 (  128, 112);
MemoryManagerArray MemoryManagerArray::S128 (   64, 128);
MemoryManagerArray MemoryManagerArray::S192 (  256, 192);
MemoryManagerArray MemoryManagerArray::S224 (  256, 224);
MemoryManagerArray MemoryManagerArray::S256 (   64, 256);
MemoryManagerArray MemoryManagerArray::S480 (  256, 480);
MemoryManagerArray MemoryManagerArray::S496 (   64, 496);
MemoryManagerArray MemoryManagerArray::S512 (   64, 512);
MemoryManagerArray MemoryManagerArray::S552 (  512, 552);
MemoryManagerArray MemoryManagerArray::S1024(   64,1024);
//1056
MemoryManagerArray MemoryManagerArray::S2048(  512,2048,"Buffer");
MemoryManagerArray MemoryManagerArray::S2464(   64,2464);
//2760
//4096
MemoryManagerArray MemoryManagerArray::S4128( 256,4128);

#define MMA_STD_ALLOC(size) ::calloc(1,size)
#define MMA_STD_FREE(ptr) ::free(ptr)

bool MemoryManagerArray::init()
{
	return true;
}

void MemoryManagerArray::cleanup()
{
}

bool MemoryManagerArray::safe( size_t size , const char* file , long line)
{
	qWarning("MemoryManagerArray::safe %ld %s:%ld",size,file,line);

	if(active)
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
		if(size<= 256) return ! S256.full();
		if(size<= 480) return ! S480.full();
		if(size<= 496) return ! S496.full();
		if(size<= 512) return ! S512.full();
		if(size<= 552) return ! S552.full();
		if(size<=1024) return !S1024.full();
		if(size<=2048) return !S2048.full();
		if(size<=2464) return !S2464.full();
		if(size<=4128) return !S4128.full();
	}

	return true;
}

void * MemoryManagerArray::alloc( size_t size , const char* file , long line)
{
	//qWarning("MemoryManagerArray::alloc %ld",size);

	if(active)
	{
		if(size<=   4) return    S4.allocate(size,file,line);
		if(size<=   8) return    S8.allocate(size,file,line);
		if(size<=  16) return   S16.allocate(size,file,line);
		if(size<=  32) return   S32.allocate(size,file,line);
		if(size<=  80) return   S80.allocate(size,file,line);
		if(size<= 112) return  S112.allocate(size,file,line);
		if(size<= 128) return  S128.allocate(size,file,line);
		if(size<= 192) return  S192.allocate(size,file,line);
		if(size<= 224) return  S224.allocate(size,file,line);
		if(size<= 256) return  S256.allocate(size,file,line);
		if(size<= 480) return  S480.allocate(size,file,line);
		if(size<= 496) return  S496.allocate(size,file,line);
		if(size<= 512) return  S512.allocate(size,file,line);
		if(size<= 552) return  S552.allocate(size,file,line);
		if(size<=1024) return S1024.allocate(size,file,line);
		if(size<=2048) return S2048.allocate(size,file,line);
		if(size<=2464) return S2464.allocate(size,file,line);
		if(size<=4128) return S4128.allocate(size,file,line);
	}

	void* r=MMA_STD_ALLOC(size);
	//if(active) qWarning("std malloc %ld %p %s#%ld",size,r,file,line);
	return r;
}

void MemoryManagerArray::free( void * ptr , const char* file , long line)
{
	if( !ptr )
	{
		return; // Null pointer deallocations are OK but do not need to be handled
	}

	if(!   S4.deallocate(ptr,file,line))
        if(!   S8.deallocate(ptr,file,line))
	if(!  S16.deallocate(ptr,file,line))
	if(!  S32.deallocate(ptr,file,line))
	if(!  S80.deallocate(ptr,file,line))
	if(! S112.deallocate(ptr,file,line))
	if(! S128.deallocate(ptr,file,line))
	if(! S192.deallocate(ptr,file,line))
	if(! S224.deallocate(ptr,file,line))
	if(! S256.deallocate(ptr,file,line))
	if(! S480.deallocate(ptr,file,line))
	if(! S496.deallocate(ptr,file,line))
	if(! S512.deallocate(ptr,file,line))
	if(! S552.deallocate(ptr,file,line))
	if(!S1024.deallocate(ptr,file,line))
	if(!S2048.deallocate(ptr,file,line))
	if(!S2464.deallocate(ptr,file,line))
	if(!S4128.deallocate(ptr,file,line))
		{
			//if(active) qWarning("std free %p in %s#%ld",ptr,file,line);
			MMA_STD_FREE(ptr);
		}
}

MemoryManagerArray::MemoryManagerArray(const int nbe, const size_t size , const char* ref) :
	m_mutex(),
        m_nbe(nbe),
	m_size(size),
	m_data(NULL),
	m_lastfree(0),
	m_count(0),
	m_max(0),
	m_wasted(0),
	m_ref(ref),
	m_available(nbe,true)
{
	if(nbe>32*1024)           qFatal("MemoryManagerArray: too big %d (32768 elements max)",nbe);
	if(nbe*size>32*1024*8192) qFatal("MemoryManagerArray: too big %ld (268435456 bytes max)",nbe*size);

	m_data     =(char*)::calloc(nbe,size);
	//memset(available,0xFF,sizeof(unsigned int)*1024);
}

MemoryManagerArray::~MemoryManagerArray()
{
	qWarning("~MemoryManagerArray %6ld : cnt=%6d : max=%6d %s wasted=%llu %s",
		 m_size,m_count,m_max,(m_nbe==m_max ? "!!!" : "   "),m_wasted,m_ref);
	::free(m_data);
	//::free(m_available);

	QHashIterator<size_t,long> i(m_stats);
	while (i.hasNext())
	{
		i.next();
		qWarning("                    %6ld : bytes=%6ld cnt=%6ld",m_size,i.key(),i.value());
	}
}

bool MemoryManagerArray::full()
{
	return m_count>=m_nbe;
}

/*
bool MemoryManagerArray::bit(const unsigned int i) const
{
	return available[i>>5]&(1<<(i&0x1F));
}

void MemoryManagerArray::set(const unsigned int i)
{
	available[i>>5]|=(1<<(i&0x1F));
}

void MemoryManagerArray::unset(const unsigned int i)
{
	available[i>>5]&=~(1<<(i&0x1F));
}

int MemoryManagerArray::nextSet(const unsigned int i0) const
{
	unsigned int i=i0+1;
	unsigned int j=i>>5;

	if(available[j]!=0)
	{
		for(unsigned int k=i&0x1F;k<32;k++)
		{
			unsigned int l=j<<5|k;
			if(l<i) continue;
			if(bit(l)) return l;
		}
		j++; i+=32; i&=~0x1F;
	}

	while(j<1024)
	{
		if(available[j]==0) { j++; i+=32; continue; }
		for(unsigned int k=0;k<32;k++)
		{
			unsigned int l=j<<5|k;
			//if(l<i) continue;
			if(bit(l)) return l;
		}
		j++; i+=32;
	}
	return -1;
}

int MemoryManagerArray::nextUnset(const unsigned int i0) const
{
	unsigned int i=i0+1;
	unsigned int j=i>>5;

	if(available[j]!=0xFFFFFFFF)
	{
		for(unsigned int k=i&0x1F;k<32;k++)
		{
			unsigned int l=j<<5|k;
			if(l<i) continue;
			if(!bit(l)) return l;
		}
		j++; i+=32; i&=~0x1F;
	}

	while(j<1024)
	{
		if(available[j]==0xFFFFFFFF) { j++; i+=32; continue; }
		for(unsigned int k=0;k<32;k++)
		{
			unsigned int l=j<<5|k;
			//if(l<i) continue;
			if(!bit(l)) return l;
		}
		j++; i+=32;
	}
	return -1;
}
*/

void * MemoryManagerArray::allocate( size_t size , const char* file , long line)
{
	if(size>m_size) //!=
	{
		void* r=MMA_STD_ALLOC(size);
		qWarning("invalid size %ld %p in %ld: %s#%ld",size,r,m_size,file,line);
		return r;
	}

	int i;
	m_mutex.lock();

	if(m_count>=m_nbe)
	{
		m_mutex.unlock();
		void* r=MMA_STD_ALLOC(size);
		qWarning("block %ld full %d (asking %ld bytes): %s#%ld",m_size,m_count,size,file,line);
		return r;
	}

	// development phase
	if(size<m_size)
	{
		m_wasted+=(m_size-size);
		m_stats[size]+=1;
		//qWarning("MemoryManagerArray::sup-allocate %ld %ld",size,m_size);
	}

	// development phase
	if((m_count>=(m_nbe*90l)/100)&&(m_nbe>=100)&&((m_count%(m_nbe/100))==0))
		qWarning("block %ld saturating %d (asking %ld bytes): %s#%ld",m_size,m_count,size,file,line);

	if((m_lastfree>=0)&&(m_lastfree<m_nbe)&&m_available.bit(m_lastfree))//m_available[m_lastfree])
	{
		i=m_lastfree;
		m_available.unset(i);//m_available[i]=false;
		m_count++;
		if(m_max<m_count) m_max++;
		m_lastfree=-1;
	}
	else
	{
		i=m_available.nextSet(0); //from start
		if((i<0)||(i>=m_nbe))
			i=m_nbe;
		else
		{
			m_available.unset(i);
			m_count++;
			if(m_max<m_count) m_max++;
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

	if((i+1<m_nbe)&&m_available.bit(i+1))//m_available[i+1])
		m_lastfree=i+1;

	m_mutex.unlock();

	if(i>=m_nbe)
	{
		qWarning("block %ld suprizingly full %d %s#%ld",m_size,m_count,file,line);
		return MMA_STD_ALLOC(size);
	}

	//qWarning("allocate n°%d %d/%d in %ld %s#%ld",i,m_count,m_nbe,m_size,file,line);
	return m_data+i*m_size;
}

bool MemoryManagerArray::deallocate( void * ptr , const char* file , long line)
{
	if((ptr<m_data)||(ptr>=m_data+m_nbe*m_size))
	{
		//qWarning("free pointer out %p",ptr);
		//MMA_STD_FREE(ptr);
		return false;
	}

	size_t s=((char*)ptr)-m_data;
	if((s%m_size)!=0)
		qFatal("error: MemoryManagerArray::free: invalid ptr %s#%ld",file,line);

	const int i=s/m_size;
	m_mutex.lock();
	if((i<0)||(i>=m_nbe))
		qFatal("error: i out of range: %d in %ld: %s#%ld",i,m_size,file,line);
	if(m_available.bit(i))//m_available[i])
		qFatal("error: should be taken n°%d in %ld: %s#%ld",i,m_size,file,line);
	m_available.set(i);//m_available[i]=true;
	m_lastfree=i;
	m_count--;
	if(m_count<0)
		qFatal("error: negative count in %ld: %s#%ld",m_size,file,line);
	m_mutex.unlock();

	//qWarning("deallocate n°%d %d/%d in %ld %s#%ld",i,m_count,m_nbe,m_size,file,line);
	return true;
}
