/*
 * Clipboard.cpp - the clipboard for patterns, notes etc.
 *
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "Clipboard.h"

#include "DataFile.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTextStream>

#define CSI_ d659c5b8_eba9_4c5b_a5c6_ec77836bdc7c

class CSI_
{
  protected:
    static const QString MIMETYPE_OBJECT;
    // static const QString MIMETYPE_OBJECTS;

    static const QString type(SerializingObject* _obj);
    static const QString type(const QString& _nodeName);

    static void set(SerializingObject* _obj, QClipboard::Mode _mode);
    static bool get(SerializingObject* _obj, QClipboard::Mode _mode);
    static bool has(const QString& _nodeName, QClipboard::Mode _mode);

    static QString     txt(QClipboard::Mode _mode);
    static QDomElement dom(const QString& _nodeName, QClipboard::Mode _mode);

    friend class Clipboard;
    friend class Selection;
};

const QString CSI_::MIMETYPE_OBJECT("text/x-lmms-object");
// const QString CSI_::MIMETYPE_OBJECTS("text/x-lmms-objects");

const QString CSI_::type(SerializingObject* _obj)
{
    QString r = MIMETYPE_OBJECT;
    r.append(";value=\"" + _obj->nodeName() + "\"");
    return r;
}

const QString CSI_::type(const QString& _nodeName)
{
    QString r = MIMETYPE_OBJECT;
    r.append(";value=\"" + _nodeName + "\"");
    return r;
}

void CSI_::set(SerializingObject* _obj, QClipboard::Mode _mode)
{
    if(_obj == NULL)
    {
        qWarning("Warning: tried to copy null object to clipboard");
        return;
    }

    DataFile doc(DataFile::ClipboardData);
    _obj->saveState(doc, doc.content());
    QMimeData* clip = new QMimeData;
    QByteArray data = doc.toString().toUtf8();

    // clip->setData(MIMETYPE_OBJECT, data);
    clip->setData(type(_obj), data);
    clip->setData("text/plain", data);
    clip->setData("text/xml", data);

    QApplication::clipboard()->setMimeData(clip, _mode);
}

bool CSI_::has(const QString& _nodeName, QClipboard::Mode _mode)
{
    const QMimeData* clip = QApplication::clipboard()->mimeData(_mode);

    const QString type = CSI_::type(_nodeName);

    if(clip->hasFormat(type))
        return true;

    // if(xml)...

    return false;
}

bool CSI_::get(SerializingObject* _obj, QClipboard::Mode _mode)
{
    if(_obj == NULL)
    {
        qWarning("Warning: tried to get null object to clipboard");
        return false;
    }

    const QMimeData* clip = QApplication::clipboard()->mimeData(_mode);

    const QString type = CSI_::type(_obj);
    QByteArray    data = clip->data(type);

    if(clip->hasFormat(type))
    {
        // OK
    }
    else
    {
        qWarning("Clipboard: invalid data (not %s)",
                 qPrintable(_obj->nodeName()));
        return false;
    }

    DataFile    doc(QString(data).toUtf8());
    QDomElement e = doc.content();
    if(e.nodeName() != "clipboarddata")
    {
        qWarning("Warning: strange top node: %s", qPrintable(e.nodeName()));
    }
    else
    {
        e = e.firstChild().toElement();
    }

    if(e.nodeName() != _obj->nodeName())
    {
        qWarning("Warning: strange content node: %s (not %s)",
                 qPrintable(e.nodeName()), qPrintable(_obj->nodeName()));
        e = e.firstChildElement(_obj->nodeName());
    }

    if(e.isNull())
    {
        qWarning("Warning: no valid xml data...");
        QString     txt;
        QTextStream xml(&txt);
        doc.write(xml);
        qWarning("XML: %s", qPrintable(txt));
        return false;
    }

    _obj->restoreState(e);
    return true;
}

QString CSI_::txt(QClipboard::Mode _mode)
{
    const QMimeData* clip = QApplication::clipboard()->mimeData(_mode);

    const QString type = "text/plain";
    QByteArray    data = clip->data(type);

    if(clip->hasFormat(type))
    {
        return QString(data).toUtf8();
    }
    else
    {
        qWarning("Clipboard: invalid data (not text/plain)");
        return "";
    }
}

QDomElement CSI_::dom(const QString& _nodeName, QClipboard::Mode _mode)
{
    const QMimeData* clip = QApplication::clipboard()->mimeData(_mode);

    const QString type = CSI_::type(_nodeName);
    QByteArray    data = clip->data(type);

    if(clip->hasFormat(type))
    {
        // OK
    }
    else
    {
        qWarning("Clipboard: invalid data (not text/xml)");
        return QDomElement();
    }

    DataFile    doc(QString(data).toUtf8());
    QDomElement e = doc.content();
    if(e.nodeName() != "clipboarddata")
    {
        qWarning("Warning: strange top node: %s", qPrintable(e.nodeName()));
    }
    else
    {
        e = e.firstChild().toElement();
    }

    if(e.nodeName() != _nodeName)
    {
        qWarning("Warning: strange content node: %s (not %s)",
                 qPrintable(e.nodeName()), qPrintable(_nodeName));
        e = e.firstChildElement(_nodeName);
    }

    if(e.isNull())
    {
        qWarning("Warning: no valid xml data...");
        QString     txt;
        QTextStream xml(&txt);
        doc.write(xml);
        qWarning("XML: %s", qPrintable(txt));
        return e;
    }

    return e;
}

// Clipboard //

void Clipboard::copy(SerializingObject* _obj)
{
    CSI_::set(_obj, QClipboard::Clipboard);
}

bool Clipboard::has(const QString& _nodeName)
{
    return CSI_::has(_nodeName, QClipboard::Clipboard);
}

bool Clipboard::paste(SerializingObject* _obj)
{
    return CSI_::get(_obj, QClipboard::Clipboard);
}

// Selection //

void Selection::select(SerializingObject* _obj)
{
    CSI_::set(_obj, QClipboard::Selection);
}

bool Selection::has(const QString& _nodeName)
{
    return CSI_::has(_nodeName, QClipboard::Selection);
}

bool Selection::inject(SerializingObject* _obj)
{
    return CSI_::get(_obj, QClipboard::Selection);
}

QString Selection::txt()
{
    return CSI_::txt(QClipboard::Selection);
}

QDomElement Selection::dom(const QString& _nodeName)
{
    return CSI_::dom(_nodeName, QClipboard::Selection);
}
