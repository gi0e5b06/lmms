/*
 * carla.cpp - Carla for LSMM
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (C) 2014      Filipe Coelho <falktx@falktx.com>
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

#include "carla.h"

#define REAL_BUILD  // FIXME this shouldn't be needed
#include "CarlaHost.h"
#include "Engine.h"
#include "GroupBox.h"
#include "InstrumentPlayHandle.h"
#include "InstrumentTrack.h"
#include "Knob.h"
#include "Mixer.h"
#include "Song.h"
#include "embed.h"
#include "gui_templates.h"

#include <QApplication>
#include <QCompleter>
#include <QFileDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QStringListModel>
#include <QTimerEvent>
#include <QVBoxLayout>

#include <cstring>

//#define CARLA_MAX_KNOBS            32
#define CARLA_SETTING_PREFIX "PARAM_KNOB_"

// this doesn't seem to be defined anywhere
static const double ticksPerBeat = 48.0;

/*
 * Current TODO items:
 *  - get plugin instance name (to use in external window title)
 *  - offline mode change callback
 *  - midi output
 *
 * All other items are to be done in Carla itself.
 */

// -----------------------------------------------------------------------

#define handlePtr ((CarlaInstrument*)handle)

static uint32_t host_get_buffer_size(NativeHostHandle handle)
{
    return handlePtr->handleGetBufferSize();
}

static double host_get_sample_rate(NativeHostHandle handle)
{
    return handlePtr->handleGetSampleRate();
}

static bool host_is_offline(NativeHostHandle handle)
{
    return handlePtr->handleIsOffline();
}

static const NativeTimeInfo* host_get_time_info(NativeHostHandle handle)
{
    return handlePtr->handleGetTimeInfo();
}

static bool host_write_midi_event(NativeHostHandle, const NativeMidiEvent*)
{
    return false;  // unsupported?
}

static void host_ui_parameter_changed(NativeHostHandle handle,
                                      uint32_t         index,
                                      FLOAT            value)
{
    handlePtr->handleUiParameterChanged(index, value);
}

static void host_ui_custom_data_changed(NativeHostHandle handle,
                                        const char*      key,
                                        const char*      value)
{
    // unused
}

static void host_ui_closed(NativeHostHandle handle)
{
    handlePtr->handleUiClosed();
}

static intptr_t host_dispatcher(NativeHostHandle           handle,
                                NativeHostDispatcherOpcode opcode,
                                int32_t                    index,
                                intptr_t                   value,
                                void*                      ptr,
                                FLOAT                      opt)
{
    return handlePtr->handleDispatcher(opcode, index, value, ptr, opt);
}

#undef handlePtr

// -----------------------------------------------------------------------

static const char* host_ui_open_file(NativeHostHandle,
                                     bool        isDir,
                                     const char* title,
                                     const char* filter)
{
    static QByteArray          retStr;
    const QFileDialog::Options options(isDir ? QFileDialog::ShowDirsOnly
                                             : 0x0);

    retStr = QFileDialog::getOpenFileName(QApplication::activeWindow(), title,
                                          "", filter, nullptr, options)
                     .toUtf8();

    return retStr.isEmpty() ? nullptr : retStr.constData();
}

static const char* host_ui_save_file(NativeHostHandle,
                                     bool        isDir,
                                     const char* title,
                                     const char* filter)
{
    static QByteArray          retStr;
    const QFileDialog::Options options(isDir ? QFileDialog::ShowDirsOnly
                                             : 0x0);

    retStr = QFileDialog::getSaveFileName(QApplication::activeWindow(), title,
                                          "", filter, nullptr, options)
                     .toUtf8();

    return retStr.isEmpty() ? nullptr : retStr.constData();
}

// -----------------------------------------------------------------------

CARLA_EXPORT
const NativePluginDescriptor* carla_get_native_patchbay_plugin();

CARLA_EXPORT
const NativePluginDescriptor* carla_get_native_rack_plugin();

// -----------------------------------------------------------------------

