/*
 * volume.h - declaration of some constants and types, concerning the volume
 *            of a note
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

#ifndef VOLUME_H
#define VOLUME_H

//#include "AutomatableModel.h"
#include "lmms_basics.h"
#include "lmmsconfig.h"
//#include "Midi.h"

const volume_t MinVolume     = 0;
const volume_t MaxVolume     = 200;
const volume_t DefaultVolume = 100;

typedef struct
{
    real_t gain[DEFAULT_CHANNELS];
} StereoGain;

#ifndef LMMS_DISABLE_SURROUND
typedef struct
{
    real_t gain[SURROUND_CHANNELS];
} SurroundGain;
#else
typedef StereoGain SurroundGain;
#endif

inline int volumeToMidi(volume_t _v)
{
    return MidiMinVolume
           + (int)((real_t(_v - MinVolume)) / (real_t(MaxVolume - MinVolume))
                   * (real_t(MidiMaxVolume - MidiMinVolume)));
}

/*
class EXPORT VolumeModel : public FloatModel
{
    Q_OBJECT

  public:
    VolumeModel(Model*         _parent,
                const QString& _displayName = tr("Volume"),
                const QString& _objectName  = "volume") :
          FloatModel(DefaultVolume,
                     MinVolume,
                     MaxVolume,
                     0.1,
                     _parent,
                     _displayName,
                     _objectName)
    {
    }

    int midiValue()
    {
        return volumeToMidi(value());
    }
};
*/

#endif
