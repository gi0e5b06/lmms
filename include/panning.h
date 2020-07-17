/*
 * panning.h - declaration of some types, concerning the
 *             panning of a note
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

#ifndef PANNING_H
#define PANNING_H

#include "Midi.h"
#include "lmms_basics.h"
#include "lmmsconfig.h"
#include "volume.h"  //REQUIRED
//#include "templates.h"

#include <cmath>

const panning_t PanningRight   = 100;
const panning_t PanningLeft    = -PanningRight;
const panning_t PanningCenter  = 0;
const panning_t DefaultPanning = PanningCenter;

inline StereoGain toStereoGain(const panning_t _p, const volume_t _v)
{
    StereoGain r = {{_v / MaxVolume, _v / MaxVolume}};
    r.gain[_p >= PanningCenter ? 0 : 1] *= 1. - abs(_p / PanningRight);
    return r;
}

inline SurroundGain toSurroundGain(const panning_t _p, const volume_t _v)
{
    SurroundGain r;
    for(ch_cnt_t ch = SURROUND_CHANNELS - 1; ch >= 0; --ch)
        r.gain[ch] = _v / MaxVolume;

    // last channel skipped if centered
    for(ch_cnt_t ch = (SURROUND_CHANNELS / 2) * 2 - 1; ch >= 0; --ch)
    {
        if((_p >= PanningCenter) && (ch % 2 == 0))
            r.gain[ch] *= 1. - abs(_p / PanningRight);
        else if((_p < PanningCenter) && (ch % 2 == 1))
            r.gain[ch] *= 1. - abs(_p / PanningRight);
    }

    return r;
}

inline int panningToMidi(panning_t _p)
{
    return MidiMinPanning
           + (int)((real_t(_p - PanningLeft))
                   / (real_t(PanningRight - PanningLeft))
                   * (real_t(MidiMaxPanning - MidiMinPanning)));
}

/*
class EXPORT PanningModel : public FloatModel
{
    Q_OBJECT

  public:
    PanningModel(Model*         _parent,
                 const QString& _name       = tr("Panning"),
                 const QString& _objectName = "panning") :
          FloatModel(DefaultPanning,
                     PanningLeft,
                     PanningRight,
                     0.1,
                     _parent,
                     _name)
    {
    }

    int midiValue()
    {
        return panningToMidi(value());
    }
};
*/

#endif
