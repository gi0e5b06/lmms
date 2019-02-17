/*
 * InstrumentPlayHandle.h - play-handle for driving an instrument
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

#ifndef INSTRUMENT_PLAY_HANDLE_H
#define INSTRUMENT_PLAY_HANDLE_H

#include "BasicFilters.h"
#include "Instrument.h"
//#include "NotePlayHandle.h"
#include "PlayHandle.h"
//#include "export.h"

class EXPORT InstrumentPlayHandle : public PlayHandle
{
    MM_OPERATORS

  public:
            //BasicFilters<>* m_filter;

    InstrumentPlayHandle(Instrument*      instrument,
                         InstrumentTrack* instrumentTrack);
    virtual ~InstrumentPlayHandle();

    virtual void play(sampleFrame* _working_buffer);
    virtual bool isFinished() const;
    virtual bool isFromTrack(const Track* _track) const;

    /*
    virtual f_cnt_t frames() const;
    virtual void    setFrames(const f_cnt_t _frames);
    virtual f_cnt_t totalFramesPlayed() const;

    virtual f_cnt_t framesLeft() const;
    virtual fpp_t   framesLeftForCurrentPeriod() const;
    virtual f_cnt_t framesBeforeRelease() const;
    virtual f_cnt_t releaseFramesDone() const;
    virtual f_cnt_t actualReleaseFramesToDo() const;
    virtual bool    isReleased() const;
    virtual bool    isReleaseStarted() const;
    virtual real_t  volumeLevel(const f_cnt_t frame);
    */

  private:
    Instrument* m_instrument;
};

#endif
