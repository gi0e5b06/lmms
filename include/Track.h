/*
 * Track.h - declaration of classes concerning tracks -> necessary for all
 *           track-like objects (beat/bassline, sample-track...)
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

#ifndef TRACK_H
#define TRACK_H

//#include "MidiTime.h"
#include "Rubberband.h"
//#include "JournallingObject.h"
#include "AutomatableModel.h"
#include "DataFile.h"
#include "ModelView.h"
#include "Mutex.h"
#include "TimeLineWidget.h"
#include "lmms_basics.h"

//#include <QVector>
//#include <QList>
#include <QMenu>
#include <QWidget>
//#include <QSignalMapper>
#include <QColor>
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

typedef QVector<Tile*>  Tiles;
typedef QVector<Track*> TrackList;
typedef QVector<Track*> Tracks;

const int DEFAULT_SETTINGS_WIDGET_WIDTH = 224;
const int TRACK_OP_WIDTH                = 78;
// This shaves 150-ish pixels off track buttons,
// ruled from config: ui.compacttrackbuttons
const int DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT = 96;
const int TRACK_OP_WIDTH_COMPACT                = 62;

/*! The minimum track height in pixels
 *
 * Tracks can be resized by shift-dragging anywhere inside the track
 * display.  This sets the minimum size in pixels for a track.
 */
const int MINIMAL_TRACK_HEIGHT = 32;
const int DEFAULT_TRACK_HEIGHT = 32;

const int TCO_BORDER_WIDTH = 1;  // 2

class Tile : public Model, public JournallingObject
{
    Q_OBJECT
    MM_OPERATORS

    mapPropertyFromModel(bool, isMuted, setMuted, m_mutedModel);

  public:
    virtual ~Tile();

    virtual void saveSettings(QDomDocument& doc, QDomElement& element);
    virtual void loadSettings(const QDomElement& element);

    inline Track* track() const
    {
        return m_track;
    }

    virtual bool    isEmpty() const = 0;
    virtual QString defaultName() const;
    virtual tick_t  unitLength() const = 0;

    virtual const QString name() const
    {
        return m_name;
    }

    inline void setName(const QString& name)
    {
        m_name = name;
        emit dataChanged();
    }

    virtual QString displayName() const
    {
        return name();
    }

    inline const MidiTime& startPosition() const
    {
        return m_startPosition;
    }

    inline MidiTime endPosition() const
    {
        return m_startPosition + m_length;
    }

    inline const MidiTime& length() const
    {
        return m_length;
    }

    inline const bool autoResize() const
    {
        return m_autoResize;
    }

    /*inline*/ void setAutoResize(const bool r)
    {
        if(r != m_autoResize)
        {
            m_autoResize = r;
            if(r)
                updateLength();
            emit dataChanged();
        }
    }

    inline const bool autoRepeat() const
    {
        return m_autoRepeat;
    }

    /*inline*/ void setAutoRepeat(const bool r)
    {
        if(r != m_autoRepeat)
        {
            m_autoRepeat = r;
            emit dataChanged();
        }
    }

    virtual void movePosition(const MidiTime& pos);
    virtual void changeLength(const MidiTime& length);
    virtual void updateLength();

    virtual void resizeLeft(const MidiTime& pos, const MidiTime& len);
    virtual void resizeRight(const MidiTime& pos, const MidiTime& len);

    virtual void rotate(tick_t _ticks)
    {
    }  //= 0;

    virtual void split(tick_t _ticks)
    {
    }  //= 0;

    /*
    unsigned int colorRGB() const
    {
            return m_color.rgb();
    }
    */

    virtual int stepResolution() const;

    bool   isFixed() const;
    bool   useStyleColor() const;
    void   setUseStyleColor(bool _b);
    QColor color() const;
    void   setColor(const QColor& _c);

    virtual TileView* createView(TrackView* tv) = 0;

    inline void selectViewOnCreate(bool select)
    {
        m_selectViewOnCreate = select;
    }

    inline bool getSelectViewOnCreate()
    {
        return m_selectViewOnCreate;
    }

