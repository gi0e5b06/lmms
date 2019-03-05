/*
 * LV2Control.h - model for controlling a LV2 port
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

#ifndef LV2_CONTROL_H
#define LV2_CONTROL_H

#include "lmmsconfig.h"

#ifdef LMMS_HAVE_LILV

// typedef float LV2_Data;
//#include <lv2.h>

#include "AutomatableModel.h"
#include "TempoSyncKnobModel.h"
//#include "ValueBuffer.h"
#include "LV2Base.h"

typedef struct LV2PortDescription lv2_port_desc_t;

class EXPORT LV2Control : public Model, public JournallingObject
{
    Q_OBJECT

  public:
    LV2Control(Model* _parent, lv2_port_desc_t* _port, bool _link = false);
    virtual ~LV2Control();

    LV2_Data     value();
    ValueBuffer* valueBuffer();
    void         setValue(LV2_Data _value);
    void         setLink(bool _state);

    void linkControls(LV2Control* _control);
    void unlinkControls(LV2Control* _control);

    inline BoolModel* toggledModel()
    {
        return &m_toggledModel;
    }

    inline FloatModel* knobModel()
    {
        return &m_knobModel;
    }

    inline TempoSyncKnobModel* tempoSyncKnobModel()
    {
        return &m_tempoSyncKnobModel;
    }

    inline lv2_port_desc_t* port()
    {
        return m_port;
    }

    virtual void saveSettings(QDomDocument&  _doc,
                              QDomElement&   _parent,
                              const QString& _name);
    virtual void loadSettings(const QDomElement& _this, const QString& _name);
    inline virtual QString nodeName() const
    {
        return "port";
    }

  signals:
    void changed(int _port, LV2_Data _value);
    void linkChanged(int _port, bool _state);

  protected slots:
    void ledChanged();
    void knobChanged();
    void tempoKnobChanged();
    void linkStateChanged();

  protected:
    virtual void saveSettings(QDomDocument& doc, QDomElement& element)
    {
        Q_UNUSED(doc)
        Q_UNUSED(element)
    }

    virtual void loadSettings(const QDomElement& element)
    {
        Q_UNUSED(element)
    }

  private:
    bool             m_link;
    lv2_port_desc_t* m_port;

    BoolModel          m_linkEnabledModel;
    BoolModel          m_toggledModel;
    FloatModel         m_knobModel;
    TempoSyncKnobModel m_tempoSyncKnobModel;

    friend class LV2ControlView;
};

#endif
#endif
