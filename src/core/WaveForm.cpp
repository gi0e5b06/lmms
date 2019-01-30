/*
 * WaveForm.cpp -
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "WaveForm.h"

#include "Backtrace.h"
#include "SampleBuffer.h"
//#include "ConfigManager.h"
#include "Engine.h"
#include "Mixer.h"
#include "interpolation.h"
#include "lmms_float.h"

#include <QDir>
#include <QMutex>
#include <QMutexLocker>

WaveForm::WaveForm(const QString&        _name,
                   const interpolation_t _mode,
                   const int             _quality) :
      m_built(false),
      m_name(_name), m_mode(_mode), m_quality(_quality)
{
    m_func   = nullptr;
    m_file   = "";
    m_data   = nullptr;
    m_size   = -1;
    m_static = true;
}

WaveForm::WaveForm(const QString&        _name,
                   const wavefunction_t  _func,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _mode, _quality)
{
    if(_func == nullptr)
    {
        BACKTRACE
        qFatal("WaveForm::WaveForm null function: %s", qPrintable(_name));
    }

    m_func = _func;
    if(m_mode == Exact)
        m_built = true;
}

WaveForm::WaveForm(const QString&        _name,
                   const QString&        _file,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _mode, _quality)
{
    m_file   = _file;
    m_static = false;
    if(m_mode == Exact)
        m_mode = Linear;  // Config
}

WaveForm::WaveForm(const QString&        _name,
                   real_t*               _data,
                   const int             _size,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _mode, _quality)
{
    m_data = MM_ALLOC(real_t, _size);
    for(int f = 0; f < _size; ++f)
        m_data[f] = _data[f];
    m_size = _size - 1;
    if(m_mode == Exact)
        m_mode = Linear;
    m_built = true;
}

WaveForm::WaveForm(const QString&        _name,
                   const sampleFrame*    _data,
                   const int             _size,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _mode, _quality)
{
    m_data = MM_ALLOC(real_t, _size);
    for(int f = 0; f < _size; ++f)
        m_data[f] = _data[f][0];
    m_size = _size - 1;
    if(m_mode == Exact)
        m_mode = Linear;
    m_built = true;
}

WaveForm::~WaveForm()
{
    if(m_data != nullptr)
        MM_FREE(m_data);
}

void WaveForm::rebuild()
{
    m_built = false;
    build();
}

void WaveForm::build()
{
    if(m_built)
        return;
    static QMutex s_building;
    QMutexLocker  locker(&s_building);
    if(m_built)
        return;
    if(build_frames())
    {
        normalize_frames();
        m_built = true;
        emit dataChanged();
    }
}

bool WaveForm::build_frames()
{
    if(m_func)
    {
        // 10 -> 389711
        // 9 -> 132427
        // 8 -> 50000
        // 7 -> 15291
        // 6 -> 5196
        // 5 -> 1766
        // 4 -> 600 !
        // 3 -> 204
        // 2 -> 69
        // 1 -> 24
        // 0 -> 8 !
        m_size = int(ceil(8. * pow(2.94283095638, m_quality))) - 1;
        if(m_data != nullptr)
            MM_FREE(m_data);
        m_data = MM_ALLOC(real_t, m_size + 1);
        for(int i = m_size; i >= 0; --i)
            m_data[i] = m_func(real_t(i) / real_t(m_size));
        return true;  // m_built = true;
    }
    else if(m_file != "")
    {
        SampleBuffer*      sb   = new SampleBuffer(m_file, false, false);
        int                size = sb->frames();
        const sampleFrame* data = sb->data();

        if(m_data != nullptr)
            MM_FREE(m_data);
        m_data = MM_ALLOC(real_t, size);
        for(int f = 0; f < size; ++f)
            m_data[f] = data[f][0];
        m_size = size - 1;
        delete sb;
        return true;  // m_built = true;
    }
    else
    {
        qWarning("Error: waveform not built: %s", qPrintable(m_name));
        BACKTRACE
        return false;
    }
}

bool WaveForm::normalize_frames()
{
    const int size = m_size + 1;

    real_t omin = m_data[0];
    real_t omax = m_data[0];

    for(int f = 1; f < size; ++f)
    {
        const real_t ov = m_data[f];
        if(ov < omin)
            omin = ov;
        if(ov > omax)
            omax = ov;
    }

    real_t os = qMax(abs(omax), abs(omin));
    if(os >= SILENCE)
    {
        const real_t k = 1. / os;
        if(k != 1.)
        {
            for(int f = 0; f < size; ++f)
                m_data[f] *= k;
            return true;
        }
    }

    return false;
}

bool WaveForm::rotate_frames(int _n)
{
    if(_n == 0)
        return false;

    const int size = m_size + 1;
    real_t*   data = MM_ALLOC(real_t, size);

    for(int f = 0; f < size; ++f)
        data[f] = m_data[(f + _n) % size];

    real_t* old = m_data;
    m_data      = data;
    MM_FREE(old);
    return true;
}

// x must be between 0. and 1.
real_t WaveForm::f(const real_t _x) const
{
    return f(_x, m_mode);
}

real_t WaveForm::f(const real_t _x, const real_t _antialias) const
{
    return f(_x, _antialias, m_mode);
}

// x must be between 0. and 1.
real_t WaveForm::f(const real_t          _x,
                   const real_t          _antialias,
                   const interpolation_t _m) const
{
    real_t r = 0.;

    if(_antialias > 0.)
    {
        // qInfo("antialias f: %f", _antialias);
        const int N = 13;
        for(int i = -N; i <= N; i++)
            r += f(fraction(_x + 1. + _antialias * i / (N + 1)), _m);
        r /= (2 * N + 1);
        /*
        const real_t d  = 2. / Engine::mixer()->processingSampleRate();
        const real_t r0 = f(fraction(_x + 1. - 3. * d), false, _m);
        const real_t r1 = f(fraction(_x + 1. - d), false, _m);
        const real_t rx = f(fraction(_x), false, _m);
        const real_t r2 = f(fraction(_x + d), false, _m);
        const real_t r3 = f(fraction(_x + 3. * d), false, _m);
        return rx + (r2 - rx) * d / sqrt(sqf(d) + sqf(r2 - rx))
               + (rx - r1) * d / sqrt(sqf(d) + sqf(rx - r1))
               + (r3 - rx) * 3. * d / sqrt(sqf(3. * d) + sqf(r3 - rx))
               + (rx - r0) * 3. * d / sqrt(sqf(3. * d) + sqf(rx - r1));
        */
        // optimal4pInterpolate(r0, r1, r2, r3, 0.5);
    }
    else
    {
        r = f(_x, _m);
    }

    return r;
}

