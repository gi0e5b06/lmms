/*
 * embed.cpp - misc stuff for using embedded resources (linked into binary)
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

#include "embed.h"

#include <QFile>
#include <QImageReader>
#include <QPixmapCache>
#include <QResource>
#include <QTextStream>

namespace embed
{

const QPixmap getIconPixmap(const QString& _name, int _width, int _height)
{
    return getPixmap(_name, _width, _height);
}

const QIcon getIcon(const QString& _name, int _width, int _height)
{
    return QIcon(getPixmap(_name, _width, _height));
}

const QPixmap getPixmap(const QString& pixmapName, int width, int height)
{
    QString cacheName;
    if(width > 0 && height > 0)
        cacheName = QString("%1_%2_%3").arg(pixmapName, width, height);
    else
        cacheName = pixmapName;

    // Return cached pixmap
    QPixmap pixmap;
    if(QPixmapCache::find(cacheName, &pixmap))
        return pixmap;

    QImageReader reader(QString("artwork:%1").arg(pixmapName));

    if(width > 0 && height > 0)
        reader.setScaledSize(QSize(width, height));

    pixmap = QPixmap::fromImageReader(&reader);
    if(pixmap.isNull())
    {
        qWarning("Warning: Fail to load pixmap %s: %s",
                 qPrintable(pixmapName), qPrintable(reader.errorString()));
        return QPixmap(1, 1);
    }

    // Save to cache and return
    QPixmapCache::insert(cacheName, pixmap);
    return pixmap;
}

const QString getText(const char* name)
{
    return QString::fromUtf8(
            (const char*)QResource(QString(":/%1").arg(name)).data());
}

const QHash<QString, QString> getProperties(const QString& _fileName)
{
    QHash<QString, QString> r;

    QFile f(QString("data:%1").arg(_fileName));
    if(f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&f);
        for(QString s: in.readAll().split('\n'))
        {
            int i = s.indexOf('#');
            if(i == 0)
                continue;
            i = s.indexOf('=');
            if(i <= 0)
                continue;
            const QString& k = s.left(i);
            const QString& v = s.mid(i + 1);

            r[k] = v;
            // qInfo("'%s' = '%s'", qPrintable(k), qPrintable(v));
        }
        f.close();
    }
    return r;
}

}  // namespace embed
