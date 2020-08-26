/*
 * TrackView.cpp -
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

/** \file TrackView.cpp
 *  \brief View base class of a track
 */

/*
 * \mainpage Track classes
 * \section introduction Introduction
 * \todo fill this out
 */

#include "TrackView.h"

#include "AutomationEditor.h"
#include "AutomationPattern.h"
#include "AutomationTrack.h"
#include "BBEditor.h"
#include "BBTrack.h"
#include "BBTrackContainer.h"
#include "CaptionMenu.h"
#include "Clipboard.h"
#include "ConfigManager.h"
#include "Engine.h"
#include "GuiApplication.h"
//#include "FxMixerView.h"
//#include "MainWindow.h"
#include "Mixer.h"
#include "Pattern.h"
#include "PixmapButton.h"
//#include "ProjectJournal.h"
#include "Backtrace.h"
#include "RenameDialog.h"
#include "SampleTrack.h"
#include "Song.h"
#include "SongEditor.h"
#include "StringPairDrag.h"
#include "TextFloat.h"
#include "TimeLineWidget.h"
#include "ToolTip.h"
#include "debug.h"
#include "embed.h"
#include "gui_templates.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QUuid>

#include <cassert>
#include <cmath>

// ===========================================================================
// trackContentWidget
// ===========================================================================
/*! \brief Create a new trackContentWidget
 *
 *  Creates a new track content widget for the given track.
 *  The content widget comprises the 'grip bar' and the 'tools' button
 *  for the track's context menu.
 *
 * \param parent The parent track.
 */
TrackContentWidget::TrackContentWidget(TrackView* parent) :
      QWidget(parent), m_trackView(parent), m_darkerColor(Qt::SolidPattern),
      m_lighterColor(Qt::SolidPattern), m_gridColor(Qt::SolidPattern),
      m_embossColor(Qt::SolidPattern)
{
    setAcceptDrops(true);
    setStyle(QApplication::style());
    updateBackground();

    connect(parent->trackContainerView(),
            SIGNAL(positionChanged(const MidiTime&)), this,
            SLOT(changePosition(const MidiTime&)));

    connect(track()->frozenModel(), SIGNAL(dataChanged()), this,
            SLOT(update()));
}

/*! \brief Destroy this trackContentWidget
 *
 *  Destroys the trackContentWidget.
 */
TrackContentWidget::~TrackContentWidget()
{
}

bool TrackContentWidget::isFixed() const
{
    return m_trackView->isFixed();
}

real_t TrackContentWidget::pixelsPerTact() const
{
    return m_trackView->pixelsPerTact();
}

const TileViews& TrackContentWidget::tileViews() const
{
    TileViews r;
    for(QObject* o: children())
    {
        TileView* tv = dynamic_cast<TileView*>(o);
        if(tv != nullptr)
            r.append(tv);
    }
    if(r != m_tileViews)
        qWarning("TrackContentWidget::tileViews");

    return m_tileViews;
}

void TrackContentWidget::updateBackground()
{
    /*
    //const int tactsPerBar = 4;
    const int tactsPerBar=Engine::song()->getTimeSigModel().getNumerator();
    const TrackContainerView * tcv = m_trackView->trackContainerView();

    // Assume even-pixels-per-tact. Makes sense, should be like this anyways
    int ppt = static_cast<int>( tcv->pixelsPerTact() );

    int w = ppt * tactsPerBar;
    int h = height();
    m_background = QPixmap( w * 2, height() );
    QPainter pmp( &m_background );

    pmp.fillRect( 0, 0, w, h, darkerColor() );
    pmp.fillRect( w, 0, w , h, lighterColor() );

    // draw lines
    // vertical lines
    pmp.setPen( QPen( gridColor(), 1 ) );
    for( real_t x = 0; x < w * 2; x += ppt )
    {
            pmp.drawLine( QLineF( x, 0.0, x, h ) );
    }

    pmp.setPen( QPen( embossColor(), 1 ) );
    for( real_t x = 1.0; x < w * 2; x += ppt )
    {
            pmp.drawLine( QLineF( x, 0.0, x, h ) );
    }

    // horizontal line
    pmp.setPen( QPen( gridColor(), 1 ) );
    pmp.drawLine( 0, h-1, w*2, h-1 );

    pmp.end();
    */

    // Force redraw
    update();
}

/*! \brief Adds a TileView to this widget.
 *
 *  Adds a(nother) TileView to our list of views.  We also
 *  check that our position is up-to-date.
 *
 * \param tcov The TileView to add.
 */
void TrackContentWidget::addTCOView(TileView* _tv)
{
    Tile* tile = _tv->tile();
    m_tileViews.append(_tv);
    tile->saveJournallingState(false);
    changePosition();
    tile->restoreJournallingState();
}

/*! \brief Removes the given TileView to this widget.
 *
 *  Removes the given TileView from our list of views.
 *
 * \param tcov The TileView to add.
 */
void TrackContentWidget::removeTCOView(TileView* _tv)
{
    /*
      TileViews::iterator it
            = qFind(m_tileViews.begin(), m_tileViews.end(), _tv);
    if(it != m_tileViews.end())
    {
        m_tileViews.erase(it);
        Engine::song()->setModified();
    }
    */
    if(m_tileViews.removeOne(_tv))
        Engine::song()->setModified();
}

/*! \brief Update ourselves by updating all the tCOViews attached.
 *
 */
void TrackContentWidget::update()
{
    for(TileView* tv: m_tileViews)
    {
        if(tv == nullptr)
            continue;
        tv->setFixedHeight(height() - 1);
        tv->update();
    }
    QWidget::update();
}

// resposible for moving track-content-widgets to appropriate position after
// change of visible viewport
/*! \brief Move the trackContentWidget to a new place in time
 *
 * \param newPos The MIDI time to move to.
 */
void TrackContentWidget::changePosition(const MidiTime& newPos)
{
    m_tileViews.removeAll(nullptr);

    if(m_trackView->trackContainerView()
       == gui->bbWindow()->trackContainerView())
    {
        const int curBB = Engine::getBBTrackContainer()->currentBB();
        setUpdatesEnabled(false);

        // show TCO for current BB and hide others
        for(TileView* tv: m_tileViews)
        {
            Tile* tile = tv->tile();
            if(tile != nullptr && tile->startPosition().getTact() == curBB)
            {
                tv->move(0, tv->y());
                tv->raise();
                tv->show();
            }
            else
            {
                tv->lower();
                tv->hide();
            }
        }

        setUpdatesEnabled(true);
        return;
    }

    MidiTime pos = newPos;
    if(pos < 0)
    {
        pos = m_trackView->trackContainerView()->currentPosition();
    }

    const int begin = pos;
    // const int end = endPosition(pos);
    const real_t ppt = pixelsPerTact();

    setUpdatesEnabled(false);
    for(TileView* tcov: m_tileViews)
    {
        Tile* tco = tcov->tile();
        if(tco == nullptr)
            continue;

        tco->changeLength(tco->length());

        const int ts = tco->startPosition();
        /*
        ??? useful?
        const int te = tco->endPosition() - 3;
        if((ts >= begin && ts <= end) || (te >= begin && te <= end)
           || (ts <= begin && te >= end))
        */
        {
            tcov->move(static_cast<int>((ts - begin) * ppt
                                        / MidiTime::ticksPerTact()),
                       tcov->y());
            if(!tcov->isVisible())
                tcov->show();
        }
        /*
        else
        {
            tcov->move(-tcov->width() - 10, tcov->y());
        }
        */
    }
    setUpdatesEnabled(true);

    // redraw background
    //	update();
}

