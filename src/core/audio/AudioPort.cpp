/*
 * AudioPort.cpp - base-class for objects providing sound at a port
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "AudioPort.h"

#include "AudioDevice.h"
#include "Backtrace.h"
#include "BufferManager.h"
#include "EffectChain.h"
#include "Engine.h"
#include "FxMixer.h"
#include "InstrumentPlayHandle.h"
#include "MixHelpers.h"
#include "Mixer.h"
#include "NotePlayHandle.h"
#include "SampleBuffer.h"
#include "Song.h"

#include <QDir>

AudioPort::AudioPort(const QString& _name,
                     bool           _has_effect_chain,
                     BoolModel*     volumeEnabledModel,
                     FloatModel*    volumeModel,
                     BoolModel*     panningEnabledModel,
                     FloatModel*    panningModel,
                     BoolModel*     bendingEnabledModel,
                     FloatModel*    bendingModel,
                     BoolModel*     mutedModel,
                     BoolModel*     frozenModel,
                     BoolModel*     clippingModel) :
      m_bufferUsage(false),
      m_portBuffer(BufferManager::acquire()), m_extOutputEnabled(false),
      m_nextFxChannel(0), m_name(_name),
      m_effects(_has_effect_chain ? new EffectChain(nullptr) : nullptr),
      m_playHandles(true), m_volumeEnabledModel(volumeEnabledModel),
      m_volumeModel(volumeModel), m_panningEnabledModel(panningEnabledModel),
      m_panningModel(panningModel),
      m_bendingEnabledModel(bendingEnabledModel),
      m_bendingModel(bendingModel), m_mutedModel(mutedModel),
      m_frozenModel(frozenModel), m_clippingModel(clippingModel),
      m_frozenBuf(nullptr)
{
    if(m_name.isEmpty())
        m_name = "[unnamed audio port]";
    Engine::mixer()->addAudioPort(this);
    setExtOutputEnabled(true);
}

AudioPort::~AudioPort()
{
    qInfo("AudioPort::~AudioPort '%s'", qPrintable(name()));
    setExtOutputEnabled(false);
    qInfo("AudioPort::~AudioPort 2");
    Engine::mixer()->removeAudioPort(this);

    if(!m_playHandles.isEmpty())
    {
        qInfo("AudioPort::~AudioPort still has %d playhandles",
              m_playHandles.size());
        m_playHandles.map(
                [](PlayHandle* ph) {
                    InstrumentPlayHandle* iph
                            = dynamic_cast<InstrumentPlayHandle*>(ph);
                    qInfo("  type=%d is_iph=%d", ph->type(),
                          (iph != nullptr));
                },
                true);
    }

    qInfo("AudioPort::~AudioPort 3");
    // if(m_effects != nullptr)
    DELETE_HELPER(m_effects);

    qInfo("AudioPort::~AudioPort 4");
    BufferManager::release(m_portBuffer);

    qInfo("AudioPort::~AudioPort 5");
    // if(m_frozenBuf != nullptr)
    DELETE_HELPER(m_frozenBuf);

    qInfo("AudioPort::~AudioPort 6");
}

void AudioPort::setExtOutputEnabled(bool _enabled)
{
    if(_enabled != m_extOutputEnabled)
    {
        m_extOutputEnabled = _enabled;
        if(m_extOutputEnabled)
        {
            Engine::mixer()->audioDev()->registerPort(this);
        }
        else
        {
            Engine::mixer()->audioDev()->unregisterPort(this);
        }
    }
}

void AudioPort::setName(const QString& _name)
{
    m_name = _name;
    Engine::mixer()->audioDev()->renamePort(this);
}

bool AudioPort::processEffects()
{
    if(m_effects)
    {
        bool more = m_effects->processAudioBuffer(
                m_portBuffer, Engine::mixer()->framesPerPeriod(),
                m_bufferUsage);
        return more;
    }
    return false;
}

void AudioPort::doProcessing()
{
    if(m_mutedModel && m_mutedModel->value())
    {
        return;
    }

    const Song*   song = Engine::getSong();
    const fpp_t   fpp  = Engine::mixer()->framesPerPeriod();
    const f_cnt_t af   = song->getPlayPos().absoluteFrame();

    if(m_frozenModel && m_frozenModel->value()
       && (song->playMode() == Song::Mode_PlaySong) && song->isPlaying())
    {
        /*
        if(m_frozenBuf&&(af+fpp<=m_frozenLen))
        {
                //qInfo("AudioPort::doProcessing use frozen buffer"
                //  " af=%d s=%p t=%p",
                //  af,m_portBuffer,this);
                //memset(buf,0,frames*BYTES_PER_FRAME);
                for(f_cnt_t f=0;f<fpp;++f)
                        for(int c=0; c<2; ++c)
                                m_portBuffer[f][c]=m_frozenBuf[af+f][c];

                // send output to fx mixer
                Engine::fxMixer()->mixToChannel( m_portBuffer, m_nextFxChannel
        );
                // TODO: improve the flow here - convert to pull model
                m_bufferUsage = false;
                return;
        }
        */
        if(m_frozenBuf)
        {
            /*
              qInfo("AudioPort::doProcessing use frozen buffer"
                  " fb=%p af=%d s=%p ap=%p",
                    m_frozenBuf,af,m_portBuffer,this);
            */
            for(f_cnt_t f = 0; f < fpp; ++f)
            {
                sample_t vch0, vch1;
                m_frozenBuf->getDataFrame(af + f, vch0, vch1);
                m_portBuffer[f][0] = vch0;
                m_portBuffer[f][1] = vch1;
                /*
                if(af+f>=1000 && af+f<1005)
                        qInfo("AudioPort::doProcessing use frozen buffer"
                              " fb=%p af=%d s=%p ap=%p vch0=%f vch1=%f",
                              m_frozenBuf,af+f,m_portBuffer,this,vch0,vch1);
                */
            }

            // send output to fx mixer
            Engine::fxMixer()->mixToChannel(m_portBuffer, m_nextFxChannel);
            // TODO: improve the flow here - convert to pull model
            m_bufferUsage = false;
            return;
        }
    }

    // clear the buffer
    BufferManager::clear(m_portBuffer);

    // qInfo("AudioPort::doProcessing #1");
    // qDebug( "Playhandles: %d", m_playHandles.size() );
    // m_playHandleLock.lock();
    // now we mix all playhandle buffers into the audioport buffer
    // for(PlayHandle* ph: m_playHandles)
    m_playHandles.map([this, fpp](PlayHandle* ph) {
        /*
          if(ph->isFinished())
          {
          qInfo("AudioPort::doProcessing skip finished ph");
          }
          else
        */
        {
            sampleFrame* buf = ph->buffer();
            if(buf)
            {
                // if(ph->type() == PlayHandle::TypeInstrumentPlayHandle)
                // qWarning("ph->type() ==
                // PlayHandle::TypeInstrumentPlayHandle");
                if(ph->usesBuffer()
                   && (ph->type() == PlayHandle::TypeNotePlayHandle
                       || !MixHelpers::isSilent(buf, fpp)))
                {
                    this->m_bufferUsage = true;
                    MixHelpers::add(this->m_portBuffer, buf, fpp);
                }
                /*
                else if(ph->usesBuffer()
                        && (ph->type() == PlayHandle::TypeSamplePlayHandle))
                {
                    if(MixHelpers::isSilent(buf, fpp))
                        qInfo("AudioPort::doProcessing buf is silent");
                }
                */

                // gets rid of playhandle's buffer and sets
                // pointer to null, so if it doesn't get re-acquired we know
                // to skip it next time
                ph->releaseBuffer();
            }
        }
    });
    // m_playHandleLock.unlock();

    // qInfo("AudioPort::doProcessing #2");
    if(m_bufferUsage)
    {
        if(m_panningModel != nullptr
           && (m_panningEnabledModel == nullptr
               || m_panningEnabledModel->value()))
        {
            // qInfo("AudioPort::doProcessing adjust panning");

            const ValueBuffer* panBuf = m_panningModel->valueBuffer();
            if(panBuf != nullptr)
            {
                const real_t* panArr = panBuf->values();
                for(f_cnt_t f = 0; f < fpp; ++f)
                {
                    const real_t panVal  = panArr[f] * 0.01;
                    const real_t panVal0 = (panVal <= 0. ? 1. : 1. - panVal);
                    const real_t panVal1 = (panVal >= 0. ? 1. : 1. + panVal);
                    m_portBuffer[f][0] *= panVal0;
                    m_portBuffer[f][1] *= panVal1;
                }
            }
            else
            {
                const real_t panVal  = m_panningModel->value() * 0.01;
                const real_t panVal0 = (panVal <= 0. ? 1. : 1. - panVal);
                const real_t panVal1 = (panVal >= 0. ? 1. : 1. + panVal);
                for(f_cnt_t f = 0; f < fpp; ++f)
                {
                    m_portBuffer[f][0] *= panVal0;
                    m_portBuffer[f][1] *= panVal1;
                }
            }
        }

        if(m_volumeModel != nullptr
           && (m_volumeEnabledModel == nullptr
               || m_volumeEnabledModel->value()))
        {
            // qInfo("AudioPort::doProcessing adjust volume");

            const ValueBuffer* volBuf = m_volumeModel->valueBuffer();
            if(volBuf != nullptr)
            {
                const real_t* volArr = volBuf->values();
                for(f_cnt_t f = 0; f < fpp; ++f)
                {
                    const real_t volVal = volArr[f] * 0.01;
                    m_portBuffer[f][0] *= volVal;
                    m_portBuffer[f][1] *= volVal;
                }
            }
            else
            {
                real_t volVal = m_volumeModel->value() * 0.01;
                for(f_cnt_t f = 0; f < fpp; ++f)
                {
                    m_portBuffer[f][0] *= volVal;
                    m_portBuffer[f][1] *= volVal;
                }
            }
        }

        /*
        if(m_bendingEnabledModel==nullptr ||
        m_bendingEnabledModel->value())
        {
                real_t benVal = m_bendingModel->value();
                if(benVal!=0.) qInfo("AudioPort::doProcessing adjust
        bending %f",benVal);
        }
        */

        // handle volume and panning
        // has both vol and pan models
        /*
        if( m_volumeModel && m_panningModel )
        {
                ValueBuffer * volBuf = m_volumeModel->valueBuffer();
                ValueBuffer * panBuf = m_panningModel->valueBuffer();

                // both vol and pan have s.ex.data:
                if( volBuf && panBuf )
                {
                        for( f_cnt_t f = 0; f < fpp; ++f )
                        {
                                float v = volBuf->values()[ f ] * 0.01f;
                                float p = panBuf->values()[ f ] * 0.01f;
                                m_portBuffer[f][0] *= ( p <= 0 ? 1.0f
        : 1.0f - p ) * v; m_portBuffer[f][1] *= ( p >= 0 ? 1.0f : 1.0f + p
        ) * v;
                        }
                }

                // only vol has s.ex.data:
                else if( volBuf )
                {
                        float p = m_panningModel->value() * 0.01f;
                        float l = ( p <= 0 ? 1.0f : 1.0f - p );
                        float r = ( p >= 0 ? 1.0f : 1.0f + p );
                        for( f_cnt_t f = 0; f < fpp; ++f )
                        {
                                float v = volBuf->values()[ f ] * 0.01f;
                                m_portBuffer[f][0] *= v * l;
                                m_portBuffer[f][1] *= v * r;
                        }
                }

                // only pan has s.ex.data:
                else if( panBuf )
                {
                        float v = m_volumeModel->value() * 0.01f;
                        for( f_cnt_t f = 0; f < fpp; ++f )
                        {
                                float p = panBuf->values()[ f ] * 0.01f;
                                m_portBuffer[f][0] *= ( p <= 0 ? 1.0f
        : 1.0f - p ) * v; m_portBuffer[f][1] *= ( p >= 0 ? 1.0f : 1.0f + p
        ) * v;
                        }
                }

                // neither has s.ex.data:
                else
                {
                        float p = m_panningModel->value() * 0.01f;
                        float v = m_volumeModel->value() * 0.01f;
                        for( f_cnt_t f = 0; f < fpp; ++f )
                        {
                                m_portBuffer[f][0] *= ( p <= 0 ? 1.0f
        : 1.0f - p ) * v; m_portBuffer[f][1] *= ( p >= 0 ? 1.0f : 1.0f + p
        ) * v;
                        }
                }
        }

        // has vol model only
        else if( m_volumeModel )
        {
                ValueBuffer * volBuf = m_volumeModel->valueBuffer();

                if( volBuf )
                {
                        for( f_cnt_t f = 0; f < fpp; ++f )
                        {
                                float v = volBuf->values()[ f ] * 0.01f;
                                m_portBuffer[f][0] *= v;
                                m_portBuffer[f][1] *= v;
                        }
                }
                else
                {
                        float v = m_volumeModel->value() * 0.01f;
                        for( f_cnt_t f = 0; f < fpp; ++f )
                        {
                                m_portBuffer[f][0] *= v;
                                m_portBuffer[f][1] *= v;
                        }
                }
        }
        */
    }
    // as of now there's no situation where we only have panning model but
    // no volume model if we have neither, we don't have to do anything
    // here - just pass the audio as is

    // handle effects
    const bool me = processEffects();
    // qInfo("AudioPort::doProcessing #4 me=%d",me);
    if(me || m_bufferUsage)
    {
        /*
        qInfo("AudioPort::doProcessing #5 frozen=%d fb=%p song=%d
        playing==%d", m_frozenModel->value(),m_frozenBuf,
        (song->playMode() == Song::Mode_PlaySong), song->isPlaying());
        */

        if(m_frozenModel && !m_frozenModel->value() && m_frozenBuf
           && (((song->playMode() == Song::Mode_PlaySong)
                && song->isPlaying())
               || (Engine::getSong()->isExporting()
                   && Engine::mixer()->processingSampleRate()
                              == Engine::mixer()->baseSampleRate())))
        {
            // qInfo("AudioPort::doProcessing #6");
            /*
              qInfo("AudioPort::doProcessing freeze"
                  " af=%d s=%p t=%p",
                  af,m_portBuffer,this);
            for(f_cnt_t f=0; f <fpp; ++f)
            for(int c=0; c<2; ++c)
            {
                    if(af+f<m_frozenLen)
                            m_frozenBuf[af+f][c]=m_portBuffer[f][c];
            }
            */
            for(f_cnt_t f = 0; f < fpp; ++f)
            {
                m_frozenBuf->setDataFrame(af + f, m_portBuffer[f][0],
                                          m_portBuffer[f][1]);
                /*
                if(af+f>=1000 && af+f<1005)
                        qInfo("AudioPort::doProcessing freeze to buffer"
                              " fb=%p af=%d s=%p ap=%p vch0=%f vch1=%f",
                              m_frozenBuf,af+f,m_portBuffer,this,
                              m_portBuffer[f][0],m_portBuffer[f][1]);
                */
            }
        }

        // if(MixHelpers::sanitize(m_portBuffer,fpp))
        //        qInfo("AudioPort: sanitize done!!!");

        if(m_clippingModel && MixHelpers::isClipping(m_portBuffer, fpp))
        {
            m_clippingModel->setValue(true);

            if(m_volumeModel != nullptr
               && (m_volumeEnabledModel == nullptr
                   || m_volumeEnabledModel->value())
               && m_volumeModel->valueBuffer() == nullptr)
                m_volumeModel->setAutomatedValue(m_volumeModel->rawValue()
                                                 * 0.995);
        }

        // send output to fx mixer
        Engine::fxMixer()->mixToChannel(m_portBuffer, m_nextFxChannel);
        // TODO: improve the flow here - convert to pull model
        m_bufferUsage = false;
    }
}

