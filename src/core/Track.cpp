/*
 * Track.cpp - implementation of classes concerning tracks -> necessary for
 *             all track-like objects (beat/bassline, sample-track...)
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

/** \file Track.cpp
 *  \brief All classes concerning tracks and track-like objects
 */

/*
 * \mainpage Track classes
 * \section introduction Introduction
 * \todo fill this out
 */

#include "Track.h"

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
// Tile
// ===========================================================================
/*! \brief Create a new Tile
 *
 *  Creates a new track content object for the given track.
 *
 * \param _track The track that will contain the new object
 */
Tile::Tile(Track* _track, const QString& _displayName) :
      Model(_track, _displayName), m_steps(DefaultStepsPerTact),
      m_stepResolution(DefaultStepsPerTact), m_track(_track),
      m_name(QString::null), m_startPosition(), m_length(),
      m_mutedModel(false, this, tr("Mute"), "mute"),
      // m_soloModel(false, this, tr("Solo"),"solo"),
      m_autoResize(false), m_autoRepeat(false), m_color(128, 128, 128),
      m_useStyleColor(true), m_selectViewOnCreate(false)
{
    /*
    if(Engine::song()&&Engine::song()->isPlaying())
    {
            static bool s_once=false;
            if(!s_once)
            {
                    BACKTRACE
                    s_once=true;
                    qWarning("Tile::Tile
    alloc...");
            }
    }
    */

    if(_track)
        _track->addTCO(this);
    // else
    // qWarning("Tile::Tile no track???");

    setJournalling(false);
    movePosition(0);
    changeLength(0);
    setJournalling(true);
}

Tile::Tile(const Tile& _other) :
      Model(_other.m_track, _other.displayName()), m_steps(_other.m_steps),
      m_stepResolution(_other.m_stepResolution), m_track(_other.m_track),
      m_name(_other.m_name), m_startPosition(_other.m_startPosition),
      m_length(_other.m_length),
      m_mutedModel(_other.m_mutedModel.value(), this, tr("Mute")),
      m_autoResize(_other.m_autoResize), m_autoRepeat(_other.m_autoRepeat),
      m_color(_other.m_color), m_useStyleColor(_other.m_useStyleColor),
      m_selectViewOnCreate(false)
{
    Track* t = track();
    if(t)
        t->addTCO(this);
}

/*! \brief Destroy a Tile
 *
 *  Destroys the given track content object.
 *
 */
Tile::~Tile()
{
    emit destroyedTCO();

    Track* t = track();
    if(t)
        t->removeTCO(this);
}

QString Tile::defaultName() const
{
    return track()->name();
}

void Tile::saveSettings(QDomDocument& doc, QDomElement& element)
{
    element.setAttribute("name", name());
    element.setAttribute("autoresize", autoResize() ? 1 : 0);
    element.setAttribute("autorepeat", autoRepeat() ? 1 : 0);
    element.setAttribute("pos", startPosition());
    element.setAttribute("len", length());
    element.setAttribute("muted", isMuted());
    element.setAttribute("color", color().rgb());
    element.setAttribute("usestyle", useStyleColor() ? 1 : 0);
}

void Tile::loadSettings(const QDomElement& element)
{
    if(element.hasAttribute("name"))
        setName(element.attribute("name"));

    if(element.hasAttribute("pos"))
        if(element.attribute("pos").toInt() >= 0)
            movePosition(element.attribute("pos").toInt());

    if(element.hasAttribute("len"))
        changeLength(element.attribute("len").toInt());

    if(element.hasAttribute("autoresize"))
        setAutoResize(element.attribute("autoresize").toInt());

    if(element.hasAttribute("autorepeat"))
        setAutoRepeat(element.attribute("autorepeat").toInt());

    if(element.hasAttribute("muted"))
        if(element.attribute("muted").toInt() != isMuted())
            toggleMute();

    if(element.hasAttribute("color"))
        setColor(QColor(element.attribute("color").toUInt()));

    if(element.hasAttribute("usestyle"))
        setUseStyleColor(element.attribute("usestyle").toUInt() == 1);
}

bool Tile::isFixed() const
{
    return track()->isFixed();
}

QColor Tile::color() const
{
    return m_color;
}

void Tile::setColor(const QColor& _c)
{
    m_color = _c;
}

bool Tile::useStyleColor() const
{
    return m_useStyleColor;
}

void Tile::setUseStyleColor(bool _b)
{
    m_useStyleColor = _b;
}

/*! \brief Move this Tile's position in time
 *
 *  If the track content object has moved, update its position.  We
 *  also add a journal entry for undo and update the display.
 *
 * \param _pos The new position of the track content object.
 */
void Tile::movePosition(const MidiTime& pos)
{
    if(m_startPosition != pos)
    {
        m_startPosition = pos;
        Engine::song()->updateLength();
        emit positionChanged();
    }
}

/*! \brief Change the length of this Tile
 *
 *  If the track content object's length has chaanged, update it.  We
 *  also add a journal entry for undo and update the display.
 *
 * \param _length The new length of the track content object.
 */
void Tile::changeLength(const MidiTime& _length)
{
    // qInfo("Tile::changeLength #1");

    real_t nom          = Engine::song()->getTimeSigModel().getNumerator();
    real_t den          = Engine::song()->getTimeSigModel().getDenominator();
    int    ticksPerTact = DefaultTicksPerTact * (nom / den);

    tick_t len = qMax(static_cast<tick_t>(_length), ticksPerTact / 32);

    if(m_length != len)
    {
        m_length = len;
        m_steps  = len * m_stepResolution * stepsPerTact()
                  / MidiTime::ticksPerTact() / 16;
        // qInfo("Tile::changeLength #2");
        Engine::song()->updateLength();
        // qInfo("Tile::changeLength #3");
        emit lengthChanged();
    }
}

void Tile::updateLength()
{
    updateLength(length());
}

void Tile::updateLength(tick_t _len)
{
    if(isFixed())
    {
        if(m_stepResolution == 0)
            m_stepResolution = DefaultStepsPerTact;
        if(m_steps == 0)
            m_steps = m_stepResolution;
        _len = m_steps * MidiTime::ticksPerTact() * 16 / m_stepResolution
               / stepsPerTact();
    }

    changeLength(_len);
    // updateBBTrack();
}

void Tile::resizeLeft(const MidiTime& pos, const MidiTime& len)
{
    movePosition(pos);
    changeLength(len);
}

void Tile::resizeRight(const MidiTime& pos, const MidiTime& len)
{
    movePosition(pos);
    changeLength(len);
}

int Tile::stepsPerTact() const
{
    int steps = MidiTime::ticksPerTact() / DefaultBeatsPerTact;
    return qMax(1, steps);
}

MidiTime Tile::stepPosition(int _step) const
{
    return _step * 16. / m_stepResolution * MidiTime::ticksPerTact()
           / stepsPerTact();
}

int Tile::stepResolution() const
{
    return m_stepResolution;
}

void Tile::setStepResolution(int _res)
{
    if(_res > 0)
    {
        int old = m_stepResolution;
        if(_res != old)
        {
            m_steps          = qMax(1, (m_steps * _res / old));
            m_stepResolution = _res;
            updateLength();
            emit dataChanged();
        }
    }
}

void Tile::addBarSteps()
{
    m_steps += stepsPerTact();
    updateLength();
    emit dataChanged();
    Engine::song()->setModified();
}

