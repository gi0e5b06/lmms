/*
 * PeakController.cpp - implementation of class controller which handles
 *                      remote-control of AutomatableModels
 *
 * Copyright (c) 2008 Paul Giblock <drfaygo/at/gmail.com>
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

#include "PeakController.h"

#include "EffectChain.h"
#include "Mixer.h"
#include "PresetPreviewPlayHandle.h"
#include "plugins/peak_controller_effect/peak_controller_effect.h"

#include <QDomElement>
#include <QMessageBox>

#include <cmath>

PeakControllerEffectVector PeakController::s_effects;

int  PeakController::s_getCount;
int  PeakController::s_loadCount;
bool PeakController::s_buggedFile;

PeakController::PeakController(Model*                _parent,
                               PeakControllerEffect* _peakEffect) :
      Controller(Controller::PeakController, _parent, tr("Peak Controller")),
      m_peakEffect(_peakEffect), m_currentSample(0.)
{
    setFrequentlyUpdated(true);
    setSampleExact(true);

    if(m_peakEffect != nullptr)
        connect(m_peakEffect, SIGNAL(destroyed()), this,
                SLOT(handleDestroyedEffect()));

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(updateCoeffs()));
    connect(m_peakEffect->attackModel(), SIGNAL(dataChanged()), this,
            SLOT(updateCoeffs()));
    connect(m_peakEffect->decayModel(), SIGNAL(dataChanged()), this,
            SLOT(updateCoeffs()));

    m_coeffNeedsUpdate = true;
}

PeakController::~PeakController()
{
    qInfo("TODO: PeakController::~PeakController");

    // EffectChain::loadSettings() appends effect to EffectChain::m_effects
    // When it's previewing, EffectChain::loadSettings(<Controller Fx XML>) is
    // not called Therefore, we shouldn't call removeEffect() as it is not
    // even appended. NB: Most XML setting are loaded on preview, except
    // controller fx.
    /*
    if(m_peakEffect != nullptr && m_peakEffect->effectChain() != nullptr)
    //&& PresetPreviewPlayHandle::isPreviewing() == false )
    {
        qWarning("PeakController::~PeakController()");
        m_peakEffect->effectChain()->removeEffect(m_peakEffect);
    }
    */
}

void PeakController::fillValueBuffer()
{
    if(m_coeffNeedsUpdate)
    {
        const real_t ratio = 44100. / Engine::mixer()->processingSampleRate();
        m_attackCoeff
                = 1.
                  - exp2(-0.3 * (1. - m_peakEffect->attackModel()->value())
                         * ratio);
        m_decayCoeff
                = 1.
                  - exp2(-0.3 * (1. - m_peakEffect->decayModel()->value())
                         * ratio);
        m_coeffNeedsUpdate = false;
    }

    if(m_peakEffect != nullptr)
    {
        sample_t targetSample = m_peakEffect->lastSample();
        if(m_currentSample != targetSample)
        {
            const f_cnt_t frames = Engine::mixer()->framesPerPeriod();
            real_t*       values = m_valueBuffer.values();

            for(f_cnt_t f = 0; f < frames; ++f)
            {
                const real_t diff = (targetSample - m_currentSample);
                if(m_currentSample < targetSample)  // going up...
                    m_currentSample += diff * m_attackCoeff;
                else if(m_currentSample > targetSample)  // going down
                    m_currentSample += diff * m_decayCoeff;
                values[f] = m_currentSample;
            }
        }
        else
        {
            m_valueBuffer.fill(m_currentSample);
        }
    }
    else
    {
        m_valueBuffer.clear();  // GDX fill( 0 );
    }

    // m_lastUpdatedPeriod = s_periods;
}

void PeakController::updateCoeffs()
{
    m_coeffNeedsUpdate = true;
}

void PeakController::handleDestroyedEffect()
{
    // possible race condition...
    // printf("disconnecting effect\n");
    disconnect(m_peakEffect);
    m_peakEffect = nullptr;
    // deleteLater();
    delete this;
}

