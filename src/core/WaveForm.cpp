/*
 * WaveForm.cpp -
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

#include "WaveForm.h"

#include "Backtrace.h"
#include "SampleBuffer.h"
//#include "ConfigManager.h"
#include "Engine.h"
#include "Mixer.h"

//#include "lmms_math.h"
#include "interpolation.h"

#include <QDir>
#include <QMutex>
#include <QMutexLocker>

WaveForm::Set::Set()
{
    for(int b = MAX_BANK - MIN_BANK; b >= 0; --b)
        for(int i = MAX_INDEX - MIN_INDEX; i >= 0; --i)
            m_stock[b][i] = NULL;

    int BANK;

    // Basic #0
    // 107 is reserved
    BANK              = 0;
    m_bankNames[BANK] = "Basic";
    new WaveForm /*PULSE*/ ("Pulse", BANK, 8, pulsef, Exact);
    new WaveForm /*CBPEAK*/ ("Cb peak", BANK, 15, cbpeakf, Linear);
    new WaveForm /*MOOGSAW*/ ("Moog saw", BANK, 52, moogsawf, Linear);
    new WaveForm /*MOOGSQUARE*/ ("Moog square", BANK, 53, moogsquaref,
                                 Linear);
    new WaveForm /*OCTAVIUSSAW*/ ("Octavius saw", BANK, 62, octaviussawf,
                                  Linear);
    new WaveForm /*EXPSAW*/ ("Exponential saw", BANK, 82, expsawf, Linear);

    // Adjusted
    // 0a/0p versions (minimize the volume at the end and the beginning.
    // 107 is reserved
    BANK              = 1;
    m_bankNames[BANK] = "Adjusted";
    new WaveForm /*SAWTOOTH0P*/ ("Saw tooth 0p", BANK, 2, sawtooth0pf, Exact);
    new WaveForm /*HARSHSAW0P*/ ("Harsh saw 0p", BANK, 4, harshsaw0pf, Exact);
    new WaveForm /*SQPEAK0P*/ ("Sq peak 0p", BANK, 5, sqpeak0pf, Linear);
    new WaveForm /*PULSE0P*/ ("Pulse 0p", BANK, 8, pulse0pf, Exact);
    new WaveForm /*CBPEAK0P*/ ("Cb peak 0p", BANK, 15, cbpeak0pf, Linear);
    new WaveForm /*MOOGSAW0P*/ ("Moog saw 0p", BANK, 52, moogsaw0pf, Linear);
    new WaveForm /*OCTAVIUSSAW0P*/ ("Octavius saw 0p", BANK, 62,
                                    octaviussaw0pf, Linear);

    // Centered (2)
    // Max energy at the center
    BANK              = 2;
    m_bankNames[BANK] = "Centered";

    // Soften
    BANK              = 5;
    m_bankNames[BANK] = "Soften";
    createSoften(5, 0.02);
    createSoften(6, 0.05);
    createSoften(7, 0.10);
    createSoften(8, 0.20);
    createSoften(9, 0.40);

    // Degraded
    createDegraded(10, false, 0);
    createDegraded(11, false, 1);
    createDegraded(12, false, 2);
    createDegraded(13, false, 3);
    createDegraded(14, false, 4);
    createDegraded(15, true, 0);
    createDegraded(16, true, 1);
    createDegraded(17, true, 2);
    createDegraded(18, true, 3);
    createDegraded(19, true, 4);

    // Constant
    BANK              = 20;
    m_bankNames[BANK] = "Constant";
    new WaveForm /*MINUS1*/ ("-1.0", BANK, 0, minus1f, Exact);
    new WaveForm /*MINUS05*/ ("-0.5", BANK, 20, minus05f, Exact);
    new WaveForm /*PLUS05*/ ("+0.5f", BANK, 60, plus05f, Exact);
    new WaveForm /*PLUS1*/ ("+1.0", BANK, 80, plus1f, Exact);

    // Mathematical
    BANK              = 21;
    m_bankNames[BANK] = "Mathematical";
    new WaveForm /*ID*/ ("id()", BANK, 1, identityf, Exact);
    new WaveForm /*SQ*/ ("sq()", BANK, 2, sqf, Exact);
    new WaveForm /*CB*/ ("cb()", BANK, 3, cbf, Exact);
    new WaveForm /*CBRT*/ ("cbrt()", BANK, 13, cbrtf, Linear);
    new WaveForm /*NEXP*/ ("n_exp()", BANK, 20, nexpf, Linear);
    new WaveForm /*NLOG*/ ("n_log()", BANK, 30, nlogf, Linear);

    int  sbank = 35;
    QDir wfrd("../../../lmms/waveforms", "*", QDir::Name | QDir::IgnoreCase,
              QDir::AllDirs | QDir::NoDotAndDotDot);
    for(QString& wfb : wfrd.entryList())
    {
        QDir wfbd(wfrd.absolutePath() + "/" + wfb, "*.wav",
                  QDir::Name | QDir::IgnoreCase,
                  QDir::Files | QDir::NoDotAndDotDot);

        QString bankname = wfb;
        bankname.replace(QRegExp("^AKWF_"), "");
        bankname.replace('_', ' ');
        m_bankNames[sbank - MIN_BANK] = bankname.trimmed();

        int sindex = MIN_INDEX;
        for(QString& wff : wfbd.entryList())
        {
            if(sindex > MAX_INDEX)
                continue;
            // qInfo("%s : %s", qPrintable(wff),
            //      qPrintable(wfbd.absolutePath() + "/" + wff));
            /*
            SampleBuffer* sb = new SampleBuffer(
                    wfbd.absolutePath() + "/" + wff, false, false);

            QString wavename = wff;
            wavename.replace(QRegExp("^AKWF_"), "");
            wavename.replace(QRegExp("[.]wav$", Qt::CaseInsensitive), "");
            wavename.replace('_', ' ');

            new WaveForm(qPrintable(wavename.trimmed()), sbank, sindex,
                         sb->data(), sb->frames(), Linear);
            delete sb;
            */
            QString filename = wfbd.absolutePath() + "/" + wff;
            QString wavename = wff;
            wavename.replace(QRegExp("^AKWF_"), "");
            wavename.replace(QRegExp("[.]wav$", Qt::CaseInsensitive), "");
            wavename.replace('_', ' ');

            new WaveForm(qPrintable(wavename.trimmed()), sbank, sindex,
                         filename, Linear);

            sindex++;
        }
        sbank++;
    }
}

