/*
 * TrackView.h -
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

#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

//#include "Rubberband.h"
#include "JournallingObject.h"
//#include "AutomatableModel.h"
//#include "DataFile.h"
#include "MidiTime.h"
#include "ModelView.h"
//#include "Mutex.h"
//#include "SafeList.h"
//#include "TimeLineWidget.h"
//#include "lmms_basics.h"

//#include <QVector>
//#include <QList>
#include <QMenu>
//#include <QPointer>
//#include <QWidget>
//#include <QSignalMapper>
//#include <QColor>
#include <QMimeData>

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

// typedef QVector<QPointer<Tile>>      Tiles;
typedef QVector<QPointer<TileView>> TileViews;
// typedef QVector<QPointer<Track>>     TrackList;
// typedef QVector<QPointer<Track>>     Tracks;
typedef QVector<QPointer<TrackView>> TrackViews;

const int DEFAULT_SETTINGS_WIDGET_WIDTH = 224;
const int TRACK_OP_WIDTH                = 78;
// This shaves 150-ish pixels off track buttons,
// ruled from config: ui.compacttrackbuttons
const int DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT = 106;  // 96;
const int TRACK_OP_WIDTH_COMPACT                = 70;   // 62;

class HyperBarView : public QObject
{
  public:
    HyperBarView(int length, const QColor& color, const QString& label);
    // HyperBarView(HyperBarView& hbv);
    INLINE const int length() const
    {
        return m_length;
    }
    INLINE const QColor& color() const
    {
        return m_color;
    }
    INLINE const QString& label() const
    {
        return m_label;
    }

  private:
    // HyperBarView();
    const int     m_length;
    const QColor  m_color;
    const QString m_label;
};

class BarView : public QObject
{
  public:
    enum Types
    {
        START,
        MIDDLE,
        END
    };

    BarView(const QPointer<HyperBarView>& hbv, Types type, bool sign);
    // BarView(BarView& bv);
    // INLINE HyperBarView hyperbar() { return m_hbv; }
    INLINE const Types type() const
    {
        return m_type;
    }
    INLINE const bool sign() const
    {
        return m_sign;
    }
    INLINE const QColor color() const
    {
        return m_hbv->color();
    }

  private:
    // BarView();
    const QPointer<HyperBarView> m_hbv;
    const Types                  m_type;
    const bool                   m_sign;
};

class TrackContentWidget final : public QWidget, public JournallingObject
{
    Q_OBJECT

    // qproperties for track background gradients
    Q_PROPERTY(QBrush darkerColor READ darkerColor WRITE setDarkerColor)
    Q_PROPERTY(QBrush lighterColor READ lighterColor WRITE setLighterColor)
    Q_PROPERTY(QBrush gridColor READ gridColor WRITE setGridColor)
    Q_PROPERTY(QBrush embossColor READ embossColor WRITE setEmbossColor)

  public:
    TrackContentWidget(TrackView* parent);
    virtual ~TrackContentWidget();

    bool   isFixed() const;
    real_t pixelsPerTact() const;

    /*! \brief Updates the background tile pixmap. */
    void updateBackground();

    void addTCOView(TileView* tcov);
    void removeTCOView(TileView* tcov);
    void removeTCOView(int tcoNum)
    {
        if(tcoNum >= 0 && tcoNum < m_tileViews.size())
        {
            removeTCOView(m_tileViews[tcoNum]);
        }
    }

    virtual const TileViews& tileViews() const final;

    bool canPasteSelection(MidiTime tcoPos, const QMimeData* mimeData);
    bool pasteSelection(MidiTime tcoPos, QDropEvent* de);

    MidiTime endPosition(const MidiTime& posStart);

    // qproperty access methods

    QBrush darkerColor() const;
    QBrush lighterColor() const;
    QBrush gridColor() const;
    QBrush embossColor() const;

    void setDarkerColor(const QBrush& c);
    void setLighterColor(const QBrush& c);
    void setGridColor(const QBrush& c);
    void setEmbossColor(const QBrush& c);

  public slots:
    void update();
    void changePosition(const MidiTime& newPos = MidiTime(-1));

  protected:
    virtual void dragEnterEvent(QDragEnterEvent* dee);
    virtual void dropEvent(QDropEvent* de);
    virtual void mousePressEvent(QMouseEvent* me);
    virtual void paintEvent(QPaintEvent* pe);
    virtual void resizeEvent(QResizeEvent* re);

    virtual void paintLoop(QPainter& p, const MidiTime& t0, const real_t ppt);
    virtual void paintGrid(QPainter&                         p,
                           const MidiTime&                   t0,
                           const real_t                      ppt,
                           const QVector<QPointer<BarView>>& barViews);
    virtual void paintCell(QPainter&                p,
                           int                      xc,
                           int                      yc,
                           int                      wc,
                           int                      hc,
                           const QPointer<BarView>& barView,
                           bool                     sign);

    virtual QString nodeName() const
    {
        return "trackcontentwidget";
    }

    virtual void saveSettings(QDomDocument& doc, QDomElement& element)
    {
        Q_UNUSED(doc)
        Q_UNUSED(element)
    }

    virtual void loadSettings(const QDomElement& element)
    {
        Q_UNUSED(element)
    }

  private:
    Track*   track();
    MidiTime getPosition(int mouseX);

    TrackView* m_trackView;
    TileViews  m_tileViews;

    QPixmap m_background;

    // qproperty fields
    QBrush m_darkerColor;
    QBrush m_lighterColor;
    QBrush m_gridColor;
    QBrush m_embossColor;
};