CarlaInstrument::CarlaInstrument(InstrumentTrack* const  instrumentTrack,
                                 const Descriptor* const descriptor,
                                 const bool              isPatchbay) :
      Instrument(instrumentTrack, descriptor),
      kIsPatchbay(isPatchbay), fHandle(nullptr),
      fDescriptor(isPatchbay ? carla_get_native_patchbay_plugin()
                             : carla_get_native_rack_plugin()),
      fMidiEventCount(0)
{
    fHost.handle     = this;
    fHost.uiName     = nullptr;
    fHost.uiParentId = 0;

    // figure out prefix from dll filename
    QString dllName(carla_get_library_filename());

#if defined(CARLA_OS_LINUX)
    fHost.resourceDir = strdup(QString(dllName.split("/lib/carla")[0]
                                       + "/share/carla/resources/")
                                       .toUtf8()
                                       .constData());
#else
    fHost.resourceDir = nullptr;
#endif

    fHost.get_buffer_size        = host_get_buffer_size;
    fHost.get_sample_rate        = host_get_sample_rate;
    fHost.is_offline             = host_is_offline;
    fHost.get_time_info          = host_get_time_info;
    fHost.write_midi_event       = host_write_midi_event;
    fHost.ui_parameter_changed   = host_ui_parameter_changed;
    fHost.ui_custom_data_changed = host_ui_custom_data_changed;
    fHost.ui_closed              = host_ui_closed;
    fHost.ui_open_file           = host_ui_open_file;
    fHost.ui_save_file           = host_ui_save_file;
    fHost.dispatcher             = host_dispatcher;

    std::memset(&fTimeInfo, 0, sizeof(NativeTimeInfo));
    fTimeInfo.bbt.valid = true;  // always valid

    fHandle = fDescriptor->instantiate(&fHost);
    Q_ASSERT(fHandle != nullptr);

    if(fHandle != nullptr && fDescriptor->activate != nullptr)
        fDescriptor->activate(fHandle);

    /*
    for(int i = 0; i < NB_MIDI_KNOBS; i++)
    {
        m_midiKnobs[i] = new FloatModel(
                0., 0., 127., 1., nullptr,
                QString("KNB%1").arg(NB_MIDI_KNOB_START + i), false);
        m_midiKnobs[i]->setStrictStepSize(true);
    }
    for(int i=0;i<NB_MIDI_LEDS;i++)
    {
            m_midiLeds[i]=new BoolModel
                    (false,nullptr,QString("LED%1").arg(NB_MIDI_LED_START+i),false);
    }
    for(int i=0;i<NB_MIDI_LCDS;i++)
    {
            m_midiLcds[i]=new IntModel
                    (0,0,127,
                     nullptr,QString("LCD%1").arg(NB_MIDI_LCD_START+i),false);
    }
    */

    // we need a play-handle which cares for calling play()
    InstrumentPlayHandle* iph
            = new InstrumentPlayHandle(this, instrumentTrack);
    Engine::mixer()->emit playHandleToAdd(PlayHandlePointer(iph));

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(sampleRateChanged()));
}

CarlaInstrument::~CarlaInstrument()
{
    qInfo("CarlaInstrument::~CarlaInstrument START");

    paramModels.clear();

    if(fHost.resourceDir != nullptr)
    {
        std::free((char*)fHost.resourceDir);
        fHost.resourceDir = nullptr;
    }

    if(fHost.uiName != nullptr)
    {
        std::free((char*)fHost.uiName);
        fHost.uiName = nullptr;
    }

    if(fHandle == nullptr)
        return;

    if(fDescriptor->deactivate != nullptr)
        fDescriptor->deactivate(fHandle);

    if(fDescriptor->cleanup != nullptr)
        fDescriptor->cleanup(fHandle);

    fHandle = nullptr;
    qInfo("CarlaInstrument::~CarlaInstrument END");
}

// -------------------------------------------------------------------

uint32_t CarlaInstrument::handleGetBufferSize() const
{
    return Engine::mixer()->framesPerPeriod();
}

double CarlaInstrument::handleGetSampleRate() const
{
    return Engine::mixer()->processingSampleRate();
}

bool CarlaInstrument::handleIsOffline() const
{
    return false;  // TODO
}

const NativeTimeInfo* CarlaInstrument::handleGetTimeInfo() const
{
    return &fTimeInfo;
}

void CarlaInstrument::handleUiParameterChanged(const uint32_t /*index*/,
                                               const FLOAT /*value*/) const
{
}

void CarlaInstrument::handleUiClosed()
{
    emit uiClosed();
}

intptr_t CarlaInstrument::handleDispatcher(
        const NativeHostDispatcherOpcode opcode,
        const int32_t                    index,
        const intptr_t                   value,
        void* const                      ptr,
        const FLOAT                      opt)
{
    intptr_t ret = 0;

    switch(opcode)
    {
        case NATIVE_HOST_OPCODE_NULL:
            break;
        case NATIVE_HOST_OPCODE_UPDATE_PARAMETER:
        case NATIVE_HOST_OPCODE_UPDATE_MIDI_PROGRAM:
        case NATIVE_HOST_OPCODE_RELOAD_PARAMETERS:
        case NATIVE_HOST_OPCODE_RELOAD_MIDI_PROGRAMS:
        case NATIVE_HOST_OPCODE_RELOAD_ALL:
            // nothing
            break;
        case NATIVE_HOST_OPCODE_INTERNAL_PLUGIN:
            // tmp, avoid warning
            break;
        case NATIVE_HOST_OPCODE_UI_UNAVAILABLE:
            handleUiClosed();
            break;
        case NATIVE_HOST_OPCODE_HOST_IDLE:
            qApp->processEvents();
            break;
    }

    return ret;

    // unused for now
    (void)index;
    (void)value;
    (void)ptr;
    (void)opt;
}

// -------------------------------------------------------------------

Instrument::Flags CarlaInstrument::flags() const
{
    return IsSingleStreamed | IsMidiBased | IsNotBendable;
}

f_cnt_t CarlaInstrument::desiredReleaseFrames() const
{
    return 0;
}

