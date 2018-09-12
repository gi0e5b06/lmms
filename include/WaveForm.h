/*
 * WaveForm.h -
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
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

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "ComboBoxModel.h"
#include "MemoryManager.h"
//#include "fft_helpers.h"
#include "lmms_basics.h"
//#include "lmms_math.h"

// fastnormsinf01 -> WaveForm::sin::f(x)

typedef float (*wavefunction_t)(const float);

class WaveForm
{
    MM_OPERATORS

  public:
    enum interpolation_t
    {
        Discrete,
        Rounded,
        Linear,
        Cosinus,
        Optimal2,
        Cubic,
        Hermite,
        Lagrange,
        Optimal4,
        Exact
    };

    WaveForm(const QString&        _name,
             const int             _bank,
             const int             _index,
             const wavefunction_t  _func,
             const interpolation_t _mode    = Exact,
             const int             _quality = 8);
    WaveForm(const QString&        _name,
             const int             _bank,
             const int             _index,
             const QString&        _file,
             const interpolation_t _mode    = Linear,
             const int             _quality = 8);
    WaveForm(const QString&        _name,
             const int             _bank,
             const int             _index,
             float*                _data,
             const int             _size,
             const interpolation_t _mode    = Linear,
             const int             _quality = 8);
    WaveForm(const QString&        _name,
             const int             _bank,
             const int             _index,
             const sampleFrame*    _data,
             const int             _size,
             const interpolation_t _mode    = Linear,
             const int             _quality = 8);
    virtual ~WaveForm();

    // x must be between 0. and 1.
    float f(const float _x) const;
    float f(const float _x, const interpolation_t _m) const;
    float f(const float _x, const float _antialias) const;
    float f(const float           _x,
            const float           _antialias,
            const interpolation_t _m) const;

    inline const QString& name() const
    {
        return m_name;
    }

    inline const int bank() const
    {
        return m_bank;
    }

    inline const int index() const
    {
        return m_index;
    }

    static const WaveForm* get(const int _bank, const int _index);
    static void            fillBankModel(ComboBoxModel& _model);
    static void fillIndexModel(ComboBoxModel& _model, const int _bank);

    // Standard waves
    static const int ZERO_BANK  = 20;
    static const int ZERO_INDEX = 40;
    static const int MIN_BANK   = 0;
    static const int MIN_INDEX  = 0;
    static const int MAX_BANK   = 127;
    static const int MAX_INDEX  = 127;

    static const WaveForm SINE;
    static const WaveForm TRIANGLE;
    static const WaveForm SAWTOOTH;
    static const WaveForm SQUARE;
    static const WaveForm HARSHSAW;
    static const WaveForm SQPEAK;
    static const WaveForm WHITENOISE;
    static const WaveForm ZERO;
    static const WaveForm SQRT;

    static const wavefunction_t sine;
    static const wavefunction_t triangle;
    static const wavefunction_t sawtooth;
    static const wavefunction_t square;
    static const wavefunction_t harshsaw;
    static const wavefunction_t sqpeak;
    static const wavefunction_t whitenoise;
    static const wavefunction_t sqrt;

    class Set
    {
      public:
        Set();
        const WaveForm* get(const int _bank, const int _index);
        void set(const int _bank, const int _index, const WaveForm* _wf);
        void fillBankModel(ComboBoxModel& _model);
        void fillIndexModel(ComboBoxModel& _model, const int _bank);

      private:
        void createDegraded(int _bank, bool _linear, int _quality);

        QString         m_bankNames[MAX_BANK - MIN_BANK + 1];
        const WaveForm* m_stock[MAX_BANK - MIN_BANK + 1]
                               [MAX_INDEX - MIN_INDEX + 1];
    };

    static Set WAVEFORMS;

    /*
    class Plan
    {
      public:
        Plan();
        ~Plan();
        void build(WaveForm& _wf);

    private:
        QMutex         m_mutex;
        const int      FFT_BUFFER_SIZE = 22050;
        fftwf_plan     m_r2cPlan;
        fftwf_plan     m_c2rPlan;
        fftwf_complex* m_specBuf;
        float*         m_normBuf;
    };

    static Plan PLAN;
    */

  protected:
    WaveForm(const QString&        _name,
             const int             _bank,
             const int             _index,
             const interpolation_t _mode,
             const int             _quality);

    void build();

    bool            m_built;
    QString         m_name;
    int             m_bank;
    int             m_index;
    interpolation_t m_mode;
    int             m_quality;
    wavefunction_t  m_func;
    QString         m_file;
    float*          m_data;
    int             m_size;  // size of the data -1
};

#endif