void AudioPort::addPlayHandle(PlayHandle* _ph)
{
    if(_ph == nullptr)
    {
        qWarning("AudioPort::addPlayHandle(null)");
        return;
    }

    if(_ph->isFinished())
    {
        qWarning("AudioPort::addPlayHandle ph is finished");
        return;
    }

    if(!Engine::mixer()->isHM())
    {
        BACKTRACE
        qWarning("AudioPort::addPlayHandle not HM thread");
    }

    if(m_playHandles.contains(_ph))
    {
        BACKTRACE
        qWarning("AudioPort::addPlayHandle already contains ph");
    }

    // m_playHandleLock.lock();

    m_playHandles.appendUnique(_ph);

    // m_playHandleLock.unlock();
}

void AudioPort::removePlayHandle(PlayHandle* _ph)
{
    if(_ph == nullptr)
    {
        qWarning("AudioPort::removePlayHandle(null)");
        return;
    }

    if(!_ph->isFinished())
    {
        qWarning("AudioPort::removePlayHandle ph is NOT finished");
        return;
    }

    if(_ph->type() == 2)
        qInfo("AudioPort::removePlayHandle(IPH)");

    if(!Engine::mixer()->isHM())
    {
        BACKTRACE
        qWarning("AudioPort::removePlayHandle not HM thread");
    }

    // m_playHandleLock.lock();

    /*
    NotePlayHandle* nph = dynamic_cast<NotePlayHandle*>(_ph);
    if(nph && !nph->isReleased())
    {
        qWarning("AudioPort::removePlayHandle calling noteOff?");
        nph->noteOff(0);
    }
    */

    if(_ph->type() == 2)
        qInfo("AudioPort: ready to remove");

    int nh = m_playHandles.removeAll(_ph);
    if(nh == 0)  // One
        qWarning("AudioPort::removePlayHandle handle not found type=%d",
                 _ph->type());
    else if(nh > 1)
        qWarning(
                "AudioPort::removePlayHandle handle found more than once "
                "type=%d nh=%d",
                _ph->type(), nh);

    // m_playHandleLock.unlock();

    if(_ph->type() == 2)
        qInfo("AudioPort: removePlayHandle(IPH) DONE!");
}