// x must be between 0. and 1.
real_t WaveForm::f(const real_t _x, const interpolation_t _m) const
{
    if(!m_built)
        const_cast<WaveForm*>(this)->build();

    interpolation_t m = _m;
    if((m != m_mode) && (m == Exact || m_mode == Exact))
        m = m_mode;

    real_t r = 0.;
    switch(m)
    {
        case Discrete:
        {
            const int i = _x * m_size;

            if(m_data == nullptr || i < 0 || i > m_size)
            {
                qInfo("WaveForm::f Discrete x=%f m_data=%p i=%d size=%d", _x,
                      m_data, i, m_size);
                if(_x < 0. || _x > 1. || isnan(_x))
                {
                    BACKTRACE
                    return 0.;
                }
            }

            r = m_data[i];
        }
        break;
        case Rounded:
        {
            const int i = round(_x * m_size);

            if(m_data == nullptr || i < 0 || i > m_size)
            {
                qInfo("WaveForm::f Rounded x=%f m_data=%p i=%d size=%d", _x,
                      m_data, i, m_size);
                if(_x < 0. || _x > 1. || isnan(_x))
                {
                    BACKTRACE
                    return 0.;
                }
            }

            r = m_data[i];
        }
        break;
        case Linear:
        case Cosinus:
        case Optimal2:
        {
            const real_t j = _x * m_size;
            const int    i = j;  // int()

            if(m_data == nullptr || i < 0 || i > m_size)
            {
                qInfo("WaveForm::f Optimal2 x=%f m_data=%p i=%d size=%d", _x,
                      m_data, i, m_size);
                if(_x < 0. || _x > 1. || isnan(_x))
                {
                    BACKTRACE
                    return 0.;
                }
            }
            const real_t d  = j - i;
            const real_t r0 = m_data[i];
            const real_t r1 = m_data[i + 1];
            // r = r0 + d * (r1 - r0);
            switch(m_mode)
            {
                case Linear:
                    r = linearInterpolate(r0, r1, d);
                    break;
                case Cosinus:
                    r = cosinusInterpolate(r0, r1, d);
                    break;
                case Optimal2:
                    r = optimalInterpolate(r0, r1, d);
                    break;
                default:
                    break;
            }
        }
        break;
        case Exact:
            r = m_func(_x);
            break;
        default:
        {
            const real_t p  = _x * m_size;
            const int    i1 = p;  // int()
            const real_t d  = p - i1;
            const int    i0 = (i1 == 0 ? m_size + 1 : i1 - 1);
            const int    i2 = i1 + 1;
            const int    i3 = (i2 == m_size + 1 ? 0 : i2 + 1);
            const real_t r0 = m_data[i0];
            const real_t r1 = m_data[i1];
            const real_t r2 = m_data[i2];
            const real_t r3 = m_data[i3];

            switch(m)
            {
                case Cubic:
                    r = cubicInterpolate(r0, r1, r2, r3, d);
                    break;
                case Hermite:
                    r = hermiteInterpolate(r0, r1, r2, r3, d);
                    break;
                case Lagrange:
                    r = lagrangeInterpolate(r0, r1, r2, r3, d);
                    break;
                case Optimal4:
                    r = optimal4pInterpolate(r0, r1, r2, r3, d);
                    break;
                default:
                    break;
            }
        }
        break;
    }
    return r;
}

