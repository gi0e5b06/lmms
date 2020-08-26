/*
 * MidiTime.h - declaration of class MidiTime which provides data type for
 *              position- and length-variables
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net
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

#ifndef MIDI_TIME_H
#define MIDI_TIME_H

#include "export.h"
#include "lmms_basics.h"

#include <QString>

// note: 1 "Tact" = 1 Measure
const tick_t DefaultTicksPerTact = 192;
const step_t DefaultStepsPerTact = 16;
const tick_t DefaultTicksPerBeat = 48;
const tick_t DefaultTicksPerStep = DefaultTicksPerTact / DefaultStepsPerTact;
const beat_t DefaultBeatsPerTact = DefaultTicksPerTact / DefaultTicksPerBeat;

class MeterModel;

class EXPORT TimeSig
{
  public:
    // in a time signature,
    // the numerator represents the number of beats in a measure.
    // the denominator indicates which type of note represents a beat.
    // example: 6/8 means 6 beats in a measure, where each beat has duration
    // equal to one 8th-note.
    TimeSig(int num, int denom);
    TimeSig(const MeterModel& model);
    int numerator() const;
    int denominator() const;

  private:
    int m_num;
    int m_denom;
};

class EXPORT MidiTime
{
  public:
    MidiTime(const tact_t tact, const beat_t beat, const tick_t tick);
    MidiTime(const tact_t tact, const tick_t tick);
    MidiTime(const tick_t ticks = 0);

    MidiTime toAbsoluteTact() const;
    MidiTime toNearestTact() const;
    MidiTime toNextTact() const;
    MidiTime toAbsoluteBeat() const;
    MidiTime toNearestBeat() const;
    MidiTime toNextBeat() const;

    MidiTime& operator+=(const MidiTime& time);
    MidiTime& operator-=(const MidiTime& time);
    MidiTime  operator+(const MidiTime& time) const;
    MidiTime  operator-(const MidiTime& time) const;

    MidiTime& operator+=(const tick_t time);
    MidiTime& operator-=(const tick_t time);
    MidiTime  operator+(const tick_t time) const;
    MidiTime  operator-(const tick_t time) const;

    // return the tact, rounded down and 0-based
    tact_t tact() const;
    tact_t tacts() const;
    // return the tact, rounded up and 0-based
    tact_t nextFullTact() const;

    // Obsolete
    inline tact_t getTact() const
    {
        return tact();
    }

    // inside the tact
    beat_t beat() const;
    // total of beats
    beat_t beats() const;

    // inside the beat
    tick_t tick() const;
    // total of ticks
    tick_t ticks() const;
    void   setTicks(tick_t ticks);

    // Obsolete
    inline tick_t getTicks() const
    {
        return ticks();
    }

    operator tick_t() const;

    QString toString() const
    {
        return QString("%1:%2:%3").arg(tact()).arg(beat()).arg(tick());
    }

    tick_t ticksPerBeat(const TimeSig& sig) const;
    // Remainder ticks after bar is removed
    tick_t getTickWithinBar(const TimeSig& sig) const;
    // Returns the beat position inside the bar, 0-based
    tick_t getBeatWithinBar(const TimeSig& sig) const;
    // Remainder ticks after bar and beat are removed
    tick_t getTickWithinBeat(const TimeSig& sig) const;

    // calculate number of frame that are needed this time
    f_cnt_t frames(const real_t framesPerTick) const;

    real_t getTimeInMilliseconds(bpm_t beatsPerMinute) const;

    static MidiTime fromFrames(const f_cnt_t frames,
                               const real_t  framesPerTick);

    static tick_t ticksPerTact();
    static tick_t ticksPerTact(const TimeSig& sig);
    static void   setTicksPerTact(tick_t tpt);

    static tick_t beatsPerTact();
    static tick_t beatsPerTact(const TimeSig& sig);
    static void   setBeatsPerTact(beat_t bpt);

    // static real_t ticksToMilliseconds(tick_t ticks, bpm_t beatsPerMinute);
    static real_t ticksToMilliseconds(real_t ticks, bpm_t beatsPerMinute);
    static real_t millisecondsToTicks(real_t ms, bpm_t beatsPerMinute);

  private:
    tick_t m_ticks;

    static tick_t s_ticksPerTact;
    static beat_t s_beatsPerTact;
};

#endif