class TrackOperationsWidget final : public QWidget
{
    Q_OBJECT

  public:
    TrackOperationsWidget(TrackView* parent);
    virtual ~TrackOperationsWidget();

  public slots:
    void handleLoopMenuAction(QAction* _a);

  protected:
    virtual void addLoopMenu(QMenu* _cm, bool _enabled) final;
    virtual void addNameMenu(QMenu* _cm, bool _enabled) final;
    virtual void addColorMenu(QMenu* _cm, bool _enabled) final;
    virtual void addSpecificMenu(QMenu* _cm, bool _enabled) final;

    virtual void mousePressEvent(QMouseEvent* me);
    virtual void paintEvent(QPaintEvent* pe);

  private slots:
    void cloneTrack();
    void spawnTrack();
    void splitTrack();
    void isolateTrack();
    void removeTrack();
    void updateMenu();
    // void recordingOn();
    // void recordingOff();
    void clearTrack();
    void changeName();
    void resetName();
    void changeColor();
    void resetColor();

  private:
    static QPixmap* s_grip;

    TrackView* m_trackView;

    QPushButton*  m_trackOps;
    PixmapButton* m_muteBtn;
    PixmapButton* m_soloBtn;
    PixmapButton* m_clippingBtn;
    PixmapButton* m_frozenBtn;

    friend class TrackView;

  signals:
    void trackRemovalScheduled(TrackView* t);
};

class TrackView : public QWidget, public ModelView, public JournallingObject
{
    Q_OBJECT

  public:
    virtual QColor cableColor() const;
    virtual bool   isFixed() const final;
    virtual real_t pixelsPerTact() const final;

    INLINE virtual const Track* track() const final
    {
        return m_track;
    }

    INLINE virtual Track* track() final
    {
        return m_track;
    }

    INLINE virtual const TrackContainerView* trackContainerView() const final
    {
        return m_trackContainerView;
    }

    INLINE virtual TrackContainerView* trackContainerView() final
    {
        return m_trackContainerView;
    }

    INLINE virtual const TrackOperationsWidget*
            getTrackOperationsWidget() const final
    {
        return &m_trackOperationsWidget;
    }

    INLINE virtual TrackOperationsWidget* getTrackOperationsWidget() final
    {
        return &m_trackOperationsWidget;
    }

    INLINE virtual const QWidget* getTrackSettingsWidget() const final
    {
        return &m_trackSettingsWidget;
    }

    INLINE virtual QWidget* getTrackSettingsWidget() final
    {
        return &m_trackSettingsWidget;
    }

    INLINE virtual const TrackContentWidget*
            getTrackContentWidget() const final
    {
        return &m_trackContentWidget;
    }

    INLINE virtual TrackContentWidget* getTrackContentWidget() final
    {
        return &m_trackContentWidget;
    }

    bool virtual isMovingTrack() const final
    {
        return m_action == MoveTrack;
    }

    virtual void addSpecificMenu(QMenu* _cm, bool _enabled) = 0;

  public slots:
    virtual bool close();
    virtual void update();

  protected:
    TrackView(Track* _track, TrackContainerView* tcv);
    virtual ~TrackView();

    virtual void modelChanged();

    virtual void saveSettings(QDomDocument& doc, QDomElement& element)
    {
        Q_UNUSED(doc)
        Q_UNUSED(element)
    }

    virtual void loadSettings(const QDomElement& element)
    {
        Q_UNUSED(element)
    }

    virtual QString nodeName() const
    {
        return "trackview";
    }

    void dragEnterEvent(QDragEnterEvent* dee) override;
    void dropEvent(QDropEvent* de) override;
    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;
    void paintEvent(QPaintEvent* pe) override;
    void resizeEvent(QResizeEvent* re) override;

  private:
    enum Actions
    {
        NoAction,
        MoveTrack,
        ResizeTrack
    };

    Track*              m_track;
    TrackContainerView* m_trackContainerView;

    TrackOperationsWidget m_trackOperationsWidget;
    QWidget               m_trackSettingsWidget;
    TrackContentWidget    m_trackContentWidget;

    Actions m_action;

    friend class TrackLabelButton;

  private slots:
    virtual void createTCOView(Tile* tco) final;
};

#endif
