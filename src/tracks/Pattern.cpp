/*
 * Pattern.cpp - implementation of class pattern which holds notes
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2005-2007 Danny McRae <khjklujn/at/yahoo.com>
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
#include "Pattern.h"

#include "AudioSampleRecorder.h"
#include "BBTrackContainer.h"
#include "Backtrace.h"
#include "CaptionMenu.h"
#include "GuiApplication.h"
#include "InstrumentTrack.h"
#include "MainWindow.h"
#include "PianoRoll.h"
#include "RenameDialog.h"
#include "SampleBuffer.h"
#include "StringPairDrag.h"
#include "ToolTip.h"
#include "embed.h"
#include "gui_templates.h"

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QTimer>

#include <cmath>
#include <limits>

QPixmap* PatternView::s_stepBtnOn0      = nullptr;
QPixmap* PatternView::s_stepBtnOn200    = nullptr;
QPixmap* PatternView::s_stepBtnOff      = nullptr;
QPixmap* PatternView::s_stepBtnOffLight = nullptr;

Pattern::Pattern(InstrumentTrack* _instrumentTrack) :
      Tile(_instrumentTrack, tr("Score tile"), "scoreTile"),
      m_instrumentTrack(_instrumentTrack), m_patternType(MelodyPattern)
{
    setName(_instrumentTrack->name());
    // if(isFixed())
    //  m_patternType = BeatPattern;
    init();

    changeLength(MidiTime(1, 0));
    setAutoResize(!isFixed());
    setAutoRepeat(!isFixed());
}

Pattern::Pattern(const Pattern& _other) :
      // Tile(_other.m_instrumentTrack, _other.displayName()),
      Tile(_other), m_instrumentTrack(_other.m_instrumentTrack),
      m_patternType(_other.m_patternType)
{
    for(const Note* note: _other.m_notes)
        m_notes.append(new Note(*note));

    // if(isFixed())
    //        m_patternType = BeatPattern;
    init();
    setAutoResize(_other.autoResize() && !isFixed());
    setAutoRepeat(_other.autoRepeat() && !isFixed());
}

Pattern::~Pattern()
{
    emit destroyedPattern(this);

    // clearNotes();
    for(Note* note: m_notes)
        delete note;

    m_notes.clear();
}

bool Pattern::isEmpty() const
{
    for(const Note* note: m_notes)
        if(note->length() != 0)
            return false;

    return true;
}

/*
QString Pattern::defaultName() const
{
        return instrumentTrack()->name();
}
*/

void Pattern::resizeToFirstTrack()
{
    // Resize this track to be the same as existing tracks in the BB
    const Tracks& tracks = m_instrumentTrack->trackContainer()->tracks();
    for(unsigned int trackID = 0; trackID < tracks.size(); ++trackID)
    {
        if(tracks.at(trackID)->type() == Track::InstrumentTrack)
        {
            if(tracks.at(trackID) != m_instrumentTrack)
            {
                unsigned int currentTCO
                        = m_instrumentTrack->getTCOs().indexOf(this);
                m_steps = static_cast<Pattern*>(
                                  tracks.at(trackID)->getTCO(currentTCO))
                                  ->m_steps;
            }
            break;
        }
    }
}

void Pattern::init()
{
    connect(Engine::song(), SIGNAL(timeSignatureChanged(int, int)), this,
            SLOT(changeTimeSignature()));
    saveJournallingState(false);
    checkType();
    updateLength();
    restoreJournallingState();
}

const Chords Pattern::chords() const
{
    Chords r;
    Chord  c;
    tick_t start = -1;
    tick_t end   = -1;
    for(const Note* note: notes())
    {
        if(note->pos().ticks() > start || note->endPos().ticks() != end)
        {
            if(!c.isEmpty())
            {
                r.append(c);
                c = Chord();
            }
            c.append(note);
            start = note->pos().ticks();
            end   = note->endPos().ticks();
        }
        else
        {
            c.append(note);
        }
    }
    if(!c.isEmpty())
        r.append(c);
    return r;
}

Note* Pattern::addNote(const Note& _new_note, const bool _quant_pos)
{
    Note* new_note = new Note(_new_note);
    if(_quant_pos && gui->pianoRollWindow())
    {
        new_note->quantizePos(gui->pianoRollWindow()->quantization());
    }

    InstrumentTrack* t = instrumentTrack();
    t->lockTrack();
    if(m_notes.size() == 0 || m_notes.back()->pos() <= new_note->pos())
    {
        m_notes.push_back(new_note);
    }
    else
    {
        // simple algorithm for inserting the note between two
        // notes with smaller and greater position
        // maybe it could be optimized by starting in the middle and
        // going forward or backward but note-inserting isn't that
        // time-critical since it is usually not done while playing...
        long            new_note_abs_time = new_note->pos();
        Notes::Iterator it                = m_notes.begin();

        while(it != m_notes.end() && (*it)->pos() < new_note_abs_time)
        {
            ++it;
        }

        m_notes.insert(it, new_note);
    }
    t->unlockTrack();

    checkType();
    updateLength();
    emit dataChanged();

    return new_note;
}

