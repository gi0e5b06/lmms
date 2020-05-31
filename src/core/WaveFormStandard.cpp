/*
 * WaveFormStandard.cpp -
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
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

#include "WaveFormStandard.h"

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

WaveFormStandard::Set::Set()
{
    for(int b = MAX_BANK - MIN_BANK; b >= 0; --b)
        for(int i = MAX_INDEX - MIN_INDEX; i >= 0; --i)
            m_stock[b][i] = nullptr;

    int BANK;

    // Basic #0
    // 107 is reserved
    BANK              = 0;
    m_bankNames[BANK] = "Basic";
    new WaveFormStandard("Sine", 0, 0, nsinf, Linear, 10);
    new WaveFormStandard("Triangle", 0, 1, trianglef, Exact);
    new WaveFormStandard("Ramp", 0, 2, rampf, Exact);
    new WaveFormStandard("Square", 0, 3, squaref, Exact);
    new WaveFormStandard("Harsh ramp", 0, 4, harshrampf, Exact);
    new WaveFormStandard("Sq peak", 0, 5, sqpeakf, Linear, 10);
    new WaveFormStandard("White noise", 0, 6, randf, Discrete, 10);
    // 7 is reserved
    new WaveFormStandard("Pulse", BANK, 8, pulsef, Exact);
    new WaveFormStandard("Exponential triangle", BANK, 11, exptrif, Linear);
    new WaveFormStandard("Sawtooth", BANK, 12, sawtoothf, Exact);
    new WaveFormStandard("Moog square", BANK, 13, moogsquaref, Linear);

    // tri

    // ramp
    new WaveFormStandard("Moog ramp", BANK, 50, moogrampf, Linear);
    new WaveFormStandard("Octavius ramp", BANK, 51, octaviusrampf, Linear);
    new WaveFormStandard("Error ramp", BANK, 52, nerf, Linear);
    new WaveFormStandard("Exp2 ramp", BANK, 53, nexp2rampf, Linear);
    new WaveFormStandard("Sin2 ramp", BANK, 54, nsin2f, Linear);
    new WaveFormStandard("Sin4 ramp", BANK, 55, nsin4f, Linear);
    new WaveFormStandard("Inv ramp", BANK, 56, ninvrampf, Linear);
    new WaveFormStandard("Corner ramp", BANK, 57, cornerrampf, Linear);

    // saw
    /*
      (new WaveFormStandard("Moog saw", BANK, 70, moogrampf, Linear))
      ->setReverse(true);
      created by createMissing.
    */

    // square

    // peak
    new WaveFormStandard("Corner peak", BANK, 117, cornerpeakf, Linear);
    new WaveFormStandard("Cb peak", BANK, 118, cbpeakf, Linear);
    new WaveFormStandard("Profil peak", BANK, 119, profilpeakf, Linear);

    createMissing();

    // Adjusted
    // 0a/0p versions (minimize the volume at the end and the beginning.
    // 107 is reserved
    BANK              = 1;
    m_bankNames[BANK] = "Adjusted";
    createZeroed(BANK);
    /*
    new WaveFormStandard("Ramp 0p", BANK, 2, ramp0pf, Exact);
    new WaveFormStandard("Harsh ramp 0p", BANK, 4, harshramp0pf, Exact);
    new WaveFormStandard("Sq peak 0p", BANK, 5, sqpeak0pf, Linear);
    new WaveFormStandard("Pulse 0p", BANK, 8, pulse0pf, Exact);
    new WaveFormStandard("Sawtooth 0p", BANK, 12, sawtooth0pf, Exact);

    new WaveFormStandard("Moog ramp 0p", BANK, 50, moogramp0pf, Linear);
    new WaveFormStandard("Octavius ramp 0p", BANK, 51, octaviusramp0pf,
                         Linear);

    (new WaveFormStandard("Moog saw 0p", BANK, 70, moogramp0pf, Linear))
            ->setReverse(true);
    (new WaveFormStandard("Octavius saw 0p", BANK, 71, octaviusramp0pf,
                          Linear))
            ->setReverse(true);
    new WaveFormStandard("Cb peak 0p", BANK, 128, cbpeak0pf, Linear);
    */

    // Centered (2)
    // Max energy at the center
    BANK              = 2;
    m_bankNames[BANK] = "Centered";
    createCentered(BANK);

    // Dephased (3)
    // Phase - 0.5
    BANK              = 3;
    m_bankNames[BANK] = "Complement";
    createComplement(BANK);

    // Reversed (4)
    // - Phase
    BANK              = 4;
    m_bankNames[BANK] = "Reverse";
    createReverse(BANK);

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
    new WaveFormStandard /*MINUS1*/ ("-1.0", BANK, 0, minus1f, Exact);
    new WaveFormStandard /*MINUS05*/ ("-0.5", BANK, 20, minus05f, Exact);
    new WaveFormStandard /*ZERO*/ (" 0.0", BANK, 40, zerof, Exact);
    new WaveFormStandard /*PLUS05*/ ("+0.5", BANK, 60, plus05f, Exact);
    new WaveFormStandard /*PLUS1*/ ("+1.0", BANK, 80, plus1f, Exact);

    // Mathematical
    BANK              = 21;
    m_bankNames[BANK] = "Mathematical";
    new WaveFormStandard /*ID*/ ("id()", BANK, 1, identityf, Exact);
    new WaveFormStandard /*SQ*/ ("sq()", BANK, 2, sqf, Exact);
    new WaveFormStandard /*CB*/ ("cb()", BANK, 3, cbf, Exact);
    new WaveFormStandard /*CMPL*/ ("complement()", BANK, 10, complementf,
                                   Exact);
    new WaveFormStandard("sqrt()", BANK, 12, sqrtf, Linear,
                         10);  // FLOAT REQUIRED
    new WaveFormStandard /*CBRT*/ ("cbrt()", BANK, 13, cbrtf, Linear);
    new WaveFormStandard /*NEXP*/ ("n_exp()", BANK, 20, nexp2f, Linear);
    new WaveFormStandard /*NLOG*/ ("n_log()", BANK, 30, nlogf, Linear);
    new WaveFormStandard /*NGAUSS*/ ("n_gauss()", BANK, 41, ngaussf, Linear);
    new WaveFormStandard("sharpgauss()", BANK, 47, sharpgaussf, Linear, 9);
    new WaveFormStandard("fibonacci1", BANK, 61, fibonacci1, Linear);
    new WaveFormStandard("fibonacci2", BANK, 62, fibonacci2, Linear);
    new WaveFormStandard /*COS*/ ("n_cos()", BANK, 80, ncosf, Linear);
    new WaveFormStandard("n_exp2()", BANK, 83, nexp2f, Linear);
    new WaveFormStandard /*COS2*/ ("n_cos2()", BANK, 90, ncos2f, Linear);
    new WaveFormStandard /*COS4*/ ("n_cos4()", BANK, 91, ncos4f, Linear);
    new WaveFormStandard /*INVDIST*/ ("d_inv()", BANK, 92, ninvdistf, Linear);
    new WaveFormStandard /*CORNERDIST*/ ("d_corner()", BANK, 93, cornerdistf,
                                         Linear);

    // Statistical
    //BANK              = 22;
    //m_bankNames[BANK] = "Statistical";

    // Lib AKWF
    int sbank = 35;
    {
        QDir wfrd("../../../lmms/waveforms", "AKWF_*",
                  QDir::Name | QDir::IgnoreCase,
                  QDir::Dirs | QDir::NoDotAndDotDot);
        for(QString& wfb: wfrd.entryList())
        {
            QDir wfbd(wfrd.absolutePath() + "/" + wfb, "*.wav",
                      QDir::Name | QDir::IgnoreCase,
                      QDir::Files | QDir::NoDotAndDotDot);

            QString bankname = wfb;
            bankname.replace(QRegExp("^AKWF_"), "");
            bankname.replace('_', ' ');
            m_bankNames[sbank - MIN_BANK] = bankname.trimmed();

            int sindex = MIN_INDEX;
            for(QString& wff: wfbd.entryList())
            {
                if(sindex > MAX_INDEX)
                    continue;
                QString filename = wfbd.absolutePath() + "/" + wff;
                QString wavename = wff;
                wavename.replace(QRegExp("^AKWF_"), "");
                wavename.replace(QRegExp("[.]wav$", Qt::CaseInsensitive), "");
                wavename.replace('_', ' ');

                new WaveFormStandard(qPrintable(wavename.trimmed()), sbank,
                                     sindex, filename, Linear);

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
        for(QString& wfb: wfrd.entryList())
        {
            QDir wfbd(wfrd.absolutePath() + "/" + wfb, "*.wav",
                      QDir::Name | QDir::IgnoreCase,
                      QDir::Files | QDir::NoDotAndDotDot);

            // qInfo("bank waveform '%s'", qPrintable(wfb));
            QString bankname = wfb;
            // bankname.replace(QRegExp("^AKWF_"), "");
            bankname.replace('_', ' ');
            m_bankNames[sbank - MIN_BANK] = bankname.trimmed();

            int sindex = MIN_INDEX;
            for(QString& wff: wfbd.entryList())
            {
                if(sindex > MAX_INDEX)
                    continue;
                QString filename = wfbd.absolutePath() + "/" + wff;
                QString wavename = wff;
                // wavename.replace(QRegExp("^AKWF_"), "");
                wavename.replace(QRegExp("[.]wav$", Qt::CaseInsensitive), "");
                wavename.replace('_', ' ');

                new WaveFormStandard(qPrintable(wavename.trimmed()), sbank,
                                     sindex, filename, Linear);

                sindex++;
            }
            sbank++;
        }
    }
}

void WaveFormStandard::Set::createMissing()
{
    for(int i = 50; i < 69; i++)
    {
        const WaveFormStandard* ramp = get(0, i);
        if(ramp == nullptr || ramp == get(ZERO_BANK, ZERO_INDEX))
            continue;
        // if(get(0,i+20)!=nullptr) continue;
        QString s = ramp->name();
        s.replace(" ramp", " saw");
        (new WaveFormStandard(s, 0, i + 20, ramp))->setReverse(true);
    }
}

WaveFormStandard::Set::~Set()
{
    qInfo("WaveFormStandard::Set::~Set before");
    for(int b = MAX_BANK - MIN_BANK; b >= 0; --b)
        for(int i = MAX_INDEX - MIN_INDEX; i >= 0; --i)
            if(m_stock[b][i] != nullptr)
            {
                delete m_stock[b][i];
                m_stock[b][i] = nullptr;
            }
    qInfo("WaveFormStandard::Set::~Set after");
}

void WaveFormStandard::Set::createZeroed(int _bank)
{
    for(int i = MIN_INDEX; i <= MAX_INDEX; i++)
    {
        const WaveFormStandard* w = get(0, i);
        if(w == nullptr || w == get(ZERO_BANK, ZERO_INDEX) || w->zeroed())
            continue;
        // if(get(_bank,i)!=nullptr) continue;
        QString s = w->name() + " PH0";
        (new WaveFormStandard(s, _bank, i, w))->setZeroed(true);
    }
}

void WaveFormStandard::Set::createCentered(int _bank)
{
    for(int i = MIN_INDEX; i <= MAX_INDEX; i++)
    {
        const WaveFormStandard* w = get(0, i);
        if(w == nullptr || w == get(ZERO_BANK, ZERO_INDEX) || w->centered())
            continue;
        // if(get(_bank,i)!=nullptr) continue;
        QString s = w->name() + " CTR";
        (new WaveFormStandard(s, _bank, i, w))->setCentered(true);
    }
}

void WaveFormStandard::Set::createComplement(int _bank)
{
    for(int i = MIN_INDEX; i <= MAX_INDEX; i++)
    {
        const WaveFormStandard* w = get(0, i);
        if(w == nullptr || w == get(ZERO_BANK, ZERO_INDEX) || w->complement())
            continue;
        // if(get(_bank,i)!=nullptr) continue;
        QString s = w->name() + " CPL";
        (new WaveFormStandard(s, _bank, i, w))->setComplement(true);
    }
}

void WaveFormStandard::Set::createReverse(int _bank)
{
    for(int i = MIN_INDEX; i <= MAX_INDEX; i++)
    {
        const WaveFormStandard* w = get(0, i);
        if(w == nullptr || w == get(ZERO_BANK, ZERO_INDEX) || w->reverse())
            continue;
        // if(get(_bank,i)!=nullptr) continue;
        QString s = w->name() + " REV";
        (new WaveFormStandard(s, _bank, i, w))->setReverse(true);
    }
}

void WaveFormStandard::Set::createDegraded(int  _bank,
                                           bool _linear,
                                           int  _quality)
{
    const int BANK = _bank;
    QString   cat  = QString("Q%1%2").arg(_quality).arg(_linear ? "L" : "D");
    interpolation_t i = (_linear ? Linear : Discrete);

    m_bankNames[BANK] = QString("%1 Degraded").arg(cat);
    new WaveFormStandard(QString("Sine %1").arg(cat), BANK, 0, nsinf, i,
                         _quality);
    new WaveFormStandard(QString("Triangle %1").arg(cat), BANK, 1, trianglef,
                         i, _quality);
    new WaveFormStandard(QString("Ramp %1").arg(cat), BANK, 2, rampf, i,
                         _quality);
    new WaveFormStandard(QString("Square %1").arg(cat), BANK, 3, squaref, i,
                         _quality);
    new WaveFormStandard(QString("Harsh ramp %1").arg(cat), BANK, 4,
                         harshrampf, i, _quality);
    new WaveFormStandard(QString("Sq peak %1").arg(cat), BANK, 5, sqpeakf, i,
                         _quality);
    new WaveFormStandard(QString("White noise %1").arg(cat), BANK, 6, randf,
                         i, _quality);
    new WaveFormStandard(QString("Pulse %1").arg(cat), BANK, 8, pulsef, i,
                         _quality);
    new WaveFormStandard(QString("Sawtooth %1").arg(cat), BANK, 12, sawtoothf,
                         i, _quality);
    new WaveFormStandard(QString("Cb peak %1").arg(cat), BANK, 15, cbpeakf, i,
                         _quality);
    new WaveFormStandard(QString("Moog ramp %1").arg(cat), BANK, 52,
                         moogrampf, i, _quality);
    new WaveFormStandard(QString("Moog square %1").arg(cat), BANK, 53,
                         moogsquaref, i, _quality);
    (new WaveFormStandard(QString("Moog saw %1").arg(cat), BANK, 54,
                          moogrampf, i, _quality))
            ->setReverse(true);
    new WaveFormStandard(QString("Octavius ramp %1").arg(cat), BANK, 62,
                         octaviusrampf, i, _quality);
    (new WaveFormStandard(QString("Octavius saw %1").arg(cat), BANK, 64,
                          octaviusrampf, i, _quality))
            ->setReverse(true);
    new WaveFormStandard(QString("Exponential triangle %1").arg(cat), BANK,
                         82, exptrif, i, _quality);
}

void WaveFormStandard::Set::createSoften(int _bank, real_t _softness)
{
    const int       BANK = _bank;
    QString         cat  = QString("S%1").arg(int(_softness * 100));
    interpolation_t i    = Linear;
    int             q    = 7;

    m_bankNames[BANK] = QString("%1 Soften").arg(cat);
    (new WaveFormStandard(QString("Sine %1").arg(cat), BANK, 0, nsinf, i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Triangle %1").arg(cat), BANK, 1, trianglef,
                          i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Ramp %1").arg(cat), BANK, 2, rampf, i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Square %1").arg(cat), BANK, 3, squaref, i,
                          q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Harsh ramp %1").arg(cat), BANK, 4,
                          harshrampf, i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Sq peak %1").arg(cat), BANK, 5, sqpeakf, i,
                          q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("White noise %1").arg(cat), BANK, 6, randf,
                          i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Pulse %1").arg(cat), BANK, 8, pulsef, i,
                          q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Sawtooth %1").arg(cat), BANK, 12,
                          sawtoothf, i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Cb peak %1").arg(cat), BANK, 15, cbpeakf,
                          i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Moog ramp %1").arg(cat), BANK, 52,
                          moogrampf, i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Moog square %1").arg(cat), BANK, 53,
                          moogsquaref, i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Moog saw %1").arg(cat), BANK, 62,
                          moogrampf, i, q))
            ->setReverse(true)
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Octavius ramp %1").arg(cat), BANK, 54,
                          octaviusrampf, i, q))
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Octavius saw %1").arg(cat), BANK, 64,
                          octaviusrampf, i, q))
            ->setReverse(true)
            ->setSoftness(_softness);
    (new WaveFormStandard(QString("Exponential triangle %1").arg(cat), BANK,
                          82, exptrif, i, q))
            ->setSoftness(_softness);
}

