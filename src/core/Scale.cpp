/*
 * Scale.cpp -
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

#include "Scale.h"

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
#include <QRegExp>
#include <QTextStream>

/*
real_t scale_et(const real_t _x,
               const real_t _b             = 0.,
               const real_t _baseFrequency = 440.,
               const real_t _baseKey       = 69.,
               const real_t _octaveFactor  = 2.,
               const real_t _octaveKeys    = 12.)
{
    return _baseFrequency
           * pow(_octaveFactor, (_key - _baseKey + _bending) / _octaveKeys);
}
*/

Scale::Set::Set()
{
    for(int b = MAX_BANK - MIN_BANK; b >= 0; --b)
        for(int i = MAX_INDEX - MIN_INDEX; i >= 0; --i)
            m_stock[b][i] = nullptr;

    new Scale("ET12", ET12_BANK, ET12_INDEX);

    int BANK;

    // Basic #0
    // 107 is reserved
    BANK              = 0;
    m_bankNames[BANK] = "Basic";
    /*new Scale ("Pulse", BANK, 8, pulsef, Exact);*/

    // Adjusted
    // 0a/0p versions (minimize the volume at the end and the beginning.
    // 107 is reserved
    BANK              = 1;
    m_bankNames[BANK] = "Adjusted";

    // Centered (2)
    // Max energy at the center
    BANK              = 2;
    m_bankNames[BANK] = "Centered";

    // Soften
    BANK              = 5;
    m_bankNames[BANK] = "Soften";

    // Degraded

    // Constant
    BANK              = 20;
    m_bankNames[BANK] = "Constant";

    // Mathematical
    BANK              = 21;
    m_bankNames[BANK] = "Mathematical";

    int sbank = 35;
    {
        QDir sclrd("../../../lmms/scales", "Scala_*",
                   QDir::Name | QDir::IgnoreCase,
                   QDir::Dirs | QDir::NoDotAndDotDot);
        for(QString& sclb: sclrd.entryList())
        {
            QDir sclbd(sclrd.absolutePath() + "/" + sclb, "*.scl",
                       QDir::Name | QDir::IgnoreCase,
                       QDir::Files | QDir::NoDotAndDotDot);

            QString bankname = sclb;
            // bankname.replace(QRegExp("^AKSCL_"), "");
            bankname.replace('_', ' ');
            m_bankNames[sbank - MIN_BANK] = bankname.trimmed();

            int sindex = MIN_INDEX;
            for(QString& sclf: sclbd.entryList())
            {
                if(sindex > MAX_INDEX)
                    continue;
                QString filename  = sclbd.absolutePath() + "/" + sclf;
                QString scalename = sclf;
                // scalename.replace(QRegExp("^AKSCL_"), "");
                scalename.replace(QRegExp("[.]scl$", Qt::CaseInsensitive),
                                  "");
                scalename.replace('_', ' ');
                scalename.replace('-', ' ');

                new Scale(qPrintable(scalename.trimmed()), sbank, sindex,
                          filename);

                sindex++;
            }
            sbank++;
        }
    }

    sbank = 100;
    {
        QDir sclrd("../../../lmms/scales", "User_*",
                   QDir::Name | QDir::IgnoreCase,
                   QDir::Dirs | QDir::NoDotAndDotDot);
        for(QString& sclb: sclrd.entryList())
        {
            QDir sclbd(sclrd.absolutePath() + "/" + sclb, "*.scl",
                       QDir::Name | QDir::IgnoreCase,
                       QDir::Files | QDir::NoDotAndDotDot);

            qInfo("bank scale '%s'", qPrintable(sclb));
            QString bankname = sclb;
            // bankname.replace(QRegExp("^AKSCL_"), "");
            bankname.replace('_', ' ');
            m_bankNames[sbank - MIN_BANK] = bankname.trimmed();

            int sindex = MIN_INDEX;
            for(QString& sclf: sclbd.entryList())
            {
                if(sindex > MAX_INDEX)
                    continue;
                QString filename  = sclbd.absolutePath() + "/" + sclf;
                QString scalename = sclf;
                // scalename.replace(QRegExp("^AKSCL_"), "");
                scalename.replace(QRegExp("[.]scl$", Qt::CaseInsensitive),
                                  "");
                scalename.replace('_', ' ');
                scalename.replace('-', ' ');

                new Scale(qPrintable(scalename.trimmed()), sbank, sindex,
                          filename);

                sindex++;
            }
            sbank++;
        }
    }
}

