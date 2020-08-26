/*
 * MidiPort.cpp - abstraction of MIDI-ports which are part of LMMS's MIDI-
 *                sequencing system
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "MidiPort.h"

#include "MidiClient.h"
#include "MidiDummy.h"
#include "MidiEventProcessor.h"
#include "MidiPortMenu.h"
#include "Note.h"
#include "Song.h"

#include <QDomElement>

static MidiDummy s_dummyClient;

const int MidiPort::NB_WIDGET_TYPES = PitchBend + 1;

const char* MidiPort::WIDGET_TYPE_NAME[]
        = {"Ignored", "Open Relay", "Close Relay", "Button",
           "Switch",  "Knob",       "Slider",      "Pad",
           "Key",     "Wheel",      "Selector",    "PitchBend"};

MidiPort::WidgetType MidiPort::findWidgetType(const QString& _s)
{
    int r = _s.toInt();
    if(r > 0 && r < NB_WIDGET_TYPES)
        return static_cast<MidiPort::WidgetType>(r);

    for(int i = 0; i < NB_WIDGET_TYPES; i++)
        if(_s == WIDGET_TYPE_NAME[i])
            return static_cast<MidiPort::WidgetType>(i);

    QString s = _s.toLower();
    for(int i = 0; i < NB_WIDGET_TYPES; i++)
        if(s == QString(WIDGET_TYPE_NAME[i]).toLower())
            return static_cast<MidiPort::WidgetType>(i);

    s.replace(" ", "");
    for(int i = 0; i < NB_WIDGET_TYPES; i++)
        if(s == QString(WIDGET_TYPE_NAME[i]).toLower().replace(" ", ""))
            return static_cast<MidiPort::WidgetType>(i);

    qInfo("Midi mapping: unrecognized widget type: '%s'", qPrintable(_s));
    return MidiPort::Ignored;
}

MidiPort::MidiPort(const QString&      name,
                   MidiClient*         client,
                   MidiEventProcessor* eventProcessor,
                   Model*              parent,
                   Mode                mode) :
      Model(parent, name.isEmpty() ? "[midi port]" : name),
      m_midiClient(client), m_midiEventProcessor(eventProcessor),
      m_mode(mode), m_readableModel(false, this, tr("Receive MIDI-events")),
      m_writableModel(false, this, tr("Send MIDI-events")),
      m_inputChannelModel(0, 0, MidiChannelCount, this, tr("Input channel")),
      m_outputChannelModel(
              1, 1, MidiChannelCount, this, tr("Output channel")),
      m_fixedInputVelocityModel(
              -1, -1, MidiMaxVelocity, this, tr("Fixed input velocity")),
      m_fixedOutputVelocityModel(
              -1, -1, MidiMaxVelocity, this, tr("Fixed output velocity")),
      m_transposeInputModel(0,
                            -NumKeys,
                            +NumKeys,
                            this,
                            tr("Transpose key"),
                            "transposeInputKey"),
      m_transposeOutputModel(0,
                             -NumKeys,
                             +NumKeys,
                             this,
                             tr("Transpose key"),
                             "transposeOutputKey"),
      m_fixedOutputNoteModel(
              -1, -1, MidiMaxKey, this, tr("Fixed output note")),
      m_outputProgramModel(
              1, 1, MidiProgramCount, this, tr("Output MIDI program")),
      m_baseVelocityModel(MidiMaxVelocity / 2,
                          1,
                          MidiMaxVelocity,
                          this,
                          tr("Base velocity")),
      m_inputControllerModel(
              0, 0, MidiControllerCount, this, tr("Input controller")),
      m_outputControllerModel(
              0, 0, MidiControllerCount, this, tr("Output controller")),
      m_widgetTypeModel(this, tr("Widget type")),
      m_defaultInputValueModel(64, 0, 127, this, tr("Default input value")),
      m_spreadInputValueModel(1, 1, 128, this, tr("Spread input value")),
      m_minInputValueModel(0, 0, 127, this, tr("Min input value")),
      m_maxInputValueModel(127, 0, 127, this, tr("Max input value")),
      m_stepInputValueModel(1, 1, 127, this, tr("Step input value")),
      m_baseInputValueModel(0, 0, 127, this, tr("Base input value")),
      m_slopeInputValueModel(1, 1, 7, this, tr("Slope input value")),
      m_deltaInputValueModel(0, 0, 127, this, tr("Delta input value"))
{
    for(int i = 0; i < MidiPort::NB_WIDGET_TYPES; i++)
        m_widgetTypeModel.addItem(MidiPort::WIDGET_TYPE_NAME[i]);

    /*
    m_widgetTypeModel.addItem("Ignored");      // 0
    m_widgetTypeModel.addItem("Open Relay");   // 1
    m_widgetTypeModel.addItem("Close Relay");  // 2
    m_widgetTypeModel.addItem("Button");       // 3
    m_widgetTypeModel.addItem("Switch");       // 4
    m_widgetTypeModel.addItem("Knob");         // 5
    m_widgetTypeModel.addItem("Fader");        // 6
    m_widgetTypeModel.addItem("Pad");          // 7
    m_widgetTypeModel.addItem("Key");          // 8
    m_widgetTypeModel.addItem("Wheel");        // 9
    m_widgetTypeModel.addItem("Selector");     // 10
    m_widgetTypeModel.addItem("PitchBend");    // 11
    */

    m_widgetTypeModel.setInitValue(MidiPort::Knob);

    m_midiClient->addPort(this);

    m_readableModel.setValue(m_mode == Input || m_mode == Duplex);
    m_writableModel.setValue(m_mode == Output || m_mode == Duplex);

    connect(&m_readableModel, SIGNAL(dataChanged()), this,
            SLOT(updateMidiPortMode()));
    connect(&m_writableModel, SIGNAL(dataChanged()), this,
            SLOT(updateMidiPortMode()));
    connect(&m_outputProgramModel, SIGNAL(dataChanged()), this,
            SLOT(updateOutputProgram()));

    // when using with non-raw-clients we can provide buttons showing
    // our port-menus when being clicked
    if(m_midiClient->isRaw() == false)
    {
        updateReadablePorts();
        updateWritablePorts();

        // we want to get informed about port-changes!
        m_midiClient->connectRPChanged(this, SLOT(updateReadablePorts()));
        m_midiClient->connectWPChanged(this, SLOT(updateWritablePorts()));
    }

    updateMidiPortMode();
}

