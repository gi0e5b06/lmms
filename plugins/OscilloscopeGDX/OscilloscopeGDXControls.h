/*
 * OscilloscopeGDXControls.h -
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

#ifndef OSCILLOSCOPEGDX_CONTROLS_H
#define OSCILLOSCOPEGDX_CONTROLS_H

#include "AutomatableModel.h"
#include "ComboBoxModel.h"
#include "EffectControls.h"
#include "OscilloscopeGDXDialog.h"
#include "TempoSyncKnobModel.h"

class OscilloscopeGDX;

class OscilloscopeGDXControls : public EffectControls
{
    Q_OBJECT

  public:
    OscilloscopeGDXControls(OscilloscopeGDX* effect);
    virtual ~OscilloscopeGDXControls();

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "OscilloscopeGDXControls";
    }

    virtual int controlCount()
    {
        return 3;
    }

    virtual EffectControlDialog* createView()
    {
        return new OscilloscopeGDXDialog(this);
    }

    Ring* ring();
    //signals:
    //void nextStereoBuffer(const sampleFrame* _buf);

  private slots:
    void onControlChanged();

  private:
    OscilloscopeGDX* m_effect;

    // ComboBoxModel m_channelModel;
    // ComboBoxModel m_displayModel;
    // FloatModel m_zoomXModel;
    // FloatModel m_zoomYModel;

    friend class OscilloscopeGDXDialog;
    friend class OscilloscopeGDX;
};

#endif
