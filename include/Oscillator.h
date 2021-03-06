/*
 * Oscillator.h - declaration of class Oscillator
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef OSCILLATOR_H
#define OSCILLATOR_H

#include "lmmsconfig.h"

//#include <math.h>
//#ifdef LMMS_HAVE_STDLIB_H
//#include <stdlib.h>
//#endif

#include "SampleBuffer.h"
#include "WaveForm.h"

//#include "lmms_constants.h"
#include "lmms_math.h"

class IntModel;

class EXPORT Oscillator /*final*/
{
    MM_OPERATORS
  public:
    enum WaveShapes
    {
        SineWave,
        TriangleWave,
        SawWave,
        SquareWave,
        MoogSawWave,
        ExponentialWave,
        WhiteNoise,
        UserDefinedWave,
        ZeroWave,
        OneWave,
        MinusOneWave,
        NumWaveShapes
    };

    enum ModulationAlgos
    {
        PhaseModulation,
        AmplitudeModulation,
        SignalMix,
        SynchronizedBySubOsc,
        FrequencyModulation,
        PulseModulation,
        OutputModulation,
        NumModulationAlgos
    };

    Oscillator(const IntModel*    _wave_shape_model,
               const IntModel*    _mod_algo_model,
               const frequency_t& _freq,
               const real_t&      _detuning,
               const real_t&      _phase_offset,
               const volume_t&    _volume,
               Oscillator*        _m_subOsc = nullptr);

    virtual ~Oscillator()
    {
        delete m_subOsc;
    }

    INLINE void setUserWave(SampleBufferPointer _wave)
    {
        m_userWave = _wave;
    }

    void update(sampleFrame* _ab, const fpp_t _frames, const ch_cnt_t _chnl);

    // now follow the wave-shape-routines...

    static INLINE sample_t sinSample(real_t _sample)
    {
        // return sinf( _sample * F_2PI );
        // return fastnormsinf01(fraction(_sample));
        return WaveForm::sine(positivefraction(_sample));
    }

    static INLINE sample_t triangleSample(const real_t _sample)
    {
        /*
        const real_t ph = fraction( _sample );
        if( ph <= 0.25f )
        {
                return ph * 4.0f;
        }
        else if( ph <= 0.75f )
        {
                return 2.0f - ph * 4.0f;
        }
        return ph * 4.0f - 4.0f;
        */
        // return fasttrianglef01(fraction(_sample));
        return WaveForm::triangle(positivefraction(_sample));
    }

    static INLINE sample_t sawSample(const real_t _sample)
    {
        // return -1.0f + fraction( _sample ) * 2.0f;
        // return fastsawf01(fraction(_sample));
        return WaveForm::ramp(positivefraction(_sample));
    }

    static INLINE sample_t squareSample(const real_t _sample)
    {
        // return ( fraction( _sample ) > 0.5f ) ? -1.0f : 1.0f;
        // return fastsquaref01(fraction(_sample));
        return WaveForm::square(positivefraction(_sample));
    }

    static INLINE sample_t moogSawSample(const real_t _sample)
    {
        /*
        const real_t ph = fraction( _sample );
        if( ph < 0.5f )
        {
                return -1.0f + ph * 4.0f;
        }
        return 1.0f - 2.0f * ph;
        */
        // return fastmoogsawf01(fraction(_sample));
        return WaveForm::harshramp(positivefraction(_sample));
    }

    static INLINE sample_t expSample(const real_t _sample)
    {
        /*
        real_t ph = fraction( _sample );
        if( ph > 0.5f )
        {
                ph = 1.0f - ph;
        }
        return -1.0f + 8.0f * ph * ph;
        */
        // return fastnormexpf01(fraction(_sample));
        return WaveForm::sqpeak(positivefraction(_sample));
    }

    static INLINE sample_t noiseSample(const real_t _sample)
    {
        // Precise implementation
        // return 1.0f - rand() * 2.0f / RAND_MAX;

        // Fast implementation
        // return 1.0f - fast_rand() * 2.0f / FAST_RAND_MAX;
        return WaveForm::whitenoise(positivefraction(_sample));
    }

    INLINE sample_t userWaveSample(const real_t _sample) const
    {
        return m_userWave->userWaveSample(_sample);
    }

  private:
    const IntModel*     m_waveShapeModel;
    const IntModel*     m_modulationAlgoModel;
    const frequency_t&  m_freq;
    const real_t&       m_detuning;
    const volume_t&     m_volume;
    const real_t&       m_ext_phaseOffset;
    Oscillator*         m_subOsc;
    real_t              m_phaseOffset;
    real_t              m_phase;
    SampleBufferPointer m_userWave;

    void updateNoSub(sampleFrame*   _ab,
                     const fpp_t    _frames,
                     const ch_cnt_t _chnl);
    void updatePM(sampleFrame*   _ab,
                  const fpp_t    _frames,
                  const ch_cnt_t _chnl);
    void updateAM(sampleFrame*   _ab,
                  const fpp_t    _frames,
                  const ch_cnt_t _chnl);
    void updateMix(sampleFrame*   _ab,
                   const fpp_t    _frames,
                   const ch_cnt_t _chnl);
    void updateSync(sampleFrame*   _ab,
                    const fpp_t    _frames,
                    const ch_cnt_t _chnl);
    void updateFM(sampleFrame*   _ab,
                  const fpp_t    _frames,
                  const ch_cnt_t _chnl);

    real_t      syncInit(sampleFrame*   _ab,
                         const fpp_t    _frames,
                         const ch_cnt_t _chnl);
    INLINE bool syncOk(real_t _osc_coeff);

    template <WaveShapes W>
    void updateNoSub(sampleFrame*   _ab,
                     const fpp_t    _frames,
                     const ch_cnt_t _chnl);
    template <WaveShapes W>
    void updatePM(sampleFrame*   _ab,
                  const fpp_t    _frames,
                  const ch_cnt_t _chnl);
    template <WaveShapes W>
    void updateAM(sampleFrame*   _ab,
                  const fpp_t    _frames,
                  const ch_cnt_t _chnl);
    template <WaveShapes W>
    void updateMix(sampleFrame*   _ab,
                   const fpp_t    _frames,
                   const ch_cnt_t _chnl);
    template <WaveShapes W>
    void updateSync(sampleFrame*   _ab,
                    const fpp_t    _frames,
                    const ch_cnt_t _chnl);
    template <WaveShapes W>
    void updateFM(sampleFrame*   _ab,
                  const fpp_t    _frames,
                  const ch_cnt_t _chnl);

    template <WaveShapes W>
    INLINE sample_t getSample(const real_t _sample);

    INLINE void recalcPhase();
};

#endif