/*
QString CarlaInstrument::nodeName() const
{
    return descriptor()->name;
}
*/

void CarlaInstrument::saveSettings(QDomDocument& doc, QDomElement& parent)
{
    /*
    for(int i = 0; i < NB_MIDI_KNOBS; i++)
        m_midiKnobs[i]->saveSettings(doc, parent, QString("knob%1").arg(i));
    for(int i=0;i<NB_MIDI_LEDS;i++)
            m_midiLeds[i]->saveSettings( doc, parent, QString("led%1").arg(i)
    ); for(int i=0;i<NB_MIDI_LCDS;i++) m_midiLcds[i]->saveSettings( doc,
    parent, QString("lcd%1").arg(i) );
    */

    if(fHandle == nullptr || fDescriptor->get_state == nullptr)
    {
        qWarning(
                "CarlaInstrument::saveSettings fHandle or fDescriptor is "
                "null");
        return;
    }

    char* const state = fDescriptor->get_state(fHandle);

    if(state == nullptr)
    {
        qWarning("CarlaInstrument: retrieved state is null");
        return;
    }

    QDomDocument carlaDoc("carla");

    if(carlaDoc.setContent(QString(state)))
    {
        QDomNode n = doc.importNode(carlaDoc.documentElement(), true);
        parent.appendChild(n);
    }

    std::free(state);

    for(int i = 0; i < paramModels.count(); ++i)
    {
        QString idStr = CARLA_SETTING_PREFIX + QString::number(i);
        paramModels[i]->saveSettings(doc, parent, idStr);
    }
}

void CarlaInstrument::refreshParams(bool valuesOnly, const QDomElement* elem)
{
    if(fDescriptor->get_parameter_count != nullptr
       && fDescriptor->get_parameter_info != nullptr
       && fDescriptor->get_parameter_value != nullptr
       && fDescriptor->set_parameter_value != nullptr)
    {
        uint32_t param_count = fDescriptor->get_parameter_count(fHandle);

        if(!valuesOnly)
        {
            clearParamModels();
            paramModels.reserve(param_count);
        }

        m_completerList.clear();

        for(uint32_t i = 0; i < param_count; ++i)
        {
            // https://github.com/falkTX/Carla/tree/master/source/native-plugins
            // source/native-plugins/resources/carla-plugin
            FLOAT param_value = fDescriptor->get_parameter_value(fHandle, i);

            if(valuesOnly && i < paramModels.size())
            {
                paramModels[i]->setValue(param_value);
                continue;
            }

            const NativeParameter* paramInfo(
                    fDescriptor->get_parameter_info(fHandle, i));

            // Get parameter name
            QString name = "_NO_NAME_";
            if(paramInfo->name != nullptr)
            {
                name = paramInfo->name;
            }

            m_completerList.push_back(name);

            // current_value, min, max, steps
            paramModels.push_back(new FloatModel(
                    param_value, paramInfo->ranges.min, paramInfo->ranges.max,
                    paramInfo->ranges.step, this, name));

            // Load settings into model.
            QString idStr = CARLA_SETTING_PREFIX + QString::number(i);
            paramModels[i]->setObjectName(QString::number(
                    i));  // use this as reference for parameter index.

            if(elem != nullptr)
                paramModels[i]->loadSettings(*elem, idStr);

            // Connect to signal dataChanged to knobChanged function.
            // connect(paramModels[i], SIGNAL(dataChanged()), this,
            // SLOT(knobModelChanged()));
            connect(paramModels[i], &FloatModel::dataChanged,
                    [=]() { paramModelChanged(i); });
        }

        // Set completer data
        // m_completerModel.setStringList(m_completerList);
    }
}

void CarlaInstrument::clearParamModels()
{
    // Delete the models, this also disconnects all connections (automation
    // and controller connections)
    for(int i = 0; i < paramModels.count(); ++i)
        delete paramModels[i];

    // Clear the list
    paramModels.clear();
}

void CarlaInstrument::paramModelChanged(uint32_t index)
{
    qInfo("CarlaInstrument::paramModelChanged(%d)", index);

    // FloatModel *senderFloatModel = qobject_cast<FloatModel *>(sender());
    if(fDescriptor->set_parameter_value != nullptr)
    {
        // fDescriptor->set_parameter_value(fHandle,
        // senderFloatModel->objectName().toInt(), senderFloatModel->value());
        // qInfo(" *** %s (%d)", qPrintable(paramModels[index]->objectName()),
        //   paramModels[index]->objectName().toInt());
        fDescriptor->set_parameter_value(
                fHandle, paramModels[index]->objectName().toInt(),
                paramModels[index]->value());
    }
}

