/*
 * TrackContainerView.h - view-component for TrackContainer
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef TRACK_CONTAINER_VIEW_H_
#define TRACK_CONTAINER_VIEW_H_

#include "ActionUpdatable.h"
#include "InstrumentTrack.h"
#include "JournallingObject.h"

//#include <QVector>
#include <QScrollArea>
#include <QThread>
#include <QWidget>

class QVBoxLayout;
class TrackContainer;

class TrackContainerView :
      public QWidget,
      public ModelView,
      public JournallingObject,
      public SerializingObjectHook,
      public virtual ActionUpdatable
{
    Q_OBJECT

  public:
    virtual QScrollArea* contentWidget() final
    {
        return m_scrollArea;
    }

    INLINE virtual const MidiTime& currentPosition() const final
    {
        return m_currentPosition;
    }

    virtual bool fixedTCOs() const
    {
        return false;
    }

    virtual real_t pixelsPerTact() const;
    virtual void   setPixelsPerTact(real_t _ppt) final;

    virtual const TrackView* trackViewAt(const int _y) const final;

    virtual bool allowRubberband() const;

    INLINE virtual bool rubberBandActive() const final
    {
        return m_rubberBand->isEnabled() && m_rubberBand->isVisible();
    }

    INLINE virtual SelectableObjects selectedObjects() const final
    {
        return m_rubberBand->selectedObjects();
    }

    virtual Tiles     selectedTCOs() final;
    virtual TileViews selectedTCOViews() final;
    virtual TileViews selectedTileViewsAt(const MidiTime& _pos,
                                          bool _startExcluded = false,
                                          bool _endExcluded   = true) final;

    virtual Tiles     allTiles() final;
    virtual TileViews allTileViews() final;
    virtual TileViews allTileViewsAt(const MidiTime& _pos,
                                     bool            _startExcluded = false,
                                     bool _endExcluded = true) final;

    static TileViews filterTileViewsAt(const TileViews& _tileViews,
                                       const MidiTime&  _pos,
                                       bool _startExcluded = false,
                                       bool _endExcluded   = true);

    virtual TrackContainer* model() final
    {
        return m_tc;
    }

    virtual const TrackContainer* model() const final
    {
        return m_tc;
    }

    virtual const TrackViews& trackViews() const final;

    virtual void moveTrackView(TrackView* trackView, int indexTo) final;
    virtual void moveTrackViewUp(TrackView* trackView) final;
    virtual void moveTrackViewDown(TrackView* trackView) final;
    virtual void scrollToTrackView(TrackView* _tv) final;

    virtual void clearAllTracks() final;

    virtual QString nodeName() const final
    {
        return "trackcontainerview";
    }

    INLINE virtual QVector<QPointer<HyperBarView>>& hyperBarViews() final
    {
        return m_hyperBarViews;
    }

    INLINE virtual QVector<QPointer<BarView>>& barViews() final
    {
        return m_barViews;
    }

    virtual RubberBand* rubberBand() const final;

  public slots:
    virtual void realignTracks() final;
    virtual void updateBackgrounds() final;

    virtual TrackView* createTrackView(Track* _t) final;
    virtual void       deleteTrackView(TrackView* _tv) final;

    ///
    /// \brief selectRegionFromPixels
    /// \param x
    /// \param y
    /// Use the rubber band to select TCO from all tracks using x, y pixels
    virtual void selectRegionFromPixels(int xStart, int xEnd) final;

    ///
    /// \brief stopRubberBand
    /// Removes the rubber band from display when finished with.
    virtual void stopRubberBand() final;

    // -- for usage by trackView only -----------------
    virtual void addTrackView(TrackView* _tv) final;
    virtual void removeTrackView(TrackView* _tv) final;
    // ------------------------------------------------

  protected:
    TrackContainerView(TrackContainer* tc);
    virtual ~TrackContainerView();

    void saveSettings(QDomDocument& _doc, QDomElement& _this) override;
    void loadSettings(const QDomElement& _this) override;

    void dropEvent(QDropEvent* _de) override;
    void dragEnterEvent(QDragEnterEvent* _dee) override;
    void mousePressEvent(QMouseEvent* _me) override;
    void mouseMoveEvent(QMouseEvent* _me) override;
    void mouseReleaseEvent(QMouseEvent* _me) override;
    void resizeEvent(QResizeEvent*) override;

    virtual void computeHyperBarViews() final;
    virtual void computeBarViews() final;

    MidiTime m_currentPosition;
    real_t   m_ppt;

    int vsbWidth()
    {
        return m_scrollArea->verticalScrollBar()->width();
    }

    int hsbHeight()
    {
        return m_scrollArea->horizontalScrollBar()->height();
    }

  private:
    enum Actions
    {
        AddTrack,
        RemoveTrack
    };

    class scrollArea : public QScrollArea
    {
      public:
        scrollArea(TrackContainerView* parent);
        virtual ~scrollArea();

      protected:
        virtual void wheelEvent(QWheelEvent* _we);

      private:
        TrackContainerView* m_trackContainerView;
    };

    TrackContainer* m_tc;
    TrackViews      m_trackViews;

    scrollArea*  m_scrollArea;
    QVBoxLayout* m_scrollLayout;

    RubberBand* m_rubberBand;
    QPoint      m_origin;

    QVector<QPointer<HyperBarView>> m_hyperBarViews;
    QVector<QPointer<BarView>>      m_barViews;

  signals:
    void positionChanged(const MidiTime& _pos);
};

class InstrumentLoaderThread : public QThread
{
    Q_OBJECT
  public:
    InstrumentLoaderThread(QObject*         parent = 0,
                           InstrumentTrack* it     = 0,
                           QString          name   = "");

    void run();

  private:
    InstrumentTrack* m_it;
    QString          m_name;
    QThread*         m_containerThread;
};

#endif
