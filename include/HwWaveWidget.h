/*
 * HwWaveWidget.h -
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
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

#ifndef HW_WAVE_WIDGET_H
#define HW_WAVE_WIDGET_H

/*

#include "QQ_MACROS.h"
#include "WaveForm.h"
#include "export.h"

#include <QPaintEvent>
#include <QPixmap>
#include <QWidget>

class EXPORT HwWaveWidget : public QWidget
{
    Q_OBJECT

    QQ_LOCAL_PROPERTY(const WaveForm*, wave, wave, setWave)
    QQ_LOCAL_PROPERTY(QString, colorSet, colorSet, setColorSet)
    QQ_LOCAL_PROPERTY(int, columns, columns, setColumns)
    QQ_LOCAL_PROPERTY(int, rows, rows, setRows)
    QQ_LOCAL_PROPERTY(QString, label, label, setLabel)

  public:
    HwWaveWidget(QWidget*       parent   = nullptr,
                 int            columns  = 30,
                 int            rows     = 15,
                 const QString& label    = QString(""),
                 const QString& colorSet = QString("bog_2px"));

    virtual ~HwWaveWidget();

  public slots:
    // virtual void setMarginWidth( int width );

  protected:
    static const int LEVELS_PER_PIXMAP = 11;

    QSize    m_cellSize;
    QPixmap* m_screenPixmap;

    virtual void paintEvent(QPaintEvent* pe);

    virtual void updateSize();

    void initUi(int            columns,
                int            rows,
                const QString& label,
                const QString& colorSet);

  private:
};

*/

#endif
