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
#include "interpolation.h"
#include "lmms_float.h"

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
    new WaveForm("Pulse", BANK, 8, pulsef, Exact);
    new WaveForm("Cb peak", BANK, 15, cbpeakf, Linear);
    new WaveForm("Moog saw", BANK, 52, moogsawf, Linear);
    new WaveForm("Moog square", BANK, 53, moogsquaref, Linear);
    new WaveForm("Octavius saw", BANK, 62, octaviussawf, Linear);
    new WaveForm("Error saw", BANK, 72, nerf, Linear);
    new WaveForm("Exponential saw", BANK, 82, expsawf, Linear);
    new WaveForm("Exp2 saw", BANK, 83, nexp2sawf, Linear);
    new WaveForm("Sin2 saw", BANK, 90, nsin2f, Linear);
    new WaveForm("Sin4 saw", BANK, 91, nsin4f, Linear);
    new WaveForm("Inv saw", BANK, 92, ninvsawf, Linear);
    new WaveForm("Corner saw", BANK, 93, cornersawf, Linear);
    new WaveForm("Corner peak", BANK, 98, cornerpeakf, Linear);
    new WaveForm("Profil peak", BANK, 99, profilpeakf, Linear);

    // Adjusted
    // 0a/0p versions (minimize the volume at the end and the beginning.
    // 107 is reserved
    BANK              = 1;
    m_bankNames[BANK] = "Adjusted";
    new WaveForm("Saw tooth 0p", BANK, 2, sawtooth0pf, Exact);
    new WaveForm("Harsh saw 0p", BANK, 4, harshsaw0pf, Exact);
    new WaveForm("Sq peak 0p", BANK, 5, sqpeak0pf, Linear);
    new WaveForm("Pulse 0p", BANK, 8, pulse0pf, Exact);
    new WaveForm("Cb peak 0p", BANK, 15, cbpeak0pf, Linear);
    new WaveForm("Moog saw 0p", BANK, 52, moogsaw0pf, Linear);
    new WaveForm("Octavius saw 0p", BANK, 62, octaviussaw0pf, Linear);

    // Centered (2)
    // Max energy at the center
    BANK              = 2;
    m_bankNames[BANK] = "Centered";

    // Dephased (3)
    // Phase - 0.5
    BANK              = 3;
    m_bankNames[BANK] = "Dephased";

    // Reversed (4)
    // - Phase
    BANK              = 4;
    m_bankNames[BANK] = "Reversed";

    // Soften
    // BANK              = 5;
    // m_bankNames[BANK] = "Soften";
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
    new WaveForm /*PLUS05*/ ("+0.5", BANK, 60, plus05f, Exact);
    new WaveForm /*PLUS1*/ ("+1.0", BANK, 80, plus1f, Exact);

    // Mathematical
    BANK              = 21;
    m_bankNames[BANK] = "Mathematical";
    new WaveForm /*ID*/ ("id()", BANK, 1, identityf, Exact);
    new WaveForm /*SQ*/ ("sq()", BANK, 2, sqf, Exact);
    new WaveForm /*CB*/ ("cb()", BANK, 3, cbf, Exact);
    new WaveForm /*CMPL*/ ("complement()", BANK, 10, complementf, Exact);
    new WaveForm /*CBRT*/ ("cbrt()", BANK, 13, cbrtf, Linear);
    new WaveForm /*NEXP*/ ("n_exp()", BANK, 20, nexp2f, Linear);
    new WaveForm /*NLOG*/ ("n_log()", BANK, 30, nlogf, Linear);
    new WaveForm /*NGAUSS*/ ("n_gauss()", BANK, 41, ngaussf, Linear);
    // SHARPGAUSS 47
    new WaveForm("fibonacci1", BANK, 61, fibonacci1, Linear);
    new WaveForm("fibonacci2", BANK, 62, fibonacci2, Linear);
    new WaveForm /*COS*/ ("n_cos()", BANK, 80, ncosf, Linear);
    new WaveForm("n_exp2()", BANK, 83, nexp2f, Linear);
    new WaveForm /*COS2*/ ("n_cos2()", BANK, 90, ncos2f, Linear);
    new WaveForm /*COS4*/ ("n_cos4()", BANK, 91, ncos4f, Linear);
    new WaveForm /*INVDIST*/ ("d_inv()", BANK, 92, ninvdistf, Linear);
    new WaveForm /*CORNERDIST*/ ("d_corner()", BANK, 93, cornerdistf, Linear);

    // Mathematical
    BANK              = 22;
    m_bankNames[BANK] = "Statistical";

    int sbank = 35;
    {
        QDir wfrd("../../../lmms/waveforms", "AKWF_*",
                  QDir::Name | QDir::IgnoreCase,
                  QDir::Dirs | QDir::NoDotAndDotDot);
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
    sbank = 100;
    {
        QDir wfrd("../../../lmms/waveforms", "User_*",
                  QDir::Name | QDir::IgnoreCase,
                  QDir::Dirs | QDir::NoDotAndDotDot);
        for(QString& wfb : wfrd.entryList())
        {
            QDir wfbd(wfrd.absolutePath() + "/" + wfb, "*.wav",
                      QDir::Name | QDir::IgnoreCase,
                      QDir::Files | QDir::NoDotAndDotDot);

            qInfo("bank waveform '%s'", qPrintable(wfb));
            QString bankname = wfb;
            // bankname.replace(QRegExp("^AKWF_"), "");
            bankname.replace('_', ' ');
            m_bankNames[sbank - MIN_BANK] = bankname.trimmed();

            int sindex = MIN_INDEX;
            for(QString& wff : wfbd.entryList())
            {
                if(sindex > MAX_INDEX)
                    continue;
                QString filename = wfbd.absolutePath() + "/" + wff;
                QString wavename = wff;
                // wavename.replace(QRegExp("^AKWF_"), "");
                wavename.replace(QRegExp("[.]wav$", Qt::CaseInsensitive), "");
                wavename.replace('_', ' ');

                new WaveForm(qPrintable(wavename.trimmed()), sbank, sindex,
                             filename, Linear);

                sindex++;
            }
            sbank++;
        }
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

void WaveForm::Set::createSoften(int _bank, real_t _softness)
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
const WaveForm WaveForm::SQRT(
        "sqrt()", 20, 12, sqrtf, Linear, 10);  // FLOAT REQUIRED
const WaveForm
        WaveForm::SHARPGAUSS("sharpgauss()", 21, 47, sharpgaussf, Linear, 9);

const wavefunction_t WaveForm::sine
        = [](const real_t _x) { return WaveForm::SINE.f(_x); };
const wavefunction_t WaveForm::triangle
        = [](const real_t _x) { return WaveForm::TRIANGLE.f(_x); };
const wavefunction_t WaveForm::sawtooth
        = [](const real_t _x) { return WaveForm::SAWTOOTH.f(_x); };
const wavefunction_t WaveForm::square
        = [](const real_t _x) { return WaveForm::SQUARE.f(_x); };
const wavefunction_t WaveForm::harshsaw
        = [](const real_t _x) { return WaveForm::HARSHSAW.f(_x); };
const wavefunction_t WaveForm::sqpeak
        = [](const real_t _x) { return WaveForm::SQPEAK.f(_x); };
const wavefunction_t WaveForm::whitenoise
        = [](const real_t _x) { return WaveForm::WHITENOISE.f(_x); };
const wavefunction_t WaveForm::sqrt
        = [](const real_t _x) { return WaveForm::SQRT.f(_x); };

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
        qWarning("Warning: replacing waveform[%d][%d] %s %s", b, i,
                 qPrintable(m_stock[b][i]->m_name), qPrintable(_wf->m_name));
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
                   real_t*               _data,
                   const int             _size,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _bank, _index, _mode, _quality)
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
                   const int             _bank,
                   const int             _index,
                   const sampleFrame*    _data,
                   const int             _size,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _bank, _index, _mode, _quality)
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
    if(m_data != NULL)
        MM_FREE(m_data);
}