WaveFormStandard::Set WaveFormStandard::WAVEFORMS;

// basic waveforms
const WaveFormStandard* const WaveFormStandard::SINE = WAVEFORMS.get(0, 0);
const WaveFormStandard* const WaveFormStandard::TRIANGLE
        = WAVEFORMS.get(0, 1);
const WaveFormStandard* const WaveFormStandard::RAMP   = WAVEFORMS.get(0, 2);
const WaveFormStandard* const WaveFormStandard::SQUARE = WAVEFORMS.get(0, 3);
const WaveFormStandard* const WaveFormStandard::HARSHRAMP
        = WAVEFORMS.get(0, 4);
const WaveFormStandard* const WaveFormStandard::SQPEAK = WAVEFORMS.get(0, 5);
const WaveFormStandard* const WaveFormStandard::WHITENOISE
        = WAVEFORMS.get(0, 6);
// 7 is reserved
const WaveFormStandard* const WaveFormStandard::ZERO
        = WAVEFORMS.get(ZERO_BANK, ZERO_INDEX);
const WaveFormStandard* const WaveFormStandard::SQRT = WAVEFORMS.get(21, 12);
const WaveFormStandard* const WaveFormStandard::SHARPGAUSS
        = WAVEFORMS.get(21, 47);

/*
const WaveFormStandard
        WaveFormStandard::SINE("Sine", 0, 0, nsinf, Linear, 10);
const WaveFormStandard
                       WaveFormStandard::TRIANGLE("Triangle", 0, 1, trianglef,
Exact); const WaveFormStandard WaveFormStandard::RAMP("Ramp", 0, 2, rampf,
Exact); const WaveFormStandard WaveFormStandard::SQUARE("Square", 0, 3,
squaref, Exact); const WaveFormStandard WaveFormStandard::HARSHRAMP("Harsh
ramp", 0, 4, harshrampf, Exact); const WaveFormStandard
                       WaveFormStandard::SQPEAK("Sq peak", 0, 5, sqpeakf,
Linear, 10); const WaveFormStandard WaveFormStandard::WHITENOISE( "White
noise", 0, 6, randf, Discrete, 10);
// 7 is reserved
const WaveFormStandard
                       WaveFormStandard::ZERO(" 0.0", ZERO_BANK, ZERO_INDEX,
zerof, Exact); const WaveFormStandard WaveFormStandard::SQRT( "sqrt()", 20,
12, sqrtf, Linear, 10);  // FLOAT REQUIRED const WaveFormStandard
WaveFormStandard::SHARPGAUSS( "sharpgauss()", 21, 47, sharpgaussf, Linear, 9);
*/

