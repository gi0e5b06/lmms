/*
 * PlayPos.h -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef PLAYPOS_H
#define PLAYPOS_H

#include "Engine.h"
#include "MidiTime.h"

#include <cmath>

class TimeLineWidget;

class PlayPos : public MidiTime
{
  public:
    PlayPos(const int abs = 0) :
          MidiTime(abs), m_timeLine(nullptr), m_currentFrame(0.)
    {
    }

    inline void setCurrentFrame(const real_t f)  // relative
    {
        m_currentFrame = f;
    }

    inline real_t currentFrame() const  // relative
    {
        return m_currentFrame;
    }

    inline real_t absoluteFrame() const
    {
        return getTicks() * Engine::framesPerTick() + m_currentFrame;
    }

    inline void setAbsoluteFrame(real_t _f)
    {
        setTicks(_f / Engine::framesPerTick());
        setCurrentFrame(fmod(_f, Engine::framesPerTick()));
    }

    TimeLineWidget* m_timeLine;

  private:
    real_t m_currentFrame;
};

#endif
