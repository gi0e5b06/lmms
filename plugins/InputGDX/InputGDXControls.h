/*
 * InputGDXControls.h - controls for audio output properties
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

#ifndef INPUTGDX_CONTROLS_H
#define INPUTGDX_CONTROLS_H

#include "EffectControls.h"
#include "InputGDXDialog.h"
#include "Knob.h"

class InputGDX;

class InputGDXControls : public EffectControls
{
    Q_OBJECT
  public:
    InputGDXControls(InputGDX* effect);
    virtual ~InputGDXControls()
    {
    }

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "InputGDXControls";
    }

    virtual int controlCount()
    {
        return 5;
    }

    virtual EffectControlDialog* createView()
    {
        return new InputGDXDialog(this);
    }

  private:
    InputGDX*  m_effect;
    FloatModel m_leftSignalModel;
    FloatModel m_rightSignalModel;
    FloatModel m_volumeModel;
    FloatModel m_balanceModel;
    FloatModel m_mixingModel;
    FloatModel m_deltaModel;

    friend class InputGDXDialog;
    friend class InputGDX;
};

#endif