/*! \brief Return the position of the trackContentWidget in Tacts.
 *
 * \param mouseX the mouse's current X position in pixels.
 */
MidiTime TrackContentWidget::getPosition(int mouseX)
{
    TrackContainerView* tv = m_trackView->trackContainerView();
    return MidiTime(tv->currentPosition()
                    + mouseX * MidiTime::ticksPerTact()
                              / static_cast<int>(pixelsPerTact()));
}

/*! \brief Returns whether a selection of TCOs can be pasted into this
 *
 * \param tcoPos the position of the TCO slot being pasted on
 * \param de the DropEvent generated
 */
bool TrackContentWidget::canPasteSelection(MidiTime         tcoPos,
                                           const QMimeData* mimeData)
{
    Track* t = track();
    if(t->isFrozen())
        return false;

    QString type  = StringPairDrag::decodeKey(mimeData);
    QString value = StringPairDrag::decodeValue(mimeData);

    // We can only paste into tracks of the same type
    if(type != ("tco_" + QString::number(t->type()))
       || isFixed())  // trackContainerView()->fixedTCOs() == true )
        return false;

    // value contains XML needed to reconstruct TCOs and place them
    DataFile dataFile(value.toUtf8());

    // Extract the metadata and which TCO was grabbed
    QDomElement metadata
            = dataFile.content().firstChildElement("copyMetadata");
    QDomAttr tcoPosAttr     = metadata.attributeNode("grabbedTCOPos");
    MidiTime grabbedTCOPos  = tcoPosAttr.value().toInt();
    MidiTime grabbedTCOTact = MidiTime(grabbedTCOPos.getTact(), 0);

    // Extract the track index that was originally clicked
    QDomAttr  tiAttr            = metadata.attributeNode("initialTrackIndex");
    const int initialTrackIndex = tiAttr.value().toInt();

    // Get the current track's index
    const Tracks tracks            = t->trackContainer()->tracks();
    const int    currentTrackIndex = tracks.indexOf(t);

    // Don't paste if we're on the same tact
    if(tcoPos == grabbedTCOTact && currentTrackIndex == initialTrackIndex)
        return false;

    // Extract the tco data
    QDomElement  tcoParent = dataFile.content().firstChildElement("tcos");
    QDomNodeList tcoNodes  = tcoParent.childNodes();

    // Determine if all the TCOs will land on a valid track
    for(int i = 0; i < tcoNodes.length(); i++)
    {
        QDomElement tcoElement = tcoNodes.item(i).toElement();
        int         trackIndex
                = tcoElement.attributeNode("trackIndex").value().toInt();
        int finalTrackIndex
                = trackIndex + currentTrackIndex - initialTrackIndex;

        // Track must be in TrackContainer's tracks
        if(finalTrackIndex < 0 || finalTrackIndex >= tracks.size())
            return false;

        // Track must be of the same type
        Track* startTrack = tracks.at(trackIndex);
        Track* endTrack   = tracks.at(finalTrackIndex);
        if(startTrack->type() != endTrack->type())
            return false;
    }

    return true;
}

/*! \brief Pastes a selection of TCOs onto the track
 *
 * \param tcoPos the position of the TCO slot being pasted on
 * \param de the DropEvent generated
 */
bool TrackContentWidget::pasteSelection(MidiTime tcoPos, QDropEvent* de)
{
    if(canPasteSelection(tcoPos, de->mimeData()) == false)
        return false;

    QString type  = StringPairDrag::decodeKey(de);
    QString value = StringPairDrag::decodeValue(de);

    track()->addJournalCheckPoint();

    // value contains XML needed to reconstruct TCOs and place them
    DataFile dataFile(value.toUtf8());

    // Extract the tco data
    QDomElement  tcoParent = dataFile.content().firstChildElement("tcos");
    QDomNodeList tcoNodes  = tcoParent.childNodes();

    // Extract the track index that was originally clicked
    QDomElement metadata
            = dataFile.content().firstChildElement("copyMetadata");
    QDomAttr tiAttr            = metadata.attributeNode("initialTrackIndex");
    int      initialTrackIndex = tiAttr.value().toInt();
    QDomAttr tcoPosAttr        = metadata.attributeNode("grabbedTCOPos");
    MidiTime grabbedTCOPos     = tcoPosAttr.value().toInt();
    MidiTime grabbedTCOTact    = MidiTime(grabbedTCOPos.getTact(), 0);

    // Snap the mouse position to the beginning of the dropped tact, in ticks
    const Tracks tracks            = track()->trackContainer()->tracks();
    const int    currentTrackIndex = tracks.indexOf(track());

    bool wasSelection = m_trackView->trackContainerView()
                                ->rubberBand()
                                ->selectedObjects()
                                .count();

    // Unselect the old group
    const SelectableObjects so
            = m_trackView->trackContainerView()->selectedObjects();
    for(SelectableObject* o: so)
        o->setSelected(false);

    // TODO -- Need to draw the hovericon either way, or ghost the TCOs
    // onto their final position.

    for(int i = 0; i < tcoNodes.length(); i++)
    {
        QDomElement outerTCOElement = tcoNodes.item(i).toElement();
        QDomElement tcoElement      = outerTCOElement.firstChildElement();

        int trackIndex
                = outerTCOElement.attributeNode("trackIndex").value().toInt();
        int finalTrackIndex
                = trackIndex + (currentTrackIndex - initialTrackIndex);
        Track* t = tracks.at(finalTrackIndex);

        // Compute the final position by moving the tco's pos by
        // the number of tacts between the first TCO and the mouse drop TCO
        MidiTime oldPos  = tcoElement.attributeNode("pos").value().toInt();
        MidiTime offset  = oldPos - MidiTime(oldPos.getTact(), 0);
        MidiTime oldTact = MidiTime(oldPos.getTact(), 0);
        MidiTime delta   = offset + (oldTact - grabbedTCOTact);
        MidiTime pos     = tcoPos + delta;

        Tile* tco = t->createTCO();
        tco->restoreState(tcoElement);
        tco->movePosition(pos);
        if(wasSelection)
            tco->selectViewOnCreate(true);

        // check tco name, if the same as source track name dont copy
        if(tco->name() == tracks[trackIndex]->name())
            tco->setName("");
    }

    AutomationPattern::resolveAllIDs();
    return true;
}

/*! \brief Respond to a drag enter event on the trackContentWidget
 *
 * \param dee the Drag Enter Event to respond to
 */
void TrackContentWidget::dragEnterEvent(QDragEnterEvent* dee)
{
    if(track()->isFrozen())
    {
        dee->ignore();
        return;
    }

    MidiTime tcoPos = MidiTime(getPosition(dee->pos().x()).getTact(), 0);
    if(canPasteSelection(tcoPos, dee->mimeData()) == false)
    {
        dee->ignore();
    }
    else
    {
        StringPairDrag::processDragEnterEvent(
                dee, "tco_" + QString::number(track()->type()));
    }
}

/*! \brief Respond to a drop event on the trackContentWidget
 *
 * \param de the Drop Event to respond to
 */