/*
WaveForm::Plan::Plan()
{
    m_normBuf = (real_t*)fftwf_malloc((FFT_BUFFER_SIZE * 2) *
sizeof(real_t));
    // memset(m_normBuf, 0, 2 * FFT_BUFFER_SIZE);

    m_specBuf = (fftwf_complex*)fftwf_malloc((FFT_BUFFER_SIZE + 1)
                                             * sizeof(fftwf_complex));

    m_r2cPlan = fftwf_plan_dft_r2c_1d(FFT_BUFFER_SIZE * 2, m_normBuf,
                                      m_specBuf, FFTW_MEASURE);

    m_c2rPlan = fftwf_plan_dft_c2r_1d(FFT_BUFFER_SIZE * 2, m_normBuf,
                                      m_specBuf, FFTW_MEASURE);
}

WaveForm::Plan::~Plan()
{
    fftwf_destroy_plan(m_c2rPlan);
    fftwf_destroy_plan(m_r2cPlan);
    fftwf_free(m_specBuf);
    fftwf_free(m_normBuf);
}

WaveForm::Plan::build(WaveForm& _wf, const real_t _cut)
{
    QMutexLocker locker(m_mutex);

    for(int i = FFT_BUFFER_SIZE * 2 - 1; i >= 0; --i)
    {
        real_t x      = real_t(i) / real_t(FFT_BUFFER_SIZE * 2);
        real_t y      = _wf->f(x, false);
        m_normBuf[i] = y;
    }

    fftwf_execute(m_r2cPlan);

    for(int f = int(ceil(_cut)); f <= FFT_BUFFER_SIZE; f++)
    {
        m_specBuf[f][0] = 0.;
        m_specBuf[f][1] = 0.;
    }

    fftwf_execute(m_c2rPlan);
}

WaveForm::Plan WaveForm::PLAN;
*/
