#include "ValueBuffer.h"

//#include "interpolation.h"

ValueBuffer::ValueBuffer(const ValueBuffer* _vb)
{
    m_len  = _vb->length();
    m_data = new float[m_len];
    copyFrom(_vb);
}

ValueBuffer::ValueBuffer(const ValueBuffer& _vb) : ValueBuffer(&_vb)
{
}

ValueBuffer::ValueBuffer(int _length)
//: std::vector<float>(_length)
{
    Q_ASSERT(_length > 2);
    m_len  = _length;
    m_data = new float[m_len];
}

ValueBuffer::~ValueBuffer()
//: std::vector<float>(_length)
{
    delete m_data;
    m_len  = 0;
    m_data = nullptr;
}

/*
float ValueBuffer::value(int _offset) const
{
        //return at(_offset % length());
        return m_data[_offset % m_len];
}

const float* ValueBuffer::values() const
{
        //return data();
        return m_data;
}

float *ValueBuffer::values()
{
        //return data();
        return m_data;
}

int ValueBuffer::length() const
{
        //return size();
        return m_len;
}

void ValueBuffer::set(int _i, float _v)
{
        m_data[_i]=_v;
}

void ValueBuffer::clear()
{
        memset(m_data,0,sizeof(float)*m_len);
}
*/

void ValueBuffer::copyFrom(const ValueBuffer* _vb)
{
    const float* src = _vb->values();
    // for(int i=m_len-1; i>=0; --i)
    //        m_data[i]=src[i];
    memcpy(m_data, src, sizeof(float) * m_len);
}

void ValueBuffer::fill(float _value)
{
    // std::fill(begin(), end(), value);
    for(int i = m_len - 1; i >= 0; --i)
        m_data[i] = _value;
}

void ValueBuffer::interpolate(float _start, float _end)
{
    /*
    float i = 0;
    std::generate(begin(), end(), [&]() {
            return linearInterpolate( start, end_, i++ / length());
    });
    */
    const float step = (_end - _start) / (m_len - 1);

    float v = _end;
    for(int i = m_len - 1; i >= 0; --i)
    {
        m_data[i] = v;
        v -= step;
    }  //_start+(_end-_start)*i/(m_len-1);
}
