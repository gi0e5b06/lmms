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
      m_instrument(instrument)
{
    setAudioPort(instrumentTrack->audioPort());
}

void InstrumentPlayHandle::play(sampleFrame* _working_buffer)
{
    // if the instrument is midi-based, we can safely render right away
    /*
    if( m_instrument->flags() & Instrument::IsMidiBased )
    {
            m_instrument->play( _working_buffer );
            return;
    }
    */

    // if not, we need to ensure that all our nph's have been processed first
    ConstNotePlayHandleList nphv = NotePlayHandle::nphsOfInstrumentTrack(
            m_instrument->instrumentTrack(), true);

    bool   processed = false;
    real_t ndm;
    bool   nphsLeft;
    do
    {
        nphsLeft = false;
        for(const NotePlayHandle* constNotePlayHandle : nphv)
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

    // ndm = m_instrument->instrumentTrack()->noteBendingModel()->value();
    // ndm*=(1.-0.05*Engine::mixer()->baseSampleRate() /
    // Engine::mixer()->processingSampleRate()); if(abs(ndm)<0.001) ndm=0.;

    if(processed)
    {
        ndm = round(1000. * ndm) / 1000.;
        m_instrument->instrumentTrack()->noteBendingModel()->setAutomatedValue(ndm);
    }

    m_instrument->play(_working_buffer);
}
