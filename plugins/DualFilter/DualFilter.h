/*
 * DualFilter.h - dual filter effect-plugin
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef DUALFILTER_H
#define DUALFILTER_H

#include "BasicFilters.h"
#include "DualFilterControls.h"
#include "Effect.h"

class DualFilterEffect : public Effect
{
  public:
    DualFilterEffect(Model*                                    parent,
                     const Descriptor::SubPluginFeatures::Key* key);
    virtual ~DualFilterEffect();
    virtual bool processAudioBuffer(sampleFrame* buf, const fpp_t frames);

    virtual EffectControls* controls()
    {
        return &m_dfControls;
    }

  private:
    DualFilterControls m_dfControls;

    BasicFilters<2>* m_filter1;
    BasicFilters<2>* m_filter2;

    bool m_filter1changed;
    bool m_filter2changed;

    real_t m_currentCut1;
    real_t m_currentRes1;
    real_t m_currentCut2;
    real_t m_currentRes2;

    friend class DualFilterControls;
};

#endif