void WaveForm::Set::createDegraded(int _bank, bool _linear, int _quality)
{
    const int BANK = _bank;
    QString   cat  = QString("Q%1%2").arg(_quality).arg(_linear ? "L" : "D");
    interpolation_t i = (_linear ? Linear : Discrete);

    m_bankNames[BANK] = QString("%1 Degraded").arg(cat);
    new WaveForm(QString("Sine %1").arg(cat), BANK, 0, nsinf, i, _quality);
    new WaveForm(QString("Triangle %1").arg(cat), BANK, 1, trianglef, i,
                 _quality);
    new WaveForm(QString("Saw tooth %1").arg(cat), BANK, 2, sawtoothf, i,
                 _quality);
    new WaveForm(QString("Square %1").arg(cat), BANK, 3, squaref, i,
                 _quality);
    new WaveForm(QString("Harsh saw %1").arg(cat), BANK, 4, harshsawf, i,
                 _quality);
    new WaveForm(QString("Sq peak %1").arg(cat), BANK, 5, sqpeakf, i,
                 _quality);
    new WaveForm(QString("White noise %1").arg(cat), BANK, 6, randf, i,
                 _quality);
    new WaveForm(QString("Pulse %1").arg(cat), BANK, 8, pulsef, i, _quality);
    new WaveForm(QString("Cb peak %1").arg(cat), BANK, 15, cbpeakf, i,
                 _quality);
    new WaveForm(QString("Moog saw %1").arg(cat), BANK, 52, moogsawf, i,
                 _quality);
    new WaveForm(QString("Moog square %1").arg(cat), BANK, 53, moogsquaref, i,
                 _quality);
    new WaveForm(QString("Octavius saw %1").arg(cat), BANK, 62, octaviussawf,
                 i, _quality);
    new WaveForm(QString("Exponential saw %1").arg(cat), BANK, 82, expsawf, i,
                 _quality);
}

void WaveForm::Set::createSoften(int _bank, float _softness)
{
    const int       BANK = _bank;
    QString         cat  = QString("S%1").arg(int(_softness * 100));
    interpolation_t i    = Linear;
    int             q    = 7;

    m_bankNames[BANK] = QString("%1 Soften").arg(cat);
    (new WaveForm(QString("Sine %1").arg(cat), BANK, 0, nsinf, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Triangle %1").arg(cat), BANK, 1, trianglef, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Saw tooth %1").arg(cat), BANK, 2, sawtoothf, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Square %1").arg(cat), BANK, 3, squaref, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Harsh saw %1").arg(cat), BANK, 4, harshsawf, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Sq peak %1").arg(cat), BANK, 5, sqpeakf, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("White noise %1").arg(cat), BANK, 6, randf, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Pulse %1").arg(cat), BANK, 8, pulsef, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Cb peak %1").arg(cat), BANK, 15, cbpeakf, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Moog saw %1").arg(cat), BANK, 52, moogsawf, i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Moog square %1").arg(cat), BANK, 53, moogsquaref,
                  i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Octavius saw %1").arg(cat), BANK, 62, octaviussawf,
                  i, q))
            ->setSoftness(_softness);
    (new WaveForm(QString("Exponential saw %1").arg(cat), BANK, 82, expsawf,
                  i, q))
            ->setSoftness(_softness);
}

