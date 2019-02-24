/*
 * Format12Export.cpp -
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

#include "Format12Export.h"

#include "BBTrack.h"
#include "BBTrackContainer.h"
#include "Engine.h"
#include "InstrumentTrack.h"
#include "Song.h"
#include "TrackContainer.h"
#include "lmms_math.h"

#include <QApplication>
#include <QDir>
#include <QDomDocument>
#include <QMessageBox>
#include <QProgressDialog>

extern "C"
{
    Plugin::Descriptor PLUGIN_EXPORT format12export_plugin_descriptor = {
            STRINGIFY(PLUGIN_NAME),
            "Format12 Export",
            QT_TRANSLATE_NOOP("pluginBrowser", "Export projects to LMMS 1.2"),
            "gi0e5b06 (on github.com)",
            0x0100,
            Plugin::ExportFilter,
            NULL,
            NULL,
            NULL};
}

Format12Export::Format12Export() :
      ExportFilter(&format12export_plugin_descriptor)
{
}

Format12Export::~Format12Export()
{
}

bool Format12Export::proceed(const QString& _fileName)
{
    qInfo("Format12Export::proceed to %s", qPrintable(_fileName));

    Song*    song = Engine::getSong();
    DataFile dataFile(DataFile::SongProject);
    song->buildProjectDataFile(dataFile);

    // dataFile is a QDomDocument.
    // Make changes, remove unrecognized nodes, modify
    // elements and attributes, etc
    QDomElement root = dataFile.documentElement();
    root.setAttribute("creatorversion", "1.2.0");

    QStringList tags;
    tags << "notehumanizing"
         << "noteduplicatesremoving"
         << "notefiltering"
         << "notekeying"
         << "noteoutting"
         << "glissando"
         << "notesustaining";
    for(QString tag: tags)
    {
        QDomNodeList nodes;
        nodes = root.elementsByTagName(tag);
        qInfo("Format12: found %d tags '%s'", nodes.length(),
              qPrintable(tag));
        for(int i = nodes.length() - 1; i >= 0; --i)
        {
            QDomNode node   = nodes.at(i);  //.clear();
            QDomNode parent = node.parentNode();
            if(!parent.isNull())
                parent.removeChild(node);
        }
    }

    dataFile.normalize();
    return dataFile.writeFile(_fileName);
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model*, void* _data)
    {
        return new Format12Export();
    }
}
