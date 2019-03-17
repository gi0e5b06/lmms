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

    QStringList tagsToRemove;
    tagsToRemove << "notehumanizing"
                 << "noteduplicatesremoving"
                 << "notefiltering"
                 << "notekeying"
                 << "noteoutting"
                 << "glissando"
                 << "notesustaining"
                 << "noteplaying"
                 << "filter1"
                 << "filter2";
    for(const QString& tag: tagsToRemove)
        removeTag(root, tag);

    QHash<QString, QString> tagsToRename;
    tagsToRename.insert("synthgdx", "audiofileprocessor");
    tagsToRename.insert("padsgdx", "audiofileprocessor");
    for(QString tag: tagsToRename.keys())
        renameTag(root, tag, tagsToRename.value(tag));

    QStringList attributesToRemove;
    // attributesToRemove << "eldata filter1_passes";
    for(const QString& s: attributesToRemove)
    {
        QStringList sl = s.trimmed().split(' ');
        if(sl.size() != 2)
        {
            qWarning("Format12: invalid line: '%s'", qPrintable(s));
            continue;
        }
        QString tagName  = sl.takeFirst();
        QString attrName = sl.takeFirst();
        removeAttribute(root, tagName, attrName);
    }

    QHash<QString, QString> attributesToRename;
    // attributesToRename.insert("eldata filter1_cutoff", "ftype");
    for(const QString& s: attributesToRename.keys())
    {
        QStringList sl = s.trimmed().split(' ');
        if(sl.size() != 2)
        {
            qWarning("Format12: invalid line: '%s'", qPrintable(s));
            continue;
        }
        QString tagName  = sl.takeFirst();
        QString attrName = sl.takeFirst();
        QString newName  = attributesToRename.value(s);
        renameAttribute(root, tagName, attrName, newName);
    }

    dataFile.normalize();
    return dataFile.writeFile(_fileName);
}

void Format12Export::removeTag(QDomElement& _root, const QString& _tagName)
{
    QDomNodeList nodes(_root.elementsByTagName(_tagName));
    qInfo("Format12: found %d tags '%s' to remove", nodes.length(),
          qPrintable(_tagName));
    for(int i = nodes.length() - 1; i >= 0; --i)
    {
        QDomNode node   = nodes.at(i);  //.clear();
        QDomNode parent = node.parentNode();
        if(!parent.isNull())
            parent.removeChild(node);
    }
}

void Format12Export::renameTag(QDomElement&   _root,
                               const QString& _oldName,
                               const QString& _newName)
{
    QDomNodeList nodes(_root.elementsByTagName(_oldName));
    qInfo("Format12: found %d tags '%s' to rename to '%s'", nodes.length(),
          qPrintable(_oldName), qPrintable(_newName));
    for(int i = nodes.length() - 1; i >= 0; --i)
    {
        QDomNode node = nodes.at(i);  //.clear();
        if(node.isElement())
        {
            QDomElement e = node.toElement();
            e.setTagName(_newName);
        }
    }
}

void Format12Export::removeAttribute(QDomElement&   _root,
                                     const QString& _tagName,
                                     const QString& _attrName)
{
    int          count = 0;
    QDomNodeList nodes(_root.elementsByTagName(_tagName));
    for(int i = nodes.length() - 1; i >= 0; --i)
    {
        QDomNode node = nodes.at(i);
        if(node.isElement())
        {
            QDomElement e = node.toElement();
            if(e.hasAttribute(_attrName))
            {
                e.removeAttribute(_attrName);
                count++;
            }
        }
    }
    if(count > 0)
        qInfo("Format12: %d attributes '%s' removed in %d elements '%s'",
              count, qPrintable(_attrName), nodes.length(),
              qPrintable(_tagName));
}

void Format12Export::renameAttribute(QDomElement&   _root,
                                     const QString& _tagName,
                                     const QString& _attrName,
                                     const QString& _newName)
{
    int          count = 0;
    QDomNodeList nodes(_root.elementsByTagName(_tagName));
    for(int i = nodes.length() - 1; i >= 0; --i)
    {
        QDomNode node = nodes.at(i);
        if(node.isElement())
        {
            QDomElement e = node.toElement();
            if(e.hasAttribute(_newName))
            {
                qWarning(
                        "Format12: invalid attribute renaming: '%s' already "
                        "exists in '%s'",
                        qPrintable(_newName), qPrintable(_tagName));
                continue;
            }

            if(e.hasAttribute(_attrName))
            {
                QString v(e.attribute(_attrName, ""));
                e.removeAttribute(_attrName);
                e.setAttribute(_newName, v);
                count++;
            }
        }
    }
    if(count > 0)
        qInfo("Format12: %d attributes '%s' renamed to '%s' in %d elements "
              "'%s'",
              count, qPrintable(_attrName), qPrintable(_newName),
              nodes.length(), qPrintable(_tagName));
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model*, void* _data)
    {
        return new Format12Export();
    }
}