const wavefunction_t WaveForm::sine
        = [](const real_t _x) { return WaveFormStandard::SINE->f(_x); };
const wavefunction_t WaveForm::triangle
        = [](const real_t _x) { return WaveFormStandard::TRIANGLE->f(_x); };
const wavefunction_t WaveForm::ramp
        = [](const real_t _x) { return WaveFormStandard::RAMP->f(_x); };
const wavefunction_t WaveForm::square
        = [](const real_t _x) { return WaveFormStandard::SQUARE->f(_x); };
const wavefunction_t WaveForm::harshramp
        = [](const real_t _x) { return WaveFormStandard::HARSHRAMP->f(_x); };
const wavefunction_t WaveForm::sqpeak
        = [](const real_t _x) { return WaveFormStandard::SQPEAK->f(_x); };
const wavefunction_t WaveForm::whitenoise
        = [](const real_t _x) { return WaveFormStandard::WHITENOISE->f(_x); };
const wavefunction_t WaveForm::sqrt
        = [](const real_t _x) { return WaveFormStandard::SQRT->f(_x); };

const WaveFormStandard* WaveFormStandard::Set::get(const int _bank,
                                                   const int _index)
{
    const int b = _bank - MIN_BANK;
    const int i = _index - MIN_INDEX;
    if(_bank < MIN_BANK || _bank > MAX_BANK || _index < MIN_INDEX
       || _index > MAX_INDEX || m_stock[b][i] == nullptr)
        return m_stock[ZERO_BANK - MIN_BANK][ZERO_INDEX - MIN_INDEX];

    return m_stock[b][i];
}