void TrackContentWidget::dropEvent(QDropEvent* de)
{
    if(track()->isFrozen())
    {
        de->ignore();
        return;
    }

    MidiTime tcoPos = MidiTime(getPosition(de->pos().x()).getTact(), 0);
    if(pasteSelection(tcoPos, de) == true)
    {
        de->accept();
    }
}

/*! \brief Respond to a mouse press on the trackContentWidget
 *
 * \param me the mouse press event to respond to
 */
void TrackContentWidget::mousePressEvent(QMouseEvent* me)
{
    if(track()->isFrozen())
    {
        me->ignore();
        return;
    }

    if(me->button() == Qt::MiddleButton
       && !m_trackView->trackContainerView()->fixedTCOs())
    {
        // inject from system selection
        Track* t = track();
        t->saveJournallingState(false);

        MidiTime pos = MidiTime(getPosition(me->pos().x()).getTact(), 0);
        Tile*    tco = t->createTCO();
        QString  nn  = tco->nodeName();

        tco->saveJournallingState(false);
        tco->movePosition(pos);
        bool ok = Selection::has(nn);
        if(ok)
        {
            ok = Selection::inject(tco);
        }

        if(!ok)
        {
            qWarning("Warning: invalid pattern type for this track");
            tco->restoreJournallingState();
            tco->deleteLater();
        }
        else
        {
            // tco->updateLength();
            tco->movePosition(pos);
            // tco->selectViewOnCreate( true );
            // if(t->name()==tco->name()) tco->setName( "" );
            AutomationPattern::resolveAllIDs();
            tco->restoreJournallingState();
        }

        t->restoreJournallingState();
    }
    else if(m_trackView->trackContainerView()->allowRubberband() == true)
    {
        QWidget::mousePressEvent(me);
    }
    else if(me->modifiers() & Qt::ShiftModifier)
    {
        QWidget::mousePressEvent(me);
    }
    else if(me->button() == Qt::LeftButton
            && !m_trackView->trackContainerView()->fixedTCOs())
    {
        SelectableObjects so = m_trackView->trackContainerView()
                                       ->rubberBand()
                                       ->selectedObjects();
        for(SelectableObject* o: so)
            o->setSelected(false);

        track()->addJournalCheckPoint();
        const MidiTime pos
                = getPosition(me->x()).getTact() * MidiTime::ticksPerTact();
        Tile* tco = track()->createTCO();

        tco->saveJournallingState(false);
        tco->movePosition(pos);
        tco->restoreJournallingState();
    }
}

/*! \brief Repaint the trackContentWidget on command
 *
 * \param pe the Paint Event to respond to
 */
void TrackContentWidget::paintEvent(QPaintEvent* pe)
{
    QPainter p(this);

    // Assume even-pixels-per-tact. Makes sense, should be like this anyways
    /*const*/ TrackContainerView* tcv = m_trackView->trackContainerView();

    // Don't draw background on BB-Editor
    // if( tcv != gui->bbWindow()->trackContainerView() )
    if(!isFixed())
    {
        /*
        p.drawTiledPixmap( rect(), m_background, QPoint(
                        tcv->currentPosition().getTact() * ppt, 0 ) );
        */
        paintGrid(p, tcv->currentPosition(), pixelsPerTact(),
                  tcv->barViews());

        paintLoop(p, tcv->currentPosition(), pixelsPerTact());
    }

    if(track()->isFrozen())
    {
        p.fillRect(0, height() - 9, width() - 1, 7,
                   QBrush(Qt::cyan, Qt::BDiagPattern));
        // p.fillRect(0,0,width()-1,height()-1,
        //           QBrush(QColor(0,160,160,48),Qt::SolidPattern));
    }
}

void TrackContentWidget::paintLoop(QPainter&       p,
                                   const MidiTime& t0,
                                   const real_t    ppt)
{
    const Track* t = track();
    const int    n = t->currentLoop();
    if(n < 0 || n >= TimeLineWidget::NB_LOOPS)
        return;

    const Song*           song = Engine::song();
    const TimeLineWidget* tl
            = song->getPlayPos(Song::Mode_PlaySong).m_timeLine;
    if(tl == nullptr)
        return;

    const MidiTime& start = tl->loopBegin(n);
    const MidiTime& end   = tl->loopEnd(n);
    const QRect     r     = rect();

    const int xstart
            = r.x()
              + qMax<int>(0, ppt * (start - t0) / MidiTime::ticksPerTact());
    const int xend = r.x()
                     + qMin<int>(r.width(),
                                 ppt * (end - t0) / MidiTime::ticksPerTact());
    if(xstart >= r.x() + r.width() || xend < r.x())
        return;

    const int wc = xend - xstart;
    const int yc = r.y() + r.height() / 2;
    QBrush    bg = Qt::yellow;
    p.fillRect(xstart, yc, wc, 5, bg);
}

void TrackContentWidget::paintGrid(QPainter&                         p,
                                   const MidiTime&                   t0,
                                   const real_t                      ppt,
                                   const QVector<QPointer<BarView>>& barViews)
{
    const QRect r    = rect();
    const int   wc   = ppt;
    const int   hc   = r.height();
    const int   y    = r.y();
    const int   xmin = r.x();
    const int   xmax = r.x() + r.width();
    tact_t      tact = t0.getTact();

    for(int x = xmin; x < xmax; x += wc, tact++)
    {
        bool                    sign = ((tact / 4) % 2 == 0);
        const QPointer<BarView> bv = ((tact >= 0) && (tact < barViews.size()))
                                             ? barViews.at(tact)
                                             : nullptr;
        paintCell(p, x, y, wc, hc, bv, sign);
    }
}

void TrackContentWidget::paintCell(QPainter&                p,
                                   int                      xc,
                                   int                      yc,
                                   int                      wc,
                                   int                      hc,
                                   const QPointer<BarView>& barView,
                                   bool                     defaultSign)
{
    bool sign = defaultSign;
    if(barView != nullptr)
        sign = barView->sign();
    QBrush bg = (sign ? darkerColor() : lighterColor());
    p.fillRect(xc, yc, wc, hc, bg);

    // vertical line
    p.setPen(QPen(embossColor(), 1));
    p.drawLine(QLineF(xc + 1, yc, xc + 1, yc + hc - 1));

    p.setPen(QPen(gridColor(), 1));
    p.drawLine(QLineF(xc, yc, xc, yc + hc - 1));

    // horizontal line
    p.setPen(QPen(gridColor(), 1));
    p.drawLine(xc, yc + hc - 1, xc + wc - 1, yc + hc - 1);

    if(barView != nullptr)
    {
        const QColor& fg = barView->color();
        if(fg != Qt::transparent)
        {
            p.fillRect(xc + 1, yc, wc - 1, hc, QBrush(fg, Qt::Dense6Pattern));

            switch(barView->type())
            {
                case BarView::START:
                    p.setPen(QPen(fg, 1));
                    p.drawLine(QLineF(xc, yc, xc, yc + hc - 1));
                    p.drawLine(QLineF(xc + 1, yc, xc + 1, yc + hc - 1));
                    break;
                case BarView::END:
                    p.setPen(QPen(fg, 1));
                    p.drawLine(QLineF(xc + wc - 1, yc, xc + wc - 1,
                                      yc + hc - 1));
                    p.drawLine(QLineF(xc + wc - 2, yc, xc + wc - 2,
                                      yc + hc - 1));
                    break;
                default:
                    break;
            }
        }
    }
}