void AudioPort::updateFrozenBuffer(f_cnt_t _len)
{
    if(m_frozenModel)
    {
        if((m_frozenBuf == nullptr) || (_len != m_frozenBuf->frames()))
        {
            if(m_frozenBuf)
                delete m_frozenBuf;
            m_frozenBuf = new SampleBuffer(_len);
            // qInfo("AudioPort::updateFrozenBuffer len=%d",_len);
        }
    }
}

void AudioPort::cleanFrozenBuffer(f_cnt_t _len)
{
    if(m_frozenModel)
    {
        if((m_frozenBuf == nullptr) || (_len != m_frozenBuf->frames())
           || m_frozenBuf->m_mmapped)
        {
            if(m_frozenBuf)
                delete m_frozenBuf;
            m_frozenBuf = new SampleBuffer(_len);
            // qInfo("AudioPort::cleanFrozenBuffer len=%d",_len);
        }
    }
}

void AudioPort::readFrozenBuffer(QString _uuid)
{
    if(m_frozenModel &&
       // m_frozenModel->value()&&
       m_frozenBuf)
    {
        delete m_frozenBuf;
        m_frozenBuf = nullptr;
        QString d   = Engine::getSong()->projectDir() + QDir::separator()
                    + "tracks" + QDir::separator() + "frozen";
        if(QFileInfo(d).exists())
        {
            QString f = d + QDir::separator() + _uuid + "."
                        + SampleBuffer::rawStereoSuffix();
            // qInfo("AudioPort::readFrozenBuffer f=%s",
            //      qPrintable(f));
            QFile fi(f);
            if(fi.exists())
            {
                if(fi.size() == 0)
                    fi.remove();
                else
                    m_frozenBuf = new SampleBuffer(f);
            }
        }
    }
}

void AudioPort::writeFrozenBuffer(QString _uuid)
{
    if(m_frozenModel &&
       // m_frozenModel->value()&&
       m_frozenBuf)
    {
        QString d = Engine::getSong()->projectDir() + QDir::separator()
                    + "tracks" + QDir::separator() + "frozen";
        if(QFileInfo(d).exists())
        {
            QString f = d + QDir::separator() + _uuid + "."
                        + SampleBuffer::rawStereoSuffix();
            // qInfo("AudioPort::writeFrozenBuffer f=%s",
            //      qPrintable(f));
            m_frozenBuf->writeCacheData(f);
        }
    }
}
