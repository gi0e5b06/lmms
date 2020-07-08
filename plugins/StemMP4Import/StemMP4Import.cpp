/*
 * StemMP4Import.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "StemMP4Import.h"

#include "GuiApplication.h"
#include "MainWindow.h"
#include "SampleTrack.h"
#include "Song.h"
#include "TrackContainer.h"
#include "debug.h"
#include "embed.h"

#include <QDir>
#include <QMessageBox>
#include <QProcess>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT stemmp4import_plugin_descriptor = {
            STRINGIFY(PLUGIN_NAME),
            "Stem.mp4 Import",
            QT_TRANSLATE_NOOP("pluginBrowser", "Import 4-track stem files"),
            "gi0e5b06 (on github.com)",
            0x0100,
            Plugin::ImportFilter,
            NULL,
            NULL,
            NULL};
}

StemMP4Import::StemMP4Import(const QString& _file) :
      ImportFilter(_file, &stemmp4import_plugin_descriptor)
{
}

StemMP4Import::~StemMP4Import()
{
}

QString StemMP4Import::pathForStem(const QFile& _file, const int _num) const
{
    QString   extension = ".mp3";
    QFileInfo fi(_file);
    QString   name = fi.baseName();

    name = QString("%1_%2%3")
                   .arg(_num, 2, 10, QChar('0'))
                   .arg(name)
                   .arg(extension);

    return QDir(Engine::getSong()->projectDir() + QDir::separator() + "stems"
                + QDir::separator() + "imported")
            .filePath(name);
}

bool StemMP4Import::tryImport(TrackContainer* _tc)
{
    if(!openFile())
        return false;

    const QFile& f = file();
    QFileInfo    fi(f);

    if(fi.completeSuffix() != "stem.mp4")
        return false;

    QFile p("/usr/bin/ffmpeg");
    if(!p.exists())
    {
        if(gui)
        {
            QMessageBox::information(
                    gui->mainWindow(), tr("FFMPEG missing"),
                    tr("The ffmpeg software is required for this operation. "
                       " Please install it and retry. "
                       "{ /usr/bin/ffmpeg not found }"));
        }
        else
        {
            qInfo("Error: ffmpeg not found");
        }
        return false;
    }

    qInfo("StemMP4Import::tryImport()");
    for(int i = 0; i < 5; i++)
    {
        QString stem = pathForStem(f, i);
        qInfo("Extracting stem mp3 #%d from %s", i,
              qPrintable(fi.fileName()));
        QProcess::execute(p.fileName(),
                          QStringList()
                                  << "-i" << fi.absoluteFilePath() << "-map"
                                  << QString("0:%1").arg(i) << "-y" << stem);

        SampleTrack* st = dynamic_cast<SampleTrack*>(
                Track::create(Track::SampleTrack, _tc));
        if(st == nullptr)
        {
            qCritical("Error: can not create a sample track");
            return false;
        }

        st->setName(fi.fileName() + " P" + i);

        Tile* tco = st->createTCO(MidiTime(0));
        if(tco == nullptr)
        {
            qCritical("Error: can not create a sample tile");
            return false;
        }
        SampleTCO* stco = dynamic_cast<SampleTCO*>(tco);
        if(stco == nullptr)
            return false;

        qInfo("Notice: loading file %s", qPrintable(stem));
        stco->setSampleFile(stem);
    }

    return true;
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model*, void* _data)
    {
        return new StemMP4Import(
                QString::fromUtf8(static_cast<const char*>(_data)));
    }
}