/*! \brief Updates the background tile pixmap on size changes.
 *
 * \param resizeEvent the resize event to pass to base class
 */
void TrackContentWidget::resizeEvent(QResizeEvent* resizeEvent)
{
    // Update backgroud
    updateBackground();
    // Force redraw
    QWidget::resizeEvent(resizeEvent);
}

/*! \brief Return the track shown by the trackContentWidget
 *
 */
Track* TrackContentWidget::track()
{
    return m_trackView->track();
}

/*! \brief Return the end position of the trackContentWidget in Tacts.
 *
 * \param posStart the starting position of the Widget (from getPosition())
 */
MidiTime TrackContentWidget::endPosition(const MidiTime& posStart)
{
    const real_t ppt = pixelsPerTact();
    const int    w   = width();
    return posStart + static_cast<int>(w * MidiTime::ticksPerTact() / ppt);
}

// qproperty access methods
//! \brief CSS theming qproperty access method
QBrush TrackContentWidget::darkerColor() const
{
    return m_darkerColor;
}

//! \brief CSS theming qproperty access method
QBrush TrackContentWidget::lighterColor() const
{
    return m_lighterColor;
}

//! \brief CSS theming qproperty access method
QBrush TrackContentWidget::gridColor() const
{
    return m_gridColor;
}

//! \brief CSS theming qproperty access method
QBrush TrackContentWidget::embossColor() const
{
    return m_embossColor;
}

//! \brief CSS theming qproperty access method
void TrackContentWidget::setDarkerColor(const QBrush& c)
{
    m_darkerColor = c;
}

//! \brief CSS theming qproperty access method
void TrackContentWidget::setLighterColor(const QBrush& c)
{
    m_lighterColor = c;
}

//! \brief CSS theming qproperty access method
void TrackContentWidget::setGridColor(const QBrush& c)
{
    m_gridColor = c;
}

//! \brief CSS theming qproperty access method
void TrackContentWidget::setEmbossColor(const QBrush& c)
{
    m_embossColor = c;
}

// ===========================================================================
// trackOperationsWidget
// ===========================================================================

QPixmap* TrackOperationsWidget::s_grip = nullptr; /*!< grip pixmap */

/*! \brief Create a new trackOperationsWidget
 *
 * The trackOperationsWidget is the grip and the mute button of a track.
 *
 * \param parent the trackView to contain this widget
 */
TrackOperationsWidget::TrackOperationsWidget(TrackView* parent) :
      QWidget(parent),    /*!< The parent widget */
      m_trackView(parent) /*!< The parent track view */
{
    if(s_grip == nullptr)
    {
        s_grip = new QPixmap(embed::getPixmap("track_op_grip"));
    }

    ToolTip::add(this, tr("Press <%1> while clicking on move-grip "
                          "to begin a new drag'n'drop-action.")
                               .arg(UI_CTRL_KEY));

    QPointer<CaptionMenu> toMenu
            = new CaptionMenu(m_trackView->track()->displayName(), this);
    // QMenu* toMenu = new QMenu(this);
    // toMenu->setFont(pointSize<9>(toMenu->font()));
    connect(toMenu, SIGNAL(aboutToShow()), this, SLOT(updateMenu()));

    // setObjectName("automationEnabled"); ??? TMP?

    m_trackOps = new QPushButton(this);
    m_trackOps->setGeometry(11, 6, 10, 20);
    m_trackOps->setIcon(embed::getIcon("menu"));
    m_trackOps->setFocusPolicy(Qt::NoFocus);
    m_trackOps->setMenu(toMenu);
    ToolTip::add(m_trackOps, tr("Actions for this track"));

    m_muteBtn = new PixmapButton(this, tr("Mute"));
    m_muteBtn->setActiveGraphic(embed::getPixmap("led_off"));
    m_muteBtn->setInactiveGraphic(embed::getPixmap("led_green"));
    m_muteBtn->setCheckable(true);
    ToolTip::add(m_muteBtn, tr("Mute this track"));

    m_soloBtn = new PixmapButton(this, tr("Solo"));
    m_soloBtn->setActiveGraphic(embed::getPixmap("led_magenta"));
    m_soloBtn->setInactiveGraphic(embed::getPixmap("led_off"));
    m_soloBtn->setCheckable(true);
    ToolTip::add(m_soloBtn, tr("Solo"));

    m_frozenBtn = new PixmapButton(this, tr("Frozen"));
    m_frozenBtn->setActiveGraphic(embed::getPixmap("led_blue"));
    m_frozenBtn->setInactiveGraphic(embed::getPixmap("led_off"));
    m_frozenBtn->setCheckable(true);
    ToolTip::add(m_frozenBtn, tr("Frozen"));

    m_clippingBtn = new PixmapButton(this, tr("Clipping"));
    m_clippingBtn->setActiveGraphic(embed::getPixmap("led_red"));
    m_clippingBtn->setInactiveGraphic(embed::getPixmap("led_off"));
    m_clippingBtn->setCheckable(true);
    m_clippingBtn->setBlinking(true);
    ToolTip::add(m_clippingBtn, tr("Clipping"));

    if(ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt())
    {
        /*
        m_muteBtn->setGeometry(39, 0, 10, 14);
        m_soloBtn->setGeometry(39, 16, 10, 14);
        m_frozenBtn->setGeometry(52, 0, 10, 14);
        m_clippingBtn->setGeometry(52, 16, 10, 14);
        */
        m_muteBtn->setGeometry(39, 4, 16, 10);
        m_soloBtn->setGeometry(39, 14, 16, 10);
        m_frozenBtn->setGeometry(55, 4, 16, 10);
        m_clippingBtn->setGeometry(55, 14, 16, 10);
    }
    else
    {
        m_muteBtn->move(39, 4);      // setGeometry(45, 4,16,14);
        m_soloBtn->move(39, 15);     // setGeometry(62, 4,16,14);
        m_clippingBtn->move(57, 4);  // 54,4 setGeometry(62,18,16,14);
        m_frozenBtn->move(57, 15);   // 54,18 setGeometry(62,18,16,14);
    }

    //(m_trackView->track()->type()!=Track::InstrumentTrack)||
    if((m_trackView->track()->trackContainer()
        == (TrackContainer*)Engine::getBBTrackContainer()))
        m_frozenBtn->setVisible(false);

    connect(this, SIGNAL(trackRemovalScheduled(TrackView*)),
            m_trackView->trackContainerView(),
            SLOT(deleteTrackView(TrackView*)), Qt::QueuedConnection);
}

/*! \brief Destroy an existing trackOperationsWidget
 *
 */
TrackOperationsWidget::~TrackOperationsWidget()
{
}

/*! \brief Respond to trackOperationsWidget mouse events
 *
 *  If it's the left mouse button, and Ctrl is held down, and we're
 *  not a Beat+Bassline Editor track, then start a new drag event to
 *  copy this track.
 *
 *  Otherwise, ignore all other events.
 *
 *  \param me The mouse event to respond to.
 */
