/*
 * Clipboard.h - the clipboard for patterns, notes etc.
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

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "SerializingObject.h"

#include <QString>

class Selection
{
 public:
    static void select(SerializingObject* _obj);
    static bool inject(SerializingObject* _obj);
    static bool has(const QString& _nodeName);
};

class Clipboard
{
  public:
    static void copy(SerializingObject* _obj);
    static bool paste(SerializingObject* _obj);
    static bool has(const QString& _nodeName);
};

#endif
