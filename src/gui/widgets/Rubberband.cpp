/*
 * Rubberband.cpp - rubberband - either own implementation for Qt3 or wrapper
 *                               for Qt4
 *
 * Copyright (c) 2006-2011 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "Rubberband.h"

RubberBand::RubberBand(QWidget* _parent) : QRubberBand(Rectangle, _parent)
{
}

RubberBand::~RubberBand()
{
}

QVector<SelectableObject*> RubberBand::selectedObjects() const
{
    QVector<SelectableObject*> so = selectableObjects();
    for(QVector<SelectableObject*>::iterator it = so.begin(); it != so.end();)
    {
        if((*it)->isSelected() == false)
        {
            it = so.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return so;
}

void RubberBand::resizeEvent(QResizeEvent* _re)
{
    QRubberBand::resizeEvent(_re);
    if(isEnabled())
    {
        QVector<SelectableObject*> so = selectableObjects();
        for(QVector<SelectableObject*>::iterator it = so.begin();
            it != so.end(); ++it)
        {
            (*it)->setSelected(
                    QRect(pos(), size())
                            .intersects(QRect(
                                    (*it)->mapTo(parentWidget(), QPoint()),
                                    (*it)->size())));
        }
    }
}

QVector<SelectableObject*> RubberBand::selectableObjects() const
{
    QVector<SelectableObject*> so;
    if(parentWidget() == NULL)
    {
        return so;
    }

    QList<SelectableObject*> l
            = parentWidget()->findChildren<SelectableObject*>();
    for(QList<SelectableObject*>::iterator it = l.begin(); it != l.end();
        ++it)
    {
        so.push_back(*it);
    }
    return so;
}