void TrackOperationsWidget::mousePressEvent(QMouseEvent* me)
{
    if(me->button() == Qt::LeftButton && me->modifiers() & Qt::ControlModifier
       && m_trackView->track()->type() != Track::BBTrack)
    {
        DataFile dataFile(DataFile::DragNDropData);
        m_trackView->track()->saveState(dataFile, dataFile.content());
        new StringPairDrag(
                QString("track_%1").arg(m_trackView->track()->type()),
                dataFile.toString(),
#if(QT_VERSION >= 0x50000)
                m_trackView->getTrackSettingsWidget()->grab(),
#else
                QPixmap::grabWidget(m_trackView->getTrackSettingsWidget()),
#endif
                this);
    }
    else if(me->button() == Qt::LeftButton)
    {
        // track-widget (parent-widget) initiates track-move
        me->ignore();
    }
}

/*! \brief Repaint the trackOperationsWidget
 *
 *  If we're not moving, and in the Beat+Bassline Editor, then turn
 *  automation on or off depending on its previous state and show
 *  ourselves.
 *
 *  Otherwise, hide ourselves.
 *
 *  \todo Flesh this out a bit - is it correct?
 *  \param pe The paint event to respond to
 */
void TrackOperationsWidget::paintEvent(QPaintEvent* pe)
{
    QPainter p(this);
    QRect    r = rect();
    p.fillRect(r, palette().brush(QPalette::Background));

    if(m_trackView->isMovingTrack())
    {
        // p.setPen(Qt::red);
        // p.drawLine(r.x(),r.y(),r.x(),r.y()+r.height()-1);
        /*
        p.drawLine(r.x()  ,r.y()+1,r.x()  ,r.y()+r.height()-2);
        p.drawLine(r.x()+2,r.y()+1,r.x()+2,r.y()+r.height()-2);
        p.drawLine(r.x()+3,r.y()+1,r.x()+3,r.y()+r.height()-2);
        p.drawLine(r.x()+6,r.y()+1,r.x()+6,r.y()+r.height()-2);
        */
        p.fillRect(r.x() + 1, r.y() + 1, 8, r.height() - 1,
                   QColor(128, 0, 0));
    }

    /*
      if( m_trackView->isMovingTrack() == false )
    {
    */
    int y = 2;
    while(y < height())
    {
        p.drawPixmap(2, y, *s_grip);
        y += s_grip->height();
    }
    /*
      m_trackOps->show();
      //m_muteBtn->show();
    }
    else
    {
            m_trackOps->hide();
            m_muteBtn->hide();
    }
    */

    // p.setPen(Qt::yellow);
    // p.drawRect(0,0,width()-1,height()-1);
}

/*! \brief Clone this track
 *
 */
void TrackOperationsWidget::cloneTrack()
{
    TrackContainerView* tcView       = m_trackView->trackContainerView();
    Track*              newTrack     = m_trackView->track()->clone();
    TrackView*          newTrackView = tcView->createTrackView(newTrack);
    int                 index = tcView->trackViews().indexOf(m_trackView);
    int                 i     = tcView->trackViews().size();
    while(i > index + 1)
    {
        tcView->moveTrackView(newTrackView, i - 1);
        i--;
    }
    // tcView->moveTrackView(newTrackView,index);

    if(newTrack->isSolo())
        newTrack->setSolo(false);
    if(newTrack->isFrozen())
        newTrack->setFrozen(false);
    if(newTrack->isClipping())
        newTrack->setClipping(false);
    newTrack->cleanFrozenBuffer();
}

/*! \brief Clone this track but without the tiles
 *
 */
void TrackOperationsWidget::spawnTrack()
{
    TrackContainerView* tcView       = m_trackView->trackContainerView();
    Track*              newTrack     = m_trackView->track()->clone();
    TrackView*          newTrackView = tcView->createTrackView(newTrack);
    int                 index = tcView->trackViews().indexOf(m_trackView);
    int                 i     = tcView->trackViews().size();
    while(i > index + 1)
    {
        tcView->moveTrackView(newTrackView, i - 1);
        i--;
    }
    // tcView->moveTrackView(newTrackView,index);

    if(newTrack->isSolo())
        newTrack->setSolo(false);
    if(newTrack->isFrozen())
        newTrack->setFrozen(false);
    if(newTrack->isClipping())
        newTrack->setClipping(false);
    newTrack->cleanFrozenBuffer();

    QCoreApplication::sendPostedEvents();

    newTrack->lockTrack();
    newTrack->deleteTCOs();
    auto bbTC = Engine::getBBTrackContainer();
    if(newTrack->trackContainer() == bbTC)
        bbTC->createTCOsForBB(newTrack);
    newTrack->unlockTrack();
}

/*! \brief Clear this track - clears all TCOs from the track */
void TrackOperationsWidget::clearTrack()
{
    Track* t = m_trackView->track();
    t->lockTrack();
    t->deleteTCOs();
    auto bbTC = Engine::getBBTrackContainer();
    if(t->trackContainer() == bbTC)
        bbTC->createTCOsForBB(t);
    t->unlockTrack();
}

/*! \brief Split the beat
 *  In the SongEditor, create a new track for each instrument in the beat
 */
void TrackOperationsWidget::splitTrack()
{
    /*const*/  // TrackContainerView*
               // tcview=m_trackView->trackContainerView();

    /*const*/ Track* t   = m_trackView->track();  // the bbtrack in SongEditor
    BBTrack*         bbt = static_cast<BBTrack*>(t);
    if(!bbt)
    {
        qCritical("TrackOperationsWidget::splitTrack bbt==null");
        return;
    }

    const int                   newidxbb = bbt->ownBBTrackIndex();
    /*const*/ BBTrackContainer* bbtc     = Engine::getBBTrackContainer();
    const int                   oldidxbb = bbtc->currentBB();

    // qInfo("TrackOperationsWidget::splitTrack start splitting");
    BBEditor* bbtcv = gui->bbWindow()->trackContainerView();
    for(TrackView* tv: bbtcv->trackViews())
    {
        // qInfo("TrackOperationsWidget::splitTrack isolate track
        // %s",qPrintable(tv->track()->name()));
        bbtc->setCurrentBB(newidxbb);

        TrackOperationsWidget* tow = tv->getTrackOperationsWidget();

        if(!tow)
        {
            qCritical("TrackOperationsWidget::split tow=null!!!");
            continue;
        }

        tow->isolateTrack();
    }

    bbtc->setCurrentBB(oldidxbb);
}

/*! \brief Isolate this track
 *  In the BBEditor, create a new pattern with only this track
 */
