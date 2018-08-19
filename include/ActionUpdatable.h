/*
 * ActionUpdatable.h -
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
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

#ifndef ACTION_UPDATABLE_H_
#define ACTION_UPDATABLE_H_

#include <QHash>
#include <QString>

class ActionUpdatable
{
  public:
    virtual void updateActions(const bool            _active,
                               QHash<QString, bool>& _table) const;  // = 0;
    virtual void actionTriggered(QString _name);                     // = 0;

    virtual void requireActionUpdate();
};

#endif
