/*
 * MidiPort.h - abstraction of MIDI ports which are part of LMMS' MIDI
 *              sequencing system
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

#ifndef MIDI_PORT_H
#define MIDI_PORT_H

//#include "Midi.h"
//#include "MidiTime.h"
//#include "AutomatableModel.h"
#include "ComboBoxModel.h"

#include <QMap>
#include <QMenu>
#include <QPointer>
#include <QString>
#include <QVector>

class MidiClient;
class MidiEvent;
class MidiEventProcessor;
class MidiPort;
class MidiPortMenu;

typedef QVector<QPointer<MidiPort>> MidiPorts;

// class for abstraction of MIDI-port
class MidiPort final : public Model, public SerializingObject
{
    Q_OBJECT

    mapPropertyFromModel(bool, isReadable, setReadable, m_readableModel);
    mapPropertyFromModel(bool, isWritable, setWritable, m_writableModel);
    mapPropertyFromModel(int,
                         inputChannel,
                         setInputChannel,
                         m_inputChannelModel);
    mapPropertyFromModel(int,
                         outputChannel,
                         setOutputChannel,
                         m_outputChannelModel);
    // Keyboard
    mapPropertyFromModel(int,
                         fixedInputVelocity,
                         setFixedInputVelocity,
                         m_fixedInputVelocityModel);
    mapPropertyFromModel(int,
                         fixedOutputVelocity,
                         setFixedOutputVelocity,
                         m_fixedOutputVelocityModel);
    mapPropertyFromModel(int,
                         transposeInput,
                         setTransposeInput,
                         m_transposeInputModel);
    mapPropertyFromModel(int,
                         transposeOutput,
                         setTransposeOutput,
                         m_transposeOutputModel);
    mapPropertyFromModel(int,
                         fixedOutputNote,
                         setFixedOutputNote,
                         m_fixedOutputNoteModel);
    mapPropertyFromModel(int,
                         outputProgram,
                         setOutputProgram,
                         m_outputProgramModel);
    mapPropertyFromModel(int,
                         baseVelocity,
                         setBaseVelocity,
                         m_baseVelocityModel);

    // Controller
    mapPropertyFromModel(int,
                         inputController,
                         setInputController,
                         m_inputControllerModel);
    mapPropertyFromModel(int,
                         outputController,
                         setOutputController,
                         m_outputControllerModel);
    mapPropertyFromModel(int, widgetType, setWidgetType, m_widgetTypeModel);
    mapPropertyFromModel(int,
                         defaultInputValue,
                         setDefaultInputValue,
                         m_defaultInputValueModel);
    mapPropertyFromModel(int,
                         spreadInputValue,
                         setSpreadInputValue,
                         m_spreadInputValueModel);
    mapPropertyFromModel(int,
                         minInputValue,
                         setMinInputValue,
                         m_minInputValueModel);
    mapPropertyFromModel(int,
                         maxInputValue,
                         setMaxInputValue,
                         m_maxInputValueModel);
    mapPropertyFromModel(int,
                         stepInputValue,
                         setStepInputValue,
                         m_stepInputValueModel);
    mapPropertyFromModel(int,
                         baseInputValue,
                         setBaseInputValue,
                         m_baseInputValueModel);
    mapPropertyFromModel(int,
                         slopeInputValue,
                         setSlopeInputValue,
                         m_slopeInputValueModel);
    mapPropertyFromModel(int,
                         deltaInputValue,
                         setDeltaInputValue,
                         m_deltaInputValueModel);

  public:
    typedef QMap<QString, bool> Map;

    enum WidgetType
    {
        Ignored,
        OpenRelay,
        CloseRelay,
        Button,
        Switch,
        Knob,
        Slider,
        Pad,
        Key,
        Wheel,
        Selector,
        PitchBend
    };

    static const int   NB_WIDGET_TYPES;
    static const char* WIDGET_TYPE_NAME[];
    static WidgetType  findWidgetType(const QString& _s);

    enum Modes
    {
        Disabled,  // don't route any MIDI-events (default)
        Input,     // from MIDI-client to MIDI-event-processor
        Output,    // from MIDI-event-processor to MIDI-client
        Duplex     // both directions
    };
    typedef Modes Mode;

    MidiPort(const QString&      name,
             MidiClient*         client,
             MidiEventProcessor* eventProcessor,
             Model*              parent = nullptr,
             Mode                mode   = Disabled);
    virtual ~MidiPort();

    Mode mode() const
    {
        return m_mode;
    }

    void setName(const QString& name);
    void setMode(Mode mode);
    void reset();

    bool isInputEnabled() const
    {
        return mode() == Input || mode() == Duplex;
    }

    bool isOutputEnabled() const
    {
        return mode() == Output || mode() == Duplex;
    }

    /*
    int realOutputChannel() const
    {
            return outputChannel() - 1;
    }
    */

    void processInEvent(const MidiEvent& event,
                        const MidiTime&  time = MidiTime());
    void processOutEvent(const MidiEvent& event,
                         const MidiTime&  time = MidiTime());

    virtual void saveSettings(QDomDocument& doc, QDomElement& thisElement);
    virtual void loadSettings(const QDomElement& thisElement);

    virtual QString nodeName() const
    {
        return "midiport";
    }

    void subscribeReadablePorts();
    void subscribeWritablePorts();

    void subscribeReadablePorts(const MidiPort::Map& _map);
    void subscribeWritablePorts(const MidiPort::Map& _map);

    void subscribeReadablePort(const QString& port, bool subscribe = true);
    void subscribeWritablePort(const QString& port, bool subscribe = true);

    const Map& readablePorts() const
    {
        return m_readablePorts;
    }

    const Map& writablePorts() const
    {
        return m_writablePorts;
    }

    void setSingleReadablePort(const QString& _p);
    void setSingleWritablePort(const QString& _p);

    void invalidateClient();

  public slots:
    void updateMidiPortMode();

  private slots:
    void updateReadablePorts();
    void updateWritablePorts();
    void updateOutputProgram();

  private:
    MidiClient*         m_midiClient;
    MidiEventProcessor* m_midiEventProcessor;

    Mode m_mode;

    BoolModel m_readableModel;
    BoolModel m_writableModel;
    IntModel  m_inputChannelModel;
    IntModel  m_outputChannelModel;

    IntModel m_fixedInputVelocityModel;
    IntModel m_fixedOutputVelocityModel;
    IntModel m_transposeInputModel;
    IntModel m_transposeOutputModel;
    IntModel m_fixedOutputNoteModel;
    IntModel m_outputProgramModel;
    IntModel m_baseVelocityModel;

    IntModel      m_inputControllerModel;
    IntModel      m_outputControllerModel;
    ComboBoxModel m_widgetTypeModel;
    IntModel      m_defaultInputValueModel;
    IntModel      m_spreadInputValueModel;
    IntModel      m_minInputValueModel;
    IntModel      m_maxInputValueModel;
    IntModel      m_stepInputValueModel;
    IntModel      m_baseInputValueModel;
    IntModel      m_slopeInputValueModel;
    IntModel      m_deltaInputValueModel;

    Map m_readablePorts;
    Map m_writablePorts;

    friend class ControllerConnectionDialog;
    friend class InstrumentMidiIOView;
    friend class MidiPortMenu;

  signals:
    void readablePortsChanged();
    void writablePortsChanged();
    void modeChanged();
};

#endif
