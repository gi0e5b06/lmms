/*
 * StringPairDrag.h - class StringPairDrag which provides general support
 *                      for drag'n'drop of string-pairs
 *
 * Copyright (c) 2005-2007 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef STRING_PAIR_DRAG_H
#define STRING_PAIR_DRAG_H

#include "export.h"
#include "lmms_basics.h"

#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPair>
#include <QPixmap>
#include <QStringList>
#include <QWidget>

class QPixmap;

class StringPair
{
  public:
    StringPair(const QString _k, const QString _v) : m_pair(_k, _v)
    {
    }

    const QString& key()
    {
        return m_pair.first;
    }

    const QString& value()
    {
        return m_pair.second;
    }

  protected:
    QPair<const QString, const QString> m_pair;
};

class EXPORT StringPairDrag : public QDrag
{
  public:
    StringPairDrag(const QString& _key,
                   const QString& _value,
                   const QPixmap& _icon,
                   QWidget*       _w);
    virtual ~StringPairDrag();

    INLINE QPixmap grabWidget(QWidget*     widget,
                              const QRect& rectangle
                              = QRect(QPoint(0, 0), QSize(-1, -1)))
    {
#if(QT_VERSION >= 0x050000)
        return widget->grab(rectangle);
#else
        return QPixmap::grabWidget(widget, rectangle);
#endif
    }

    // return "application/x-lmms-stringpair"
    static const char* mimeType();

    static bool    hasStringPair(const QMimeData* _md);
    static QString decodeKey(const QMimeData* _md);
    static QString decodeValue(const QMimeData* _md);
    static bool shouldProcess(const QMimeData* _md, const QStringList& _keys);
    static bool shouldProcess(const QMimeData* _md, const QString& _keys);

    static bool    hasStringPair(const QDropEvent* _de);
    static QString decodeKey(const QDropEvent* _de);
    static QString decodeValue(const QDropEvent* _de);
    static bool    shouldProcess(const QDropEvent*  _de,
                                 const QStringList& _keys);
    static bool    shouldProcess(const QDropEvent* _de, const QString& _keys);

    static bool processDragEnterEvent(QDragEnterEvent* _dee,
                                      const QString&   _allowedKeys);

    // modify external drops to lmms
    static StringPair convertExternal(const QMimeData* _md);
};

#endif
