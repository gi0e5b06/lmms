/*
 * InstrumentPlayHandle.cpp - play-handle for driving an instrument
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

#include "InstrumentPlayHandle.h"

#include "Engine.h"
#include "InstrumentTrack.h"
#include "Mixer.h"

#include <cmath>

InstrumentPlayHandle::InstrumentPlayHandle(Instrument*      instrument,
                                           InstrumentTrack* instrumentTrack) :
      PlayHandle(TypeInstrumentPlayHandle),
      m_instrument(instrument), m_track(instrumentTrack)
{
    setAudioPort(instrumentTrack->audioPort());
}

InstrumentPlayHandle::~InstrumentPlayHandle()
{
    qInfo("~InstrumentPlayHandle");
    setAudioPort(nullptr);
    m_instrument = nullptr;
    m_track      = nullptr;
}

void InstrumentPlayHandle::play(sampleFrame* _working_buffer)
{
    if(m_instrument == nullptr)
    {
        qWarning("InstrumentPlayHandle::play m_instrument is null");
        return;
    }

    InstrumentTrack* track = m_instrument->instrumentTrack();
    if(track == nullptr)
    {
        qWarning("InstrumentPlayHandle::play m_instrument->track is null");
        return;
    }
    // if the instrument is midi-based, we can safely render right away
    /*
    if( m_instrument->flags() & Instrument::IsMidiBased )
    {
            m_instrument->play( _working_buffer );
            return;
    }
    */

    // if not, we need to ensure that all our nph's have been processed first
    // ConstNotePlayHandleList nphv = NotePlayHandle::nphsOfInstrumentTrack(
    //        m_instrument->instrumentTrack(), true);
    ConstNotePlayHandleList cnphv = Engine::mixer()->nphsOfTrack(track, true);

    /*
    do
    {
        nphsLeft = false;
        for(const NotePlayHandle* constNotePlayHandle: cnphv)
        {
            NotePlayHandle* notePlayHandle
                    = const_cast<NotePlayHandle*>(constNotePlayHandle);
            if(notePlayHandle->state() != ThreadableJob::Done
               && !notePlayHandle->isFinished())
            {
                processed = true;
                nphsLeft  = true;
                notePlayHandle->process();
                ndm = notePlayHandle->automationDetune()
                      + notePlayHandle->effectDetune();
            }
        }
    } while(nphsLeft);
    */

    bool processed = false;
    bool nphsLeft;
    do
    {
        nphsLeft = false;
        for(const NotePlayHandle* cnph: cnphv)
        {
            if(cnph != nullptr && cnph->state() != ThreadableJob::Done
               && !cnph->isFinished())
            {
                processed = true;
                nphsLeft  = true;

                NotePlayHandle* nph = const_cast<NotePlayHandle*>(cnph);
                nph->process();
            }
        }
    } while(nphsLeft);

    real_t  ndm    = 0.;
    f_cnt_t maxtfp = -1;
    f_cnt_t maxfbr = -1;
    for(const NotePlayHandle* cnph: cnphv)
    {
        if(cnph != nullptr && !cnph->isFinished())
        {
            NotePlayHandle* nph = const_cast<NotePlayHandle*>(cnph);

            const f_cnt_t tfp = nph->totalFramesPlayed();
            const f_cnt_t fbr = tfp - nph->releaseFramesDone()
                                + nph->framesBeforeRelease();
            const f_cnt_t off = nph->noteOffset();
            // const f_cnt_t left = nph->framesLeft();

            /*
            if(tfp <= 256)
                qInfo("IPH: tfp=%d off=%d br=%d leftfcp=%d left=%d", tfp, off,
                      fbr, nph->framesLeftForCurrentPeriod(), left);
            */

            if(/*left > 0 &&*/ (maxtfp == -1 || tfp > maxtfp
                                || (tfp == maxtfp && fbr > maxfbr)))
            {
                track->setEnvTotalFramesPlayed(tfp);
                track->setEnvOffset(off);
                track->setEnvReleaseBegin(fbr);
                track->setEnvLegato(nph->legato());
                track->setEnvLegato(nph->marcato());
                track->setEnvLegato(nph->staccato());
                track->setEnvVolume(nph->getVolume());
                track->setEnvPanning(nph->getPanning());
                ndm    = cnph->automationDetune() + cnph->effectDetune();
                maxfbr = fbr;
                maxtfp = tfp;
            }
        }
    }

    // ndm = m_instrument->instrumentTrack()->noteBendingModel()->value();
    // ndm*=(1.-0.05*Engine::mixer()->baseSampleRate() /
    // Engine::mixer()->processingSampleRate()); if(abs(ndm)<0.001) ndm=0.;

    if(processed)
    {
        ndm = round(1000. * ndm) / 1000.;
        m_instrument->instrumentTrack()
                ->noteBendingModel()
                ->setAutomatedValue(ndm);
    }

    m_instrument->play(_working_buffer);
}

bool InstrumentPlayHandle::isFinished() const
{
    return m_finished || m_track == nullptr || m_instrument == nullptr;
    // || m_instrument->instrumentTrack() == nullptr;
}

bool InstrumentPlayHandle::isFromTrack(const Track* _track) const
{
    qInfo("IPH: isFromTrack finished=%d result=%d", isFinished(),
          (m_track == _track));
    return m_track == _track;

    /*
if(m_instrument == nullptr)
{
    qWarning("InstrumentPlayHandle::isFromTrack m_instrument is null");
    return false;
}

if(_track == nullptr)
{
    qWarning("InstrumentPlayHandle::isFromTrack _track is null");
    return false;
}

qInfo("IPH: isFromTrack finished=%d i=%p (%s) it=%p (%s) tt=%p (%s) --> "
      "%d",
      m_finished, m_instrument, qPrintable(m_instrument->displayName()),
      m_instrument->instrumentTrack(),
      qPrintable(m_instrument->instrumentTrack()->name()), _track,
      qPrintable(_track->name()), m_instrument->isFromTrack(_track));

qInfo("IPH (new) test=%d",_track==m_track);
// qWarning("InstrumentPlayHandle::isFromTrack %p %p", m_instrument,
// _track);
return m_instrument->isFromTrack(_track);
    */
}

bool InstrumentPlayHandle::isFromInstrument(const Instrument* _instrument) const
{
    return m_instrument == _instrument;
}