MidiPort::~MidiPort()
{
    qInfo("MidiPort::~MidiPort START");

    // unsubscribe ports
    m_readableModel.setValue(false);
    m_writableModel.setValue(false);

    // and finally unregister ourself
    m_midiClient->removePort(this);

    qInfo("MidiPort::~MidiPort END");
}

void MidiPort::setName(const QString& name)
{
    setDisplayName(name);
    m_midiClient->applyPortName(this);
}

void MidiPort::setMode(Mode mode)
{
    m_mode = mode;
    m_midiClient->applyPortMode(this);
}

void MidiPort::reset()
{
    setInputChannel(0);
    setInputController(0);
    setWidgetType(5);
    setDefaultInputValue(64);
    setSpreadInputValue(1);
    setMinInputValue(0);
    setMaxInputValue(127);
    setStepInputValue(1);
    setBaseInputValue(0);
    setSlopeInputValue(1);
    setDeltaInputValue(0);
}

void MidiPort::processInEvent(const MidiEvent& event, const MidiTime& time)
{
    // mask event
    if(isInputEnabled()
       && (inputChannel() == 0 || inputChannel() - 1 == event.channel()))
    {
        MidiEvent e = event;

        if(e.type() == MidiNoteOn || e.type() == MidiNoteOff
           || e.type() == MidiKeyPressure)
        {
            if(fixedInputVelocity() >= 0 && e.velocity() > 0)
                e.setVelocity(fixedInputVelocity());
            if(transposeInput() != 0)
                e.setKey(e.key() + transposeInput());
        }

        /*
        static MidiEvent prev;
        static tick_t    prtime;
        if((prev == inEvent)&&(abs(prtime-time.getTicks())<16))
        {
                qWarning("MidiPort: skip duplicate in event");
        }
        else
        */
        {
            // qInfo("MidiPort: process in event t=%d
            // pt=%d",time.getTicks(),prtime); qInfo("MidiPort: process in
            // event"); prev=inEvent; prtime=time.getTicks();
            if(m_midiEventProcessor != nullptr)
                m_midiEventProcessor->processInEvent(e, time);
        }
    }
}

