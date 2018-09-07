/*
 * ChainGDX.h - A chaining effect
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

#ifndef CHAINGDX_H
#define CHAINGDX_H

#include "ChainGDXControls.h"
#include "Effect.h"
#include "EffectChain.h"
//#include "ValueBuffer.h"
//#include "lmms_math.h"

class PLUGIN_EXPORT ChainGDXEffect : public Effect
{
  public:
    ChainGDXEffect(Model*                                    parent,
                   const Descriptor::SubPluginFeatures::Key* key);
    virtual ~ChainGDXEffect();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_gdxControls;
    }

  private:
    ChainGDXControls m_gdxControls;
    EffectChain*     m_chain;

    friend class ChainGDXDialog;
    friend class ChainGDXControls;
};

#endif
