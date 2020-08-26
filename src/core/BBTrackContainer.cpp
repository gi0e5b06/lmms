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
      TrackContainer(nullptr, tr("Beats"), "beats"),
      m_currentBB(this, tr("Current beat"), "beat")
{
    // connect(&m_currentBB, SIGNAL(dataChanged()), this,
    //        SLOT(currentBBChanged()));
    // we *always* want to receive updates even in case BB actually did
    // not change upon setCurrentBB()-call
    // connect(&m_currentBB, SIGNAL(dataUnchanged()), this,
    //        SLOT(currentBBChanged()));
    setType(BBContainer);
}

BBTrackContainer::~BBTrackContainer()
{
    qInfo("BBTrackContainer::~BBTrackContainer");
}

bool BBTrackContainer::play(MidiTime      _start,
                            fpp_t         _frames,
                            f_cnt_t       _offset,
                            int           _bb,
                            const Bitset* _mask)
{
    tact_t beatlen = lengthOfBB(_bb);
    if(beatlen <= 0)
        return false;

    //_start = _start % (beatlen * MidiTime::ticksPerTact());

    bool   played_a_note = false;
    Tracks tl            = tracks();
    int    index         = 0;
    for(Tracks::iterator it = tl.begin(); it != tl.end(); ++it, ++index)
    {
        if(_mask != nullptr && index < _mask->size() && _mask->bit(index))
            continue;

        f_cnt_t realstart = _start;
        // Tile* p = (*it)->getTCO(_bb);
        Tile* p = (*it)->tileForBB(_bb);
        // Pattern* p=dynamic_cast<Pattern*>((*it)->getTCO( _bb ));
        if(p != nullptr)
        {
            tick_t tlen = p->unitLength();  // length();
            if(tlen <= 0)
                continue;
            realstart = _start % tlen;  // GDX
            // qInfo("realstart=%d tlen=%d",realstart,tlen);
        }
        if((*it)->play(realstart, _frames, _offset, _bb))
        {
            played_a_note = true;
        }
    }

    return played_a_note;
}

tact_t BBTrackContainer::lengthOfBB(int _bb) const
{
    MidiTime max_length = MidiTime::ticksPerTact();

    for(const Track* t: tracks())
    {
        // test when restoring, need to be removed
        // if(_bb >= t->getTCOs().size())
        //    continue;
        Tile* tco = t->tileForBB(_bb);  // t->getTCO(_bb);
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
        // test when restoring, need to be removed
        // if(_bb >= t->getTCOs().size())
        //    continue;
        // Pattern* p = dynamic_cast<Pattern*>(t->getTCO(_bb));
        Tile* p = t->tileForBB(_bb);
        if(p != nullptr)
        {
            tick_t plen
                    = p->unitLength();  // length();  // beatPatternLength();
            max_length = qMax(max_length, plen);
        }
    }
    return max_length;
}

/*
int BBTrackContainer::numOfBBs() const
{
    int r1 = Engine::song()->countTracks(Track::BBTrack);
    int r2 = infoMapSize();
    qInfo("BBTrackContainer::numOfBBs r1=%d r2=%d", r1, r2);
    return r1;
}
*/

/*
void BBTrackContainer::removeBB(int _bb)
{
    qInfo("BBTrackContainer::removeBB");
    for(Track* t: tracks())
    {
        delete t->getTCO(_bb);
        t->removeTact(_bb * DefaultTicksPerTact);
    }

    if(_bb <= currentBB())
        setCurrentBB(qMax(currentBB() - 1, 0));
}
*/

/*
void BBTrackContainer::swapBB(int _bb1, int _bb2)
{
    for(Track* t: tracks())
        t->swapPositionOfTCOs(_bb1, _bb2);
    updateComboBox();
}
*/

void BBTrackContainer::updateAfterInnerTrackAdd(Track* _track)
{
    qInfo("BBTrackContainer::updateAfterInnerTrackAdd");
    /*
    if(_track != nullptr)
        for(const BBTrack* t: bbTracks())
            if(t != nullptr)
                _track->createTCOsForBB(t->ownBBTrackIndex());
    checkTCOs();
    */
}