void TrackOperationsWidget::isolateTrack()
{
    /*const*/ TrackContainerView* tcview = m_trackView->trackContainerView();
    /*const*/ Track* instr = m_trackView->track();  // the inst. in BBEditor
    /*const*/ BBTrackContainer* bbtc     = Engine::getBBTrackContainer();
    const int                   idxinstr = bbtc->tracks().indexOf(instr);
    if(idxinstr < 0 || idxinstr > bbtc->tracks().size() - 1)
        qWarning("TrackOperationsWidget::isolateTrack#0 idxinstr=%d",
                 idxinstr);

    int oldidxbb = bbtc->currentBB();
    // if(oldidxbb < 0 || oldidxbb > bbtc->numOfBBs() - 1)
    //    qWarning("TrackOperationsWidget::isolateTrack#1 oldidxbb=%d",
    //             oldidxbb);

    /*const*/ BBTrack* oldbbt = BBTrack::findBBTrack(oldidxbb);
    if(oldbbt == nullptr)
    {
        qCritical("TrackOperationsWidget::isolateTrack oldbbt=null!!!");
        return;
    }

    BBTrack* newbbt = dynamic_cast<BBTrack*>(oldbbt->clone());
    newbbt->setName(instr->name() /*+" isolated"*/);  // use the name of the
                                                      // instrument

    int newidxbb = newbbt->ownBBTrackIndex();
    // if(newidxbb < 0 || newidxbb > bbtc->numOfBBs() - 1)
    //    qWarning("TrackOperationsWidget::isolateTrack#2 newidxbb=%d",
    //             newidxbb);
    bbtc->setCurrentBB(newidxbb);

    // qInfo("TrackOperationsWidget::isolateTrack start cleaning");
    for(Track* t: bbtc->tracks())
    {
        /*TrackView* tv=*/tcview->createTrackView(t);
        if(t == instr)
            continue;

        // qInfo("TrackOperationsWidget::isolateTrack clear all notes in
        // %s",qPrintable(t->name()));
        Tile* o = t->tileForBB(newidxbb);  // getTCO(newidxbb);
        // for(Tile* o : t->getTCOs(idxbb))
        {
            Pattern* p = dynamic_cast<Pattern*>(o);  // static
            if(!p)
            {
                qCritical("TrackOperationsWidget::isolateTrack p=null!!!");
                continue;
            }
            p->clearNotes();
        }
    }

    bbtc->setCurrentBB(oldidxbb);
}

void TrackOperationsWidget::changeName()
{
    Track*       t = m_trackView->track();
    QString      s = t->name();
    RenameDialog rename_dlg(s);
    rename_dlg.exec();
    t->setName(s);
}

void TrackOperationsWidget::resetName()
{
    Track*  t = m_trackView->track();
    QString s = t->defaultName();
    t->setName(s);
}

void TrackOperationsWidget::changeColor()
{
    Track* t = m_trackView->track();

    QColor new_color = QColorDialog::getColor(t->color());

    if(!new_color.isValid())
    {
        return;
    }
    /*
    if( isSelected() )
    {
            SelectableObjects selected =
                    gui->songWindow()->m_editor->selectedObjects();
            for( QVector<SelectableObject *>::iterator it =
                         selected.begin();
                 it != selected.end(); ++it )
            {
                    TileView* tcov=
                            dynamic_cast<TileView*>( *it );
                    if( tcov )
                            tcov->setColor( new_color );
            }
    }
    else
    */
    if(new_color != t->color())
    {
        t->setColor(new_color);
        t->setUseStyleColor(false);
        Engine::song()->setModified();
        m_trackView->update();
    }
}

void TrackOperationsWidget::resetColor()
{
    Track* t = m_trackView->track();

    /*
    if( isSelected() )
    {
            SelectableObjects selected =
                    gui->songWindow()->m_editor->selectedObjects();
            for( QVector<SelectableObject *>::iterator it =
                         selected.begin();
                 it != selected.end(); ++it )
            {
                    TileView* tcov=
                            dynamic_cast<TileView*>( *it );
                    if( tcov )
                            tcov->setUseStyleColor(true);
            }
    }
    else
    */

    if(!t->useStyleColor())
    {
        t->setUseStyleColor(true);
        Engine::song()->setModified();
        m_trackView->update();
    }

    // BBTrack::clearLastTCOColor();
}

void TrackOperationsWidget::handleLoopMenuAction(QAction* _a)
{
    Track*    t = m_trackView->track();
    const int n = _a->data().toInt();
    if(n != t->currentLoop())
    {
        t->setCurrentLoop(n);
        m_trackView->getTrackContentWidget()->update();
    }
}

/*! \brief Remove this track from the track list
 *
 */
void TrackOperationsWidget::removeTrack()
{
    emit trackRemovalScheduled(m_trackView);
}

void TrackOperationsWidget::addLoopMenu(QMenu* _cm, bool _enabled)
{
    Track*   t = m_trackView->track();
    QMenu*   m = new QMenu(tr("Loop"));
    QAction* a;
    for(int i = -1; i < TimeLineWidget::NB_LOOPS; i++)
    {
        a = m->addAction(i == -1 ? tr("None")
                                 : QString("%1").arg((char)(65 + i)));
        a->setData(QVariant(i));
        a->setEnabled(_enabled && (i != t->currentLoop()));
    }

    connect(m, SIGNAL(triggered(QAction*)), this,
            SLOT(handleLoopMenuAction(QAction*)));
    _cm->addMenu(m);
}

void TrackOperationsWidget::addNameMenu(QMenu* _cm, bool _enabled)
{
    QAction* a;
    a = _cm->addAction(embed::getIcon("edit_rename"), tr("Change name"), this,
                       SLOT(changeName()));
    a->setEnabled(_enabled);
    a = _cm->addAction(embed::getIcon("reload"), tr("Reset name"), this,
                       SLOT(resetName()));
    a->setEnabled(_enabled);
}

void TrackOperationsWidget::addColorMenu(QMenu* _cm, bool _enabled)
{
    QAction* a;
    a = _cm->addAction(embed::getIcon("colorize"), tr("Change color"), this,
                       SLOT(changeColor()));
    a->setEnabled(_enabled);
    a = _cm->addAction(embed::getIcon("colorize"), tr("Reset color"), this,
                       SLOT(resetColor()));
    a->setEnabled(_enabled);
}

void TrackOperationsWidget::addSpecificMenu(QMenu* _cm, bool _enabled)
{
    m_trackView->addSpecificMenu(_cm, _enabled);
    /*
    if(InstrumentTrackView* trackView
       = dynamic_cast<InstrumentTrackView*>(m_trackView))
    {
        _cm->addSeparator();
        _cm->addMenu(trackView->createAudioInputMenu());
        _cm->addMenu(trackView->createAudioOutputMenu());
        _cm->addMenu(trackView->createMidiInputMenu());
        _cm->addMenu(trackView->createMidiOutputMenu());

        // Old MIDI menu
        // _cm->addSeparator();
        // _cm->addMenu(trackView->midiMenu());
    }
    if(SampleTrackView* trackView
       = dynamic_cast<SampleTrackView*>(m_trackView))
    {
        _cm->addSeparator();
        _cm->addMenu(trackView->createAudioInputMenu());
        _cm->addMenu(trackView->createAudioOutputMenu());
    }
    if(dynamic_cast<AutomationTrackView*>(m_trackView))
    {
        _cm->addSeparator();
        _cm->addAction(tr("Turn all recording on"), this,
                          SLOT(recordingOn()));
        _cm->addAction(tr("Turn all recording off"), this,
                          SLOT(recordingOff()));
    }
    */
}

/*! \brief Update the trackOperationsWidget context menu
 *
 *  For all track types, we have the Clone and Remove options.
 *  For instrument-tracks we also offer the MIDI-control-menu
 *  For automation tracks, extra options: turn on/off recording
 *  on all TCOs (same should be added for sample tracks when
 *  sampletrack recording is implemented)
 */
