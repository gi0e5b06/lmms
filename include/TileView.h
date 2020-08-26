/*
 * TileView.h -
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef TILE_VIEW_H
#define TILE_VIEW_H

//#include "MidiTime.h"
#include "Rubberband.h"
//#include "JournallingObject.h"
//#include "AutomatableModel.h"
#include "DataFile.h"
#include "ModelView.h"
//#include "Mutex.h"
//#include "SafeList.h"
//#include "TimeLineWidget.h"
#include "lmms_basics.h"

//#include <QVector>
//#include <QList>
#include <QMenu>
#include <QPointer>
//#include <QWidget>
//#include <QSignalMapper>
#include <QColor>
//#include <QMimeData>

class QMenu;
class QPushButton;

class PixmapButton;
class TextFloat;
class Tile;
class Track;
class TileView;
class TrackContainer;
class TrackContainerView;
class TrackContentWidget;
class TrackView;

typedef QVector<QPointer<TileView>> TileViews;

const int TCO_BORDER_WIDTH = 1;  // 2

class TileView : public SelectableObject, public ModelView
{
    Q_OBJECT

    // theming qproperties
    Q_PROPERTY(QColor mutedColor READ mutedColor WRITE setMutedColor)
    Q_PROPERTY(QColor mutedBackgroundColor READ mutedBackgroundColor WRITE
                       setMutedBackgroundColor)
    Q_PROPERTY(QColor selectedColor READ selectedColor WRITE setSelectedColor)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QColor textBackgroundColor READ textBackgroundColor WRITE
                       setTextBackgroundColor)
    Q_PROPERTY(QColor textShadowColor READ textShadowColor WRITE
                       setTextShadowColor)
    Q_PROPERTY(QColor BBPatternBackground READ BBPatternBackground WRITE
                       setBBPatternBackground)
    // Q_PROPERTY( bool gradient READ gradient WRITE setGradient )

  public:
    // bool fixedTCOs();

    INLINE Tile* tile()
    {
        return m_tile;
    }

    // obsolete
    INLINE Tile* getTile()
    {
        return tile();
    }

    bool   isFixed() const;
    real_t pixelsPerTact() const;
    bool   useStyleColor() const;
    void   setUseStyleColor(bool _use);
    QColor color() const;
    void   setColor(const QColor& _newColor);

    // qproperty access func
    QColor mutedColor() const;
    QColor mutedBackgroundColor() const;
    QColor selectedColor() const;
    QColor textColor() const;
    QColor textBackgroundColor() const;
    QColor textShadowColor() const;
    QColor BBPatternBackground() const;
    // bool gradient() const;
    void setMutedColor(const QColor& c);
    void setMutedBackgroundColor(const QColor& c);
    void setSelectedColor(const QColor& c);
    void setTextColor(const QColor& c);
    void setTextBackgroundColor(const QColor& c);
    void setTextShadowColor(const QColor& c);
    void setBBPatternBackground(const QColor& c);
    // void setGradient( const bool & b );

    // access needsUpdate member variable
    bool needsUpdate();
    void setNeedsUpdate(bool b);

#if(QT_VERSION < 0x050000)
    INLINE QPixmap grab(const QRect& rectangle
                        = QRect(QPoint(0, 0), QSize(-1, -1)))
    {
        return QPixmap::grabWidget(this, rectangle);
    }
#endif

  public slots:
    virtual bool close();
    virtual void update();

    virtual void remove() final;
    virtual void mute() final;
    virtual void clear() final;
    virtual void cut() final;
    virtual void copy() final;
    virtual void paste() final;
    virtual void changeAutoResize() final;
    virtual void changeAutoRepeat() final;
    virtual void changeName() final;
    virtual void resetName() final;
    virtual void changeColor();
    virtual void resetColor();

  protected:
    TileView(Tile* _tile, TrackView* _trackView);
    virtual ~TileView();

    void doConnections() override;
    void undoConnections() override;

    virtual QMenu* buildContextMenu() = 0;
    virtual void   addRemoveMuteClearMenu(QMenu* _cm,
                                          bool   _remove,
                                          bool   _mute,
                                          bool   _clear) final;
    virtual void   addCutCopyPasteMenu(QMenu* _cm,
                                       bool   _cut,
                                       bool   _copy,
                                       bool   _paste) final;
    virtual void   addSplitMenu(QMenu* _cm, bool _one, bool _four) final;
    virtual void   addFlipMenu(QMenu* _cm, bool _x, bool _y) final;
    virtual void   addRotateMenu(QMenu* _cm,
                                 bool   _bar,
                                 bool   _beat,
                                 bool   _step) final;
    virtual void
            addStepMenu(QMenu* _cm, bool _bar, bool _beat, bool _step) final;
    virtual void
                 addPropertiesMenu(QMenu* _cm, bool _resize, bool _repeat) final;
    virtual void addNameMenu(QMenu* _cm, bool _enabled) final;
    virtual void addColorMenu(QMenu* _cm, bool _enabled) final;
    virtual void contextMenuEvent(QContextMenuEvent* _cme) final;

    void dragEnterEvent(QDragEnterEvent* dee) override;
    // void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent* de) override;
    void leaveEvent(QEvent* e) override;
    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;

    void resizeEvent(QResizeEvent* re) override
    {
        m_needsUpdate = true;
        SelectableObject::resizeEvent(re);
    }

    INLINE TrackView* getTrackView()
    {
        return m_trackView;
    }

    DataFile createTCODataFiles(const TileViews& tcos) const;

    virtual void paintTileLoop(QPainter& painter) final;
    virtual void paintTextLabel(const QString& text,
                                const QColor&  bg,
                                QPainter&      painter) final;
    virtual void paintTileBorder(const bool    current,
                                 const bool    ghost,
                                 const QColor& bg,
                                 QPainter&     painter) final;
    virtual void paintTileTacts(const bool    current,
                                tact_t        nbt,
                                tact_t        tpg,
                                const QColor& bg,
                                QPainter&     painter) final;
    virtual void paintMutedIcon(const bool muted, QPainter& painter) final;
    virtual void paintFrozenIcon(const bool frozen, QPainter& painter) final;

  protected slots:
    void updateLength();
    void updatePosition();

  private:
    enum Actions
    {
        NoAction,
        Move,
        MoveSelection,
        CopySelection,
        ResizeLeft,
        ResizeRight,
        ToggleSelected
    };

    static TextFloat* s_textFloat;

    Tile*      m_tile;
    TrackView* m_trackView;
    Actions    m_action;
    QPoint     m_initialMousePos;
    QPoint     m_initialMouseGlobalPos;

    TextFloat* m_hint;

    // qproperty fields
    QColor m_mutedColor;
    QColor m_mutedBackgroundColor;
    QColor m_selectedColor;
    QColor m_textColor;
    QColor m_textBackgroundColor;
    QColor m_textShadowColor;
    QColor m_BBPatternBackground;
    // bool m_gradient;
    bool m_needsUpdate;

    INLINE void setInitialMousePos(QPoint pos)
    {
        m_initialMousePos       = pos;
        m_initialMouseGlobalPos = mapToGlobal(pos);
    }

    bool mouseMovedDistance(QMouseEvent* me, int distance);
};

#endif