void BBTrackContainer::updateBBTrack(Tile* _tco)
{
    BBTrack* t = BBTrack::findBBTrack(_tco->startPosition()
                                      / DefaultTicksPerTact);
    if(t != nullptr)
        t->emit dataChanged();
}

void BBTrackContainer::fixIncorrectPositions()
{
    qInfo("BBTrackContainer::fixIncorrectPositions");
    // const int n = numOfBBs();
    for(Track* t: tracks())
        fixIncorrectPositions(t);
}

void BBTrackContainer::fixIncorrectPositions(Track* _t)
{
    qInfo("BBTrackContainer::fixIncorrectPositions for track");
    // const int n = numOfBBs();
    if(_t != nullptr)
    {
        _t->rearrangeAllTiles();
        int i = 0;
        for(BBTrack* bbt: bbTracks())
        {
            // for(int i = 0; i < n; ++i)
            // for(Tile* tco: t->getTCOs())
            int   j = bbt->ownBBTrackIndex();
            Tile* tco;
            if(i < _t->numOfTCOs())
                tco = _t->getTCO(i);
            else
                tco = _t->createTCO();

            tco->movePosition(MidiTime(j, 0));
            tco->setAutoResize(false);
            tco->setAutoRepeat(false);
            tco->changeLength(qMax(tco->unitLength(),
                                   MidiTime::ticksPerTact()
                                           / MidiTime::beatsPerTact()));
            // qMax(tco->unitLength(), tco->length().getTicks()));
            i++;
        }
    }
}

void BBTrackContainer::play()
{
    if(Engine::song()->playMode() != Song::Mode_PlayBB)
        Engine::song()->playBB();
    else
        Engine::song()->togglePause();
}

void BBTrackContainer::stop()
{
    Engine::song()->stop();
}

int BBTrackContainer::currentBB()
{
    return m_currentBB.value();
}

void BBTrackContainer::setCurrentBB(int _bb)
{
    m_currentBB.setValue(_bb);
    if(m_currentBB.value() != _bb)
        m_currentBB.setValue(lastUsedBeatIndex());
}

ComboBoxModel& BBTrackContainer::currentBBModel()
{
    return m_currentBB;
}

void BBTrackContainer::updateComboBox()
{
    // qInfo("BBTrackContainer::updateComboBox 1");
    const int cur_bb = currentBB();
    // qInfo("BBTrackContainer::updateComboBox 2 cur_bb=%d", cur_bb);
    m_currentBB.clear();
    // qInfo("BBTrackContainer::updateComboBox 3");
    /*
    const int n = numOfBBs();
    if(n > 0)
    {
        for(int i = 0; i < n; ++i)
        {
            // qInfo("BBTrackContainer::updateComboBox 5a i=%d", i);
            BBTrack* bbt = BBTrack::findBBTrack(i);
            // qInfo("BBTrackContainer::updateComboBox 5b i=%d", i);
            if(bbt == nullptr)
                qWarning("BBTrackContainer::updateComboBox bbt is null");
            else
            {
                QString s = bbt->name();
                // qInfo("BBTrackContainer::updateComboBox 5c i=%d s=%s", i,
                //      qPrintable(s));
                m_currentBB.addItem(i,s,nullptr,i);
                // qInfo("BBTrackContainer::updateComboBox 5d i=%d", i);
            }
        }
        setCurrentBB(qBound(0, cur_bb, n));
    }
    else
    {
        setCurrentBB(0);
    }
    */
    for(const BBTrack* t: bbTracks())
    {
        const int i = t->ownBBTrackIndex();
        m_currentBB.addItem(i, t->name(), nullptr, i);
    }
    setCurrentBB(cur_bb);
    // qInfo("BBTrackContainer::updateComboBox 6");
}