void CarlaInstrument::loadSettings(const QDomElement& elem)
{
    qInfo("CarlaInstrument::loadSettings start");

    /*
    for(int i = 0; i < NB_MIDI_KNOBS; i++)
        m_midiKnobs[i]->loadSettings(elem, QString("knob%1").arg(i));
    for(int i=0;i<NB_MIDI_LEDS;i++)
            m_midiLeds[i]->loadSettings( elem, QString("led%1").arg(i) );
    for(int i=0;i<NB_MIDI_LCDS;i++)
            m_midiLcds[i]->loadSettings( elem, QString("lcd%1").arg(i) );
    */

    if(fHandle == nullptr || fDescriptor->set_state == nullptr)
    {
        qWarning("CarlaInstrument: fHandle or fDescriptor is null");
        return;
    }

    QDomNode node = elem.firstChild();
    while(!node.isNull())
    {
        if(node.isElement())
        {
            if(node.nodeName() == "CARLA-PROJECT")
            {
                QDomDocument carlaDoc("carla");
                carlaDoc.appendChild(carlaDoc.importNode(node, true));
                fDescriptor->set_state(
                        fHandle, carlaDoc.toString(0).toUtf8().constData());
            }
        }
        node = node.nextSibling();
    }

    // settingsElem = const_cast<QDomElement&>(elem);
    refreshParams(false, &elem);

    qInfo("CarlaInstrument::loadSettings end");
}

void CarlaInstrument::play(sampleFrame* workingBuffer)
{
    const fpp_t bufsize = Engine::mixer()->framesPerPeriod();

    // std::memset(workingBuffer, 0,
    //            sizeof(sampleFrame) * bufsize);

    if(fHandle == nullptr)
    {
        // instrumentTrack()->processAudioBuffer(workingBuffer, bufsize,
        //                                nullptr);
        // qInfo("carla fHandle == nullptr");
        return;
    }

    // set time info
    Song* const s     = Engine::getSong();
    fTimeInfo.playing = s->isPlaying();
    fTimeInfo.frame
            = s->getPlayPos(s->playMode()).frames(Engine::framesPerTick());
    fTimeInfo.usecs            = s->getMilliseconds() * 1000;
    fTimeInfo.bbt.bar          = s->getTacts() + 1;
    fTimeInfo.bbt.beat         = s->getBeat() + 1;
    fTimeInfo.bbt.tick         = s->getBeatTicks();
    fTimeInfo.bbt.barStartTick = ticksPerBeat
                                 * s->getTimeSigModel().getNumerator()
                                 * s->getTacts();
    fTimeInfo.bbt.beatsPerBar    = s->getTimeSigModel().getNumerator();
    fTimeInfo.bbt.beatType       = s->getTimeSigModel().getDenominator();
    fTimeInfo.bbt.ticksPerBeat   = ticksPerBeat;
    fTimeInfo.bbt.beatsPerMinute = s->getTempo();

    FLOAT  buf1[bufsize];
    FLOAT  buf2[bufsize];
    FLOAT* rBuf[] = {buf1, buf2};
    std::memset(buf1, 0, sizeof(FLOAT) * bufsize);
    std::memset(buf2, 0, sizeof(FLOAT) * bufsize);

    {
        const QMutexLocker ml(&fMutex);
        fDescriptor->process(fHandle, rBuf, rBuf, bufsize, fMidiEvents,
                             fMidiEventCount);
        fMidiEventCount = 0;
    }

    for(uint i = 0; i < bufsize; ++i)
    {
        workingBuffer[i][0] = buf1[i];
        workingBuffer[i][1] = buf2[i];
    }

    instrumentTrack()->processAudioBuffer(workingBuffer, bufsize, nullptr);
}

bool CarlaInstrument::handleMidiEvent(const MidiEvent& event,
                                      const MidiTime&,
                                      f_cnt_t offset)
{
    const QMutexLocker ml(&fMutex);

    if(fMidiEventCount >= kMaxMidiEvents)
        return false;

    NativeMidiEvent& nEvent(fMidiEvents[fMidiEventCount++]);
    std::memset(&nEvent, 0, sizeof(NativeMidiEvent));

    nEvent.port    = 0;
    nEvent.time    = offset;
    nEvent.data[0] = event.type() | (event.channel() & 0x0F);

    switch(event.type())
    {
        case MidiNoteOn:
            if(event.velocity() > 0)
            {
                if(event.key() < 0 || event.key() > MidiMaxKey)
                    break;

                nEvent.data[1] = event.key();
                nEvent.data[2] = event.velocity();
                nEvent.size    = 3;
                break;
            }
            else
            {
                nEvent.data[0] = MidiNoteOff | (event.channel() & 0x0F);
                // nobreak
            }

        case MidiNoteOff:
            if(event.key() < 0 || event.key() > MidiMaxKey)
                break;

            nEvent.data[1] = event.key();
            nEvent.data[2] = event.velocity();
            nEvent.size    = 3;
            break;

        case MidiKeyPressure:
            nEvent.data[1] = event.key();
            nEvent.data[2] = event.velocity();
            nEvent.size    = 3;
            break;

        case MidiControlChange:
            nEvent.data[1] = event.controllerNumber();
            nEvent.data[2] = event.controllerValue();
            nEvent.size    = 3;
            break;

        case MidiProgramChange:
            nEvent.data[1] = event.program();
            nEvent.size    = 2;
            break;

        case MidiChannelPressure:
            nEvent.data[1] = event.channelPressure();
            nEvent.size    = 2;
            break;

        case MidiPitchBend:
            // nEvent.data[1] = event.pitchBend() & 0x7f;
            // nEvent.data[2] = event.pitchBend() >> 7;
            event.midiPitchBendLE(&nEvent.data[1], &nEvent.data[2]);
            nEvent.size = 3;
            break;

        default:
            // unhandled
            --fMidiEventCount;
            break;
    }

    return true;
}

