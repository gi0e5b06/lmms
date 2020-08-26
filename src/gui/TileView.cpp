/*
 * TileView.cpp -
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

/** \file TileView.cpp
 *  \brief View base class of a tile
 */

/*
 * \mainpage Track classes
 * \section introduction Introduction
 * \todo fill this out
 */

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
#include "Tile.h"
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

/*! The width of the resize grip in pixels
 */
const int RESIZE_GRIP_WIDTH = 4;

/*! A pointer for that text bubble used when moving segments, etc.
 *
 * In a number of situations, LMMS displays a floating text bubble
 * beside the cursor as you move or resize elements of a track about.
 * This pointer keeps track of it, as you only ever need one at a time.
 */
TextFloat* TileView::s_textFloat = nullptr;

// ===========================================================================
// TileView
// ===========================================================================
/*! \brief Create a new TileView
 *
 *  Creates a new track content object view for the given
 *  track content object in the given track view.
 *
 * \param _tco The track content object to be displayed
 * \param _tv  The track view that will contain the new object
 */
TileView::TileView(Tile* tco, TrackView* tv) :
      SelectableObject(tv->getTrackContentWidget()), ModelView(tco, this),
      m_tile(tco), m_trackView(tv), m_action(NoAction),
      m_initialMousePos(QPoint(0, 0)), m_initialMouseGlobalPos(QPoint(0, 0)),
      m_hint(nullptr), m_mutedColor(0, 0, 0), m_mutedBackgroundColor(0, 0, 0),
      m_selectedColor(0, 0, 0), m_textColor(0, 0, 0),
      m_textShadowColor(0, 0, 0), m_BBPatternBackground(0, 0, 0),
      // m_gradient( true ),
      m_needsUpdate(true)
{
    if(s_textFloat == nullptr)
    {
        s_textFloat = new TextFloat();
        s_textFloat->setPixmap(embed::getPixmap("clock"));
    }

    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    move(0, 0);
    show();

    setFixedHeight(tv->getTrackContentWidget()->height() - 1);
    setAcceptDrops(true);
    setMouseTracking(true);

    // doconnections
    // setModel(m_tile);

    m_trackView->getTrackContentWidget()->addTCOView(this);
    updateLength();
    updatePosition();
    doConnections();
}

/*! \brief Destroy a TileView
 *
 *  Destroys the given track content object view.
 *
 */
TileView::~TileView()
{
    qInfo("TileView::~TileView 1");
    // we have to give our track-container the focus because otherwise the
    // op-buttons of our track-widgets could become focus and when the user
    // presses space for playing song, just one of these buttons is pressed
    // which results in unwanted effects
    if(hasFocus())
        m_trackView->trackContainerView()->setFocus();
    hide();
    // undoConnections();
    qInfo("TileView::~TileView 2");
    // m_trackView->getTrackContentWidget()->removeTCOView(this);
    delete m_hint;
    qInfo("TileView::~TileView 3");
}

void TileView::doConnections()
{
    // qInfo("TileView::doConnections");
    QObject::connect(m_tile, SIGNAL(lengthChanged()), this,
                     SLOT(updateLength()));
    QObject::connect(gui->songWindow()->m_editor->zoomingXModel(),
                     SIGNAL(dataChanged()), this, SLOT(updateLength()));
    QObject::connect(m_tile, SIGNAL(positionChanged()), this,
                     SLOT(updatePosition()));
    // QObject::connect(m_tile, SIGNAL(destroyedTCO()), this, SLOT(close()));
    ModelView::doConnections();
}

void TileView::undoConnections()
{
    qInfo("TileView::undoConnections");
    QObject::disconnect(m_tile, SIGNAL(lengthChanged()), this,
                        SLOT(updateLength()));
    QObject::disconnect(gui->songWindow()->m_editor->zoomingXModel(),
                        SIGNAL(dataChanged()), this, SLOT(updateLength()));
    QObject::disconnect(m_tile, SIGNAL(positionChanged()), this,
                        SLOT(updatePosition()));
    // QObject::disconnect(m_tile, SIGNAL(destroyedTCO()), this,
    // SLOT(close()));
    ModelView::undoConnections();
}

/*! \brief Update a TileView
 *
 *  TCO's get drawn only when needed,
 *  and when a TCO is updated,
 *  it needs to be redrawn.
 *
 */
void TileView::update()
{
    if(isFixed())
        Engine::getBBTrackContainer()->updateBBTrack(m_tile);

    m_needsUpdate = true;
    SelectableObject::update();
}

/*! \brief Does this TileView have a fixed TCO?
 *
 *  Returns whether the containing trackView has fixed
 *  TCOs.
 *
 * \todo What the hell is a TCO here - track content object?  And in
 *  what circumstance are they fixed?
 */
/*
bool TileView::fixedTCOs()
{
        return m_trackView->trackContainerView()->fixedTCOs();
}
*/

// qproperty access functions, to be inherited & used by TCOviews
//! \brief CSS theming qproperty access method
QColor TileView::mutedColor() const
{
    return m_mutedColor;
}

QColor TileView::mutedBackgroundColor() const
{
    return m_mutedBackgroundColor;
}

QColor TileView::selectedColor() const
{
    return m_selectedColor;
}

QColor TileView::textColor() const
{
    return m_textColor;
}

QColor TileView::textBackgroundColor() const
{
    return m_textBackgroundColor;
}

QColor TileView::textShadowColor() const
{
    return m_textShadowColor;
}

QColor TileView::BBPatternBackground() const
{
    return m_BBPatternBackground;
}

// bool TileView::gradient() const
//{ return m_gradient; }

//! \brief CSS theming qproperty access method
void TileView::setMutedColor(const QColor& c)
{
    m_mutedColor = QColor(c);
}

void TileView::setMutedBackgroundColor(const QColor& c)
{
    m_mutedBackgroundColor = QColor(c);
}

void TileView::setSelectedColor(const QColor& c)
{
    m_selectedColor = QColor(c);
}

void TileView::setTextColor(const QColor& c)
{
    m_textColor = QColor(c);
}

void TileView::setTextBackgroundColor(const QColor& c)
{
    m_textBackgroundColor = c;
}