void TrackOperationsWidget::updateMenu()
{
    QMenu* toMenu = m_trackOps->menu();
    toMenu->clear();
    toMenu->addAction(embed::getIcon("clone", 16, 16), tr("Clone this track"),
                      this, SLOT(cloneTrack()));
    toMenu->addAction(embed::getIcon("clone", 16, 16), tr("Spawn this track"),
                      this, SLOT(spawnTrack()));

    if(!m_trackView->trackContainerView()->fixedTCOs())
    {
        if(m_trackView->track()->type() == Track::BBTrack)
            toMenu->addAction(tr("Split this track"), this,
                              SLOT(splitTrack()));
    }
    else
    {
        toMenu->addAction(tr("Isolate this track"), this,
                          SLOT(isolateTrack()));
    }

    toMenu->addSeparator();

    if(!m_trackView->trackContainerView()->fixedTCOs())
    {
        toMenu->addAction(tr("Clear this track"), this, SLOT(clearTrack()));
    }

    toMenu->addAction(embed::getIcon("cancel", 16, 16),
                      tr("Remove this track"), this, SLOT(removeTrack()));

    toMenu->addSeparator();
    addLoopMenu(toMenu, true);
    addSpecificMenu(toMenu, true);

    toMenu->addSeparator();
    addNameMenu(toMenu, true);
    toMenu->addSeparator();
    addColorMenu(toMenu, true);
}

/*
void TrackOperationsWidget::recordingOn()
{
    AutomationTrackView* atv
            = dynamic_cast<AutomationTrackView*>(m_trackView);
    if(atv)
    {
        const Tiles& tcov = atv->track()->getTCOs();
        for(Tiles::const_iterator it = tcov.begin();
            it != tcov.end(); ++it)
        {
            AutomationPattern* ap = dynamic_cast<AutomationPattern*>(*it);
            if(ap)
            {
                ap->setRecording(true);
            }
        }
        atv->update();
    }
}

void TrackOperationsWidget::recordingOff()
{
    AutomationTrackView* atv
            = dynamic_cast<AutomationTrackView*>(m_trackView);
    if(atv)
    {
        const Tiles& tcov = atv->track()->getTCOs();
        for(Tiles::const_iterator it = tcov.begin();
            it != tcov.end(); ++it)
        {
            AutomationPattern* ap = dynamic_cast<AutomationPattern*>(*it);
            if(ap)
            {
                ap->setRecording(false);
            }
        }
        atv->update();
    }
}
*/

// ===========================================================================
// trackView
// ===========================================================================

/*! \brief Create a new track View.
 *
 *  The track View is handles the actual display of the track, including
 *  displaying its various widgets and the track segments.
 *
 *  \param track The track to display.
 *  \param tcv The track Container View for us to be displayed in.
 *  \todo Is my description of these properties correct?
 */
TrackView::TrackView(Track* track, TrackContainerView* tcv) :
      QWidget(tcv->contentWidget()), /*!< The Track Container View's content
                                        widget. */
      // ModelView(nullptr, this),      /*!< The model view of this track */
      ModelView(track, this),  // <-- TRY TMP
      m_track(track),          /*!< The track we're displaying */
      m_trackContainerView(
              tcv), /*!< The track Container View we're displayed in */
      m_trackOperationsWidget(this), /*!< Our trackOperationsWidget */
      m_trackSettingsWidget(this),   /*!< Our trackSettingsWidget */
      m_trackContentWidget(this),    /*!< Our trackContentWidget */
      m_action(NoAction) /*!< The action we're currently performing */
{
    // allowModelChange(true);

    setAutoFillBackground(true);
    QPalette pal;
    pal.setColor(backgroundRole(), QColor(32, 36, 40));
    setPalette(pal);

    m_trackSettingsWidget.setAutoFillBackground(true);

    // Fake model view for loop
    new DummyModelView(&track->m_loopEnabledModel, this);
    new DummyModelView(&track->m_currentLoopModel, this);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(&m_trackOperationsWidget);
    layout->addWidget(&m_trackSettingsWidget);
    layout->addWidget(&m_trackContentWidget, 1);
    setFixedHeight(m_track->height());

    resizeEvent(nullptr);

    setAcceptDrops(true);
    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(m_track, SIGNAL(destroyedTrack()), this, SLOT(close()),
            Qt::DirectConnection);
    connect(m_track, SIGNAL(tileAdded(Tile*)), this,
            SLOT(createTCOView(Tile*)), Qt::QueuedConnection);

    connect(&m_track->m_mutedModel, SIGNAL(dataChanged()),
            &m_trackContentWidget, SLOT(update()));
    connect(&m_track->m_frozenModel, SIGNAL(dataChanged()),
            &m_trackContentWidget, SLOT(update()));
    connect(&m_track->m_clippingModel, SIGNAL(dataChanged()),
            &m_trackContentWidget, SLOT(update()));

    connect(&m_track->m_loopEnabledModel, SIGNAL(dataChanged()),
            &m_trackContentWidget, SLOT(update()));
    connect(&m_track->m_currentLoopModel, SIGNAL(dataChanged()),
            &m_trackContentWidget, SLOT(update()));

    // create views for already existing TCOs
    /*
    for(Tiles::iterator it = m_track->m_tiles.begin();
        it != m_track->m_tiles.end(); ++it)
        createTCOView(*it);
    */

    setUpdatesEnabled(false);
    for(Tile* tco: m_track->m_tiles)
        createTCOView(tco);
    setUpdatesEnabled(true);

    m_trackContainerView->addTrackView(this);
    modelChanged();
}

/*! \brief Destroy this track View.
 *
 */
TrackView::~TrackView()
{
    qInfo("TrackView::~TrackView");
    // m_trackContainerView->removeTrackView(this);
}

QColor TrackView::cableColor() const
{
    return m_track->color();
}

bool TrackView::isFixed() const
{
    return m_track->isFixed();
}

real_t TrackView::pixelsPerTact() const
{
    return m_trackContainerView->pixelsPerTact();
}

/*! \brief Resize this track View.
 *
 *  \param re the Resize Event to handle.
 */
void TrackView::resizeEvent(QResizeEvent* re)
{
    if(CONFIG_GET_BOOL("ui.compacttrackbuttons"))
    {
        m_trackOperationsWidget.setFixedSize(TRACK_OP_WIDTH_COMPACT,
                                             height() - 1);
        m_trackSettingsWidget.setFixedSize(
                DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT, height() - 1);
    }
    else
    {
        m_trackOperationsWidget.setFixedSize(TRACK_OP_WIDTH, height() - 1);
        m_trackSettingsWidget.setFixedSize(DEFAULT_SETTINGS_WIDGET_WIDTH,
                                           height() - 1);
    }
    m_trackContentWidget.setFixedHeight(height());
}

/*! \brief Update this track View and all its content objects.
 *
 */
void TrackView::update()
{
    m_trackContentWidget.update();
    if(!m_trackContainerView->fixedTCOs())
        m_trackContentWidget.changePosition();
    QWidget::update();
}

/*! \brief Close this track View.
 *
 */
bool TrackView::close()
{
    qInfo("TrackView::close");
    m_trackContainerView->removeTrackView(this);
    return QWidget::close();
}

/*! \brief Register that the model of this track View has changed.
 *
 */