void Pattern::removeNote(Note* _note_to_del)
{
    InstrumentTrack* t = instrumentTrack();
    t->lockTrack();

    Notes::Iterator it = m_notes.begin();
    while(it != m_notes.end())
    {
        if(*it == _note_to_del)
        {
            delete *it;
            m_notes.erase(it);
            break;
        }
        ++it;
    }
    t->unlockTrack();

    checkType();
    updateLength();
    emit dataChanged();
}

// returns a pointer to the note at specified step, or nullptr if note doesn't
// exist

Note* Pattern::noteAtStep(int _step)
{
    for(Notes::Iterator it = m_notes.begin(); it != m_notes.end(); ++it)
    {
        if((*it)->pos() == stepPosition(_step) && (*it)->length() < 0)
        {
            return *it;
        }
    }
    return nullptr;
}

void Pattern::rearrangeAllNotes()
{
    // sort notes by start time
    qSort(m_notes.begin(), m_notes.end(), Note::lessThan);
}

void Pattern::clearNotes()
{
    if(m_notes.size() > 0)
    {
        // BACKTRACE
        InstrumentTrack* t = instrumentTrack();
        if(t != nullptr)
            t->lockTrack();

        for(Notes::Iterator it = m_notes.begin(); it != m_notes.end(); ++it)
        {
            // qInfo("deleting note %p pattern %p",*it,this);
            delete *it;
        }
        m_notes.clear();

        if(t != nullptr)
            t->unlockTrack();
    }
    // else qFatal("not a pattern\n");

    checkType();
    emit dataChanged();
}

Note* Pattern::addStepNote(int step)
{
    Note* note = noteAtStep(step);
    return note == nullptr ? addNote(
                   Note(MidiTime(-DefaultTicksPerTact), stepPosition(step)),
                   false)
                           : note;
}

void Pattern::removeStepNote(int step)
{
    while(Note* note = noteAtStep(step))
        removeNote(note);
}

void Pattern::toggleStepNote(int step)
{
    Note* note = noteAtStep(step);
    if(note != nullptr)
        removeStepNote(step);
    else
        addStepNote(step);
}

void Pattern::setStepNote(int step, bool enabled)
{
    if(enabled)
        addStepNote(step);
    else
        removeStepNote(step);
}

void Pattern::setType(PatternTypes _new_pattern_type)
{
    /*
    if( _new_pattern_type == BeatPattern ||
        _new_pattern_type == MelodyPattern )
    {
    */
    m_patternType = _new_pattern_type;
    //}
}

void Pattern::checkType()
{
    Notes::Iterator it = m_notes.begin();
    while(it != m_notes.end())
    {
        if((*it)->length() > 0)
        {
            setType(MelodyPattern);
            return;
        }
        ++it;
    }
    setType(isFixed() ? BeatPattern : MelodyPattern);
}

void Pattern::flipHorizontally()
{
    if(isEmpty())
        return;

    InstrumentTrack* t = instrumentTrack();
    if(t != nullptr)
        t->lockTrack();

    tick_t len      = autoRepeat() ? unitLength() : length().getTicks();
    bool   modified = false;
    for(Note* note: m_notes)
    {
        tick_t p = (len - note->endPos()) % len;
        if(p < 0)
            p += len;
        note->setPos(p);
        modified = true;
    }
    if(modified)
        rearrangeAllNotes();

    if(t != nullptr)
        t->unlockTrack();

    // checkType();
    updateLength();  // not needed?
    emit dataChanged();
    if(modified)
        Engine::song()->setModified();
}

void Pattern::flipVertically()
{
    if(isEmpty())
        return;

    InstrumentTrack* t = instrumentTrack();
    if(t != nullptr)
        t->lockTrack();

    bool modified = false;
    int  minkey   = -1;
    int  maxkey   = -1;
    for(Note* note: m_notes)
    {
        if(minkey == -1 || minkey > note->key())
            minkey = note->key();
        if(maxkey == -1 || maxkey < note->key())
            maxkey = note->key();
    }
    for(Note* note: m_notes)
    {
        int ko = note->key();
        int kn = maxkey - (ko - minkey);
        if(ko != kn)
        {
            note->setKey(kn);
            modified = true;
        }
    }
    if(modified)
        rearrangeAllNotes();

    if(t != nullptr)
        t->unlockTrack();

    // checkType();
    // updateLength();
    emit dataChanged();
    if(modified)
        Engine::song()->setModified();
}

void Pattern::rotate(tick_t _ticks)
{
    if(_ticks == 0 || isEmpty())
        return;

    InstrumentTrack* t = instrumentTrack();
    if(t != nullptr)
        t->lockTrack();

    tick_t len      = autoRepeat() ? unitLength() : length().ticks();
    bool   modified = false;
    tick_t min      = len;
    tick_t max      = len;
    for(Note* note: m_notes)
    {
        tick_t p = (note->pos() + _ticks) % len;
        if(p < 0)
            p += len;
        note->setPos(p);
        min      = qMin<tick_t>(min, note->pos());
        max      = qMax<tick_t>(max, note->endPos());
        modified = true;
    }
    if(modified)
    {
        if(max > len && min > 0)
            for(Note* note: m_notes)
                note->setPos(note->pos() - qMin(min, max - len));
        rearrangeAllNotes();
    }
    if(t != nullptr)
        t->unlockTrack();

    // checkType();
    updateLength();
    emit dataChanged();
    if(modified)
        Engine::song()->setModified();
}

