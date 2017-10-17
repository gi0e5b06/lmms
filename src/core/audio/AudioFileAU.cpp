/*
 * AudioFileAU.cpp - audio-device which encodes au-stream and writes it
 *                   into a AU-file. This is used for song-export.
 *
 * Copyright (c) 2017
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

#include <unistd.h>

#include "AudioFileAU.h"
#include "endian_handling.h"
#include "Mixer.h"


AudioFileDevice * AudioFileAU::getInst( const QString & outputFilename,
					OutputSettings const & outputSettings,
					const ch_cnt_t channels,
					Mixer * mixer,
					bool & successful )
{
	AudioFileAU * r=new AudioFileAU( outputSettings, channels,
					 successful, outputFilename,
					 mixer );
	qWarning("AudioFileAU: hasStreamSupport=%d",r->hasStreamSupport());
	r->initOutputFile();
	r->openOutputFile();

	successful = r->outputFileOpened() && r->startEncoding();
	qWarning("AudioFileAU::getInst success=%d",successful);

	return r;
}




AudioFileAU::AudioFileAU( const OutputSettings & outputSettings,
			  const ch_cnt_t channels, bool & successful,
			  const QString & file,
			  Mixer * mixer ) :
	AudioFileDevice( outputSettings, channels, file, mixer ),
	m_sf( nullptr )
{
}




AudioFileAU::~AudioFileAU()
{
	finishEncoding();
}




bool AudioFileAU::hasStreamSupport() const
{
	return true;
}




bool AudioFileAU::startEncoding()
{
	m_si.samplerate = sampleRate();
	m_si.channels = channels();
	m_si.frames = mixer()->framesPerPeriod();
	m_si.sections = 1;
	m_si.seekable = 0;

	m_si.format = SF_FORMAT_AU;

	switch( getOutputSettings().getBitDepth() )
	{
	case OutputSettings::Depth_32Bit:
		m_si.format |= SF_FORMAT_FLOAT;
		break;
	case OutputSettings::Depth_24Bit:
		m_si.format |= SF_FORMAT_PCM_24;
		break;
	case OutputSettings::Depth_16Bit:
	default:
		m_si.format |= SF_FORMAT_PCM_16;
		break;
	}

	//	if( m_si.format & SF_FORMAT_PCM_24 )
	//	m_si.format = SF_FORMAT_AU | SF_ENDIAN_LITTLE | SF_FORMAT_PCM_16;

	if( m_useStdout )
	{
		qWarning("AudioFileAU: output to STDOUT");
		m_sf = sf_open_fd( STDOUT_FILENO, SFM_WRITE, &m_si, false );
		qWarning("AudioFileAU: m_sf=%p",m_sf);
 	}
	else
	{
		m_sf = sf_open(
#ifdef LMMS_BUILD_WIN32
			       outputFile().toLocal8Bit().constData(),
#else
			       outputFile().toUtf8().constData(),
#endif
			       SFM_WRITE, &m_si );
	}

	// Prevent fold overs when encountering clipped data
	sf_command(m_sf, SFC_SET_CLIPPING, NULL, SF_TRUE);

	sf_set_string ( m_sf, SF_STR_SOFTWARE, "LMMS" );

	return true;
}




void AudioFileAU::writeBuffer( const surroundSampleFrame * _ab,
			       const fpp_t _frames,
			       const float _master_gain )
{
	OutputSettings::BitDepth bitDepth = getOutputSettings().getBitDepth();

	if( bitDepth == OutputSettings::Depth_32Bit || bitDepth == OutputSettings::Depth_24Bit )
	{
		float *  buf = new float[_frames*channels()];
		for( fpp_t frame = 0; frame < _frames; ++frame )
		{
			for( ch_cnt_t chnl = 0; chnl < channels(); ++chnl )
			{
				buf[frame*channels()+chnl] = _ab[frame][chnl] *
								_master_gain;
			}
		}
		sf_writef_float( m_sf, buf, _frames );
		delete[] buf;
	}
	else
	{
		int_sample_t * buf = new int_sample_t[_frames * channels()];
		convertToS16( _ab, _frames, _master_gain, buf,
							!isLittleEndian() );

		sf_writef_short( m_sf, buf, _frames );
		delete[] buf;
	}
}




void AudioFileAU::finishEncoding()
{
	if( m_sf )
	{
		sf_close( m_sf );
	}
}

