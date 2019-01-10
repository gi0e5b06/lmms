/*
 * Track.h - declaration of classes concerning tracks -> necessary for all
 *           track-like objects (beat/bassline, sample-track...)
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

#ifndef TRACK_H
#define TRACK_H

//#include <QVector>
//#include <QList>
#include <QWidget>
//#include <QSignalMapper>
#include "lmms_basics.h"

#include <QColor>
#include <QMimeData>
//#include "MidiTime.h"
#include "Rubberband.h"
//#include "JournallingObject.h"
#include "AutomatableModel.h"
#include "DataFile.h"
#include "ModelView.h"

class QMenu;
class QPushButton;

class PixmapButton;
class TextFloat;
class Track;
class TrackContentObjectView;
class TrackContainer;
class TrackContainerView;
class TrackContentWidget;
class TrackView;

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

class TrackContentObject : public Model, public JournallingObject
{
    Q_OBJECT
    MM_OPERATORS

    mapPropertyFromModel(bool, isMuted, setMuted, m_mutedModel);
    mapPropertyFromModel(bool, isSolo, setSolo, m_soloModel);

  public:
    TrackContentObject(Track* track);
    TrackContentObject(const TrackContentObject& _other);
    virtual ~TrackContentObject();

    virtual void saveSettings(QDomDocument& doc, QDomElement& element);
    virtual void loadSettings(const QDomElement& element);

    inline Track* getTrack() const
    {
        return m_track;
    }

    virtual bool    isEmpty() const = 0;
    virtual QString defaultName() const;

    inline const QString& name() const
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
        const int sp = m_startPosition;
        return sp + m_length;
    }

    inline const MidiTime& length() const
    {
        return m_length;
    }

    inline const bool autoResize() const
    {
        return m_autoResize;
    }

    inline void setAutoResize(const bool r)
    {
        m_autoResize = r;
    }

    inline const bool autoRepeat() const
    {
        return m_autoRepeat;
    }

    inline void setAutoRepeat(const bool r)
    {
        m_autoRepeat = r;
    }

    virtual void movePosition(const MidiTime& pos);
    virtual void changeLength(const MidiTime& length);
    virtual void updateLength();
    virtual void rotate(tick_t _ticks)
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

    virtual TrackContentObjectView* createView(TrackView* tv) = 0;

    inline void selectViewOnCreate(bool select)
    {
        m_selectViewOnCreate = select;
    }

    inline bool getSelectViewOnCreate()
    {
        return m_selectViewOnCreate;
    }

    /// Returns true if and only if a->startPosition() < b->startPosition()
    static bool comparePosition(const TrackContentObject* a,
                                const TrackContentObject* b);

  public slots:
    virtual void clear();
    virtual void copy();
    virtual void paste();
    virtual void toggleMute() final;

  signals:
    void lengthChanged();
    void positionChanged();
    void destroyedTCO();

  protected:
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

  private:
    /*
    enum Actions
    {
        NoAction,
        Move,
        Resize
    };
    */

    Track*  m_track;
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

    friend class TrackContentObjectView;
};