void TileView::setTextShadowColor(const QColor& c)
{
    m_textShadowColor = QColor(c);
}

void TileView::setBBPatternBackground(const QColor& c)
{
    m_BBPatternBackground = QColor(c);
}

// void TileView::setGradient( const bool & b )
//{ m_gradient = b; }

// access needsUpdate member variable
bool TileView::needsUpdate()
{
    return m_needsUpdate;
}

void TileView::setNeedsUpdate(bool b)
{
    m_needsUpdate = b;
}

/*! \brief Close a TileView
 *
 *  Closes a track content object view by asking the track
 *  view to remove us and then asking the QWidget to close us.
 *
 * \return Boolean state of whether the QWidget was able to close.
 */
bool TileView::close()
{
    qInfo("TileView::close");
    // m_trackView->getTrackContentWidget()->removeTCOView(this);
    // deleteLater();
    return QWidget::close();
    // delete this;
    // return true;
}

/*! \brief Removes a TileView from its track view.
 *
 *  Like the close() method, this asks the track view to remove this
 *  track content object view.  However, the track content object is
 *  scheduled for later deletion rather than closed immediately.
 *
 */
void TileView::remove()
{
    qInfo("TileView::remove");
    m_trackView->track()->addJournalCheckPoint();

    // delete ourself
    // delete m_tile;
    m_tile->deleteLater();
    // close();
}

void TileView::mute()
{
    m_tile->toggleMute();
}

void TileView::clear()
{
    m_tile->clear();
}

/*! \brief Cut this TileView from its track to the clipboard.
 *
 *  Perform the 'cut' action of the clipboard - copies the track content
 *  object to the clipboard and then removes it from the track.
 */
void TileView::cut()
{
    m_tile->copy();
    remove();
}

void TileView::copy()
{
    m_tile->copy();
}

void TileView::paste()
{
    m_tile->paste();
}

void TileView::changeAutoResize()
{
    m_tile->setAutoResize(!m_tile->autoResize());
}

void TileView::changeAutoRepeat()
{
    m_tile->setAutoRepeat(!m_tile->autoRepeat());
}

void TileView::changeName()
{
    QString      s = m_tile->name();
    RenameDialog rename_dlg(s);
    rename_dlg.exec();
    m_tile->setName(s);
}

void TileView::resetName()
{
    m_tile->setName(m_tile->track()->name());
}

void TileView::changeColor()
{
    QColor new_color = QColorDialog::getColor(color());

    if(!new_color.isValid())
    {
        return;
    }

    if(isSelected())
    {
        SelectableObjects selected
                = gui->songWindow()->m_editor->selectedObjects();
        for(SelectableObjects::iterator it = selected.begin();
            it != selected.end(); ++it)
        {
            TileView* tcov = dynamic_cast<TileView*>(*it);
            if(tcov)
                tcov->setColor(new_color);
        }
    }
    else
        setColor(new_color);
}

void TileView::resetColor()
{
    if(isSelected())
    {
        SelectableObjects selected
                = gui->songWindow()->m_editor->selectedObjects();
        for(SelectableObjects::iterator it = selected.begin();
            it != selected.end(); ++it)
        {
            TileView* tcov = dynamic_cast<TileView*>(*it);
            if(tcov)
                tcov->setUseStyleColor(true);
        }
    }
    else
        setUseStyleColor(true);

    // BBTrack::clearLastTCOColor();
}

bool TileView::isFixed() const
{
    return m_tile->isFixed();
}

/*! \brief How many pixels a tact (bar) takes for this TileView.
 *
 * \return the number of pixels per tact (bar).
 */
real_t TileView::pixelsPerTact() const
{
    return m_trackView->pixelsPerTact();
}

bool TileView::useStyleColor() const
{
    return m_tile->useStyleColor();
}

void TileView::setUseStyleColor(bool _use)
{
    if(_use != m_tile->useStyleColor())
    {
        m_tile->setUseStyleColor(_use);
        Engine::song()->setModified();
        update();
    }
}

QColor TileView::color() const
{
    return m_tile->color();
}

void TileView::setColor(const QColor& new_color)
{
    if(new_color != m_tile->color())
    {
        m_tile->setColor(new_color);
        m_tile->setUseStyleColor(false);
        Engine::song()->setModified();
        update();
    }
    // BBTrack::setLastTCOColor( new_color );
}

/*! \brief Updates a TileView's length
 *
 *  If this track content object view has a fixed TCO, then we must
 *  keep the width of our parent.  Otherwise, calculate our width from
 *  the track content object's length in pixels adding in the border.
 *
 */
void TileView::updateLength()
{
    // qInfo("TileView::updateLength %f", pixelsPerTact());
    // if( fixedTCOs() )
    //{
    //	setFixedWidth( parentWidget()->width() );
    //}
    // else
    {
        real_t w = pixelsPerTact() * m_tile->length()
                   / MidiTime::ticksPerTact();
        // if( isFixed() ) w*=16.;
        //+ 1 + TCO_BORDER_WIDTH * 2-1 );
        setFixedWidth(static_cast<int>(round(w)));
    }
    m_trackView->trackContainerView()->update();
}

/*! \brief Updates a TileView's position.
 *
 *  Ask our track view to change our position.  Then make sure that the
 *  track view is updated in case this position has changed the track
 *  view's length.
 *
 */
void TileView::updatePosition()
{
    m_trackView->getTrackContentWidget()->changePosition();

    // moving a TCO can result in change of song-length etc.,
    // therefore we update the track-container
    m_trackView->trackContainerView()->update();
    // moving a TCO can result in change of sub loop
    update();
}

/*! \brief Change the TileView's display when something
 *  being dragged enters it.
 *
 *  We need to notify Qt to change our display if something being
 *  dragged has entered our 'airspace'.
 *
 * \param dee The QDragEnterEvent to watch.
 */
void TileView::dragEnterEvent(QDragEnterEvent* dee)
{
    if(m_tile->track()->isFrozen())
    {
        dee->ignore();
        return;
    }

    TrackContentWidget* tcw = getTrackView()->getTrackContentWidget();
    MidiTime tcoPos         = MidiTime(m_tile->startPosition().getTact(), 0);

    if(tcw->canPasteSelection(tcoPos, dee->mimeData()) == false)
    {
        dee->ignore();
    }
    else
    {
        StringPairDrag::processDragEnterEvent(
                dee, "tco_" + QString::number(m_tile->track()->type()));
    }
}

