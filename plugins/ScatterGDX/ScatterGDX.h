/*
 * ScatterGDX.h - scatter effect
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
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

#ifndef SCATTERGDX_H
#define SCATTERGDX_H

#include "Effect.h"
#include "ScatterGDXControls.h"
#include "ValueBuffer.h"
#include "lmms_math.h"

class ScatterGDXEffect : public Effect
{
  public:
    ScatterGDXEffect(Model*                                    parent,
                     const Descriptor::SubPluginFeatures::Key* key);
    virtual ~ScatterGDXEffect();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_gdxControls;
    }

  protected:
  private:
    ScatterGDXControls m_gdxControls;
    sampleFrame*       m_buffer;
    uint32_t           m_len, m_prev;
    int32_t            m_pos, m_time;
    int32_t            m_start, m_end;

    friend class ScatterGDXControls;
};

#endif
