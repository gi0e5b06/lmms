/*
 * lmms_qt_gui.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include "lmms_qt_gui.h"

void lmms::fillRoundedRect(QPainter&     _p,
                           const QRectF& rect,
                           qreal         xRadius,
                           qreal         yRadius,
                           Qt::SizeMode  mode)
{
    if(mode == Qt::RelativeSize)
    {
        xRadius = rect.width() * xRadius / 2.;
        yRadius = rect.height() * yRadius / 2.;
    }
    QPainterPath path;
    path.addRoundedRect(rect, xRadius, yRadius);
    _p.fillPath(path,_p.brush());
}