/*! \brief Handle something being dropped on this TileView.
 *
 *  When something has been dropped on this TileView, and
 *  it's a track content object, then use an instance of our dataFile reader
 *  to take the xml of the track content object and turn it into something
 *  we can write over our current state.
 *
 * \param de The QDropEvent to handle.
 */
void TileView::dropEvent(QDropEvent* de)
{
    if(m_tile->track()->isFrozen())
    {
        de->ignore();
        return;
    }

    QString type  = StringPairDrag::decodeKey(de);
    QString value = StringPairDrag::decodeValue(de);

    // Track must be the same type to paste into
    if(type != ("tco_" + QString::number(m_tile->track()->type())))
    {
        de->ignore();
        return;
    }

    // Defer to rubberband paste if we're in that mode
    if(m_trackView->trackContainerView()->allowRubberband() == true)
    {
        TrackContentWidget* tcw = getTrackView()->getTrackContentWidget();
        MidiTime tcoPos = MidiTime(m_tile->startPosition().getTact(), 0);
        if(tcw->pasteSelection(tcoPos, de) == true)
        {
            de->accept();
        }
        else
        {
            de->ignore();
        }
        return;
    }

    // Don't allow pasting a tco into itself.
    QObject* qwSource = de->source();
    if(qwSource != nullptr && dynamic_cast<TileView*>(qwSource) == this)
    {
        de->ignore();
        return;
    }

    // Copy state into existing tco
    DataFile    dataFile(value.toUtf8());
    MidiTime    pos  = m_tile->startPosition();
    QDomElement tcos = dataFile.content().firstChildElement("tcos");
    m_tile->restoreState(tcos.firstChildElement().firstChildElement());
    m_tile->movePosition(pos);
    AutomationPattern::resolveAllIDs();
    de->accept();
}

/*! \brief Handle a dragged selection leaving our 'airspace'.
 *
 * \param e The QEvent to watch.
 */
void TileView::leaveEvent(QEvent* _le)
{
    Editor::resetOverrideCursor();

    if(m_tile->track()->isFrozen())
    {
        if(_le != nullptr)
            _le->ignore();
        return;
    }

    if(_le != nullptr)
        QWidget::leaveEvent(_le);
}

/*! \brief Create a DataFile suitable for copying multiple Tiles.
 *
 * Tiles in the vector are written to the "tcos" node in the DataFile.
 * The TileView's initial mouse position is written to the "initialMouseX"
 * node in the DataFile. When dropped on a track, this is used to create
 * copies of the TCOs.
 *
 * \param tcos The trackContectObjects to save in a DataFile
 */
DataFile TileView::createTCODataFiles(const TileViews& _tileViews) const
{
    Track*          t  = m_trackView->track();
    TrackContainer* tc = t->trackContainer();
    DataFile        dataFile(DataFile::DragNDropData);
    QDomElement     tcoParent = dataFile.createElement("tcos");

    for(TileView* tv: _tileViews)
    {
        // Insert into the dom under the "tcos" element
        int trackIndex = tc->tracks().indexOf(tv->m_trackView->track());
        QDomElement tcoElement = dataFile.createElement("tco");
        tcoElement.setAttribute("trackIndex", trackIndex);
        tv->m_tile->saveState(dataFile, tcoElement);
        tcoParent.appendChild(tcoElement);
    }

    dataFile.content().appendChild(tcoParent);

    // Add extra metadata needed for calculations later
    int initialTrackIndex = tc->tracks().indexOf(t);
    if(initialTrackIndex < 0)
    {
        qCritical(
                "TileView::createTCODataFiles: "
                "Fail to find selected track in the TrackContainer");
        return dataFile;
    }
    QDomElement metadata = dataFile.createElement("copyMetadata");
    // initialTrackIndex is the index of the track that was touched
    metadata.setAttribute("initialTrackIndex", initialTrackIndex);
    // grabbedTCOPos is the pos of the tact containing the TCO we grabbed
    metadata.setAttribute("grabbedTCOPos", m_tile->startPosition());

    dataFile.content().appendChild(metadata);

    return dataFile;
}

void TileView::paintTileLoop(QPainter& p)
{
    const Track* t = m_tile->track();
    const int    n = t->currentLoop();
    if(n < 0 || n >= TimeLineWidget::NB_LOOPS)
        return;

    const Song*           song = Engine::song();
    const TimeLineWidget* tl
            = song->getPlayPos(Song::Mode_PlaySong).m_timeLine;
    if(tl == nullptr)
        return;

    const MidiTime& tstart = m_tile->startPosition();
    const MidiTime& tend   = m_tile->endPosition();
    const MidiTime& lstart = tl->loopBegin(n);
    const MidiTime& lend   = tl->loopEnd(n);
    const real_t    ppt    = pixelsPerTact();
    const int       w      = width();
    const int       h      = height();

    if(lstart >= tend || lend < tstart)
        return;

    const int xstart = qMax<int>(0, ppt * (lstart - tstart)
                                            / MidiTime::ticksPerTact());
    const int xend
            = qMin<int>(ppt * (lend - tstart) / MidiTime::ticksPerTact(), w);
    // if(xstart >= w || xend < 0)
    //  return;

    const int wc = xend - xstart + 1;
    const int yc = h / 2;
    QBrush    bg = Qt::yellow;
    p.fillRect(xstart, yc + 1, wc, 5, bg);
}