    /// Returns true if and only if a->startPosition() < b->startPosition()
    /*
    static bool comparePosition(const Tile* a,
                                const Tile* b);
    */
    static bool lessThan(const Tile* a, const Tile* b)
    {
        const tick_t pa = a->startPosition().ticks();
        const tick_t pb = b->startPosition().ticks();
        if(pa != pb)
            return pa < pb;
        const tick_t la = a->length().ticks();
        const tick_t lb = b->length().ticks();
        if(la != lb)
            return la < lb;
        const tick_t ua = a->unitLength();
        const tick_t ub = b->unitLength();
        if(ua != ub)
            return ua < ub;
        return a < b;
    }

  public slots:
    virtual void clear();
    virtual void copy();
    virtual void paste();
    virtual void toggleMute() final;

    virtual void flipHorizontally()
    {
    }  //= 0;
    virtual void flipVertically()
    {
    }  //= 0;

  signals:
    void lengthChanged();
    void positionChanged();
    void destroyedTCO();

  protected:
    Tile(Track* track, const QString& _displayName);
    Tile(const Tile& _other);

    void     updateBBTrack();
    void     setStepResolution(int _res);
    int      stepsPerTact() const;
    MidiTime stepPosition(int _step) const;
    void     updateLength(tick_t _len);

    int m_steps;
    int m_stepResolution;

  protected slots:
    void addBarSteps();
    void addBeatSteps();
    void addOneStep();
    void removeBarSteps();
    void removeBeatSteps();
    void removeOneStep();
    void rotateOneBarLeft();
    void rotateOneBeatLeft();
    void rotateOneStepLeft();
    void rotateOneBarRight();
    void rotateOneBeatRight();
    void rotateOneStepRight();
    void splitAfterEveryBar();
    void splitAfterEveryFourthBar();

  protected:
    Track* m_track;

  private:
    /*
    enum Actions
    {
        NoAction,
        Move,
        Resize
    };
    */

    QString m_name;

    MidiTime m_startPosition;
    MidiTime m_length;

    BoolModel m_mutedModel;
    BoolModel m_soloModel;

    bool   m_autoResize;
    bool   m_autoRepeat;
    QColor m_color;
    bool   m_useStyleColor;

    bool m_selectViewOnCreate;

    friend class TileView;
};

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
    TileView(Tile* tco, TrackView* tv);
    virtual ~TileView();

    // bool fixedTCOs();

    inline Tile* getTile()
    {
        return m_tco;
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
    inline QPixmap grab(const QRect& rectangle
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

    virtual void dragEnterEvent(QDragEnterEvent* dee);
    // virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent* de);
    virtual void leaveEvent(QEvent* e);
    virtual void mousePressEvent(QMouseEvent* me);
    virtual void mouseMoveEvent(QMouseEvent* me);
    virtual void mouseReleaseEvent(QMouseEvent* me);
    virtual void resizeEvent(QResizeEvent* re)
    {
        m_needsUpdate = true;
        SelectableObject::resizeEvent(re);
    }

    inline TrackView* getTrackView()
    {
        return m_trackView;
    }

    DataFile createTCODataFiles(const QVector<TileView*>& tcos) const;

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

    Tile*      m_tco;
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

    inline void setInitialMousePos(QPoint pos)
    {
        m_initialMousePos       = pos;
        m_initialMouseGlobalPos = mapToGlobal(pos);
    }

    bool mouseMovedDistance(QMouseEvent* me, int distance);
};

class HyperBarView : public QObject
{
  public:
    HyperBarView(int length, const QColor& color, const QString& label);
    // HyperBarView(HyperBarView& hbv);
    inline const int length() const
    {
        return m_length;
    }
    inline const QColor& color() const
    {
        return m_color;
    }
    inline const QString& label() const
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
    // inline HyperBarView hyperbar() { return m_hbv; }
    inline const Types type() const
    {
        return m_type;
    }
    inline const bool sign() const
    {
        return m_sign;
    }
    inline const QColor color() const
    {
        return m_hbv->color();
    }

  private:
    // BarView();
    const QPointer<HyperBarView> m_hbv;
    const Types                  m_type;
    const bool                   m_sign;
};

class TrackContentWidget : public QWidget, public JournallingObject
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
        if(tcoNum >= 0 && tcoNum < m_tcoViews.size())
        {
            removeTCOView(m_tcoViews[tcoNum]);
        }
    }

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

    typedef QVector<TileView*> tcoViewVector;
    tcoViewVector              m_tcoViews;

    QPixmap m_background;

    // qproperty fields
    QBrush m_darkerColor;
    QBrush m_lighterColor;
    QBrush m_gridColor;
    QBrush m_embossColor;
};

