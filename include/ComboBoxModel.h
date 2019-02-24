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

#include <QVariant>
#include <QVector>

#include "AutomatableModel.h"

class PixmapLoader;

class EXPORT ComboBoxModel : public IntModel
{
    Q_OBJECT
  public:
    ComboBoxModel(Model*         parent = nullptr,
                  const QString& displayName = "[combo box model]",
                  bool           isDefaultConstructed = false) :
          IntModel(0, 0, 0, parent, displayName, isDefaultConstructed)
    {
    }

    virtual ~ComboBoxModel()
    {
        clear();
    }

    void addItem(const QString&  _text,
                 PixmapLoader*   _icon = NULL,
                 const QVariant& _data = QVariant());

    void clear();

    int findText(const QString& _what) const;

    const QString currentText() const
    {
        return m_texts[value()];
    }

    const PixmapLoader* currentIcon() const
    {
        return m_icons[value()];
    }

    const QVariant currentData() const
    {
        return m_datas[value()];
    }

    const QString itemText(int _i) const
    {
        return m_texts[_i];
    }

    const PixmapLoader* itemIcon(int _i) const
    {
        return m_icons[_i];
    }

    const QVariant itemData(int _i) const
    {
        return m_texts[_i];
    }

    int size() const
    {
        return m_texts.size();
    }

  protected:
    QVector<QString>       m_texts;
    QVector<PixmapLoader*> m_icons;
    QVector<QVariant>      m_datas;
};

#endif
