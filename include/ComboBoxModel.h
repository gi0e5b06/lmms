/*
 * ComboBoxModel.h - declaration of class ComboBoxModel
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

#ifndef COMBOBOX_MODEL_H
#define COMBOBOX_MODEL_H

#include "AutomatableModel.h"

#include <QVariant>
#include <QVector>

class PixmapLoader;

class EXPORT ComboBoxModel : public IntModel
{
    Q_OBJECT

  public:
    struct Item
    {
      public:
        Item(const int           _value = 0,
             const QString&      _text  = QString::null,
             const PixmapLoader* _icon  = nullptr,
             const QVariant&     _data  = QVariant()) :
              m_value(_value),
              m_text(_text), m_icon(_icon), m_data(_data)
        {
        }

        Item(const Item& _other) :
              Item(_other.m_value,
                   _other.m_text,
                   _other.m_icon,
                   _other.m_data)
        {
        }

      private:
        const int           m_value;
        const QString       m_text;
        const PixmapLoader* m_icon;
        const QVariant      m_data;

        friend class ComboBoxModel;
    };

    typedef QVector<Item> Items;

    ComboBoxModel(Items*         items,
                  Model*         parent               = nullptr,
                  const QString& displayName          = "[combo box model]",
                  const QString& objectName           = QString::null,
                  bool           isDefaultConstructed = false) :
          IntModel(0,
                   0,
                   0,
                   parent,
                   displayName,
                   objectName,
                   isDefaultConstructed),
          m_items(items != nullptr ? items : new Items()),
          m_itemsOwned(items == nullptr)
    {
        setRange(min(), max());
        setValue(min());
    }

    ComboBoxModel(Model*         parent               = nullptr,
                  const QString& displayName          = "[combo box model]",
                  const QString& objectName           = QString::null,
                  bool           isDefaultConstructed = false) :
          ComboBoxModel(nullptr,
                        parent,
                        displayName,
                        objectName,
                        isDefaultConstructed)
    {
    }

    virtual ~ComboBoxModel()
    {
        cleanup();
    }

    void addItem(const QString&      _text,
                 const PixmapLoader* _icon = nullptr,
                 const QVariant&     _data = QVariant());

    void addItem(const int           _val,
                 const QString&      _text,
                 const PixmapLoader* _icon = nullptr,
                 const QVariant&     _data = QVariant());

    void addItem(const Item& _item);
    void addItems(const Items& _items);

    void setItems(const Items* _items);

    void clear();

    int findText(const QString& _what, bool _required, int notfound) const;
    int findData(const QVariant& _what, bool _required, int notfound) const;

    int nextValue() const;
    int previousValue() const;

    const QString currentText() const
    {
        int i = currentIndex();
        assertValidIndex(i);
        return m_items->at(i).m_text;
    }

    const PixmapLoader* currentIcon() const
    {
        int i = currentIndex();
        assertValidIndex(i);
        return m_items->at(i).m_icon;
    }

    const QVariant currentData() const
    {
        int i = currentIndex();
        assertValidIndex(i);
        return m_items->at(i).m_data;
    }

    const int itemValue(int _i) const
    {
        assertValidIndex(_i);
        return m_items->at(_i).m_value;
    }

    const QString itemText(int _i) const
    {
        assertValidIndex(_i);
        return m_items->at(_i).m_text;
    }

    const PixmapLoader* itemIcon(int _i) const
    {
        assertValidIndex(_i);
        return m_items->at(_i).m_icon;
    }

    const QVariant itemData(int _i) const
    {
        assertValidIndex(_i);
        return m_items->at(_i).m_data;
    }

    int min() const;
    int max() const;

    bool isEmpty() const
    {
        return size() == 0;
    }

    int size() const
    {
        return m_items->size();
    }

  protected:
    void cleanup();
    int  currentIndex() const;
    int  indexOf(int _val) const;

    Items* m_items;
    bool   m_itemsOwned;
    /*
    QVector<int>           m_values;
    QVector<QString>       m_texts;
    QVector<PixmapLoader*> m_icons;
    QVector<QVariant>      m_datas;
    */

  private:
    void assertNotEmpty() const;
    void assertValidIndex(int _i) const;
};

#endif
