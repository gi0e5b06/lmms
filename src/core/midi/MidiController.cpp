/*
 * MidiController.cpp - implementation of class midi-controller which handles
 *                      MIDI control change messages
 *
 * Copyright (c) 2008 Paul Giblock <drfaygo/at/gmail.com>
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

#include <QDomElement>
//#include <QObject>

#include "Engine.h"
#include "MidiClient.h"
#include "MidiController.h"
#include "Mixer.h"
//#include "Song.h"

#include "lmms_math.h"  // REQUIRED

MidiController::MidiController(Model* _parent) :
      Controller(Controller::MidiController, _parent, tr("MIDI Controller")),
      MidiEventProcessor(), m_midiPort(tr("unnamed_midi_controller"),
                                       Engine::mixer()->midiClient(),
                                       this,
                                       this,
                                       MidiPort::Input),
      m_lastValue(-1.), m_previousValue(-1.), m_switch(true)
{
    setSampleExact(true);
    connect(&m_midiPort, SIGNAL(modeChanged()), this, SLOT(updateName()));
}

MidiController::~MidiController()
{
}

void MidiController::fillValueBuffer()
{
    if(m_previousValue != m_lastValue)
    {
        m_valueBuffer.interpolate(m_previousValue, m_lastValue);
        m_previousValue = m_lastValue;
    }
    else
    {
        m_valueBuffer.fill(m_lastValue);
    }
}

void MidiController::updateName()
{
    int type = m_midiPort.widgetType();
    if(type == 10)
        setName(QString("MIDI ch%1 pitchbend")
                        .arg(m_midiPort.inputChannel()));
    else
        setName(QString("MIDI ch%1 ctrl%2")
                        .arg(m_midiPort.inputChannel())
                        .arg(m_midiPort.inputController()));
}

/*
void MidiController::updateLastValue()
{
    int type = m_midiPort.widgetType();
    if(type <= 4)
        m_previousValue = m_lastValue;
    int v=m_midiPort.defaultInputValue();
    m_lastValue=qBound(0,v,127)/127.;
    qInfo("MidiController::updateLastValue %d %f",v,m_lastValue);
    updateValueBuffer();
    emit controlledValueChanged(m_lastValue);
    emit propertiesChanged();
}
*/

void MidiController::processInEvent(const MidiEvent& event,
                                    const MidiTime&  time,
                                    f_cnt_t          offset)
{
    const int et = event.type();
    const int wt = m_midiPort.widgetType();

    int controllerNum  = event.controllerNumber();
    int selectorNum    = 0;
    int selectorSpread = m_midiPort.spreadInputValue();

    if(wt <= MidiPort::Ignored || wt >= MidiPort::NB_WIDGET_TYPES)
        return;
    if(wt == MidiPort::PitchBend && et != MidiPitchBend)
        return;
    if(wt != MidiPort::PitchBend && et == MidiPitchBend)
        return;
    if(wt == MidiPort::Selector
       && controllerNum + 1 >= m_midiPort.inputController()
       && controllerNum + 1 < m_midiPort.inputController() + selectorSpread)
    {
        selectorNum   = controllerNum + 1 - m_midiPort.inputController();
        controllerNum = m_midiPort.inputController() - 1;

        qInfo("MidiController: sn=%d cc=%d et=%d wt=%d %s", selectorNum,
              controllerNum, et, wt,
              MidiPort::WIDGET_TYPE_NAME[wt % MidiPort::NB_WIDGET_TYPES]);
    }

    if(((controllerNum + 1 >= m_midiPort.inputController()
         && controllerNum + 1 < m_midiPort.inputController() + selectorSpread)
        || et == MidiPitchBend)
       && (m_midiPort.inputChannel() == event.channel() + 1
           || m_midiPort.inputChannel() == 0))
    {
        double val = 0.;

        if(et == MidiControlChange)
            val = event.controllerValue();
        else if(et == MidiNoteOn)
            val = event.velocity();
        else if(et == MidiNoteOff)
            val = event.velocity();
        else if(et == MidiPitchBend)
            val = 127. * event.midiPitchBend() / 16383.;
        else
            // Don't care - maybe add special cases for mod later
            qWarning("MidiController: in event Default");
        if(wt == MidiPort::Ignored)
        {
            return;
        }
        else if(wt == MidiPort::OpenRelay)
        {
            if(et == MidiNoteOn)
                val = 0.;
            else if(et == MidiNoteOff)
                ;  // val=127;
            else
                return;
        }
        else if(wt == MidiPort::CloseRelay)
        {
            if(et == MidiNoteOn)
                ;  // val=127;
            else if(et == MidiNoteOff)
                val = 0.;
            else
                return;
        }
        else if(wt == MidiPort::Switch || wt == MidiPort::Button)
        {
            if(et != MidiNoteOn)
                return;

            val      = (m_switch ? val : 0.);
            m_switch = !m_switch;
        }
        else if(wt == MidiPort::Knob || wt == MidiPort::Slider)
        {
            if(et != MidiControlChange)
                return;
        }
        else if(wt == MidiPort::Pad || wt == MidiPort::Key)
        {
            if(et != MidiNoteOn)
                return;
        }
        else if(wt == MidiPort::Selector)
        {
            if(et != MidiNoteOn)
                return;

            val = double(selectorNum);  // / 7. * 127.;
            // qInfo("MidiController: selector val=%f", val);
        }

        {
            const double base  = m_midiPort.baseInputValue();
            const double slope = m_midiPort.slopeInputValue();
            const double delta = m_midiPort.deltaInputValue();
            const double vmin  = m_midiPort.minInputValue();
            const double vmax  = m_midiPort.maxInputValue();
            const double step  = m_midiPort.stepInputValue();

            val -= delta;
            if(step > 1.)
                val = floor(val / step) * step;

            val = base + slope * val;

            if(val < vmin)
                val = vmin;
            if(val > vmax)
                val = vmax;

            if(val < 0.)
                val = 0.;  // just for safety
            if(val > 127.)
                val = 127.;  // just for safety

            m_previousValue = m_lastValue;
            m_lastValue     = val / 127.;
            qInfo("MidiController: val=%f val=%d last=%f ~%d prev=%f", val, int(val),
                  m_lastValue, int(round(16 * m_lastValue)), m_previousValue);
        }

        if(m_previousValue != m_lastValue)
        {
            if(m_lastValue < 0. || wt <= MidiPort::Button
               || wt == MidiPort::Selector)
                m_previousValue = m_lastValue;
            updateValueBuffer();
            emit controlledValueChanged(m_lastValue);

            // qInfo("MidiController: val=%f m_lv=%f cc=%d sn=%d", val,
            //      m_lastValue, controllerNum, selectorNum);
        }
        else
        {
            // qWarning("MidiController: emit dataUnchanged");
            // emit dataUnchanged();
        }
    }
}

/*
void MidiController::subscribeReadablePorts(const MidiPort::Map& _map)
{
    m_midiPort.subscribeReadablePorts(_map);
}
*/

void MidiController::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    Controller::saveSettings(_doc, _this);
    m_midiPort.saveSettings(_doc, _this);
}

void MidiController::loadSettings(const QDomElement& _this)
{
    Controller::loadSettings(_this);
    m_midiPort.loadSettings(_this);
    updateName();
}

QString MidiController::nodeName() const
{
    return "Midicontroller";
}

ControllerDialog* MidiController::createDialog(QWidget* _parent)
{
    return nullptr;
}