class TrackOperationsWidget : public QWidget
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

// base-class for all tracks
class EXPORT Track : public Model, public JournallingObject
{
    Q_OBJECT
    MM_OPERATORS

    mapPropertyFromModel(bool, isMuted, setMuted, m_mutedModel);
    mapPropertyFromModel(bool, isSolo, setSolo, m_soloModel);
    mapPropertyFromModel(bool, isFrozen, setFrozen, m_frozenModel);
    mapPropertyFromModel(bool, isClipping, setClipping, m_clippingModel);

  public:
    // typedef QVector<Tile*> tco Vector;
    // typedef Tiles tco Vector;

    enum TrackType
    {
        InstrumentTrack,
        BBTrack,
        SampleTrack,
        EventTrack,  // not used but must be kept
        VideoTrack,  // not used but must be kept
        AutomationTrack,
        HiddenAutomationTrack,
    };
    // NumTrackType

    Track(TrackType type, TrackContainer* tc);
    virtual ~Track();

    virtual QString objectName() const;

    bool   isFixed() const;
    bool   useStyleColor() const;
    void   setUseStyleColor(bool _b);
    QColor color() const;
    void   setColor(const QColor& _newColor);

    int  currentLoop() const;
    void setCurrentLoop(int _loop);

    // void selectSubloop(const MidiTime& _pos);

    static Track* create(TrackType tt, TrackContainer* tc);
    static Track* create(const QDomElement& element, TrackContainer* tc);
    Track*        clone();

    virtual TrackType type() const final
    {
        return m_type;
    }

    virtual bool play(const MidiTime& start,
                      const fpp_t     frames,
                      const f_cnt_t   frameBase,
                      int             tcoNum = -1)
            = 0;

    virtual TrackView* createView(TrackContainerView* view) = 0;
    virtual Tile*      createTCO(const MidiTime& pos)       = 0;

    virtual void saveTrackSpecificSettings(QDomDocument& doc,
                                           QDomElement&  parent)
            = 0;
    virtual void loadTrackSpecificSettings(const QDomElement& element) = 0;

    virtual void saveSettings(QDomDocument& doc, QDomElement& element);
    virtual void loadSettings(const QDomElement& element);

    void setSimpleSerializing()
    {
        m_simpleSerializingMode = true;
    }

    // -- for usage by Tile only ---------------
    Tile* addTCO(Tile* tco);
    void  removeTCO(Tile* tco);
    // -------------------------------------------------------
    void deleteTCOs();

    int   numOfTCOs() const;
    Tile* getTCO(int tcoNum) const;
    int   getTCONum(const Tile* tco) const;

    const Tiles& getTCOs() const
    {
        return m_tiles;
    }
    void getTCOsInRange(Tiles&          tcoV,
                        const MidiTime& start,
                        const MidiTime& end) const;
    void swapPositionOfTCOs(int tcoNum1, int tcoNum2);

    void createTCOsForBB(int bb);
    void deleteUnusedTCOsForBB();
    void fixOverlappingTCOs();

    void insertTact(const MidiTime& pos);
    void removeTact(const MidiTime& pos);

    tact_t length() const;

    inline TrackContainer* trackContainer() const
    {
        return m_trackContainer;
    }

    // name-stuff
    virtual const QString& name() const
    {
        return m_name;
    }

    virtual QString displayName() const
    {
        return name();
    }

