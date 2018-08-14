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

#include "lmms_math.h"  // REQUIRED

#include <QDir>

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
    new WaveForm /*EXPSAW*/ ("Exponential saw", BANK, 80, expsawf, Linear);

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
    new WaveForm /*MOOGSAW0P*/ ("Moog saw 0p", BANK, 50, moogsaw0pf, Linear);

    // Centered
    // Max energy at the center

    // Degraded Q1
    BANK              = 11;
    m_bankNames[BANK] = "Degraded Q1";
    new WaveForm /*SINEQ1*/ ("Sine Q1", BANK, 0, nsinf, Linear, 1);
    new WaveForm /*TRIANGLEQ1*/ ("Triangle Q1", BANK, 1, trianglef, Linear,
                                 1);
    new WaveForm /*SAWTOOTHQ1*/ ("Saw tooth Q1", BANK, 2, sawtoothf, Linear,
                                 1);
    new WaveForm /*SQUAREQ1*/ ("Square Q1", BANK, 3, squaref, Linear, 1);
    new WaveForm /*HARSHSAWQ1*/ ("Harsh saw Q1", BANK, 4, harshsawf, Linear,
                                 1);
    new WaveForm /*SQPEAKQ1*/ ("Sq peak Q1", BANK, 5, sqpeakf, Linear, 1);
    new WaveForm /*WHITENOISEQ1*/ ("White noise Q1", BANK, 6, randf, Discrete,
                                   1);
    new WaveForm /*PULSEQ1*/ ("Pulse Q1", BANK, 8, pulsef, Linear, 1);
    new WaveForm /*CBPEAKQ1*/ ("Cb peak Q1", BANK, 15, cbpeakf, Linear, 1);
    new WaveForm /*MOOGSAWQ1*/ ("Moog saw Q1", BANK, 50, moogsawf, Linear, 1);
    new WaveForm /*EXPSAWQ1*/ ("Exponential saw Q1", BANK, 80, expsawf,
                               Linear, 1);

    // Degraded Q2
    BANK              = 12;
    m_bankNames[BANK] = "Degraded Q2";
    new WaveForm /*SINEQ2*/ ("Sine Q2", BANK, 0, nsinf, Linear, 2);
    new WaveForm /*TRIANGLEQ2*/ ("Triangle Q2", BANK, 1, trianglef, Linear,
                                 2);
    new WaveForm /*SAWTOOTHQ2*/ ("Saw tooth Q2", BANK, 2, sawtoothf, Linear,
                                 2);
    new WaveForm /*SQUAREQ2*/ ("Square Q2", BANK, 3, squaref, Linear, 2);
    new WaveForm /*HARSHSAWQ2*/ ("Harsh saw Q2", BANK, 4, harshsawf, Linear,
                                 2);
    new WaveForm /*SQPEAKQ2*/ ("Sq peak Q2", BANK, 5, sqpeakf, Linear, 2);
    new WaveForm /*WHITENOISEQ2*/ ("White noise Q2", BANK, 6, randf, Discrete,
                                   2);
    new WaveForm /*PULSEQ2*/ ("Pulse Q2", BANK, 8, pulsef, Linear, 2);
    new WaveForm /*CBPEAKQ2*/ ("Cb peak Q2", BANK, 15, cbpeakf, Linear, 2);
    new WaveForm /*MOOGSAWQ2*/ ("Moog saw Q2", BANK, 50, moogsawf, Linear, 2);
    new WaveForm /*EXPSAWQ2*/ ("Exponential saw Q2", BANK, 80, expsawf,
                               Linear, 2);

    // Degraded Q3
    BANK              = 13;
    m_bankNames[BANK] = "Degraded Q3";
    new WaveForm /*SINEQ3*/ ("Sine Q3", BANK, 0, nsinf, Linear, 3);
    new WaveForm /*TRIANGLEQ3*/ ("Triangle Q3", BANK, 1, trianglef, Linear,
                                 3);
    new WaveForm /*SAWTOOTHQ3*/ ("Saw tooth Q3", BANK, 2, sawtoothf, Linear,
                                 3);
    new WaveForm /*SQUAREQ3*/ ("Square Q3", BANK, 3, squaref, Linear, 3);
    new WaveForm /*HARSHSAWQ3*/ ("Harsh saw Q3", BANK, 4, harshsawf, Linear,
                                 3);
    new WaveForm /*SQPEAKQ3*/ ("Sq peak Q3", BANK, 5, sqpeakf, Linear, 3);
    new WaveForm /*WHITENOISEQ3*/ ("White noise Q3", BANK, 6, randf, Discrete,
                                   3);
    new WaveForm /*PULSEQ3*/ ("Pulse Q3", BANK, 8, pulsef, Linear, 3);
    new WaveForm /*CBPEAKQ3*/ ("Cb peak Q3", BANK, 15, cbpeakf, Linear, 3);
    new WaveForm /*MOOGSAWQ3*/ ("Moog saw Q3", BANK, 50, moogsawf, Linear, 3);
    new WaveForm /*EXPSAWQ3*/ ("Exponential saw Q3", BANK, 80, expsawf,
                               Linear, 3);

    // Constant
    BANK              = 19;
    m_bankNames[BANK] = "Constant";
    new WaveForm /*MINUS1*/ ("-1.0", BANK, 0, minus1f, Exact);
    new WaveForm /*MINUS05*/ ("-0.5", BANK, 20, minus05f, Exact);
    new WaveForm /*PLUS05*/ ("+0.5f", BANK, 60, plus05f, Exact);
    new WaveForm /*PLUS1*/ ("+1.0", BANK, 80, plus1f, Exact);

    // Mathematical
    BANK              = 20;
    m_bankNames[BANK] = "Mathematical";
    new WaveForm /*SQ*/ ("sq ()", BANK, 0, sqf, Exact);
    new WaveForm /*CB*/ ("cb()", BANK, 1, cbf, Exact);
    new WaveForm /*CBRT*/ ("cbrt()", BANK, 11, cbrtf, Linear);
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
        m_bankNames[sbank - MIN_BANK] = wfb;
        int sindex                    = MIN_INDEX;
        for(QString& wff : wfbd.entryList())
        {
            if(sindex > MAX_INDEX)
                continue;
            qInfo("%s : %s", qPrintable(wff),
                  qPrintable(wfbd.absolutePath() + "/" + wff));
            SampleBuffer* sb = new SampleBuffer(
                    wfbd.absolutePath() + "/" + wff, false, false);
            new WaveForm(qPrintable(wff), sbank, sindex, sb->data(),
                         sb->frames(), Linear);
            delete sb;
            sindex++;
        }
        sbank++;
    }
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
const WaveForm WaveForm::SQRT("sqrt()", 20, 10, sqrtf, Linear, 10);

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
            text = QString("%1 %2")
                           .arg(b, 2, 16, QChar('0'))
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
            text = QString("%1 %2")
                           .arg(i, 2, 16, QChar('0'))
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