WaveForm::Set WaveForm::WAVEFORMS;

// basic waveforms
const WaveForm WaveForm::SINE("Sine", 0, 0, nsinf, Linear, 10);
const WaveForm WaveForm::TRIANGLE("Triangle", 0, 1, trianglef, Exact);
const WaveForm WaveForm::SAWTOOTH("Saw tooth", 0, 2, sawtoothf, Exact);
const WaveForm WaveForm::SQUARE("Square", 0, 3, squaref, Exact);
const WaveForm WaveForm::HARSHSAW("Harsh saw", 0, 4, harshsawf, Exact);
const WaveForm WaveForm::SQPEAK("Sq peak", 0, 5, sqpeakf, Linear, 10);
const WaveForm WaveForm::WHITENOISE("White noise", 0, 6, randf, Discrete, 10);
// 7 is reserved
const WaveForm WaveForm::ZERO(" 0.0", ZERO_BANK, ZERO_INDEX, zerof, Exact);
const WaveForm WaveForm::SQRT("sqrt()", 20, 12, sqrtf, Linear, 10);

const wavefunction_t WaveForm::sine
        = [](const float _x) { return WaveForm::SINE.f(_x); };
const wavefunction_t WaveForm::triangle
        = [](const float _x) { return WaveForm::TRIANGLE.f(_x); };
const wavefunction_t WaveForm::sawtooth
        = [](const float _x) { return WaveForm::SAWTOOTH.f(_x); };
const wavefunction_t WaveForm::square
        = [](const float _x) { return WaveForm::SQUARE.f(_x); };
const wavefunction_t WaveForm::harshsaw
        = [](const float _x) { return WaveForm::HARSHSAW.f(_x); };
const wavefunction_t WaveForm::sqpeak
        = [](const float _x) { return WaveForm::SQPEAK.f(_x); };
const wavefunction_t WaveForm::whitenoise
        = [](const float _x) { return WaveForm::WHITENOISE.f(_x); };
const wavefunction_t WaveForm::sqrt
        = [](const float _x) { return WaveForm::SQRT.f(_x); };

const WaveForm* WaveForm::Set::get(const int _bank, const int _index)
{
    const int b = _bank - MIN_BANK;
    const int i = _index - MIN_INDEX;
    if(_bank < MIN_BANK || _bank > MAX_BANK || _index < MIN_INDEX
       || _index > MAX_INDEX || m_stock[b][i] == NULL)
        return m_stock[ZERO_BANK - MIN_BANK][ZERO_INDEX - MIN_INDEX];

    return m_stock[b][i];
}

void WaveForm::Set::set(const int       _bank,
                        const int       _index,
                        const WaveForm* _wf)
{
    const int b = _bank - MIN_BANK;
    const int i = _index - MIN_INDEX;
    if(m_stock[b][i] == _wf)
        return;
    if(m_stock[b][i] != NULL)
        qWarning("Warning: replacing waveform[%d][%d]", b, i);
    m_stock[b][i] = _wf;
}

void WaveForm::Set::fillBankModel(ComboBoxModel& _model)
{
    _model.setDisplayName(QString("Banks"));
    _model.clear();
    for(int b = MIN_BANK; b <= MAX_BANK; b++)
    {
        QString text("--");
        if(m_bankNames[b - MIN_BANK] != "")
            text = QString("%1")  //("%1 %2")
                                  //.arg(b, 2, 16, QChar('0'))
                           .arg(m_bankNames[b - MIN_BANK])
                           .trimmed();
        _model.addItem(text, NULL, b);
    }
}

void WaveForm::Set::fillIndexModel(ComboBoxModel& _model, const int _bank)
{
    // BACKTRACE
    _model.setDisplayName(QString("Bank %1").arg(_bank));
    _model.clear();
    for(int i = MIN_INDEX; i <= MAX_INDEX; i++)
    {
        QString         text("--");
        const WaveForm* wf = get(_bank, i);
        if(wf != &ZERO || (_bank == ZERO_BANK && i == ZERO_INDEX))
            text = QString("%1")  //("%1 %2")
                                  //.arg(i, 2, 16, QChar('0'))
                           .arg(wf->name())
                           .trimmed();
        _model.addItem(text, NULL, i);
    }
}

