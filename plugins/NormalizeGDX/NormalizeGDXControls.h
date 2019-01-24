/*
 * NormalizeGDXControls.h -
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

#ifndef NORMALIZEGDX_CONTROLS_H
#define NORMALIZEGDX_CONTROLS_H

#include "AutomatableModel.h"
#include "EffectControls.h"
#include "NormalizeGDXDialog.h"

class NormalizeGDX;

class NormalizeGDXControls : public EffectControls
{
    Q_OBJECT

  public:
    NormalizeGDXControls(NormalizeGDX* effect);
    virtual ~NormalizeGDXControls();

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "NormalizeGDXControls";
    }

    virtual int controlCount()
    {
        return 7;
    }

    virtual EffectControlDialog* createView()
    {
        return new NormalizeGDXDialog(this);
    }


  private:
    NormalizeGDX* m_effect;

    BoolModel m_skewEnabledModel;
    BoolModel m_volumeEnabledModel;
    BoolModel m_balanceEnabledModel;

    FloatModel m_skewSpeedModel;
    FloatModel m_volumeUpSpeedModel;
    FloatModel m_volumeDownSpeedModel;
    FloatModel m_balanceSpeedModel;

    FloatModel m_outGainModel;

    friend class NormalizeGDXDialog;
    friend class NormalizeGDX;
};

#endif
