/*
 * OutputGDXControls.h - controls for audio output properties
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

#ifndef OUTPUTGDX_CONTROLS_H
#define OUTPUTGDX_CONTROLS_H

#include "EffectControls.h"
#include "Knob.h"
#include "OutputGDXDialog.h"

class OutputGDX;

class OutputGDXControls : public EffectControls
{
    Q_OBJECT
  public:
    OutputGDXControls(OutputGDX* effect);
    virtual ~OutputGDXControls()
    {
    }

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "OutputGDXControls";
    }

    virtual int controlCount()
    {
        return 4;
    }

    virtual EffectControlDialog* createView()
    {
        return new OutputGDXDialog(this);
    }

  private slots:
    void changeControl();

  private:
    OutputGDX* m_effect;
    FloatModel       m_leftModel;
    FloatModel       m_rightModel;
    FloatModel       m_rmsModel;
    FloatModel       m_volModel;
    FloatModel       m_panModel;
    FloatModel       m_frequencyModel;

    friend class OutputGDXDialog;
    friend class OutputGDX;
};

#endif
