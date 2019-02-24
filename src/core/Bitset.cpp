
#include "Bitset.h"
#include "MemoryManager.h"

#include <cassert>
#include <cstring>

/*
Bitset::Bitset(unsigned int size)
        : Bitset(size,false)
{
}
*/

Bitset::Bitset(unsigned int _size, bool _initial)
{
	assert(sizeof(unsigned int)>=4); //32 bits
	m_size=_size;
	m_ints=_size/sizeof(unsigned int)+1;
	m_data=MM_ALLOC(unsigned int,m_ints);//new unsigned int[m_ints];
	fill(_initial);
}

Bitset::Bitset(const Bitset& _other)
{
	m_size=_other.m_size;
	m_ints=_other.m_ints;
	m_data=MM_ALLOC(unsigned int,m_ints);//new unsigned int[m_ints];
	memcpy(m_data,_other.m_data,m_ints);
}

Bitset::~Bitset()
{
	MM_FREE(m_data);//delete m_data;
}

unsigned int Bitset::size() const
{
        return m_size;
}

void Bitset::fill(bool b)
{
	memset(m_data,(b ? 0xFF : 0x00),m_ints*sizeof(unsigned int));
}

bool Bitset::bit(const unsigned int i) const
{
	assert(/*(i>=0)&&*/(i<m_size));
	return m_data[i>>5]&(1<<(i&0x1F));
}

void Bitset::set(const unsigned int i)
{
	assert(/*(i>=0)&&*/(i<m_size));
	m_data[i>>5]|=(1<<(i&0x1F));
}

void Bitset::unset(const unsigned int i)
{
	assert(/*(i>=0)&&*/(i<m_size));
	m_data[i>>5]&=~(1<<(i&0x1F));
}

void Bitset::toggle(const unsigned int i)
{
	if(bit(i)) unset(i);
        else set(i);
}

int Bitset::nextSet(const unsigned int i0) const
{
	assert(/*(i0>=0)&&*/(i0<m_size));
	unsigned int i=i0;
	unsigned int j=i>>5;

	if(m_data[j]!=0)
	{
		for(unsigned int k=i&0x1F;k<32;k++)
		{
			unsigned int l=j<<5|k;
			if(l<i) continue;
			if(bit(l))
			{
				if(l>=m_size) return -1;
				return l;
			}
		}
		j++; i+=32; i&=~0x1F;
	}

	while(j<m_ints)
	{
		if(m_data[j]==0) { j++; i+=32; continue; }
		for(unsigned int k=0;k<32;k++)
		{
			unsigned int l=j<<5|k;
			if(bit(l))
			{
				if(l>=m_size) return -1;
				return l;
			}
		}
		j++; i+=32;
	}
	return -1;
}

int Bitset::nextUnset(const unsigned int i0) const
{
	assert(/*(i0>=0)&&*/(i0<m_size));
	unsigned int i=i0;
	unsigned int j=i>>5;

	if(m_data[j]!=~0x00)
	{
		for(unsigned int k=i&0x1F;k<32;k++)
		{
			unsigned int l=j<<5|k;
			if(l<i) continue;
			if(!bit(l))
			{
				if(l>=m_size) return -1;
				return l;
			}
		}
		j++; i+=32; i&=~0x1F;
	}

	while(j<m_ints)
	{
		if(m_data[j]==~0x00) { j++; i+=32; continue; }
		for(unsigned int k=0;k<32;k++)
		{
			unsigned int l=j<<5|k;
			if(!bit(l))
			{
				if(l>=m_size) return -1;
				return l;
			}
		}
		j++; i+=32;
	}
	return -1;
}