void PeakController::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    if(m_peakEffect != nullptr)
    {
        Controller::saveSettings(_doc, _this);
        _this.setAttribute("effectId", m_peakEffect->m_effectId);
        _this.setAttribute("target", m_peakEffect->uuid());
    }
    else
    {
        qWarning("PeakController::saveSettings effect is null");
    }
}

void PeakController::loadSettings(const QDomElement& _this)
{
    Controller::loadSettings(_this);

    int effectId = _this.attribute("effectId").toInt();
    if(s_buggedFile == true)
        effectId = s_loadCount++;

    /*
    PeakControllerEffectVector::Iterator i;
    for(i = s_effects.begin(); i != s_effects.end(); ++i)
    {
        if((*i)->m_effectId == effectId)
        {
            m_peakEffect = *i;
            return;
        }
    }
    */
    for(PeakControllerEffect* e: s_effects)
        if(e->m_effectId == effectId)
        {
            m_peakEffect = e;
            return;
        }

    if(_this.hasAttribute("target"))
    {
        QString target = _this.attribute("target");
        Model*  m      = Model::find(target);
        if(m != nullptr)
        {
            PeakControllerEffect* e = static_cast<PeakControllerEffect*>(m);
            if(e != nullptr)
            {
                m_peakEffect = e;
                return;
            }
        }
        qWarning("PeakController::loadSettings invalid target %s",
                 qPrintable(target));
    }

    qWarning("PeakController::loadSettings effect #%d not found", effectId);
    m_peakEffect = nullptr;
}

// Backward compatibility function for bug in <= 0.4.15
void PeakController::initGetControllerBySetting()
{
    s_loadCount  = 0;
    s_getCount   = 0;
    s_buggedFile = false;
}

PeakController*
        PeakController::getControllerBySetting(const QDomElement& _this)
{
    int effectId = _this.attribute("effectId").toInt();

    // PeakControllerEffectVector::Iterator i;
    // Backward compatibility for bug in <= 0.4.15 . For >= 1.0.0 ,
    // foundCount should always be 1 because m_effectId is initialized with
    // rand()
    int foundCount = 0;
    if(s_buggedFile == false)
    {
        /*
        for(i = s_effects.begin(); i != s_effects.end(); ++i)
            if((*i)->m_effectId == effectId)
                foundCount++;
        */
        for(PeakControllerEffect* e: s_effects)
            if(e->m_effectId == effectId)
                foundCount++;

        if(foundCount >= 2)
        {
            s_buggedFile    = true;
            int newEffectId = 0;
            /*
            for(i = s_effects.begin(); i != s_effects.end(); ++i)
            {
                (*i)->m_effectId = newEffectId++;
            }
            */
            for(PeakControllerEffect* e: s_effects)
                e->m_effectId = newEffectId++;

            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setWindowTitle(tr("Peak Controller Bug"));
            msgBox.setText(
                    tr("Due to a bug in older version of LMMS, the peak "
                       "controllers may not be connect properly. "
                       "Please ensure that peak controllers are connected "
                       "properly and re-save this file. "
                       "Sorry for any inconvenience caused."));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.exec();
        }
    }

    if(s_buggedFile == true)
        effectId = s_getCount;

    s_getCount++;  // NB: s_getCount should be increased even s_buggedFile is
                   // false

    /*
    for(i = s_effects.begin(); i != s_effects.end(); ++i)
    {
        if((*i)->m_effectId == effectId)
        {
            return (*i)->controller();
        }
    }
    */
    for(PeakControllerEffect* e: s_effects)
        if(e->m_effectId == effectId)
            return e->controller();

    return nullptr;
}

QString PeakController::nodeName() const
{
    return "Peakcontroller";
}

ControllerDialog* PeakController::createDialog(QWidget* _parent)
{
    return new PeakControllerDialog(this, _parent);
}