void Pattern::splitEvery(tick_t _ticks)
{
    Q_UNUSED(_ticks)
    qInfo("Pattern: splitEvery() not implemented");
}

void Pattern::splitAt(tick_t _tick)
{
    if(_tick <= 0 || _tick + 1 >= length())
        return;

    qInfo("Pattern::split at tick %d", _tick);

    MidiTime sp = startPosition();
    MidiTime ep = endPosition();

    Pattern* newp1 = new Pattern(*this);  // 1 left, before
    newp1->setJournalling(false);
    newp1->setAutoResize(false);
    newp1->movePosition(sp);
    if(!newp1->isEmpty())
    {
        Notes todelete;
        for(Note* n: newp1->notes())
        {
            if(newp1->autoRepeat())
                continue;

            if(n->pos() < _tick)
                continue;
            // qInfo("p1 pos: s=%d n=%d r=%d remove note",
            //  (int)newp1->startPosition(),(int)n->pos(),_splitPos);
            todelete.append(n);
            // newp1->removeNote(n);
        }
        for(Note* n: todelete)
            newp1->removeNote(n);
    }
    newp1->changeLength(_tick);
    newp1->rearrangeAllNotes();
    newp1->setJournalling(true);
    newp1->emit dataChanged();

    sp += _tick;

    Pattern* newp2 = new Pattern(*this);  // 2 right, after
    newp2->setJournalling(false);
    newp2->movePosition(sp);
    if(!newp2->isEmpty())
    {
        if(newp2->autoRepeat())
            newp2->rotate(-_tick);  //(_tick % newp2->unitLength()));

        Notes todelete;
        for(Note* n: newp2->notes())
        {
            if(newp2->autoRepeat())
                continue;

            if(n->pos() >= _tick)
            {
                // qInfo("p2 pos: s=%d n=%d r=%d k=%d move note",
                //  (int)newp2->startPosition(),(int)n->pos(),
                // _splitPos,n->key());
                n->setPos(n->pos() - _tick);
                continue;
            }
            // qInfo("p2 pos: s=%d n=%d r=%d k=%d remove note",
            //  (int)newp2->startPosition(),(int)n->pos(),_splitPos,n->key());
            todelete.append(n);
        }
        for(Note* n: todelete)
            newp2->removeNote(n);
    }
    newp2->changeLength(ep - sp);
    newp2->rearrangeAllNotes();
    newp2->setJournalling(true);
    newp2->emit dataChanged();

    deleteLater();  // tv->remove();
}

void Pattern::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    Tile::saveSettings(_doc, _this);

    _this.setAttribute("type", m_patternType);

    //_this.setAttribute("name", name());
    // as the target of copied/dragged pattern is always an existing
    // pattern, we must not store actual position, instead we store -1
    // which tells loadSettings() not to mess around with position
    /*
    if( _this.parentNode().nodeName() == "clipboarddata" ||
        _this.parentNode().nodeName() == "dnddata" )
    {
            _this.setAttribute( "pos", -1 );
    }
    else
    */
    {
        //_this.setAttribute("pos", startPosition());
    }

    // len not used but added for coherency
    // if(length() > 0)
    //    _this.setAttribute("len", length());

    //_this.setAttribute("muted", isMuted());

    _this.setAttribute("steps", m_steps);
    _this.setAttribute("stepres", m_stepResolution);

    // now save settings of all notes
    for(Notes::Iterator it = m_notes.begin(); it != m_notes.end(); ++it)
    {
        (*it)->saveState(_doc, _this);
    }
}

void Pattern::loadSettings(const QDomElement& _this)
{
    m_patternType
            = static_cast<PatternTypes>(_this.attribute("type").toInt());

    Tile::loadSettings(_this);

    /*
      setName(_this.attribute("name"));
      if(_this.attribute("pos").toInt() >= 0)
      {
      movePosition(_this.attribute("pos").toInt());
      }
      if(_this.attribute("muted").toInt() != isMuted())
      {
      toggleMute();
      }
    */

    clearNotes();

    QDomNode node = _this.firstChild();
    while(!node.isNull())
    {
        if(node.isElement()
           && !node.toElement().attribute("metadata").toInt())
        {
            Note* n = new Note;
            n->restoreState(node.toElement());
            m_notes.push_back(n);
        }
        node = node.nextSibling();
    }

    m_steps = _this.attribute("steps").toInt();
    if(m_steps == 0)
    {
        m_steps = stepsPerTact();
    }

    m_stepResolution = _this.attribute("stepres").toInt();
    if(m_stepResolution == 0)
    {
        m_stepResolution = DefaultStepsPerTact;
    }

    checkType();
    updateLength();

    emit dataChanged();
}

/*
Pattern* Pattern::previousPattern() const
{
    return adjacentPatternByOffset(-1);
}

Pattern* Pattern::nextPattern() const
{
    return adjacentPatternByOffset(1);
}

Pattern* Pattern::adjacentPatternByOffset(int offset) const
{
    m_instrumentTrack->lockTrack();
    m_instrumentTrack->rearrangeAllTiles();
    Tiles    tcos   = m_instrumentTrack->getTCOs();
    int      tcoNum = m_instrumentTrack->getTCONum(this);
    Pattern* r = dynamic_cast<Pattern*>(tcos.value(tcoNum + offset, nullptr));
    m_instrumentTrack->unlockTrack();
    return r;
}
*/

