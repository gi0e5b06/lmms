/*
 * MidiTime.cpp - Class that encapsulates the position of a note/event in
 *                terms of its bar, beat and tick.
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net
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

#include "MidiTime.h"

#include "MeterModel.h"

tick_t MidiTime::s_ticksPerTact = DefaultTicksPerTact;
beat_t MidiTime::s_beatsPerTact = DefaultBeatsPerTact;

TimeSig::TimeSig(int num, int denom) : m_num(num), m_denom(denom)
{
}

TimeSig::TimeSig(const MeterModel& model) :
      m_num(model.getNumerator()), m_denom(model.getDenominator())
{
}

int TimeSig::numerator() const
{
    return m_num;
}

int TimeSig::denominator() const
{
    return m_denom;
}

MidiTime::MidiTime(const tact_t tact, const beat_t beat, const tick_t tick) :
      MidiTime(tact * s_ticksPerTact + beat * s_ticksPerTact / s_beatsPerTact
               + tick)
{
}

MidiTime::MidiTime(const tact_t tact, const tick_t tick) :
      MidiTime(tact * s_ticksPerTact + tick)
{
}

MidiTime::MidiTime(const tick_t ticks) : m_ticks(ticks)
{
}

MidiTime MidiTime::toAbsoluteTact() const
{
    return tact() * s_ticksPerTact;
}

MidiTime MidiTime::toNearestTact() const
{
    if(m_ticks % s_ticksPerTact >= s_ticksPerTact / 2)
        return (tact() + 1) * s_ticksPerTact;
    else
        return tact() * s_ticksPerTact;
}

MidiTime MidiTime::toNextTact() const
{
    return (tact() + 1) * s_ticksPerTact;
}

MidiTime MidiTime::toNearestBeat() const
{
    return tick_t(round(round(real_t(m_ticks) * real_t(s_beatsPerTact)
                              / real_t(s_ticksPerTact))
                        * real_t(s_ticksPerTact) / real_t(s_beatsPerTact)));
}

MidiTime MidiTime::toAbsoluteBeat() const
{
    return beats() * s_ticksPerTact / s_beatsPerTact;
}

MidiTime MidiTime::toNextBeat() const
{
    return (beats() + 1) * s_ticksPerTact / s_beatsPerTact;
}

MidiTime& MidiTime::operator+=(const MidiTime& time)
{
    m_ticks += time.m_ticks;
    return *this;
}

MidiTime& MidiTime::operator-=(const MidiTime& time)
{
    m_ticks -= time.m_ticks;
    return *this;
}

MidiTime& MidiTime::operator+=(const tick_t _ticks)
{
    m_ticks += _ticks;
    return *this;
}

MidiTime& MidiTime::operator-=(const tick_t _ticks)
{
    m_ticks -= _ticks;
    return *this;
}

MidiTime MidiTime::operator+(const MidiTime& time) const
{
    return MidiTime(m_ticks + time.m_ticks);
}

MidiTime MidiTime::operator-(const MidiTime& time) const
{
    return MidiTime(m_ticks - time.m_ticks);
}

MidiTime MidiTime::operator+(const tick_t _ticks) const
{
    return MidiTime(m_ticks + _ticks);
}

MidiTime MidiTime::operator-(const tick_t _ticks) const
{
    return MidiTime(m_ticks - _ticks);
}

tact_t MidiTime::tacts() const
{
    return m_ticks / s_ticksPerTact;
}

tact_t MidiTime::tact() const
{
    return m_ticks / s_ticksPerTact;
}

beat_t MidiTime::beats() const
{
    return (m_ticks * s_beatsPerTact) / s_ticksPerTact;
}

beat_t MidiTime::beat() const
{
    return ((m_ticks - tacts() * s_ticksPerTact) * s_beatsPerTact)
           / s_ticksPerTact;
}

tick_t MidiTime::ticks() const
{
    return m_ticks;
}

tick_t MidiTime::tick() const
{
    // return m_ticks - tact() * s_ticksPerTact
    //       - beat() * s_ticksPerTact / s_beatsPerTact;
    return (m_ticks % s_ticksPerTact) % (s_ticksPerTact / s_beatsPerTact);
}

tact_t MidiTime::nextFullTact() const
{
    return (m_ticks + (s_ticksPerTact - 1)) / s_ticksPerTact;
}

void MidiTime::setTicks(tick_t ticks)
{
    m_ticks = ticks;
}

MidiTime::operator tick_t() const
{
    return m_ticks;
}

tick_t MidiTime::ticksPerBeat(const TimeSig& sig) const
{
    // (number of ticks per bar) divided by (number of beats per bar)
    return ticksPerTact(sig) / sig.numerator();
}

tick_t MidiTime::getTickWithinBar(const TimeSig& sig) const
{
    return m_ticks % ticksPerTact(sig);
}

tick_t MidiTime::getBeatWithinBar(const TimeSig& sig) const
{
    return getTickWithinBar(sig) / ticksPerBeat(sig);
}

tick_t MidiTime::getTickWithinBeat(const TimeSig& sig) const
{
    return getTickWithinBar(sig) % ticksPerBeat(sig);
}

f_cnt_t MidiTime::frames(const real_t framesPerTick) const
{
    return m_ticks > 0 ? static_cast<f_cnt_t>(real_t(m_ticks) * framesPerTick)
                       : 0;
}

real_t MidiTime::getTimeInMilliseconds(bpm_t beatsPerMinute) const
{
    return ticksToMilliseconds(ticks(), beatsPerMinute);
}

MidiTime MidiTime::fromFrames(const f_cnt_t frames,
                              const real_t  framesPerTick)
{
    return MidiTime(static_cast<tick_t>(real_t(frames) / framesPerTick));
}

tick_t MidiTime::ticksPerTact()
{
    return s_ticksPerTact;
}

tick_t MidiTime::ticksPerTact(const TimeSig& sig)
{
    return DefaultTicksPerTact * sig.numerator() / sig.denominator();
}

void MidiTime::setTicksPerTact(tick_t tpt)
{
    s_ticksPerTact = tpt;
}

beat_t MidiTime::beatsPerTact()
{
    return s_beatsPerTact;
}

tick_t MidiTime::beatsPerTact(const TimeSig& sig)
{
    return sig.numerator();
}

void MidiTime::setBeatsPerTact(beat_t bpt)
{
    s_beatsPerTact = bpt;
}

/*
real_t MidiTime::ticksToMilliseconds(tick_t ticks, bpm_t beatsPerMinute)
{
    return MidiTime::ticksToMilliseconds(static_cast<real_t>(ticks),
                                         beatsPerMinute);
}
*/

real_t MidiTime::ticksToMilliseconds(real_t ticks, bpm_t beatsPerMinute)
{
    // 60 * 1000 / 48 = 1250
    return (ticks * 1250) / beatsPerMinute;
}

real_t MidiTime::millisecondsToTicks(real_t ms, bpm_t beatsPerMinute)
{
    // 60 * 1000 / 48 = 1250
    return (ms * beatsPerMinute) / 1250;
}
