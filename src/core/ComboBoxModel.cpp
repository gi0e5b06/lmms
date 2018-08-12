/*
 * ComboBoxModel.cpp - implementation of ComboBoxModel
 *
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "ComboBoxModel.h"

#include "embed.h"

void ComboBoxModel::addItem(const QString&  _text,
                            PixmapLoader*   _icon,
                            const QVariant& _data)
{
    m_texts.push_back(_text);
    m_icons.push_back(_icon);
    m_datas.push_back(_data);
    setRange(0, m_texts.size() - 1);
}

void ComboBoxModel::clear()
{
    //setRange(0, 0);
    for(const PixmapLoader* i : m_icons)
        delete i;
    m_texts.clear();
    m_icons.clear();
    m_datas.clear();

    emit propertiesChanged();
}

int ComboBoxModel::findText(const QString& _what) const
{
    for(QVector<QString>::ConstIterator it = m_texts.begin();
        it != m_texts.end(); ++it)
    {
        if((*it) == _what)
        {
            return it - m_texts.begin();
        }
    }
    return -1;
}