QMenu* BBTrackContainer::createBeatMenu(bool _enabled, int _index)
{
    QMenu* smb = new QMenu(tr("Beat"));

    QAction* a;
    // const int n = Engine::getBBTrackContainer()->numOfBBs();
    // for(int i = -1; i < n; i++)
    {
        a = smb->addAction(tr("Default"));
        a->setData(QVariant(-1));
        a->setCheckable(true);
        a->setChecked(-1 == _index);  // m_bbTCO->m_playBBTrackIndex);
        a->setEnabled(_enabled);
    }
    for(const BBTrack* t: bbTracks())
    {
        QString s(t->name());
        a     = smb->addAction(s);
        int i = t->ownBBTrackIndex();
        a->setData(QVariant(i));
        a->setCheckable(true);
        a->setChecked(i == _index);  // m_bbTCO->m_playBBTrackIndex
        a->setEnabled(_enabled);
    }

    return smb;
}

/*
void BBTrackContainer::currentBBChanged()
{
    // now update all track-labels (the current one has to become white,
    // the others gray)
    for(Track* t: Engine::song()->tracks())
        if(t->type() == Track::BBTrack)
            t->emit dataChanged();
}
*/

void BBTrackContainer::createTCOsForBB(int _bb)
{
    if(_bb < 0)
        return;
    for(Track* t: tracks())
        t->createTCOsForBB(_bb);
    printDebug();
}

void BBTrackContainer::createTCOsForBB(Track* _t)
{
    for(BBTrack* t: bbTracks())
        _t->createTCOsForBB(t->ownBBTrackIndex());
    printDebug();
}

void BBTrackContainer::deleteTCOsForBB(int _bb)
{
    if(_bb < 0)
        return;
    for(Track* t: tracks())
        t->deleteTCOsForBB(_bb);
    printDebug();
}

void BBTrackContainer::moveTCOsForBB(int _fromBB, int _toBB)
{
    if(_fromBB == _toBB)
    {
        qWarning("BBTrackContainer::moveTCOsForBB nothing to do %d==%d",
                 _fromBB, _toBB);
        return;
    }

    BBTrack* fromTrack = registryTrack(_fromBB);
    if(fromTrack == nullptr)
    {
        qWarning("BBTrackContainer::moveTCOsForBB %d not found", _fromBB);
        return;
    }
    BBTrack* toTrack = registryTrack(_toBB);
    if(toTrack != nullptr)
    {
        qWarning("BBTrackContainer::moveTCOsForBB already a track at %d",
                 _toBB);
        return;
    }

    qInfo("BBTrackContainer::moveTCOsForBB %d -> %d", _fromBB, _toBB);
    const int cur_bb = currentBB();
    for(Track* t: tracks())
        t->moveTCOsForBB(_fromBB, _toBB);
    fromTrack->setOwnBBTrackIndex(_toBB);
    if(cur_bb == _fromBB)
    {
        updateComboBox();
        setCurrentBB(_toBB);
    }
    // TODO: update tiles
}

void BBTrackContainer::rearrangeBB()
{
    int from, to;
    while((to = firstAvailableBeatIndex()) < (from = lastUsedBeatIndex()))
    {
        if(from > 0)
            moveTCOsForBB(from, to);
        else
            break;
    }
}

