/*
 * Widget.h - base widget for new components
 *
 * Copyright (c) 2018 gi0e5b06
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of"the GNU General Public
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

#include "Widget.h"

Widget::Widget(QWidget* _parent) : QWidget(_parent)
{
}

Widget::~Widget()
{
}

void Widget::paintEvent(QPaintEvent*)
{
    resizeCache(width(), height());

    QPainter p(this);
    if(paintCache(p))
        return;

    QPainter& pc = beginCache();
    drawWidget(pc);
    endCache();

    paintCache(p);
}

void Widget::updateNow()
{
    QWidget::update();
}

void Widget::update()
{
    // invalidateCache();
    // if(isVisible())
    PaintCacheable::update();
}
