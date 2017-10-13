
#include <assert.h>
#include <string.h>
#include "Bitset.h"

Bitset::Bitset(unsigned int size)
{
	assert(sizeof(unsigned int)>=4); //32 bits
	m_size=size;
	m_ints=size/sizeof(unsigned int)+1;
	m_data=new unsigned int[m_ints];
}

Bitset::Bitset(unsigned int size, bool initial)
{
	assert(sizeof(unsigned int)>=4); //32 bits
	m_size=size;
	m_ints=size/sizeof(unsigned int)+1;
	m_data=new unsigned int[m_ints];
	fill(initial);
}

Bitset::~Bitset()
{
	delete m_data;
}

void Bitset::fill(bool b)
{
	memset(m_data,(b ? 0xFF : 0x00),m_ints);
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
