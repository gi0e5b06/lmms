/*
 * BBTrack.cpp - implementation of class BBTrack and bbTCO
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
#include "BBTrack.h"

#include "BBEditor.h"
#include "BBTrackContainer.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "Mixer.h"
#include "RenameDialog.h"
#include "Song.h"
//#include "SongEditor.h"
#include "TimeLineWidget.h"
#include "ToolTip.h"
#include "TrackLabelButton.h"
#include "embed.h"
#include "gui_templates.h"

#include <QColorDialog>
#include <QMenu>
#include <QPainter>

#define MASKSZ 8 * sizeof(qulonglong)

BBTrack::infoMap BBTrack::s_infoMap;

BBTCO::BBTCO(Track* _track) :
      Tile(_track, "Beat tile"), m_bbTrackIndex(-1), m_mask(nullptr)
{
    tact_t t = Engine::getBBTrackContainer()->lengthOfBB(bbTrackIndex());
    if(t > 0)
    {
        saveJournallingState(false);
        changeLength(MidiTime(t, 0));
        restoreJournallingState();
    }
    setAutoResize(false);
    setAutoRepeat(true);
}

BBTCO::BBTCO(const BBTCO& _other) :
      Tile(_other), m_bbTrackIndex(_other.m_bbTrackIndex), m_mask(nullptr)
{
    const Bitset* mask = _other.mask();
    if(mask != nullptr)
        m_mask = new Bitset(*mask);

    // changeLength(_other.length());
    // setAutoResize(_other.autoResize());
    // setAutoRepeat(_other.autoRepeat());
}

BBTCO::~BBTCO()
{
    if(m_mask)
        delete m_mask;
}

QString BBTCO::defaultName() const
{
    Track* t = track();
    if(m_bbTrackIndex >= 0
       && m_bbTrackIndex < Engine::getBBTrackContainer()->numOfBBs())
        t = BBTrack::findBBTrack(m_bbTrackIndex);
    return t->name();
}

int BBTCO::bbTrackIndex() const
{
    if(m_bbTrackIndex >= 0
       && m_bbTrackIndex < Engine::getBBTrackContainer()->numOfBBs())
        return m_bbTrackIndex;
    else
        return dynamic_cast<BBTrack*>(track())->index();
}

void BBTCO::setBBTrackIndex(int _index)
{
    qWarning("BBTCO::setBBTrackIndex o=%d n=%d", m_bbTrackIndex, _index);
    if(m_bbTrackIndex != _index && _index >= -1
       && _index < Engine::getBBTrackContainer()->numOfBBs())
    {
        bool un        = (name() == defaultName());
        m_bbTrackIndex = _index;
        Track* t       = nullptr;
        if(m_bbTrackIndex >= 0
           && m_bbTrackIndex < Engine::getBBTrackContainer()->numOfBBs())
            t = BBTrack::findBBTrack(m_bbTrackIndex);
        if(t == nullptr)
            t = track();
        qWarning("BBTCO::setBBTrackIndex t=%s", qPrintable(t->name()));
        setColor(t->color());
        setUseStyleColor(false);
        if(un)
            setName(m_bbTrackIndex >= 0 ? defaultName() : "");
        emit dataChanged();
    }
}

bool BBTCO::isEmpty() const
{
    int index = bbTrackIndex();
    for(const Track* t: Engine::getBBTrackContainer()->tracks())
        if(!t->isMuted() && !t->getTCO(index)->isEmpty())
            return false;
    return true;
}

tick_t BBTCO::unitLength() const
{
    return qMax(
            MidiTime::ticksPerTact() / 8,
            Engine::getBBTrackContainer()->beatLengthOfBB(bbTrackIndex()));
}

void BBTCO::saveSettings(QDomDocument& doc, QDomElement& element)
{
    Tile::saveSettings(doc, element);

    if(m_mask != nullptr)
    {
        qulonglong mask = 0;
        for(int i = qMin<int>(MASKSZ, m_mask->size()) - 1; i >= 0; i--)
            if(m_mask->bit(i))
                mask |= 1 << i;
        element.setAttribute("mask", mask);
    }

    /*
    element.setAttribute( "name", name() );
    //if( element.parentNode().nodeName() == "clipboarddata" )
    //{
    //	element.setAttribute( "pos", -1 );
    //}
    //else
    {
            element.setAttribute( "pos", startPosition() );
    }
    element.setAttribute( "len", length() );
    element.setAttribute( "muted", isMuted() );
    element.setAttribute( "color", color() );

    if( m_useStyleColor )
    {
            element.setAttribute( "usestyle", 1 );
    }
    else
    {
            element.setAttribute( "usestyle", 0 );
    }
    */
}