void TileView::paintTextLabel(QString const& text,
                              const QColor&  c,
                              QPainter&      p)
{
    if(text.trimmed() == "")
        return;

    p.setRenderHint(QPainter::TextAntialiasing, true);

    QFont labelFont = this->font();
    labelFont.setHintingPreference(QFont::PreferFullHinting);
    p.setFont(labelFont);

    const int textTop  = TCO_BORDER_WIDTH + 1;
    const int textLeft = TCO_BORDER_WIDTH + 3;

    QFontMetrics fontMetrics(labelFont);
    QString      elidedPatternName = fontMetrics.elidedText(
            text, Qt::ElideMiddle, width() - 2 * textLeft);

    if(elidedPatternName.length() < 2)
        elidedPatternName = text.trimmed();

    // p.fillRect(QRect(0, 0, width(), fontMetrics.height() + 2 * textTop),
    // textBackgroundColor());

    int const finalTextTop = textTop + fontMetrics.ascent();
    p.setPen(textShadowColor());
    p.drawText(textLeft + 1, finalTextTop + 1, elidedPatternName);
    p.setPen(textColor());
    p.drawText(textLeft, finalTextTop, elidedPatternName);

    p.setRenderHints(QPainter::Antialiasing, false);
}

void TileView::paintTileBorder(const bool    current,
                               const bool    ghost,
                               const QColor& c,
                               QPainter&     p)
{
    if(TCO_BORDER_WIDTH > 0)
    {
        const int w = width();
        const int h = height();

        p.setPen(current ? c.lighter(150) : c.lighter(200));
        p.drawLine(0, 0, w - 1, 0);
        p.drawLine(0, 1, 0, h - 1);
        p.setPen(current ? c.darker(150) : c.darker(200));
        p.drawLine(0, h - 1, w - 1, h - 1);
        p.drawLine(w - 1, 1, w - 1, h - 2);
        if(ghost)
        {
            p.setPen(QColor(255, 128, 0));
            p.drawRect(0, 0, w - 1, h - 1);  // 1,1,w-2,h-2);
        }
        if(current)
        {
            p.setPen(Qt::yellow);
            p.drawRect(0, 0, w - 1, h - 1);  // 1,1,w-2,h-2);
        }
    }
}

void TileView::paintTileTacts(const bool    current,
                              tact_t        nbt,
                              tick_t        tpg,
                              const QColor& c,
                              QPainter&     p)
{
    p.setRenderHints(QPainter::Antialiasing, true);

    // const int w=width();
    const int h = height();

    const real_t ppt = pixelsPerTact();
    const QPen   pen1(
            QPen(current ? Qt::yellow : c.darker(125), 1, Qt::DashLine));
    p.setPen(pen1);
    for(tact_t t = 1; t <= nbt; t++)
    {
        real_t x = ppt * t;
        p.drawLine(x, 1, x, h - 2);
    }

    int tpt = MidiTime::ticksPerTact();
    if((tpg != 1) && (tpg != tpt))
    {
        const QPen pen2(
                QPen(current ? Qt::yellow : c.darker(200), 1, Qt::SolidLine));
        p.setPen(pen2);
        for(tick_t t = tpg; t <= nbt * tpt; t += tpg)
        {
            real_t x = ppt * t / tpt;
            p.drawLine(x, 1, x, h - 2);
        }
    }

    p.setRenderHints(QPainter::Antialiasing, false);
}

void TileView::paintMutedIcon(const bool muted, QPainter& p)
{
    if(muted)
    {
        const int spacing = TCO_BORDER_WIDTH;
        const int size    = 14;
        p.drawPixmap(spacing, height() - (size + spacing),
                     embed::getPixmap("muted", size, size));
    }
}

void TileView::paintFrozenIcon(const bool frozen, QPainter& p)
{
    if(frozen)
    {
        p.setRenderHints(QPainter::Antialiasing, true);

        const int w = width();
        const int h = height();
        p.fillRect(0, h - 8, w - 1, 7, QBrush(Qt::cyan, Qt::BDiagPattern));

        p.setRenderHints(QPainter::Antialiasing, false);
    }
}

/*! \brief Handle a mouse press on this TileView.
 *
 *  Handles the various ways in which a TileView can be
 *  used with a click of a mouse button.
 *
 *  * If our container supports rubber band selection then handle
 *    selection events.
 *  * or if shift-left button, add this object to the selection
 *  * or if ctrl-left button, start a drag-copy event
 *  * or if just plain left button, resize if we're resizeable
 *  * or if ctrl-middle button, mute the track content object
 *  * or if middle button, maybe delete the track content object.
 *
 * \param me The QMouseEvent to handle.
 */
