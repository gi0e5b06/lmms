/*
 * MidiMapper.cpp -
 *
 * Copyright (c) 2020      gi0e5b06 (on github.com)
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

#include "MidiMapper.h"

#include "AutomatableModel.h"
#include "ControllerConnection.h"
#include "MidiController.h"

int MidiMapper::list(QWidget* _w)
{
    QHash<ModelView*, Model*> table;
    collect(_w, table);

    int r = 0;
    for(ModelView* mv: table.keys())
    {
        AutomatableModel* am
                = dynamic_cast<AutomatableModel*>(table.value(mv));
        if(am == nullptr)
            continue;

        r++;
        qInfo("%-30s %-10s", qPrintable(am->fullObjectName()),
              am->isControlled() ? "controlled" : "-");
        // am->uuid(),
        // qInfo("___");
        // qInfo("ON  %s",qPrintable(am->objectName()));
        // qInfo("DN  %s",qPrintable(am->displayName()));
        // qInfo("FDN %s",qPrintable(am->fullDisplayName()));
    }
    return r;
}

int MidiMapper::map(QWidget* _w)
{
    QHash<ModelView*, Model*> table;
    collect(_w, table);

    int r = 0;
    for(ModelView* mv: table.keys())
    {
        AutomatableModel* am
                = dynamic_cast<AutomatableModel*>(table.value(mv));
        if(am == nullptr)
            continue;
        if(am->isControlled())
            continue;

        r++;
        qInfo("set-midi-connection %s", qPrintable(am->fullObjectName()));

        ControllerConnection* cc = am->controllerConnection();
        if(cc != nullptr)
            continue;

        MidiController* mc = new MidiController(am);
        //mc->updateReadablePorts();

        /*
        mc->m_midiPort.setInputChannel(m_midiPort.inputChannel());
        mc->m_midiPort.setInputController(m_midiPort.inputController());

        mc->m_midiPort.setWidgetType(4);
        mc->m_midiPort.setMinInputValue(0);
        mc->m_midiPort.setMaxInputValue(127);
        mc->m_midiPort.setStepInputValue(1);
        mc->m_midiPort.setBaseInputValue(0);
        mc->m_midiPort.setSlopeInputValue(1);
        mc->m_midiPort.setDeltaInputValue(0);

        mc->subscribeReadablePorts(m_midiPort.readablePorts());
        mc->updateName();
        */

        cc=new ControllerConnection(mc);
        am->setControllerConnection(cc);
    }
    return r;
}

int MidiMapper::unmap(QWidget* _w)
{
    QHash<ModelView*, Model*> table;
    collect(_w, table);

    int r = 0;
    for(ModelView* mv: table.keys())
    {
        AutomatableModel* am
                = dynamic_cast<AutomatableModel*>(table.value(mv));
        if(am == nullptr)
            continue;
        if(!am->isControlled())
            continue;
        ControllerConnection* cc = am->controllerConnection();
        if(cc == nullptr)
            continue;
        MidiController* mc = dynamic_cast<MidiController*>(cc->controller());
        if(mc == nullptr)
            continue;

        qInfo("unset-midi-connection %s", qPrintable(am->fullObjectName()));
        // am->unlinkControllerConnection();
        delete cc;
        am->setControllerConnection(nullptr);
    }

    return r;
}

void MidiMapper::collect(QWidget* _w, QHash<ModelView*, Model*>& _table)
{
    if(_w == nullptr)  // || _w == this)
        return;

    // if(!_w->isVisible()) return;
    // int oldsize = _table.size();

    ModelView* mv = dynamic_cast<ModelView*>(_w);
    if(mv != nullptr)
    {
        Model* m = dynamic_cast<Model*>(mv->model());
        if(m != nullptr)
            _table.insert(mv, m);

        for(ModelView* cmv: mv->childModelViews())
        {
            Model* cm = dynamic_cast<Model*>(cmv->model());
            if(cm != nullptr)
                _table.insert(cmv, cm);
        }
    }

    if(_w->isVisible())
        for(QObject* o: _w->children())
        {
            QWidget* w = dynamic_cast<QWidget*>(o);
            if(w == nullptr)
                continue;
            collect(w, _table);
        }

    /*
    int newsize = _table.size();
    if(oldsize != newsize)
        _w->installEventFilter(this);
    */
}
