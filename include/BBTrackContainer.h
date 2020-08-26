/*
 * BBTrackContainer.h - model-component of BB-Editor
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

#ifndef BB_TRACK_CONTAINER_H
#define BB_TRACK_CONTAINER_H

#include "BBTrack.h"
#include "Bitset.h"
#include "ComboBoxModel.h"
#include "TrackContainer.h"

class EXPORT BBTrackContainer : public TrackContainer
{
    Q_OBJECT

  public:
    BBTrackContainer();
    virtual ~BBTrackContainer();

    virtual bool isFixed() const
    {
        return true;
    }

    // virtual bool play( MidiTime _start, const fpp_t _frames,
    //                   const f_cnt_t _frame_base, int _tco_num = -1 );
    virtual bool play(MidiTime      _start,
                      const fpp_t   _frames,
                      const f_cnt_t _frame_base,
                      int           _tco_num,
                      const Bitset* _mask);

    QString nodeName() const override
    {
        return "bbtrackcontainer";
    }

    tact_t lengthOfBB(int _bb) const;
    tact_t lengthOfCurrentBB()
    {
        return lengthOfBB(currentBB());
    }

    tick_t beatLengthOfBB(int _bb) const;

    // int numOfBBs() const;
    // void removeBB(int _bb);
    // void swapBB(int _bb1, int _bb2);

    virtual void updateAfterInnerTrackAdd(Track* _track);

    void updateBBTrack(Tile* _tco);
    void fixIncorrectPositions();
    void fixIncorrectPositions(Track* _t);
    void createTCOsForBB(int _bb);
    void createTCOsForBB(Track* _t);
    void deleteTCOsForBB(int _bb);
    void moveTCOsForBB(int _fromBB, int _toBB);
    void checkTCOs();
    void printDebug() const;
    void rearrangeBB();

    // AutomatedValueMap
    void automatedValuesAt(MidiTime           time,
                           int                tcoNum,
                           AutomatedValueMap& _map) const override;

    // BBTracks bbTracks();
    BBTracks bbTracks() const;
    int      firstAvailableBeatIndex();
    int      lastUsedBeatIndex();

    int      registerTrack(BBTrack* _bbTrack);
    void     unregisterTrack(BBTrack* _bbTrack);
    int      registryIndex(const BBTrack* _bbTrack) const;
    BBTrack* registryTrack(int _bbNum) const;
    QMenu*   createBeatMenu(bool _enabled, int _index);
    // void swapBBTracks(Track* _track1, Track* _track2);

    int            currentBB();
    void           setCurrentBB(int _bb);
    ComboBoxModel& currentBBModel();

  public slots:
    void play();
    void stop();
    void updateComboBox();
    // void currentBBChanged();

  private:
    ComboBoxModel m_currentBB;

    friend class BBWindow;
};

#endif