Scale::Set::~Set()
{
    qInfo("Scale::Set::~Set START");
    for(int b = MAX_BANK - MIN_BANK; b >= 0; --b)
        for(int i = MAX_INDEX - MIN_INDEX; i >= 0; --i)
            if(m_stock[b][i] != nullptr)  // && m_stock[b][i] != ET12)
                delete m_stock[b][i];
    qInfo("Scale::Set::~Set END");
}

Scale::Set Scale::SCALES;

// basic scales
const Scale* const Scale::ET12 = SCALES.get(ET12_BANK, ET12_INDEX);

const Scale* Scale::Set::get(const int _bank, const int _index)
{
    const int b = _bank - MIN_BANK;
    const int i = _index - MIN_INDEX;
    if(_bank < MIN_BANK || _bank > MAX_BANK || _index < MIN_INDEX
       || _index > MAX_INDEX || m_stock[b][i] == nullptr)
        return m_stock[ET12_BANK - MIN_BANK][ET12_INDEX - MIN_INDEX];

    return m_stock[b][i];
}

void Scale::Set::set(const int _bank, const int _index, const Scale* _scl)
{
    const int b = _bank - MIN_BANK;
    const int i = _index - MIN_INDEX;
    if(m_stock[b][i] == _scl)
        return;
    if(m_stock[b][i] != nullptr)
        qWarning("Warning: replacing scale[%d][%d]", b, i);
    m_stock[b][i] = _scl;
}

void Scale::Set::fillBankModel(ComboBoxModel& _model)
{
    _model.setDisplayName(QString("Scale Banks"));
    _model.clear();
    for(int b = MIN_BANK; b <= MAX_BANK; b++)
    {
        QString text("--");
        if(m_bankNames[b - MIN_BANK] != "")
            text = QString("%1")  //("%1 %2")
                                  //.arg(b, 2, 16, QChar('0'))
                           .arg(m_bankNames[b - MIN_BANK])
                           .trimmed();
        _model.addItem(text, nullptr, b);
    }
}

void Scale::Set::fillIndexModel(ComboBoxModel& _model, const int _bank)
{
    // BACKTRACE
    _model.setDisplayName(QString("Bank %1").arg(_bank));
    _model.clear();
    for(int i = MIN_INDEX; i <= MAX_INDEX; i++)
    {
        QString      text("--");
        const Scale* scl = get(_bank, i);
        if(scl != ET12 || (_bank == ET12_BANK && i == ET12_INDEX))
            text = QString("%1")  //("%1 %2")
                                  //.arg(i, 2, 16, QChar('0'))
                           .arg(scl->name())
                           .trimmed();
        _model.addItem(text, nullptr, i);
    }
}

const Scale* Scale::get(const int _bank, const int _index)
{
    return SCALES.get(_bank, _index);
}

void Scale::fillBankModel(ComboBoxModel& _model)
{
    SCALES.fillBankModel(_model);
}

void Scale::fillIndexModel(ComboBoxModel& _model, const int _bank)
{
    SCALES.fillIndexModel(_model, _bank);
}

Scale::Scale(const QString& _name,
             const int      _bank,
             const int      _index,
             const real_t   _baseFrequency,
             const real_t   _baseKey,
             const real_t   _octaveFactor,
             const real_t   _octaveKeys,
             const real_t   _bendingFactor) :
      m_built(false),
      m_name(_name), m_bank(_bank), m_index(_index),
      m_baseFrequency(_baseFrequency), m_baseKey(_baseKey),
      m_octaveFactor(_octaveFactor), m_octaveKeys(_octaveKeys),
      m_bendingFactor(_bendingFactor)
{
    m_file = "";
    m_size = -1;
    m_data = nullptr;

    SCALES.set(_bank, _index, static_cast<const Scale*>(this));
}

Scale::Scale(const QString& _name,
             const int      _bank,
             const int      _index,
             const QString& _file) :
      Scale(_name, _bank, _index)
{
    m_file = _file;
}

Scale::~Scale()
{
    qInfo("Scale::~Scale");
    if(m_data != nullptr)
    {
        MM_FREE(m_data);
        m_data = nullptr;
    }
}

void Scale::rebuild()
{
    m_built = false;
    build();
}

