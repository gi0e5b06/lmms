/*
 * BBTrack.h - class BBTrack, a wrapper for using bbEditor
 *              (which is a singleton-class) as track
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

#ifndef BB_TRACK_H_
#define BB_TRACK_H_

#include "Bitset.h"
#include "Track.h"

#include <QAction>
#include <QMap>
#include <QObject>
#include <QStaticText>

class TrackLabelButton;
class TrackContainer;

class BBTCO : public Tile
{
  public:
    BBTCO(Track* _track);
    BBTCO(const BBTCO& _other);
    virtual ~BBTCO();

    int  bbTrackIndex() const;
    void setBBTrackIndex(int _index);

    virtual bool    isEmpty() const;
    virtual QString defaultName() const;
    virtual tick_t  unitLength() const;

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    inline virtual QString nodeName() const
    {
        return "bbtco";
    }

    virtual const Bitset* mask() const
    {
        return m_mask;
    }

    virtual TileView* createView(TrackView* _tv);

    virtual void clear();

  protected:
    virtual Bitset* mask()
    {
        return m_mask;
    }

  private:
    int     m_bbTrackIndex;
    Bitset* m_mask;

    friend class BBTCOView;
};

class BBTCOView : public TileView
{
    Q_OBJECT
  public:
    // BBTCOView( Tile * _tco, TrackView * _tv );
    BBTCOView(BBTCO* _tco, TrackView* _tv);
    virtual ~BBTCOView();

    /*
    QColor color() const
    {
            return( m_bbTCO->m_color );
    }
    void setColor( QColor _new_color );
    */

  public slots:
    virtual void update();

  protected slots:
    void openInBBEditor();
    // void resetName();
    // void changeName();
    // void changeColor();
    // void resetColor();
    void toggleMask(QAction* _action);
    void chooseBeat(QAction* _action);

  protected:
    virtual QMenu* buildContextMenu();
    virtual void   addMuteMenu(QMenu* _cm, bool _enabled);
    virtual void   addBeatMenu(QMenu* _cm, bool _enabled);

    virtual void paintEvent(QPaintEvent* pe);
    virtual void mouseDoubleClickEvent(QMouseEvent* _me);

  private:
    BBTCO*  m_bbTCO;
    QPixmap m_paintPixmap;

    QStaticText m_staticTextName;
};

class EXPORT BBTrack : public Track
{
    Q_OBJECT

  public:
    BBTrack(TrackContainer* tc);
    virtual ~BBTrack();

    virtual QString defaultName() const;

    virtual bool play(const MidiTime& _start,
                      const fpp_t     _frames,
                      const f_cnt_t   _frame_base,
                      int             _tco_num = -1);

    virtual TrackView* createView(TrackContainerView* tcv);
    virtual Tile*      createTCO(const MidiTime& _pos);

    virtual void saveTrackSpecificSettings(QDomDocument& _doc,
                                           QDomElement&  _parent);
    virtual void loadTrackSpecificSettings(const QDomElement& _this);

    int index() const
    {
        return s_infoMap.value(this);
    }

    bool automationDisabled(Track* _track)
    {
        return m_disabledTracks.contains(_track);
    }
    void disableAutomation(Track* _track)
    {
        m_disabledTracks.append(_track);
    }
    void enableAutomation(Track* _track)
    {
        m_disabledTracks.removeAll(_track);
    }

    /*
    static void setLastTCOColor( const QColor & c )
    {
            if( ! s_lastTCOColor )
            {
                    s_lastTCOColor = new QColor( c );
            }
            else
            {
                    *s_lastTCOColor = QColor( c );
            }
    }

    static void clearLastTCOColor()
    {
            if( s_lastTCOColor )
            {
                    delete s_lastTCOColor;
            }
            s_lastTCOColor = nullptr;
    }
    */

    static BBTrack* findBBTrack(int _bb_num);
    static void     swapBBTracks(Track* _track1, Track* _track2);

  protected:
    inline virtual QString nodeName() const
    {
        return "bbtrack";
    }

  private:
    QList<Track*> m_disabledTracks;

    typedef QMap<const BBTrack*, int> infoMap;
    static infoMap                    s_infoMap;

    // static QColor * s_lastTCOColor;

    friend class BBTrackView;
};

class BBTrackView : public TrackView
{
    Q_OBJECT

  public:
    BBTrackView(BBTrack* bbt, TrackContainerView* tcv);
    virtual ~BBTrackView();

    virtual bool close();

    /*
    const BBTrack* bbTrack() const
    {
        return m_bbTrack;
    }
    */

    virtual void addSpecificMenu(QMenu* _cm, bool _enabled);

  public slots:
    void clickedTrackLabel();

  private:
    BBTrack* m_bbTrack;
    // TrackLabelButton* m_trackLabel;
};

#endif