const WaveForm* WaveForm::get(const int _bank, const int _index)
{
    return WAVEFORMS.get(_bank, _index);
}

void WaveForm::fillBankModel(ComboBoxModel& _model)
{
    WAVEFORMS.fillBankModel(_model);
}

void WaveForm::fillIndexModel(ComboBoxModel& _model, const int _bank)
{
    WAVEFORMS.fillIndexModel(_model, _bank);
}

WaveForm::WaveForm(const QString&        _name,
                   const int             _bank,
                   const int             _index,
                   const interpolation_t _mode,
                   const int             _quality) :
      m_built(false),
      m_name(_name), m_bank(_bank), m_index(_index), m_mode(_mode),
      m_quality(_quality)
{
    m_func = NULL;
    m_file = "";
    m_data = NULL;
    m_size = -1;

    WAVEFORMS.set(_bank, _index, static_cast<const WaveForm*>(this));
}

WaveForm::WaveForm(const QString&        _name,
                   const int             _bank,
                   const int             _index,
                   const wavefunction_t  _func,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _bank, _index, _mode, _quality)
{
    m_func = _func;
    if(m_mode == Exact)
        m_built = true;
}

WaveForm::WaveForm(const QString&        _name,
                   const int             _bank,
                   const int             _index,
                   const QString&        _file,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _bank, _index, _mode, _quality)
{
    m_file = _file;
    if(m_mode == Exact)
        m_mode = Linear;
}

WaveForm::WaveForm(const QString&        _name,
                   const int             _bank,
                   const int             _index,
                   float*                _data,
                   const int             _size,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _bank, _index, _mode, _quality)
{
    m_data = MM_ALLOC(float, _size);
    for(int f = 0; f < _size; ++f)
        m_data[f] = _data[f];
    m_size = _size - 1;
    if(m_mode == Exact)
        m_mode = Linear;
    m_built = true;
}

WaveForm::WaveForm(const QString&        _name,
                   const int             _bank,
                   const int             _index,
                   const sampleFrame*    _data,
                   const int             _size,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _bank, _index, _mode, _quality)
{
    m_data = MM_ALLOC(float, _size);
    for(int f = 0; f < _size; ++f)
        m_data[f] = _data[f][0];
    m_size = _size - 1;
    if(m_mode == Exact)
        m_mode = Linear;
    m_built = true;
}

WaveForm::~WaveForm()
{
    if(m_data != NULL)
        MM_FREE(m_data);
}

WaveForm* WaveForm::setSoftness(float _softness)
{
    m_softness = _softness;
    if(m_built)
        rebuild();
    return this;
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
        m_size = int(ceilf(8.f * pow(2.94283095638f, m_quality))) - 1;
        if(m_data != nullptr)
            MM_FREE(m_data);
        m_data = MM_ALLOC(float, m_size + 1);
        for(int i = m_size; i >= 0; --i)
            m_data[i] = m_func(float(i) / float(m_size));
        m_built = true;
    }
    else if(m_file != "")
    {
        SampleBuffer*      sb   = new SampleBuffer(m_file, false, false);
        int                size = sb->frames();
        const sampleFrame* data = sb->data();

        if(m_data != nullptr)
            MM_FREE(m_data);
        m_data = MM_ALLOC(float, size);
        for(int f = 0; f < size; ++f)
            m_data[f] = data[f][0];
        m_size = size - 1;
        delete sb;
        m_built = true;
    }
    else
    {
        qWarning("Error: waveform not built: %s", qPrintable(m_name));
        return;
    }

    if(m_softness != 0.f)
        soften();
}

void WaveForm::soften()
{
    const int size = m_size + 1;
    const int bw   = 0.5f * m_softness * m_size;
    if(bw == 0)
        return;

    float v0 = 0.f;
    for(int i = -bw; i <= bw; ++i)
    {
        const int f = (i + size) % size;
        v0 += m_data[f];
    }
    const float n = 1.f / float(1 + 2 * bw);
    v0 *= n;

    float omin = m_data[0];
    float omax = m_data[0];
    float nmin = v0;
    float nmax = v0;

    float* data = MM_ALLOC(float, size);
    data[0]     = v0;
    for(int f = 1; f < size; ++f)
    {
        const float ov = m_data[f];
        if(ov < omin)
            omin = ov;
        if(ov > omax)
            omax = ov;

        const float nv = data[f - 1]
                         + (m_data[(f + bw) % size]
                            - m_data[(f - bw - 1 + size) % size])
                                   * n;
        if(nv < nmin)
            nmin = nv;
        if(nv > nmax)
            nmax = nv;

        data[f] = nv;
    }

    float os = qMax(fabsf(omax), fabsf(omin));
    float ns = qMax(fabsf(nmax), fabsf(nmin));
    if(ns >= SILENCE)
    {
        const float k = os / ns;
        if(k != 1.f)
            for(int f = 0; f < size; ++f)
                data[f] *= k;
    }

    float* old = m_data;
    m_data     = data;
    MM_FREE(old);
}