void Pattern::clear()
{
    addJournalCheckPoint();
    clearNotes();
}

void Pattern::cloneSteps()
{
    int oldLength = m_steps;
    m_steps *= 2;  // cloning doubles the track
    for(int i = 0; i < oldLength; ++i)
    {
        Note* toCopy = noteAtStep(i);
        if(toCopy)
        {
            setStepNote(oldLength + i, true);
            Note* newNote = noteAtStep(oldLength + i);
            newNote->setKey(toCopy->key());
            newNote->setLength(toCopy->length());
            newNote->setPanning(toCopy->getPanning());
            newNote->setVolume(toCopy->getVolume());
        }
    }
    updateLength();
    emit dataChanged();
}

TileView* Pattern::createView(TrackView* _tv)
{
    return PatternView::create(this, _tv);
}

/*
void Pattern::updateBBTrack()
{
        if( track()->trackContainer() == Engine::getBBTrackContainer() )
        {
                Engine::getBBTrackContainer()->updateBBTrack( this );
        }

        if( gui && gui->pianoRollWindow() &&
gui->pianoRollWindow()->currentPattern() == this )
        {
                gui->pianoRollWindow()->update();
        }
}
*/

tick_t Pattern::unitLength() const
{
    tick_t len = MidiTime::ticksPerTact();

    if(isFixed())
    {
        len = m_steps * 16.f / m_stepResolution * MidiTime::ticksPerTact()
              / stepsPerTact();
        // qInfo("Pattern::unitLength len=%d tl=%d", len,
        //       MidiTime::ticksPerTact());
    }
    else
    {
        for(Notes::ConstIterator it = m_notes.begin(); it != m_notes.end();
            ++it)
        {
            // if((*it)->length() > 0)
            //{
            len = qMax<tick_t>(len, (*it)->endPos());
            //}
        }
        len = MidiTime(len).nextFullTact() * MidiTime::ticksPerTact();
    }

    return len;
}

void Pattern::updateLength()
{
    checkType();

    tick_t len;
    if(autoResize())
        len = unitLength();
    else
        len = length();

    Tile::updateLength(len);
}

void Pattern::changeTimeSignature()
{
    MidiTime last_pos = MidiTime::ticksPerTact() - 1;
    for(Notes::ConstIterator cit = m_notes.begin(); cit != m_notes.end();
        ++cit)
    {
        if((*cit)->length() < 0 && (*cit)->pos() > last_pos)
        {
            last_pos = (*cit)->pos()
                       + MidiTime::ticksPerTact() / stepsPerTact();
        }
    }
    last_pos = last_pos.nextFullTact() * MidiTime::ticksPerTact();
    m_steps  = qMax<tick_t>(stepsPerTact(),
                           last_pos.getTact() * stepsPerTact());
    updateLength();
}

PatternView::PatternView(Pattern* pattern, TrackView* parent) :
      TileView(pattern, parent), m_pat(pattern), m_paintPixmap(),
      m_noteFillColor(255, 255, 255, 220),
      m_noteBorderColor(255, 255, 255, 220),
      m_mutedNoteFillColor(100, 100, 100, 220),
      m_mutedNoteBorderColor(100, 100, 100, 220)
{
    if(s_stepBtnOn0 == nullptr)
        s_stepBtnOn0 = new QPixmap(embed::getPixmap("step_btn_on_0"));
    if(s_stepBtnOn200 == nullptr)
        s_stepBtnOn200 = new QPixmap(embed::getPixmap("step_btn_on_200"));
    if(s_stepBtnOff == nullptr)
        s_stepBtnOff = new QPixmap(embed::getPixmap("step_btn_off"));
    if(s_stepBtnOffLight == nullptr)
        s_stepBtnOffLight
                = new QPixmap(embed::getPixmap("step_btn_off_light"));

    // connect(m_pat, SIGNAL(dataChanged()), this, SLOT(update()));
    connect(gui->pianoRollWindow(), SIGNAL(currentPatternChanged()), this,
            SLOT(update()));

    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setStyle(QApplication::style());
    update();
}

PatternView::~PatternView()
{
}

void PatternView::update()
{
    ToolTip::add(this, m_pat->name());
    TileView::update();
}

void PatternView::openInPianoRoll()
{
    gui->pianoRollWindow()->open(m_pat, false);
}

void PatternView::setGhostInPianoRoll()
{
    gui->pianoRollWindow()->open(m_pat, true);
}

/*
void PatternView::changeName()
{
        QString s = m_pat->name();
        RenameDialog rename_dlg( s );
        rename_dlg.exec();
        m_pat->setName( s );
}

void PatternView::resetName()
{
        m_pat->setName( m_pat->m_instrumentTrack->name() );
}
*/

void PatternView::setNameFromFirstNote()
{
    if(!m_pat->isEmpty())
    {
        int    kmin = NumKeys - 1;
        tick_t smin = m_pat->length();

        for(Note* n: m_pat->notes())
        {
            int    k = n->key();
            tick_t s = n->pos();
            if(s < smin)
            {
                smin = s;
                kmin = k;
            }
            if(k < kmin && s <= smin)
            {
                smin = s;
                kmin = k;
            }
        }
        m_pat->setName(Note::findKeyName(kmin));
    }
}

