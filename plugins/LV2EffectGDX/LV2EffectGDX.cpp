/*
 * LV2EffectGDX.cpp - class for processing LV2 effects
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

#include "LV2EffectGDX.h"

#include "AudioDevice.h"
#include "AutomationPattern.h"
#include "ConfigManager.h"
#include "ControllerConnection.h"
#include "DataFile.h"
#include "EffectChain.h"
#include "LV22LMMS.h"
#include "MemoryManager.h"
#include "Mixer.h"
#include "Song.h"
#include "ValueBuffer.h"
#include "debug.h"
#include "embed.h"

#include <QMessageBox>
//#include "LV2Control.h"
#include "LV2EffectGDXSubPluginFeatures.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT lv2effectgdx_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "LV2",
               QT_TRANSLATE_NOOP("LV2 Effect GDX",
                                 "host for using an arbitrary LV2 effect "
                                 "inside LMMS."),
               "gi0e5b06 (on github.com)",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo.png"),
               NULL,
               new LV2EffectGDXSubPluginFeatures(Plugin::Effect)};
}

LV2EffectGDX::LV2EffectGDX(Model*                                    _parent,
                           const Descriptor::SubPluginFeatures::Key* _key) :
      Effect(&lv2effectgdx_plugin_descriptor, _parent, _key),
      m_controls(NULL),
      // m_maxSampleRate( 0 ),
      m_uri(LV2EffectGDXSubPluginFeatures::subPluginKeyToLV2Key(_key))
{
    qInfo("LV2EffectGDX::LV2EffectGDX beginning");
    setColor(QColor(59,128,74));

    LV22LMMS* manager = Engine::getLV2Manager();

    QString name = manager->getName(m_uri);
    if(name == "")  // manager->getDescription( m_uri ) == NULL )
    {
        Engine::getSong()->collectError(
                tr("Unknown LV2 effect %1 requested.").arg(m_uri));
        setOkay(false);
        return;
    }

    setDisplayName(name);

    qInfo("LV2EffectGDX::LV2EffectGDX pluginInstantiation()");
    setObjectName(QString("LV2EffectGDX-%1").arg((unsigned long)this, 16));
    pluginInstantiation();

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(changeSampleRate()));
}

LV2EffectGDX::~LV2EffectGDX()
{
    pluginDestruction();
}

/*
uint32_t LV2EffectGDX::portCount()
{
        LV22LMMS* manager=Engine::getLV2Manager();
        return manager->getPortCount(m_uri);
}
*/

void LV2EffectGDX::changeSampleRate()
{
    DataFile dataFile(DataFile::EffectSettings);
    m_controls->saveState(dataFile, dataFile.content());

    LV2EffectGDXControls* old_controls = m_controls;
    m_controls                         = NULL;

    m_pluginMutex.lock();
    pluginDestruction();
    pluginInstantiation();
    m_pluginMutex.unlock();

    old_controls->effectModelChanged(m_controls);
    delete old_controls;

    m_controls->restoreState(dataFile.content().firstChild().toElement());

    // the IDs of re-created controls have been saved and now need to be
    // resolved again
    AutomationPattern::resolveAllIDs();

    // make sure, connections are ok
    ControllerConnection::finalizeConnections();
}