void MidiPort::processOutEvent(const MidiEvent& event, const MidiTime& time)
{
    // mask event
    if(isOutputEnabled()
       && (outputChannel() == 0 || outputChannel() - 1 == event.channel()))
    {
        MidiEvent e = event;

        if(e.type() == MidiNoteOn || e.type() == MidiNoteOff
           || e.type() == MidiKeyPressure)
        {
            if(fixedOutputVelocity() >= 0 && e.velocity() > 0)
                e.setVelocity(fixedOutputVelocity());

            if(fixedOutputNote() >= 0)
                e.setKey(fixedOutputNote());
            else if(transposeOutput() != 0)
                e.setKey(e.key() + transposeOutput());
        }

        m_midiClient->processOutEvent(e, time, this);
    }
}

void MidiPort::saveSettings(QDomDocument& doc, QDomElement& thisElement)
{
    m_inputChannelModel.saveSettings(doc, thisElement, "inputchannel");
    m_outputChannelModel.saveSettings(doc, thisElement, "outputchannel");
    m_inputControllerModel.saveSettings(doc, thisElement, "inputcontroller");
    m_outputControllerModel.saveSettings(doc, thisElement,
                                         "outputcontroller");
    m_fixedInputVelocityModel.saveSettings(doc, thisElement,
                                           "fixedinputvelocity");
    m_fixedOutputVelocityModel.saveSettings(doc, thisElement,
                                            "fixedoutputvelocity");
    m_transposeInputModel.saveSettings(doc, thisElement, "transposeinput");
    m_transposeOutputModel.saveSettings(doc, thisElement, "transposeoutput");
    m_fixedOutputNoteModel.saveSettings(doc, thisElement, "fixedoutputnote");
    m_outputProgramModel.saveSettings(doc, thisElement, "outputprogram");
    m_baseVelocityModel.saveSettings(doc, thisElement, "basevelocity");
    m_readableModel.saveSettings(doc, thisElement, "readable");
    m_writableModel.saveSettings(doc, thisElement, "writable");

    m_widgetTypeModel.saveSettings(doc, thisElement, "ivtype");
    m_defaultInputValueModel.saveSettings(doc, thisElement, "ivdefault");
    m_spreadInputValueModel.saveSettings(doc, thisElement, "ivspread");
    m_minInputValueModel.saveSettings(doc, thisElement, "ivmin");
    m_maxInputValueModel.saveSettings(doc, thisElement, "ivmax");
    m_stepInputValueModel.saveSettings(doc, thisElement, "ivstep");
    m_baseInputValueModel.saveSettings(doc, thisElement, "ivbase");
    m_slopeInputValueModel.saveSettings(doc, thisElement, "ivslope");
    m_deltaInputValueModel.saveSettings(doc, thisElement, "ivdelta");

    if(isInputEnabled())
    {
        QString rp;
        for(Map::ConstIterator it = m_readablePorts.begin();
            it != m_readablePorts.end(); ++it)
        {
            if(it.value())
            {
                rp += it.key() + ",";
            }
        }
        // cut off comma
        if(rp.length() > 0)
        {
            rp.truncate(rp.length() - 1);
        }
        thisElement.setAttribute("inports", rp);
    }

    if(isOutputEnabled())
    {
        QString wp;
        for(Map::ConstIterator it = m_writablePorts.begin();
            it != m_writablePorts.end(); ++it)
        {
            if(it.value())
            {
                wp += it.key() + ",";
            }
        }
        // cut off comma
        if(wp.length() > 0)
        {
            wp.truncate(wp.length() - 1);
        }
        thisElement.setAttribute("outports", wp);
    }
}