void PatternView::changeStepResolution(QAction* _a)
{
    const int res = _a->data().toInt();
    if(res > 0)
    {
        m_pat->setStepResolution(res);
        update();
    }
}

QMenu* PatternView::buildContextMenu()
{
    QPointer<CaptionMenu> cm = new CaptionMenu(model()->displayName(), this);
    // QMenu* cm = new QMenu(this);

    QAction* a;

    cm->addAction(embed::getIcon("piano"), tr("Open in the piano roll"), this,
                  SLOT(openInPianoRoll()));
    cm->addSeparator();
    addRemoveMuteClearMenu(cm, true, true, !m_pat->isEmpty());
    cm->addSeparator();
    addCutCopyPasteMenu(cm, true, true, true);

    /*
    cm->addSeparator();
    cm->addAction( embed::getIcon( "edit_erase" ),
                    tr( "Clear all notes" ), m_pat, SLOT( clear() ) );
    */

    cm->addSeparator();
    addFlipMenu(cm, !m_pat->isEmpty(), !m_pat->isEmpty());
    addRotateMenu(cm, !m_pat->isEmpty(), !m_pat->isEmpty(), isFixed());
    addSplitMenu(cm, m_pat->length() > MidiTime::ticksPerTact(),
                 m_pat->length() > 4 * MidiTime::ticksPerTact());

    if(isFixed())
    {
        cm->addSeparator();
        addStepMenu(cm, m_pat->length() >= MidiTime::ticksPerTact(), true,
                    true);
        if(m_pat->type() == Pattern::BeatPattern)
            cm->addAction(embed::getIcon("step_btn_duplicate"),
                          tr("Clone Steps"), m_pat, SLOT(cloneSteps()));
    }

    cm->addSeparator();
    addPropertiesMenu(cm, !isFixed(), !isFixed());

    a = cm->addAction(embed::getIcon("ghost_note"), tr("Ghost notes"), this,
                      SLOT(setGhostInPianoRoll()));
    a->setEnabled(gui->pianoRollWindow()->currentPattern()
                  && gui->pianoRollWindow()->currentPattern() != m_pat
                  && !m_pat->empty());

    cm->addSeparator();
    addNameMenu(cm, true);

    a = cm->addAction(tr("Set name from first note"), this,
                      SLOT(setNameFromFirstNote()));
    a->setEnabled(!m_pat->empty());

    cm->addSeparator();
    addColorMenu(cm, true);

    return cm;
}

/*
void PatternView::addStepMenu(QMenu* _cm, bool _enabled)
{
        Q_UNUSED(_enabled)

        QMenu* sma=new QMenu(tr("Add steps"));
        QMenu* smr=new QMenu(tr("Remove steps"));
        sma->addAction( embed::getIcon( "step_btn_add" ),
                        tr( "One bar" ), m_pat, SLOT( addBarSteps() ) );
        smr->addAction( embed::getIcon( "step_btn_remove" ),
                        tr( "One bar" ), m_pat, SLOT( removeBarSteps() ) );
        sma->addAction( embed::getIcon( "step_btn_add" ),
                        tr( "One beat" ), m_pat, SLOT( addBeatSteps() ) );
        smr->addAction( embed::getIcon( "step_btn_remove" ),
                        tr( "One beat" ), m_pat, SLOT( removeBeatSteps() ) );
        sma->addAction( embed::getIcon( "step_btn_add" ),
                        tr( "One step" ), m_pat, SLOT( addOneStep() ) );
        smr->addAction( embed::getIcon( "step_btn_remove" ),
                        tr( "One step" ), m_pat, SLOT( removeOneStep() ) );

        _cm->addSeparator();
        _cm->addMenu(sma);
        _cm->addMenu(smr);
        _cm->addAction( embed::getIcon( "step_btn_duplicate" ),
                        tr( "Clone Steps" ), m_pat, SLOT( cloneSteps() ) );

        QMenu* sme=new QMenu(tr("Step resolution"));
        connect(sme,SIGNAL( triggered(QAction*) ),
                this, SLOT( changeStepResolution(QAction*)));

        static const int labels[]={
                2,4,8,16,32,64,128,
                3,6,12,24,48};
        static const QString icons[]={
                "note_whole",
                "note_half",
                "note_quarter",
                "note_eighth",
                "note_sixteenth",
                "note_thirtysecond",
                "note_sixtyfourth",
                "note_onehundredtwentyeighth",
                "note_triplethalf",
                "note_tripletquarter",
                "note_tripleteighth",
                "note_tripletsixteenth",
                "note_tripletthirtysecond"};
        for(int i=0;i<11;i++)
                sme->addAction(embed::getIcon(icons[i]),
                               QString::number(labels[i]))
                        ->setData(labels[i]);
        _cm->addSeparator();
        _cm->addMenu(sme);
}
*/

