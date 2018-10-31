/*
 * VideoWaveExport.h - support for exporting a simple video line
 *                     .mpg (M-JPEG, BPM images per 10 second)
 *
 * Copyright (c) 2018 gi0e5b06
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

#include "VideoWaveExport.h"

#include "lmms_math.h"
#include "Engine.h"
#include "Song.h"
#include "SongEditor.h"
#include "GuiApplication.h"
//#include "TimeLineWidget.h"
//#include "TrackContainer.h"
//#include "BBTrackContainer.h"
//#include "AutomationTrack.h"
//#include "BBTrack.h"
#include "MainWindow.h"
//#include "InstrumentTrack.h"
//#include "SampleTrack.h"
#include "gui_templates.h"

#include <QDomDocument>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QApplication>
#include <QImage>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QProgressDialog>
#include <QRegExp>

/*
ffmpeg -i yoursong.mp3 -filter_complex "[0:a]showwaves=s=1920x100:mode=p2p,format=yuv420p[v]" -map "[v]" -map 0:a -c:v libx264 -c:a copy output.mkv(edited)
*/

extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT videowaveexport_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"Video Line Export",
	QT_TRANSLATE_NOOP( "pluginBrowser",
                           "Filter for exporting M-JPEG Video lines. "
                           "72x40@15fps. 10 minutes max."),
	"gi0e5b06",
	0x0100,
	Plugin::ExportFilter,
	NULL,
	NULL,
	NULL
} ;

}


VideoWaveExport::VideoWaveExport() : ExportFilter( &videowaveexport_plugin_descriptor)
{
}




VideoWaveExport::~VideoWaveExport()
{
}


bool VideoWaveExport::proceed(const QString& _fileName)
{
        Song* song=Engine::getSong();
        const QString& fileName=song->projectFileName();
        if(fileName.isEmpty()) return false;

        const QFileInfo file(fileName);
        if(!file.exists()) return false;

        //song->createProjectTree();
        QString wav_path=song->projectDir()+QDir::separator()
                +"song"+QDir::separator()
                +"rendered"+QDir::separator()
                +file.baseName()
                +".wav";
        qInfo("Notice: wav_path=%s",qPrintable(wav_path));

	if( !QFile(wav_path).exists() )
	{
		QMessageBox::information( gui->mainWindow(),
                                          tr( "Error" ),
                                          tr( "The file of the rendered song was not found: %1" ).
                                          arg(wav_path) );
		return false;
	}

        QFile p("/usr/bin/ffmpeg");
        if(p.exists())
        {
                QProcess::execute(p.fileName(),
                                  QStringList() << "-i" << wav_path << "-filter_complex"
                                  << "[0:a]showwaves=s=426x240:r=14:colors=red|white:mode=p2p,format=yuv420p[v]"
                                  << "-map" << "[v]" << "-map" << "0:a" << "-c:v" << "libx264"
                                  << _fileName);
                //<< "-c:a" << "copy"
                //426x240
        }

        return true;
}


void VideoWaveExport::error()
{
	//qDebug() << "VideoWaveExport error: " << m_error ;
}



extern "C"
{

// necessary for getting instance out of shared lib
Plugin * PLUGIN_EXPORT lmms_plugin_main( Model *, void * _data )
{
	return new VideoWaveExport();
}


}