void BBTCO::loadSettings(const QDomElement& element)
{
    Tile::loadSettings(element);

    if(element.hasAttribute("mask"))
    {
        qulonglong mask = element.attribute("mask").toULongLong(nullptr);
        if(mask)
        {
            if(m_mask == nullptr)
                m_mask = new Bitset(MASKSZ, false);
            int i = 0;
            while(mask != 0)
            {
                if(mask & 1)
                    m_mask->set(i);
                mask = mask >> 1;
                i++;
            }
        }
    }

    /*
    setName( element.attribute( "name" ) );
    if( element.attribute( "pos" ).toInt() >= 0 )
    {
            movePosition( element.attribute( "pos" ).toInt() );
    }
    changeLength( element.attribute( "len" ).toInt() );
    if( element.attribute( "muted" ).toInt() != isMuted() )
    {
            toggleMute();
    }

    if( element.hasAttribute( "color" ) )
    {
            setColor( QColor( element.attribute( "color" ).toUInt() ) );
    }

    if( element.hasAttribute( "usestyle" ) )
    {
            if( element.attribute( "usestyle" ).toUInt() == 1 )
            {
                    m_useStyleColor = true;
            }
            else
            {
                    m_useStyleColor = false;
            }
    }
    else
    {
            if( m_color.rgb() == qRgb( 128, 182, 175 ) || m_color.rgb() ==
    qRgb( 64, 128, 255 ) ) // old or older default color
            {
                    m_useStyleColor = true;
            }
            else
            {
                    m_useStyleColor = false;
            }
    }
    */
}

void BBTCO::clear()
{
    if(m_mask == nullptr)
        m_mask = new Bitset(MASKSZ, false);

    for(int index = Engine::getBBTrackContainer()->tracks().size() - 1;
        index >= 0; index--)
        m_mask->set(index);

    emit dataChanged();
}

TileView* BBTCO::createView(TrackView* _tv)
{
    // qInfo("BBTCO::createView tv=%p", _tv);
    return new BBTCOView(this, _tv);
}

/*
BBTCOView::BBTCOView(Tile* _tco, TrackView* _tv) :
      TileView(_tco, _tv), m_bbTCO(dynamic_cast<BBTCO*>(_tco)),
      m_paintPixmap()
*/
BBTCOView::BBTCOView(BBTCO* _tco, TrackView* _tv) :
      TileView(_tco, _tv), m_bbTCO(_tco), m_paintPixmap()
{
    // qInfo("BBTCOView::BBTCOView %p", _tco);
    connect(_tco->track(), SIGNAL(dataChanged()), this, SLOT(update()));

    setStyle(QApplication::style());
}

BBTCOView::~BBTCOView()
{
}

QMenu* BBTCOView::buildContextMenu()
{
    QMenu* cm = new QMenu(this);
    /*
    QAction* a;

    a=
    */
    cm->addAction(embed::getIconPixmap("bb_track"),
                  tr("Open in the beat editor"), this,
                  SLOT(openInBBEditor()));
    addRemoveMuteClearMenu(cm, true, true, true);
    cm->addSeparator();
    addCutCopyPasteMenu(cm, true, true, true);

    cm->addSeparator();
    addMuteMenu(cm, !m_bbTCO->isMuted());
    addBeatMenu(cm, !m_bbTCO->isMuted()
                            && Engine::getBBTrackContainer()->numOfBBs() > 1);

    cm->addSeparator();
    addPropertiesMenu(cm, false, false);
    cm->addSeparator();
    addNameMenu(cm, true);
    cm->addSeparator();
    addColorMenu(cm, true);

    return cm;
}