void WaveFormStandard::Set::set(const int               _bank,
                                const int               _index,
                                const WaveFormStandard* _wf)
{
    const int b = _bank - MIN_BANK;
    const int i = _index - MIN_INDEX;
    if(m_stock[b][i] == _wf)
        return;
    if(m_stock[b][i] != nullptr)
        qWarning("Warning: replacing waveform[%d][%d] %s %s", b, i,
                 qPrintable(m_stock[b][i]->m_name), qPrintable(_wf->m_name));
    m_stock[b][i] = _wf;
}

void WaveFormStandard::Set::fillBankModel(ComboBoxModel& _model)
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

void WaveFormStandard::Set::fillIndexModel(ComboBoxModel& _model,
                                           const int      _bank)
{
    // BACKTRACE
    _model.setDisplayName(QString("Bank %1").arg(_bank));
    _model.clear();
    for(int i = MIN_INDEX; i <= MAX_INDEX; i++)
    {
        QString                 text("--");
        const WaveFormStandard* wf = get(_bank, i);
        if(wf != get(ZERO_BANK, ZERO_INDEX)
           || (_bank == ZERO_BANK && i == ZERO_INDEX))
            text = QString("%1")  //("%1 %2")
                                  //.arg(i, 2, 16, QChar('0'))
                           .arg(wf->name())
                           .trimmed();
        _model.addItem(text, NULL, i);
    }
}