PluginView* CarlaInstrument::instantiateView(QWidget* parent)
{
    if(QWidget* const window = parent->window())
        fHost.uiParentId = window->winId();
    else
        fHost.uiParentId = 0;

    std::free((char*)fHost.uiName);

    // TODO - get plugin instance name
    // fHost.uiName = strdup(parent->windowTitle().toUtf8().constData());
    fHost.uiName
            = strdup(kIsPatchbay ? "CarlaPatchbay-LMMS" : "CarlaRack-LMMS");

    return new CarlaInstrumentView(this, parent);
}

void CarlaInstrument::sampleRateChanged()
{
    fDescriptor->dispatcher(fHandle, NATIVE_PLUGIN_OPCODE_SAMPLE_RATE_CHANGED,
                            0, 0, nullptr, handleGetSampleRate());
}

// -------------------------------------------------------------------

CarlaInstrumentView::CarlaInstrumentView(CarlaInstrument* const instrument,
                                         QWidget* const         parent) :
      InstrumentView(instrument, parent),
      fHandle(instrument->fHandle), fDescriptor(instrument->fDescriptor),
      fTimerId(fHandle != nullptr && fDescriptor->ui_idle != nullptr
                       ? startTimer(30)
                       : 0),
      m_carlaInstrument(instrument), m_paramsView(nullptr)
{
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(),
                 instrument->kIsPatchbay
                         ? PLUGIN_NAME::getIconPixmap("artwork-patchbay")
                         : PLUGIN_NAME::getIconPixmap("artwork-rack"));
    setPalette(pal);

    QVBoxLayout* l = new QVBoxLayout(this);
    l->setContentsMargins(6, 66, 6, 6);
    l->setSpacing(6);

    m_toggleUIButton = new QPushButton(tr("Show GUI"), this);
    m_toggleUIButton->setCheckable(true);
    m_toggleUIButton->setChecked(false);
    m_toggleUIButton->setIcon(embed::getIconPixmap("zoom"));
    m_toggleUIButton->setFont(pointSize<8>(m_toggleUIButton->font()));
    connect(m_toggleUIButton, SIGNAL(clicked(bool)), this,
            SLOT(toggleUI(bool)));
    m_toggleUIButton->setWhatsThis(
            tr("Click here to show or hide the graphical user interface "
               "(GUI) of Carla."));
    // m_toggleUIButton->setFixedSize(250, 22);
    l->addWidget(m_toggleUIButton, Qt::AlignHCenter);

    // Open params sub window button
    m_toggleParamsWindowButton = new QPushButton(tr("Parameters"), this);
    m_toggleParamsWindowButton->setCheckable(true);
    m_toggleParamsWindowButton->setChecked(false);
    m_toggleParamsWindowButton->setIcon(embed::getIcon("controller"));
    m_toggleParamsWindowButton->setFont(
            pointSize<8>(m_toggleParamsWindowButton->font()));
    connect(m_toggleParamsWindowButton, SIGNAL(clicked(bool)), this,
            SLOT(toggleParamsWindow(bool)));
    m_toggleParamsWindowButton->setWhatsThis(
            tr("Click here to show or hide the parameters "
               "of your Carla instrument."));
    // m_toggleParamsWindowButton->setFixedSize(250, 22);
    l->addWidget(m_toggleParamsWindowButton, Qt::AlignHCenter);

    /*
    QScrollArea* knobsSCA = new QScrollArea(this);
    knobsSCA->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    knobsSCA->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    GroupBox*    knobsGBX = new GroupBox(tr("MIDI CC"), this, false, true);
    QWidget*     knobsPNL = new QWidget(knobsGBX);
    QGridLayout* knobsLOT = new QGridLayout(knobsPNL);
    knobsLOT->setContentsMargins(6, 6, 6, 6);
    knobsLOT->setSpacing(3);
    knobsPNL->setLayout(knobsLOT);
    for(int i = 0; i < NB_MIDI_KNOBS; i++)
    {
        m_midiKnobs[i] = new Knob(knobsPNL);
        m_midiKnobs[i]->setLabel(
                QString("CC %1").arg(NB_MIDI_KNOB_START + i));
        m_midiKnobs[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
                                            .arg(MIDI_CH)
                                            .arg(NB_MIDI_KNOB_START + i),
                                    "");
        m_midiKnobs[i]->setWhatsThis("");
        // m_midiKnobs[i]->setGeometry(39*(i%6),39*(i/6),39,39);
        m_midiKnobs[i]->setModel(instrument->m_midiKnobs[i]);
        connect(instrument->m_midiKnobs[i], SIGNAL(dataChanged()), this,
                SLOT(onDataChanged()));
        knobsLOT->addWidget(m_midiKnobs[i], i / 7, i % 7);
    }
    */
    /*
    knobsGBX->setContentWidget(knobsPNL);
    knobsSCA->setWidget(knobsGBX);
    l->addWidget(knobsSCA);
    */

    l->addStretch(1);
    connect(instrument, SIGNAL(uiClosed()), this, SLOT(uiClosed()));
}