bool LV2EffectGDX::processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
{
    m_pluginMutex.lock();

    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(_buf, _frames, smoothBegin, smoothEnd))
    {
        // qInfo("okay=%d dontRun=%d isRunning=%d isEnabled=%d",
        //      isOkay(),dontRun(),isRunning(),isEnabled());
        m_pluginMutex.unlock();
        return false;
    }

    // int frames = _frames;
    // sampleFrame * o_buf = NULL;
    // sampleFrame sBuf [_frames];

    /*
    if( m_maxSampleRate < Engine::mixer()->processingSampleRate() )
    {
            o_buf = _buf;
            _buf = &sBuf[0];
            sampleDown( o_buf, _buf, m_maxSampleRate );
            frames = _frames * m_maxSampleRate /
                            Engine::mixer()->processingSampleRate();
    }
    */

    // Copy the LMMS audio buffer to the LV2 input buffer and initialize
    // the control ports.
    ch_cnt_t channel = 0;
    for(ch_cnt_t proc = 0; proc < processorCount(); ++proc)
    {
        for(int port = 0; port < m_portCount; ++port)
        {
            lv2_port_desc_t* pp = m_ports.at(proc).at(port);
            switch(pp->rate)
            {
                case CHANNEL_IN:
                    for(fpp_t frame = 0; frame < _frames; ++frame)
                        pp->buffer[frame].f = _buf[frame][channel];
                    ++channel;
                    break;
                case AUDIO_RATE_INPUT:
                {
                    ValueBuffer* vb = pp->control->valueBuffer();
                    if(vb)
                    {
                        //memcpy(pp->buffer, vb->values(),
                        //       _frames * sizeof(FLOAT));
                        for(fpp_t frame = 0; frame < _frames; ++frame)
                            pp->buffer[frame] = vb->value(frame);
                    }
                    else
                    {
                        pp->value.f = pp->control->value().f / pp->scale;
                        // This only supports control rate ports, so the audio
                        // rates are treated as though they were control rate
                        // by setting the port buffer to all the same value.
                        for(fpp_t frame = 0; frame < _frames; ++frame)
                            pp->buffer[frame] = pp->value;
                    }
                    break;
                }
                case CONTROL_RATE_INPUT:
                    if(pp->control == nullptr)
                        break;

                    pp->value.f   = pp->control->value().f / pp->scale;
                    pp->buffer[0] = pp->value;
                    break;
                case CHANNEL_OUT:
                case AUDIO_RATE_OUTPUT:
                case CONTROL_RATE_OUTPUT:
                    break;
                default:
                    break;
            }
        }
    }

    LV22LMMS* manager = Engine::getLV2Manager();
    // Process the buffers.
    for(ch_cnt_t proc = 0; proc < processorCount(); ++proc)
    {
        //(m_descriptor->run)( m_handles[proc], _frames );
        // tmp
        manager->run(m_uri, m_handles[proc], _frames);
    }

    // Copy the LV2 output buffers to the LMMS buffer.
    channel = 0;
    for(ch_cnt_t proc = 0; proc < processorCount(); ++proc)
    {
        for(int port = 0; port < m_portCount; ++port)
        {
            lv2_port_desc_t* pp = m_ports.at(proc).at(port);
            switch(pp->rate)
            {
                case CHANNEL_IN:
                case AUDIO_RATE_INPUT:
                case CONTROL_RATE_INPUT:
                    break;
                case CHANNEL_OUT:
                    for(fpp_t f = 0; f < _frames; ++f)
                    {
                        real_t w0, d0, w1, d1;
                        computeWetDryLevels(f, _frames, smoothBegin,
                                            smoothEnd, w0, d0, w1, d1);

                        if(channel == 0)
                            _buf[f][channel] = d0 * _buf[f][channel]
                                                   + w0 * pp->buffer[f].f;
                        else if(channel == 1)
                            _buf[f][channel] = d1 * _buf[f][channel]
                                                   + w1 * pp->buffer[f].f;
                    }
                    ++channel;
                    break;
                case AUDIO_RATE_OUTPUT:
                case CONTROL_RATE_OUTPUT:
                    break;
                default:
                    break;
            }
        }
    }

    /*
    if( o_buf != NULL )
    {
            sampleBack( _buf, o_buf, m_maxSampleRate );
    }
    */

    m_pluginMutex.unlock();
    return true;
}

void LV2EffectGDX::setControl(int _control, LV2_Data _value)
{
    if(!isOkay())
    {
        return;
    }
    m_portControls[_control]->value = _value;
}

uint32_t LV2EffectGDX::inputChannels()
{
    LV22LMMS* manager = Engine::getLV2Manager();

    uint32_t r = 0;
    for(int i = 0; i < m_portCount; i++)
        if(manager->isPortInput(m_uri, i) && manager->isPortAudio(m_uri, i))
            r++;
    return r;
}

uint32_t LV2EffectGDX::outputChannels()
{
    LV22LMMS* manager = Engine::getLV2Manager();

    uint32_t r = 0;
    for(int i = 0; i < m_portCount; i++)
        if(manager->isPortOutput(m_uri, i) && manager->isPortAudio(m_uri, i))
            r++;
    return r;
}