void BBTCOView::addMuteMenu(QMenu* _cm, bool _enabled)
{
    QMenu* smmi = new QMenu(tr("Muted Instruments"));

    const Bitset* mask = m_bbTCO->mask();

    QAction* a;
    int      index = 0;
    for(auto track: Engine::getBBTrackContainer()->tracks())
    {
        bool muted = (mask != nullptr && index < mask->size()
                      && mask->bit(index));
        a          = smmi->addAction(track->name());
        // ,this , SLOT( setMask() ) );
        a->setData(QVariant(index));
        a->setCheckable(true);
        a->setChecked(muted);
        a->setEnabled(_enabled);
        index++;
    }

    connect(smmi, SIGNAL(triggered(QAction*)), this,
            SLOT(toggleMask(QAction*)));
    _cm->addMenu(smmi);
}

void BBTCOView::addBeatMenu(QMenu* _cm, bool _enabled)
{
    QMenu* smb = new QMenu(tr("Beat"));

    QAction*  a;
    const int n = Engine::getBBTrackContainer()->numOfBBs();
    for(int i = -1; i < n; i++)
    {
        QString s(i == -1 ? tr("Default") : BBTrack::findBBTrack(i)->name());
        a = smb->addAction(s);
        a->setData(QVariant(i));
        a->setCheckable(true);
        a->setChecked(i == m_bbTCO->m_bbTrackIndex);
        a->setEnabled(_enabled);
    }

    connect(smb, SIGNAL(triggered(QAction*)), this,
            SLOT(chooseBeat(QAction*)));
    _cm->addMenu(smb);
}

void BBTCOView::toggleMask(QAction* _a)
{
    const int index = _a->data().toInt();
    // qInfo("BBTCOView::setMask %d", index);
    Bitset* mask = m_bbTCO->mask();
    if(mask == nullptr)
    {
        mask            = new Bitset(MASKSZ, false);
        m_bbTCO->m_mask = mask;
    }
    mask->toggle(index);
    setNeedsUpdate(true);
}

void BBTCOView::chooseBeat(QAction* _a)
{
    const int index = _a->data().toInt();
    qInfo("BBTCOView::chooseBeat %d", index);
    m_bbTCO->setBBTrackIndex(index);
    setNeedsUpdate(true);
}

void BBTCOView::mouseDoubleClickEvent(QMouseEvent*)
{
    openInBBEditor();
}

void BBTCOView::paintEvent(QPaintEvent*)
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

    // QLinearGradient lingrad( 0, 0, 0, height() );

    bool muted = m_bbTCO->track()->isMuted() || m_bbTCO->isMuted();

    // state: selected, muted, default, user selected
    QColor bgcolor
            = isSelected()
                      ? selectedColor()
                      : (muted ? mutedBackgroundColor()
                               : (useStyleColor() ? (
                                          m_bbTCO->track()->useStyleColor()
                                                  ? painter.background()
                                                            .color()
                                                  : m_bbTCO->track()->color())
                                                  : color()));

    /*
    lingrad.setColorAt( 0, c.light( 130 ) );
    lingrad.setColorAt( 1, c.light( 70 ) );

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

    /*
    // bar lines
    const int lineSize = 3;
    p.setPen( c.darker( 200 ) );

    tact_t t = Engine::getBBTrackContainer()->lengthOfBB(
    m_bbTCO->bbTrackIndex() ); if( m_bbTCO->length() >
    MidiTime::ticksPerTact() && t > 0 )
    {
            for( int x = static_cast<int>( t * pixelsPerTact() );
                                                            x < width() - 2;
                    x += static_cast<int>( t * pixelsPerTact() ) )
            {
                    p.drawLine( x, TCO_BORDER_WIDTH, x, TCO_BORDER_WIDTH +
    lineSize ); p.drawLine( x, rect().bottom() - ( TCO_BORDER_WIDTH + lineSize
    ), x, rect().bottom() - TCO_BORDER_WIDTH );
            }
    }
    */

    Bitset* mask = m_bbTCO->mask();
    if(mask != nullptr)
    {
        p.setPen(bgcolor.darker(200));
        for(int y = qMin<int>(height() - 5, mask->size() - 1); y >= 0; --y)
            if(mask->bit(y))
                p.drawLine(2, 2 + y, width() - 4, 2 + y);
    }

    bool frozen = m_bbTCO->track()->isFrozen();
    paintFrozenIcon(frozen, p);

    // float
    // ppt=(width()-2*TCO_BORDER_WIDTH)/(float)m_bbTCO->length().getTact();
    // tact_t tpg=Engine::getBBTrackContainer()->...lengthOfBB(
    // m_bbTCO->bbTrackIndex() );
    // tick_t tpg = Engine::getBBTrackContainer()->beatLengthOfBB(
    //        m_bbTCO->bbTrackIndex());
    tick_t tpg = m_bbTCO->unitLength();
    paintTileTacts(false, m_bbTCO->length().nextFullTact(), tpg, bgcolor, p);

    // pattern name
    paintTextLabel(m_bbTCO->name(), bgcolor, p);

    paintTileBorder(false, false, bgcolor, p);
    paintTileLoop(p);
    paintMutedIcon(m_bbTCO->isMuted(), p);

    p.end();

    painter.drawPixmap(0, 0, m_paintPixmap);
}

