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

#ifndef CHART_H
#define CHART_H

#include <QColor>
#include <QPointF>
#include <QVector>
#include <QWidget>

class ChartSeries : QObject
{
    Q_OBJECT

  public:
    ChartSeries(QVector<QPointF>& _points, QColor& _foreground);
    ~ChartSeries();

    QVector<QPointF>& points();
    QColor&           foreground();

  private:
    QVector<QPointF> m_points;
    QColor           m_foreground;
};

class Chart : public QWidget
{
    Q_OBJECT

  public:
    Chart(QWidget* _parent);
    virtual ~Chart();

  protected:
    virtual void paintEvent(QPaintEvent* _pe);

  private:
    QVector<ChartSeries*> m_series;
};

#endif
