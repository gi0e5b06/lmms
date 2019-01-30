/*
 * WaveForm.h -
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

#ifndef WAVEFORM_H
#define WAVEFORM_H

#include "JournallingObject.h"
//#include "Model.h"
//#include "ComboBoxModel.h"
#include "MemoryManager.h"
//#include "fft_helpers.h"
#include "lmms_basics.h"
//#include "lmms_math.h"
#include "interpolation.h"

#include <QObject>
#include <QDomElement>

// fastnormsinf01 -> WaveForm::sin::f(x)

typedef real_t (*wavefunction_t)(const real_t);

class WaveForm : public QObject  // public JournallingObject
{
    Q_OBJECT
    MM_OPERATORS

  public:
    WaveForm(const QString&        _name,
             const wavefunction_t  _func,
             const interpolation_t _mode    = Exact,
             const int             _quality = 8);
    WaveForm(const QString&        _name,
             const QString&        _file,
             const interpolation_t _mode    = Linear,
             const int             _quality = 8);
    WaveForm(const QString&        _name,
             real_t*               _data,
             const int             _size,
             const interpolation_t _mode    = Linear,
             const int             _quality = 8);
    WaveForm(const QString&        _name,
             const sampleFrame*    _data,
             const int             _size,
             const interpolation_t _mode    = Linear,
             const int             _quality = 8);
    virtual ~WaveForm();

    /*
    virtual void saveSettings(QDomDocument&  doc,
                              QDomElement&   element,
                              const QString& name,
                              const bool     unique = true);
    virtual void loadSettings(const QDomElement& element,
                              const QString&     name,
                              const bool         required = true);
    */

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

    static const wavefunction_t sine;
    static const wavefunction_t triangle;
    static const wavefunction_t sawtooth;
    static const wavefunction_t square;
    static const wavefunction_t harshsaw;
    static const wavefunction_t sqpeak;
    static const wavefunction_t whitenoise;
    static const wavefunction_t sqrt;

  signals:
    void dataChanged();

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
        real_t*         m_normBuf;
    };

    static Plan PLAN;
    */

  protected:
    WaveForm(const QString&        _name,
             const interpolation_t _mode,
             const int             _quality);

    virtual void rebuild() final;
    virtual void build() final;

    // internal
    virtual bool build_frames();
    virtual bool normalize_frames();
    virtual bool rotate_frames(int _n);

    bool            m_built;
    QString         m_name;
    interpolation_t m_mode;
    int             m_quality;
    wavefunction_t  m_func;
    QString         m_file;
    real_t*         m_data;
    int             m_size;    // size of the data -1
    bool            m_static;  // true if data should be saved
};

#endif