    virtual QString defaultName() const = 0;

    virtual int height() final
    {
        return m_height >= MINIMAL_TRACK_HEIGHT ? m_height
                                                : DEFAULT_TRACK_HEIGHT;
    }

    virtual void setHeight(int _height) final
    {
        m_height = _height;
    }

    void lockTrack()
    {
        m_processingLock.lock();
    }

    void unlockTrack()
    {
        m_processingLock.unlock();
    }

    bool tryLockTrack()
    {
        return m_processingLock.tryLock();
    }

    inline const BoolModel* clippingModel() const
    {
        return &m_clippingModel;
    }

    inline const BoolModel* frozenModel() const
    {
        return &m_frozenModel;
    }

    inline const BoolModel* mutedModel() const
    {
        return &m_mutedModel;
    }

    virtual void cleanFrozenBuffer();
    virtual void readFrozenBuffer();
    virtual void writeFrozenBuffer();

    void clearAllTrackPlayHandles();

    // signals:
    // using Model::dataChanged(); <-- not working
    // void dataChanged(); <-- not needed

  public slots:
    virtual void setName(const QString& newName)
    {
        m_name = newName;
        emit nameChanged();
    }

    virtual void toggleSolo();
    virtual void toggleFrozen();

  private:
    virtual int trackIndex() const final;

  protected:
    BoolModel m_frozenModel;
    BoolModel m_clippingModel;
    BoolModel m_mutedModel;
    BoolModel m_soloModel;
    BoolModel m_loopEnabledModel;
    IntModel  m_currentLoopModel;

  private:
    TrackContainer* m_trackContainer;
    TrackType       m_type;
    QString         m_name;
    int             m_height;
    QColor          m_color;
    bool            m_useStyleColor;

    bool m_mutedBeforeSolo;
    bool m_simpleSerializingMode;

    Tiles m_tiles;

    Mutex m_processingLock;

    friend class TrackView;

  signals:
    void destroyedTrack();
    void nameChanged();
    void tileAdded(Tile*);
};

class TrackView : public QWidget, public ModelView, public JournallingObject
{
    Q_OBJECT

  public:
    TrackView(Track* _track, TrackContainerView* tcv);
    virtual ~TrackView();

    virtual QColor cableColor() const;

    bool   isFixed() const;
    real_t pixelsPerTact() const;

    inline const Track* track() const
    {
        return m_track;
    }

    inline Track* track()
    {
        return m_track;
    }

    inline const TrackContainerView* trackContainerView() const
    {
        return m_trackContainerView;
    }

    inline TrackContainerView* trackContainerView()
    {
        return m_trackContainerView;
    }

    inline const TrackOperationsWidget* getTrackOperationsWidget() const
    {
        return &m_trackOperationsWidget;
    }

    inline TrackOperationsWidget* getTrackOperationsWidget()
    {
        return &m_trackOperationsWidget;
    }

    inline const QWidget* getTrackSettingsWidget() const
    {
        return &m_trackSettingsWidget;
    }

    inline QWidget* getTrackSettingsWidget()
    {
        return &m_trackSettingsWidget;
    }

    inline const TrackContentWidget* getTrackContentWidget() const
    {
        return &m_trackContentWidget;
    }

    inline TrackContentWidget* getTrackContentWidget()
    {
        return &m_trackContentWidget;
    }

    bool isMovingTrack() const
    {
        return m_action == MoveTrack;
    }

    virtual void update();

    virtual void addSpecificMenu(QMenu* _cm, bool _enabled) = 0;

  public slots:
    virtual bool close();

  protected:
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

    virtual void dragEnterEvent(QDragEnterEvent* dee);
    virtual void dropEvent(QDropEvent* de);
    virtual void mousePressEvent(QMouseEvent* me);
    virtual void mouseMoveEvent(QMouseEvent* me);
    virtual void mouseReleaseEvent(QMouseEvent* me);
    virtual void paintEvent(QPaintEvent* pe);
    virtual void resizeEvent(QResizeEvent* re);

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
    void createTCOView(Tile* tco);
};

#endif
