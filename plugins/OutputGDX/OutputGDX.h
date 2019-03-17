/*
 * OutputGDX.h - audio output properties
 *
 * Copyright (c) 2018-219 gi0e5b06 (on github.com)
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

#ifndef OUTPUTGDX_H
#define OUTPUTGDX_H

#include "Effect.h"
#include "OutputGDXControls.h"
#include "ValueBuffer.h"
#include "lmms_math.h"

class OutputGDX : public Effect
{
    MM_OPERATORS
    Q_OBJECT

  public:
    OutputGDX(Model* parent, const Descriptor::SubPluginFeatures::Key* key);
    virtual ~OutputGDX();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_gdxControls;
    }

    virtual bool isSwitchable() const
    {
        return false;
    }

    virtual bool isWetDryable() const
    {
        return false;
    }

    virtual bool isGateable() const
    {
        return false;
    }

    virtual bool isBalanceable() const
    {
        return false;
    }

  signals:
    void sendRms(const real_t _v);
    void sendVol(const real_t _v);
    void sendPan(const real_t _v);
    void sendLeft(const ValueBuffer* _v);
    void sendRight(const ValueBuffer* _v);

  private:
    OutputGDXControls m_gdxControls;
    int               m_currentVB;
    ValueBuffer*      m_left[3];
    ValueBuffer*      m_right[3];

    friend class OutputGDXControls;
};

#endif