WaveForm::WaveForm(const char*           _name,
                   const int             _bank,
                   const int             _index,
                   const interpolation_t _mode,
                   const int             _quality) :
      m_name(_name),
      m_bank(_bank), m_index(_index), m_mode(_mode), m_quality(_quality)
{
    m_func = NULL;
    m_file = "";
    m_data = NULL;
    m_size = -1;

    WAVEFORMS.set(_bank, _index, static_cast<const WaveForm*>(this));
}

WaveForm::WaveForm(const char*           _name,
                   const int             _bank,
                   const int             _index,
                   const wavefunction_t  _func,
                   const interpolation_t _mode,
                   const int             _quality) :
      WaveForm(_name, _bank, _index, _mode, _quality)
{
    m_func = _func;
    if(m_mode != Exact)
    {
        m_size = 128 * pow(2, _quality) - 1;
        m_data = MM_ALLOC(float, m_size + 1);
        for(int i = m_size; i >= 0; --i)
            m_data[i] = m_func(float(i) / float(m_size));
    }
}

WaveForm::WaveForm(const char*           _name,
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

WaveForm::WaveForm(const char*           _name,
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
}

WaveForm::WaveForm(const char*           _name,
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
}

WaveForm::~WaveForm()
{
    if(m_data != NULL)
        MM_FREE(m_data);
}

// x must be between 0. and 1.
float WaveForm::f(const float _x) const
{
    float r;
    switch(m_mode)
    {
        case Discrete:
            r = m_data[int(_x * m_size)];
            break;
        case Linear:
        {
            const float j  = _x * m_size;
            const int   i  = int(j);
            const float d  = j - i;
            const float r0 = m_data[i];
            const float r1 = m_data[i + 1];
            r              = r0 + d * (r1 - r0);
            break;
        }
        case Exact:
            r = m_func(_x);
            break;
    }
    return r;
}