void BBTCOView::openInBBEditor()
{
    Engine::getBBTrackContainer()->setCurrentBB(m_bbTCO->bbTrackIndex());

    gui->mainWindow()->toggleBBWindow();
}

/*
void BBTCOView::resetName()
{
        m_bbTCO->setName( m_bbTCO->track()->name() );
}




void BBTCOView::changeName()
{
        QString s = m_bbTCO->name();
        RenameDialog rename_dlg( s );
        rename_dlg.exec();
        m_bbTCO->setName( s );
}




void BBTCOView::changeColor()
{
        QColor new_color = QColorDialog::getColor( m_bbTCO->m_color );
        if( ! new_color.isValid() )
        {
                return;
        }
        if( isSelected() )
        {
                QVector<SelectableObject *> selected =
                                gui->songEditor()->m_editor->selectedObjects();
                for( QVector<SelectableObject *>::iterator it =
                                                        selected.begin();
                                                it != selected.end(); ++it )
                {
                        BBTCOView * bb_tcov = dynamic_cast<BBTCOView *>( *it
); if( bb_tcov )
                        {
                                bb_tcov->setColor( new_color );
                        }
                }
        }
        else
        {
                setColor( new_color );
        }
}
*/

/** \brief Makes the BB pattern use the colour defined in the stylesheet */
/*
void BBTCOView::resetColor()
{
        if( ! m_bbTCO->m_useStyleColor )
        {
                m_bbTCO->m_useStyleColor = true;
                Engine::getSong()->setModified();
                update();
        }
        BBTrack::clearLastTCOColor();
}


void BBTCOView::setColor( QColor new_color )
{
        if( new_color.rgb() != m_bbTCO->color() )
        {
                m_bbTCO->setColor( new_color );
                m_bbTCO->m_useStyleColor = false;
                Engine::getSong()->setModified();
                update();
        }
        BBTrack::setLastTCOColor( new_color );
}
*/

void BBTCOView::update()
{
    ToolTip::add(this, m_bbTCO->name());

    TileView::update();
}

// QColor * BBTrack::s_lastTCOColor = nullptr;

BBTrack::BBTrack(TrackContainer* tc) : Track(Track::BBTrack, tc)
{
    setColor(QColor("#22BBBB"));

    int bbNum       = s_infoMap.size();
    s_infoMap[this] = bbNum;

    setName(tr("Beat %1").arg(bbNum + 1));
    Engine::getBBTrackContainer()->createTCOsForBB(bbNum);
    Engine::getBBTrackContainer()->setCurrentBB(bbNum);
    Engine::getBBTrackContainer()->updateComboBox();

    connect(this, SIGNAL(nameChanged()), Engine::getBBTrackContainer(),
            SLOT(updateComboBox()));
}

BBTrack::~BBTrack()
{
    /*
      Engine::mixer()->removePlayHandlesOfTypes(
      this, PlayHandle::TypeNotePlayHandle
      | PlayHandle::TypeInstrumentPlayHandle
      | PlayHandle::TypeSamplePlayHandle);
    */

    const int bb = s_infoMap[this];
    Engine::getBBTrackContainer()->removeBB(bb);
    for(infoMap::iterator it = s_infoMap.begin(); it != s_infoMap.end(); ++it)
    {
        if(it.value() > bb)
        {
            --it.value();
        }
    }
    s_infoMap.remove(this);

    // remove us from TC so bbTrackContainer::numOfBBs() returns a smaller
    // value and thus combobox-updating in bbTrackContainer works well
    trackContainer()->removeTrack(this);
    Engine::getBBTrackContainer()->updateComboBox();
}

