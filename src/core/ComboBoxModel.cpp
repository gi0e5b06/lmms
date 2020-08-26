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

#include "Backtrace.h"
#include "embed.h"

void ComboBoxModel::assertNotEmpty() const
{
    if(m_items == nullptr)
    {
        BACKTRACE
        qCritical("ComboBoxModel: m_items null (%s)",
                  qPrintable(objectName()));
    }
    else if(m_items->size() == 0)
    {
        BACKTRACE
        qWarning("ComboBoxModel: m_items empty (%s)",
                 qPrintable(objectName()));
    }
}

void ComboBoxModel::assertValidIndex(int _i) const
{
    if(m_items == nullptr)
    {
        BACKTRACE
        qCritical("ComboBoxModel: m_items null (%s)",
                  qPrintable(objectName()));
    }
    else if(m_items->size() == 0)
    {
        BACKTRACE
        qWarning("ComboBoxModel: m_items empty (%s)",
                 qPrintable(objectName()));
    }
    else if(_i < 0 || _i >= m_items->size())
    {
        BACKTRACE
        qCritical("ComboBoxModel: m_items invalid index %d (%d, %s)", _i,
                  m_items->size(), qPrintable(objectName()));
    }
}

void ComboBoxModel::addItem(const QString&      _text,
                            const PixmapLoader* _icon,
                            const QVariant&     _data)
{
    addItem(max() + 1, _text, _icon, _data);
}

void ComboBoxModel::addItem(const int           _value,
                            const QString&      _text,
                            const PixmapLoader* _icon,
                            const QVariant&     _data)
{
    if(m_itemsOwned)
    {
        m_items->append(Item(_value, _text, _icon, _data));
        setRange(min(), max());
    }
    else
    {
        qWarning("ComboBoxModel::addItem %s is immutable",
                 qPrintable(objectName()));
    }
}

void ComboBoxModel::addItem(const Item& _item)
{
    addItem(_item.m_value, _item.m_text, _item.m_icon, _item.m_data);
}

void ComboBoxModel::addItems(const Items& _items)
{
    for(const Item& item: _items)
        addItem(item);
}

void ComboBoxModel::cleanup()
{
    if(m_itemsOwned)
    {
        for(const Item& item: *m_items)
            delete item.m_icon;
        m_items->clear();
        delete m_items;
    }
    m_items = nullptr;
}

void ComboBoxModel::setItems(const Items* _items)
{
    cleanup();

    if(_items != nullptr)
    {
        m_items      = const_cast<Items*>(_items);
        m_itemsOwned = false;
    }
    else
    {
        m_items      = new Items();
        m_itemsOwned = true;
    }

    setRange(min(), max());
    setValue(value());
}

void ComboBoxModel::clear()
{
    setItems(nullptr);
}

int ComboBoxModel::findText(const QString& _what,
                            bool           _required,
                            int            _notfound) const
{
    for(const Item& item: *m_items)
        if(item.m_text == _what)
            return item.m_value;

    if(_required)
    {
        BACKTRACE
        qWarning("ComboBoxModel::findText missing required text '%s'",
                 qPrintable(_what));
    }
    return _notfound;
}

int ComboBoxModel::findData(const QVariant& _what,
                            bool            _required,
                            int             _notfound) const
{
    for(const Item& item: *m_items)
        if(item.m_data == _what)
            return item.m_value;
    if(_required)
    {
        BACKTRACE
        qWarning("ComboBoxModel::findData missing required data '%s'",
                 qPrintable(_what.toString()));
    }
    return _notfound;
}

int ComboBoxModel::nextValue() const
{
    if(m_items->isEmpty())
        return 0;
    int i = indexOf(value());
    return m_items->at(qMin(i + 1, size() - 1)).m_value;
}

int ComboBoxModel::previousValue() const
{
    if(m_items->isEmpty())
        return 0;
    int i = indexOf(value());
    return m_items->at(qMax(i - 1, 0)).m_value;
}

int ComboBoxModel::min() const
{
    int l = m_items->size();
    if(l == 0)
        return 0;
    int r = m_items->at(0).m_value;
    for(int i = 1; i < l; i++)
        if(r > m_items->at(i).m_value)
            r = m_items->at(i).m_value;
    return r;
}

int ComboBoxModel::max() const
{
    int l = m_items->size();
    if(l == 0)
        return -1;
    int r = m_items->at(0).m_value;
    for(int i = 1; i < l; i++)
        if(r < m_items->at(i).m_value)
            r = m_items->at(i).m_value;
    return r;
}

int ComboBoxModel::currentIndex() const
{
    int i = indexOf(value());
    if(i >= 0)
        return i;
    return 0;
}

int ComboBoxModel::indexOf(int _value) const
{
    int i = 0;
    for(const Item& item: *m_items)
    {
        if(item.m_value == _value)
            return i;
        i++;
    }
    return -1;
}
