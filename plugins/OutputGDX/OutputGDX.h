/*
 * OutputGDX.h - audio output properties
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

  signals:
    void sendRms(const float _v);
    void sendVol(const float _v);
    void sendPan(const float _v);
    void sendLeft(const ValueBuffer* _v);
    void sendRight(const ValueBuffer* _v);
    void sendFrequency(const float _v);

  private:
    OutputGDXControls m_gdxControls;
    ValueBuffer       m_left;
    ValueBuffer       m_right;

    friend class OutputGDXControls;
};

#endif