const WaveFormStandard* WaveFormStandard::get(const int _bank,
                                              const int _index)
{
    return WAVEFORMS.get(_bank, _index);
}

void WaveFormStandard::fillBankModel(ComboBoxModel& _model)
{
    WAVEFORMS.fillBankModel(_model);
}

void WaveFormStandard::fillIndexModel(ComboBoxModel& _model, const int _bank)
{
    WAVEFORMS.fillIndexModel(_model, _bank);
}

WaveFormStandard::WaveFormStandard(const QString&        _name,
                                   const int             _bank,
                                   const int             _index,
                                   const interpolation_t _mode,
                                   const int             _quality) :
      WaveForm(_name, _mode, _quality),
      /*
    m_hardness(0.), m_softness(0.), m_absolute(false), m_opposite(false),
    m_complement(false), m_reverse(false), m_zero(false), m_center(false),
    m_phase(0.), */
      m_bank(_bank), m_index(_index)
{
    WAVEFORMS.set(_bank, _index, static_cast<const WaveFormStandard*>(this));
}

WaveFormStandard::WaveFormStandard(const QString&        _name,
                                   const int             _bank,
                                   const int             _index,
                                   const wavefunction_t  _func,
                                   const interpolation_t _mode,
                                   const int             _quality) :
      WaveFormStandard(_name, _bank, _index, _mode, _quality)
{
    if(_func == nullptr)
    {
        BACKTRACE
        qFatal("WaveFormStandard::WaveFormStandard null function: %s",
               qPrintable(_name));
    }

    m_func = _func;
    if(m_mode == Exact)
        m_built = true;
}

