/*
 * FxLineLcdSpinBox.h -
 *
 * Copyright (c) 2019      gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef FX_LINE_LCD_SPINBOX_H
#define FX_LINE_LCD_SPINBOX_H

#include "LcdSpinBox.h"

class FxLineLcdSpinBox : public LcdSpinBox
{
    Q_OBJECT

  public:
    FxLineLcdSpinBox(int            _num_digits,
                     QWidget*       _parent,
                     const QString& _name);
    ~FxLineLcdSpinBox();

  protected:
    virtual void mouseDoubleClickEvent(QMouseEvent* _me);
    virtual void contextMenuEvent(QContextMenuEvent* _cme);
};

#endif