void LV2EffectGDX::pluginInstantiation()
{
    qInfo("LV2EffectGDX::pluginInstantiation()");

    // m_maxSampleRate = maxSamplerate( displayName() );

    LV22LMMS* manager = Engine::getLV2Manager();

    // get inPlaceBroken property
    m_inPlaceBroken = manager->isInplaceBroken(m_uri);

    // Categorize the ports, and create the buffers.
    m_portCount = manager->getPortCount(m_uri);

    // Calculate how many processing units are needed.
    const ch_cnt_t lmms_chnls      = Engine::mixer()->audioDev()->channels();
    int            effect_channels = inputChannels();
    // manager->getDescription( m_uri )->inputChannels;
    setProcessorCount(lmms_chnls / effect_channels);

    qInfo("LV2EffectGDX::pluginInstantiation effect_channels=%d "
          "processors=%d",
          effect_channels, processorCount());

    int       inputch  = 0;
    int       outputch = 0;
    LV2_Data* inbuf[2];
    inbuf[0] = NULL;
    inbuf[1] = NULL;
    for(ch_cnt_t proc = 0; proc < processorCount(); proc++)
    {
        lv2_multi_proc_t ports;
        for(int port = 0; port < m_portCount; port++)
        {
            lv2_port_desc_t* p = new LV2PortDescription;

            p->name    = manager->getPortName(m_uri, port);
            p->proc    = proc;
            p->port_id = port;
            p->control = NULL;
            p->buffer  = NULL;

            // Determine the port's category.
            if(manager->isPortAudio(m_uri, port))
            {
                if(p->name.toUpper().contains("IN")
                   && manager->isPortInput(m_uri, port))
                {
                    p->rate        = CHANNEL_IN;
                    p->buffer      = MM_ALLOC(LV2_Data,
                                         Engine::mixer()->framesPerPeriod());
                    inbuf[inputch] = p->buffer;
                    inputch++;
                }
                else if(p->name.toUpper().contains("OUT")
                        && manager->isPortOutput(m_uri, port))
                {
                    p->rate = CHANNEL_OUT;
                    if(!m_inPlaceBroken && inbuf[outputch])
                    {
                        p->buffer = inbuf[outputch];
                        outputch++;
                    }
                    else
                    {
                        p->buffer = MM_ALLOC(
                                LV2_Data, Engine::mixer()->framesPerPeriod());
                        m_inPlaceBroken = true;
                    }
                }
                else if(manager->isPortInput(m_uri, port))
                {
                    p->rate   = AUDIO_RATE_INPUT;
                    p->buffer = MM_ALLOC(LV2_Data,
                                         Engine::mixer()->framesPerPeriod());
                }
                else
                {
                    p->rate   = AUDIO_RATE_OUTPUT;
                    p->buffer = MM_ALLOC(LV2_Data,
                                         Engine::mixer()->framesPerPeriod());
                }
            }
            else if(manager->isPortControl(m_uri, port))
            {
                p->buffer = MM_ALLOC(LV2_Data, 1);

                if(manager->isPortInput(m_uri, port))
                {
                    p->rate = CONTROL_RATE_INPUT;
                }
                else
                {
                    p->rate = CONTROL_RATE_OUTPUT;
                }
            }
            else
            {
                p->buffer = MM_ALLOC(LV2_Data, 1);
                p->rate   = IGNORED;
            }

            p->scale = 1.0f;
            if(manager->isPortToggled(m_uri, port))
            {
                p->data_type = TOGGLED;
            }
            else if(manager->isInteger(m_uri, port))
            {
                p->data_type = INTEGER;
            }
            else if(p->name.toUpper().contains("(SECONDS)"))
            {
                p->data_type = TIME;
                p->scale     = 1000.0f;
                int loc      = p->name.toUpper().indexOf("(SECONDS)");
                p->name.replace(loc, 9, "(ms)");
            }
            else if(p->name.toUpper().contains("(S)"))
            {
                p->data_type = TIME;
                p->scale     = 1000.0f;
                int loc      = p->name.toUpper().indexOf("(S)");
                p->name.replace(loc, 3, "(ms)");
            }
            else if(p->name.toUpper().contains("(MS)"))
            {
                p->data_type = TIME;
                int loc      = p->name.toUpper().indexOf("(MS)");
                p->name.replace(loc, 4, "(ms)");
            }
            else
            {
                p->data_type = FLOATING;
            }

            // Get the range and default values.
            p->max.f = manager->getUpperBound(m_uri, port);
            /*
              if( p->max == NOHINT )
            {
                    p->max = p->name.toUpper() == "GAIN" ? 10.0f :
                            1.0f;
            }
            */

            /*
            if( manager->areHintsSampleRateDependent(
                                                    m_uri, port ) )
            {
                    p->max *= m_maxSampleRate;
            }
            */

            p->min.f = manager->getLowerBound(m_uri, port);
            /*
            if( p->min == NOHINT )
            {
                    p->min = 0.0f;
            }
            */

            /*
            if( manager->areHintsSampleRateDependent(
                                                    m_uri, port ) )
            {
                    p->min *= m_maxSampleRate;
            }
            */

            p->def.f = manager->getDefaultSetting(m_uri, port);
            /*
            if( p->def == NOHINT )
            {
                    if( p->data_type != TOGGLED )
                    {
                            p->def = ( p->min + p->max ) / 2.0f;
                    }
                    else
                    {
                            p->def = 1.0f;
                    }
            }
            else
            if( manager->areHintsSampleRateDependent( m_uri, port ) )
            {
                    p->def *= m_maxSampleRate;
            }
            */

            p->max.f *= p->scale;
            p->min.f *= p->scale;
            p->def.f *= p->scale;

            p->value.f = p->def.f;

            p->suggests_logscale = manager->isLogarithmic(m_uri, port);

            ports.append(p);

            // For convenience, keep a separate list of the ports that are
            // used to control the processors.
            if(p->rate == AUDIO_RATE_INPUT || p->rate == CONTROL_RATE_INPUT)
            {
                p->control_id = m_portControls.count();
                m_portControls.append(p);
            }
        }
        m_ports.append(ports);
    }

    // Instantiate the processing units.
    qInfo("LV2EffectGDX::pluginInstantiation 2");

    /*
    m_descriptor = manager->getDescriptor( m_uri );
    if( m_descriptor == NULL )
    {
            QMessageBox::warning( 0, "Effect",
                                  "Can't get LV2 descriptor function:
    "+m_uri,//.second QMessageBox::Ok, QMessageBox::NoButton ); setOkay( false
    ); return;
    }
    if( m_descriptor->run == NULL )
    {
            QMessageBox::warning( 0, "Effect",
                                  "Plugin has no processor: " +
    m_uri,//.second, QMessageBox::Ok, QMessageBox::NoButton ); setDontRun(
    true );
    }
    */

    QString name = manager->getName(m_uri);
    if(name == "")
    {
        QMessageBox::warning(0, "LV2 Effect", "Can't find: " + m_uri,
                             QMessageBox::Ok, QMessageBox::NoButton);
        setOkay(false);
        return;
    }

    qInfo("LV2EffectGDX::pluginInstantiation 3");
    for(ch_cnt_t proc = 0; proc < processorCount(); proc++)
    {
        LV2_Instance effect = manager->instantiate(
                m_uri,
                Engine::mixer()
                        ->processingSampleRate());  // m_maxSampleRate);
        if(effect == NULL)
        {
            QMessageBox::warning(0, "LV2 Effect: " + name,
                                 "Can't create: " + m_uri, QMessageBox::Ok,
                                 QMessageBox::NoButton);
            setOkay(false);
            return;
        }
        m_handles.append(effect);
    }

    qInfo("LV2EffectGDX::pluginInstantiation 4");
    // Connect the ports.
    for(ch_cnt_t proc = 0; proc < processorCount(); proc++)
    {
        for(int port = 0; port < m_portCount; port++)
        {
            lv2_port_desc_t* pp = m_ports.at(proc).at(port);
            if(!manager->connectPort(m_uri, m_handles[proc], port,
                                     pp->buffer))
            {
                QMessageBox::warning(0, "LV2 Effect: " + name,
                                     "Failed to connect port: " + m_uri,
                                     QMessageBox::Ok, QMessageBox::NoButton);
                setDontRun(true);
                return;
            }
        }
    }

    qInfo("LV2EffectGDX::pluginInstantiation 5");
    // Activate the processing units.
    for(ch_cnt_t proc = 0; proc < processorCount(); proc++)
    {
        manager->activate(m_uri, m_handles[proc]);
    }

    qInfo("LV2EffectGDX::pluginInstantiation 6");
    m_controls = new LV2EffectGDXControls(this);
    qInfo("LV2EffectGDX::pluginInstantiation 7");
}