void MidiPort::loadSettings(const QDomElement& thisElement)
{
    m_inputChannelModel.loadSettings(thisElement, "inputchannel");
    m_outputChannelModel.loadSettings(thisElement, "outputchannel");
    m_inputControllerModel.loadSettings(thisElement, "inputcontroller");
    m_outputControllerModel.loadSettings(thisElement, "outputcontroller");
    m_fixedInputVelocityModel.loadSettings(thisElement, "fixedinputvelocity");
    m_fixedOutputVelocityModel.loadSettings(thisElement,
                                            "fixedoutputvelocity");
    m_transposeInputModel.loadSettings(thisElement, "transposeinput");
    m_transposeOutputModel.loadSettings(thisElement, "transposeoutput");
    m_outputProgramModel.loadSettings(thisElement, "outputprogram");
    m_baseVelocityModel.loadSettings(thisElement, "basevelocity");
    m_readableModel.loadSettings(thisElement, "readable");
    m_writableModel.loadSettings(thisElement, "writable");

    m_widgetTypeModel.loadSettings(thisElement, "ivtype");
    m_defaultInputValueModel.loadSettings(thisElement, "ivdefault");
    m_spreadInputValueModel.loadSettings(thisElement, "ivspread");
    m_minInputValueModel.loadSettings(thisElement, "ivmin");
    m_maxInputValueModel.loadSettings(thisElement, "ivmax");
    m_stepInputValueModel.loadSettings(thisElement, "ivstep");
    m_baseInputValueModel.loadSettings(thisElement, "ivbase");
    m_slopeInputValueModel.loadSettings(thisElement, "ivslope");
    m_deltaInputValueModel.loadSettings(thisElement, "ivdelta");

    // restore connections

    if(isInputEnabled())
    {
        QStringList rp = thisElement.attribute("inports").split(',');
        for(Map::ConstIterator it = m_readablePorts.begin();
            it != m_readablePorts.end(); ++it)
        {
            if(it.value() != (rp.indexOf(it.key()) != -1))
            {
                subscribeReadablePort(it.key());
            }
        }
        emit readablePortsChanged();
    }

    if(isOutputEnabled())
    {
        QStringList wp = thisElement.attribute("outports").split(',');
        for(Map::ConstIterator it = m_writablePorts.begin();
            it != m_writablePorts.end(); ++it)
        {
            if(it.value() != (wp.indexOf(it.key()) != -1))
            {
                subscribeWritablePort(it.key());
            }
        }
        emit writablePortsChanged();
    }

    if(thisElement.hasAttribute("basevelocity") == false)
    {
        // for projects created by LMMS < 0.9.92 there's no value for the base
        // velocity and for compat reasons we have to stick with maximum
        // velocity which did not allow note volumes > 100%
        m_baseVelocityModel.setValue(MidiMaxVelocity);
    }
}

void MidiPort::subscribeReadablePorts()
{
    subscribeReadablePorts(readablePorts());
}

void MidiPort::subscribeWritablePorts()
{
    subscribeWritablePorts(writablePorts());
}

void MidiPort::subscribeReadablePorts(const MidiPort::Map& _map)
{
    for(MidiPort::Map::ConstIterator it = _map.constBegin();
        it != _map.constEnd(); ++it)
        subscribeReadablePort(it.key(), *it);
}

void MidiPort::subscribeWritablePorts(const MidiPort::Map& _map)
{
    for(MidiPort::Map::ConstIterator it = _map.constBegin();
        it != _map.constEnd(); ++it)
        subscribeWritablePort(it.key(), *it);
}

