/*
 * carla.cpp - Carla for LMMS
 *
 * Copyright (C) 2014 Filipe Coelho <falktx@falktx.com>
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

#include "carla.h"

#define REAL_BUILD // FIXME this shouldn't be needed
#include "CarlaHost.h"

#include "Engine.h"
#include "Song.h"
#include "gui_templates.h"
#include "InstrumentPlayHandle.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "Knob.h"

#include <QApplication>
#include <QFileDialog>
#include <QPushButton>
#include <QTimerEvent>
#include <QVBoxLayout>

#include <cstring>

#include "embed.h"

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
    return false; // unsupported?
}

static void host_ui_parameter_changed(NativeHostHandle handle, uint32_t index, float value)
{
    handlePtr->handleUiParameterChanged(index, value);
}

static void host_ui_custom_data_changed(NativeHostHandle handle, const char* key, const char* value)
{
    // unused
}

static void host_ui_closed(NativeHostHandle handle)
{
    handlePtr->handleUiClosed();
}

static intptr_t host_dispatcher(NativeHostHandle handle, NativeHostDispatcherOpcode opcode, int32_t index, intptr_t value, void* ptr, float opt)
{
    return handlePtr->handleDispatcher(opcode, index, value, ptr, opt);
}

#undef handlePtr

// -----------------------------------------------------------------------

static const char* host_ui_open_file(NativeHostHandle, bool isDir, const char* title, const char* filter)
{
    static QByteArray retStr;
    const QFileDialog::Options options(isDir ? QFileDialog::ShowDirsOnly : 0x0);

    retStr = QFileDialog::getOpenFileName(QApplication::activeWindow(), title, "", filter, NULL, options).toUtf8();

    return retStr.isEmpty() ? NULL : retStr.constData();
}

static const char* host_ui_save_file(NativeHostHandle, bool isDir, const char* title, const char* filter)
{
    static QByteArray retStr;
    const QFileDialog::Options options(isDir ? QFileDialog::ShowDirsOnly : 0x0);

    retStr = QFileDialog::getSaveFileName(QApplication::activeWindow(), title, "", filter, NULL, options).toUtf8();

    return retStr.isEmpty() ? NULL : retStr.constData();
}

// -----------------------------------------------------------------------

CARLA_EXPORT
const NativePluginDescriptor* carla_get_native_patchbay_plugin();

CARLA_EXPORT
const NativePluginDescriptor* carla_get_native_rack_plugin();

// -----------------------------------------------------------------------

CarlaInstrument::CarlaInstrument(InstrumentTrack* const instrumentTrack, const Descriptor* const descriptor, const bool isPatchbay)
    : Instrument(instrumentTrack, descriptor),
      kIsPatchbay(isPatchbay),
      fHandle(NULL),
      fDescriptor(isPatchbay ? carla_get_native_patchbay_plugin() : carla_get_native_rack_plugin()),
      fMidiEventCount(0)
{
    fHost.handle      = this;
    fHost.uiName      = NULL;
    fHost.uiParentId  = 0;

    // figure out prefix from dll filename
    QString dllName(carla_get_library_filename());

#if defined(CARLA_OS_LINUX)
    fHost.resourceDir = strdup(QString(dllName.split("/lib/carla")[0] + "/share/carla/resources/").toUtf8().constData());
#else
    fHost.resourceDir = NULL;
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
    fTimeInfo.bbt.valid = true; // always valid

    fHandle = fDescriptor->instantiate(&fHost);
    Q_ASSERT(fHandle != NULL);

    if (fHandle != NULL && fDescriptor->activate != NULL)
        fDescriptor->activate(fHandle);

    for(int i=0;i<NB_KNOBS;i++)
    {
            m_knobs[i]=new FloatModel
                    (0.f,0.f,127.f,1.f,
                     NULL,QString("KNB%1").arg(NB_KNOB_START+i),false);
    }
    for(int i=0;i<NB_LEDS;i++)
    {
            m_leds[i]=new BoolModel
                    (false,NULL,QString("LED%1").arg(NB_LED_START+i),false);
    }
    for(int i=0;i<NB_LCDS;i++)
    {
            m_lcds[i]=new IntModel
                    (0,0,127,
                     NULL,QString("LCD%1").arg(NB_LCD_START+i),false);
    }

    // we need a play-handle which cares for calling play()
    InstrumentPlayHandle * iph = new InstrumentPlayHandle( this, instrumentTrack );
    Engine::mixer()->addPlayHandle( iph );

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this, SLOT(sampleRateChanged()));
}

CarlaInstrument::~CarlaInstrument()
{
    Engine::mixer()->removePlayHandlesOfTypes(instrumentTrack(), PlayHandle::TypeNotePlayHandle | PlayHandle::TypeInstrumentPlayHandle);

    if (fHost.resourceDir != NULL)
    {
        std::free((char*)fHost.resourceDir);
        fHost.resourceDir = NULL;
    }

    if (fHost.uiName != NULL)
    {
        std::free((char*)fHost.uiName);
        fHost.uiName = NULL;
    }

    if (fHandle == NULL)
        return;

    if (fDescriptor->deactivate != NULL)
        fDescriptor->deactivate(fHandle);

    if (fDescriptor->cleanup != NULL)
        fDescriptor->cleanup(fHandle);

    fHandle = NULL;
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
    return false; // TODO
}

const NativeTimeInfo* CarlaInstrument::handleGetTimeInfo() const
{
    return &fTimeInfo;
}

void CarlaInstrument::handleUiParameterChanged(const uint32_t /*index*/, const float /*value*/) const
{
}