WaveFormStandard::WaveFormStandard(const QString&        _name,
                                   const int             _bank,
                                   const int             _index,
                                   const QString&        _file,
                                   const interpolation_t _mode,
                                   const int             _quality) :
      WaveFormStandard(_name, _bank, _index, _mode, _quality)
{
    m_file = _file;
    if(m_mode == Exact)
        m_mode = Linear;
}

WaveFormStandard::WaveFormStandard(const QString&        _name,
                                   const int             _bank,
                                   const int             _index,
                                   real_t*               _data,
                                   const int             _size,
                                   const interpolation_t _mode,
                                   const int             _quality) :
      WaveFormStandard(_name, _bank, _index, _mode, _quality)
{
    m_data = MM_ALLOC(real_t, _size);
    for(int f = 0; f < _size; ++f)
        m_data[f] = _data[f];
    m_size = _size - 1;
    if(m_mode == Exact)
        m_mode = Linear;
    m_built = true;
}

WaveFormStandard::WaveFormStandard(const QString&        _name,
                                   const int             _bank,
                                   const int             _index,
                                   const sampleFrame*    _data,
                                   const int             _size,
                                   const interpolation_t _mode,
                                   const int             _quality) :
      WaveFormStandard(_name, _bank, _index, _mode, _quality)
{
    m_data = MM_ALLOC(real_t, _size);
    for(int f = 0; f < _size; ++f)
        m_data[f] = _data[f][0];
    m_size = _size - 1;
    if(m_mode == Exact)
        m_mode = Linear;
    m_built = true;
}

