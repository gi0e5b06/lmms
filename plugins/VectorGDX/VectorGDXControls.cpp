/*
 * VectorGDXControls.cpp - controls for wall effect
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

#include "VectorGDXControls.h"

#include "Engine.h"
#include "Knob.h"
#include "Mixer.h"
#include "Song.h"
#include "TempoSyncKnob.h"
#include "VectorGDX.h"

#include <QDomElement>

VectorGDXControls::VectorGDXControls(VectorGDXEffect* effect) :
      EffectControls(effect), m_effect(effect),
      m_sizeModel(qMin(16, N / 2), 1, N, 1, this, tr("Size")),
      m_levelModel(10., 0., 100., 0.1, this, tr("Level")),
      m_gainModel(1., 0., 10., 0.001, this, tr("Gain")),
      m_feedbackModel(0., 0., 100., 0.1, this, tr("Feedback")),
      m_latencyModel(100., 0., 1000., 0.1, 1000., this, tr("Latency")),
      m_frequencyModel(441., 1., 22050., 0.1, this, tr("Frequency"))
{
    const sample_rate_t SR = Engine::mixer()->baseSampleRate();
    const real_t        pmax
            = pow((long double)2048., (long double)1. / (long double)(N - 1));
    for(int i = 0; i < N; i++)
    {
        m_pos[i] = round(SR * pow(pmax, i) / 2048.) - 22;
        //qInfo("pos[%d]=%d", i, m_pos[i]);
    }

    m_sizeModel.setStrictStepSize(true);
    connect(&m_sizeModel, SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));

    //connect(&m_latencyModel,SIGNAL(dataChanged()),this,SLOT(updateFrequency()));
    //connect(&m_frequencyModel,SIGNAL(dataChanged()),this,SLOT(updateLatency()));

    randomizeVector();
    /*
    connect( &m_distanceModel, SIGNAL( dataChanged() ), this, SLOT(
      changeControl() ) );
    connect( &m_wetModel, SIGNAL(
      dataChanged() ), this, SLOT( changeControl() ) );
    */
}

void VectorGDXControls::randomizeVector()
{
    const int    last  = size() - 1;
    const real_t level = m_levelModel.value() / 100.;

    for(int i = last; i >= 0; --i)
        m_vector[i] += level * level * (fastrand01inc() * 2. - 1.);

    normalizeVector();
}

void VectorGDXControls::softenVector()
{
    const int    n     = size();
    const int    last  = n - 1;
    const real_t level = m_levelModel.value() / 100.;

    for(int i0 = last; i0 >= 0; --i0)
    {
        real_t a = 1.;
        real_t v = m_vector[i0];
        for(int x = -1; x <= 1; x++)
        {
            const int i = i0 + x;
            if(x == 0 || i < 0 || i > last)
                continue;

            const int d = abs(x);
            a += d;
            v += m_vector[i] / d;
        }
        m_vector[i0] = level * v / a + (1. - level) * m_vector[i0];
    }

    normalizeVector();
}

void VectorGDXControls::headVector()
{
    const int    n     = size();
    const int    last  = n - 1;
    const real_t level = m_levelModel.value() / 100.;

    for(int i = last; i > 0; --i)
    {
        const real_t v = m_vector[i] * real_t(n - i) / real_t(n);

        m_vector[i] = level * v + (1. - level) * m_vector[i];
    }

    normalizeVector();
}

void VectorGDXControls::tailVector()
{
    const int    n     = size();
    const int    last  = n - 1;
    const real_t level = m_levelModel.value() / 100.;

    for(int i = last; i >= 0; --i)
    {
        const real_t v = m_vector[i] * real_t(i) / real_t(n);

        m_vector[i] = level * v + (1. - level) * m_vector[i];
    }

    normalizeVector();
}

void VectorGDXControls::identifyVector()
{
    const int    n     = size();
    const int    last  = n - 1;
    const real_t level = m_levelModel.value() / 100.;

    /*
    if(m_vector[0] < 0.)
        for(int i = last; i >= 0; --i)
            m_vector[i] = -m_vector[i];
    */

    m_vector[0] += level;
    for(int i = last; i > 0; --i)
        m_vector[i] = (1. - level) * m_vector[i];

    normalizeVector();
}

void VectorGDXControls::normalizeVector()
{
    const int n    = size();
    const int last = n - 1;

    real_t vmax = 0.;
    for(int i = last; i >= 0; --i)
    {
        real_t v = abs(m_vector[i]);
        if(v<0.0001) m_vector[i]=v=0.;
        if(vmax < v)
            vmax = v;
    }
    if(vmax != 0.)
        for(int i = last; i >= 0; --i)
            m_vector[i] /= vmax;

    real_t c = 0.;
    for(int i = last; i >= 0; --i)
    {
        real_t v = abs(m_vector[i]);
        c += v * v;
    }
    if(c != 0.)
    {
        c = sqrt(c);
        for(int i = last; i >= 0; --i)
            m_vector[i] /= c;
    }

    // for(int i = n; i < N; i++)
    //    m_vector[i] = 0.;

    QString s("");
    for(int i = 0; i <= last; i++)
        s.append(QString("%1").arg(m_vector[i], 8, 'f', 4));
    qInfo("%s\n", qPrintable(s));

    emit dataChanged();
}

void VectorGDXControls::changeControl()
{
    // engine::getSong()->setModified();
}

void VectorGDXControls::loadSettings(const QDomElement& _this)
{
    m_sizeModel.loadSettings(_this, "size");
    m_levelModel.loadSettings(_this, "level");
    m_gainModel.loadSettings(_this, "gain");
    m_feedbackModel.loadSettings(_this, "feedback");
    m_latencyModel.loadSettings(_this, "latency");

    // const int         n = size();
    const QStringList s(_this.attribute("vector").split(QChar(',')));
    for(int i = 0; i < N; i++)
    {
        if(i < s.size())
            m_vector[i] = s.at(i).toDouble();
        else
            m_vector[i] = 0.;
    }
}

void VectorGDXControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_sizeModel.saveSettings(doc, _this, "size");
    m_levelModel.saveSettings(doc, _this, "level");
    m_gainModel.saveSettings(doc, _this, "gain");
    m_feedbackModel.saveSettings(doc, _this, "feedback");
    m_latencyModel.saveSettings(doc, _this, "latency");

    // const int   n = size();
    QStringList s;
    for(int i = 0; i < N; i++)
        s.append(QString::number(m_vector[i], 'f'));
    _this.setAttribute("vector", s.join(QChar(',')));
}
