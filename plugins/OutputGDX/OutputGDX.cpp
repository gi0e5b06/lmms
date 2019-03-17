/*
 * OutputGDX.cpp - audio output properties
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

#include "OutputGDX.h"

#include "ValueBuffer.h"
#include "WaveForm.h"
#include "embed.h"

#include <math.h>
//#include "lmms_math.h"

extern "C"
{
    Plugin::Descriptor PLUGIN_EXPORT outputgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "OutputGDX",
               QT_TRANSLATE_NOOP("pluginBrowser",
                                 "A audio output properties plugin"),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               nullptr,
               nullptr};
}

OutputGDX::OutputGDX(Model*                                    parent,
                     const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&outputgdx_plugin_descriptor, parent, key),
      m_gdxControls(this), m_currentVB(0)
{
    for(int i = 0; i < 3; i++)
    {
        m_left[i]  = new ValueBuffer(Engine::mixer()->framesPerPeriod());
        m_right[i] = new ValueBuffer(Engine::mixer()->framesPerPeriod());
    }

    connect(this, SIGNAL(sendRms(const real_t)), &m_gdxControls.m_rmsModel,
            SLOT(setAutomatedValue(const real_t)));
    connect(this, SIGNAL(sendVol(const real_t)), &m_gdxControls.m_volModel,
            SLOT(setAutomatedValue(const real_t)));
    connect(this, SIGNAL(sendPan(const real_t)), &m_gdxControls.m_panModel,
            SLOT(setAutomatedValue(const real_t)));
    connect(this, SIGNAL(sendLeft(const ValueBuffer*)),
            &m_gdxControls.m_leftModel,
            SLOT(setAutomatedBuffer(const ValueBuffer*)));
    connect(this, SIGNAL(sendRight(const ValueBuffer*)),
            &m_gdxControls.m_rightModel,
            SLOT(setAutomatedBuffer(const ValueBuffer*)));
}

OutputGDX::~OutputGDX()
{
    // disconnect(&m_gdxControls.m_rightModel);
    // disconnect(&m_gdxControls.m_leftModel);

    for(int i = 0; i < 3; i++)
    {
        delete m_left[i];
        delete m_right[i];
    }
}

bool OutputGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    /*
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
    {
        m_left.lock();
        m_right.lock();
        m_left.fill(0.);
        m_right.fill(0.);
        m_left.setPeriod(AutomatableModel::periodCounter() + 1);
        m_right.setPeriod(AutomatableModel::periodCounter() + 1);
        m_left.unlock();
        m_right.unlock();
        emit sendLeft(&m_left);
        emit sendRight(&m_right);
        emit sendRms(0.);
        emit sendVol(0.);
        emit sendPan(0.);
        return false;
    }
    */
    if(!isRunning())
        startRunning();

    // qInfo("OutputGDX::processAudioBuffer rms=%f scl=%f val=%f", rms,
    //      m_rmsModel.scaledValue(rms), m_rmsModel.value());
    real_t rms[2] = {0., 0.};
    real_t max[2] = {0., 0.};

    m_currentVB=(m_currentVB+1)%3;

    if(m_left[m_currentVB]->length() != _frames || m_right[m_currentVB]->length() != _frames)
    {
        qWarning("OutputGDX::processAudioBuffer");
        return false;
    }

    m_left[m_currentVB]->lock();
    m_right[m_currentVB]->lock();
    m_left[m_currentVB]->setPeriod(AutomatableModel::periodCounter() + 1);
    m_right[m_currentVB]->setPeriod(AutomatableModel::periodCounter() + 1);

    for(fpp_t f = 0; f < _frames; ++f)
    {
        const sample_t curVal0 = _buf[f][0];
        const sample_t curVal1 = _buf[f][1];
        // if(f == 10)
        //   qInfo("Output: L=%5f R=%5f", curVal0, curVal1);

        m_left[m_currentVB]->set(f, curVal0);
        m_right[m_currentVB]->set(f, curVal1);
        rms[0] += curVal0 * curVal0;
        rms[1] += curVal1 * curVal1;
        max[0] = qMax(max[0], abs(curVal0));
        max[1] = qMax(max[1], abs(curVal1));
    }

    m_left[m_currentVB]->unlock();
    m_right[m_currentVB]->unlock();

    emit sendLeft(m_left[m_currentVB]);
    emit sendRight(m_right[m_currentVB]);

    // bool silent = true;
    if(max[0] <= SILENCE)
    {
        max[0] = 0.;
        rms[0] = 0.;
    }
    else
    {
        // silent = false;
        rms[0] = WaveForm::sqrt(bound(0., rms[0] / _frames, 1.));
    }

    if(max[1] <= SILENCE)
    {
        max[1] = 0.;
        rms[1] = 0.;
    }
    else
    {
        // silent = false;
        rms[1] = WaveForm::sqrt(bound(0., rms[1] / _frames, 1.));
    }

    emit sendRms(bound(0., rms[0] + rms[1], 1.));
    emit sendVol(qMax(max[0], max[1]));
    emit sendPan(bound(-1., rms[1] - rms[0], 1.));

    return true;  // ! silent;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new OutputGDX(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
