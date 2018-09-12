/*
 * Ring.cpp - an effective and flexible implementation of a ringbuffer
 * for LMMS
 *
 * Copyright (c) 2014 Vesa Kivimäki
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "Ring.h"

#include "Engine.h"
//#include "MixHelpers.h"
#include "Mixer.h"

Ring::Ring(f_cnt_t size) :
      // m_fpp( Engine::mixer()->framesPerPeriod() ),
      m_maxLevel(0.f), m_frozen(false), m_sampleRateAware(false),
      m_sampleRate(Engine::mixer()->processingSampleRate()),
      m_size(size)  // + m_fpp )
{
    m_duration = framesToMs(m_size);
    m_buffer   = new sampleFrame[m_size];
    memset(m_buffer, 0, m_size * sizeof(sampleFrame));
    m_position = 0;
}

Ring::Ring(float _duration) :
      // m_fpp( Engine::mixer()->framesPerPeriod() ),
      m_maxLevel(0.f), m_frozen(false),
      m_sampleRateAware(false),  // changed below
      m_sampleRate(Engine::mixer()->processingSampleRate()),
      m_duration(_duration)
{
    m_size   = msToFrames(_duration);  // m_fpp;
    m_buffer = new sampleFrame[m_size];
    memset(m_buffer, 0, m_size * sizeof(sampleFrame));
    m_position = 0;
    setSampleRateAware(true);
    // qDebug( "m_size %d, m_position %d", m_size, m_position );
}

Ring::~Ring()
{
    delete[] m_buffer;
}

sample_t Ring::value(f_cnt_t _offset, ch_cnt_t _ch)
{
    return m_buffer[relpos(_offset - 1)][_ch];
}

sample_t Ring::value(f_cnt_t _offset, ch_cnt_t _ch, f_cnt_t _pos)
{
    return m_buffer[abspos(_pos + _offset)][_ch];
}

void Ring::reset()
{
    memset(m_buffer, 0, m_size * sizeof(sampleFrame));
    m_position = 0;
    m_maxLevel = 0.f;
}

void Ring::changeSize(f_cnt_t _size)
{
    // size += m_fpp;
    f_cnt_t      old_size   = m_size;
    sampleFrame* old_buffer = m_buffer;
    m_size                  = _size;
    m_buffer                = new sampleFrame[m_size];
    memset(m_buffer, 0, m_size * sizeof(sampleFrame));
    memcpy(m_buffer, old_buffer, qMin(_size, old_size));
    m_position %= m_size;
    m_duration = framesToMs(m_size);
    delete[] old_buffer;
}

void Ring::changeDuration(float _duration)
{
    changeSize(msToFrames(_duration));
}

void Ring::rewind(f_cnt_t _amount)
{
    m_position = relpos(-_amount);
}

void Ring::rewind(float _amount)
{
    rewind(msToFrames(_amount));
}

void Ring::forward(f_cnt_t _amount)
{
    m_position = relpos(_amount);
}

void Ring::forward(float _amount)
{
    forward(msToFrames(_amount));
}

bool Ring::isSampleRateAware()
{
    return m_sampleRateAware;
}

void Ring::setSampleRateAware(bool _b)
{
    Mixer* mixer = Engine::mixer();
    if(!mixer)
        return;

    if(_b)
    {
        connect(mixer, SIGNAL(sampleRateChanged()), this,
                SLOT(updateSamplerate()), Qt::UniqueConnection);
    }
    else
    {
        disconnect(mixer, SIGNAL(sampleRateChanged()), this,
                   SLOT(updateSamplerate()));
    }
}

void Ring::read(sampleFrame& _dst) const
{
    read(&_dst, 1, m_position - 1);
}

void Ring::read(sampleFrame* _dst, const f_cnt_t _length) const
{
    read(_dst, _length, m_position - _length);
}

void Ring::read(sampleFrame*  _dst,
                const f_cnt_t _length,
                const f_cnt_t _pos) const
{
    for(f_cnt_t f = 0; f < _length; ++f)
        for(ch_cnt_t ch = DEFAULT_CHANNELS - 1; ch >= 0; --ch)
            _dst[f][ch] = m_buffer[abspos(_pos + f)][ch];
}

void Ring::read(sampleFrame*  _dst,
                const f_cnt_t _length,
                const float   _pos) const
{
    read(_dst, msToFrames(_pos), _length);
}

void Ring::write(const sampleFrame& _src,
                 const OpMode       _op,
                 const float        _factor)
{
    if(m_frozen)
        return;

    write(&_src, 1, m_position, _op, _factor);
    forward(1);
}

void Ring::write(const sampleFrame* _src,
                 const f_cnt_t      _length,
                 const OpMode       _op,
                 const float        _factor)
{
    if(m_frozen)
        return;

    write(_src, _length, m_position, _op, _factor);
    forward(_length);
}

void Ring::write(const sampleFrame* _src,
                 const f_cnt_t      _length,
                 const f_cnt_t      _pos,
                 const OpMode       _op,
                 const float        _factor)
{
    if(m_frozen)
        return;

    OpMode op = _op;

    if(op == PutMul)
    {
        if(_factor == 1.f)
            op = Put;
        else if(_factor == 0.f)
            op = Nop;
    }

    if(op == AddMul)
    {
        if(_factor == 1.f)
            op = Add;
        else if(_factor == 0.f)
            op = Nop;
    }

    if(op == Nop)
        return;

    for(f_cnt_t f = 0; f < _length; ++f)
        for(ch_cnt_t ch = DEFAULT_CHANNELS - 1; ch >= 0; --ch)
        {
            size_t p = abs(_pos + f);
            switch(op)
            {
                case Put:
                    m_buffer[p][ch] = _src[f][ch];
                    break;
                case Add:
                    m_buffer[p][ch] += _src[f][ch];
                    break;
                case PutMul:
                    m_buffer[p][ch] = _factor * _src[f][ch];
                    break;
                case AddMul:
                    m_buffer[p][ch] += _factor * _src[f][ch];
                    break;
                default:
                    break;
            }
            const float a = fabsf(m_buffer[p][ch]);
            m_maxLevel *= 0.991;
            if(a > m_maxLevel)
                m_maxLevel = a;
        }
}

void Ring::write(const sampleFrame* _src,
                 const f_cnt_t      _length,
                 const float        _pos,
                 const OpMode       _op,
                 const float        _factor)
{
    if(m_frozen)
        return;

    write(_src, _length, msToFrames(_pos), _op, _factor);
}

void Ring::updateSamplerate()
{
    sample_rate_t old_samplerate = m_sampleRate;
    f_cnt_t       old_size       = m_size;
    float         old_duration   = m_duration;

    if(m_sampleRateAware)
    {
        m_sampleRate = Engine::mixer()->processingSampleRate();
        m_duration   = old_duration * m_sampleRate / old_samplerate;
        changeSize(msToFrames(m_duration));
    }
    else
    {
        m_sampleRate = Engine::mixer()->processingSampleRate();
        changeSize(old_size * m_sampleRate / old_samplerate);
    }
}
