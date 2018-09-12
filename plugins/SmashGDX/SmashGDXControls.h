/*
 * SmashGDXControls.h - controls for wall effect
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

#ifndef SMASHGDX_CONTROLS_H
#define SMASHGDX_CONTROLS_H

#include "EffectControls.h"
#include "Knob.h"
#include "SmashGDXDialog.h"

class SmashGDXEffect;

class SmashGDXControls : public EffectControls
{
    Q_OBJECT
  public:
    SmashGDXControls(SmashGDXEffect* effect);
    virtual ~SmashGDXControls()
    {
    }

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "SmashGDXControls";
    }

    virtual int controlCount()
    {
        return 2;
    }

    virtual EffectControlDialog* createView()
    {
        return new SmashGDXDialog(this);
    }

  private slots:
    void changeControl();

  private:
    SmashGDXEffect* m_effect;
    FloatModel      m_rateModel;
    FloatModel      m_levelModel;
    //FloatModel      m_wetModel;

    friend class SmashGDXDialog;
    friend class SmashGDXEffect;
};

#endif
