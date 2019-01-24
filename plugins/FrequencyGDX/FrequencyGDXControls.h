/*
 * FrequencyGDXControls.h -
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

#ifndef FREQUENCYGDX_CONTROLS_H
#define FREQUENCYGDX_CONTROLS_H

#include "AutomatableModel.h"
#include "ComboBoxModel.h"
#include "EffectControls.h"
#include "FrequencyGDXDialog.h"
#include "TempoSyncKnobModel.h"

class FrequencyGDX;

class FrequencyGDXControls : public EffectControls
{
    Q_OBJECT
  public:
    FrequencyGDXControls(FrequencyGDX* effect);
    virtual ~FrequencyGDXControls();

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "FrequencyGDXControls";
    }

    virtual int controlCount()
    {
        return 1;
    }

    virtual EffectControlDialog* createView()
    {
        return new FrequencyGDXDialog(this);
    }

  private slots:
    void changeControl();

  private:
    FrequencyGDX* m_effect;
    ComboBoxModel m_modeModel;

    FloatModel m_topFrequencyModel;
    FloatModel m_avgFrequencyModel;
    FloatModel m_mainFrequencyModel;

    FloatModel m_topKeyModel;
    FloatModel m_avgKeyModel;
    FloatModel m_mainKeyModel;

    FloatModel m_topNoteModel;
    FloatModel m_avgNoteModel;
    FloatModel m_mainNoteModel;

    friend class FrequencyGDXDialog;
    friend class FrequencyGDX;
};

#endif
