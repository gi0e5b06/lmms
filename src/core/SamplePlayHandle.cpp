/*
 * SamplePlayHandle.cpp - implementation of class SamplePlayHandle
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

#include "SamplePlayHandle.h"
#include "AudioPort.h"
#include "BBTrack.h"
#include "Engine.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "SampleTrack.h"



SamplePlayHandle::SamplePlayHandle( const QString& sampleFile ) :
	PlayHandle( TypeSamplePlayHandle ),
	m_sampleBuffer( new SampleBuffer( sampleFile ) ),
	m_doneMayReturnTrue( true ),
	m_currentFrame( 0 ),
	m_ownAudioPort( true ),
	m_defaultVolumeModel( DefaultVolume, MinVolume, MaxVolume, 1 ),
	m_volumeModel( &m_defaultVolumeModel ),
	m_track( NULL ),
	m_bbTrack( NULL )
{
        m_frames=m_sampleBuffer->frames();
	setAudioPort( new AudioPort( "SamplePlayHandle", false ) );
}




SamplePlayHandle::SamplePlayHandle( SampleBuffer* sampleBuffer ) :
	PlayHandle( TypeSamplePlayHandle ),
	m_sampleBuffer( sharedObject::ref( sampleBuffer ) ),
	m_doneMayReturnTrue( true ),
	m_currentFrame( 0 ),
	m_ownAudioPort( true ),
	m_defaultVolumeModel( DefaultVolume, MinVolume, MaxVolume, 1 ),
	m_volumeModel( &m_defaultVolumeModel ),
	m_track( NULL ),
	m_bbTrack( NULL )
{
        m_frames=m_sampleBuffer->frames();
	setAudioPort( new AudioPort( "SamplePlayHandle", false ) );
}




SamplePlayHandle::SamplePlayHandle( SampleTCO* tco ) :
	PlayHandle( TypeSamplePlayHandle ),
	m_sampleBuffer( sharedObject::ref( tco->sampleBuffer() ) ),
	m_doneMayReturnTrue( true ),
	m_currentFrame( 0 ),
	m_ownAudioPort( false ),
	m_defaultVolumeModel( DefaultVolume, MinVolume, MaxVolume, 1 ),
	m_volumeModel( &m_defaultVolumeModel ),
	m_track( tco->getTrack() ),
	m_bbTrack( NULL )
{
        m_frames=m_sampleBuffer->frames();
	setAudioPort( ( (SampleTrack *)tco->getTrack() )->audioPort() );
}




SamplePlayHandle::~SamplePlayHandle()
{
	sharedObject::unref( m_sampleBuffer );
	if( m_ownAudioPort )
	{
		delete audioPort();
	}
}




void SamplePlayHandle::play( sampleFrame * buffer )
{
	//qWarning("SamplePlayHandle::play buffer=%p",buffer);

	const fpp_t fpp = Engine::mixer()->framesPerPeriod();
	//play( 0, _try_parallelizing );
	if( framesDone() >= frames() )//totalFrames() )
	{
		memset( buffer, 0, BYTES_PER_FRAME * fpp );
		return;
	}

	sampleFrame * workingBuffer = buffer;
	f_cnt_t frames = fpp;

	// apply offset for the first period
	if( framesDone() == 0 )
	{
		memset( buffer, 0, BYTES_PER_FRAME * offset() );
		workingBuffer += offset();
		frames -= offset();
	}

	if( !( m_track && m_track->isMuted() ) &&
	    !( m_bbTrack && m_bbTrack->isMuted() ) )
	{
/*		stereoVolumeVector v =
			{ { m_volumeModel->value() / DefaultVolume,
				m_volumeModel->value() / DefaultVolume } };*/
		//qWarning("SamplePlayHandle::play workingBuffer=%p",workingBuffer);
		if( ! m_sampleBuffer->play( workingBuffer, &m_state, frames,
								BaseFreq ) )
		{
			//qWarning("SamplePlayHandle::play not played workingBuffer=%p",workingBuffer);
			memset( workingBuffer, 0, frames * BYTES_PER_FRAME );
		}
	}

	m_currentFrame += frames;
}




bool SamplePlayHandle::isFinished() const
{
	return framesDone() >= frames() && m_doneMayReturnTrue == true; //total
}




bool SamplePlayHandle::isFromTrack( const Track * _track ) const
{
	return m_track == _track || m_bbTrack == _track;
}



/*
f_cnt_t SamplePlayHandle::totalFrames() const
{
	return ( m_sampleBuffer->endFrame() - m_sampleBuffer->startFrame() ) * ( Engine::mixer()->processingSampleRate() / Engine::mixer()->baseSampleRate() );
}
*/

f_cnt_t SamplePlayHandle::frames() const
{
        return m_frames;
}


void SamplePlayHandle::setFrames( const f_cnt_t _frames )
{
        m_frames=_frames;//qMin(_frames,totalFrames());
}



