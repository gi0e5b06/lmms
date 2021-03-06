/*
 * AudioFileDevice.h - base-class for audio-device-classes which write
 *                     their output into a file
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

#ifndef AUDIO_FILE_DEVICE_H
#define AUDIO_FILE_DEVICE_H

#include "AudioDevice.h"

#include <QFile>

#include "OutputSettings.h"


class AudioFileDevice : public AudioDevice
{
public:
	virtual ~AudioFileDevice();

	QString outputFile() const
	{
		return m_outputFile ? m_outputFile->fileName() : "-";
	}

	OutputSettings const & getOutputSettings() const { return m_outputSettings; }

protected:
	AudioFileDevice(OutputSettings const & outputSettings,
			const ch_cnt_t _channels, const QString & _file,
			Mixer* mixer );

	virtual bool hasStreamSupport() const;
	virtual void initOutputFile();
	virtual void openOutputFile();
	virtual bool outputFileOpened() const;
	virtual void closeOutputFile();

	virtual int  writeData( const void* data, int len );

	const QString& m_outputFileName;
	QFile*   m_outputFile;
	bool     m_useTmpFile;
	bool     m_useStdout;
	OutputSettings m_outputSettings;
} ;


typedef AudioFileDevice * ( * AudioFileDeviceInstantiaton )
	( const QString & outputFilename,
	  OutputSettings const & outputSettings,
	  const ch_cnt_t channels,
	  Mixer* mixer,
	  bool & successful );


#endif