void Tile::addBeatSteps()
{
    m_steps += stepsPerTact()
               / Engine::song()->getTimeSigModel().getNumerator();
    updateLength();
    emit dataChanged();
    Engine::song()->setModified();
}

void Tile::addOneStep()
{
    m_steps++;  //= stepsPerTact();
    updateLength();
    emit dataChanged();
    Engine::song()->setModified();
}

void Tile::removeBarSteps()
{
    int n = stepsPerTact();
    if(n < m_steps)
    {
        /*
        for( int i = m_steps - n; i < m_steps; ++i )
        {
                setStep( i, false );
        }
        */
        m_steps -= n;
        updateLength();
        emit dataChanged();
        Engine::song()->setModified();
    }
}

void Tile::removeBeatSteps()
{
    int n = stepsPerTact() / Engine::song()->getTimeSigModel().getNumerator();
    if(n < m_steps)
    {
        /*
        for( int i = m_steps - n; i < m_steps; ++i )
        {
                setStep( i, false );
        }
        */
        m_steps -= n;
        updateLength();
        emit dataChanged();
        Engine::song()->setModified();
    }
}

void Tile::removeOneStep()
{
    int n = 1;
    if(n < m_steps)
    {
        /*
        for( int i = m_steps - n; i < m_steps; ++i )
        {
                setStep( i, false );
        }
        */
        m_steps -= n;
        updateLength();
        emit dataChanged();
        Engine::song()->setModified();
    }
}

void Tile::rotateOneStepLeft()
{
    rotate(-MidiTime::ticksPerTact() / stepsPerTact());
}

void Tile::rotateOneStepRight()
{
    rotate(MidiTime::ticksPerTact() / stepsPerTact());
}

void Tile::rotateOneBeatLeft()
{
    rotate(-MidiTime::ticksPerTact()
           / Engine::song()->getTimeSigModel().getNumerator());
}

void Tile::rotateOneBeatRight()
{
    rotate(MidiTime::ticksPerTact()
           / Engine::song()->getTimeSigModel().getNumerator());
}

void Tile::rotateOneBarLeft()
{
    rotate(-MidiTime::ticksPerTact());
}

void Tile::rotateOneBarRight()
{
    rotate(MidiTime::ticksPerTact());
}

void Tile::splitAfterEveryBar()
{
    split(MidiTime::ticksPerTact());
}

void Tile::splitAfterEveryFourthBar()
{
    split(4 * MidiTime::ticksPerTact());
}

/*
bool Tile::comparePosition(const Tile* a,
                                         const Tile* b)
{
    return a->startPosition() < b->startPosition();
}
*/

void Tile::clear()
{
}

/*! \brief Copy this Tile to the clipboard.
 *
 *  Copies this track content object to the clipboard.
 */
void Tile::copy()
{
    Clipboard::copy(this);
}

/*! \brief Pastes this Tile into a track.
 *
 *  Pastes this track content object into a track.
 *
 * \param _je The journal entry to undo
 */
void Tile::paste()
{
    if(Clipboard::has(nodeName()))
    {
        const MidiTime pos = startPosition();
        Clipboard::paste(this);
        movePosition(pos);
    }
    /*
    if( Clipboard::getContent( nodeName() ) != nullptr )
    {
            const MidiTime pos = startPosition();
            restoreState( *( Clipboard::getContent( nodeName() ) ) );
            movePosition( pos );
    }
    */
    AutomationPattern::resolveAllIDs();
    gui->automationWindow()->m_editor->updateAfterPatternChange();
}

/*! \brief Mutes this Tile
 *
 *  Restore the previous state of this track content object.  This will
 *  restore the position or the length of the track content object
 *  depending on what was changed.
 *
 * \param _je The journal entry to undo
 */
void Tile::toggleMute()
{
    m_mutedModel.setValue(!m_mutedModel.value());
    emit dataChanged();
}

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
      SelectableObject(tv->getTrackContentWidget()), ModelView(nullptr, this),
      m_tco(tco), m_trackView(tv), m_action(NoAction),
      m_initialMousePos(QPoint(0, 0)), m_initialMouseGlobalPos(QPoint(0, 0)),
      m_hint(nullptr), m_mutedColor(0, 0, 0), m_mutedBackgroundColor(0, 0, 0),
      m_selectedColor(0, 0, 0), m_textColor(0, 0, 0),
      m_textShadowColor(0, 0, 0), m_BBPatternBackground(0, 0, 0),
      // m_gradient( true ),
      m_needsUpdate(true)
{
    if(s_textFloat == nullptr)
    {
        s_textFloat = new TextFloat;
        s_textFloat->setPixmap(embed::getPixmap("clock"));
    }

    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(QCursor(embed::getPixmap("hand"), 3, 3));
    move(0, 0);
    show();

    setFixedHeight(tv->getTrackContentWidget()->height() - 1);
    setAcceptDrops(true);
    setMouseTracking(true);

    connect(m_tco, SIGNAL(lengthChanged()), this, SLOT(updateLength()));
    connect(gui->songWindow()->m_editor->zoomingXModel(),
            SIGNAL(dataChanged()), this, SLOT(updateLength()));
    connect(m_tco, SIGNAL(positionChanged()), this, SLOT(updatePosition()));
    connect(m_tco, SIGNAL(destroyedTCO()), this, SLOT(close()));
    setModel(m_tco);

    m_trackView->getTrackContentWidget()->addTCOView(this);
    updateLength();
    updatePosition();
}

/*! \brief Destroy a TileView
 *
 *  Destroys the given track content object view.
 *
 */
TileView::~TileView()
{
    delete m_hint;
    // we have to give our track-container the focus because otherwise the
    // op-buttons of our track-widgets could become focus and when the user
    // presses space for playing song, just one of these buttons is pressed
    // which results in unwanted effects
    m_trackView->trackContainerView()->setFocus();
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
        Engine::getBBTrackContainer()->updateBBTrack(m_tco);

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
    m_trackView->getTrackContentWidget()->removeTCOView(this);
    return QWidget::close();
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
    m_trackView->track()->addJournalCheckPoint();

    // delete ourself
    close();
    m_tco->deleteLater();
}

void TileView::mute()
{
    m_tco->toggleMute();
}

void TileView::clear()
{
    m_tco->clear();
}

/*! \brief Cut this TileView from its track to the clipboard.
 *
 *  Perform the 'cut' action of the clipboard - copies the track content
 *  object to the clipboard and then removes it from the track.
 */
void TileView::cut()
{
    m_tco->copy();
    remove();
}

void TileView::copy()
{
    m_tco->copy();
}

void TileView::paste()
{
    m_tco->paste();
}

void TileView::changeAutoResize()
{
    m_tco->setAutoResize(!m_tco->autoResize());
}

void TileView::changeAutoRepeat()
{
    m_tco->setAutoRepeat(!m_tco->autoRepeat());
}

void TileView::changeName()
{
    QString      s = m_tco->name();
    RenameDialog rename_dlg(s);
    rename_dlg.exec();
    m_tco->setName(s);
}

