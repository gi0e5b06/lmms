/*
 * Chart.h - a minimal widget for displaying series
 *
 * Copyright (c) 2018 gi0e5b06
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


#include <QPainter>

#include "Chart.h"


Chart::Chart(QWidget* _parent)
        : QWidget(_parent)
{
        setFixedSize(220,55);
}

Chart::~Chart()
{
}

void Chart::paintEvent(QPaintEvent* _pe)
{
    QPainter  p(this);
    const int wc = width();
    const int hc = height();

    p.fillRect(0, 0, wc, hc, Qt::white);
    for(ChartSeries* s : m_series)
    {
        p.setPen(s->foreground());
        p.drawPolyline(s->points().constData(), s->points().size());
    }
}

ChartSeries::ChartSeries(QVector<QPointF>& _points, QColor& _foreground)
        : m_points(_points), m_foreground(_foreground)
{
}

ChartSeries::~ChartSeries()
{
}

QVector<QPointF>& ChartSeries::points()
{
        return m_points;
}

QColor& ChartSeries::foreground()
{
        return m_foreground;
}