void CarlaInstrument::handleUiClosed()
{
    emit uiClosed();
}

intptr_t CarlaInstrument::handleDispatcher(const NativeHostDispatcherOpcode opcode, const int32_t index, const intptr_t value, void* const ptr, const float opt)
{
    intptr_t ret = 0;

    switch (opcode)
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
    (void)index; (void)value; (void)ptr; (void)opt;
}

// -------------------------------------------------------------------

Instrument::Flags CarlaInstrument::flags() const
{
    return IsSingleStreamed|IsMidiBased|IsNotBendable;
}

QString CarlaInstrument::nodeName() const
{
    return  descriptor()->name;
}

void CarlaInstrument::saveSettings(QDomDocument& doc, QDomElement& parent)
{
    for(int i=0;i<NB_KNOBS;i++)
            m_knobs[i]->saveSettings( doc, parent, QString("knob%1").arg(i) );
    for(int i=0;i<NB_LEDS;i++)
            m_leds[i]->saveSettings( doc, parent, QString("led%1").arg(i) );
    for(int i=0;i<NB_LCDS;i++)
            m_lcds[i]->saveSettings( doc, parent, QString("lcd%1").arg(i) );

    if (fHandle == NULL ||
        fDescriptor->get_state == NULL)
    {
        qWarning("CarlaInstrument::saveSettings fHandle or fDescriptor is null");
        return;
    }

    char* const state = fDescriptor->get_state(fHandle);

    if (state == NULL)
    {
        qWarning("CarlaInstrument: retrieved state is null");
        return;
    }

    QDomDocument carlaDoc("carla");

    if (carlaDoc.setContent(QString(state)))
    {
        QDomNode n = doc.importNode(carlaDoc.documentElement(), true);
        parent.appendChild(n);
    }

    std::free(state);
}

void CarlaInstrument::loadSettings(const QDomElement& elem)
{
    for(int i=0;i<NB_KNOBS;i++)
            m_knobs[i]->loadSettings( elem, QString("knob%1").arg(i) );
    for(int i=0;i<NB_LEDS;i++)
            m_leds[i]->loadSettings( elem, QString("led%1").arg(i) );
    for(int i=0;i<NB_LCDS;i++)
            m_lcds[i]->loadSettings( elem, QString("lcd%1").arg(i) );

    if (fHandle == NULL || fDescriptor->set_state == NULL)
    {
            qWarning("CarlaInstrument: fHandle or fDescriptor is null");
                return;
    }

    QDomNode node=elem.firstChild();
    while( !node.isNull() )
    {
        if( node.isElement() )
        {
            if( node.nodeName() == "CARLA-PROJECT" )
            {
                QDomDocument carlaDoc("carla");
                carlaDoc.appendChild(carlaDoc.importNode(node, true ));
                fDescriptor->set_state(fHandle,
                                       carlaDoc.toString(0).toUtf8().constData());
            }
        }
        node = node.nextSibling();
    }
}