QString BBTrack::defaultName() const
{
    const int i = index();
    for(const Track* t: Engine::getBBTrackContainer()->tracks())
        if(!t->isMuted())
        {
            Tile* tco = t->getTCO(i);
            if(!tco->isEmpty())
                return tco->name();
        }
    return tr("Beat %1").arg(i);
}

// play _frames frames of given TCO within starting with _start
bool BBTrack::play(const MidiTime& _start,
                   const fpp_t     _frames,
                   const f_cnt_t   _offset,
                   int             _tco_num)
{
    if(isMuted())
        return false;

    if(_tco_num >= 0)
    {
        // playing in BB editor qInfo("tco_num=%d",_tco_num);
        return Engine::getBBTrackContainer()->play(_start, _frames, _offset,
                                                   s_infoMap[this], nullptr);
    }

    const Song* song = Engine::getSong();
    const float fpt  = Engine::framesPerTick();

    MidiTime start = _start;
    Tiles    tcos;
    int      n = currentLoop();
    if(song->playMode() == Song::Mode_PlaySong)
    {
        TimeLineWidget* tl = song->getPlayPos().m_timeLine;
        if(tl != nullptr)
            start = tl->adjustTime(start, n);
    }
    // getTCOsInRange(tcos, _start, _start + static_cast<int>(_frames / fpt));
    getTCOsInRange(tcos, start, start + tick_t(ceilf(float(_frames) / fpt)));

    if(tcos.size() == 0)
        return false;

    /*
    MidiTime lastPosition;
    MidiTime lastLen;
    for( Tiles::iterator it = tcos.begin(); it != tcos.end(); ++it )
    {
            if( !( *it )->isMuted() &&
                ( *it )->startPosition() >= lastPosition )
            {
                    lastPosition = ( *it )->startPosition();
                    lastLen = ( *it )->length();
            }
    }

    //qInfo("bbtrack sz=%d _start=%d lastPos=%d lastLen=%d",
    //      tcos.size(),_start,lastPosition,lastLen);
    if( _start - lastPosition < lastLen )
    {
            return Engine::getBBTrackContainer()->play( _start - lastPosition,
    _frames, _offset, s_infoMap[this] );
    }
    return false;
    */

    // MidiTime lastPosition;
    // MidiTime lastLen;
    bool played = false;
    for(Tiles::iterator it = tcos.begin(); it != tcos.end(); ++it)
    {
        if((*it)->isMuted())
            continue;

        const MidiTime& stp = (*it)->startPosition();
        const MidiTime& len = (*it)->length();
        // qInfo("bbtrack sz=%d _start=%d start=%d lastPos=%d lastLen=%d",
        //      tcos.size(),_start,start,lastPosition,lastLen);
        if(start < stp + len)
        {
            const BBTCO* tco = dynamic_cast<const BBTCO*>(*it);
            if(tco != nullptr)
            {
                const fpp_t nbf
                        = qMin<int>(_frames, (stp + len - start) * fpt);
                BBTrack*  t   = this;
                const int idx = tco->bbTrackIndex();
                if(idx != -1)
                    t = BBTrack::findBBTrack(idx);
                const int num = s_infoMap[t];
                /*
                qWarning("BBTrack::play t=%s num=%d idx=%d",
                         qPrintable(t->name()), num, idx);
                */
                played |= Engine::getBBTrackContainer()->play(
                        start - stp, nbf, _offset, num, tco->mask());
            }
        }
    }
    return played;
}

TrackView* BBTrack::createView(TrackContainerView* tcv)
{
    return new BBTrackView(this, tcv);
}

Tile* BBTrack::createTCO(const MidiTime& _pos)
{
    BBTCO* bbtco = new BBTCO(this);
    /*
    if( s_lastTCOColor )
    {
            bbtco->setColor( *s_lastTCOColor );
            bbtco->setUseStyleColor( false );
    }
    */
    return bbtco;
}

void BBTrack::saveTrackSpecificSettings(QDomDocument& _doc,
                                        QDomElement&  _this)
{
    //	_this.setAttribute( "icon", m_trackLabel->pixmapFile() );
    /*	_this.setAttribute( "current", s_infoMap[this] ==
                                            engine::bbWindow()->currentBB()
       );*/
    if(s_infoMap[this] == 0
       && _this.parentNode().parentNode().nodeName() != "clone"
       && _this.parentNode().parentNode().nodeName() != "journaldata")
    {
        ((JournallingObject*)(Engine::getBBTrackContainer()))
                ->saveState(_doc, _this);
    }
    if(_this.parentNode().parentNode().nodeName() == "clone")
    {
        _this.setAttribute("clonebbt", s_infoMap[this]);
    }
}

