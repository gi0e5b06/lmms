/*
 * BBTrackContainer.cpp - model-component of BB-Editor
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

#include "BBTrackContainer.h"

#include "BBTrack.h"
#include "Engine.h"
#include "Pattern.h"
#include "Song.h"

BBTrackContainer::BBTrackContainer() :
      TrackContainer(nullptr, "Beats"),
      m_bbComboBoxModel(this, tr("Current track"))
{
    connect(&m_bbComboBoxModel, SIGNAL(dataChanged()), this,
            SLOT(currentBBChanged()));
    // we *always* want to receive updates even in case BB actually did
    // not change upon setCurrentBB()-call
    connect(&m_bbComboBoxModel, SIGNAL(dataUnchanged()), this,
            SLOT(currentBBChanged()));
    setType(BBContainer);
}

BBTrackContainer::~BBTrackContainer()
{
    qInfo("BBTrackContainer::~BBTrackContainer");
}

bool BBTrackContainer::play(MidiTime      _start,
                            fpp_t         _frames,
                            f_cnt_t       _offset,
                            int           _tco_num,
                            const Bitset* _mask)
{
    tact_t beatlen = lengthOfBB(_tco_num);
    if(beatlen <= 0)
        return false;

    _start = _start % (beatlen * MidiTime::ticksPerTact());

    bool   played_a_note = false;
    Tracks tl            = tracks();
    int    index         = 0;
    for(Tracks::iterator it = tl.begin(); it != tl.end(); ++it, ++index)
    {
        if(_mask != nullptr && index < _mask->size() && _mask->bit(index))
            continue;

        f_cnt_t             realstart = _start;
        TrackContentObject* p         = (*it)->getTCO(_tco_num);
        // Pattern* p=dynamic_cast<Pattern*>((*it)->getTCO( _tco_num ));
        // if(p)
        {
            tick_t tlen = p->length();
            if(tlen <= 0)
                continue;
            realstart = _start % tlen;  // GDX
            // qInfo("realstart=%d tlen=%d",realstart,tlen);
        }
        if((*it)->play(realstart, _frames, _offset, _tco_num))
        {
            played_a_note = true;
        }
    }

    return played_a_note;
}

void BBTrackContainer::updateAfterTrackAdd()
{
    if(!Engine::getSong()->isLoadingProject())
    {
        if(numOfBBs() == 0)
            Engine::getSong()->addBBTrack();
    }
}

tact_t BBTrackContainer::lengthOfBB(int _bb) const
{
    MidiTime max_length = MidiTime::ticksPerTact();

    for(const Track* t: tracks())
    {
        TrackContentObject* tco = t->getTCO(_bb);
        if(tco != nullptr)
            max_length = qMax(max_length, tco->length());
    }

    return max_length.nextFullTact();
}

tick_t BBTrackContainer::beatLengthOfBB(int _bb) const
{
    tick_t max_length = 0;
    for(const Track* t: tracks())
    {
        Pattern* p = dynamic_cast<Pattern*>(t->getTCO(_bb));
        if(p != nullptr)
        {
            tick_t plen = p->length();  // beatPatternLength();
            max_length  = qMax(max_length, plen);
        }
    }
    return max_length;
}

int BBTrackContainer::numOfBBs() const
{
    return Engine::getSong()->countTracks(Track::BBTrack);
}

void BBTrackContainer::removeBB(int _bb)
{
    /*
    Tracks tl = tracks();
    for(Tracks::iterator it = tl.begin(); it != tl.end(); ++it)
    {
        delete(*it)->getTCO(_bb);
        (*it)->removeTact(_bb * DefaultTicksPerTact);
    }
    if(_bb <= currentBB())
    {
        setCurrentBB(qMax(currentBB() - 1, 0));
    }
    */

    for(Track* t: tracks())
    {
        delete t->getTCO(_bb);
        t->removeTact(_bb * DefaultTicksPerTact);
    }

    if(_bb <= currentBB())
        setCurrentBB(qMax(currentBB() - 1, 0));
}

void BBTrackContainer::swapBB(int _bb1, int _bb2)
{
    /*
    Tracks tl = tracks();
    for(Tracks::iterator it = tl.begin(); it != tl.end(); ++it)
    {
        (*it)->swapPositionOfTCOs(_bb1, _bb2);
    }
    */

    for(Track* t: tracks())
        t->swapPositionOfTCOs(_bb1, _bb2);

    updateComboBox();
}

void BBTrackContainer::updateBBTrack(TrackContentObject* _tco)
{
    BBTrack* t = BBTrack::findBBTrack(_tco->startPosition()
                                      / DefaultTicksPerTact);
    if(t != nullptr)
        t->emit dataChanged();
}

void BBTrackContainer::fixIncorrectPositions()
{
    /*
    Tracks tl = tracks();
    for(Tracks::iterator it = tl.begin(); it != tl.end(); ++it)
    {
        for(int i = 0; i < numOfBBs(); ++i)
        {
            (*it)->getTCO(i)->movePosition(MidiTime(i, 0));
        }
    }
    */

    const int n = numOfBBs();
    for(Track* t: tracks())
        for(int i = 0; i < n; ++i)
        {
            TrackContentObject* tco = t->getTCO(i);
            tco->movePosition(MidiTime(i, 0));
            tco->setAutoResize(false);
            tco->setAutoRepeat(false);
            tco->changeLength(
                    qMin(tco->unitLength(), tco->length().getTicks()));
        }
}

void BBTrackContainer::play()
{
    if(Engine::getSong()->playMode() != Song::Mode_PlayBB)
    {
        Engine::getSong()->playBB();
    }
    else
    {
        Engine::getSong()->togglePause();
    }
}

void BBTrackContainer::stop()
{
    Engine::getSong()->stop();
}

void BBTrackContainer::updateComboBox()
{
    const int cur_bb = currentBB();

    m_bbComboBoxModel.clear();

    const int n = numOfBBs();
    for(int i = 0; i < n; ++i)
    {
        BBTrack* bbt = BBTrack::findBBTrack(i);
        m_bbComboBoxModel.addItem(bbt->name());
    }

    setCurrentBB(cur_bb);
}

void BBTrackContainer::currentBBChanged()
{
    // now update all track-labels (the current one has to become white,
    // the others gray)
    /*
    for(Track* t: Engine::getSong()->tracks())
        if(t->type() == Track::BBTrack)
            t->emit dataChanged();
    */
}

void BBTrackContainer::createTCOsForBB(int _bb)
{
    /*
    Tracks tl = tracks();
    for(int i = 0; i < tl.size(); ++i)
    {
        tl[i]->createTCOsForBB(_bb);
    }
    */

    for(Track* t: tracks())
        t->createTCOsForBB(_bb);
}

// AutomatedValueMap
void BBTrackContainer::automatedValuesAt(MidiTime           _start,
                                         int                _tcoNum,
                                         AutomatedValueMap& _map) const
{
    /*
    Q_ASSERT(_tcoNum >= 0);
    Q_ASSERT(_start.getTicks() >= 0);

    auto length_tacts = lengthOfBB(_tcoNum);
    auto length_ticks = length_tacts * MidiTime::ticksPerTact();
    if (_start > length_ticks) {
            _start = length_ticks;
    }
    */
    // return
    TrackContainer::automatedValuesAt(
            _start + (MidiTime::ticksPerTact() * _tcoNum), _tcoNum, _map);
}