void CarlaInstrument::play(sampleFrame* workingBuffer)
{
    const uint bufsize = Engine::mixer()->framesPerPeriod();

    std::memset(workingBuffer, 0, sizeof(sample_t)*bufsize*DEFAULT_CHANNELS);

    if (fHandle == NULL)
    {
        instrumentTrack()->processAudioBuffer(workingBuffer, bufsize, NULL);
        return;
    }

    // set time info
    Song * const s = Engine::getSong();
    fTimeInfo.playing  = s->isPlaying();
    fTimeInfo.frame    = s->getPlayPos(s->playMode()).frames(Engine::framesPerTick());
    fTimeInfo.usecs    = s->getMilliseconds()*1000;
    fTimeInfo.bbt.bar  = s->getTacts() + 1;
    fTimeInfo.bbt.beat = s->getBeat() + 1;
    fTimeInfo.bbt.tick = s->getBeatTicks();
    fTimeInfo.bbt.barStartTick   = ticksPerBeat*s->getTimeSigModel().getNumerator()*s->getTacts();
    fTimeInfo.bbt.beatsPerBar    = s->getTimeSigModel().getNumerator();
    fTimeInfo.bbt.beatType       = s->getTimeSigModel().getDenominator();
    fTimeInfo.bbt.ticksPerBeat   = ticksPerBeat;
    fTimeInfo.bbt.beatsPerMinute = s->getTempo();

    float buf1[bufsize];
    float buf2[bufsize];
    float* rBuf[] = { buf1, buf2 };
    std::memset(buf1, 0, sizeof(float)*bufsize);
    std::memset(buf2, 0, sizeof(float)*bufsize);

    {
        const QMutexLocker ml(&fMutex);
        fDescriptor->process(fHandle, rBuf, rBuf, bufsize, fMidiEvents, fMidiEventCount);
        fMidiEventCount = 0;
    }

    for (uint i=0; i < bufsize; ++i)
    {
        workingBuffer[i][0] = buf1[i];
        workingBuffer[i][1] = buf2[i];
    }

    instrumentTrack()->processAudioBuffer(workingBuffer, bufsize, NULL);
}