class TrackContentObjectView : public SelectableObject, public ModelView
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
    TrackContentObjectView(TrackContentObject* tco, TrackView* tv);
    virtual ~TrackContentObjectView();

    // bool fixedTCOs();

    inline TrackContentObject* getTrackContentObject()
    {
        return m_tco;
    }

    bool   isFixed() const;
    real_t pixelsPerTact();
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

    DataFile createTCODataFiles(
            const QVector<TrackContentObjectView*>& tcos) const;

    virtual void paintTextLabel(const QString& text,
                                const QColor&  bg,
                                QPainter&      painter) final;
    virtual void paintTileBorder(const bool    current,
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

    TrackContentObject* m_tco;
    TrackView*          m_trackView;
    Actions             m_action;
    QPoint              m_initialMousePos;
    QPoint              m_initialMouseGlobalPos;

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

    bool        m_needsUpdate;
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
    real_t pixelsPerTact();

    /*! \brief Updates the background tile pixmap. */
    void updateBackground();

    void addTCOView(TrackContentObjectView* tcov);
    void removeTCOView(TrackContentObjectView* tcov);
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

    virtual void paintGrid(QPainter&                   p,
                           int                         tact,
                           real_t                      ppt,
                           QVector<QPointer<BarView>>& barViews);
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
    Track*   getTrack();
    MidiTime getPosition(int mouseX);

    TrackView* m_trackView;

    typedef QVector<TrackContentObjectView*> tcoViewVector;
    tcoViewVector                            m_tcoViews;

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
    ~TrackOperationsWidget();

  protected:
    virtual void addNameMenu(QMenu* _cm, bool _enabled) final;
    virtual void addColorMenu(QMenu* _cm, bool _enabled) final;

    virtual void mousePressEvent(QMouseEvent* me);
    virtual void paintEvent(QPaintEvent* pe);

  private slots:
    void cloneTrack();
    void splitTrack();
    void isolateTrack();
    void removeTrack();
    void updateMenu();
    void recordingOn();
    void recordingOff();
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
    typedef QVector<TrackContentObject*> tcoVector;

    enum TrackTypes
    {
        InstrumentTrack,
        BBTrack,
        SampleTrack,
        EventTrack,
        VideoTrack,
        AutomationTrack,
        HiddenAutomationTrack,
        NumTrackTypes
    };

    Track(TrackTypes type, TrackContainer* tc);
    virtual ~Track();

    bool   isFixed() const;
    bool   useStyleColor() const;
    void   setUseStyleColor(bool _b);
    QColor color() const;
    void   setColor(const QColor& _newColor);

    static Track* create(TrackTypes tt, TrackContainer* tc);
    static Track* create(const QDomElement& element, TrackContainer* tc);
    Track*        clone();

    // pure virtual functions
    TrackTypes type() const
    {
        return m_type;
    }

    virtual bool play(const MidiTime& start,
                      const fpp_t     frames,
                      const f_cnt_t   frameBase,
                      int             tcoNum = -1)
            = 0;

    virtual TrackView*          createView(TrackContainerView* view) = 0;
    virtual TrackContentObject* createTCO(const MidiTime& pos)       = 0;

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

    // -- for usage by TrackContentObject only ---------------
    TrackContentObject* addTCO(TrackContentObject* tco);
    void                removeTCO(TrackContentObject* tco);
    // -------------------------------------------------------
    void deleteTCOs();

    int                 numOfTCOs() const;
    TrackContentObject* getTCO(int tcoNum) const;
    int                 getTCONum(const TrackContentObject* tco) const;

    const tcoVector& getTCOs() const
    {
        return m_trackContentObjects;
    }
    void getTCOsInRange(tcoVector&      tcoV,
                        const MidiTime& start,
                        const MidiTime& end) const;
    void swapPositionOfTCOs(int tcoNum1, int tcoNum2);

    void createTCOsForBB(int bb);

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

    using Model::dataChanged;

    inline int getHeight()
    {
        return m_height >= MINIMAL_TRACK_HEIGHT ? m_height
                                                : DEFAULT_TRACK_HEIGHT;
    }
    inline void setHeight(int height)
    {
        m_height = height;
    }

    void lock()
    {
        m_processingLock.lock();
    }
    void unlock()
    {
        m_processingLock.unlock();
    }
    bool tryLock()
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

  public slots:
    virtual void setName(const QString& newName)
    {
        m_name = newName;
        emit nameChanged();
    }

    virtual void toggleSolo();
    virtual void toggleFrozen();

  protected:
    BoolModel m_frozenModel;
    BoolModel m_clippingModel;
    BoolModel m_mutedModel;
    BoolModel m_soloModel;

  private:
    TrackContainer* m_trackContainer;
    TrackTypes      m_type;
    QString         m_name;
    int             m_height;
    QColor          m_color;
    bool            m_useStyleColor;

    bool m_mutedBeforeSolo;
    bool m_simpleSerializingMode;

    tcoVector m_trackContentObjects;

    QMutex m_processingLock;

    friend class TrackView;

  signals:
    void destroyedTrack();
    void nameChanged();
    void trackContentObjectAdded(TrackContentObject*);
};

class TrackView : public QWidget, public ModelView, public JournallingObject
{
    Q_OBJECT
  public:
    TrackView(Track* _track, TrackContainerView* tcv);
    virtual ~TrackView();

    bool   isFixed() const;
    real_t pixelsPerTact();

    inline const Track* getTrack() const
    {
        return m_track;
    }

    inline Track* getTrack()
    {
        return m_track;
    }

    inline TrackContainerView* trackContainerView()
    {
        return m_trackContainerView;
    }

    inline TrackOperationsWidget* getTrackOperationsWidget()
    {
        return &m_trackOperationsWidget;
    }

    inline QWidget* getTrackSettingsWidget()
    {
        return &m_trackSettingsWidget;
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
    void createTCOView(TrackContentObject* tco);
};

#endif