void MidiPort::subscribeReadablePort(const QString& port, bool subscribe)
{
    m_readablePorts[port] = subscribe;

    // make sure, MIDI-port is configured for input
    if(subscribe == true && !isInputEnabled())
        m_readableModel.setValue(true);

    // qInfo("MidiPort::subscribeReadablePort src=%s subscribe=%d",
    //      qPrintable(port), subscribe);
    m_midiClient->subscribeReadablePort(this, port, subscribe);
}

void MidiPort::subscribeWritablePort(const QString& port, bool subscribe)
{
    m_writablePorts[port] = subscribe;

    // make sure, MIDI-port is configured for output
    if(subscribe == true && !isOutputEnabled())
        m_writableModel.setValue(true);

    // qInfo("MidiPort::subscribeWritablePort dest=%s subscribe=%d",
    //      qPrintable(port), subscribe);
    m_midiClient->subscribeWritablePort(this, port, subscribe);
}

void MidiPort::setSingleReadablePort(const QString& _p)
{
    m_readablePorts.clear();
    /*
      for(const QString& p: m_midiClient->readablePorts())
        m_readablePorts[p] = (p == _p);
    */
    m_readablePorts[_p] = true;
    subscribeReadablePorts();
}

void MidiPort::setSingleWritablePort(const QString& _p)
{
    m_writablePorts.clear();
    /*
      for(const QString& p: m_midiClient->writablePorts())
      m_writablePorts[p] = (p == _p);
    */
    m_writablePorts[_p] = true;
    subscribeWritablePorts();
}

void MidiPort::updateMidiPortMode()
{
    // this small lookup-table makes everything easier
    static const Modes modeTable[2][2]
            = {{Disabled, Output}, {Input, Duplex}};
    setMode(modeTable[m_readableModel.value()][m_writableModel.value()]);

    // check whether we have to dis-check items in connection-menu
    if(!isInputEnabled())
    {
        for(Map::ConstIterator it = m_readablePorts.begin();
            it != m_readablePorts.end(); ++it)
        {
            // subscribed?
            if(it.value())
            {
                subscribeReadablePort(it.key(), false);
            }
        }
    }

    if(!isOutputEnabled())
    {
        for(Map::ConstIterator it = m_writablePorts.begin();
            it != m_writablePorts.end(); ++it)
        {
            // subscribed?
            if(it.value())
            {
                subscribeWritablePort(it.key(), false);
            }
        }
    }

    emit readablePortsChanged();
    emit writablePortsChanged();
    emit modeChanged();

    // if(Engine::song())
    //    Engine::song()->setModified();
}

void MidiPort::updateReadablePorts()
{
    // first save all selected ports
    QStringList selectedPorts;
    for(Map::ConstIterator it = m_readablePorts.begin();
        it != m_readablePorts.end(); ++it)
    {
        if(it.value())
        {
            selectedPorts.push_back(it.key());
        }
    }

    m_readablePorts.clear();
    const QStringList& wp = m_midiClient->readablePorts();
    // now insert new ports and restore selections
    for(QStringList::ConstIterator it = wp.begin(); it != wp.end(); ++it)
    {
        m_readablePorts[*it] = (selectedPorts.indexOf(*it) != -1);
    }

    emit readablePortsChanged();
}

void MidiPort::updateWritablePorts()
{
    // first save all selected ports
    QStringList selectedPorts;
    for(Map::ConstIterator it = m_writablePorts.begin();
        it != m_writablePorts.end(); ++it)
    {
        if(it.value())
        {
            selectedPorts.push_back(it.key());
        }
    }

    m_writablePorts.clear();
    const QStringList& wp = m_midiClient->writablePorts();
    // now insert new ports and restore selections
    for(QStringList::ConstIterator it = wp.begin(); it != wp.end(); ++it)
    {
        m_writablePorts[*it] = (selectedPorts.indexOf(*it) != -1);
    }

    emit writablePortsChanged();
}

void MidiPort::updateOutputProgram()
{
    processOutEvent(MidiEvent(MidiProgramChange, outputChannel() - 1,
                              outputProgram() - 1));
}

void MidiPort::invalidateClient()
{
    m_midiClient = &s_dummyClient;
}
