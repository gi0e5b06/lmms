/*
 * embed.h - misc. stuff for using embedded data (resources linked into
 * binary)
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

#ifndef EMBED_H
#define EMBED_H

#include "export.h"
#include "lmms_basics.h"  // REQUIRED

#include <QIcon>
#include <QPixmap>
#include <QString>

namespace embed
{

const QIcon EXPORT   getIcon(const QString& _name, int _w = -1, int _h = -1);
const QPixmap EXPORT getPixmap(const QString& _name, int _w = -1, int _h = -1);
const QString EXPORT getText(const char* _name);
const QHash<QString, QString> EXPORT getProperties(const QString& _fileName);

// obsolete
const QPixmap EXPORT getIconPixmap(const QString& _name, int _w = -1, int _h = -1);

}  // namespace embed

#ifdef PLUGIN_NAME
namespace PLUGIN_NAME
{

inline const QIcon getIcon(const QString& _name, int _w = -1, int _h = -1)
{
    return embed::getIcon(QString("%1/%2").arg(STRINGIFY(PLUGIN_NAME), _name),
                          _w, _h);
}

inline const QPixmap getPixmap(const QString& _name, int _w = -1, int _h = -1)
{
    return embed::getPixmap(
            QString("%1/%2").arg(STRINGIFY(PLUGIN_NAME), _name), _w, _h);
}

// obsolete
inline const QPixmap getIconPixmap(const QString& _name, int _w = -1, int _h = -1)
{
    return embed::getIconPixmap(
            QString("%1/%2").arg(STRINGIFY(PLUGIN_NAME), _name), _w, _h);
}
// QString getText( const char * _name );

}  // namespace PLUGIN_NAME
#endif

class PixmapLoader
{
  public:
    /*
    PixmapLoader(const PixmapLoader* _ref) :
          m_name(_ref != nullptr ? _ref->m_name : QString::null)
    {
    }
    */

    PixmapLoader(const QString& _name = QString::null,
                 int            _w    = -1,
                 int            _h    = -1) :
          m_name(_name),
          m_w(_w), m_h(_h)
    {
    }

    virtual ~PixmapLoader()
    {
        qInfo("deleting PixmapLoader %s", qPrintable(m_name));
    }

    virtual const QPixmap pixmap() const
    {
        if(!m_name.isEmpty())
            return embed::getPixmap(m_name, m_w, m_h);
        // m_name.toLatin1().constData());

        return QPixmap(qMax(0, m_w), qMax(0, m_h));
    }

    operator QPixmap()
    {
        return pixmap();
    }

    /*
    virtual const QString& name() const
    {
        return m_name;
    }
    */

    virtual int width() const
    {
        return m_w;
    }

    virtual int height() const
    {
        return m_h;
    }

  protected:
    QString m_name;
    int     m_w;
    int     m_h;
};

#ifdef PLUGIN_NAME
class PluginPixmapLoader : public PixmapLoader
{
  public:
    PluginPixmapLoader(const QString& _name = QString::null,
                       int            _w    = -1,
                       int            _h    = -1) :
          PixmapLoader(_name, _w, _h)
    {
    }

    virtual ~PluginPixmapLoader()
    {
        qInfo("deleting PluginPixmapLoader %s", qPrintable(m_name));
    }

    const QPixmap pixmap() const override
    {
        if(!m_name.isEmpty())
            return PLUGIN_NAME::getPixmap(m_name);
        // m_name.toLatin1().constData()));

        return QPixmap(qMax(0, m_w), qMax(0, m_h));
    }

    /*
    operator QPixmap()
    {
        return pixmap();
    }
    */

    /*
    const QString& name() const override
    {
        return QString(STRINGIFY(PLUGIN_NAME)) + "::" + m_name;
    }
    */
};
#endif

#endif
