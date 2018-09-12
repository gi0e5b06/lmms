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

#ifndef VISUALIZATION_WIDGET_H_
#define VISUALIZATION_WIDGET_H_

//#include "PaintCacheable.h"
//#include "MemoryManager.h"
#include "Ring.h"
#include "Widget.h"
//#include "lmms_basics.h"

#include <QPixmap>
//#include <QWidget>

class VisualizationWidget
      : public Widget
      , public virtual PaintCacheable
{
    Q_OBJECT
    // MM_OPERATORS

  public:
    static constexpr auto _CMSL_ = __LINE__;
    enum ChannelMode
    {
        Left,
        Right,
        Stereo,
        Joint,
        Average,
        Difference,
        MinMax,
        RhoTan
    };
    static constexpr auto ChannelModeCount = __LINE__ - _CMSL_ - 4;

    static constexpr auto _DMSL_ = __LINE__;
    enum DisplayMode
    {
        Off,
        TimeLast,
        Time,
        XYLast,
        XY
    };
    static constexpr auto DisplayModeCount = __LINE__ - _DMSL_ - 4;

    VisualizationWidget(int         _width,
                        int         _height,
                        QWidget*    _parent,
                        Ring*       _ring,
                        ChannelMode _channelMode = Stereo,
                        DisplayMode _displayMode = Time);
    VisualizationWidget(const QPixmap& _bg,
                        QWidget*       _parent,
                        Ring*          _ring,
                        ChannelMode    _channelMode = Stereo,
                        DisplayMode    _displayMode = Time);
    virtual ~VisualizationWidget();

    void setDisplayMode(DisplayMode _mode);
    void setChannelMode(ChannelMode _mode);
    void setFrozen(bool _frozen);
    void setStabilized(bool _stabilized);

    virtual void invalidateCache();

  public slots:
    virtual void update();  // needed because of Qt slot

  protected:
    virtual void drawWidget(QPainter& _p);
    // virtual void paintEvent(QPaintEvent* _pe);
    virtual void mousePressEvent(QMouseEvent* _me);
    virtual void wheelEvent(QWheelEvent* _we);

    // interfaces
    // using PaintCacheable::update;
    // virtual void updateNow()
    //{
    //    QWidget::update();
    //}

    // protected slots:
    // void updateSurroundBuffer(const surroundSampleFrame* _buffer);
    // void updateStereoBuffer(const sampleFrame* _buffer);

  private:
    void applyChannelMode(float& _v0, float& _v1) const;

    ChannelMode m_channelMode;
    DisplayMode m_displayMode;
    bool        m_stabilized;
    float       m_zoomX;
    float       m_zoomY;
    Ring*       m_ring;
    // sampleFrame* m_buffer;
    // int          m_len;
    //float    m_masterOutput;
    //float    m_peakLeft;
    //float    m_peakRight;
    //float    m_maxLevel;
    int      m_pointCount;
    QPointF* m_points;
    QPixmap  m_background;
};

#endif