bool CarlaInstrument::handleMidiEvent(const MidiEvent& event, const MidiTime&, f_cnt_t offset)
{
    const QMutexLocker ml(&fMutex);

    if (fMidiEventCount >= kMaxMidiEvents)
        return false;

    NativeMidiEvent& nEvent(fMidiEvents[fMidiEventCount++]);
    std::memset(&nEvent, 0, sizeof(NativeMidiEvent));

    nEvent.port    = 0;
    nEvent.time    = offset;
    nEvent.data[0] = event.type() | (event.channel() & 0x0F);

    switch (event.type())
    {
    case MidiNoteOn:
        if (event.velocity() > 0)
        {
            if (event.key() < 0 || event.key() > MidiMaxKey)
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
        if (event.key() < 0 || event.key() > MidiMaxKey)
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
        //nEvent.data[1] = event.pitchBend() & 0x7f;
        //nEvent.data[2] = event.pitchBend() >> 7;
        event.midiPitchBendLE(&nEvent.data[1],&nEvent.data[2]);
        nEvent.size    = 3;
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
    if (QWidget* const window = parent->window())
        fHost.uiParentId = window->winId();
    else
        fHost.uiParentId = 0;

    std::free((char*)fHost.uiName);

    // TODO - get plugin instance name
    //fHost.uiName = strdup(parent->windowTitle().toUtf8().constData());
    fHost.uiName = strdup(kIsPatchbay ? "CarlaPatchbay-LMMS" : "CarlaRack-LMMS");

    return new CarlaInstrumentView(this, parent);
}

void CarlaInstrument::sampleRateChanged()
{
    fDescriptor->dispatcher(fHandle, NATIVE_PLUGIN_OPCODE_SAMPLE_RATE_CHANGED, 0, 0, nullptr, handleGetSampleRate());
}

// -------------------------------------------------------------------

CarlaInstrumentView::CarlaInstrumentView(CarlaInstrument* const instrument, QWidget* const parent)
    : InstrumentView(instrument, parent),
      fHandle(instrument->fHandle),
      fDescriptor(instrument->fDescriptor),
      fTimerId(fHandle != NULL && fDescriptor->ui_idle != NULL ? startTimer(30) : 0)
{
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(),
                 instrument->kIsPatchbay
                 ? PLUGIN_NAME::getIconPixmap("artwork-patchbay")
                 : PLUGIN_NAME::getIconPixmap("artwork-rack"));
    setPalette(pal);

    QVBoxLayout * l = new QVBoxLayout( this );
    l->setContentsMargins( 8, 66, 8, 8 );
    l->setSpacing( 10 );

    m_toggleUIButton = new QPushButton( tr( "Show GUI" ), this );
    m_toggleUIButton->setCheckable( true );
    m_toggleUIButton->setChecked( false );
    m_toggleUIButton->setIcon( embed::getIconPixmap( "zoom" ) );
    m_toggleUIButton->setFont( pointSize<8>( m_toggleUIButton->font() ) );
    connect( m_toggleUIButton, SIGNAL( clicked(bool) ), this, SLOT( toggleUI( bool ) ) );
    m_toggleUIButton->setWhatsThis(
                tr( "Click here to show or hide the graphical user interface (GUI) of Carla." ) );
    l->addWidget( m_toggleUIButton );


    QWidget* kg=new QWidget(this);
    kg->setFixedSize(228,117+21+0);
    for(int i=0;i<NB_KNOBS;i++)
    {
            m_knobs[i]=new Knob(knobBright_26,kg);
            m_knobs[i]->setLabel(QString("CC %1").arg(NB_KNOB_START+i));
            m_knobs[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
                                    .arg(MIDI_CH).arg(NB_KNOB_START+i),"");
            m_knobs[i]->setWhatsThis("");
            m_knobs[i]->setGeometry(39*(i%6),39*(i/6),39,39);
            m_knobs[i]->setModel(instrument->m_knobs[i]);
            connect(instrument->m_knobs[i], SIGNAL(dataChanged()),
                    this, SLOT(onDataChanged()));
    }
    for(int i=0;i<NB_LEDS;i++)
    {
            m_leds[i]=new LedCheckBox(kg);
            m_leds[i]->setText(QString("CC %1").arg(NB_LED_START+i));
            m_leds[i]->setTextAnchorPoint(Qt::AnchorBottom);
            //m_leds[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
            //                        .arg(MIDI_CH).arg(NB_LED_START+i),"");
            //m_leds[i]->setWhatsThis("");
            m_leds[i]->setGeometry(29*(i%8),117+21*(i/8),29,21);
            m_leds[i]->setModel(instrument->m_leds[i]);
            connect(instrument->m_leds[i], SIGNAL(dataChanged()),
                    this, SLOT(onDataChanged()));
    }
    for(int i=0;i<NB_LCDS;i++)
    {
            m_lcds[i]=new LcdSpinBox(3,kg);
            m_lcds[i]->setText(QString("CC %1").arg(NB_LCD_START+i));
            //m_lcds[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
            //                        .arg(MIDI_CH).arg(NB_LCD_START+i),"");
            //m_lcds[i]->setWhatsThis("");
            m_lcds[i]->setGeometry(58*(i%4),117+21+34*(i/4),58,34);
            m_lcds[i]->setModel(instrument->m_lcds[i]);
            connect(instrument->m_lcds[i],SIGNAL(dataChanged()),
                    this, SLOT(onDataChanged()));
    }
    l->addWidget(kg);

    l->addStretch();

    connect(instrument, SIGNAL(uiClosed()), this, SLOT(uiClosed()));
}

CarlaInstrumentView::~CarlaInstrumentView()
{
    if (m_toggleUIButton->isChecked())
        toggleUI(false);
}

void CarlaInstrumentView::toggleUI(bool visible)
{
    if (fHandle != NULL && fDescriptor->ui_show != NULL)
        fDescriptor->ui_show(fHandle, visible);
}

void CarlaInstrumentView::uiClosed()
{
    m_toggleUIButton->setChecked(false);
}

void CarlaInstrumentView::onDataChanged()
{
        QObject* o=sender();
        if(!o) qInfo("no sender");
        AutomatableModel* m=dynamic_cast<AutomatableModel*>(sender());
        if(m)
        {
                int cc=-1;
                int v =-1;
                if(cc==-1)
                        for(int i=0;i<NB_KNOBS;i++)
                                if(m_knobs[i]->model()==m)
                                {
                                        cc=NB_KNOB_START+i;
                                        v =m_knobs[i]->model()->value();
                                }
                if(cc==-1)
                        for(int i=0;i<NB_LEDS;i++)
                                if(m_leds[i]->model()==m)
                                {
                                        cc=NB_LED_START+i;
                                        v =m_leds[i]->model()->value();
                                }

                if(cc==-1)
                        for(int i=0;i<NB_LCDS;i++)
                                if(m_lcds[i]->model()==m)
                                {
                                        cc=NB_LCD_START+i;
                                        v =m_lcds[i]->model()->value();
                                }
                if(cc!=-1)
                {
                        qInfo("cc %d: data changed: %d",cc,v);
                        CarlaInstrument* fx=dynamic_cast<CarlaInstrument*>(model());
                        if(fx)
                        {
                                MidiEvent ev(MidiEventTypes::MidiControlChange,
                                             MIDI_CH-1,
                                             cc,
                                             v);
                                Song::PlayPos pos=Engine::getSong()->getPlayPos();
                                //qInfo("sending midi event");
                                fx->handleMidiEvent(ev,pos,pos.currentFrame());
                        }
                        else qInfo("fx is null");
                }
                else qInfo("cc model not found");
        }
        else qInfo("sender but no model");
}


void CarlaInstrumentView::modelChanged()
{
}

void CarlaInstrumentView::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == fTimerId)
        fDescriptor->ui_idle(fHandle);

    InstrumentView::timerEvent(event);
}

// -------------------------------------------------------------------