void BBTrack::loadTrackSpecificSettings(const QDomElement& _this)
{
    /*	if( _this.attribute( "icon" ) != "" )
            {
                    m_trackLabel->setPixmapFile( _this.attribute( "icon" ) );
            }*/

    if(_this.hasAttribute("clonebbt"))
    {
        const int src = _this.attribute("clonebbt").toInt();
        const int dst = s_infoMap[this];
        Tracks    tl  = Engine::getBBTrackContainer()->tracks();
        // copy TCOs of all tracks from source BB (at bar "src") to
        // destination TCOs (which are created if they do not exist yet)
        for(Tracks::iterator it = tl.begin(); it != tl.end(); ++it)
        {
            (*it)->getTCO(src)->copy();
            (*it)->getTCO(dst)->paste();
        }
        setName(tr("Clone of %1")
                        .arg(_this.parentNode().toElement().attribute(
                                "name")));
    }
    else
    {
        QDomNode node = _this.namedItem(TrackContainer::classNodeName());
        if(node.isElement())
        {
            ((JournallingObject*)Engine::getBBTrackContainer())
                    ->restoreState(node.toElement());
        }
    }
    /*	doesn't work yet because BBTrack-ctor also sets current bb so if
            bb-tracks are created after this function is called, this doesn't
            help at all....
            if( _this.attribute( "current" ).toInt() )
            {
                    engine::bbWindow()->setCurrentBB( s_infoMap[this] );
            }*/
}

// return pointer to BBTrack specified by _bb_num
BBTrack* BBTrack::findBBTrack(int _bb_num)
{
    /*
    for( infoMap::iterator it = s_infoMap.begin(); it != s_infoMap.end();
                                                                    ++it )
    {
            if( it.value() == _bb_num )
            {
                    return const_cast<BBTrack*>(it.key());
            }
    }
    return nullptr;
    */
    return const_cast<BBTrack*>(s_infoMap.key(_bb_num, nullptr));
}

void BBTrack::swapBBTracks(Track* _track1, Track* _track2)
{
    BBTrack* t1 = dynamic_cast<BBTrack*>(_track1);
    BBTrack* t2 = dynamic_cast<BBTrack*>(_track2);
    if(t1 != nullptr && t2 != nullptr)
    {
        qSwap(s_infoMap[t1], s_infoMap[t2]);
        Engine::getBBTrackContainer()->swapBB(s_infoMap[t1], s_infoMap[t2]);
        Engine::getBBTrackContainer()->setCurrentBB(s_infoMap[t1]);
    }
}

BBTrackView::BBTrackView(BBTrack* _bbt, TrackContainerView* _tcv) :
      TrackView(_bbt, _tcv), m_bbTrack(_bbt)
{
    setFixedHeight(32);
    // drag'n'drop with bb-tracks only causes troubles (and makes no sense
    // too), so disable it
    setAcceptDrops(false);

    TrackLabelButton* tbl
            = new TrackLabelButton(this, getTrackSettingsWidget());
    tbl->setIcon(embed::getIconPixmap("bb_track"));
    tbl->move(3, 1);
    tbl->show();
    connect(tbl, SIGNAL(clicked(bool)), this, SLOT(clickedTrackLabel()));

    setModel(_bbt);
}

BBTrackView::~BBTrackView()
{
    gui->bbWindow()->removeBBView(BBTrack::s_infoMap[m_bbTrack]);
}

bool BBTrackView::close()
{
    gui->bbWindow()->removeBBView(BBTrack::s_infoMap[m_bbTrack]);
    return TrackView::close();
}

void BBTrackView::addSpecificMenu(QMenu* _cm, bool _enabled)
{
    // TODO Select optional tconum
}

void BBTrackView::clickedTrackLabel()
{
    Engine::getBBTrackContainer()->setCurrentBB(m_bbTrack->index());
    // gui->bbWindow()->show();
    gui->mainWindow()->toggleBBWindow();
}
