/*
 * KickerOsc.h - alternative sweeping oscillator
 *
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2014 grejppi <grejppi/at/gmail.com>
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

#ifndef KICKER_OSC_H
#define KICKER_OSC_H

#include "DspEffectLibrary.h"
//#include "Oscillator.h"
#include "MemoryManager.h"
#include "WaveForm.h"
#include "interpolation.h"
#include "lmms_math.h"

template <class FX = DspEffectLibrary::StereoBypass>
class KickerOsc
{
    MM_OPERATORS

  public:
    KickerOsc(const FX&       fx,
              const real_t    start,
              const real_t    end,
              const real_t    tail,
              const real_t    noise,
              const real_t    offset,
              const real_t    slope,
              const real_t    phaseFactor,
              const real_t    env,
              const real_t    diststart,
              const real_t    distend,
              const real_t    length,
              const WaveForm* _sin,
              const WaveForm* _whn) :
          m_phase(offset),
          m_startFreq(start), m_endFreq(end), m_tail(tail), m_noise(noise),
          m_slope(slope), m_phaseFactor(phaseFactor), m_env(env),
          m_distStart(diststart), m_distEnd(distend),
          m_hasDistEnv(diststart != distend), m_length(length), m_sin(_sin),
          m_whn(_whn), m_FX(fx), m_counter(0), m_freq(start)
    {
    }

    virtual ~KickerOsc()
    {
    }

    void update(sampleFrame* buf, const fpp_t frames, const real_t sampleRate)
    {
        for(fpp_t frame = 0; frame < frames; ++frame)
        {
            // const double gain = ( 1 - fastPow( ( m_counter < m_length ) ?
            // m_counter / m_length : 1, m_env ) );
            if(m_counter < m_length)
            {
                /*
                  m_versionModel
                  const real_t gain = 1. - fastPow( m_counter / m_length,
                  m_env ); const sample_t s = ( Oscillator::sinSample(
                  m_phase ) * ( 1. - m_noise ) ) +
                  ( Oscillator::noiseSample( 0. ) * gain *
                  gain * m_noise );
                */
                /*
                  const real_t gain
                  = 1. - fastPowf(m_counter / m_length, m_env);
                  const real_t    p = positivefraction(m_phase);
                  const sample_t s = (WaveForm::SINE.f(p) * (1. - m_noise))
                  + (WaveForm::WHITENOISE.f(p) * gain * gain * m_noise);
                */

                const real_t gain
                        = 1.
                          - fastPow(real_t(m_counter) / real_t(m_length),
                                    m_env);
                const real_t   p1 = positivefraction(m_phase);
                const real_t   p2 = positivefraction(m_phaseFactor * m_phase);
                const sample_t s
                        = (m_sin->f(p1) * (1. - m_noise))
                          + (m_whn->f(p2) * fastPow(gain, m_tail) * m_noise);

                // if( gain>1. || m_noise>1. || s>1. ) qInfo("Kicker g=%f
                // n=%f s=%f",gain,m_noise,s);
                buf[frame][0] = s * gain;
                buf[frame][1] = s * gain;
            }
            else
            {
                buf[frame][0] = 0.;
                buf[frame][1] = 0.;
            }

            // update distortion envelope if necessary
            if(m_hasDistEnv && m_counter < m_length)
            {
                real_t thres = linearInterpolate(m_distStart, m_distEnd,
                                                 m_counter / m_length);
                m_FX.leftFX().setThreshold(thres);
                m_FX.rightFX().setThreshold(thres);
            }

            m_FX.nextSample(buf[frame][0], buf[frame][1]);
            m_phase += m_freq / double(sampleRate);

            if(m_counter < m_length)
            {
                const double change
                        = double(m_startFreq - m_endFreq)
                          * (1.
                             - fastPow(double(m_counter) / double(m_length),
                                       double(m_slope)));
                m_freq = m_endFreq + change;
            }
            ++m_counter;
        }
    }

  private:
    double          m_phase;
    const real_t    m_startFreq;
    const real_t    m_endFreq;
    const real_t    m_tail;
    const real_t    m_noise;
    const real_t    m_slope;
    const real_t    m_phaseFactor;
    const real_t    m_env;
    const real_t    m_distStart;
    const real_t    m_distEnd;
    const bool      m_hasDistEnv;
    const real_t    m_length;
    const WaveForm* m_sin;
    const WaveForm* m_whn;

    FX            m_FX;
    unsigned long m_counter;
    double        m_freq;
};

#endif
