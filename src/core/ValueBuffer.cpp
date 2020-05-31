#include "ValueBuffer.h"

//#include "interpolation.h"

ValueBuffer::ValueBuffer(const ValueBuffer* _vb)
{
    m_len    = _vb->length();
    m_data   = new real_t[m_len];
    m_period = _vb->period();
    copyFrom(_vb);
}

ValueBuffer::ValueBuffer(const ValueBuffer& _vb) : ValueBuffer(&_vb)
{
}

ValueBuffer::ValueBuffer(int _length)
//: std::vector<real_t>(_length)
{
    Q_ASSERT(_length > 2);
    m_len    = _length;
    m_data   = new real_t[m_len];
    m_period = -1;
}

ValueBuffer::~ValueBuffer()
//: std::vector<real_t>(_length)
{
    delete m_data;
    m_len    = 0;
    m_data   = nullptr;
    m_period = -1;
}

/*
real_t ValueBuffer::value(int _offset) const
{
        //return at(_offset % length());
        return m_data[_offset % m_len];
}

const real_t* ValueBuffer::values() const
{
        //return data();
        return m_data;
}

real_t *ValueBuffer::values()
{
        //return data();
        return m_data;
}

int ValueBuffer::length() const
{
        //return size();
        return m_len;
}

void ValueBuffer::set(int _i, real_t _v)
{
        m_data[_i]=_v;
}

void ValueBuffer::clear()
{
        memset(m_data,0,sizeof(real_t)*m_len);
}
*/

void ValueBuffer::copyFrom(const ValueBuffer* const _vb)
{
    _vb->lock();
    real_t* const src = _vb->values();
    // for(int i=m_len-1; i>=0; --i)
    //      m_data[i]=src[i];
    memcpy(m_data, src, sizeof(real_t) * m_len);
    _vb->unlock();
}

void ValueBuffer::fill(real_t _value)
{
    // std::fill(begin(), end(), value);
    for(int i = m_len - 1; i >= 0; --i)
        m_data[i] = _value;
    // memset?
}

void ValueBuffer::interpolate(real_t _start, real_t _end)
{
    /*
    real_t i = 0;
    std::generate(begin(), end(), [&]() {
            return linearInterpolate( start, end_, i++ / length());
    });
    */
    const real_t step = (_end - _start) / real_t(m_len - 1);

    real_t v = _end;
    for(int i = m_len - 1; i >= 0; --i)
    {
        m_data[i] = v;
        v -= step;
    }  //_start+(_end-_start)*i/(m_len-1);
}