WaveFormStandard::WaveFormStandard(const QString&          _name,
                                   const int               _bank,
                                   const int               _index,
                                   const WaveFormStandard* _wave) :
      WaveFormStandard(_name, _bank, _index, _wave->mode(), _wave->quality())
{
    m_func = _wave->m_func;
    m_file = _wave->m_file;
    // m_data  = _wave->m_data;
    // m_size  = _wave->m_size;
    m_built = _wave->m_built;

    if(_wave->m_data != nullptr && _wave->m_size > 0)
    {
        m_size = _wave->m_size;
        m_data = MM_ALLOC(real_t, m_size + 1);
        for(int f = 0; f < m_size + 1; ++f)
            m_data[f] = _wave->m_data[f];
    }
    else
    {
        m_data = nullptr;
        m_size = 0;
    }

    m_hardness   = _wave->m_hardness;
    m_softness   = _wave->m_softness;
    m_absolute   = _wave->m_absolute;
    m_opposite   = _wave->m_opposite;
    m_complement = _wave->m_complement;
    m_reverse    = _wave->m_reverse;
    m_zero       = _wave->m_zero;
    m_center     = _wave->m_center;
    m_phase      = _wave->m_phase;
}

WaveFormStandard::~WaveFormStandard()
{
    qInfo("~WaveFormStandard (%d,%d,%s)", m_bank, m_index,
          qPrintable(m_name));
    /*
    if(m_data != nullptr)
    {
        MM_FREE(m_data);
        m_data = nullptr;
    }
    */
}