// x must be between 0. and 1.
float WaveForm::f(const float _x) const
{
    return f(_x, m_mode);
}

float WaveForm::f(const float _x, const float _antialias) const
{
    return f(_x, _antialias, m_mode);
}

// x must be between 0. and 1.
float WaveForm::f(const float           _x,
                  const float           _antialias,
                  const interpolation_t _m) const
{
    float r = 0.f;

    if(_antialias > 0.f)
    {
        // qInfo("antialias f: %f", _antialias);
        const int N = 13;
        for(int i = -N; i <= N; i++)
            r += f(fraction(_x + 1.f + _antialias * i / (N + 1)), _m);
        r /= (2 * N + 1);
        /*
        const float d  = 2.f / Engine::mixer()->processingSampleRate();
        const float r0 = f(fraction(_x + 1.f - 3.f * d), false, _m);
        const float r1 = f(fraction(_x + 1.f - d), false, _m);
        const float rx = f(fraction(_x), false, _m);
        const float r2 = f(fraction(_x + d), false, _m);
        const float r3 = f(fraction(_x + 3.f * d), false, _m);
        return rx + (r2 - rx) * d / sqrtf(sqf(d) + sqf(r2 - rx))
               + (rx - r1) * d / sqrtf(sqf(d) + sqf(rx - r1))
               + (r3 - rx) * 3.f * d / sqrtf(sqf(3.f * d) + sqf(r3 - rx))
               + (rx - r0) * 3.f * d / sqrtf(sqf(3.f * d) + sqf(rx - r1));
        */
        // optimal4pInterpolate(r0, r1, r2, r3, 0.5f);
    }
    else
    {
        r = f(_x, _m);
    }

    return r;
}

// x must be between 0. and 1.
float WaveForm::f(const float _x, const interpolation_t _m) const
{
    if(!m_built)
        const_cast<WaveForm*>(this)->build();

    interpolation_t m = _m;
    if((m != m_mode) && (m == Exact || m_mode == Exact))
        m = m_mode;

    float r = 0.f;
    switch(m_mode)
    {
        case Discrete:
            r = m_data[int(_x * m_size)];
            break;
        case Rounded:
        {
            int i = roundf(_x * m_size);
            r     = m_data[i];
        }
        break;
        case Linear:
        case Cosinus:
        case Optimal2:
        {
            const float j  = _x * m_size;
            const int   i  = int(j);
            const float d  = j - i;
            const float r0 = m_data[i];
            const float r1 = m_data[i + 1];
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
            const float p  = _x * m_size;
            const int   i1 = int(p);
            const float d  = p - i1;
            const int   i0 = (i1 == 0 ? m_size + 1 : i1 - 1);
            const int   i2 = i1 + 1;
            const int   i3 = (i2 == m_size + 1 ? 0 : i2 + 1);
            const float r0 = m_data[i0];
            const float r1 = m_data[i1];
            const float r2 = m_data[i2];
            const float r3 = m_data[i3];

            switch(m_mode)
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
    m_normBuf = (float*)fftwf_malloc((FFT_BUFFER_SIZE * 2) *
sizeof(float));
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

WaveForm::Plan::build(WaveForm& _wf, const float _cut)
{
    QMutexLocker locker(m_mutex);

    for(int i = FFT_BUFFER_SIZE * 2 - 1; i >= 0; --i)
    {
        float x      = float(i) / float(FFT_BUFFER_SIZE * 2);
        float y      = _wf->f(x, false);
        m_normBuf[i] = y;
    }

    fftwf_execute(m_r2cPlan);

    for(int f = int(ceilf(_cut)); f <= FFT_BUFFER_SIZE; f++)
    {
        m_specBuf[f][0] = 0.f;
        m_specBuf[f][1] = 0.f;
    }

    fftwf_execute(m_c2rPlan);
}

WaveForm::Plan WaveForm::PLAN;
*/
