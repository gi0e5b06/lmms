/*
 * BasicFilters.h - simple but powerful filter-class with most used filters
 *
 * original file by ???
 * modified and enhanced by Tobias Doerffel
 *
 * Lowpass_SV code originally from Nekobee, Copyright (C) 2004 Sean Bolton and
 * others adapted & modified for use in LMMS
 *
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef BASIC_FILTERS_H
#define BASIC_FILTERS_H

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include "lmms_basics.h"

#include <cmath>
//#include "templates.h"
#include "MemoryManager.h"
#include "interpolation.h"
#include "lmms_constants.h"

template <ch_cnt_t CHANNELS = DEFAULT_CHANNELS>
class BasicFilters;

template <ch_cnt_t CHANNELS = DEFAULT_CHANNELS>
class LinkwitzRiley
{
    MM_OPERATORS
  public:
    LinkwitzRiley(sample_rate_t sampleRate)
    {
        m_sampleRate = sampleRate;
        clearHistory();
    }
    virtual ~LinkwitzRiley()
    {
    }

    inline void clearHistory()
    {
        for(int i = 0; i < CHANNELS; ++i)
        {
            m_z1[i] = m_z2[i] = m_z3[i] = m_z4[i] = 0.;
        }
    }

    inline void setSampleRate(sample_rate_t sampleRate)
    {
        m_sampleRate = sampleRate;
    }

    inline void setCoeffs(frequency_t freq)
    {
        // wc
        const double wc  = D_2PI * freq;
        const double wc2 = wc * wc;
        const double wc3 = wc2 * wc;
        m_wc4            = wc2 * wc2;

        // k
        const double k  = wc / tan(D_PI * freq / m_sampleRate);
        const double k2 = k * k;
        const double k3 = k2 * k;
        m_k4            = k2 * k2;

        // a
        static const double sqrt2   = sqrt(2.0);
        const double        sq_tmp1 = sqrt2 * wc3 * k;
        const double        sq_tmp2 = sqrt2 * wc * k3;

        m_a = 1.0
              / (4.0 * wc2 * k2 + 2.0 * sq_tmp1 + m_k4 + 2.0 * sq_tmp2
                 + m_wc4);

        // b
        m_b1 = (4. * (m_wc4 + sq_tmp1 - m_k4 - sq_tmp2)) * m_a;
        m_b2 = (6. * m_wc4 - 8. * wc2 * k2 + 6. * m_k4) * m_a;
        m_b3 = (4. * (m_wc4 - sq_tmp1 + sq_tmp2 - m_k4)) * m_a;
        m_b4 = (m_k4 - 2. * sq_tmp1 + m_wc4 - 2. * sq_tmp2 + 4. * wc2 * k2)
               * m_a;
    }

    inline void setLowpass(frequency_t freq)
    {
        setCoeffs(freq);
        m_a0 = m_wc4 * m_a;
        m_a1 = 4. * m_a0;
        m_a2 = 6. * m_a0;
    }

    inline void setHighpass(frequency_t freq)
    {
        setCoeffs(freq);
        m_a0 = m_k4 * m_a;
        m_a1 = -4. * m_a0;
        m_a2 = 6. * m_a0;
    }

    inline sample_t update(sample_t in, ch_cnt_t ch)
    {
        const double x = in - (m_z1[ch] * m_b1) - (m_z2[ch] * m_b2)
                         - (m_z3[ch] * m_b3) - (m_z4[ch] * m_b4);
        const double y = (m_a0 * x) + (m_z1[ch] * m_a1) + (m_z2[ch] * m_a2)
                         + (m_z3[ch] * m_a1) + (m_z4[ch] * m_a0);
        m_z4[ch] = m_z3[ch];
        m_z3[ch] = m_z2[ch];
        m_z2[ch] = m_z1[ch];
        m_z1[ch] = x;

        return y;
    }

  private:
    sample_rate_t m_sampleRate;
    double        m_wc4;
    double        m_k4;
    double        m_a, m_a0, m_a1, m_a2;
    double        m_b1, m_b2, m_b3, m_b4;

    typedef double frame[CHANNELS];
    frame          m_z1, m_z2, m_z3, m_z4;
};
typedef LinkwitzRiley<2> StereoLinkwitzRiley;

template <ch_cnt_t CHANNELS = DEFAULT_CHANNELS>
class BiQuad
{
    MM_OPERATORS
  public:
    BiQuad()
    {
        clearHistory();
    }
    virtual ~BiQuad()
    {
    }

    inline void
            setCoeffs(real_t a1, real_t a2, real_t b0, real_t b1, real_t b2)
    {
        m_a1 = a1;
        m_a2 = a2;
        m_b0 = b0;
        m_b1 = b1;
        m_b2 = b2;
    }
    inline void clearHistory()
    {
        for(int i = 0; i < CHANNELS; ++i)
        {
            m_z1[i] = 0.;
            m_z2[i] = 0.;
        }
    }
    inline sample_t update(sample_t in, ch_cnt_t ch)
    {
        // biquad filter in transposed form
        const real_t out = m_z1[ch] + m_b0 * in;
        m_z1[ch]         = m_b1 * in + m_z2[ch] - m_a1 * out;
        m_z2[ch]         = m_b2 * in - m_a2 * out;
        return out;
    }

  private:
    real_t m_a1, m_a2, m_b0, m_b1, m_b2;
    real_t m_z1[CHANNELS], m_z2[CHANNELS];

    friend class BasicFilters<CHANNELS>;  // needed for subfilter stuff in
                                          // BasicFilters
};
typedef BiQuad<2> StereoBiQuad;

template <ch_cnt_t CHANNELS = DEFAULT_CHANNELS>
class OnePole
{
    MM_OPERATORS
  public:
    OnePole()
    {
        m_a0 = 1.0;
        m_b1 = 0.0;
        for(int i = 0; i < CHANNELS; ++i)
        {
            m_z1[i] = 0.0;
        }
    }
    virtual ~OnePole()
    {
    }

    inline void setCoeffs(real_t a0, real_t b1)
    {
        m_a0 = a0;
        m_b1 = b1;
    }

    inline sample_t update(sample_t s, ch_cnt_t ch)
    {
        if(abs(s) <= SILENCE && abs(m_z1[ch]) <= SILENCE)
            return 0.;
        return m_z1[ch] = s * m_a0 + m_z1[ch] * m_b1;
    }

  private:
    real_t m_a0, m_b1;
    real_t m_z1[CHANNELS];
};
typedef OnePole<2> StereoOnePole;

template <ch_cnt_t CHANNELS>
class BasicFilters
{
    MM_OPERATORS
  public:
    enum FilterTypes
    {
        LowPass,
        HiPass,
        BandPass_CSG,
        BandPass_CZPG,
        Notch,
        AllPass,
        Moog,
        DoubleLowPass,
        Lowpass_RC12,
        Bandpass_RC12,
        Highpass_RC12,
        Lowpass_RC24,
        Bandpass_RC24,
        Highpass_RC24,
        Formantfilter,
        DoubleMoog,
        Lowpass_SV,
        Bandpass_SV,
        Highpass_SV,
        Notch_SV,
        FastFormant,
        Tripole,
        Brown,
        Pink,
        NumFilters
    };

    static inline frequency_t minFreq()
    {
        return 5.;
    }

    static inline real_t minQ()
    {
        return 0.01;
    }

    static inline frequency_t maxFreq()
    {
        return 20000.;
    }

    static inline real_t maxQ()
    {
        return 10.;
    }

    /*inline*/ void setFilterType(const int _idx)
    {
        m_doubleFilter = _idx == DoubleLowPass || _idx == DoubleMoog;
        if(!m_doubleFilter)
        {
            m_type = static_cast<FilterTypes>(_idx);
            return;
        }

        // Double lowpass mode, backwards-compat for the goofy
        // Add-NumFilters to signify doubleFilter stuff
        m_type = _idx == DoubleLowPass ? LowPass : Moog;
        if(m_subFilter == NULL)
        {
            m_subFilter = new BasicFilters<CHANNELS>(
                    static_cast<sample_rate_t>(m_sampleRate));
        }
        m_subFilter->m_type = m_type;
    }

    /*inline*/ BasicFilters(const sample_rate_t _sample_rate) :
          m_doubleFilter(false), m_sampleRate(_sample_rate),
          m_sampleRatio(1. / double(m_sampleRate)), m_subFilter(NULL)
    {
        clearHistory();
    }

    /*inline*/ virtual ~BasicFilters()
    {
        delete m_subFilter;
    }

    /*inline*/ void clearHistory()
    {
        // reset in/out history for biquads
        m_biQuad.clearHistory();

        // reset in/out history
        for(ch_cnt_t _chnl = 0; _chnl < CHANNELS; ++_chnl)
        {
            // reset in/out history for moog-filter
            m_y1[_chnl] = m_y2[_chnl] = m_y3[_chnl] = m_y4[_chnl]
                    = m_oldx[_chnl] = m_oldy1[_chnl] = m_oldy2[_chnl]
                    = m_oldy3[_chnl]                 = 0.;

            // tripole
            m_last[_chnl]   = 0.;
            m_brownv[_chnl] = 0.;

            // reset in/out history for RC-filters
            m_rclp0[_chnl] = m_rcbp0[_chnl] = m_rchp0[_chnl]
                    = m_rclast0[_chnl]      = 0.;
            m_rclp1[_chnl] = m_rcbp1[_chnl] = m_rchp1[_chnl]
                    = m_rclast1[_chnl]      = 0.;

            for(int i = 0; i < 6; i++)
                m_vfbp[i][_chnl] = m_vfhp[i][_chnl] = m_vflast[i][_chnl] = 0.;

            // reset in/out history for SV-filters
            m_delay1[_chnl] = 0.;
            m_delay2[_chnl] = 0.;
            m_delay3[_chnl] = 0.;
            m_delay4[_chnl] = 0.;
        }
    }

    /*inline*/ sample_t update(sample_t _in0, ch_cnt_t _chnl)
    {
        sample_t out;
        switch(m_type)
        {
            case Moog:
            {
                sample_t x = _in0 - m_r * m_y4[_chnl];

                // four cascaded onepole filters
                // (bilinear transform)
                m_y1[_chnl] = bound(
                        -10., (x + m_oldx[_chnl]) * m_p - m_k * m_y1[_chnl],
                        10.);
                m_y2[_chnl] = bound(-10.,
                                    (m_y1[_chnl] + m_oldy1[_chnl]) * m_p
                                            - m_k * m_y2[_chnl],
                                    10.);
                m_y3[_chnl] = bound(-10.,
                                    (m_y2[_chnl] + m_oldy2[_chnl]) * m_p
                                            - m_k * m_y3[_chnl],
                                    10.);
                m_y4[_chnl] = bound(-10.,
                                    (m_y3[_chnl] + m_oldy3[_chnl]) * m_p
                                            - m_k * m_y4[_chnl],
                                    10.);

                m_oldx[_chnl]  = x;
                m_oldy1[_chnl] = m_y1[_chnl];
                m_oldy2[_chnl] = m_y2[_chnl];
                m_oldy3[_chnl] = m_y3[_chnl];
                out            = m_y4[_chnl]
                      - m_y4[_chnl] * m_y4[_chnl] * m_y4[_chnl] * (1. / 6.);
                break;
            }

            // 3x onepole filters with 4x oversampling and interpolation of
            // oversampled signal: input signal is linear-interpolated after
            // oversampling, output signal is averaged from oversampled
            // outputs
            case Tripole:
            {
                out       = 0.;
                real_t ip = 0.;
                for(int i = 0; i < 4; ++i)
                {
                    ip += 0.25;
                    sample_t x = linearInterpolate(m_last[_chnl], _in0, ip)
                                 - m_r * m_y3[_chnl];

                    m_y1[_chnl]    = bound(-10.,
                                        (x + m_oldx[_chnl]) * m_p
                                                - m_k * m_y1[_chnl],
                                        10.);
                    m_y2[_chnl]    = bound(-10.,
                                        (m_y1[_chnl] + m_oldy1[_chnl]) * m_p
                                                - m_k * m_y2[_chnl],
                                        10.);
                    m_y3[_chnl]    = bound(-10.,
                                        (m_y2[_chnl] + m_oldy2[_chnl]) * m_p
                                                - m_k * m_y3[_chnl],
                                        10.);
                    m_oldx[_chnl]  = x;
                    m_oldy1[_chnl] = m_y1[_chnl];
                    m_oldy2[_chnl] = m_y2[_chnl];

                    out += (m_y3[_chnl]
                            - m_y3[_chnl] * m_y3[_chnl] * m_y3[_chnl]
                                      * (1. / 6.));
                }
                out *= 0.25;
                m_last[_chnl] = _in0;
                return out;
            }

                // 4-pole state-variant lowpass filter, adapted from Nekobee
                // source code and extended to other SV filter types
                // /* Hal Chamberlin's state variable filter */

            case Lowpass_SV:
            case Bandpass_SV:
            {
                real_t highpass;

                for(int i = 0; i < 2; ++i)  // 2x oversample
                {
                    m_delay2[_chnl]
                            = m_delay2[_chnl]
                              + m_svf1 * m_delay1[_chnl]; /* delay2/4 =
                                                             lowpass output */
                    highpass = _in0 - m_delay2[_chnl]
                               - m_svq * m_delay1[_chnl];
                    m_delay1[_chnl] = m_svf1 * highpass
                                      + m_delay1[_chnl]; /* delay1/3 =
                                                            bandpass output */

                    m_delay4[_chnl]
                            = m_delay4[_chnl] + m_svf2 * m_delay3[_chnl];
                    highpass = m_delay2[_chnl] - m_delay4[_chnl]
                               - m_svq * m_delay3[_chnl];
                    m_delay3[_chnl] = m_svf2 * highpass + m_delay3[_chnl];
                }

                /* mix filter output into output buffer */
                return m_type == Lowpass_SV ? m_delay4[_chnl]
                                            : m_delay3[_chnl];
            }

            case Highpass_SV:
            {
                real_t hp;

                for(int i = 0; i < 2; ++i)  // 2x oversample
                {
                    m_delay2[_chnl]
                            = m_delay2[_chnl] + m_svf1 * m_delay1[_chnl];
                    hp = _in0 - m_delay2[_chnl] - m_svq * m_delay1[_chnl];
                    m_delay1[_chnl] = m_svf1 * hp + m_delay1[_chnl];
                }

                return hp;
            }

            case Notch_SV:
            {
                real_t hp1, hp2;

                for(int i = 0; i < 2; ++i)  // 2x oversample
                {
                    m_delay2[_chnl]
                            = m_delay2[_chnl]
                              + m_svf1 * m_delay1[_chnl]; /* delay2/4 =
                                                             lowpass output */
                    hp1 = _in0 - m_delay2[_chnl] - m_svq * m_delay1[_chnl];
                    m_delay1[_chnl] = m_svf1 * hp1
                                      + m_delay1[_chnl]; /* delay1/3 =
                                                            bandpass output */

                    m_delay4[_chnl]
                            = m_delay4[_chnl] + m_svf2 * m_delay3[_chnl];
                    hp2 = m_delay2[_chnl] - m_delay4[_chnl]
                          - m_svq * m_delay3[_chnl];
                    m_delay3[_chnl] = m_svf2 * hp2 + m_delay3[_chnl];
                }

                /* mix filter output into output buffer */
                return m_delay4[_chnl] + hp1;
            }

                // 4-times oversampled simulation of an active
                // RC-Bandpass,-Lowpass,-Highpass- Filter-Network as it was
                // used in nearly all modern analog synthesizers. This can be
                // driven up to self-oscillation (BTW: do not remove the
                // limits!!!). (C) 1998 ... 2009 S.Fendt. Released under the
                // GPL v2.0 or any later version.

            case Lowpass_RC12:
            {
                sample_t lp, bp, hp, in;
                for(int n = 4; n != 0; --n)
                {
                    in = _in0 + m_rcbp0[_chnl] * m_rcq;
                    in = bound(-1., in, 1.);

                    lp = in * m_rcb + m_rclp0[_chnl] * m_rca;
                    lp = bound(-1., lp, 1.);

                    hp = m_rcc * (m_rchp0[_chnl] + in - m_rclast0[_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_rcb + m_rcbp0[_chnl] * m_rca;
                    bp = bound(-1., bp, 1.);

                    m_rclast0[_chnl] = in;
                    m_rclp0[_chnl]   = lp;
                    m_rchp0[_chnl]   = hp;
                    m_rcbp0[_chnl]   = bp;
                }
                return lp;
            }
            case Highpass_RC12:
            case Bandpass_RC12:
            {
                sample_t hp, bp, in;
                for(int n = 4; n != 0; --n)
                {
                    in = _in0 + m_rcbp0[_chnl] * m_rcq;
                    in = bound(-1., in, 1.);

                    hp = m_rcc * (m_rchp0[_chnl] + in - m_rclast0[_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_rcb + m_rcbp0[_chnl] * m_rca;
                    bp = bound(-1., bp, 1.);

                    m_rclast0[_chnl] = in;
                    m_rchp0[_chnl]   = hp;
                    m_rcbp0[_chnl]   = bp;
                }
                return m_type == Highpass_RC12 ? hp : bp;
            }

            case Lowpass_RC24:
            {
                sample_t lp, bp, hp, in;
                for(int n = 4; n != 0; --n)
                {
                    // first stage is as for the 12dB case...
                    in = _in0 + m_rcbp0[_chnl] * m_rcq;
                    in = bound(-1., in, 1.);

                    lp = in * m_rcb + m_rclp0[_chnl] * m_rca;
                    lp = bound(-1., lp, 1.);

                    hp = m_rcc * (m_rchp0[_chnl] + in - m_rclast0[_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_rcb + m_rcbp0[_chnl] * m_rca;
                    bp = bound(-1., bp, 1.);

                    m_rclast0[_chnl] = in;
                    m_rclp0[_chnl]   = lp;
                    m_rcbp0[_chnl]   = bp;
                    m_rchp0[_chnl]   = hp;

                    // second stage gets the output of the first stage as
                    // input...
                    in = lp + m_rcbp1[_chnl] * m_rcq;
                    in = bound(-1., in, 1.);

                    lp = in * m_rcb + m_rclp1[_chnl] * m_rca;
                    lp = bound(-1., lp, 1.);

                    hp = m_rcc * (m_rchp1[_chnl] + in - m_rclast1[_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_rcb + m_rcbp1[_chnl] * m_rca;
                    bp = bound(-1., bp, 1.);

                    m_rclast1[_chnl] = in;
                    m_rclp1[_chnl]   = lp;
                    m_rcbp1[_chnl]   = bp;
                    m_rchp1[_chnl]   = hp;
                }
                return lp;
            }
            case Highpass_RC24:
            case Bandpass_RC24:
            {
                sample_t hp, bp, in;
                for(int n = 4; n != 0; --n)
                {
                    // first stage is as for the 12dB case...
                    in = _in0 + m_rcbp0[_chnl] * m_rcq;
                    in = bound(-1., in, 1.);

                    hp = m_rcc * (m_rchp0[_chnl] + in - m_rclast0[_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_rcb + m_rcbp0[_chnl] * m_rca;
                    bp = bound(-1., bp, 1.);

                    m_rclast0[_chnl] = in;
                    m_rchp0[_chnl]   = hp;
                    m_rcbp0[_chnl]   = bp;

                    // second stage gets the output of the first stage as
                    // input...
                    in = m_type == Highpass_RC24
                                 ? hp + m_rcbp1[_chnl] * m_rcq
                                 : bp + m_rcbp1[_chnl] * m_rcq;

                    in = bound(-1., in, 1.);

                    hp = m_rcc * (m_rchp1[_chnl] + in - m_rclast1[_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_rcb + m_rcbp1[_chnl] * m_rca;
                    bp = bound(-1., bp, 1.);

                    m_rclast1[_chnl] = in;
                    m_rchp1[_chnl]   = hp;
                    m_rcbp1[_chnl]   = bp;
                }
                return m_type == Highpass_RC24 ? hp : bp;
            }

            case Formantfilter:
            case FastFormant:
            {
                if(abs(_in0) <= SILENCE && abs(m_vflast[0][_chnl]) <= SILENCE)
                {
                    return 0.;
                }  // performance hack - skip processing when the numbers get
                   // too small
                sample_t hp, bp, in;

                out = 0;
                const int os
                        = m_type == FastFormant
                                  ? 1
                                  : 4;  // no oversampling for fast formant
                for(int o = 0; o < os; ++o)
                {
                    // first formant
                    in = _in0 + m_vfbp[0][_chnl] * m_vfq;
                    in = bound(-1., in, 1.);

                    hp = m_vfc[0]
                         * (m_vfhp[0][_chnl] + in - m_vflast[0][_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_vfb[0] + m_vfbp[0][_chnl] * m_vfa[0];
                    bp = bound(-1., bp, 1.);

                    m_vflast[0][_chnl] = in;
                    m_vfhp[0][_chnl]   = hp;
                    m_vfbp[0][_chnl]   = bp;

                    in = bp + m_vfbp[2][_chnl] * m_vfq;
                    in = bound(-1., in, 1.);

                    hp = m_vfc[0]
                         * (m_vfhp[2][_chnl] + in - m_vflast[2][_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_vfb[0] + m_vfbp[2][_chnl] * m_vfa[0];
                    bp = bound(-1., bp, 1.);

                    m_vflast[2][_chnl] = in;
                    m_vfhp[2][_chnl]   = hp;
                    m_vfbp[2][_chnl]   = bp;

                    in = bp + m_vfbp[4][_chnl] * m_vfq;
                    in = bound(-1., in, 1.);

                    hp = m_vfc[0]
                         * (m_vfhp[4][_chnl] + in - m_vflast[4][_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_vfb[0] + m_vfbp[4][_chnl] * m_vfa[0];
                    bp = bound(-1., bp, 1.);

                    m_vflast[4][_chnl] = in;
                    m_vfhp[4][_chnl]   = hp;
                    m_vfbp[4][_chnl]   = bp;

                    out += bp;

                    // second formant
                    in = _in0 + m_vfbp[0][_chnl] * m_vfq;
                    in = bound(-1., in, 1.);

                    hp = m_vfc[1]
                         * (m_vfhp[1][_chnl] + in - m_vflast[1][_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_vfb[1] + m_vfbp[1][_chnl] * m_vfa[1];
                    bp = bound(-1., bp, 1.);

                    m_vflast[1][_chnl] = in;
                    m_vfhp[1][_chnl]   = hp;
                    m_vfbp[1][_chnl]   = bp;

                    in = bp + m_vfbp[3][_chnl] * m_vfq;
                    in = bound(-1., in, 1.);

                    hp = m_vfc[1]
                         * (m_vfhp[3][_chnl] + in - m_vflast[3][_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_vfb[1] + m_vfbp[3][_chnl] * m_vfa[1];
                    bp = bound(-1., bp, 1.);

                    m_vflast[3][_chnl] = in;
                    m_vfhp[3][_chnl]   = hp;
                    m_vfbp[3][_chnl]   = bp;

                    in = bp + m_vfbp[5][_chnl] * m_vfq;
                    in = bound(-1., in, 1.);

                    hp = m_vfc[1]
                         * (m_vfhp[5][_chnl] + in - m_vflast[5][_chnl]);
                    hp = bound(-1., hp, 1.);

                    bp = hp * m_vfb[1] + m_vfbp[5][_chnl] * m_vfa[1];
                    bp = bound(-1., bp, 1.);

                    m_vflast[5][_chnl] = in;
                    m_vfhp[5][_chnl]   = hp;
                    m_vfbp[5][_chnl]   = bp;

                    out += bp;
                }
                return m_type == FastFormant ? out * 2. : out * 0.5;
            }
            case Brown:
            {
                if(m_brownf != 1.)
                    _in0 = _in0 * m_brownf
                           + (1. - m_brownf) * m_last[_chnl];  // * 3.5;

                //+ bound(-1., exp(_in0 -
                // m_last[_chnl]), 1.)
                //       * (m_brownq - 1.);
                if(m_brownq != 1.)
                    _in0 += folding(
                            (m_brownq - 1.) * 0.25
                            * folding(m_last[_chnl] + m_brownv[_chnl]));

                //_in0 = fraction(_in0);
                _in0 = bound(-1., _in0, 1.);

                out = (m_brownv[_chnl] + (0.02 * _in0)) / 1.02;

                m_brownv[_chnl] = out;
                m_last[_chnl]   = _in0;
                return out;  // * 3.5;  // compensate for gain
            }
            case Pink:
            {
                if(m_pinkf != 1.)
                    _in0 = _in0 * m_pinkf + (1. - m_pinkf) * m_last[_chnl];
                if(m_pinkq != 1.)
                    _in0 += folding(
                            (m_pinkq - 1.) * 0.25
                            * folding(m_last[_chnl] + m_pinkv[_chnl]));

                //_in0 = fraction(_in0);
                _in0 = bound(-1., _in0, 1.);

                b0[_chnl] = 0.99886 * b0[_chnl] + _in0 * 0.0555179;
                b1[_chnl] = 0.99332 * b1[_chnl] + _in0 * 0.0750759;
                b2[_chnl] = 0.96900 * b2[_chnl] + _in0 * 0.1538520;
                b3[_chnl] = 0.86650 * b3[_chnl] + _in0 * 0.3104856;
                b4[_chnl] = 0.55000 * b4[_chnl] + _in0 * 0.5329522;
                b5[_chnl] = -0.7616 * b5[_chnl] - _in0 * 0.0168980;

                out = (b0[_chnl] + b1[_chnl] + b2[_chnl] + b3[_chnl]
                       + b4[_chnl] + b5[_chnl]
                       + b6[_chnl]
                       //+ b5[_chnl] * b5[_chnl] * b5[_chnl] * (m_pinkq - 1.)
                       //+ b6[_chnl] * b6[_chnl] * b6[_chnl] * (m_pinkq - 1.)
                       + (_in0 * 0.5362))
                      * 0.11;
                b6[_chnl]       = _in0 * 0.115926;
                m_pinkv[_chnl] = out;
                m_last[_chnl]   = _in0;
                return out;
            }
            default:
                out = m_biQuad.update(_in0, _chnl);
                break;
        }

        if(m_doubleFilter)
        {
            return m_subFilter->update(out, _chnl);
        }

        // Clipper band limited sigmoid
        return out;
    }

    inline void calcFilterCoeffs(frequency_t _freq, real_t _q)
    {
        // temp coef vars
        _q    = bound(minQ(), _q, maxQ());
        _freq = bound(minFreq(), _freq, maxFreq());

        if(m_type == Lowpass_RC12 || m_type == Bandpass_RC12
           || m_type == Highpass_RC12 || m_type == Lowpass_RC24
           || m_type == Bandpass_RC24 || m_type == Highpass_RC24)
        {
            //_freq = bound(50., _freq, 20000.);

            const real_t sr = m_sampleRatio * 0.25;
            const real_t f  = 1. / (_freq * D_2PI);

            m_rca = 1. - sr / (f + sr);
            m_rcb = 1. - m_rca;
            m_rcc = f / (f + sr);

            // Stretch Q/resonance, as self-oscillation reliably starts at a q
            // of ~2.5 - ~2.6
            m_rcq = _q * 0.25;
            return;
        }

        if(m_type == Formantfilter || m_type == FastFormant)
        {
            //_freq = qBound<real_t>(minFreq(), _freq, 20000.);
            // limit freq and q for not getting bad noise
            // out of the filter...

            // formats for a, e, i, o, u, a
            static const real_t FF[6][2]
                    = {{1000, 1400}, {500, 2300}, {320, 3200},
                       {500, 1000},  {320, 800},  {1000, 1400}};
            static const real_t freqRatio = 4. / 14000.;

            // Stretch Q/resonance
            m_vfq = _q * 0.25;

            // frequency in lmms ranges from 1Hz to 20kHz --14000Hz
            const real_t vowelf = _freq * freqRatio;
            const int    vowel  = static_cast<int>(vowelf);
            const real_t fract  = vowelf - vowel;

            // interpolate between formant frequencies
            const real_t f0 = 1.
                              / (linearInterpolate(FF[vowel + 0][0],
                                                   FF[vowel + 1][0], fract)
                                 * D_2PI);
            const real_t f1 = 1.
                              / (linearInterpolate(FF[vowel + 0][1],
                                                   FF[vowel + 1][1], fract)
                                 * D_2PI);

            // samplerate coeff: depends on oversampling
            const real_t sr = m_type == FastFormant ? m_sampleRatio
                                                    : m_sampleRatio * 0.25;

            m_vfa[0] = 1. - sr / (f0 + sr);
            m_vfb[0] = 1. - m_vfa[0];
            m_vfc[0] = f0 / (f0 + sr);
            m_vfa[1] = 1. - sr / (f1 + sr);
            m_vfb[1] = 1. - m_vfa[1];
            m_vfc[1] = f1 / (f1 + sr);
            return;
        }

        if(m_type == Moog || m_type == DoubleMoog)
        {
            // [ 0 - 0.5 ]
            // const real_t f = bound(minFreq(), _freq, 20000.) *
            // m_sampleRatio;
            const real_t f = _freq * m_sampleRatio;
            // (Empirical tunning)
            m_p = (3.6 - 3.2 * f) * f;
            m_k = 2. * m_p - 1;
            m_r = _q * fastexp((1 - m_p) * 1.386249);  // pow(D_E,...)

            if(m_doubleFilter)
            {
                m_subFilter->m_r = m_r;
                m_subFilter->m_p = m_p;
                m_subFilter->m_k = m_k;
            }
            return;
        }

        if(m_type == Tripole)
        {
            // bound(20., _freq, 20000.)
            const real_t f = _freq * m_sampleRatio * 0.25;

            m_p = (3.6 - 3.2 * f) * f;
            m_k = 2. * m_p - 1.;
            m_r = _q * 0.1 * exp((1. - m_p) * 1.386249);  // pow(D_E,...)

            return;
        }

        if(m_type == Lowpass_SV || m_type == Bandpass_SV
           || m_type == Highpass_SV || m_type == Notch_SV)
        {
            // qMax(minFreq(), _freq)
            const real_t f = sin(_freq * m_sampleRatio * D_PI);
            m_svf1         = qMin(f, 0.825);
            m_svf2         = qMin(f * 2., 0.825);
            m_svq          = qMax(0.0001, 2. - (_q * 0.1995));
            return;
        }

        if(m_type == Brown)
        {
            m_brownf = _freq / maxFreq();
            m_brownf = log10(1. + m_brownf) / log10(2.);
            m_brownq = _q / 0.5;
            qInfo("brown: %f", m_brownf);
            return;
        }
        if(m_type == Pink)
        {
            m_pinkf = _freq / maxFreq();
            m_pinkf = log10(1. + m_pinkf) / log10(2.);
            m_pinkq = _q / 0.5;
            qInfo("pink: %f", m_pinkf);
            return;
        }

        // other filters
        //_freq = bound(minFreq(), _freq, 20000.);

        const real_t omega = D_2PI * _freq * m_sampleRatio;
        const real_t tsin  = sin(omega) * 0.5;
        const real_t tcos  = cos(omega);

        const real_t alpha = tsin / _q;

        const real_t a0 = 1. / (1. + alpha);

        const real_t a1 = -2. * tcos * a0;
        const real_t a2 = (1. - alpha) * a0;

        switch(m_type)
        {
            case LowPass:
            {
                const real_t b1 = (1. - tcos) * a0;
                const real_t b0 = b1 * 0.5;
                m_biQuad.setCoeffs(a1, a2, b0, b1, b0);
                break;
            }
            case HiPass:
            {
                const real_t b1 = (-1. - tcos) * a0;
                const real_t b0 = b1 * -0.5;
                m_biQuad.setCoeffs(a1, a2, b0, b1, b0);
                break;
            }
            case BandPass_CSG:
            {
                const real_t b0 = tsin * a0;
                m_biQuad.setCoeffs(a1, a2, b0, 0., -b0);
                break;
            }
            case BandPass_CZPG:
            {
                const real_t b0 = alpha * a0;
                m_biQuad.setCoeffs(a1, a2, b0, 0., -b0);
                break;
            }
            case Notch:
            {
                m_biQuad.setCoeffs(a1, a2, a0, a1, a0);
                break;
            }
            case AllPass:
            {
                m_biQuad.setCoeffs(a1, a2, a2, a1, 1.);
                break;
            }
            default:
                break;
        }

        if(m_doubleFilter)
        {
            m_subFilter->m_biQuad.setCoeffs(m_biQuad.m_a1, m_biQuad.m_a2,
                                            m_biQuad.m_b0, m_biQuad.m_b1,
                                            m_biQuad.m_b2);
        }
    }

  private:
    // biquad filter
    BiQuad<CHANNELS> m_biQuad;

    // coeffs for moog-filter
    real_t m_r, m_p, m_k;

    // coeffs for RC-type-filters
    real_t m_rca, m_rcb, m_rcc, m_rcq;

    // coeffs for formant-filters
    real_t m_vfa[4], m_vfb[4], m_vfc[4], m_vfq;

    // coeffs for Lowpass_SV (state-variant lowpass)
    real_t m_svf1, m_svf2, m_svq;

    real_t m_brownf, m_brownq, m_pinkf, m_pinkq;

    typedef sample_t bf_frame_t[CHANNELS];

    // in/out history for moog-filter
    bf_frame_t m_y1, m_y2, m_y3, m_y4, m_oldx, m_oldy1, m_oldy2, m_oldy3;
    // additional one for Tripole filter
    bf_frame_t m_last;
    bf_frame_t m_brownv,m_pinkv;

    // in/out history for RC-type-filters
    bf_frame_t m_rcbp0, m_rclp0, m_rchp0, m_rclast0;
    bf_frame_t m_rcbp1, m_rclp1, m_rchp1, m_rclast1;

    // in/out history for Formant-filters
    bf_frame_t m_vfbp[6], m_vfhp[6], m_vflast[6];

    // in/out history for Lowpass_SV (state-variant lowpass)
    bf_frame_t m_delay1, m_delay2, m_delay3, m_delay4;

    // in/out history for Pink, ...
    bf_frame_t b0, b1, b2, b3, b4, b5, b6;

    FilterTypes m_type;
    bool        m_doubleFilter;

    sample_rate_t           m_sampleRate;
    DOUBLE                  m_sampleRatio;
    BasicFilters<CHANNELS>* m_subFilter;
};

#endif