void TrackView::modelChanged()
{
    m_track = castModel<Track>();
    assert(m_track != nullptr);
    connect(m_track, SIGNAL(destroyedTrack()), this, SLOT(close()),
            Qt::DirectConnection);
    m_trackOperationsWidget.m_muteBtn->setModel(&m_track->m_mutedModel);
    m_trackOperationsWidget.m_soloBtn->setModel(&m_track->m_soloModel);
    m_trackOperationsWidget.m_frozenBtn->setModel(&m_track->m_frozenModel);
    m_trackOperationsWidget.m_clippingBtn->setModel(
            &m_track->m_clippingModel);
    ModelView::modelChanged();
    setFixedHeight(m_track->height());
}

/*! \brief Start a drag event on this track View.
 *
 *  \param dee the DragEnterEvent to start.
 */
void TrackView::dragEnterEvent(QDragEnterEvent* dee)
{
    StringPairDrag::processDragEnterEvent(
            dee, "track_" + QString::number(m_track->type()));
}

/*! \brief Accept a drop event on this track View.
 *
 *  We only accept drop events that are of the same type as this track.
 *  If so, we decode the data from the drop event by just feeding it
 *  back into the engine as a state.
 *
 *  \param de the DropEvent to handle.
 */
void TrackView::dropEvent(QDropEvent* de)
{
    QString type  = StringPairDrag::decodeKey(de);
    QString value = StringPairDrag::decodeValue(de);
    if(type == ("track_" + QString::number(m_track->type())))
    {
        // value contains our XML-data so simply create a
        // DataFile which does the rest for us...
        DataFile dataFile(value.toUtf8());
        Engine::mixer()->requestChangeInModel();
        m_track->restoreState(dataFile.content().firstChild().toElement());
        Engine::mixer()->doneChangeInModel();
        de->accept();
    }
}

/*! \brief Handle a mouse press event on this track View.
 *
 *  If this track container supports rubber band selection, let the
 *  widget handle that and don't bother with any other handling.
 *
 *  If the left mouse button is pressed, we handle two things.  If
 *  SHIFT is pressed, then we resize vertically.  Otherwise we start
 *  the process of moving this track to a new position.
 *
 *  Otherwise we let the widget handle the mouse event as normal.
 *
 *  \param me the MouseEvent to handle.
 */
void TrackView::mousePressEvent(QMouseEvent* me)
{
    if(me->x() > 10)  // TODO: should be the width of the handle
    {
        QWidget::mousePressEvent(me);
        return;
    }

    // If previously dragged too small, restore on shift-leftclick
    if(height() < DEFAULT_TRACK_HEIGHT && me->modifiers() & Qt::ShiftModifier
       && me->button() == Qt::LeftButton)
    {
        setFixedHeight(DEFAULT_TRACK_HEIGHT);
        m_track->setHeight(DEFAULT_TRACK_HEIGHT);
    }

    int widgetTotal
            = ConfigManager::inst()->value("ui", "compacttrackbuttons")
                                      .toInt()
                              == 1
                      ? DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT
                                + TRACK_OP_WIDTH_COMPACT
                      : DEFAULT_SETTINGS_WIDGET_WIDTH + TRACK_OP_WIDTH;
    if(m_trackContainerView->allowRubberband() == true
       && me->x() > widgetTotal)
    {
        QWidget::mousePressEvent(me);
    }
    else if(me->button() == Qt::LeftButton)
    {
        if(me->modifiers() & Qt::ShiftModifier)
        {
            m_action = ResizeTrack;
            QCursor::setPos(mapToGlobal(QPoint(me->x(), height())));
            Editor::applyOverrideCursor(Qt::SizeVerCursor);
        }
        else
        {
            m_action = MoveTrack;

            Editor::applyOverrideCursor(Qt::SizeAllCursor);
            // update because in move-mode, all elements in
            // track-op-widgets are hidden as a visual feedback
            m_trackOperationsWidget.update();
        }

        me->accept();
    }
    else
    {
        QWidget::mousePressEvent(me);
    }
}

/*! \brief Handle a mouse move event on this track View.
 *
 *  If this track container supports rubber band selection, let the
 *  widget handle that and don't bother with any other handling.
 *
 *  Otherwise if we've started the move process (from mousePressEvent())
 *  then move ourselves into that position, reordering the track list
 *  with moveTrackViewUp() and moveTrackViewDown() to suit.  We make a
 *  note of this in the undo journal in case the user wants to undo this
 *  move.
 *
 *  Likewise if we've started a resize process, handle this too, making
 *  sure that we never go below the minimum track height.
 *
 *  \param me the MouseEvent to handle.
 */
void TrackView::mouseMoveEvent(QMouseEvent* me)
{
    int widgetTotal
            = ConfigManager::inst()->value("ui", "compacttrackbuttons")
                                      .toInt()
                              == 1
                      ? DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT
                                + TRACK_OP_WIDTH_COMPACT
                      : DEFAULT_SETTINGS_WIDGET_WIDTH + TRACK_OP_WIDTH;
    if(m_trackContainerView->allowRubberband() == true
       && me->x() > widgetTotal)
    {
        QWidget::mouseMoveEvent(me);
    }
    else if(m_action == MoveTrack)
    {
        // look which track-widget the mouse-cursor is over
        const int yPos = m_trackContainerView->contentWidget()
                                 ->mapFromGlobal(me->globalPos())
                                 .y();
        const TrackView* trackAtY = m_trackContainerView->trackViewAt(yPos);

        // debug code
        //			qDebug( "y position %d", yPos );

        // a track-widget not equal to ourself?
        if(trackAtY != nullptr && trackAtY != this)
        {
            // then move us up/down there!
            if(me->y() < 0)
            {
                m_trackContainerView->moveTrackViewUp(this);
            }
            else
            {
                m_trackContainerView->moveTrackViewDown(this);
            }
        }
    }
    else if(m_action == ResizeTrack)
    {
        setFixedHeight(qMax<int>(me->y(), MINIMAL_TRACK_HEIGHT));
        m_trackContainerView->realignTracks();
        m_track->setHeight(height());
    }

    if(height() < DEFAULT_TRACK_HEIGHT)
    {
        ToolTip::add(this, m_track->m_name);
    }
}

/*! \brief Handle a mouse release event on this track View.
 *
 *  \param me the MouseEvent to handle.
 */
void TrackView::mouseReleaseEvent(QMouseEvent* me)
{
    m_action = NoAction;
    m_trackOperationsWidget.update();
    Editor::resetOverrideCursor();
    QWidget::mouseReleaseEvent(me);
}

/*! \brief Repaint this track View.
 *
 *  \param pe the PaintEvent to start.
 */
void TrackView::paintEvent(QPaintEvent* pe)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

/*! \brief Create a Tile View in this track View.
 *
 *  \param tco the Tile to create the view for.
 *  \todo is this a good description for what this method does?
 */
void TrackView::createTCOView(Tile* _tco)
{
    // qInfo("TrackView::createTCOView tco=%p", _tco);
    TileView* tv = _tco->createView(this);
    if(_tco->getSelectViewOnCreate() == true)
        tv->setSelected(true);

    _tco->selectViewOnCreate(false);
}

HyperBarView::HyperBarView(int            length,
                           const QColor&  color,
                           const QString& label) :
      m_length(length),
      m_color(color), m_label(label)

{
}

BarView::BarView(const QPointer<HyperBarView>& hbv, Types type, bool sign) :
      m_hbv(hbv), m_type(type), m_sign(sign)
{
}
