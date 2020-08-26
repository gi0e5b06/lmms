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

#ifndef BB_TRACK_H
#define BB_TRACK_H

#include "Bitset.h"
#include "Track.h"
#include "TrackView.h"

#include <QAction>
#include <QMap>
#include <QObject>
#include <QStaticText>

class TrackLabelButton;
class TrackContainer;

class BBTCO : public Tile
{
    Q_OBJECT

  public:
    BBTCO(Track* _track);
    BBTCO(const BBTCO& _other);
    virtual ~BBTCO();

    int  playBBTrackIndex() const;
    void setPlayBBTrackIndex(int _index);

    bool    isEmpty() const override;
    QString defaultName() const override;
    tick_t  unitLength() const override;
    void    rotate(tick_t _ticks) override;
    void    splitEvery(tick_t _ticks) override;
    void    splitAt(tick_t _tick) override;

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "bbtco";
    }

    virtual const Bitset* mask() const
    {
        return m_mask;
    }

    virtual TileView* createView(TrackView* _tv);

  public slots:
    void clear() override;
    void flipHorizontally() override;
    void flipVertically() override;

  protected:
    virtual Bitset* mask()
    {
        return m_mask;
    }

  private:
    int     m_playBBTrackIndex;
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

    QString defaultName() const override;

    bool play(const MidiTime& _start,
              const fpp_t     _frames,
              const f_cnt_t   _frame_base,
              int             _tco_num = -1) override;

    TrackView* createView(TrackContainerView* tcv) override;
    Tile*      createTCO() override;

    INLINE QString nodeName() const override
    {
        return "bbtrack";
    }

    void saveTrackSpecificSettings(QDomDocument& _doc,
                                   QDomElement&  _parent) override;
    void loadTrackSpecificSettings(const QDomElement& _this) override;

    // index()
    int  ownBBTrackIndex() const;
    void setOwnBBTrackIndex(int _bb);

    /*
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
    */

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

    // obsolete
    static BBTrack* findBBTrack(int _bb);
    // static void swapBBTracks(Track* _track1, Track* _track2);

  private:
    int m_ownBBTrackIndex;

    // Tracks m_disabledTracks;

    /*
    typedef QMap<QPointer<const BBTrack>, int> InfoMap;
    static InfoMap                             s_infoMap;
    */

    // static QColor * s_lastTCOColor;

    friend class BBTrackView;
};

typedef QVector<QPointer<BBTrack>> BBTracks;

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
    QPointer<BBTrack> m_bbTrack;
    // TrackLabelButton* m_trackLabel;
};

#endif
