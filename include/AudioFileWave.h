/*
 * AudioFileWave.h - AudioDevice which encodes wave-stream and writes it
 *                   into a WAVE-file. This is used for song-export.
 *
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef AUDIO_FILE_WAVE_H
#define AUDIO_FILE_WAVE_H

#include "lmmsconfig.h"
#include "AudioFileDevice.h"

#include <sndfile.h>


class AudioFileWave : public AudioFileDevice
{
 public:
	virtual ~AudioFileWave();

	static AudioFileDevice * getInst( const QString & outputFileName,
					  const OutputSettings & outputSettings,
					  const ch_cnt_t channels,
					  Mixer * mixer,
					  bool & successful );

 protected:
	AudioFileWave( const OutputSettings & outputSettings,
		       const ch_cnt_t channels,
		       bool & successful,
		       const QString & file,
		       Mixer* mixer );
	virtual void writeBuffer( const surroundSampleFrame * _ab,
				  const fpp_t _frames );

 private:
	bool startEncoding();
	void finishEncoding();

	SF_INFO m_si;
	SNDFILE * m_sf;
} ;

#endif
