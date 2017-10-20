/*
 * AudioFileDevice.cpp - base-class for audio-device-classes which write
 *                       their output into a file
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

#include <QMessageBox>
#include <QTemporaryFile>

#include "AudioFileDevice.h"
#include "ExportProjectDialog.h"
#include "GuiApplication.h"


AudioFileDevice::AudioFileDevice( OutputSettings const & outputSettings,
					const ch_cnt_t _channels,
					const QString & _file,
					Mixer*  _mixer ) :
	AudioDevice( _channels, _mixer ),
	m_outputFileName( _file ),
	m_outputFile( NULL ),
	m_useTmpFile( false ),
	m_useStdout ( false ),
	m_outputSettings(outputSettings)
{
	setSampleRate( outputSettings.getSampleRate() );
}




AudioFileDevice::~AudioFileDevice()
{
	closeOutputFile();
	if(m_outputFile) delete m_outputFile;
}




bool AudioFileDevice::hasStreamSupport() const
{
	return false;
}




void AudioFileDevice::initOutputFile()
{
	if(m_outputFileName == "-")
	{
		if(hasStreamSupport())
		{
			m_outputFile=new QFile();
			m_useTmpFile=false;
			m_useStdout=true;
			qWarning("Notice: Streaming to stdout");
		}
		else
	        {
			m_outputFile=new QTemporaryFile("lmms-XXXXXX.tmp");
			m_useTmpFile=true;
			m_useStdout=false;
			qWarning("Notice: Using a temporary file");
		}
	}
	else
	{
		m_outputFile=new QFile(m_outputFileName);
		m_useTmpFile=false;
		m_useStdout=false;
	}

	qWarning("Notice: output uses tmpf=%d, stdout=%d, is %s",
		 m_useTmpFile,
		 m_useStdout,
		 qPrintable(m_outputFileName));
}




void AudioFileDevice::openOutputFile()
{
	bool opened=false;

	if(m_useStdout)
		opened=true;
	else
		opened=m_outputFile->open( QFile::WriteOnly | QFile::Truncate );

	if(!opened)
        {
		QString title, message;
		title = ExportProjectDialog::tr( "Could not open file" );
		message = ExportProjectDialog::tr( "Could not open file %1 "
						   "for writing.\nPlease make "
						   "sure you have write "
						   "permission to the file and "
						   "the directory containing the "
						   "file and try again!"
						   ).arg( m_outputFile->fileName() );

		if( gui )
		{
			QMessageBox::critical( NULL, title, message,
					       QMessageBox::Ok,
					       QMessageBox::NoButton );
		}
		else
		{
			qCritical( "%s", message.toUtf8().constData() );
			exit( EXIT_FAILURE );
		}
	}
}




bool AudioFileDevice::outputFileOpened() const
{
	return (m_useStdout || (m_outputFile && m_outputFile->isOpen()));
}




void AudioFileDevice::closeOutputFile()
{
	if(!m_useStdout) m_outputFile->close();

	if(m_useTmpFile)
	{
		qWarning("\nAudioFileDevice::closeOutputFile read tmp data");
		m_outputFile->open(QFile::ReadOnly);
		QFile out;
		qWarning("AudioFileDevice::closeOutputFile send tmp data to stdout");
		out.open(stdout, QIODevice::WriteOnly); //::stdout

		QByteArray ba;
		while(!(ba=m_outputFile->readLine(4096)).isEmpty())
			out.write(ba);

		out.flush();//close();
		m_outputFile->close();
	}
}




int AudioFileDevice::writeData( const void* data, int len )
{
	if( m_useStdout )
	{
		QFile out;
		qWarning("AudioFileDevice::writeData to stdout");
		out.open(stdout, QIODevice::WriteOnly); //::stdout
		out.write( (const char *) data, len );
		out.flush();
	}
	else
	if( outputFileOpened() )
	{
		return m_outputFile->write( (const char *) data, len );
	}

	return -1;
}
