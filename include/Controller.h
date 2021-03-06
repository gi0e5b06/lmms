/*
 * Controller.h - declaration of class controller, which provides a
 *                standard for all controllers and controller plugins
 *
 * Copyright (c) 2008-2009 Paul Giblock <pgllama/at/gmail.com>
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

#ifndef CONTROLLER_H
#define CONTROLLER_H

//#include "Engine.h"
//#include "Model.h"
#include "AutomatableModel.h"
//#include "JournallingObject.h"
//#include "lmms_math.h"
#include "templates.h"
//#include "ValueBuffer.h"

class ControllerDialog;
class Controller;
class ControllerConnection;

typedef QVector<Controller*> Controllers;

class Controller : public Model, public JournallingObject
{
    Q_OBJECT
  public:
    enum ControllerTypes
    {
        DummyController,
        LfoController,
        MidiController,
        PeakController,
        /*
        XYController,
        EquationController
        */
        NumControllerTypes
    };

    Controller(ControllerTypes _type,
               Model*          _parent,
               const QString&  _displayName);

    virtual ~Controller();

    virtual real_t currentValue(int _offset = 0);
    // The per-controller get-value-in-buffers function
    // virtual ValueBuffer * valueBuffer();
    // virtual bool hasChanged() const;

    INLINE bool isSampleExact() const
    {
        return m_sampleExact;
    }

    void setSampleExact(bool _exact)
    {
        m_sampleExact = _exact;
    }

    INLINE ControllerTypes type() const
    {
        return m_type;
    }

    virtual const QString& name() const
    {
        return m_name;
    }

    INLINE bool isEnabled() const
    {
        return m_enabledModel.value();
    }

    virtual void    saveSettings(QDomDocument& _doc, QDomElement& _this);
    virtual void    loadSettings(const QDomElement& _this);
    virtual QString nodeName() const;

    static Controller* create(ControllerTypes _tt, Model* _parent);
    static Controller* create(const QDomElement& _this, Model* _parent);

    INLINE static real_t fittedValue(real_t _val)
    {
        return bound(0., _val, 1.);
        // return tLimit<real_t>(_val, 0., 1.);
    }

    static long runningPeriods()
    {
        return s_periods;
    }
    static unsigned int runningFrames();
    static real_t       runningTime();

    static void triggerFrameCounter();
    static void resetFrameCounter();

    // Accepts a ControllerConnection * as it may be used in the future.
    void addConnection(ControllerConnection*);
    void removeConnection(ControllerConnection*);
    int  connectionCount() const;

    bool hasModel(const Model* m) const;

  public slots:
    virtual ControllerDialog* createDialog(QWidget* _parent);

    virtual void setName(const QString& _new_name)
    {
        m_name = _new_name;
    }

  protected:
    virtual bool isConfigurable()
    {
        return true;
    }
    virtual bool isRemovable()
    {
        return true;
    }

    // The internal per-controller get-value function
    virtual real_t value(int _offset) final;

    virtual void updateValueBuffer() final;
    virtual void fillValueBuffer();

    // buffer for storing sample-exact values in case there
    // are more than one model wanting it, so we don't have to create it
    // again every time
    ValueBuffer m_valueBuffer;
    // when we last updated the valuebuffer - so we know if we have to update
    // it
    long m_lastUpdatedPeriod;
    // bool  m_hasChanged;
    real_t m_currentValue;
    bool   m_sampleExact;
    int    m_connectionCount;

    QString         m_name;
    ControllerTypes m_type;

    BoolModel m_enabledModel;

    static Controllers s_controllers;

    static long s_periods;

  signals:
    // The value changed while the mixer isn't running (i.e: MIDI CC)
    // void valueChanged();
    void controlledValueChanged(real_t _v);
    void controlledBufferChanged(const ValueBuffer* _vb);

    friend class ControllerView;
    friend class ControllerDialog;
    friend class ControllerConnection;
};

#endif
