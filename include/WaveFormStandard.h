/*
 * WaveFormStandard.h -
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

#ifndef WAVEFORM_STANDARD_H
#define WAVEFORM_STANDARD_H

#include "ComboBoxModel.h"
#include "WaveForm.h"
//#include "MemoryManager.h"
//#include "fft_helpers.h"
//#include "lmms_basics.h"
//#include "lmms_math.h"
//#include "interpolation.h"

class WaveFormStandard : public WaveForm
{
    MM_OPERATORS

  public:
    WaveFormStandard(const QString&        _name,
                     const int             _bank,
                     const int             _index,
                     const wavefunction_t  _func,
                     const interpolation_t _mode    = Exact,
                     const int             _quality = 8);
    WaveFormStandard(const QString&        _name,
                     const int             _bank,
                     const int             _index,
                     const QString&        _file,
                     const interpolation_t _mode    = Linear,
                     const int             _quality = 8);
    WaveFormStandard(const QString&        _name,
                     const int             _bank,
                     const int             _index,
                     real_t*               _data,
                     const int             _size,
                     const interpolation_t _mode    = Linear,
                     const int             _quality = 8);
    WaveFormStandard(const QString&        _name,
                     const int             _bank,
                     const int             _index,
                     const sampleFrame*    _data,
                     const int             _size,
                     const interpolation_t _mode    = Linear,
                     const int             _quality = 8);
    WaveFormStandard(const QString&          _name,
                     const int               _bank,
                     const int               _index,
                     const WaveFormStandard* _wave);
    virtual ~WaveFormStandard();

    /*
    // function
    // x must be between 0. and 1.
    // return between -1. and 1.
    real_t f(const real_t _x) const;
    real_t f(const real_t _x, const interpolation_t _m) const;
    real_t f(const real_t _x, const real_t _antialias) const;
    real_t f(const real_t          _x,
             const real_t          _antialias,
             const interpolation_t _m) const;

    inline const QString& name() const
    {
        return m_name;
    }
    */

    inline const int bank() const
    {
        return m_bank;
    }

    inline const int index() const
    {
        return m_index;
    }

    static const WaveFormStandard* get(const int _bank, const int _index);

    static void fillBankModel(ComboBoxModel& _model);
    static void fillIndexModel(ComboBoxModel& _model, const int _bank);

    // Standard waves
    static const int ZERO_BANK        = 20;
    static const int ZERO_INDEX       = 40;
    static const int SINE_BANK        = 0;
    static const int SINE_INDEX       = 0;
    static const int WHITENOISE_BANK  = 0;
    static const int WHITENOISE_INDEX = 6;

    static const int MIN_BANK  = 0;
    static const int MIN_INDEX = 0;
    static const int MAX_BANK  = 127;
    static const int MAX_INDEX = 127;

    static const WaveFormStandard* const SINE;
    static const WaveFormStandard* const TRIANGLE;
    static const WaveFormStandard* const RAMP;
    static const WaveFormStandard* const SQUARE;
    static const WaveFormStandard* const HARSHRAMP;
    static const WaveFormStandard* const SQPEAK;
    static const WaveFormStandard* const WHITENOISE;
    static const WaveFormStandard* const ZERO;
    static const WaveFormStandard* const SQRT;
    static const WaveFormStandard* const SHARPGAUSS;

    /*
    static const wavefunction_t sine;
    static const wavefunction_t triangle;
    static const wavefunction_t sawtooth;
    static const wavefunction_t square;
    static const wavefunction_t harshsaw;
    static const wavefunction_t sqpeak;
    static const wavefunction_t whitenoise;
    static const wavefunction_t sqrt;
    */

    class Set
    {
      public:
        Set();
        virtual ~Set();
        const WaveFormStandard* get(const int _bank, const int _index);
        void                    set(const int               _bank,
                                    const int               _index,
                                    const WaveFormStandard* _wf);
        void                    fillBankModel(ComboBoxModel& _model);
        void fillIndexModel(ComboBoxModel& _model, const int _bank);

      private:
        void createMissing();
        void createZeroed(int _bank);
        void createCentered(int _bank);
        void createDegraded(int _bank, bool _linear, int _quality);
        void createSoften(int _bank, real_t _bandwidth);

        QString                 m_bankNames[MAX_BANK - MIN_BANK + 1];
        const WaveFormStandard* m_stock[MAX_BANK - MIN_BANK + 1]
                                       [MAX_INDEX - MIN_INDEX + 1];
    };

    static Set WAVEFORMS;

    /*
    class Plan
    {
      public:
        Plan();
        virtual ~Plan();
        void build(WaveFormStandard& _wf);

    private:
        QMutex         m_mutex;
        const int      FFT_BUFFER_SIZE = 22050;
        fftwf_plan     m_r2cPlan;
        fftwf_plan     m_c2rPlan;
        fftwf_complex* m_specBuf;
        real_t*         m_normBuf;
    };

    static Plan PLAN;
    */

  protected:
    WaveFormStandard(const QString&        _name,
                     const int             _bank,
                     const int             _index,
                     const interpolation_t _mode,
                     const int             _quality);
    /*
    real_t hardness();
    real_t softness();
    bool   absolute();
    bool   complement();
    bool   opposite();
    bool   reverse();
    bool   zeroed();
    bool   centered();
    real_t phase();

    WaveFormStandard* setHardness(real_t _hardness);
    WaveFormStandard* setSoftness(real_t _softness);
    WaveFormStandard* setAbsolute(bool _b);
    WaveFormStandard* setComplement(bool _b);
    WaveFormStandard* setOpposite(bool _b);
    WaveFormStandard* setReverse(bool _b);
    WaveFormStandard* setZeroed(bool _b);
    WaveFormStandard* setCentered(bool _b);
    WaveFormStandard* setPhase(real_t _p);

    void rebuild();
    void build();
    void harden();
    void soften();
    void acoren();
    void zero();
    void center();
    void dephase();

    real_t          m_hardness;
    real_t          m_softness;
    bool            m_absolute;
    bool            m_opposite;
    bool            m_complement;
    bool            m_reverse;
    bool            m_zero;
    bool            m_center;
    real_t          m_phase;
    */

    int m_bank;
    int m_index;
};

#endif
