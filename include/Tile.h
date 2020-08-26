/*
 * Tile.h -
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

#ifndef TILE_H
#define TILE_H

//#include "MidiTime.h"
//#include "Rubberband.h"
//#include "JournallingObject.h"
#include "AutomatableModel.h"
//#include "DataFile.h"
//#include "ModelView.h"
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

typedef QVector<QPointer<Tile>> Tiles;

// const int TCO_BORDER_WIDTH = 1;  // 2

class Tile : public Model, public JournallingObject
{
    Q_OBJECT
    MM_OPERATORS

    mapPropertyFromModel(bool, isMuted, setMuted, m_mutedModel);

  public:
    virtual ~Tile();

    void saveSettings(QDomDocument& doc, QDomElement& element) override;
    void loadSettings(const QDomElement& element) override;

    INLINE Track* track() const
    {
        return m_track;
    }

    virtual bool    isEmpty() const = 0;
    virtual QString defaultName() const;
    virtual tick_t  unitLength() const = 0;

    virtual QString name() const
    {
        return m_name;
    }

    INLINE void setName(const QString& name)
    {
        m_name = name;
        emit dataChanged();
    }

    virtual QString displayName() const
    {
        return name();
    }

    INLINE const MidiTime& startPosition() const
    {
        return m_startPosition;
    }

    INLINE MidiTime endPosition() const
    {
        return m_startPosition + m_length;
    }

    INLINE const MidiTime& length() const
    {
        return m_length;
    }

    INLINE const bool autoResize() const
    {
        return m_autoResize;
    }

    void setAutoResize(const bool r)
    {
        if(r != m_autoResize)
        {
            m_autoResize = r;
            if(r)
                updateLength();
            emit dataChanged();
        }
    }

    INLINE const bool autoRepeat() const
    {
        return m_autoRepeat;
    }

    void setAutoRepeat(const bool r)
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

    virtual void rotate(tick_t _ticks)     = 0;
    virtual void splitEvery(tick_t _ticks) = 0;
    virtual void splitAt(tick_t _tick)     = 0;

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

    INLINE void selectViewOnCreate(bool select)
    {
        m_selectViewOnCreate = select;
    }

    INLINE bool getSelectViewOnCreate()
    {
        return m_selectViewOnCreate;
    }

    Tile* previousTile() const;
    Tile* nextTile() const;

    static bool lessThan(const Tile* a, const Tile* b)
    {
        const tick_t pa = a->startPosition().ticks();
        const tick_t pb = b->startPosition().ticks();
        if(pa != pb)
            return pa < pb;
        const tick_t la = a->length().ticks();
        const tick_t lb = b->length().ticks();
        if(la != lb)
            return la > lb;  // to display longer tiles under
        const tick_t ua = a->unitLength();
        const tick_t ub = b->unitLength();
        if(ua != ub)
            return ua < ub;
        return a < b;
    }

  public slots:
    virtual void clear() = 0;
    virtual void flipHorizontally() = 0;
    virtual void flipVertically()   = 0;
    virtual void copy() final;
    virtual void paste() final;
    virtual void toggleMute() final;

    virtual void cloneSteps()
    {
    }

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

  signals:
    void lengthChanged();
    void positionChanged();
    void destroyedTCO();

  protected:
    Tile(Track*         _track,
         const QString& _displayName,
         const QString& _objectName = QString::null);
    Tile(const Tile& _other);

    void     updateBBTrack();
    void     setStepResolution(int _res);
    step_t   stepsPerTact() const;
    MidiTime stepPosition(step_t _step) const;
    void     updateLength(tick_t _len);

    Tile* adjacentTileByOffset(int offset) const;

    step_t m_steps;
    int    m_stepResolution;

  protected:
    QPointer<Track> m_track;

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
    // BoolModel m_soloModel;

    bool   m_autoResize;
    bool   m_autoRepeat;
    QColor m_color;
    bool   m_useStyleColor;

    bool m_selectViewOnCreate;

    friend class TileView;
};

#endif
