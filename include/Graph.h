/*
 * Graph.h - a QT widget for displaying and manipulating waveforms
 *
 * Copyright (c) 2006-2007 Andreas Brandmaier <andy/at/brandmaier/dot/de>
 *               2008 Paul Giblock <drfaygo/at/gmail/dot/com>
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

#ifndef GRAPH_H
#define GRAPH_H

#include "Model.h"
#include "ModelView.h"
#include "lmms_basics.h"

#include <QPixmap>
#include <QWidget>
//#include <QCursor>

class GraphModel;

class EXPORT Graph
      : public QWidget
      , public ModelView
{
    Q_OBJECT
  public:
    enum graphStyle
    {
        NearestStyle,
        LinearStyle,
        LinearNonCyclicStyle,
        BarStyle,
        // BarCenterGradStyle,
        NumGraphStyles
    };

    Graph(QWidget*   _parent,
          graphStyle _style  = Graph::LinearStyle,
          int        _width  = 132,
          int        _height = 104);
    virtual ~Graph();

    void setForeground(const QPixmap& _pixmap);

    void setGraphColor(const QColor);

    inline GraphModel* model()
    {
        return castModel<GraphModel>();
    }

    inline graphStyle getGraphStyle()
    {
        return m_graphStyle;
    }

    inline void setGraphStyle(graphStyle _s)
    {
        m_graphStyle = _s;
        update();
    }

  signals:
    void drawn();

  protected:
    virtual void paintEvent(QPaintEvent* _pe);
    virtual void dropEvent(QDropEvent* _de);
    virtual void dragEnterEvent(QDragEnterEvent* _dee);
    virtual void mousePressEvent(QMouseEvent* _me);
    virtual void mouseMoveEvent(QMouseEvent* _me);
    virtual void mouseReleaseEvent(QMouseEvent* _me);

  protected slots:
    void updateGraph(int _startPos, int _endPos);
    void updateGraph();

  private:
    virtual void modelChanged();

    void changeSampleAt(int _x, int _y);
    void drawLineAt(int _x, int _y, int _lastx);

    QPixmap m_foreground;
    QColor  m_graphColor;

    graphStyle m_graphStyle;

    bool m_mouseDown;
    int  m_lastCursorX;
};

class EXPORT GraphModel : public Model
{
    Q_OBJECT
  public:
    GraphModel(FLOAT    _min,
               FLOAT    _max,
               int      _size,
               ::Model* _parent,
               bool     _default_constructed = false,
               FLOAT    _step                = 0.);

    virtual ~GraphModel();

    // TODO: saveSettings, loadSettings?

    inline FLOAT minValue() const
    {
        return (m_minValue);
    }

    inline FLOAT maxValue() const
    {
        return (m_maxValue);
    }

    inline int length() const
    {
        return m_length;
    }

    inline const FLOAT* samples() const
    {
        return (m_samples.data());
    }

    void convolve(const FLOAT* convolution,
                  const int    convolutionLength,
                  const int    centerOffset);

  public slots:
    void setRange(FLOAT _min, FLOAT _max);

    void setLength(int _size);

    void setSampleAt(int x, FLOAT val);
    void setSamples(const FLOAT* _value);

    void    setWaveToSine();
    void    setWaveToTriangle();
    void    setWaveToSaw();
    void    setWaveToSquare();
    void    setWaveToNoise();
    QString setWaveToUser();

    void smooth();
    void smoothNonCyclic();
    void normalize();
    void invert();
    void shiftPhase(int _deg);
    void clear();

  signals:
    void lengthChanged();
    void samplesChanged(int startPos, int endPos);
    void rangeChanged();

  private:
    void drawSampleAt(int x, FLOAT val);

    QVector<FLOAT> m_samples;
    int            m_length;
    FLOAT          m_minValue;
    FLOAT          m_maxValue;
    FLOAT          m_step;

    friend class Graph;
};

#endif