WaveForm* WaveForm::setSoftness(real_t _softness)
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
        m_size = int(ceil(8. * pow(2.94283095638, m_quality))) - 1;
        if(m_data != nullptr)
            MM_FREE(m_data);
        m_data = MM_ALLOC(real_t, m_size + 1);
        for(int i = m_size; i >= 0; --i)
            m_data[i] = m_func(real_t(i) / real_t(m_size));
        m_built = true;
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
        m_built = true;
    }
    else
    {
        qWarning("Error: waveform not built: %s", qPrintable(m_name));
        BACKTRACE
        return;
    }

    if(m_softness != 0.)
        soften();
}

void WaveForm::soften()
{
    const int size = m_size + 1;
    const int bw   = 0.5 * m_softness * m_size;
    if(bw == 0)
        return;

    real_t v0 = 0.;
    for(int i = -bw; i <= bw; ++i)
    {
        const int f = (i + size) % size;
        v0 += m_data[f];
    }
    const real_t n = 1. / real_t(1 + 2 * bw);
    v0 *= n;

    real_t omin = m_data[0];
    real_t omax = m_data[0];
    real_t nmin = v0;
    real_t nmax = v0;

    real_t* data = MM_ALLOC(real_t, size);
    data[0]      = v0;
    for(int f = 1; f < size; ++f)
    {
        const real_t ov = m_data[f];
        if(ov < omin)
            omin = ov;
        if(ov > omax)
            omax = ov;

        const real_t nv = data[f - 1]
                          + (m_data[(f + bw) % size]
                             - m_data[(f - bw - 1 + size) % size])
                                    * n;
        if(nv < nmin)
            nmin = nv;
        if(nv > nmax)
            nmax = nv;

        data[f] = nv;
    }

    real_t os = qMax(abs(omax), abs(omin));
    real_t ns = qMax(abs(nmax), abs(nmin));
    if(ns >= SILENCE)
    {
        const real_t k = os / ns;
        if(k != 1.)
            for(int f = 0; f < size; ++f)
                data[f] *= k;
    }

    real_t* old = m_data;
    m_data      = data;
    MM_FREE(old);
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
            r = m_data[int(_x * m_size)];
            break;
        case Rounded:
        {
            int i = round(_x * m_size);
            r     = m_data[i];
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
                qInfo("WaveForm::f x=%f m_data=%p i=%d size=%d", _x, m_data,
                      i, m_size);
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