void TileView::mousePressEvent(QMouseEvent* me)
{
    if(m_tile->track()->isFrozen())
    {
        me->ignore();
        return;
    }

    setInitialMousePos(me->pos());
    if(!isFixed() && me->button() == Qt::LeftButton)
    {
        if(me->modifiers() & Qt::ControlModifier)
        {
            if(isSelected())
            {
                m_action = CopySelection;
            }
            else
            {
                gui->songWindow()->m_editor->unselectAll();
                TileViews tcoViews;
                tcoViews.push_back(this);
                QString data = createTCODataFiles(tcoViews)
                                       .toString();  // DataFile dataFile
                QPixmap thumbnail = grab();
                if((thumbnail.width() > 128) || (thumbnail.height() > 128))
                    thumbnail
                            = thumbnail.scaled(128, 128, Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
                new StringPairDrag(
                        QString("tco_%1").arg(m_tile->track()->type()), data,
                        thumbnail, this);
            }
        }
        else if(isSelected())
        {
            m_action = MoveSelection;

            // TODO: ResizeSelection
        }
        else
        {
            gui->songWindow()->m_editor->unselectAll();
            m_tile->addJournalCheckPoint();

            // move or resize
            m_tile->setJournalling(false);

            setInitialMousePos(me->pos());

            if(!m_tile->autoResize() && !m_tile->isFixed()
               && (me->x() >= width() - RESIZE_GRIP_WIDTH))
            {
                m_action = ResizeRight;
                Editor::applyOverrideCursor(Qt::SizeHorCursor);

                delete m_hint;
                m_hint = TextFloat::displayMessage(tr("Hint"),
                                                   tr("Press <%1> for free "
                                                      "resizing.")
                                                           .arg(UI_CTRL_KEY),
                                                   embed::getPixmap("hint"),
                                                   0);
                s_textFloat->setTitle(tr("Current length"));
                s_textFloat->setText(
                        tr("%1:%2 (%3:%4 to %5:%6)")
                                .arg(m_tile->length().getTact())
                                .arg(m_tile->length().getTicks()
                                     % MidiTime::ticksPerTact())
                                .arg(m_tile->startPosition().getTact() + 1)
                                .arg(m_tile->startPosition().getTicks()
                                     % MidiTime::ticksPerTact())
                                .arg(m_tile->endPosition().getTact() + 1)
                                .arg(m_tile->endPosition().getTicks()
                                     % MidiTime::ticksPerTact()));
                s_textFloat->moveGlobal(this,
                                        QPoint(width() + 2, height() + 2));
            }
            else if(!m_tile->autoResize() && !m_tile->isFixed()
                    && (me->x() < RESIZE_GRIP_WIDTH))
            {
                m_action = ResizeLeft;
                Editor::applyOverrideCursor(Qt::SizeHorCursor);

                delete m_hint;
                m_hint = TextFloat::displayMessage(tr("Hint"),
                                                   tr("Press <%1> for free "
                                                      "resizing.")
                                                           .arg(UI_CTRL_KEY),
                                                   embed::getPixmap("hint"),
                                                   0);
                s_textFloat->setTitle(tr("Current length"));
                s_textFloat->setText(
                        tr("%1:%2 (%3:%4 to %5:%6)")
                                .arg(m_tile->length().getTact())
                                .arg(m_tile->length().getTicks()
                                     % MidiTime::ticksPerTact())
                                .arg(m_tile->startPosition().getTact() + 1)
                                .arg(m_tile->startPosition().getTicks()
                                     % MidiTime::ticksPerTact())
                                .arg(m_tile->endPosition().getTact() + 1)
                                .arg(m_tile->endPosition().getTicks()
                                     % MidiTime::ticksPerTact()));
                s_textFloat->moveGlobal(this, QPoint(2, height() + 2));
            }
            else
            {
                m_action = Move;
                Editor::applyOverrideCursor(Qt::SizeAllCursor);

                delete m_hint;
                m_hint = TextFloat::displayMessage(
                        tr("Hint"),
                        tr("Press <%1> to disable the magnetic grid.")
                                .arg(UI_CTRL_KEY),
                        embed::getPixmap("hint"), 0);

                s_textFloat->setTitle(tr("Current position"));
                s_textFloat->setText(
                        QString("%1:%2")
                                .arg(m_tile->startPosition().getTact() + 1)
                                .arg(m_tile->startPosition().getTicks()
                                     % MidiTime::ticksPerTact()));
                s_textFloat->moveGlobal(this,
                                        QPoint(width() + 2, height() + 2));
            }
            // s_textFloat->reparent( this );
            s_textFloat->show();
        }
    }
    else if(me->button() == Qt::RightButton)
    {
        if(me->modifiers() & Qt::ControlModifier)
            m_tile->toggleMute();
        else if(me->modifiers() & Qt::ShiftModifier && !isFixed())
            remove();
    }
    else if(me->button() == Qt::MidButton)
    {
        if(me->modifiers() & Qt::ControlModifier)
            m_tile->toggleMute();
        else if(!isFixed())
            remove();
    }
}

/*! \brief Handle a mouse movement (drag) on this TileView.
 *
 *  Handles the various ways in which a TileView can be
 *  used with a mouse drag.
 *
 *  * If in move mode, move ourselves in the track,
 *  * or if in move-selection mode, move the entire selection,
 *  * or if in resize mode, resize ourselves,
 *  * otherwise ???
 *
 * \param me The QMouseEvent to handle.
 * \todo what does the final else case do here?
 */
void TileView::mouseMoveEvent(QMouseEvent* me)
{
    if(m_tile == nullptr)
        qInfo("TileView::mouseMoveEvent m_tile is null");
    if(m_tile->track() == nullptr)
        qInfo("TileView::mouseMoveEvent m_tile->track() is null");
    if(m_tile->track()->frozenModel() == nullptr)
        qInfo("TileView::mouseMoveEvent frozen is null");

    if(m_tile->track()->isFrozen())
    {
        me->ignore();
        return;
    }

    if(m_action == CopySelection)
    {
        if(mouseMovedDistance(me, 2) == true)
        {
            // Clear the action here because mouseReleaseEvent will not get
            // triggered once we go into drag.
            m_action = NoAction;

            // Collect all selected TCOs
            TileViews         tcoViews;
            SelectableObjects so
                    = m_trackView->trackContainerView()->selectedObjects();
            for(SelectableObjects::iterator it = so.begin(); it != so.end();
                ++it)
            {
                TileView* tcov = dynamic_cast<TileView*>(*it);
                if(tcov != nullptr)
                    tcoViews.append(tcov);
            }

            // Write the TCOs to the DataFile for copying
            QString data = createTCODataFiles(tcoViews)
                                   .toString();  // DataFile dataFile

            // TODO -- thumbnail for all selected
            QPixmap thumbnail = grab();
            if((thumbnail.width() > 128) || (thumbnail.height() > 128))
                thumbnail = thumbnail.scaled(128, 128, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
            new StringPairDrag(QString("tco_%1").arg(m_tile->track()->type()),
                               data, thumbnail, this);
        }
    }

    if(me->modifiers() & Qt::ControlModifier)
    {
        delete m_hint;
        m_hint = nullptr;
    }

    const real_t ppt = pixelsPerTact();

    if(m_action == Move)
    {
        // qInfo("Track: Move: Is this called?");

        /*
        const int x = mapToParent(me->pos()).x() - m_initialMousePos.x();
        MidiTime  t = qMax(
                0, (int)m_trackView->trackContainerView()->currentPosition()
                           + static_cast<int>(x * MidiTime::ticksPerTact()
                                              / ppt));
        if(!(me->modifiers() & Qt::ControlModifier)
           && me->button() == Qt::NoButton)
        {
            t = t.toNearestTact();
        }
        m_tile->movePosition(t);
        */
        const int dx = me->x() - m_initialMousePos.x();
        if(dx == 0)
            return;

        const tick_t q = gui->songWindow()->m_editor->quantization();

        tick_t delta = dx * MidiTime::ticksPerTact() / ppt;
        if(me->modifiers() & Qt::ShiftModifier)
            delta = round(delta / q) * q;
        if(delta == 0)
            return;

        tick_t smallest_pos = m_tile->startPosition();

        if(!(me->modifiers() & Qt::ControlModifier)
           && !(me->modifiers() & Qt::ShiftModifier))
        //&& me->button() == Qt::NoButton)
        {
            delta = delta - smallest_pos % q - delta % q;
            // if(delta%q>q/2) delta+=q;
        }
        if(smallest_pos + delta < 0)
            return;
        // qInfo("q=%d delta=%d sp=%d", q, delta, smallest_pos);
        if(delta == 0)
            return;

        tick_t t = m_tile->startPosition() + delta;
        m_tile->movePosition(t);

        m_trackView->getTrackContentWidget()->changePosition();

        s_textFloat->setText(
                QString("%1:%2")
                        .arg(m_tile->startPosition().getTact() + 1)
                        .arg(m_tile->startPosition().getTicks()
                             % MidiTime::ticksPerTact()));
        s_textFloat->moveGlobal(this, QPoint(width() + 2, height() + 2));
    }
    else if(m_action == MoveSelection)
    {
        SelectableObjects so
                = m_trackView->trackContainerView()->selectedObjects();
        if(so.isEmpty())
            return;

        const int dx = me->x() - m_initialMousePos.x();
        if(dx == 0)
            return;

        const tick_t q = gui->songWindow()->m_editor->quantization();

        tick_t delta = dx * MidiTime::ticksPerTact() / ppt;
        if(me->modifiers() & Qt::ShiftModifier)
            delta = round(delta / q) * q;
        if(delta == 0)
            return;

        Tiles tcos;
        // MidiTime::ticksPerTact()
        // find out smallest position of all selected objects for not
        // moving an object before zero
        bool   first        = true;
        tick_t smallest_pos = 0;
        for(SelectableObjects::iterator it = so.begin(); it != so.end(); ++it)
        {
            TileView* tcov = dynamic_cast<TileView*>(*it);
            if(tcov == nullptr)
                continue;

            Tile* tco = tcov->m_tile;
            tcos.append(tco);
            tick_t p = tco->startPosition();
            if(first || smallest_pos > p)
            {
                smallest_pos = p;
                first        = false;
            }
        }
        if(!(me->modifiers() & Qt::ControlModifier)
           && !(me->modifiers() & Qt::ShiftModifier))
        //&& me->button() == Qt::NoButton)
        {
            delta = delta - smallest_pos % q - delta % q;
            // if(delta%q>q/2) delta+=q;
        }
        if(smallest_pos + delta < 0)
            return;
        // qInfo("q=%d delta=%d sp=%d", q, delta, smallest_pos);
        if(delta == 0)
            return;
        for(Tiles::iterator it = tcos.begin(); it != tcos.end(); ++it)
        {
            tick_t t = (*it)->startPosition() + delta;
            (*it)->movePosition(t);
        }

        // ??? GDX
        m_trackView->getTrackContentWidget()->changePosition();
    }
    else if(m_action == ResizeRight)
    {
        // qInfo("Track: ResizeRight");

        const int    dx = me->x() - m_initialMousePos.x();
        const tick_t q  = gui->songWindow()->m_editor->quantization();

        tick_t delta = dx * MidiTime::ticksPerTact() / ppt;
        if(me->modifiers() & Qt::ShiftModifier)
            delta = round(delta / q) * q;
        if(delta == 0)
            return;

        tick_t smallest_len = m_tile->length();

        if(!(me->modifiers() & Qt::ControlModifier)
           && !(me->modifiers() & Qt::ShiftModifier))
        {
            delta = delta - smallest_len % q - delta % q;
            // if(delta%q>q/2) delta+=q;
        }
        // if(smallest_len + delta < 4)
        //    return;
        // qInfo("q=%d delta=%d sl=%d", q, delta, smallest_len);
        if(delta == 0)
            return;

        tick_t nl = m_tile->length() + delta;
        if(nl < MidiTime::ticksPerTact() / 8)
            return;
        // qInfo("before: m_initialMousePos %d,%d", m_initialMousePos.x(),
        //  m_initialMousePos.y());

        tick_t sp = m_tile->startPosition();
        m_tile->resizeRight(sp, nl);
        // m_tile->changeLength(nl);

        m_initialMousePos = QPoint(width(), height() / 2);

        s_textFloat->setText(
                tr("%1:%2 (%3:%4 to %5:%6)")
                        .arg(m_tile->length().getTact())
                        .arg(m_tile->length().getTicks()
                             % MidiTime::ticksPerTact())
                        .arg(m_tile->startPosition().getTact() + 1)
                        .arg(m_tile->startPosition().getTicks()
                             % MidiTime::ticksPerTact())
                        .arg(m_tile->endPosition().getTact() + 1)
                        .arg(m_tile->endPosition().getTicks()
                             % MidiTime::ticksPerTact()));
        s_textFloat->moveGlobal(this, QPoint(width() + 2, height() + 2));
    }
    else if(m_action == ResizeLeft)
    {
        // qInfo("Track: ResizeLeft");

        const int    dx = me->x() - m_initialMousePos.x();
        const tick_t q  = gui->songWindow()->m_editor->quantization();

        tick_t delta = dx * MidiTime::ticksPerTact() / ppt;
        if(me->modifiers() & Qt::ShiftModifier)
            delta = round(delta / q) * q;
        if(delta == 0)
            return;

        tick_t smallest_len = m_tile->length();

        if(!(me->modifiers() & Qt::ControlModifier)
           && !(me->modifiers() & Qt::ShiftModifier))
        {
            delta = delta - smallest_len % q - delta % q;
            // if(delta%q>q/2) delta+=q;
        }
        // if(smallest_len + delta < MidiTime::ticksPerTact() / 8)
        //    return;
        qInfo("q=%d delta=%d sl=%d", q, delta, smallest_len);
        if(delta == 0)
            return;

        qInfo("before: mX=%d m_initialMousePos=%d", me->x(),
              m_initialMousePos.x());
        tick_t nl = m_tile->length() - delta;
        qInfo("nl=%d", nl);
        if(nl < MidiTime::ticksPerTact() / 8)
            return;
        tick_t sp = m_tile->startPosition() + delta;
        if(sp < 0)
            return;

        m_tile->resizeLeft(sp, nl);
        // m_tile->movePosition(sp);
        // m_tile->changeLength(nl);

        m_initialMousePos = QPoint(0, height() / 2);

        s_textFloat->setText(
                tr("%1:%2 (%3:%4 to %5:%6)")
                        .arg(m_tile->length().getTact())
                        .arg(m_tile->length().getTicks()
                             % MidiTime::ticksPerTact())
                        .arg(m_tile->startPosition().getTact() + 1)
                        .arg(m_tile->startPosition().getTicks()
                             % MidiTime::ticksPerTact())
                        .arg(m_tile->endPosition().getTact() + 1)
                        .arg(m_tile->endPosition().getTicks()
                             % MidiTime::ticksPerTact()));
        s_textFloat->moveGlobal(this, QPoint(width() + 2, height() + 2));
    }
    else
    {
        if((me->x() >= width() - RESIZE_GRIP_WIDTH
            || me->x() < RESIZE_GRIP_WIDTH)
           && !me->buttons() && !m_tile->autoResize() && !m_tile->isFixed())
        {
            Editor::applyOverrideCursor(Qt::SizeHorCursor);
        }
        else
        {
            // Editor::resetOverrideCursor();
            leaveEvent(nullptr);
        }
    }
}

/*! \brief Handle a mouse release on this TileView.
 *
 *  If we're in move or resize mode, journal the change as appropriate.
 *  Then tidy up.
 *
 * \param me The QMouseEvent to handle.
 */
void TileView::mouseReleaseEvent(QMouseEvent* me)
{
    Editor::resetOverrideCursor();

    if(m_tile->track()->isFrozen())
    {
        me->ignore();
        return;
    }

    // If the CopySelection was chosen as the action due to mouse movement,
    // it will have been cleared.  At this point Toggle is the desired action.
    // An active StringPairDrag will prevent this method from being called,
    // so a real CopySelection would not have occurred.
    if(m_action == CopySelection
       || (m_action == ToggleSelected && mouseMovedDistance(me, 2) == false))
    {
        setSelected(!isSelected());
    }

    if(m_action == Move || m_action == ResizeLeft || m_action == ResizeRight)
    {
        m_tile->setJournalling(true);
        Selection::select(m_tile);
    }
    m_action = NoAction;

    delete m_hint;
    m_hint = nullptr;
    s_textFloat->hide();

    leaveEvent(nullptr);
    SelectableObject::mouseReleaseEvent(me);
}

/*! \brief Set up the context menu for this TileView.
 *
 *  Set up the various context menu events that can apply to a
 *  track content object view.
 *
 * \param cme The QContextMenuEvent to add the actions to.
 */
void TileView::contextMenuEvent(QContextMenuEvent* _cme)
{
    if(m_tile->track()->isFrozen())
    {
        _cme->ignore();
        return;
    }

    if(_cme->modifiers())
    {
        _cme->ignore();
        return;
    }

    QMenu* cm = buildContextMenu();
    cm->exec(QCursor::pos());
    delete cm;

    /*
    QMenu contextMenu( this );
    if( fixedTCOs() == false )
    {
            contextMenu.addAction( embed::getIcon( "cancel" ),
                                    tr( "Delete (middle click)" ),
                                            this, SLOT( remove() ) );
            contextMenu.addSeparator();
            contextMenu.addAction( embed::getIcon( "edit_cut" ),
                                    tr( "Cut" ), this, SLOT( cut() ) );
    }
    contextMenu.addAction( embed::getIcon( "edit_copy" ),
                                    tr( "Copy" ), m_tile, SLOT( copy() ) );
    contextMenu.addAction( embed::getIcon( "edit_paste" ),
                                    tr( "Paste" ), m_tile, SLOT( paste() ) );
    contextMenu.addSeparator();
    contextMenu.addAction( embed::getIcon( "muted" ),
                           tr( "Mute/unmute (<%1> + middle click)" )
                           .arg(UI_CTRL_KEY),
                           m_tile, SLOT( toggleMute() ) );
    build...ContextMenu( &contextMenu );

    contextMenu.exec( QCursor::pos() );
    */
}

void TileView::addRemoveMuteClearMenu(QMenu* _cm,
                                      bool   _remove,
                                      bool   _mute,
                                      bool   _clear)
{
    QAction* a;
    a = _cm->addAction(
            embed::getIcon("cancel"),
            tr("Remove this tile (<%1>+middle click)").arg(UI_SHIFT_KEY),
            this, SLOT(remove()));
    a->setEnabled(_remove);
    a = _cm->addAction(embed::getIcon("muted"),
                       tr("Mute/unmute (<%1>+middle click)").arg(UI_CTRL_KEY),
                       this, SLOT(mute()));
    a->setEnabled(_mute);
    a = _cm->addAction(embed::getIcon("edit_erase"), tr("Clear"), this,
                       SLOT(clear()));
    a->setEnabled(_clear);
}

void TileView::addCutCopyPasteMenu(QMenu* _cm,
                                   bool   _cut,
                                   bool   _copy,
                                   bool   _paste)
{
    QAction* a;
    a = _cm->addAction(embed::getIcon("edit_cut"), tr("Cut"), this,
                       SLOT(cut()));
    a->setEnabled(_cut);
    a = _cm->addAction(embed::getIcon("edit_copy"), tr("Copy"), this,
                       SLOT(copy()));
    a->setEnabled(_copy);
    a = _cm->addAction(embed::getIcon("edit_paste"), tr("Paste"), this,
                       SLOT(paste()));
    a->setEnabled(_paste);
}

void TileView::addSplitMenu(QMenu* _cm, bool _one, bool _four)
{
    QMenu* sms = new QMenu(tr("Split"));

    QAction* a;
    a = sms->addAction(embed::getIcon("note_whole"), tr("after every bar"),
                       m_tile, SLOT(splitAfterEveryBar()));
    a->setEnabled(_one);
    a = sms->addAction(embed::getIcon("note_four_whole"),
                       tr("after every fourth bar"), m_tile,
                       SLOT(splitAfterEveryFourthBar()));
    a->setEnabled(_four);

    _cm->addMenu(sms);
}

void TileView::addFlipMenu(QMenu* _cm, bool _x, bool _y)
{
    QAction* a;
    a = _cm->addAction(embed::getIcon("flip_x"), tr("Flip horizontally"),
                       m_tile, SLOT(flipHorizontally()));
    a->setEnabled(_x);
    a = _cm->addAction(embed::getIcon("flip_y"), tr("Flip vertically"),
                       m_tile, SLOT(flipVertically()));
    a->setEnabled(_y);
}

void TileView::addRotateMenu(QMenu* _cm, bool _bar, bool _beat, bool _step)
{
    QMenu* smrl = new QMenu(tr("Rotate left"));
    QMenu* smrr = new QMenu(tr("Rotate right"));

    QAction* a;
    a = smrl->addAction(embed::getIcon("arrow_first"), tr("One bar"), m_tile,
                        SLOT(rotateOneBarLeft()));
    a->setEnabled(_bar);
    a = smrr->addAction(embed::getIcon("arrow_last"), tr("One bar"), m_tile,
                        SLOT(rotateOneBarRight()));
    a->setEnabled(_bar);

    a = smrl->addAction(embed::getIcon("arrow_first"), tr("One beat"), m_tile,
                        SLOT(rotateOneBeatLeft()));
    a->setEnabled(_beat);
    a = smrr->addAction(embed::getIcon("arrow_last"), tr("One beat"), m_tile,
                        SLOT(rotateOneBeatRight()));
    a->setEnabled(_beat);

    a = smrl->addAction(embed::getIcon("arrow_left"), tr("One step"), m_tile,
                        SLOT(rotateOneStepLeft()));
    a->setEnabled(_step);
    a = smrr->addAction(embed::getIcon("arrow_right"), tr("One step"), m_tile,
                        SLOT(rotateOneStepRight()));
    a->setEnabled(_step);

    _cm->addMenu(smrl);
    _cm->addMenu(smrr);
}

void TileView::addStepMenu(QMenu* _cm, bool _bar, bool _beat, bool _step)
{
    QMenu* sma = new QMenu(tr("Add steps"));
    QMenu* smr = new QMenu(tr("Remove steps"));

    QAction* a;
    a = sma->addAction(embed::getIcon("step_btn_add"), tr("One bar"), m_tile,
                       SLOT(addBarSteps()));
    a->setEnabled(_bar);
    smr->addAction(embed::getIcon("step_btn_remove"), tr("One bar"), m_tile,
                   SLOT(removeBarSteps()));
    a->setEnabled(_bar);

    sma->addAction(embed::getIcon("step_btn_add"), tr("One beat"), m_tile,
                   SLOT(addBeatSteps()));
    a->setEnabled(_beat);
    smr->addAction(embed::getIcon("step_btn_remove"), tr("One beat"), m_tile,
                   SLOT(removeBeatSteps()));
    a->setEnabled(_beat);

    sma->addAction(embed::getIcon("step_btn_add"), tr("One step"), m_tile,
                   SLOT(addOneStep()));
    a->setEnabled(_step);
    smr->addAction(embed::getIcon("step_btn_remove"), tr("One step"), m_tile,
                   SLOT(removeOneStep()));
    a->setEnabled(_step);

    //_cm->addSeparator();
    _cm->addMenu(sma);
    _cm->addMenu(smr);
    //_cm->addAction( embed::getIcon( "step_btn_duplicate" ),
    //                tr( "Clone Steps" ), m_tile, SLOT( cloneSteps() ) );

    QMenu* sme = new QMenu(tr("Step resolution"));
    connect(sme, SIGNAL(triggered(QAction*)), this,
            SLOT(changeStepResolution(QAction*)));

    static const int labels[] = {2, 4, 8, 16, 32, 64, 128, 3, 6, 12, 24, 48};
    static const QString icons[] = {"note_whole",
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
    for(int i = 0; i < 11; i++)
        sme->addAction(embed::getIcon(icons[i]), QString::number(labels[i]))
                ->setData(labels[i]);
    //_cm->addSeparator();
    _cm->addMenu(sme);
}

void TileView::addPropertiesMenu(QMenu* _cm, bool _resize, bool _repeat)
{
    QAction* a;
    // embed::getIcon("zoom_x"),
    a = _cm->addAction(tr("Autoresize"), this, SLOT(changeAutoResize()));
    a->setCheckable(true);
    a->setChecked(m_tile->autoResize());
    a->setEnabled(_resize);
    // embed::getIcon("reload"),
    a = _cm->addAction(tr("Autorepeat"), this, SLOT(changeAutoRepeat()));
    a->setCheckable(true);
    a->setChecked(m_tile->autoRepeat());
    a->setEnabled(_repeat);
}

void TileView::addNameMenu(QMenu* _cm, bool _enabled)
{
    QAction* a;
    a = _cm->addAction(embed::getIcon("edit_rename"), tr("Change name"), this,
                       SLOT(changeName()));
    a->setEnabled(_enabled);
    a = _cm->addAction(embed::getIcon("reload"), tr("Reset name"), this,
                       SLOT(resetName()));
    a->setEnabled(_enabled);
}

void TileView::addColorMenu(QMenu* _cm, bool _enabled)
{
    QAction* a;
    a = _cm->addAction(embed::getIcon("colorize"), tr("Change color"), this,
                       SLOT(changeColor()));
    a->setEnabled(_enabled);
    a = _cm->addAction(embed::getIcon("colorize"), tr("Reset color"), this,
                       SLOT(resetColor()));
    a->setEnabled(_enabled);
}

/*! \brief Detect whether the mouse moved more than n pixels on screen.
 *
 * \param _me The QMouseEvent.
 * \param distance The threshold distance that the mouse has moved to return
 * true.
 */
bool TileView::mouseMovedDistance(QMouseEvent* me, int distance)
{
    QPoint    dPos        = mapToGlobal(me->pos()) - m_initialMouseGlobalPos;
    const int pixelsMoved = dPos.manhattanLength();
    return (pixelsMoved > distance || pixelsMoved < -distance);
}
