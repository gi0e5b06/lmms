/*
 * AudioFileFlac.h - Audio device which encodes a wave stream into a FLAC file.
 *
 * Copyright (c) 2017 Levin Oehlmann <irrenhaus3/at/gmail[dot]com> et al.
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

#ifndef AUDIO_FILE_FLAC_H
#define AUDIO_FILE_FLAC_H

#include "lmmsconfig.h"

#include "AudioFileDevice.h"
#include <sndfile.h>

class AudioFileFlac : public AudioFileDevice
{
 public:
	virtual ~AudioFileFlac();

	static AudioFileDevice* getInst( const QString & outputFileName,
					 const OutputSettings & outputSettings,
					 const ch_cnt_t channels,
					 Mixer* mixer,
					 bool& successful );

 protected:
	AudioFileFlac( const OutputSettings & outputSettings,
		       const ch_cnt_t channels,
		       bool & successful,
		       const QString & file,
		       Mixer * mixer );

 private:
	virtual void writeBuffer( const surroundSampleFrame * _ab,
				  const fpp_t frames );
	bool startEncoding();
	void finishEncoding();

	SF_INFO  m_si;
	SNDFILE* m_sf;
};

#endif //AUDIO_FILE_FLAC_H