CarlaInstrumentView::~CarlaInstrumentView()
{
    qInfo("CarlaInstrumentView::~CarlaInstrumentView START");
    if(m_toggleUIButton->isChecked())
        toggleUI(false);
    qInfo("CarlaInstrumentView::~CarlaInstrumentView END");
}

void CarlaInstrumentView::toggleUI(bool visible)
{
    if(fHandle != nullptr && fDescriptor->ui_show != nullptr)
        fDescriptor->ui_show(fHandle, visible);
}

void CarlaInstrumentView::uiClosed()
{
    m_toggleUIButton->setChecked(false);
}

void CarlaInstrumentView::toggleParamsWindow(bool visible)
{
    qInfo("CarlaInstrumentView::toggleParamsWindow 0.1");
    if(m_paramsView == nullptr)
    {
        qInfo("CarlaInstrumentView::toggleParamsWindow 0.2");
        m_paramsView = new CarlaInstrumentParamsView(m_carlaInstrument,
                                                     this);  // p_parent);
        connect(m_paramsView->subWindow(), SIGNAL(closed()), this,
                SLOT(paramsWindowClosed()));
    }
    qInfo("CarlaInstrumentView::toggleParamsWindow 0.3");
    if(visible != m_paramsView->subWindow()->isVisible())
    {
        if(visible)
        {
            qInfo("CarlaInstrumentView::toggleParamsWindow 1");
            m_paramsView->subWindow()->show();
            qInfo("CarlaInstrumentView::toggleParamsWindow 2");
            m_paramsView->show();
            qInfo("CarlaInstrumentView::toggleParamsWindow 3");
        }
        else
        {
            m_paramsView->subWindow()->hide();
            m_paramsView->hide();
        }
    }
}

void CarlaInstrumentView::paramsWindowClosed()
{
    m_toggleParamsWindowButton->setChecked(false);
}

/*
void CarlaInstrumentView::onDataChanged()
{
    QObject* o = sender();
    if(!o)
        qInfo("no sender");
    AutomatableModel* m = dynamic_cast<AutomatableModel*>(sender());
    if(m)
    {
        int cc = -1;
        int v  = -1;
        if(cc == -1)
            for(int i = 0; i < NB_MIDI_KNOBS; i++)
                if(m_midiKnobs[i]->model() == m)
                {
                    cc = NB_MIDI_KNOB_START + i;
                    v  = m_midiKnobs[i]->model()->value();
                }
        if(cc != -1)
        {
            qInfo("cc %d: data changed: %d", cc, v);
            CarlaInstrument* fx = dynamic_cast<CarlaInstrument*>(model());
            if(fx)
            {
                MidiEvent ev(MidiEventTypes::MidiControlChange, MIDI_CH - 1,
                             cc, v);
                PlayPos   pos = Engine::getSong()->getPlayPos();
                // qInfo("sending midi event");
                fx->handleMidiEvent(ev, pos, pos.currentFrame());
            }
            else
                qInfo("fx is null");
        }
        else
            qInfo("cc model not found");
    }
    else
        qInfo("sender but no model");
}
*/

void CarlaInstrumentView::modelChanged()
{
}

void CarlaInstrumentView::timerEvent(QTimerEvent* event)
{
    if(event->timerId() == fTimerId)
        fDescriptor->ui_idle(fHandle);

    InstrumentView::timerEvent(event);
}