int PatternView::mouseToStep(int _x, int _y)
{
    const int   sr = m_pat->m_stepResolution;
    const float ws = 16.f * 16.f / sr;
    const float w1 = qMin(12, int(ws >= 16.f ? ws - 4.f : ws - 1.f));
    const int   h1 = 24;

    float tmp  = (float(_x) - TCO_BORDER_WIDTH - 2) / ws;
    int   step = int(tmp);
    if((tmp - step) * ws >= w1)
        return -1;

    const int hw = height();
    if(_y < (hw - h1) / 2)
        return -1;
    if(_y > (hw + h1) / 2)
        return -1;

    return step;
}

void PatternView::mousePressEvent(QMouseEvent* _me)
{
    if(m_pat->track()->isFrozen())
        _me->ignore();

    if((_me->button() == Qt::MiddleButton)
       && (_me->modifiers() & Qt::ShiftModifier)
       && (m_pat->m_patternType == Pattern::MelodyPattern || !isFixed()))
    {
        openInPianoRoll();
    }

    // when mouse button is pressed in beat/bassline -mode
    if(_me->button() == Qt::LeftButton
       && m_pat->m_patternType == Pattern::BeatPattern
       && _me->y() > height() - s_stepBtnOff->height())
    // fixedTCOs() )
    //|| pixelsPerTact() >= 96 ||
    // m_pat->m_steps != m_pat->stepsPerTact() ) &&
    {
        int step = mouseToStep(_me->x(), _me->y());
        if(step < 0 || step >= m_pat->m_steps)
            return;

        m_pat->addJournalCheckPoint();
        m_pat->toggleStepNote(step);
        Engine::song()->setModified();
        update();

        if(gui->pianoRollWindow()->currentPattern() == m_pat)
            gui->pianoRollWindow()->update();
    }
    else

    // if not in beat/bassline -mode, let parent class handle the event

    {
        TileView::mousePressEvent(_me);
    }
}

void PatternView::mouseDoubleClickEvent(QMouseEvent* _me)
{
    if(m_pat->track()->isFrozen())
        _me->ignore();

    if(_me->button() != Qt::LeftButton)
    {
        _me->ignore();
        return;
    }
    if(m_pat->m_patternType == Pattern::MelodyPattern || !isFixed())
    {
        openInPianoRoll();
    }
}

void PatternView::wheelEvent(QWheelEvent* _we)
{
    if(m_pat->track()->isFrozen())
        _we->ignore();

    if(m_pat->m_patternType == Pattern::BeatPattern
       && _we->y() > height() - s_stepBtnOff->height())
    //&& ( fixedTCOs())
    //|| pixelsPerTact() >= 96 ||
    // m_pat->m_steps != m_pat->stepsPerTact() )
    {
        int step = mouseToStep(_we->x(), _we->y());
        if(step < 0 || step >= m_pat->m_steps)
            return;

        Note* n = m_pat->noteAtStep(step);

        if(!n && _we->delta() > 0)
        {
            n = m_pat->addStepNote(step);
            n->setVolume(0);
        }

        if(n != nullptr)
        {
            int vol = n->getVolume();

            if(_we->delta() > 0)
                n->setVolume(qMin(100, vol + 5));
            else
                n->setVolume(qMax(0, vol - 5));

            Engine::song()->setModified();
            update();
            if(gui->pianoRollWindow()->currentPattern() == m_pat)
                gui->pianoRollWindow()->update();
        }
        _we->accept();
    }
    else
    {
        TileView::wheelEvent(_we);
    }
}

static int computeNoteRange(int minKey, int maxKey)
{
    return (maxKey - minKey) + 1;
}

