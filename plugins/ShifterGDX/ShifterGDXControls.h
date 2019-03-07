/*
 * ShifterGDXControls.h -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#ifndef SHIFTERGDX_CONTROLS_H
#define SHIFTERGDX_CONTROLS_H

#include "AutomatableModel.h"
#include "ComboBoxModel.h"
#include "EffectControls.h"
#include "ShifterGDXDialog.h"
//#include "TempoSyncKnobModel.h"

class ShifterGDX;

class ShifterGDXControls : public EffectControls
{
    Q_OBJECT
  public:
    ShifterGDXControls(ShifterGDX* effect);
    virtual ~ShifterGDXControls();

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "ShifterGDXControls";
    }

    virtual int controlCount()
    {
        return 7;
    }

    virtual EffectControlDialog* createView()
    {
        return new ShifterGDXDialog(this);
    }

  private slots:
    void changeControl();

  private:
    ShifterGDX* m_effect;

    FloatModel    m_deltaFrequencyModel;
    FloatModel    m_lowFrequencyModel;
    FloatModel    m_highFrequencyModel;
    FloatModel    m_slopeFrequencyModel;
    FloatModel    m_deltaVolumeModel;
    FloatModel    m_lowVolumeModel;
    FloatModel    m_highVolumeModel;
    FloatModel    m_slopeVolumeModel;

    friend class ShifterGDXDialog;
    friend class ShifterGDX;
};

#endif
