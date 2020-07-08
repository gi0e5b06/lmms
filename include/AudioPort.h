/*
 * AudioPort.h - base-class for objects providing sound at a port
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

#ifndef AUDIO_PORT_H
#define AUDIO_PORT_H

#include <QMutex>
#include <QSharedPointer>
#include <QString>
//#include <QMutexLocker>

#include "AutomatableModel.h"
#include "MemoryManager.h"
#include "PlayHandle.h"
#include "SafeList.h"
#include "ThreadableJob.h"

class AudioPort;
class EffectChain;
// class FloatModel;
// class BoolModel;
class SampleBuffer;
// class PlayHandle;

// typedef QSharedPointer<PlayHandle> PlayHandlePointer;
typedef QSharedPointer<AudioPort>  AudioPortPointer;
typedef SafeList<AudioPortPointer> AudioPorts;

class AudioPort : public ThreadableJob
{
    MM_OPERATORS
  public:
    AudioPort(const QString& _name,
              bool           _has_effect_chain,    // = true,
              BoolModel*     volumeEnabledModel,   // = nullptr,
              FloatModel*    volumeModel,          // = nullptr,
              BoolModel*     panningEnabledModel,  // = nullptr,
              FloatModel*    panningModel,         // = nullptr,
              BoolModel*     bendingEnabledModel,  // = nullptr,
              FloatModel*    bendingModel,         // = nullptr,
              BoolModel*     mutedModel,           // = nullptr,
              BoolModel*     frozenModel,          // = nullptr,
              BoolModel*     clippingModel);           // = nullptr);
    virtual ~AudioPort();

    inline sampleFrame* buffer()
    {
        return m_portBuffer;
    }

    /*
    inline void lockBuffer()
    {
        m_portBufferLock.lock();
    }

    inline void unlockBuffer()
    {
        m_portBufferLock.unlock();
    }
    */

    // indicate whether JACK & Co should provide output-buffer at ext. port
    inline bool extOutputEnabled() const
    {
        return m_extOutputEnabled;
    }

    void setExtOutputEnabled(bool _enabled);

    // next effect-channel after this audio-port
    // (-1 = none  0 = master)
    inline fx_ch_t nextFxChannel() const
    {
        return m_nextFxChannel;
    }

    inline EffectChain* effects()
    {
        return m_effects;
    }

    void setNextFxChannel(const fx_ch_t _chnl)
    {
        m_nextFxChannel = _chnl;
    }

    const QString& name() const
    {
        return m_name;
    }

    void setName(const QString& _new_name);

    bool processEffects();

    virtual bool requiresProcessing() const final
    {
        // return !m_playHandles.isEmpty();
        return true;
    }

    void lock()
    {
        m_processingLock.lock();
    }

    void unlock()
    {
        m_processingLock.unlock();
    }

    bool tryLock()
    {
        return m_processingLock.tryLock();
    }

    void addPlayHandle(PlayHandlePointer handle);
    void removePlayHandle(PlayHandlePointer handle);

    void updateFrozenBuffer(f_cnt_t _len);
    void cleanFrozenBuffer(f_cnt_t _len);
    void readFrozenBuffer(QString _uuid);
    void writeFrozenBuffer(QString _uuid);

    virtual AudioPortPointer& pointer()
    {
        return *m_pointer;
    }

    virtual const AudioPortPointer& pointer() const
    {
        return *m_pointer;
    }

  protected:
    virtual void doProcessing() final;

  private:
    volatile bool m_bufferUsage;
    sampleFrame*  m_portBuffer;
    // QMutex                      m_portBufferLock;
    Mutex                       m_processingLock;
    bool                        m_extOutputEnabled;
    fx_ch_t                     m_nextFxChannel;
    QString                     m_name;
    EffectChain*                m_effects;
    SafeList<PlayHandlePointer> m_playHandles;
    BoolModel*                  m_volumeEnabledModel;
    FloatModel*                 m_volumeModel;
    BoolModel*                  m_panningEnabledModel;
    FloatModel*                 m_panningModel;
    BoolModel*                  m_bendingEnabledModel;
    FloatModel*                 m_bendingModel;
    BoolModel*                  m_mutedModel;
    BoolModel*                  m_frozenModel;
    BoolModel*                  m_clippingModel;
    SampleBuffer*               m_frozenBuf;
    AudioPortPointer*           m_pointer;

    friend class Mixer;
    friend class MixerWorkerThread;
};

#endif