void PatternView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    if(!needsUpdate())
    {
        painter.drawPixmap(0, 0, m_paintPixmap);
        return;
    }

    setNeedsUpdate(false);

    if(m_paintPixmap.isNull() || m_paintPixmap.size() != size())
    {
        m_paintPixmap = QPixmap(size());
    }

    QPainter p(&m_paintPixmap);

    bool const muted   = m_pat->track()->isMuted() || m_pat->isMuted();
    bool       current = gui->pianoRollWindow()->currentPattern() == m_pat;
    bool       ghost   = gui->pianoRollWindow()->ghostPattern() == m_pat;
    bool       beatPattern = m_pat->m_patternType == Pattern::BeatPattern;

    // state: selected, normal, user, beat pattern, muted
    QColor bgcolor
            = isSelected()
                      ? selectedColor()
                      : ((!muted && !beatPattern)
                                 ? (useStyleColor() ? (
                                            m_pat->track()->useStyleColor()
                                                    ? painter.background()
                                                              .color()
                                                    : m_pat->track()->color())
                                                    : color())
                                 : (beatPattern ? BBPatternBackground()
                                                : mutedBackgroundColor()));

    // invert the gradient for the background in the B&B editor
    /*
    QLinearGradient lingrad( 0, 0, 0, height() );
    lingrad.setColorAt( beatPattern ? 0 : 1, c.darker( 300 ) );
    lingrad.setColorAt( beatPattern ? 1 : 0, c );

    // paint a black rectangle under the pattern to prevent glitches with
    transparent backgrounds p.fillRect( rect(), QColor( 0, 0, 0 ) );

    if( gradient() )
    {
            p.fillRect( rect(), lingrad );
    }
    else
    */
    {
        p.fillRect(rect(), bgcolor);
    }

    // Check whether we will paint a text box and compute its potential height
    // This is needed so we can paint the notes underneath it.
    bool const isDefaultName = m_pat->name() == m_pat->defaultName();
    bool const drawTextBox   = !beatPattern && !isDefaultName;

    // TODO Warning! This might cause problems if
    // TileView::paintTextLabel changes
    int       textBoxHeight = 0;
    const int textTop       = TCO_BORDER_WIDTH + 1;
    if(drawTextBox)
    {
        QFont labelFont = this->font();
        labelFont.setHintingPreference(QFont::PreferFullHinting);

        QFontMetrics fontMetrics(labelFont);
        textBoxHeight = fontMetrics.height() + 2 * textTop;
    }

    // Compute pixels per tact
    // const int baseWidth = fixedTCOs() ? parentWidget()->width() : width();
    // const float pixelsPerTact = ( baseWidth - 2 * TCO_BORDER_WIDTH ) /
    // (float) m_pat->length().getTact();

    // Length of one tact/beat in the [0,1] x [0,1] coordinate system
    // const float tactLength = 1. / ceilf(m_pat->length().getTact());
    // const float tactLength = 1. / MidiTime(m_pat->unitLength()).getTact();
    // const float tickLength = tactLength / MidiTime::ticksPerTact();
    const float tickLength = 1. / m_pat->length();

    // melody pattern paint event
    Notes const& noteCollection = m_pat->m_notes;
    if(m_pat->m_patternType == Pattern::MelodyPattern
       && !noteCollection.empty())
    {
        // Compute the minimum and maximum key in the pattern
        // so that we know how much there is to draw.
        int maxKey = std::numeric_limits<int>::min();
        int minKey = std::numeric_limits<int>::max();

        for(Note const* note: noteCollection)
        {
            int const key = note->key();
            maxKey        = qMax(maxKey, key);
            minKey        = qMin(minKey, key);
        }
        if(minKey == maxKey)
        {
            minKey--;
            maxKey++;
        }
        if(maxKey % 12 != 0)
            maxKey = maxKey - maxKey % 12 + 12;
        minKey -= minKey % 12;
        if(maxKey - minKey <= 12)
        {
            if((minKey / 12) % 2 == 1)
                minKey -= 12;
            else
                maxKey += 12;
        }
        // If needed adjust the note range so that we always have paint a
        // certain interval
        int const minimalNoteRange = 12;  // Always paint at least one octave
        int const actualNoteRange  = computeNoteRange(minKey, maxKey);

        if(actualNoteRange < minimalNoteRange)
        {
            int missingNumberOfNotes = minimalNoteRange - actualNoteRange;
            minKey = std::max(0, minKey - missingNumberOfNotes / 2);
            maxKey = maxKey + missingNumberOfNotes / 2;
            if(missingNumberOfNotes % 2 == 1)
            {
                // Put more range at the top to bias drawing towards the
                // bottom
                ++maxKey;
            }
        }

        int const adjustedNoteRange = computeNoteRange(minKey, maxKey);

        // Transform such that [0, 1] x [0, 1] paints in the correct area
        float distanceToTop = textBoxHeight;

        // This moves the notes smoothly under the text
        int widgetHeight      = height();
        int fullyAtTopAtLimit = MINIMAL_TRACK_HEIGHT;
        int fullyBelowAtLimit = 4 * fullyAtTopAtLimit;
        if(widgetHeight <= fullyBelowAtLimit)
        {
            if(widgetHeight <= fullyAtTopAtLimit)
            {
                distanceToTop = 0;
            }
            else
            {
                float const a = 1. / (fullyAtTopAtLimit - fullyBelowAtLimit);
                float const b = -float(fullyBelowAtLimit)
                                / (fullyAtTopAtLimit - fullyBelowAtLimit);
                float const scale = a * widgetHeight + b;
                distanceToTop     = (1. - scale) * textBoxHeight;
            }
        }

        int const notesBorder = 4;  // Border for the notes towards the top
                                    // and bottom in pixels

        // The relavant painting code starts here
        p.save();

        p.translate(0., distanceToTop + notesBorder);
        p.scale(width(), height() - distanceToTop - 2 * notesBorder);

        // set colour based on mute status
        QColor noteFillColor
                = muted ? getMutedNoteFillColor() : getNoteFillColor();
        QColor noteBorderColor
                = muted ? getMutedNoteBorderColor() : getNoteBorderColor();

        bool const drawAsLines = height() < 64;
        if(drawAsLines)
        {
            p.setPen(noteFillColor);
        }
        else
        {
            p.setPen(noteBorderColor);
            // p.setRenderHint(QPainter::Antialiasing); ??? GDX
        }

        // Needed for Qt5 although the documentation for
        // QPainter::setPen(QColor) as it's used above states that it should
        // already set a width of 0.
        QPen pen = p.pen();
        pen.setWidth(0);
        p.setPen(pen);

        float const noteHeight = 1. / adjustedNoteRange;

        float x0 = 0.f;
        while(x0 < width())
        {
            // scan through all the notes and draw them on the pattern
            for(Note const* currentNote: noteCollection)
            {
                // Map to 0, 1, 2, ...
                int mappedNoteKey = currentNote->key() - minKey;
                int invertedMappedNoteKey
                        = adjustedNoteRange - mappedNoteKey - 1;

                float const noteStartX = x0 + currentNote->pos() * tickLength;
                float const noteLength
                        = qMax<int>(1, currentNote->length()) * tickLength;

                float const noteStartY = invertedMappedNoteKey * noteHeight;

                QRectF noteRectF(noteStartX, noteStartY, noteLength,
                                 noteHeight);
                if(drawAsLines)
                {
                    p.drawLine(QPointF(noteStartX,
                                       noteStartY + 0.5 * noteHeight),
                               QPointF(noteStartX + noteLength,
                                       noteStartY + 0.5 * noteHeight));
                }
                else
                {
                    p.fillRect(noteRectF, noteFillColor);
                    p.drawRect(noteRectF);
                }
            }

            if(!m_pat->autoRepeat())
                break;

            x0 += m_pat->unitLength() * tickLength;
        }

        p.restore();
    }

    // beat pattern paint event
    else if(beatPattern)  //&& fixedTCOs() )
                          //||
                          // pixelsPerTact >= 96 ||
                          // m_pat->m_steps != m_pat->stepsPerTact() ) )
    {
        QPixmap stepon0;
        QPixmap stepon200;
        QPixmap stepoff;
        QPixmap stepoffl;

        const int   steps = qMax(1, m_pat->m_steps);
        const int   sr    = m_pat->m_stepResolution;
        const float ws    = 16.f * 16.f / sr;
        const float w1    = qMin(12, int(ws >= 16.f ? ws - 4.f : ws - 1.f));
        const int   h1    = 24;

        // qInfo("steps=%d sr=%d ws=%f w1=%f h1=%d",steps,sr,ws,w1,h1);

        // scale step graphics to fit the beat pattern length
        stepon0   = s_stepBtnOn0->scaled(w1,
                                       h1,  // s_stepBtnOn0->height(),
                                       Qt::IgnoreAspectRatio,
                                       Qt::SmoothTransformation);
        stepon200 = s_stepBtnOn200->scaled(w1,
                                           h1,  // s_stepBtnOn200->height(),
                                           Qt::IgnoreAspectRatio,
                                           Qt::SmoothTransformation);
        stepoff   = s_stepBtnOff->scaled(w1,
                                       h1,  // s_stepBtnOff->height(),
                                       Qt::IgnoreAspectRatio,
                                       Qt::SmoothTransformation);
        stepoffl  = s_stepBtnOffLight->scaled(
                w1,
                h1,  // s_stepBtnOffLight->height(),
                Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        int den = Engine::song()->getTimeSigModel().getDenominator();
        for(int it = 0; it < steps;
            it++)  // go through all the steps in the beat pattern
        {
            Note* n = m_pat->noteAtStep(it);

            // figure out x and y coordinates for step graphic
            // const int x = TCO_BORDER_WIDTH + static_cast<int>( it * w /
            // steps ); const int y = height() - s_stepBtnOff->height() - 1;
            const int x = TCO_BORDER_WIDTH + static_cast<int>(it * ws) + 2;
            const int y = (height() - h1) / 2;

            if(n)
            {
                const int vol = n->getVolume();
                p.drawPixmap(x, y, stepoffl);
                p.drawPixmap(x, y, stepon0);
                p.setOpacity(sqrt(vol / 200.0));
                p.drawPixmap(x, y, stepon200);
                p.setOpacity(1);
            }
            else if(it % den)  // ( it / 4 ) % 2 )
            {
                p.drawPixmap(x, y, stepoffl);
            }
            else
            {
                p.drawPixmap(x, y, stepoff);
            }
        }  // end for loop

        // draw a transparent rectangle over muted patterns
        if(muted)
        {
            p.setBrush(mutedBackgroundColor());
            p.setOpacity(0.5);
            p.drawRect(0, 0, width(), height());
        }
    }

    bool frozen = m_pat->track()->isFrozen();
    paintFrozenIcon(frozen, p);

    if(!beatPattern)
    {
        paintTileTacts(current, m_pat->length().nextFullTact(), 1, bgcolor,
                       p);
    }

    // pattern name
    if(drawTextBox)
    {
        paintTextLabel(m_pat->name(), bgcolor, p);
    }

    if(!beatPattern)  //&& fixedTCOs() ) )
    {
        // inner border
        /*
        p.setPen( c.lighter( current ? 160 : 130 ) );
        p.drawRect( 1, 1, rect().right() - TCO_BORDER_WIDTH,
                rect().bottom() - TCO_BORDER_WIDTH );
        */

        // outer border
        // p.setPen( current ? c.lighter( 130 ) : c.darker( 300 ) );
        // p.drawRect( rect() );
        /*
        p.setPen(current ? c.lighter(300) : c.lighter(150));
        p.drawLine(0,0,width()-1,0);
        p.drawLine(0,1,0,height()-1);
        p.setPen(current ? c.darker(300) : c.darker(150));
        p.drawLine(0,height()-1,width()-1,height()-1);
        p.drawLine(width()-1,1,width()-1,height()-2);
        */
        paintTileBorder(current, ghost, bgcolor, p);
        paintTileLoop(p);
    }

    paintMutedIcon(m_pat->isMuted(), p);

    painter.drawPixmap(0, 0, m_paintPixmap);
}