void Scale::build()
{
    if(m_built)
        return;

    static QMutex s_building;
    QMutexLocker  locker(&s_building);
    if(m_built)
        return;

    if(m_data != nullptr)
    {
        MM_FREE(m_data);
        m_data = nullptr;
    }

    if(m_file != "")
    {
        QFile f(m_file);
        if(!f.open(QFile::ReadOnly | QFile::Text))
        {
            qWarning("Warning: can not open file: %s", qPrintable(m_file));
            return;
        }
        QTextStream in(&f);

        QVector<real_t> degrees;
        degrees.append(1.);
        bool    description = false;
        int     count       = -1;
        QString s;
        while(!(s = in.readLine()).isNull())
        {
            s = s.trimmed();
            if(s.startsWith(QChar('!')))
                continue;
            if(!description)
            {
                qInfo("Scale: description: '%s'", qPrintable(s));
                description = true;
                continue;
            }
            if(count < 0)
            {
                bool ok = true;
                count   = s.toInt(&ok);
                if(!ok || count < 0)
                {
                    qWarning("Warning: Scale: invalid degree number: %d",
                             count);
                    return;
                }
                continue;
            }
            s.replace(QRegExp("[^0-9./].*$"), "");
            if(!s.contains(QRegExp("^[0-9]+[.][0-9]*$"))
               && !s.contains(QRegExp("^[0-9]+/[0-9]+$"))
               && !s.contains(QRegExp("^[0-9]+$")))
            {
                qWarning("Warning: Scale: invalid degree value: %s",
                         qPrintable(s));
                return;
            }

            if(s.indexOf(QChar('.')) >= 0)
            {
                degrees.append((1200. + s.toDouble()) / 1200.);
            }
            else
            {
                if(s.indexOf(QChar('/')) < 0)
                    s.append("/1");
                int    p           = s.indexOf(QChar('/'));
                double numerator   = s.left(p).toDouble();
                double denominator = s.mid(p + 1).toDouble();
                // qInfo("Notice: Scale: %f/%f", numerator, denominator);
                degrees.append(numerator / denominator);
            }
        }

        if(count + 1 != degrees.size())
        {
            qWarning("Warning: Scale: bad file format: %d != %d", count + 1,
                     degrees.size());
        }

        int size = degrees.size();
        m_data   = MM_ALLOC(real_t, size);
        for(int i = 0; i < size; i++)
        {
            m_data[i] = degrees.at(i);
            qInfo("Scale: degree n°%d is %f (%f Hz)", i, m_data[i],
                  440. * m_data[i]);
        }
        m_size         = size - 1;
        m_octaveKeys   = m_size;
        m_octaveFactor = m_data[m_size];
        if(size >= 1)
            m_bendingFactor = m_data[1];
        m_built = true;
    }
    else
    {
        m_built = true;
    }
}

// convenient f() for later (curve, waveform)
// x must be between 0. and 1.
real_t Scale::f(const real_t _x) const
{
    real_t k = qBound(0., round(127. * _x), 127.);
    real_t b = 100. * (127. * _x - k);
    return qBound(0., frequency(k, b) / 22050., 1.);
}

// x must be between 0. and 1.
real_t Scale::tune(const real_t _x) const
{
    if(!m_built)
        const_cast<Scale*>(this)->build();

    if(m_data == nullptr)
        return m_baseFrequency
               * pow(m_octaveFactor, _x * 127. / m_octaveKeys);

    const real_t x = _x * 127.;
    int          k = x;  // floor(x);
    const real_t b = x - k;
    real_t       r = m_baseFrequency;
    while(k < 0)
    {
        r /= m_data[m_size];
        k += m_size;
    }
    while(k >= m_size)
    {
        r *= m_data[m_size];
        k -= m_size;
    }
    if(k >= 1 && k <= m_size)
    {
        r *= m_data[k];
        k = 0;
    }
    if(m_size >= 1)
        r *= (1. + b * (m_data[1] - 1.));
    return r;
}

real_t Scale::bending(const real_t _x) const
{
    if(!m_built)
        const_cast<Scale*>(this)->build();

    return pow(m_bendingFactor, _x / m_octaveKeys);

    // TODO
    // return pow(m_bendingFactor, sin(_x) / m_octaveKeys);
}

// k should be between 0. and 127.
real_t Scale::frequency(const real_t _key, const real_t _cents) const
{
    real_t x = (_key - m_baseKey + _cents / 100.);
    real_t k = round(x);
    real_t b = x - k;
    // qInfo("FREQ %f %f %f",x,k,b);
    return tune(bound(-1., k / 127., 1.)) * bending(bound(-1., b, 1.));
}