void BBTrackContainer::checkTCOs()
{
    for(Track* t: tracks())
    {
        for(Tile* tile: t->getTCOs())
        {
            tact_t p = tile->startPosition().tact();
            if(BBTrack::findBBTrack(p) == nullptr)
                qCritical("POS PROBLEM: BBTrackContainer::checkTCOs: %d", p);
        }
    }
    printDebug();
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

BBTracks BBTrackContainer::bbTracks() const
{
    BBTracks r;
    for(Track* t: Engine::song()->tracks())
    {
        BBTrack* bbt = dynamic_cast<BBTrack*>(t);
        if(bbt != nullptr)
            r.append(bbt);
    }
    return r;
}

int BBTrackContainer::firstAvailableBeatIndex()
{
    const int lubi  = lastUsedBeatIndex();
    int       bbNum = 0;
    while(bbNum <= lubi)
    {
        bool found = false;
        for(BBTrack* bbt: bbTracks())
            if(bbNum == bbt->ownBBTrackIndex())
            {
                found = true;
                break;
            }
        if(!found)
            break;
        bbNum++;
    }
    return bbNum;
}

int BBTrackContainer::lastUsedBeatIndex()
{
    int bbNum = -1;
    for(BBTrack* bbt: bbTracks())
        bbNum = qMax(bbNum, bbt->ownBBTrackIndex());
    return bbNum;
}

int BBTrackContainer::registerTrack(BBTrack* _bbTrack)
{
    int bbNum = firstAvailableBeatIndex();
    _bbTrack->setOwnBBTrackIndex(bbNum);

    qInfo("+++ BBTrackContainer::registerTrack bbNum=%d size=%d", bbNum,
          bbTracks().size());
    createTCOsForBB(bbNum);
    // fixIncorrectPositions();
    setCurrentBB(bbNum);
    // checkTCOs();
    return bbNum;
}

void BBTrackContainer::unregisterTrack(BBTrack* _bbTrack)
{
    if(_bbTrack == nullptr)
    {
        qInfo("BBTrackContainer::unregisterTrack track not found");
        return;
    }

    const int bbNum  = _bbTrack->ownBBTrackIndex();
    const int cur_bb = currentBB();
    deleteTCOsForBB(bbNum);

    qInfo("--- BBTrackContainer::unregister bbNum=%d size=%d", bbNum,
          bbTracks().size());
    checkTCOs();

    if(bbNum <= cur_bb)
        setCurrentBB(cur_bb - 1);  // qMax(currentBB() - 1, 0));
}

int BBTrackContainer::registryIndex(const BBTrack* _bbTrack) const
{
    if(_bbTrack == nullptr)
    {
        qCritical("BBTrackContainer::registryIndex bbtrack is null");
        return -1;
    }

    return _bbTrack->ownBBTrackIndex();
    /*
    int r = m_infoMap.value(_bbTrack, -1);
    if(r < 0)
    {
        BACKTRACE
        qCritical(
                "BBTrackContainer::registryIndex bbtrack '%s' not found "
                "imSize=%d",
                qPrintable(_bbTrack->name()), infoMapSize());
        printDebug();
    }
    return r;
    */
}

BBTrack* BBTrackContainer::registryTrack(int _bbNum) const
{
    for(BBTrack* bbt: bbTracks())
        if(_bbNum == bbt->ownBBTrackIndex())
            return bbt;

    if(!Engine::song()->isLoadingProject())
    {
        BACKTRACE
        qCritical("BBTrackContainer::registryTrack %d not found", _bbNum);
    }
    return nullptr;
}

/*
void BBTrackContainer::swapBBTracks(Track* _track1, Track* _track2)
{
    BBTrack* t1 = dynamic_cast<BBTrack*>(_track1);
    BBTrack* t2 = dynamic_cast<BBTrack*>(_track2);
    if(t1 != nullptr && t2 != nullptr)
    {
        qSwap(m_infoMap[t1], m_infoMap[t2]);
        swapBB(m_infoMap[t1], m_infoMap[t2]);
        setCurrentBB(m_infoMap[t1]);
    }
}
*/

void BBTrackContainer::printDebug() const
{
    const int imsize = bbTracks().size();
    for(Track* t: tracks())
    {
        Tiles   tiles   = t->getTCOs();
        int     tileNum = 0;
        QString r;
        for(Tile* tile: tiles)
        {
            if(tile == nullptr)
                r += "0";
            else if(tileNum >= imsize)
                r += ">";
            else
            {
                const int index = tile->startPosition().ticks()
                                  / MidiTime::ticksPerTact();
                /*
                  if(index != tileNum)
                  r += "?";
                  else
                */
                if(registryTrack(index) == nullptr)
                    r += "N";
                else
                    r += "+";
            }
            tileNum++;
        }
        qInfo("    BBTC %30s %s", qPrintable(t->name()), qPrintable(r));
    }
}
