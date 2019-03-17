/*
 * SamplePlayHandle.cpp - implementation of class SamplePlayHandle
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

#include "SamplePlayHandle.h"

#include "AudioPort.h"
#include "BBTrack.h"
#include "Engine.h"
#include "Instrument.h"
#include "MixHelpers.h"
#include "Mixer.h"
#include "SampleTrack.h"

SamplePlayHandle::SamplePlayHandle(SampleBuffer* sampleBuffer,
                                   bool          ownAudioPort) :
      PlayHandle(TypeSamplePlayHandle),
      m_sampleBuffer(sharedObject::ref(sampleBuffer)),
      m_doneMayReturnTrue(true), m_totalFramesPlayed(0),
      m_ownAudioPort(ownAudioPort),
      m_defaultVolumeModel(DefaultVolume, MinVolume, MaxVolume, 1),
      m_volumeModel(&m_defaultVolumeModel), m_track(nullptr),
      m_bbTrack(nullptr)
{
    m_frames = m_sampleBuffer->frames();
    setCurrentFrame(0);
    if(ownAudioPort)
    {
        setAudioPort(new AudioPort("[SamplePlayHandle AudioPort]", false,
                                   nullptr, m_volumeModel, nullptr, nullptr,
                                   nullptr, nullptr, nullptr, nullptr,
                                   nullptr));
    }
}

SamplePlayHandle::SamplePlayHandle(const QString& sampleFile) :
      SamplePlayHandle(new SampleBuffer(sampleFile), true)
{
    sharedObject::unref(m_sampleBuffer);
}

SamplePlayHandle::SamplePlayHandle(SampleTCO* tco) :
      SamplePlayHandle(tco->sampleBuffer(), false)
{
    m_track = tco->getTrack();
    m_totalFramesPlayed
            = 0;  // tco->initialPlayTick() * Engine::framesPerTick();
    m_frames = (/*tco->initialPlayTick() +*/ tco->length())
               * Engine::framesPerTick();
    setCurrentFrame(tco->initialPlayTick() * Engine::framesPerTick());
    setAudioPort(((SampleTrack*)tco->getTrack())->audioPort());
}

SamplePlayHandle::~SamplePlayHandle()
{
    sharedObject::unref(m_sampleBuffer);
    if(m_ownAudioPort)
    {
        AudioPort* ap = audioPort();
        if(ap == nullptr)
        {
            qWarning("SamplePlayHandle::~SamplePlayHandle ap==nullptr");
        }
        else
        {
            qWarning("SamplePlayHandle::~SamplePlayHandle delete");
            setAudioPort(nullptr);
            delete ap;
        }
    }
}

f_cnt_t SamplePlayHandle::currentFrame()
{
    return m_state.frameIndex();
}

void SamplePlayHandle::setCurrentFrame(f_cnt_t _f)
{
    m_state.setFrameIndex(_f);
}

f_cnt_t SamplePlayHandle::autoRepeat()
{
    return m_autoRepeat;
}

void SamplePlayHandle::setAutoRepeat(f_cnt_t _a)
{
    m_autoRepeat = _a;
}

void SamplePlayHandle::play(sampleFrame* buffer)
{
    // qInfo("SamplePlayHandle::play buffer=%p", buffer);

    const fpp_t fpp = Engine::mixer()->framesPerPeriod();
    // play( 0, _try_parallelizing );
    if(framesDone() >= frames())  // totalFrames() )
    {
        memset(buffer, 0, BYTES_PER_FRAME * fpp);
        return;
    }

    sampleFrame* workingBuffer = buffer;
    f_cnt_t      frames        = fpp;

    // apply offset for the first period
    if(framesDone() == 0)
    {
        memset(buffer, 0, BYTES_PER_FRAME * offset());
        workingBuffer += offset();
        frames -= offset();
    }

    if(!(m_track && m_track->isMuted())
       && !(m_bbTrack && m_bbTrack->isMuted()))
    {
        /*
          StereoVolume vv =
                {{ m_volumeModel->value() / DefaultVolume,
                   m_volumeModel->value() / DefaultVolume }};
        */

        f_cnt_t a = autoRepeat();
        if(a > 0)
        {
            f_cnt_t f = currentFrame();
            if(f >= a)
                f %= a;
            if(f < 0)
                f = 0;
            setCurrentFrame(f);
            // qInfo("f=%d a=%d", f, a);
        }

        // qWarning("SamplePlayHandle::play workingBuffer=%p",workingBuffer);
        if(!m_sampleBuffer->play(workingBuffer, &m_state, frames, BaseFreq))
        {
            // qWarning("SamplePlayHandle::play not played
            // workingBuffer=%p",workingBuffer);
            memset(workingBuffer, 0, frames * BYTES_PER_FRAME);
        }
    }

    m_totalFramesPlayed += frames;

    // if(MixHelpers::isSilent(workingBuffer, fpp))
    //        qInfo("SamplePlayHandle::play wb is silent");
}

bool SamplePlayHandle::isFinished() const
{
    return m_finished
           || (framesDone() >= frames()
               && m_doneMayReturnTrue == true);  // total
}

bool SamplePlayHandle::isFromTrack(const Track* _track) const
{
    return m_track == _track || m_bbTrack == _track;
}

bool SamplePlayHandle::isFromInstrument(const Instrument* _instrument) const
{
    return false;
}

/*
f_cnt_t SamplePlayHandle::totalFrames() const
{
        return ( m_sampleBuffer->endFrame() - m_sampleBuffer->startFrame() ) *
( Engine::mixer()->processingSampleRate() / Engine::mixer()->baseSampleRate()
);
}
*/

f_cnt_t SamplePlayHandle::frames() const
{
    return m_frames;
}

void SamplePlayHandle::setFrames(const f_cnt_t _frames)
{
    m_frames = _frames;  // qMin(_frames,totalFrames());
}
