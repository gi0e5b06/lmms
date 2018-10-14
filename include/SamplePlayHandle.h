/*
 * SamplePlayHandle.h - play-handle for playing a sample
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

#ifndef SAMPLE_PLAY_HANDLE_H
#define SAMPLE_PLAY_HANDLE_H

#include "SampleBuffer.h"
#include "AutomatableModel.h"
#include "PlayHandle.h"

class BBTrack;
class SampleTCO;
class Track;
class AudioPort;


class SamplePlayHandle : public PlayHandle
{
public:
	SamplePlayHandle( const QString& sampleFile );
        SamplePlayHandle( SampleBuffer* sampleBuffer, bool ownAudioPort = true );
	SamplePlayHandle( SampleTCO* tco );
	virtual ~SamplePlayHandle();

	virtual inline bool affinityMatters() const
	{
		return true;
	}

	virtual void play( sampleFrame * buffer );
	virtual bool isFinished() const;

	virtual bool isFromTrack( const Track * _track ) const;

	/*! Returns total numbers of frames to play (including release frames = 0) */
	f_cnt_t frames() const;
	/*! Sets the total number of frames to play (including release frames = 0) */
	void setFrames( const f_cnt_t _frames );

	//f_cnt_t totalFrames() const;

	inline f_cnt_t framesDone() const
	{
		return m_currentFrame;
	}

	void setDoneMayReturnTrue( bool _enable )
	{
		m_doneMayReturnTrue = _enable;
	}

	void setBBTrack( BBTrack * _bb_track )
	{
		m_bbTrack = _bb_track;
	}

	void setVolumeModel( FloatModel * _model )
	{
		m_volumeModel = _model;
	}


private:
	SampleBuffer * m_sampleBuffer;
	bool m_doneMayReturnTrue;

	f_cnt_t m_currentFrame;
	f_cnt_t m_frames;       // total frames to play
	SampleBuffer::handleState m_state;

	const bool m_ownAudioPort;

	FloatModel  m_defaultVolumeModel;
	FloatModel* m_volumeModel;
	Track*      m_track;
	BBTrack*    m_bbTrack;
} ;


#endif