/*
real_t WaveFormStandard::softness()
{
    return m_softness;
}

WaveFormStandard* WaveFormStandard::setSoftness(real_t _softness)
{
    if(m_softness != _softness)
    {
        m_softness = _softness;
        m_built    = false;
        // if(m_built) rebuild();
    }
    return this;
}

real_t WaveFormStandard::hardness()
{
    return m_hardness;
}

WaveFormStandard* WaveFormStandard::setHardness(real_t _hardness)
{
    if(m_hardness != _hardness)
    {
        m_hardness = _hardness;
        m_built    = false;
        // if(m_built) rebuild();
    }
    return this;
}

bool WaveFormStandard::absolute()
{
    return m_absolute;
}

WaveFormStandard* WaveFormStandard::setAbsolute(bool _absolute)
{
    if(m_absolute != _absolute)
    {
        m_absolute = _absolute;
        m_built    = false;
        // if(m_built) rebuild();
    }
    return this;
}

bool WaveFormStandard::complement()
{
    return m_complement;
}

WaveFormStandard* WaveFormStandard::setComplement(bool _complement)
{
    if(m_complement != _complement)
    {
        m_complement = _complement;
        m_built      = false;
        // if(m_built) rebuild();
    }
    return this;
}

bool WaveFormStandard::opposite()
{
    return m_opposite;
}

WaveFormStandard* WaveFormStandard::setOpposite(bool _opposite)
{
    if(m_opposite != _opposite)
    {
        m_opposite = _opposite;
        m_built    = false;
        // if(m_built) rebuild();
    }
    return this;
}

bool WaveFormStandard::reverse()
{
    return m_reverse;
}

WaveFormStandard* WaveFormStandard::setReverse(bool _reverse)
{
    if(m_reverse != _reverse)
    {
        m_reverse = _reverse;
        m_built   = false;
        // if(m_built) rebuild();
    }
    return this;
}

bool WaveFormStandard::zeroed()
{
    return m_zero;
}

WaveFormStandard* WaveFormStandard::setZeroed(bool _zero)
{
    if(m_zero != _zero)
    {
        m_zero  = _zero;
        m_built = false;
        // if(m_built) rebuild();
    }
    return this;
}

bool WaveFormStandard::centered()
{
    return m_center;
}

WaveFormStandard* WaveFormStandard::setCentered(bool _center)
{
    if(m_center != _center)
    {
        m_center = _center;
        m_built  = false;
        // if(m_built) rebuild();
    }
    return this;
}

void WaveFormStandard::rebuild()
{
    m_built = false;
    build();
}

void WaveFormStandard::build()
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

    if(m_hardness != 0.)
        harden();
    if(m_softness != 0.)
        soften();
    if(m_absolute || m_opposite || m_complement || m_reverse)
        acoren();
    if(m_center)
        center();
    if(m_zero)
        zero();
    if(m_phase != 0.)
        dephase();
}

void WaveFormStandard::harden()
{
    const int size = m_size + 1;
    const int bw   = 0.5 * m_hardness * m_size;
    if(bw == 0)
        return;

    real_t omin = m_data[0];
    real_t omax = m_data[0];
    real_t nmin = omin;
    real_t nmax = omax;

    real_t* data = MM_ALLOC(real_t, size);
    for(int f = 0; f < size; ++f)
    {
        const real_t ov = m_data[f];
        if(ov < omin)
            omin = ov;
        if(ov > omax)
            omax = ov;

        const real_t nv = m_data[(f / bw) * bw];
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

void WaveFormStandard::soften()
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

void WaveFormStandard::acoren()
{
    const int size = m_size + 1;

    if(m_absolute)
        for(int f = 0; f < size; ++f)
            m_data[f] = abs(m_data[f]);
    if(m_opposite)
        for(int f = 0; f < size; ++f)
            m_data[f] = -m_data[f];
    if(m_complement)
    {
    }
    if(m_reverse)
    {
        real_t v;
        for(int f = m_size / 2 - 1; f >= 0; f--)
        {
            v                  = m_data[f];
            m_data[f]          = m_data[m_size - f];
            m_data[m_size - f] = v;
        }
    }
}

void WaveFormStandard::rotate_frames(int _n)
{
    const int size = m_size + 1;
    real_t*   data = MM_ALLOC(real_t, size);

    for(int f = 0; f < size; ++f)
        data[f] = m_data[(f + _n) % size];

    real_t* old = m_data;
    m_data      = data;
    MM_FREE(old);
}

void WaveFormStandard::zero()
{
    const int size = m_size + 1;

    int    f0 = 0;
    real_t v0 = 1.;
    for(int f = 0; f < size; ++f)
    {
        const real_t v = abs(m_data[f]);
        if(v < v0)
        {
            v0 = v;
            f0 = f;
        }
    }
    if(f0 > 0)
        rotate_frames(-f0);
}

void WaveFormStandard::center()
{
    const int size = m_size + 1;

    int    f0 = 0;
    real_t e0 = 0.;
    for(int f = 0; f < size; ++f)
    {
        const real_t e = abs(m_data[(f + 1) % size]) + abs(m_data[f])
                         + abs(m_data[(f - 1 + size) % size]);
        if(e < e0)
        {
            e0 = e;
            f0 = f;
        }
    }
    if(f0 > 0)
        rotate_frames(-f0 + m_size / 2);
}

void WaveFormStandard::dephase()
{
    const int size = m_size + 1;
    rotate_frames(int(round(m_phase * size)));
}

void WaveFormStandard::normalize_frames()
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
            for(int f = 0; f < size; ++f)
                m_data[f] *= k;
    }
}

// x must be between 0. and 1.
real_t WaveFormStandard::f(const real_t _x) const
{
    return f(_x, m_mode);
}

real_t WaveFormStandard::f(const real_t _x, const real_t _antialias) const
{
    return f(_x, _antialias, m_mode);
}

// x must be between 0. and 1.
real_t WaveFormStandard::f(const real_t          _x,
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
        // optimal4pInterpolate(r0, r1, r2, r3, 0.5);
    }
    else
    {
        r = f(_x, _m);
    }

    return r;
}

// x must be between 0. and 1.
real_t WaveFormStandard::f(const real_t _x, const interpolation_t _m) const
{
    if(!m_built)
        const_cast<WaveFormStandard*>(this)->build();

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
                qInfo("WaveFormStandard::f Discrete x=%f m_data=%p i=%d "
                      "size=%d",
                      _x, m_data, i, m_size);
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
                qInfo("WaveFormStandard::f Rounded x=%f m_data=%p i=%d "
                      "size=%d",
                      _x, m_data, i, m_size);
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
                qInfo("WaveFormStandard::f Optimal2 x=%f m_data=%p i=%d "
                      "size=%d",
                      _x, m_data, i, m_size);
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
*/

/*
WaveFormStandard::Plan::Plan()
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

WaveFormStandard::Plan::~Plan()
{
    fftwf_destroy_plan(m_c2rPlan);
    fftwf_destroy_plan(m_r2cPlan);
    fftwf_free(m_specBuf);
    fftwf_free(m_normBuf);
}

WaveFormStandard::Plan::build(WaveFormStandard& _wf, const real_t _cut)
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

WaveFormStandard::Plan WaveFormStandard::PLAN;
*/