CarlaInstrumentParamsView::CarlaInstrumentParamsView(
        CarlaInstrument* instrument, QWidget* parent) :
      QWidget(parent),
      m_carlaInstrument(instrument)
{
    lMaxColumns = 3;
    lCurColumn  = 0;
    lCurRow     = 0;
    // Create central widget
    /*  ___ centralWidget _______________	QWidget
     * |  __ verticalLayout _____________	QVBoxLayout
     * | |  __ m_toolBarLayout __________	QHBoxLayout
     * | | |
     * | | | option_0 | option_1 ..
     * | | |_____________________________
     * | |
     * | |  __ m_scrollArea _____________	QScrollArea
     * | | |  __ m_scrollAreaWidgetContent	QWidget
     * | | | |  __ m_scrollAreaLayout ___	QGridLayout
     * | | | | |
     * | | | | | knob | knob | knob
     * | | | | | knob | knob | knob
     * | | | | | knob | knob | knob
     * | | | | |_________________________
     * | | | |___________________________
     * | | |_____________________________
     * */
    // QWidget*     centralWidget  = new QWidget(this);
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);  // centralWidget);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->setSpacing(6);

    // Toolbar
    m_toolBarLayout = new QHBoxLayout();
    m_toolBarLayout->setContentsMargins(3, 3, 3, 3);
    m_toolBarLayout->setSpacing(3);

    // Refresh params button
    m_refreshParamsButton = new QPushButton(tr(""), this);
    m_refreshParamsButton->setIcon(embed::getIcon("reload"));
    m_refreshParamsButton->setToolTip(tr("Reload Carla parameters."));
    m_refreshParamsButton->setFixedSize(24, 24);

    // Refresh param values button
    m_refreshParamValuesButton = new QPushButton(tr(""), this);
    m_refreshParamValuesButton->setIcon(embed::getIcon("synchronize"));
    m_refreshParamValuesButton->setToolTip(tr("Synchronize LSMM with Carla"));
    m_refreshParamValuesButton->setFixedSize(24, 24);

    // Params filter line edit
    m_paramsFilterLineEdit = new QLineEdit(this);
    m_paramsFilterLineEdit->setPlaceholderText(tr("Search.."));
    m_paramCompleter = new QCompleter(m_carlaInstrument->m_completerList);
    m_paramsFilterLineEdit->setCompleter(m_paramCompleter);
    m_paramsFilterLineEdit->setFixedHeight(24);

    // Clear filter line edit button
    m_clearFilterButton = new QPushButton(tr(""), this);
    m_clearFilterButton->setIcon(embed::getIcon("edit_erase"));
    m_clearFilterButton->setToolTip(tr("Clear filter text"));
    m_clearFilterButton->setFixedSize(24, 24);

    // Show automated only button
    m_automatedOnlyButton = new QPushButton(tr(""), this);
    m_automatedOnlyButton->setIcon(embed::getIcon("automation"));
    m_automatedOnlyButton->setToolTip(
            tr("Only show knobs with a connection."));
    m_automatedOnlyButton->setCheckable(true);
    m_automatedOnlyButton->setFixedSize(24, 24);

    // Add stuff to toolbar
    m_toolBarLayout->addWidget(m_refreshParamsButton);
    m_toolBarLayout->addWidget(m_refreshParamValuesButton);
    m_toolBarLayout->addWidget(m_paramsFilterLineEdit);
    m_toolBarLayout->addWidget(m_clearFilterButton);
    m_toolBarLayout->addWidget(m_automatedOnlyButton);

    // Create scroll area for the knobs
    m_scrollArea              = new QScrollArea(this);
    m_scrollAreaWidgetContent = new QWidget();
    m_scrollAreaLayout        = new QGridLayout(m_scrollAreaWidgetContent);
    m_scrollAreaWidgetContent->setLayout(m_scrollAreaLayout);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setWidget(m_scrollAreaWidgetContent);
    m_scrollArea->setWidgetResizable(true);
    m_scrollAreaLayout->setContentsMargins(3, 3, 3, 3);
    m_scrollAreaLayout->setVerticalSpacing(3);
    m_scrollAreaLayout->setHorizontalSpacing(6);
    m_scrollAreaLayout->setColumnStretch(lMaxColumns, 1);

    // Add m_toolBarLayout and m_scrollArea to the verticalLayout.
    verticalLayout->addLayout(m_toolBarLayout);
    verticalLayout->addWidget(m_scrollArea);

    /*
    CarlaParamsSubWindow* win = new CarlaParamsSubWindow(
            gui->mainWindow()->workspace()->viewport(),
            Qt::SubWindow | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                    | Qt::WindowSystemMenuHint);
    m_subWindow
            = gui->mainWindow()->workspace()->addSubWindow(win);
    */

    /*m_subWindow->*/
    setWindowTitle(m_carlaInstrument->instrumentTrack()->name() + " - "
                   + tr("Parameters"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setFixedWidth(250);
    setMinimumHeight(500);

    qInfo("CarlaInstrumentParamsView::CarlaInstrumentParamsView 1");
    m_subWindow = SubWindow::putWidgetOnWorkspace(this, false, false, false);
    qInfo("CarlaInstrumentParamsView::CarlaInstrumentParamsView 2");
    // m_subWindow->setWidget(centralWidget);
    m_subWindow->hide();

    // Connect signals
    connect(m_refreshParamsButton, SIGNAL(clicked(bool)), this,
            SLOT(onRefreshButton()));
    connect(m_refreshParamValuesButton, SIGNAL(clicked(bool)), this,
            SLOT(onRefreshValuesButton()));
    connect(m_paramsFilterLineEdit, SIGNAL(textChanged(const QString)), this,
            SLOT(filterKnobs()));
    connect(m_clearFilterButton, SIGNAL(clicked(bool)), this,
            SLOT(clearFilterText()));
    connect(m_automatedOnlyButton, SIGNAL(toggled(bool)), this,
            SLOT(filterKnobs()));

    qInfo("CarlaInstrumentParamsView::CarlaInstrumentParamsView 3");
    refreshKnobs();  // Add buttons if there are any already.
    qInfo("CarlaInstrumentParamsView::CarlaInstrumentParamsView 4");
    m_subWindow->show();  // Show the subwindow
    qInfo("CarlaInstrumentParamsView::CarlaInstrumentParamsView 5");
}

CarlaInstrumentParamsView::~CarlaInstrumentParamsView()
{
    qInfo("CarlaInstrumentParamsView::~CarlaInstrumentParamsView START");

    // Close and delete m_subWindow
    if(m_subWindow != nullptr)
    {
        m_subWindow->setAttribute(Qt::WA_DeleteOnClose);
        m_subWindow->close();
        // if(m_subWindow != nullptr)
        // delete m_subWindow;
        m_subWindow = nullptr;
    }
    // p_subWindow = nullptr;
    // Clear models
    if(m_carlaInstrument->paramModels.isEmpty() == false)
    {
        m_carlaInstrument->clearParamModels();
    }

    qInfo("CarlaInstrumentParamsView::~CarlaInstrumentParamsView END");
}

void CarlaInstrumentParamsView::clearFilterText()
{
    m_paramsFilterLineEdit->setText("");
}

void CarlaInstrumentParamsView::filterKnobs()
{
    QString text = m_paramsFilterLineEdit->text();
    clearKnobs();  // Remove all knobs from the layout.
    for(uint32_t i = 0; i < m_knobs.count(); ++i)
    {
        // Filter on automation only
        if(m_automatedOnlyButton->isChecked())
        {
            if(!m_carlaInstrument->paramModels[i]->isAutomatedOrControlled())
            {
                continue;
            }
        }

        // Filter on text
        if(text != "")
        {
            if(m_knobs[i]->objectName().contains(text, Qt::CaseInsensitive))
            {
                addKnob(i);
            }
        }
        else
        {
            addKnob(i);
        }
    }
}

void CarlaInstrumentParamsView::onRefreshButton()
{
    if(m_carlaInstrument->paramModels.isEmpty() == false)
    {
        if(QMessageBox::warning(
                   nullptr, tr("Reload knobs"),
                   tr("There are already knobs loaded, if any of them "
                      "are connected to a controller or automation track "
                      "their connection will be lost. Do you want to "
                      "continue?"),
                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
           != QMessageBox::Yes)
        {
            return;
        }
    }
    m_carlaInstrument->refreshParams(false);
    refreshKnobs();
}

void CarlaInstrumentParamsView::onRefreshValuesButton()
{
    m_carlaInstrument->refreshParams(true);
}

void CarlaInstrumentParamsView::refreshKnobs()
{
    if(m_carlaInstrument->paramModels.count() == 0)
    {
        return;
    }
    // Make sure all the knobs are deleted.
    for(uint32_t i = 0; i < m_knobs.count(); ++i)
    {
        delete m_knobs[i];  // Delete knob widgets itself.
    }
    m_knobs.clear();  // Clear the pointer list.
                      // Clear the layout (posible spacer).
    QLayoutItem* item;
    while((item = m_scrollAreaLayout->takeAt(0)))
    {
        if(item->widget())
        {
            delete item->widget();
        }
        delete item;
    }

    // Reset position data.
    lCurColumn = 0;
    lCurRow    = 0;
    // Make room in QList m_knobs
    m_knobs.reserve(m_carlaInstrument->paramModels.count());
    for(uint32_t i = 0; i < m_carlaInstrument->paramModels.count(); ++i)
    {
        m_knobs.push_back(new Knob(m_scrollAreaWidgetContent));
        QString name = (*m_carlaInstrument->paramModels[i]).displayName();
        m_knobs[i]->setHintText(name, "");
        m_knobs[i]->setText(name);
        m_knobs[i]->setObjectName(
                name);  // this is being used for filtering the knobs.
                        // Set the newly created model to the knob.
        m_knobs[i]->setModel(m_carlaInstrument->paramModels[i]);
        m_knobs[i]->setFixedSize(228 / lMaxColumns - 6, 36);
        // Add knob to layout
        addKnob(i);
    }
    // Add spacer so all knobs go to top
    /*
    QSpacerItem* verticalSpacer = new QSpacerItem(
            20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_scrollAreaLayout->addItem(verticalSpacer, lCurRow + 1, 0, 1, 1);
    */
}

void CarlaInstrumentParamsView::addKnob(uint32_t index)
{
    // Add the new knob to layout
    m_scrollAreaLayout->addWidget(m_knobs[index], lCurRow, lCurColumn,
                                  Qt::AlignHCenter | Qt::AlignTop);
    // Chances that we did close() on the widget is big, so show it.
    m_knobs[index]->show();
    // Keep track of current column and row index.
    if(lCurColumn < lMaxColumns - 1)
    {
        lCurColumn++;
    }
    else
    {
        lCurColumn = 0;
        lCurRow++;
    }
}

void CarlaInstrumentParamsView::clearKnobs()
{
    // Remove knobs from layout.
    for(uint32_t i = 0; i < m_knobs.count(); ++i)
    {
        m_knobs[i]->close();
    }
    // Reset position data.
    lCurColumn = 0;
    lCurRow    = 0;
}

void CarlaInstrumentParamsView::modelChanged()
{
    refreshKnobs();
}
