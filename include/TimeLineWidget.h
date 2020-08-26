/*
 * TimeLineWidget.h - class timeLine, representing a time-line with position
 * marker
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef TIMELINE_H
#define TIMELINE_H

#include "JournallingObject.h"
#include "PlayPos.h"
#include "Widget.h"

#include <QToolButton>
//#include <QWidget>

class QPixmap;
class QToolBar;
class QToolButton;
class MidiTime;
class NStateButton;
// class SongEditor;
class TextFloat;

class TimeLineWidget : public Widget, public JournallingObject
{
    Q_OBJECT

  public:
    Q_PROPERTY(QColor barLineColor READ getBarLineColor WRITE setBarLineColor)
    Q_PROPERTY(QColor barNumberColor READ getBarNumberColor WRITE
                       setBarNumberColor)
    Q_PROPERTY(int loopRectangleVerticalPadding READ
                       getLoopRectangleVerticalPadding WRITE
                               setLoopRectangleVerticalPadding)

    Q_PROPERTY(QColor inactiveLoopColor READ getInactiveLoopColor WRITE
                       setInactiveLoopColor)
    Q_PROPERTY(QBrush inactiveLoopBrush READ getInactiveLoopBrush WRITE
                       setInactiveLoopBrush)
    Q_PROPERTY(QColor inactiveLoopInnerColor READ getInactiveLoopInnerColor
                       WRITE setInactiveLoopInnerColor)
    Q_PROPERTY(QColor inactiveLoopTextColor READ getInactiveLoopTextColor
                       WRITE setInactiveLoopTextColor)

    Q_PROPERTY(QColor activeLoopColor READ getActiveLoopColor WRITE
                       setActiveLoopColor)
    Q_PROPERTY(QBrush activeLoopBrush READ getActiveLoopBrush WRITE
                       setActiveLoopBrush)
    Q_PROPERTY(QColor activeLoopInnerColor READ getActiveLoopInnerColor WRITE
                       setActiveLoopInnerColor)
    Q_PROPERTY(QColor activeLoopTextColor READ getActiveLoopTextColor WRITE
                       setActiveLoopTextColor)

    Q_PROPERTY(QColor selectedLoopColor READ getSelectedLoopColor WRITE
                       setSelectedLoopColor)
    Q_PROPERTY(QBrush selectedLoopBrush READ getSelectedLoopBrush WRITE
                       setSelectedLoopBrush)
    Q_PROPERTY(QColor selectedLoopInnerColor READ getSelectedLoopInnerColor
                       WRITE setSelectedLoopInnerColor)
    Q_PROPERTY(QColor selectedLoopTextColor READ getSelectedLoopTextColor
                       WRITE setSelectedLoopTextColor)

    enum AutoScrollStates
    {
        AutoScrollEnabled,
        AutoScrollDisabled
    };

    enum LoopPointStates
    {
        LoopPointsDisabled,
        LoopPointsEnabled
    };

    enum BehaviourAtStopStates
    {
        BackToZero,
        BackToStart,
        KeepStopPosition
    };

    static const int NB_LOOPS      = 16;
    static const int NB_LOOP_SIZES = 9;

    TimeLineWidget(int             xoff,
                   int             yoff,
                   float           ppt,
                   PlayPos&        pos,
                   const MidiTime& begin,
                   QWidget*        parent);
    virtual ~TimeLineWidget();

    INLINE QColor const& getBarLineColor() const
    {
        return m_barLineColor;
    }

    INLINE void setBarLineColor(QColor const& tactLineColor)
    {
        m_barLineColor = tactLineColor;
    }

    INLINE QColor const& getBarNumberColor() const
    {
        return m_barNumberColor;
    }

    INLINE void setBarNumberColor(QColor const& tactNumberColor)
    {
        m_barNumberColor = tactNumberColor;
    }

    INLINE int const& getLoopRectangleVerticalPadding() const
    {
        return m_loopRectangleVerticalPadding;
    }

    INLINE void setLoopRectangleVerticalPadding(
            int const& loopRectangleVerticalPadding)
    {
        m_loopRectangleVerticalPadding = loopRectangleVerticalPadding;
    }

    INLINE QColor const& getInactiveLoopColor() const
    {
        return m_inactiveLoopColor;
    }

    INLINE void setInactiveLoopColor(QColor const& inactiveLoopColor)
    {
        m_inactiveLoopColor = inactiveLoopColor;
    }

    INLINE QBrush const& getInactiveLoopBrush() const
    {
        return m_inactiveLoopBrush;
    }

    INLINE void setInactiveLoopBrush(QBrush const& inactiveLoopBrush)
    {
        m_inactiveLoopBrush = inactiveLoopBrush;
    }

    INLINE QColor const& getInactiveLoopInnerColor() const
    {
        return m_inactiveLoopInnerColor;
    }

    INLINE void
            setInactiveLoopInnerColor(QColor const& inactiveLoopInnerColor)
    {
        m_inactiveLoopInnerColor = inactiveLoopInnerColor;
    }

    INLINE QColor const& getInactiveLoopTextColor() const
    {
        return m_inactiveLoopTextColor;
    }

    INLINE void setInactiveLoopTextColor(QColor const& inactiveLoopTextColor)
    {
        m_inactiveLoopTextColor = inactiveLoopTextColor;
    }

    INLINE QColor const& getActiveLoopColor() const
    {
        return m_activeLoopColor;
    }

    INLINE void setActiveLoopColor(QColor const& activeLoopColor)
    {
        m_activeLoopColor = activeLoopColor;
    }

    INLINE QBrush const& getActiveLoopBrush() const
    {
        return m_activeLoopBrush;
    }

    INLINE void setActiveLoopBrush(QBrush const& activeLoopBrush)
    {
        m_activeLoopBrush = activeLoopBrush;
    }

    INLINE QColor const& getActiveLoopInnerColor() const
    {
        return m_activeLoopInnerColor;
    }

    INLINE void setActiveLoopInnerColor(QColor const& activeLoopInnerColor)
    {
        m_activeLoopInnerColor = activeLoopInnerColor;
    }

    INLINE QColor const& getActiveLoopTextColor() const
    {
        return m_activeLoopTextColor;
    }

    INLINE void setActiveLoopTextColor(QColor const& activeLoopTextColor)
    {
        m_activeLoopTextColor = activeLoopTextColor;
    }

    INLINE QColor const& getSelectedLoopColor() const
    {
        return m_selectedLoopColor;
    }

    INLINE void setSelectedLoopColor(QColor const& selectedLoopColor)
    {
        m_selectedLoopColor = selectedLoopColor;
    }

    INLINE QBrush const& getSelectedLoopBrush() const
    {
        return m_selectedLoopBrush;
    }

    INLINE void setSelectedLoopBrush(QBrush const& selectedLoopBrush)
    {
        m_selectedLoopBrush = selectedLoopBrush;
    }

    INLINE QColor const& getSelectedLoopInnerColor() const
    {
        return m_selectedLoopInnerColor;
    }

    INLINE void
            setSelectedLoopInnerColor(QColor const& selectedLoopInnerColor)
    {
        m_selectedLoopInnerColor = selectedLoopInnerColor;
    }

    INLINE QColor const& getSelectedLoopTextColor() const
    {
        return m_selectedLoopTextColor;
    }

    INLINE void setSelectedLoopTextColor(QColor const& selectedLoopTextColor)
    {
        m_selectedLoopTextColor = selectedLoopTextColor;
    }

    INLINE PlayPos& pos() const
    {
        return m_pos;
    }

    AutoScrollStates autoScroll() const
    {
        return m_autoScroll;
    }

    BehaviourAtStopStates behaviourAtStop() const
    {
        return m_behaviourAtStop;
    }

    /*INLINE*/ int currentLoop() const
    {
        return m_currentLoop;
    }

    void setCurrentLoop(int n);

    /*INLINE*/ int nextLoop() const
    {
        return m_nextLoop;
    }

    void setNextLoop(int n);

    int findLoop(const MidiTime& t);

    bool loopPointsEnabled(int n = -1) const
    {
        if(n == -1)
            n = m_currentLoop;
        Q_ASSERT((n >= 0) && (n < NB_LOOPS));

        return (n == m_currentLoop) && (m_loopPoints == LoopPointsEnabled);
    }

    const MidiTime& loopBegin(int n = -1) const
    {
        if(n == -1)
            n = m_currentLoop;
        Q_ASSERT((n >= 0) && (n < NB_LOOPS));

        return (m_loopPos[2 * n + 0] < m_loopPos[2 * n + 1])
                       ? m_loopPos[2 * n + 0]
                       : m_loopPos[2 * n + 1];
    }

    const MidiTime& loopEnd(int n = -1) const
    {
        if(n == -1)
            n = m_currentLoop;
        Q_ASSERT((n >= 0) && (n < NB_LOOPS));

        return (m_loopPos[2 * n + 0] > m_loopPos[2 * n + 1])
                       ? m_loopPos[2 * n + 0]
                       : m_loopPos[2 * n + 1];
    }

    void setLoopStart(int _n, int _x);
    void setLoopEnd(int _n, int _x);

    MidiTime adjustTime(const MidiTime& _time, int _loop = -1)
    {
        if((_loop < 0) || (_loop >= NB_LOOPS))
            return _time;

        return loopBegin(_loop) + _time % (loopEnd(_loop) - loopBegin(_loop));
    }

    INLINE void savePos(const MidiTime& _pos)
    {
        m_savedPos = _pos;
    }

    INLINE const MidiTime& savedPos() const
    {
        return m_savedPos;
    }

    INLINE void setPixelsPerTact(float _ppt)
    {
        m_ppt = _ppt;
        update();
    }

    void addToolButtons(QToolBar* _toolBar);
    void addLoopMarkButtons(QToolBar* _toolBar);
    void addLoopSizeButtons(QToolBar* _toolBar);

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "timeline";
    }

    INLINE int markerX(const MidiTime& _t) const
    {
        return m_xOffset
               + static_cast<int>((_t - m_begin) * m_ppt
                                  / MidiTime::ticksPerTact());
    }

  signals:
    void regionSelectedFromPixels(int, int);
    void selectionFinished();

  public slots:
    void updatePosition(const MidiTime& t);
    void updatePosition();
    void updateLoopButtons();
    void updateResizeButtons();

    void toggleAutoScroll(int _n);
    void toggleLoopPoints(int _n);
    void toggleBehaviourAtStop(int _n);

    void selectLoop(QAction* _a);
    void selectLoop(const MidiTime& t);
    void resizeLoop(QAction* _a);
    void handleContextMenuAction(QAction* _a);
    void selectSubloop(QAction* _a);

  protected:
    virtual void contextMenuEvent(QContextMenuEvent*);
    // virtual void paintEvent(QPaintEvent* _pe);
    virtual void mousePressEvent(QMouseEvent* _me);
    virtual void mouseMoveEvent(QMouseEvent* _me);
    virtual void mouseReleaseEvent(QMouseEvent* _me);

    virtual void drawLoop(const int num, QPainter& p, const int cy);
    virtual void drawWidget(QPainter& _p);

  private:
    static QPixmap* s_posMarkerPixmap;

    QColor m_inactiveLoopColor;
    QBrush m_inactiveLoopBrush;
    QColor m_inactiveLoopInnerColor;
    QColor m_inactiveLoopTextColor;

    QColor m_activeLoopColor;
    QBrush m_activeLoopBrush;
    QColor m_activeLoopInnerColor;
    QColor m_activeLoopTextColor;

    QColor m_selectedLoopColor;
    QBrush m_selectedLoopBrush;
    QColor m_selectedLoopInnerColor;
    QColor m_selectedLoopTextColor;

    int m_loopRectangleVerticalPadding;

    QColor m_barLineColor;
    QColor m_barNumberColor;

    AutoScrollStates      m_autoScroll;
    LoopPointStates       m_loopPoints;
    BehaviourAtStopStates m_behaviourAtStop;

    bool m_changedPosition;

    int             m_xOffset;
    int             m_posMarkerX;
    real_t          m_ppt;
    PlayPos&        m_pos;
    const MidiTime& m_begin;
    MidiTime        m_loopPos[2 * NB_LOOPS];

    MidiTime m_savedPos;

    int          m_currentLoop;
    int          m_nextLoop;
    QToolButton* m_loopButtons[NB_LOOPS];

    static const float LOOP_SIZES[NB_LOOP_SIZES];
    QToolButton*       m_resizeButtons[NB_LOOP_SIZES];

    TextFloat* m_hint;
    int        m_initalXSelect;

    enum actions
    {
        NoAction,
        MovePositionMarker,
        MoveLoopBegin,
        MoveLoopEnd,
        SelectSongTCO,
    } m_action;

    int m_moveXOff;

  signals:
    void positionChanged(const MidiTime& _t);
    void verticalLine(int _xs, int _xe);
    void loopPointStateLoaded(int _n);
    void positionMarkerMoved();

    friend class ExportFilter;
    friend class ImportFilter;
};

#endif
