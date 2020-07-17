/*
 * StringPairDrag.cpp - class StringPairDrag which provides general support
 *                        for drag'n'drop of string-pairs and which is the
 * base for all drag'n'drop-actions within LMMS
 *
 * Copyright (c) 2005-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "StringPairDrag.h"

#include "GuiApplication.h"
#include "MainWindow.h"

#include <QDragEnterEvent>
#include <QMimeData>

StringPairDrag::StringPairDrag(const QString& _key,
                               const QString& _value,
                               const QPixmap& _icon,
                               QWidget*       _w) :
      QDrag(_w)
{
    if(_icon.isNull() && _w)
    {
        setPixmap(grabWidget(_w).scaled(64, 64, Qt::KeepAspectRatio,
                                        Qt::SmoothTransformation));
    }
    else
    {
        setPixmap(_icon);
    }

    QString    txt = _key + ":" + _value;
    QMimeData* m   = new QMimeData();
    m->setData(mimeType(), txt.toUtf8());
    setMimeData(m);
    start(Qt::IgnoreAction);
}

StringPairDrag::~StringPairDrag()
{
    // during a drag, we might have lost key-press-events, so reset
    // modifiers of main-win
    if(gui->mainWindow())
    {
        gui->mainWindow()->clearKeyModifiers();
    }
}

const char* StringPairDrag::mimeType()
{
    return "application/x-lmms-stringpair";
}

// MimeData

bool StringPairDrag::hasStringPair(const QMimeData* _md)
{
    return _md->hasFormat(mimeType());
}

QString StringPairDrag::decodeKey(const QMimeData* _md)
{
    return QString::fromUtf8(_md->data(mimeType())).section(':', 0, 0);
}

QString StringPairDrag::decodeValue(const QMimeData* _md)
{
    return QString::fromUtf8(_md->data(mimeType())).section(':', 1, -1);
}

bool StringPairDrag::shouldProcess(const QMimeData*   _md,
                                   const QStringList& _keys)
{
    return hasStringPair(_md) && _keys.contains(decodeKey(_md));
}

bool StringPairDrag::shouldProcess(const QMimeData* _md, const QString& _keys)
{
    return shouldProcess(_md, _keys.split(','));
}

// Drop

bool StringPairDrag::hasStringPair(const QDropEvent* _de)
{
    return hasStringPair(_de->mimeData());
}

QString StringPairDrag::decodeKey(const QDropEvent* _de)
{
    return decodeKey(_de->mimeData());
}

QString StringPairDrag::decodeValue(const QDropEvent* _de)
{
    return decodeValue(_de->mimeData());
}

bool StringPairDrag::shouldProcess(const QDropEvent*  _de,
                                   const QStringList& _keys)
{
    return shouldProcess(_de->mimeData(), _keys);
}

bool StringPairDrag::shouldProcess(const QDropEvent* _de,
                                   const QString&    _keys)
{
    return shouldProcess(_de->mimeData(), _keys);
}

bool StringPairDrag::processDragEnterEvent(QDragEnterEvent* _dee,
                                           const QString&   _keys)
{
    if(hasStringPair(_dee) && shouldProcess(_dee, _keys))
    {
        _dee->acceptProposedAction();
        return true;
    }

    _dee->ignore();
    return false;
}

StringPair StringPairDrag::convertExternal(const QMimeData* _md)
{
    if(hasStringPair(_md))
        return StringPair(decodeKey(_md), decodeValue(_md));

    if(_md->hasUrls())
    {
        const QList<QUrl> urls = _md->urls();

        QStringList projectfile      = QString("mmp|mpt|mmpz").split('|');
        QStringList presetfile       = QString("xpf|xml").split('|');
        QStringList pluginpresetfile = QString("xiz").split('|');
        QStringList samplefile
                = QString("wav|ogg|ds|flac|mp3|spx|voc|aif|aiff|au")
                          .split('|');
        // QStringList instrument = QString("").split('|');
        QStringList importedproject = QString("mid").split('|');
        QStringList patchfile       = QString("pat").split('|');
        QStringList soundfontfile   = QString("sf2|sfz").split('|');
        QStringList vstpluginfile   = QString("dll|so|vst").split('|');

        // if(!pluginFactory->pluginSupportingExtension(e).isNull())

        for(const QUrl& u: urls)
            if(u.isValid() && u.isLocalFile())
            {
                const QFileInfo f(u.toLocalFile());
                const QString   p = f.canonicalFilePath();
                const QString   e = f.suffix().toLower();
                qInfo("StringPairDrag::convertExternal f='%s' e='%s'",
                      qPrintable(p), qPrintable(e));

                if(projectfile.contains(e))
                    return StringPair("projectfile", p);
                if(presetfile.contains(e))
                    return StringPair("presetfile", p);
                if(pluginpresetfile.contains(e))
                    return StringPair("pluginpresetfile", p);
                if(samplefile.contains(e))
                    return StringPair("samplefile", p);
                if(importedproject.contains(e))
                    return StringPair("importedproject", p);
                if(patchfile.contains(e))
                    return StringPair("patchfile", p);
                if(soundfontfile.contains(e))
                    return StringPair("soundfontfile", p);
                if(vstpluginfile.contains(e))
                    return StringPair("vstpluginfile", p);
            }
    }

    return StringPair("", "");
}
