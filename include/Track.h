/*
 * Track.h -
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
//#include "Rubberband.h"
//#include "JournallingObject.h"
//#include "AutomatableModel.h"
//#include "DataFile.h"
//#include "ModelView.h"
#include "Mutex.h"
//#include "SafeList.h"
#include "Tile.h"
#include "TileView.h"
//#include "TimeLineWidget.h"
//#include "lmms_basics.h"

//#include <QVector>
//#include <QList>
//#include <QMenu>
//#include <QPointer>
//#include <QWidget>
//#include <QSignalMapper>
//#include <QColor>
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

typedef QVector<QPointer<Track>> TrackList;
typedef QVector<QPointer<Track>> Tracks;

const int MINIMAL_TRACK_HEIGHT = 32;
const int DEFAULT_TRACK_HEIGHT = 32;

class EXPORT Track : public Model, public JournallingObject
{
    Q_OBJECT
    MM_OPERATORS

    mapPropertyFromModel(bool, isMuted, setMuted, m_mutedModel);
    mapPropertyFromModel(bool, isSolo, setSolo, m_soloModel);
    mapPropertyFromModel(bool, isFrozen, setFrozen, m_frozenModel);
    mapPropertyFromModel(bool, isClipping, setClipping, m_clippingModel);

  public:
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
    virtual Tile*      createTCO()                          = 0;

    virtual void saveTrackSpecificSettings(QDomDocument& doc,
                                           QDomElement&  parent)
            = 0;
    virtual void loadTrackSpecificSettings(const QDomElement& element) = 0;

    void saveSettings(QDomDocument& doc, QDomElement& element) override;
    void loadSettings(const QDomElement& element) override;

    void setSimpleSerializing()
    {
        m_simpleSerializingMode = true;
    }

    // -- for usage by Tile only ---------------
    void addTCO(Tile* tco);
    void removeTCO(Tile* tco);
    // -----------------------------------------
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

    // void swapPositionOfTCOs(int tcoNum1, int tcoNum2);
    // for BB
    Tile* tileForBB(int _bb) const;
    void createTCOsForBB(int _bb);
    void deleteTCOsForBB(int _bb);
    void moveTCOsForBB(int _fromBB, int _toBB);

    void fixOverlappingTCOs();
    void insertTact(const MidiTime& pos);
    void removeTact(const MidiTime& pos);

    tact_t length() const;

    INLINE TrackContainer* trackContainer() const
    {
        return m_trackContainer;
    }

    // name-stuff
    virtual QString name() const
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

    INLINE const BoolModel* clippingModel() const
    {
        return &m_clippingModel;
    }

    INLINE const BoolModel* frozenModel() const
    {
        return &m_frozenModel;
    }

    INLINE const BoolModel* mutedModel() const
    {
        return &m_mutedModel;
    }

    virtual void cleanFrozenBuffer();
    virtual void readFrozenBuffer();
    virtual void writeFrozenBuffer();

    void clearAllTrackPlayHandles();

    virtual void rearrangeAllTiles() final;

    // signals:
    // using Model::dataChanged(); <-- not working
    // void dataChanged(); <-- not needed

  public slots:
    virtual void setName(const QString& _newName)
    {
        m_name = _newName;
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
    void iconChanged();
    void nameChanged();
    void tileAdded(Tile*);
};

#endif
