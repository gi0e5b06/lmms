/*
 * SpaceGDXControls.h - controls for wall effect
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

#ifndef SPACEGDX_CONTROLS_H
#define SPACEGDX_CONTROLS_H

#include "EffectControls.h"
#include "Knob.h"
#include "SpaceGDXDialog.h"

class SpaceGDXEffect;

class SpaceGDXControls : public EffectControls
{
    Q_OBJECT
  public:
    SpaceGDXControls(SpaceGDXEffect* effect);
    virtual ~SpaceGDXControls()
    {
    }

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "SpaceGDXControls";
    }

    virtual int controlCount()
    {
        return 3;
    }

    virtual EffectControlDialog* createView()
    {
        return new SpaceGDXDialog(this);
    }

  private slots:
    void changeControl();

  private:
    SpaceGDXEffect* m_effect;

    FloatModel m_rightPhaseModel;
    FloatModel m_rightGainModel;
    FloatModel m_rightLowModel;
    FloatModel m_rightHighModel;

    friend class SpaceGDXDialog;
    friend class SpaceGDXEffect;
};

#endif