void TileView::resetName()
{
    m_tco->setName(m_tco->track()->name());
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
        QVector<SelectableObject*> selected
                = gui->songWindow()->m_editor->selectedObjects();
        for(QVector<SelectableObject*>::iterator it = selected.begin();
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
        QVector<SelectableObject*> selected
                = gui->songWindow()->m_editor->selectedObjects();
        for(QVector<SelectableObject*>::iterator it = selected.begin();
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
    return m_tco->isFixed();
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
    return m_tco->useStyleColor();
}

void TileView::setUseStyleColor(bool _use)
{
    if(_use != m_tco->useStyleColor())
    {
        m_tco->setUseStyleColor(_use);
        Engine::song()->setModified();
        update();
    }
}

QColor TileView::color() const
{
    return m_tco->color();
}

void TileView::setColor(const QColor& new_color)
{
    if(new_color != m_tco->color())
    {
        m_tco->setColor(new_color);
        m_tco->setUseStyleColor(false);
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
    // if( fixedTCOs() )
    //{
    //	setFixedWidth( parentWidget()->width() );
    //}
    // else
    {
        real_t w = pixelsPerTact() * m_tco->length()
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
    if(m_tco->track()->isFrozen())
    {
        dee->ignore();
        return;
    }

    TrackContentWidget* tcw = getTrackView()->getTrackContentWidget();
    MidiTime tcoPos         = MidiTime(m_tco->startPosition().getTact(), 0);

    if(tcw->canPasteSelection(tcoPos, dee->mimeData()) == false)
    {
        dee->ignore();
    }
    else
    {
        StringPairDrag::processDragEnterEvent(
                dee, "tco_" + QString::number(m_tco->track()->type()));
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
    if(m_tco->track()->isFrozen())
    {
        de->ignore();
        return;
    }

    QString type  = StringPairDrag::decodeKey(de);
    QString value = StringPairDrag::decodeValue(de);

    // Track must be the same type to paste into
    if(type != ("tco_" + QString::number(m_tco->track()->type())))
    {
        de->ignore();
        return;
    }

    // Defer to rubberband paste if we're in that mode
    if(m_trackView->trackContainerView()->allowRubberband() == true)
    {
        TrackContentWidget* tcw = getTrackView()->getTrackContentWidget();
        MidiTime tcoPos = MidiTime(m_tco->startPosition().getTact(), 0);
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
    MidiTime    pos  = m_tco->startPosition();
    QDomElement tcos = dataFile.content().firstChildElement("tcos");
    m_tco->restoreState(tcos.firstChildElement().firstChildElement());
    m_tco->movePosition(pos);
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

    if(m_tco->track()->isFrozen())
    {
        if(_le != nullptr)
            _le->ignore();
        return;
    }

    if(_le != nullptr)
        QWidget::leaveEvent(_le);
}

/*! \brief Create a DataFile suitable for copying multiple
 *Tiles.
 *
 *	Tiles in the vector are written to the "tcos" node in
 *the DataFile.  The TileView's initial mouse position is
 *written to the "initialMouseX" node in the DataFile.  When dropped on a
 *track, this is used to create copies of the TCOs.
 *
 * \param tcos The trackContectObjects to save in a DataFile
 */
DataFile
        TileView::createTCODataFiles(const QVector<TileView*>& tcoViews) const
{
    Track*          t  = m_trackView->track();
    TrackContainer* tc = t->trackContainer();
    DataFile        dataFile(DataFile::DragNDropData);
    QDomElement     tcoParent = dataFile.createElement("tcos");

    typedef QVector<TileView*> tcoViewVector;
    for(tcoViewVector::const_iterator it = tcoViews.begin();
        it != tcoViews.end(); ++it)
    {
        // Insert into the dom under the "tcos" element
        int trackIndex = tc->tracks().indexOf((*it)->m_trackView->track());
        QDomElement tcoElement = dataFile.createElement("tco");
        tcoElement.setAttribute("trackIndex", trackIndex);
        (*it)->m_tco->saveState(dataFile, tcoElement);
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
    metadata.setAttribute("grabbedTCOPos", m_tco->startPosition());

    dataFile.content().appendChild(metadata);

    return dataFile;
}

void TileView::paintTileLoop(QPainter& p)
{
    const Track* t = m_tco->track();
    const int    n = t->currentLoop();
    if(n < 0 || n >= TimeLineWidget::NB_LOOPS)
        return;

    const Song*           song = Engine::song();
    const TimeLineWidget* tl
            = song->getPlayPos(Song::Mode_PlaySong).m_timeLine;
    if(tl == nullptr)
        return;

    const MidiTime& tstart = m_tco->startPosition();
    const MidiTime& tend   = m_tco->endPosition();
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
    if(m_tco->track()->isFrozen())
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
                gui->songWindow()->m_editor->selectAllTcos(false);
                QVector<TileView*> tcoViews;
                tcoViews.push_back(this);
                QString data = createTCODataFiles(tcoViews)
                                       .toString();  // DataFile dataFile
                QPixmap thumbnail = grab();
                if((thumbnail.width() > 128) || (thumbnail.height() > 128))
                    thumbnail
                            = thumbnail.scaled(128, 128, Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
                new StringPairDrag(
                        QString("tco_%1").arg(m_tco->track()->type()), data,
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
            gui->songWindow()->m_editor->selectAllTcos(false);
            m_tco->addJournalCheckPoint();

            // move or resize
            m_tco->setJournalling(false);

            setInitialMousePos(me->pos());

            if(!m_tco->autoResize() && !m_tco->isFixed()
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
                                .arg(m_tco->length().getTact())
                                .arg(m_tco->length().getTicks()
                                     % MidiTime::ticksPerTact())
                                .arg(m_tco->startPosition().getTact() + 1)
                                .arg(m_tco->startPosition().getTicks()
                                     % MidiTime::ticksPerTact())
                                .arg(m_tco->endPosition().getTact() + 1)
                                .arg(m_tco->endPosition().getTicks()
                                     % MidiTime::ticksPerTact()));
                s_textFloat->moveGlobal(this,
                                        QPoint(width() + 2, height() + 2));
            }
            else if(!m_tco->autoResize() && !m_tco->isFixed()
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
                                .arg(m_tco->length().getTact())
                                .arg(m_tco->length().getTicks()
                                     % MidiTime::ticksPerTact())
                                .arg(m_tco->startPosition().getTact() + 1)
                                .arg(m_tco->startPosition().getTicks()
                                     % MidiTime::ticksPerTact())
                                .arg(m_tco->endPosition().getTact() + 1)
                                .arg(m_tco->endPosition().getTicks()
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
                                .arg(m_tco->startPosition().getTact() + 1)
                                .arg(m_tco->startPosition().getTicks()
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
            m_tco->toggleMute();
        else if(me->modifiers() & Qt::ShiftModifier && !isFixed())
            remove();
    }
    else if(me->button() == Qt::MidButton)
    {
        if(me->modifiers() & Qt::ControlModifier)
            m_tco->toggleMute();
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
    if(m_tco->track()->isFrozen())
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
            QVector<TileView*>         tcoViews;
            QVector<SelectableObject*> so
                    = m_trackView->trackContainerView()->selectedObjects();
            for(QVector<SelectableObject*>::iterator it = so.begin();
                it != so.end(); ++it)
            {
                TileView* tcov = dynamic_cast<TileView*>(*it);
                if(tcov != nullptr)
                {
                    tcoViews.push_back(tcov);
                }
            }

            // Write the TCOs to the DataFile for copying
            QString data = createTCODataFiles(tcoViews)
                                   .toString();  // DataFile dataFile

            // TODO -- thumbnail for all selected
            QPixmap thumbnail = grab();
            if((thumbnail.width() > 128) || (thumbnail.height() > 128))
                thumbnail = thumbnail.scaled(128, 128, Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
            new StringPairDrag(QString("tco_%1").arg(m_tco->track()->type()),
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
        m_tco->movePosition(t);
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

        tick_t smallest_pos = m_tco->startPosition();

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

        tick_t t = m_tco->startPosition() + delta;
        m_tco->movePosition(t);

        m_trackView->getTrackContentWidget()->changePosition();

        s_textFloat->setText(
                QString("%1:%2")
                        .arg(m_tco->startPosition().getTact() + 1)
                        .arg(m_tco->startPosition().getTicks()
                             % MidiTime::ticksPerTact()));
        s_textFloat->moveGlobal(this, QPoint(width() + 2, height() + 2));
    }
    else if(m_action == MoveSelection)
    {
        QVector<SelectableObject*> so
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

        QVector<Tile*> tcos;
        // MidiTime::ticksPerTact()
        // find out smallest position of all selected objects for not
        // moving an object before zero
        bool   first        = true;
        tick_t smallest_pos = 0;
        for(QVector<SelectableObject*>::iterator it = so.begin();
            it != so.end(); ++it)
        {
            TileView* tcov = dynamic_cast<TileView*>(*it);
            if(tcov == nullptr)
                continue;

            Tile* tco = tcov->m_tco;
            tcos.push_back(tco);
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
        for(QVector<Tile*>::iterator it = tcos.begin(); it != tcos.end();
            ++it)
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

        tick_t smallest_len = m_tco->length();

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

        tick_t nl = m_tco->length() + delta;
        if(nl < MidiTime::ticksPerTact() / 8)
            return;
        // qInfo("before: m_initialMousePos %d,%d", m_initialMousePos.x(),
        //  m_initialMousePos.y());

        tick_t sp = m_tco->startPosition();
        m_tco->resizeRight(sp, nl);
        // m_tco->changeLength(nl);

        m_initialMousePos = QPoint(width(), height() / 2);

        s_textFloat->setText(
                tr("%1:%2 (%3:%4 to %5:%6)")
                        .arg(m_tco->length().getTact())
                        .arg(m_tco->length().getTicks()
                             % MidiTime::ticksPerTact())
                        .arg(m_tco->startPosition().getTact() + 1)
                        .arg(m_tco->startPosition().getTicks()
                             % MidiTime::ticksPerTact())
                        .arg(m_tco->endPosition().getTact() + 1)
                        .arg(m_tco->endPosition().getTicks()
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

        tick_t smallest_len = m_tco->length();

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
        tick_t nl = m_tco->length() - delta;
        qInfo("nl=%d", nl);
        if(nl < MidiTime::ticksPerTact() / 8)
            return;
        tick_t sp = m_tco->startPosition() + delta;
        if(sp < 0)
            return;

        m_tco->resizeLeft(sp, nl);
        // m_tco->movePosition(sp);
        // m_tco->changeLength(nl);

        m_initialMousePos = QPoint(0, height() / 2);

        s_textFloat->setText(
                tr("%1:%2 (%3:%4 to %5:%6)")
                        .arg(m_tco->length().getTact())
                        .arg(m_tco->length().getTicks()
                             % MidiTime::ticksPerTact())
                        .arg(m_tco->startPosition().getTact() + 1)
                        .arg(m_tco->startPosition().getTicks()
                             % MidiTime::ticksPerTact())
                        .arg(m_tco->endPosition().getTact() + 1)
                        .arg(m_tco->endPosition().getTicks()
                             % MidiTime::ticksPerTact()));
        s_textFloat->moveGlobal(this, QPoint(width() + 2, height() + 2));
    }
    else
    {
        if((me->x() >= width() - RESIZE_GRIP_WIDTH
            || me->x() < RESIZE_GRIP_WIDTH)
           && !me->buttons() && !m_tco->autoResize() && !m_tco->isFixed())
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

    if(m_tco->track()->isFrozen())
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
        m_tco->setJournalling(true);
        Selection::select(m_tco);
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
    if(m_tco->track()->isFrozen())
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
                                    tr( "Copy" ), m_tco, SLOT( copy() ) );
    contextMenu.addAction( embed::getIcon( "edit_paste" ),
                                    tr( "Paste" ), m_tco, SLOT( paste() ) );
    contextMenu.addSeparator();
    contextMenu.addAction( embed::getIcon( "muted" ),
                           tr( "Mute/unmute (<%1> + middle click)" )
                           .arg(UI_CTRL_KEY),
                           m_tco, SLOT( toggleMute() ) );
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
                       m_tco, SLOT(splitAfterEveryBar()));
    a->setEnabled(_one);
    a = sms->addAction(embed::getIcon("note_four_whole"),
                       tr("after every fourth bar"), m_tco,
                       SLOT(splitAfterEveryFourthBar()));
    a->setEnabled(_four);

    _cm->addMenu(sms);
}

void TileView::addFlipMenu(QMenu* _cm, bool _x, bool _y)
{
    QAction* a;
    a = _cm->addAction(embed::getIcon("flip_x"), tr("Flip horizontally"),
                       m_tco, SLOT(flipHorizontally()));
    a->setEnabled(_x);
    a = _cm->addAction(embed::getIcon("flip_y"), tr("Flip vertically"), m_tco,
                       SLOT(flipVertically()));
    a->setEnabled(_y);
}

void TileView::addRotateMenu(QMenu* _cm, bool _bar, bool _beat, bool _step)
{
    QMenu* smrl = new QMenu(tr("Rotate left"));
    QMenu* smrr = new QMenu(tr("Rotate right"));

    QAction* a;
    a = smrl->addAction(embed::getIcon("arrow_first"), tr("One bar"), m_tco,
                        SLOT(rotateOneBarLeft()));
    a->setEnabled(_bar);
    a = smrr->addAction(embed::getIcon("arrow_last"), tr("One bar"), m_tco,
                        SLOT(rotateOneBarRight()));
    a->setEnabled(_bar);

    a = smrl->addAction(embed::getIcon("arrow_first"), tr("One beat"), m_tco,
                        SLOT(rotateOneBeatLeft()));
    a->setEnabled(_beat);
    a = smrr->addAction(embed::getIcon("arrow_last"), tr("One beat"), m_tco,
                        SLOT(rotateOneBeatRight()));
    a->setEnabled(_beat);

    a = smrl->addAction(embed::getIcon("arrow_left"), tr("One step"), m_tco,
                        SLOT(rotateOneStepLeft()));
    a->setEnabled(_step);
    a = smrr->addAction(embed::getIcon("arrow_right"), tr("One step"), m_tco,
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
    a = sma->addAction(embed::getIcon("step_btn_add"), tr("One bar"), m_tco,
                       SLOT(addBarSteps()));
    a->setEnabled(_bar);
    smr->addAction(embed::getIcon("step_btn_remove"), tr("One bar"), m_tco,
                   SLOT(removeBarSteps()));
    a->setEnabled(_bar);

    sma->addAction(embed::getIcon("step_btn_add"), tr("One beat"), m_tco,
                   SLOT(addBeatSteps()));
    a->setEnabled(_beat);
    smr->addAction(embed::getIcon("step_btn_remove"), tr("One beat"), m_tco,
                   SLOT(removeBeatSteps()));
    a->setEnabled(_beat);

    sma->addAction(embed::getIcon("step_btn_add"), tr("One step"), m_tco,
                   SLOT(addOneStep()));
    a->setEnabled(_step);
    smr->addAction(embed::getIcon("step_btn_remove"), tr("One step"), m_tco,
                   SLOT(removeOneStep()));
    a->setEnabled(_step);

    //_cm->addSeparator();
    _cm->addMenu(sma);
    _cm->addMenu(smr);
    //_cm->addAction( embed::getIcon( "step_btn_duplicate" ),
    //                tr( "Clone Steps" ), m_tco, SLOT( cloneSteps() ) );

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
    a->setChecked(m_tco->autoResize());
    a->setEnabled(_resize);
    // embed::getIcon("reload"),
    a = _cm->addAction(tr("Autorepeat"), this, SLOT(changeAutoRepeat()));
    a->setCheckable(true);
    a->setChecked(m_tco->autoRepeat());
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
void TrackContentWidget::addTCOView(TileView* tcov)
{
    Tile* tco = tcov->getTile();

    m_tcoViews.push_back(tcov);

    tco->saveJournallingState(false);
    changePosition();
    tco->restoreJournallingState();
}

/*! \brief Removes the given TileView to this widget.
 *
 *  Removes the given TileView from our list of views.
 *
 * \param tcov The TileView to add.
 */
void TrackContentWidget::removeTCOView(TileView* tcov)
{
    tcoViewVector::iterator it
            = qFind(m_tcoViews.begin(), m_tcoViews.end(), tcov);
    if(it != m_tcoViews.end())
    {
        m_tcoViews.erase(it);
        Engine::song()->setModified();
    }
}

/*! \brief Update ourselves by updating all the tCOViews attached.
 *
 */
void TrackContentWidget::update()
{
    for(tcoViewVector::iterator it = m_tcoViews.begin();
        it != m_tcoViews.end(); ++it)
    {
        (*it)->setFixedHeight(height() - 1);
        (*it)->update();
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
    if(m_trackView->trackContainerView()
       == gui->bbWindow()->trackContainerView())
    {
        const int curBB = Engine::getBBTrackContainer()->currentBB();
        setUpdatesEnabled(false);

        // first show TCO for current BB...
        for(tcoViewVector::iterator it = m_tcoViews.begin();
            it != m_tcoViews.end(); ++it)
        {
            if((*it)->getTile()->startPosition().getTact() == curBB)
            {
                (*it)->move(0, (*it)->y());
                (*it)->raise();
                (*it)->show();
            }
            else
            {
                (*it)->lower();
            }
        }
        // ...then hide others to avoid flickering
        for(tcoViewVector::iterator it = m_tcoViews.begin();
            it != m_tcoViews.end(); ++it)
        {
            if((*it)->getTile()->startPosition().getTact() != curBB)
            {
                (*it)->hide();
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

    const int    begin = pos;
    const int    end   = endPosition(pos);
    const real_t ppt   = pixelsPerTact();

    setUpdatesEnabled(false);
    for(tcoViewVector::iterator it = m_tcoViews.begin();
        it != m_tcoViews.end(); ++it)
    {
        TileView* tcov = *it;
        Tile*     tco  = tcov->getTile();

        tco->changeLength(tco->length());

        const int ts = tco->startPosition();
        const int te = tco->endPosition() - 3;
        if((ts >= begin && ts <= end) || (te >= begin && te <= end)
           || (ts <= begin && te >= end))
        {
            tcov->move(static_cast<int>((ts - begin) * ppt
                                        / MidiTime::ticksPerTact()),
                       tcov->y());
            if(!tcov->isVisible())
            {
                tcov->show();
            }
        }
        else
        {
            tcov->move(-tcov->width() - 10, tcov->y());
        }
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
    {
        return false;
    }

    QString type  = StringPairDrag::decodeKey(mimeData);
    QString value = StringPairDrag::decodeValue(mimeData);

    // We can only paste into tracks of the same type
    if(type != ("tco_" + QString::number(t->type()))
       || isFixed())  // trackContainerView()->fixedTCOs() == true )
    {
        return false;
    }

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
    {
        return false;
    }

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
        {
            return false;
        }

        // Track must be of the same type
        Track* startTrack = tracks.at(trackIndex);
        Track* endTrack   = tracks.at(finalTrackIndex);
        if(startTrack->type() != endTrack->type())
        {
            return false;
        }
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
    {
        return false;
    }

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
    const QVector<SelectableObject*> so
            = m_trackView->trackContainerView()->selectedObjects();
    for(QVector<SelectableObject*>::const_iterator it = so.begin();
        it != so.end(); ++it)
    {
        (*it)->setSelected(false);
    }

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

        Tile* tco = t->createTCO(pos);
        tco->restoreState(tcoElement);
        tco->movePosition(pos);
        if(wasSelection)
        {
            tco->selectViewOnCreate(true);
        }

        // check tco name, if the same as source track name dont copy
        if(tco->name() == tracks[trackIndex]->name())
        {
            tco->setName("");
        }
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
        Tile*    tco = t->createTCO(pos);
        QString  nn  = tco->nodeName();

        tco->saveJournallingState(false);
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
        QVector<SelectableObject*> so = m_trackView->trackContainerView()
                                                ->rubberBand()
                                                ->selectedObjects();
        for(int i = 0; i < so.count(); ++i)
        {
            so.at(i)->setSelected(false);
        }
        track()->addJournalCheckPoint();
        const MidiTime pos
                = getPosition(me->x()).getTact() * MidiTime::ticksPerTact();
        Tile* tco = track()->createTCO(pos);

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
        m_muteBtn->setGeometry(39, 0, 10, 14);
        m_soloBtn->setGeometry(39, 16, 10, 14);
        m_frozenBtn->setGeometry(52, 0, 10, 14);
        m_clippingBtn->setGeometry(52, 16, 10, 14);
    }
    else
    {
        m_muteBtn->move(39, 4);      // setGeometry(45, 4,16,14);
        m_soloBtn->move(39, 18);     // setGeometry(62, 4,16,14);
        m_clippingBtn->move(54, 4);  // setGeometry(62,18,16,14);
        m_frozenBtn->move(54, 18);   // setGeometry(62,18,16,14);
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
    if(newTrack->trackContainer() == Engine::getBBTrackContainer())
        newTrack->createTCOsForBB(Engine::getBBTrackContainer()->numOfBBs()
                                  - 1);
    newTrack->unlockTrack();
}

/*! \brief Clear this track - clears all TCOs from the track */
void TrackOperationsWidget::clearTrack()
{
    Track* t = m_trackView->track();
    t->lockTrack();
    t->deleteTCOs();
    if(t->trackContainer() == Engine::getBBTrackContainer())
        t->createTCOsForBB(Engine::getBBTrackContainer()->numOfBBs() - 1);
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

    const int                   newidxbb = bbt->index();
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
    if(oldidxbb < 0 || oldidxbb > bbtc->numOfBBs() - 1)
        qWarning("TrackOperationsWidget::isolateTrack#1 oldidxbb=%d",
                 oldidxbb);

    /*const*/ BBTrack* oldbbt = BBTrack::findBBTrack(oldidxbb);
    if(!oldbbt)
    {
        qCritical("TrackOperationsWidget::isolateTrack oldbbt=null!!!");
        return;
    }

    BBTrack* newbbt = dynamic_cast<BBTrack*>(oldbbt->clone());
    newbbt->setName(instr->name() /*+" isolated"*/);  // use the name of the
                                                      // instrument

    int newidxbb = newbbt->index();
    if(newidxbb < 0 || newidxbb > bbtc->numOfBBs() - 1)
        qWarning("TrackOperationsWidget::isolateTrack#2 newidxbb=%d",
                 newidxbb);
    bbtc->setCurrentBB(newidxbb);

    // qInfo("TrackOperationsWidget::isolateTrack start cleaning");
    for(Track* t: bbtc->tracks())
    {
        /*TrackView* tv=*/tcview->createTrackView(t);
        if(t == instr)
            continue;

        // qInfo("TrackOperationsWidget::isolateTrack clear all notes in
        // %s",qPrintable(t->name()));
        Tile* o = t->getTCO(newidxbb);
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
            QVector<SelectableObject*> selected =
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
            QVector<SelectableObject*> selected =
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
// track
// ===========================================================================

/*! \brief Create a new (empty) track object
 *
 *  The track object is the whole track, linking its contents, its
 *  automation, name, type, and so forth.
 *
 * \param type The type of track (Song Editor or Beat+Bassline Editor)
 * \param tc The track Container object to encapsulate in this track.
 *
 * \todo check the definitions of all the properties - are they OK?
 */
Track::Track(TrackType type, TrackContainer* tc) :
      Model(tc, "Track"), /*!< The track Model */
      m_frozenModel(false, this, tr("Frozen"), "frozen"),
      /*!< For controlling track freezing */
      m_clippingModel(false, this, tr("Clipping"), "clipping"),
      /*!< For showing track clipping alerts */
      m_mutedModel(false, this, tr("Mute"), "mute"),
      /*!< For controlling track muting */
      m_soloModel(false, this, tr("Solo"), "solo"),
      /*!< For controlling track soloing */
      m_loopEnabledModel(false, this, tr("Loop Enabled"), "loopEnabled"),
      m_currentLoopModel(0,
                         0,
                         TimeLineWidget::NB_LOOPS - 1,
                         this,
                         tr("Current Loop"),
                         "currentLoop"),
      m_trackContainer(tc), /*!< The track container object */
      m_type(type),         /*!< The track type */
      m_name(),             /*!< The track's name */
      m_color(Qt::white), m_useStyleColor(true),
      m_simpleSerializingMode(false),
      m_tiles(), /*!< The track content objects (segments) */
      m_processingLock("Track::m_processingLock", QMutex::Recursive, false)
{
    m_trackContainer->addTrack(this);
    m_height = -1;

    connect(&m_soloModel, SIGNAL(dataChanged()), this, SLOT(toggleSolo()));

    connect(&m_frozenModel, SIGNAL(dataChanged()), this,
            SLOT(toggleFrozen()));
}

/*! \brief Destroy this track
 *
 *  If the track container is a Beat+Bassline container, step through
 *  its list of tracks and remove us.
 *
 *  Then delete the Tile's contents, remove this track from
 *  the track container.
 *
 *  Finally step through this track's automation and forget all of them.
 */
Track::~Track()
{
    qInfo("Track::~Track 0 [%s]", qPrintable(name()));
    // setMuted(true);  // <-- DEBUG
    lockTrack();
    qInfo("Track::~Track 1");
    clearAllTrackPlayHandles();
    emit destroyedTrack();
    qInfo("Track::~Track 2");
    deleteTCOs();
    qInfo("Track::~Track 3");
    m_trackContainer->removeTrack(this);
    unlockTrack();
    qInfo("Track::~Track 4");
}

QString Track::objectName() const
{
    QString r = QObject::objectName();
    if(r.isEmpty() || r == "track")
        r = QString("track%1").arg(trackIndex() + 1);
    return r;
}

bool Track::isFixed() const
{
    return m_trackContainer->isFixed();
}

QColor Track::color() const
{
    return m_color;
}

void Track::setColor(const QColor& _c)
{
    m_color = _c;
}

bool Track::useStyleColor() const
{
    return m_useStyleColor;
}

void Track::setUseStyleColor(bool b)
{
    m_useStyleColor = b;
}

int Track::currentLoop() const
{
    if(m_loopEnabledModel.value())
        return m_currentLoopModel.value();
    else
        return -1;
}

void Track::setCurrentLoop(int _loop)
{
    if(_loop < 0 || _loop >= TimeLineWidget::NB_LOOPS)
    {
        m_loopEnabledModel.setValue(false);
        return;
    }

    if(m_loopEnabledModel.value() && m_currentLoopModel.value() == _loop)
    {
        m_loopEnabledModel.setValue(false);
    }
    else
    {
        m_loopEnabledModel.setValue(true);
        m_currentLoopModel.setValue(_loop);
    }

    qInfo("Track: loop=%d enabled=%d", m_currentLoopModel.value(),
          m_loopEnabledModel.value());
}

/*
void Track::selectSubloop(const MidiTime& _pos)
{
    qWarning("Track::selectSubloop %d %s",_pos.getTicks(),qPrintable(name()));
}
*/

/*! \brief Create a track based on the given track type and container.
 *
 *  \param tt The type of track to create
 *  \param tc The track container to attach to
 */
Track* Track::create(TrackType tt, TrackContainer* tc)
{
    Engine::mixer()->requestChangeInModel();

    Track* t = nullptr;

    switch(tt)
    {
        case InstrumentTrack:
            t = new ::InstrumentTrack(tc);
            break;
        case BBTrack:
            t = new ::BBTrack(tc);
            break;
        case SampleTrack:
            t = new ::SampleTrack(tc);
            break;
            //		case EVENT_TRACK:
            //		case VIDEO_TRACK:
        case AutomationTrack:
            t = new ::AutomationTrack(tc);
            break;
        case HiddenAutomationTrack:
            t = new ::AutomationTrack(tc, true);
            break;
        default:
            break;
    }

    if(tc == Engine::getBBTrackContainer() && t)
    {
        t->createTCOsForBB(Engine::getBBTrackContainer()->numOfBBs() - 1);
    }

    tc->updateAfterTrackAdd();

    Engine::mixer()->doneChangeInModel();

    return t;
}

/*! \brief Create a track inside TrackContainer from track type in a
 * QDomElement and restore state from XML
 *
 *  \param element The QDomElement containing the type of track to create
 *  \param tc The track container to attach to
 */
Track* Track::create(const QDomElement& element, TrackContainer* tc)
{
    Engine::mixer()->requestChangeInModel();

    Track* t = create(
            static_cast<TrackType>(element.attribute("type").toInt()), tc);

    if(t != nullptr)
        t->restoreState(element);

    Engine::mixer()->doneChangeInModel();

    return t;
}

/*! \brief Clone a track from this track
 *
 */
Track* Track::clone()
{
    QDomDocument doc;
    QDomElement  parent = doc.createElement("clone");
    saveState(doc, parent);
    Track* r = create(parent.firstChild().toElement(), m_trackContainer);
    r->resetUuid();  // m_uuid = "";
    return r;
}

/*! \brief Save this track's settings to file
 *
 *  We save the track type and its muted state and solo state, then append the
 * track- specific settings.  Then we iterate through the Tiles
 *  and save all their states in turn.
 *
 *  \param doc The QDomDocument to use to save
 *  \param element The The QDomElement to save into
 *  \todo Does this accurately describe the parameters?  I think not!?
 *  \todo Save the track height
 */
void Track::saveSettings(QDomDocument& doc, QDomElement& element)
{
    if(!m_simpleSerializingMode)
    {
        element.setTagName("track");
    }
    element.setAttribute("type", type());
    element.setAttribute("name", name());
    // element.setAttribute("muted", isMuted());
    // element.setAttribute("solo", isSolo());
    // element.setAttribute("frozen", isFrozen());
    m_mutedModel.saveSettings(doc, element, "muted");
    m_soloModel.saveSettings(doc, element, "solo");
    m_frozenModel.saveSettings(doc, element, "frozen");

    element.setAttribute("color", color().rgb());
    element.setAttribute("usestyle", useStyleColor() ? 1 : 0);

    if(m_height >= MINIMAL_TRACK_HEIGHT && m_height != DEFAULT_TRACK_HEIGHT)
        element.setAttribute("trackheight", m_height);

    if(hasUuid())                              //! m_uuid.isEmpty())
        element.setAttribute("uuid", uuid());  // m_uuid);

    QDomElement tsDe = doc.createElement(nodeName());
    // let actual track (InstrumentTrack, bbTrack, sampleTrack etc.) save
    // its settings
    element.appendChild(tsDe);
    saveTrackSpecificSettings(doc, tsDe);

    if(m_simpleSerializingMode)
    {
        m_simpleSerializingMode = false;
        return;
    }

    // now save settings of all TCO's
    for(Tiles::const_iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
    {
        (*it)->saveState(doc, element);
    }
}

/*! \brief Load the settings from a file
 *
 *  We load the track's type and muted state and solo state, then clear out
 * our current Tile.
 *
 *  Then we step through the QDomElement's children and load the
 *  track-specific settings and Tiles states from it
 *  one at a time.
 *
 *  \param element the QDomElement to load track settings from
 *  \todo Load the track height.
 */
void Track::loadSettings(const QDomElement& element)
{
    if(element.attribute("type").toInt() != type())
    {
        qWarning(
                "Track::loadSettings Current track-type does not"
                " match track-type of settings-node!");
    }

    QString name = "";
    if(element.hasAttribute("name"))
    {
        name = element.attribute("name");
    }
    else
    {
        QDomElement e = element.firstChildElement();
        if(!e.isNull() && e.hasAttribute("name"))
            name = e.attribute("name");
    }
    setName(name);

    // setMuted(element.attribute("muted").toInt());
    // setSolo(element.attribute("solo").toInt());
    // setFrozen(element.attribute("frozen").toInt());
    m_mutedModel.loadSettings(element, "muted");
    m_soloModel.loadSettings(element, "solo");
    m_frozenModel.loadSettings(element, "frozen");

    if(element.hasAttribute("color"))
        setColor(QColor(element.attribute("color").toUInt()));

    if(element.hasAttribute("usestyle"))
        setUseStyleColor(element.attribute("usestyle").toUInt() == 1);

    if(m_simpleSerializingMode)
    {
        QDomNode node = element.firstChild();
        while(!node.isNull())
        {
            if(node.isElement() && node.nodeName() == nodeName())
            {
                loadTrackSpecificSettings(node.toElement());
                break;
            }
            node = node.nextSibling();
        }
        m_simpleSerializingMode = false;
        return;
    }

    deleteTCOs();
    /*
    while(!m_tiles.empty())
    {
        delete m_tiles.front();
        // m_tiles.erase( m_tiles.begin() );
    }
    */

    QDomNode node = element.firstChild();
    while(!node.isNull())
    {
        if(node.isElement())
        {
            if(node.nodeName() == nodeName())
            {
                loadTrackSpecificSettings(node.toElement());
            }
            else if(node.nodeName() != "muted" && node.nodeName() != "solo"
                    && node.nodeName() != "frozen"
                    && !node.toElement().attribute("metadata").toInt())
            {
                Tile* tco = createTCO(MidiTime(0));
                tco->restoreState(node.toElement());
                saveJournallingState(false);
                restoreJournallingState();
            }
        }
        node = node.nextSibling();
    }

    if(element.hasAttribute("trackheight"))
    {
        int storedHeight = element.attribute("trackheight").toInt();
        qWarning("Track: storedH=%d", storedHeight);
        if(storedHeight >= MINIMAL_TRACK_HEIGHT)
            m_height = storedHeight;
        else
            m_height = MINIMAL_TRACK_HEIGHT;
    }
    else
    {
        m_height = DEFAULT_TRACK_HEIGHT;
    }
    // setFixedHeight(m_height);

    if(element.hasAttribute("uuid"))
    {
        if(hasUuid())  //! m_uuid.isEmpty())
            qWarning("Track::loadSettings this track already has an UUID");
        // m_uuid = element.attribute("uuid");
        setUuid(element.attribute("uuid"));
        if(isFrozen())
            readFrozenBuffer();
    }

    // if(!isFrozen()) cleanFrozenBuffer();
}

/*! \brief Add another Tile into this track
 *
 *  \param tco The Tile to attach to this track.
 */
Tile* Track::addTCO(Tile* _tco)
{
    // qInfo("Track::addTCO tco=%p 'emit tileAdded'", _tco);
    if(!m_tiles.contains(_tco))
        m_tiles.append(_tco);
    else
        qInfo("Track::addTCO tco was already added");
    emit tileAdded(_tco);

    return _tco;  // just for convenience
}

/*! \brief Remove a given Tile from this track
 *
 *  \param tco The Tile to remove from this track.
 */
void Track::removeTCO(Tile* _tco)
{
    /*
      Tiles::iterator it = qFind(m_tiles.begin(),
      m_tiles.end(), tco);
      if(it != m_tiles.end())
      {
      m_tiles.erase(it);
      if(Engine::song())
      {
      Engine::song()->updateLength();
      Engine::song()->setModified();
      }
      }
    */
    if(m_tiles.removeOne(_tco))
    {
        Song* song = Engine::song();
        if(song)
        {
            Engine::song()->updateLength();
            Engine::song()->setModified();
        }

        // tmp test
        if(m_tiles.contains(_tco))
            qInfo("Track::removeTCO() tco was more than one");
    }
    else
        qInfo("Track::removeTCO() tco was not there");
}

/*! \brief Remove all TCOs from this track */
void Track::deleteTCOs()
{
    while(!m_tiles.isEmpty())
        delete m_tiles.first();
}

/*! \brief Return the number of Tiles we contain
 *
 *  \return the number of Tiles we currently contain.
 */
int Track::numOfTCOs() const
{
    // BACKTRACE
    return m_tiles.size();
}

/*! \brief Get a Tile by number
 *
 *  If the TCO number is less than our TCO array size then fetch that
 *  numbered object from the array.  Otherwise we warn the user that
 *  we've somehow requested a TCO that is too large, and create a new
 *  TCO for them.
 *  \param tcoNum The number of the Tile to fetch.
 *  \return the given Tile or a new one if out of range.
 *  \todo reject TCO numbers less than zero.
 *  \todo if we create a TCO here, should we somehow attach it to the
 *     track?
 */
Tile* Track::getTCO(int tcoNum) const
{
    if(tcoNum < m_tiles.size())
        return m_tiles[tcoNum];

    BACKTRACE
    qCritical(
            "Track::getTCO( %d ): "
            "TCO %d doesn't exist\n",
            tcoNum, tcoNum);
    return nullptr;  // createTCO( tcoNum * MidiTime::ticksPerTact() );
}

/*! \brief Determine the given Tile's number in our array.
 *
 *  \param tco The Tile to search for.
 *  \return its number in our array.
 */
int Track::getTCONum(const Tile* tco) const
{
    //	for( int i = 0; i < getTrackContentWidget()->numOfTCOs(); ++i )
    Tiles::iterator it = const_cast<Tiles::iterator>(
            qFind(m_tiles.begin(), m_tiles.end(), tco));
    if(it != m_tiles.end())
    {
        /*
          if( getTCO( i ) == _tco )
          {
          return i;
          }
        */
        return it - m_tiles.begin();
    }
    qCritical("Track::getTCONum(...): TCO not found!\n");
    return 0;
}

/*! \brief Retrieve a list of Tiles that fall within a period.
 *
 *  Here we're interested in a range of Tiles that intersect
 *  the given time period.
 *
 *  We return the TCOs we find in order by time, earliest TCOs first.
 *
 *  \param tcoV The list to contain the found Tiles.
 *  \param start The MIDI start time of the range.
 *  \param end   The MIDI endi time of the range.
 */
void Track::getTCOsInRange(Tiles&          tcoV,
                           const MidiTime& start,
                           const MidiTime& end) const
{
    for(Tile* tco: m_tiles)
    {
        int s = tco->startPosition();
        int e = tco->endPosition();
        if((s < end) && (e > start))  // >=
        {
            // TCO is within given range
            // Insert sorted by TCO's position
            tcoV.insert(std::upper_bound(tcoV.begin(), tcoV.end(), tco,
                                         Tile::lessThan),
                        tco);
        }
    }
}

/*! \brief Swap the position of two Tiles.
 *
 *  First, we arrange to swap the positions of the two TCOs in the
 *  Tiles list.  Then we swap their start times as well.
 *
 *  \param tcoNum1 The first Tile to swap.
 *  \param tcoNum2 The second Tile to swap.
 */
void Track::swapPositionOfTCOs(int tcoNum1, int tcoNum2)
{
    qSwap(m_tiles[tcoNum1], m_tiles[tcoNum2]);

    const MidiTime pos = m_tiles[tcoNum1]->startPosition();

    m_tiles[tcoNum1]->movePosition(m_tiles[tcoNum2]->startPosition());
    m_tiles[tcoNum2]->movePosition(pos);
}

void Track::createTCOsForBB(int bb)
{
    while(numOfTCOs() < bb + 1)
    {
        MidiTime position = MidiTime(numOfTCOs(), 0);
        Tile*    tco      = createTCO(position);
        tco->movePosition(position);
        tco->changeLength(MidiTime(1, 0));
    }
}

void Track::deleteUnusedTCOsForBB()
{
    while(numOfTCOs() > Engine::getBBTrackContainer()->numOfBBs())
        delete getTCO(numOfTCOs() - 1);
}

void Track::fixOverlappingTCOs()
{
    MidiTime  p(0);
    const int n = numOfTCOs();
    for(int i = 0; i < n; ++i)
    {
        Tile* tco = getTCO(i);
        if(tco->startPosition() < p)
            tco->movePosition(p);
        p = tco->endPosition();
    }
}

/*! \brief Move all the Tiles after a certain time later by one
 * bar.
 *
 *  \param pos The time at which we want to insert the bar.
 *  \todo if we stepped through this list last to first, and the list was
 *    in ascending order by TCO time, once we hit a TCO that was earlier
 *    than the insert time, we could fall out of the loop early.
 */
void Track::insertTact(const MidiTime& pos)
{
    // we'll increase the position of every TCO, positioned behind pos, by
    // one tact
    for(Tiles::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
    {
        if((*it)->startPosition() >= pos)
        {
            (*it)->movePosition((*it)->startPosition()
                                + MidiTime::ticksPerTact());
        }
    }
}

/*! \brief Move all the Tiles after a certain time earlier by
 * one bar.
 *
 *  \param pos The time at which we want to remove the bar.
 */
void Track::removeTact(const MidiTime& pos)
{
    // we'll decrease the position of every TCO, positioned behind pos, by
    // one tact
    for(Tiles::iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
    {
        if((*it)->startPosition() >= pos)
        {
            (*it)->movePosition(qMax((*it)->startPosition().getTicks()
                                             - MidiTime::ticksPerTact(),
                                     0));
        }
    }
}

/*! \brief Return the length of the entire track in bars
 *
 *  We step through our list of TCOs and determine their end position,
 *  keeping track of the latest time found in ticks.  Then we return
 *  that in bars by dividing by the number of ticks per bar.
 */
tact_t Track::length() const
{
    // find last end-position
    tick_t last = 0;
    for(Tiles::const_iterator it = m_tiles.begin(); it != m_tiles.end(); ++it)
    {
        if(Engine::song()->isExporting() && (*it)->isMuted())
        {
            continue;
        }

        const tick_t cur = (*it)->endPosition();
        if(cur > last)
        {
            last = cur;
        }
    }

    return last / MidiTime::ticksPerTact();
}

/*! \brief Invert the track's solo state.
 *
 *  We have to go through all the tracks determining if any other track
 *  is already soloed.  Then we have to save the mute state of all tracks,
 *  and set our mute state to on and all the others to off.
 */
void Track::toggleSolo()
{
    const Tracks& tl = m_trackContainer->tracks();

    bool soloBefore = false;
    for(Tracks::const_iterator it = tl.begin(); it != tl.end(); ++it)
    {
        if(*it != this)
        {
            if((*it)->m_soloModel.value())
            {
                soloBefore = true;
                break;
            }
        }
    }

    const bool solo = m_soloModel.value();
    for(Tracks::const_iterator it = tl.begin(); it != tl.end(); ++it)
    {
        if(solo)
        {
            // save mute-state in case no track was solo before
            if(!soloBefore)
            {
                (*it)->m_mutedBeforeSolo = (*it)->isMuted();
            }
            (*it)->setMuted(*it == this ? false : true);
            if(*it != this)
            {
                (*it)->m_soloModel.setValue(false);
            }
        }
        else if(!soloBefore)
        {
            (*it)->setMuted((*it)->m_mutedBeforeSolo);
        }
    }
}

void Track::toggleFrozen()
{
    // qInfo("Track::toggleFrozen");
}

void Track::cleanFrozenBuffer()
{
}

void Track::readFrozenBuffer()
{
}

void Track::writeFrozenBuffer()
{
}

void Track::clearAllTrackPlayHandles()
{
    if(QThread::currentThread() != thread())
        qWarning("Mixer::cleanHandles not in the main thread");

    if(Engine::mixer() != nullptr)
    {
        quint8 types = 0xFF;

        Engine::mixer()->emit playHandlesOfTypesToRemove(this, types);
        QCoreApplication::sendPostedEvents();
        QThread::yieldCurrentThread();
        Engine::mixer()->waitUntilNoPlayHandle(this, types);
    }
    else
        qInfo("Track::clearAllTrackPlayHandles no mixer");
}

int Track::trackIndex() const
{
    const TrackContainer* tc = trackContainer();
    if(tc == nullptr)
        return -1;
    return tc->tracks().indexOf(const_cast<Track*>(this));
}

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

    connect(m_track, SIGNAL(destroyedTrack()), this, SLOT(close()));
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
    for(Tile* tco: m_track->m_tiles)
        createTCOView(tco);

    m_trackContainerView->addTrackView(this);
}

/*! \brief Destroy this track View.
 *
 */
TrackView::~TrackView()
{
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
    if(ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt())
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
    connect(m_track, SIGNAL(destroyedTrack()), this, SLOT(close()));
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
    {
        tv->setSelected(true);
    }
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
