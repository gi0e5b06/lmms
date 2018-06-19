/*
 * Carla for LMMS
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

#include "CarlaEffect.h"

#define REAL_BUILD // FIXME this shouldn't be needed
#include "CarlaHost.h"

#include "Engine.h"
#include "Song.h"
#include "gui_templates.h"
//#include "InstrumentPlayHandle.h"
//#include "InstrumentTrack.h"
//#include "Mixer.h"

#include <QApplication>
#include <QFileDialog>
#include <QPushButton>
#include <QTimerEvent>
#include <QVBoxLayout>

#include <cstring>

#include "embed.h"
#include "CarlaEffect.h"
#include "CarlaEffectControls.h"
#include "CarlaEffectControlDialog.h"
//#include "CarlaEffectView.h"

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

#define handlePtr ((CarlaEffect*)handle)

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

CarlaEffect::CarlaEffect(Model* parent, const Descriptor* const descriptor, const bool isPatchbay)
        : Effect(descriptor,parent,NULL),
          m_gdxControls( new CarlaEffectControls( this )),
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

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this, SLOT(sampleRateChanged()));
}

CarlaEffect::~CarlaEffect()
{
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

uint32_t CarlaEffect::handleGetBufferSize() const
{
    return Engine::mixer()->framesPerPeriod();
}

double CarlaEffect::handleGetSampleRate() const
{
    return Engine::mixer()->processingSampleRate();
}

bool CarlaEffect::handleIsOffline() const
{
    return false; // TODO
}

const NativeTimeInfo* CarlaEffect::handleGetTimeInfo() const
{
    return &fTimeInfo;
}

void CarlaEffect::handleUiParameterChanged(const uint32_t /*index*/, const float /*value*/) const
{
}

void CarlaEffect::handleUiClosed()
{
    emit uiClosed();
}

intptr_t CarlaEffect::handleDispatcher(const NativeHostDispatcherOpcode opcode, const int32_t index, const intptr_t value, void* const ptr, const float opt)
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

QString CarlaEffect::nodeName() const
{
    return  descriptor()->name;
}

void CarlaEffect::saveSettings(QDomDocument& doc, QDomElement& parent)
{
    if (fHandle == NULL || fDescriptor->get_state == NULL)
        return;

    char* const state = fDescriptor->get_state(fHandle);

    if (state == NULL)
        return;

    QDomDocument carlaDoc("carla");

    if (carlaDoc.setContent(QString(state)))
    {
        QDomNode n = doc.importNode(carlaDoc.documentElement(), true);
        parent.appendChild(n);
    }

    std::free(state);
}

void CarlaEffect::loadSettings(const QDomElement& elem)
{
    if (fHandle == NULL || fDescriptor->set_state == NULL)
        return;

    QDomDocument carlaDoc("carla");
    carlaDoc.appendChild(carlaDoc.importNode(elem.firstChildElement(), true ));

    fDescriptor->set_state(fHandle, carlaDoc.toString(0).toUtf8().constData());
}

//void CarlaEffect::play(sampleFrame* workingBuffer)
bool CarlaEffect::processAudioBuffer(sampleFrame* buf, const fpp_t frames)
{
    if( !isEnabled() || !isRunning () )
    {
            return false;
    }

    const uint bufsize = frames;//Engine::mixer()->framesPerPeriod();


    if(fHandle==NULL)
    {
            //std::memset(workingBuffer, 0, sizeof(sampleFrame)*bufsize);
            return false;
    }

    // set time info
    Song* const s = Engine::getSong();
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
    //std::memset(buf1, 0, sizeof(float)*bufsize);
    //std::memset(buf2, 0, sizeof(float)*bufsize);

    float curVal0;
    float curVal1;

    double outSum = 0.0;

    for(fpp_t f=0;f<frames;f++)
    {
            buf1[f]=buf[f][0];
            buf2[f]=buf[f][1];

            curVal0=buf[f][0];
            curVal1=buf[f][1];
            outSum += curVal0*curVal0+curVal1*curVal1;
    }

    {
        const QMutexLocker ml(&fMutex);
        fDescriptor->process(fHandle, rBuf, rBuf, bufsize, fMidiEvents, fMidiEventCount);
        fMidiEventCount = 0;
    }

    const float d = dryLevel();
    const float w = wetLevel();

    for(fpp_t f=0;f<frames;f++)
    {
            buf[f][0]=d*buf[f][0]+w*buf1[f];
            buf[f][1]=d*buf[f][1]+w*buf2[f];
    }

    checkGate( outSum / frames );

    return isRunning();
}


EffectControls* CarlaEffect::controls()
{
        return m_gdxControls;
}


void CarlaEffect::sampleRateChanged()
{
    fDescriptor->dispatcher(fHandle, NATIVE_PLUGIN_OPCODE_SAMPLE_RATE_CHANGED, 0, 0, nullptr, handleGetSampleRate());
}


/*
PluginView* CarlaEffect::createNativeView(QWidget* parent)
{
    if (QWidget* const window = parent->window())
        fHost.uiParentId = window->winId();
    else
        fHost.uiParentId = 0;

    std::free((char*)fHost.uiName);

    // TODO - get plugin instance name
    //fHost.uiName = strdup(parent->windowTitle().toUtf8().constData());
    fHost.uiName = strdup(kIsPatchbay ? "CarlaEffectBay-LMMS" : "CarlaEffectRack-LMMS");

    return new CarlaEffectView(this, parent);
}
*/
