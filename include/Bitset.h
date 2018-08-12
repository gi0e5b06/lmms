
#ifndef BITSET_H
#define BITSET_H

class Bitset /*final*/
{
 public:
	Bitset(unsigned int size);
	Bitset(unsigned int size, bool initial);
	~Bitset();

	void fill(bool b);
	bool bit(const unsigned int i) const;
	void set(const unsigned int i);
	void unset(const unsigned int i);
	int  nextSet(const unsigned int i) const;
	int  nextUnset(const unsigned int i) const;

 private:
	unsigned int   m_size;
	unsigned int   m_ints;
	unsigned int*  m_data;
};

#endif
