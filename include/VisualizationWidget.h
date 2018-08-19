/*
 * VisualizationWidget.h - widget for visualization of sound-data
 *
 * Copyright (c) 2005-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef _VISUALIZATION_WIDGET
#define _VISUALIZATION_WIDGET

#include "lmms_basics.h"

#include <QPixmap>
#include <QWidget>

class VisualizationWidget : public QWidget
{
    Q_OBJECT
  public:
    enum visualizationTypes
    {
        Simple  // add more here
    };

    VisualizationWidget(const QPixmap&     _bg,
                        QWidget*           _parent,
                        visualizationTypes _vtype = Simple);
    virtual ~VisualizationWidget();

    void setActive(bool _active);
    void setFrozen(bool _frozen);
    void setStabilized(bool _stabilized);

  protected:
    virtual void paintEvent(QPaintEvent* _pe);
    virtual void mousePressEvent(QMouseEvent* _me);

  protected slots:
    void updateAudioBuffer(const surroundSampleFrame* _buffer);

  private:
    bool         m_active;
    bool         m_frozen;
    bool         m_stabilized;
    sampleFrame* m_buffer;
    int          m_pointCount;
    QPointF*     m_points;
    QPixmap      s_background;
};

#endif
