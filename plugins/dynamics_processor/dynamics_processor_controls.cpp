/*
 * dynamics_processor_controls.cpp - controls for dynamics_processor-effect
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "dynamics_processor_controls.h"

#include "Engine.h"
#include "Graph.h"
#include "Song.h"
#include "base64.h"
#include "dynamics_processor.h"

#include <QDomElement>

//#define onedB 1.1220184543019633
#define onedB 1.122018454301963341634973403415642678737640380859375

dynProcControls::dynProcControls(dynProcEffect* _eff) :
      EffectControls(_eff), m_effect(_eff),
      m_inputModel(1., 0., 5., 0.01f, this, tr("Input gain")),
      m_outputModel(1., 0., 5., 0.01f, this, tr("Output gain")),
      m_attackModel(10., 1., 500., 1., this, tr("Attack time")),
      m_releaseModel(100., 1., 500., 1., this, tr("Release time")),
      m_waveGraphModel(0., 1., 200, this),
      m_stereomodeModel(0, 0, 2, this, tr("Stereo mode"))
{
    connect(&m_waveGraphModel, SIGNAL(samplesChanged(int, int)), this,
            SLOT(samplesChanged(int, int)));
    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(sampleRateChanged()));

    setDefaultShape();
}

void dynProcControls::sampleRateChanged()
{
    m_effect->m_needsUpdate = true;
}

void dynProcControls::samplesChanged(int _begin, int _end)
{
    Engine::getSong()->setModified();
}

void dynProcControls::loadSettings(const QDomElement& _this)
{
    // load knobs, stereomode
    m_inputModel.loadSettings(_this, "inputGain");
    m_outputModel.loadSettings(_this, "outputGain");
    m_attackModel.loadSettings(_this, "attack");
    m_releaseModel.loadSettings(_this, "release");
    m_stereomodeModel.loadSettings(_this, "stereoMode");

    // load waveshape
    // int   size = 0;
    FLOAT* dst = nullptr;
    base64::decodeFloats(_this.attribute("waveShape"), &dst);
    m_waveGraphModel.setSamples((FLOAT*)dst);
    delete[] dst;
}

void dynProcControls::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    // save input, output knobs
    m_inputModel.saveSettings(_doc, _this, "inputGain");
    m_outputModel.saveSettings(_doc, _this, "outputGain");
    m_attackModel.saveSettings(_doc, _this, "attack");
    m_releaseModel.saveSettings(_doc, _this, "release");
    m_stereomodeModel.saveSettings(_doc, _this, "stereoMode");

    // save waveshape
    _this.setAttribute("waveShape",
                       base64::encodeFloats(m_waveGraphModel.samples(),
                                            m_waveGraphModel.length()));
}

void dynProcControls::setDefaultShape()
{
    FLOAT shp[200] = {};
    for(int i = 0; i < 200; i++)
        shp[i] = (FLOAT(i) + 1.) / 200.;

    m_waveGraphModel.setLength(200);
    m_waveGraphModel.setSamples(shp);  // const_cast<FLOAT*>(shp)
}

void dynProcControls::resetClicked()
{
    setDefaultShape();
    Engine::getSong()->setModified();
}

void dynProcControls::smoothClicked()
{
    m_waveGraphModel.smoothNonCyclic();
    Engine::getSong()->setModified();
}

void dynProcControls::addOneClicked()
{
    for(int i = 0; i < 200; i++)
    {
        m_waveGraphModel.setSampleAt(
                i, qBound(0., m_waveGraphModel.samples()[i] * onedB, 1.));
    }
    Engine::getSong()->setModified();
}

void dynProcControls::subOneClicked()
{
    for(int i = 0; i < 200; i++)
    {
        m_waveGraphModel.setSampleAt(
                i, qBound(0., m_waveGraphModel.samples()[i] / onedB, 1.));
    }
    Engine::getSong()->setModified();
}