void LV2EffectGDX::pluginDestruction()
{
    if(!isOkay())
    {
        return;
    }

    delete m_controls;

    for(ch_cnt_t proc = 0; proc < processorCount(); proc++)
    {
        LV22LMMS* manager = Engine::getLV2Manager();
        manager->deactivate(m_uri, m_handles[proc]);
        manager->cleanup(m_uri, m_handles[proc]);
        for(int port = 0; port < m_portCount; port++)
        {
            lv2_port_desc_t* pp = m_ports.at(proc).at(port);
            if(m_inPlaceBroken || pp->rate != CHANNEL_OUT)
            {
                if(pp->buffer)
                    MM_FREE(pp->buffer);
            }
            delete pp;
        }
        m_ports[proc].clear();
    }
    m_ports.clear();
    m_handles.clear();
    m_portControls.clear();
}

static QMap<QString, sample_rate_t> __buggy_plugins;

/*
sample_rate_t LV2EffectGDX::maxSamplerate( const QString & _name )
{
        if( __buggy_plugins.isEmpty() )
        {
                __buggy_plugins["C* AmpVTS"] = 88200;
                __buggy_plugins["Chorus2"] = 44100;
                __buggy_plugins["Notch Filter"] = 96000;
                __buggy_plugins["TAP Reflector"] = 192000;
        }
        if( __buggy_plugins.contains( _name ) )
        {
                return( __buggy_plugins[_name] );
        }
        return( Engine::mixer()->processingSampleRate() );
}
*/

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* _parent, void* _data)
    {
        qInfo("LV2EffectGDX::lmms_plugin_main data=%p", _data);
        return new LV2EffectGDX(
                _parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        _data));
    }
}
