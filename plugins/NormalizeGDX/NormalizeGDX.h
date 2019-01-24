/*
 * NormalizeGDX.h -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#ifndef NORMALIZEGDX_H
#define NORMALIZEGDX_H

#include "Effect.h"
#include "NormalizeGDXControls.h"
#include "lmms_math.h"

class PLUGIN_EXPORT NormalizeGDX : public Effect
{
  public:
    NormalizeGDX(Model* parent, const Descriptor::SubPluginFeatures::Key* key);
    virtual ~NormalizeGDX();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_gdxControls;
    }

  private:
    NormalizeGDXControls m_gdxControls;

    real_t m_skew;
    real_t m_volume;
    real_t m_balance;

    friend class NormalizeGDXControls;
};

#endif
