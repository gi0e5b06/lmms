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
#include "MidiPort.h"
#include "embed.h"

int MidiMapper::list(QWidget* _w)
{
    QHash<ModelView*, Model*> table;
    collect(_w, table);

    int r = 0;
    for(ModelView* mv: table.keys())
    {
        Model* m = table.value(mv, nullptr);
        if(m == nullptr)
            continue;

        MidiEventProcessor* mep = dynamic_cast<MidiEventProcessor*>(m);
        if(mep != nullptr)
        {
            r++;
            QString details;
            if(mep->hasMidiIn())
                details += "in ";
            if(mep->hasMidiOut())
                details += "out ";
            qInfo("%-40s %-10s %s", qPrintable(m->fullObjectName()),
                  "processor", qPrintable(details));
        }

        AutomatableModel* am = dynamic_cast<AutomatableModel*>(m);
        if(am != nullptr)
        {
            r++;
            QString details;

            if(dynamic_cast<BoolModel*>(am))
                details += "bool  ";
            else if(dynamic_cast<IntModel*>(am))
                details += "int   ";
            else if(dynamic_cast<FloatModel*>(am))
                details += "float ";
            else
                details += "????? ";

            if(am->isControlled())
            {
                ControllerConnection* cc = am->controllerConnection();
                if(cc != nullptr)
                {

                    MidiController* mc
                            = dynamic_cast<MidiController*>(cc->controller());
                    if(mc != nullptr)
                    {
                        details += QString("ch %1, cc %2, type %3")
                                           .arg(mc->m_midiPort.inputChannel())
                                           .arg(mc->m_midiPort
                                                        .inputController())
                                           .arg(mc->m_midiPort.widgetType());
                    }
                }
            }

            qInfo("%-40s %-10s %s", qPrintable(am->fullObjectName()),
                  am->isControlled() ? "controlled" : "-",
                  qPrintable(details));
            // am->uuid(),
            // qInfo("___");
            // qInfo("ON  %s",qPrintable(am->objectName()));
            // qInfo("DN  %s",qPrintable(am->displayName()));
            // qInfo("FDN %s",qPrintable(am->fullDisplayName()));
        }
    }
    return r;
}

int MidiMapper::map(QWidget* _w, const QString& _p)
{
    QString s = _p.mid(_p.indexOf("from ") + 5);
    QString f = QString("midi/%1.prp").arg(s);
    f.replace(' ', '_');
    qWarning("MidiMapper: property filename: %s", qPrintable(f));

    QHash<QString, QString> h = embed::getProperties(f);
    if(h.size() == 0)
    {
        qWarning("MidiMapper: empty property file %s", qPrintable(f));
        return 0;
    }
    qWarning("MidiMapper: %d properties", h.size());

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

        ControllerConnection* cc = am->controllerConnection();
        if(cc != nullptr)
            continue;

        QString k = am->fullObjectName();
        if(!h.contains(k))
            continue;

        QString v = h.value(k);
        if(v.isEmpty())
            continue;

        r++;
        qInfo("set-midi-connection %s %s (%s)", qPrintable(k), qPrintable(v),
              qPrintable(_p));

        QVector<int> vv = {0, 0, 4, 64, 1, 0, 127, 1, 0, 1, 0};
        int          i  = 0;
        for(QString t: v.trimmed().split(','))
        {
            t = t.trimmed();
            if(i >= vv.size())
                break;
            if(i == 2)
            {
                vv[i] = MidiPort::findWidgetType(t);
                qInfo("                    %s -> %d", qPrintable(t), vv[i]);
            }
            else
                vv[i] = t.toInt();
            i++;
        }

        MidiController* mc = new MidiController(am);
        mc->m_midiPort.setSingleReadablePort(_p);
        // mc->m_midiPort.reset();
        mc->m_midiPort.setInputChannel(vv[0]);
        mc->m_midiPort.setInputController(vv[1]);
        mc->m_midiPort.setWidgetType(vv[2]);
        mc->m_midiPort.setDefaultInputValue(vv[3]);
        mc->m_midiPort.setSpreadInputValue(vv[4]);
        mc->m_midiPort.setMinInputValue(vv[5]);
        mc->m_midiPort.setMaxInputValue(vv[6]);
        mc->m_midiPort.setStepInputValue(vv[7]);
        mc->m_midiPort.setBaseInputValue(vv[8]);
        mc->m_midiPort.setSlopeInputValue(vv[9]);
        mc->m_midiPort.setDeltaInputValue(vv[10]);

        // mc->m_midiPort.subscribeReadablePorts();
        mc->updateName();
        // mc->m_midiPort.processInEvent
        // mc->updateLastValue();

        cc = new ControllerConnection(mc);
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
