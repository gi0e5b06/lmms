/*
 * BBEditor.cpp - basic main-window for editing of beats and basslines
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "BBEditor.h"

#include <QAction>
//#include <QKeyEvent>
//#include <QLayout>

#include "BBTrack.h"
#include "BBTrackContainer.h"
#include "ComboBox.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "SongEditor.h"
#include "Pattern.h"
#include "Song.h"
#include "StringPairDrag.h"

#include "embed.h"

#include <QCoreApplication>

BBWindow::BBWindow(BBTrackContainer* tc) :
      EditorWindow(false), m_trackContainerView(new BBEditor(tc))
{
    setWindowIcon(embed::getIcon("bb_track_btn"));
    setWindowTitle(tr("Beat"));
    setCentralWidget(m_trackContainerView);

    /*
      setAcceptDrops(true);
      m_toolBar->setAcceptDrops(true);
      connect(m_toolBar, SIGNAL(dragEntered(QDragEnterEvent*)),
      m_trackContainerView, SLOT(dragEnterEvent(QDragEnterEvent*)));
      connect(m_toolBar, SIGNAL(dropped(QDropEvent*)),
      m_trackContainerView, SLOT(dropEvent(QDropEvent*)));
    */

    // TODO: Use style sheet
    if(ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt())
    {
        setMinimumWidth(TRACK_OP_WIDTH_COMPACT
                        + DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT
                        + 2 * TCO_BORDER_WIDTH + 384);
    }
    else
    {
        setMinimumWidth(TRACK_OP_WIDTH + DEFAULT_SETTINGS_WIDGET_WIDTH
                        + 2 * TCO_BORDER_WIDTH + 384);
    }

    m_playAction->setToolTip(tr("Play/pause current beat/bassline (Space)"));
    m_stopAction->setToolTip(
            tr("Stop playback of current beat/bassline (Space)"));

    m_playAction->setWhatsThis(
            tr("Click here to play the current "
               "beat/bassline.  The beat/bassline is automatically "
               "looped when its end is reached."));
    m_stopAction->setWhatsThis(
            tr("Click here to stop playing of current "
               "beat/bassline."));

    // Beat selector
    DropToolBar* beatSelectionToolBar
            = addDropToolBarToTop(tr("Beat selector"));

    m_bbComboBox = new ComboBox(m_toolBar, "[beat selection]");
    m_bbComboBox->setFixedSize(140, 32);
    m_bbComboBox->setModel(&tc->m_bbComboBoxModel);

    beatSelectionToolBar->addSeparator();
    beatSelectionToolBar->addWidget(m_bbComboBox);
    beatSelectionToolBar->addSeparator();

    // Track actions
    DropToolBar* trackActionsToolBar
            = addDropToolBarToTop(tr("Track actions"));

    trackActionsToolBar->addAction(embed::getIcon("clone"),
                                   tr("Clone the beat"), m_trackContainerView,
                                   SLOT(cloneBeat()));

    trackActionsToolBar->addAction(embed::getIcon("add_instrument_track"),
                                   tr("Add instrument-track"),
                                   m_trackContainerView,
                                   SLOT(addInstrumentTrack()));

    /*
    trackActionsToolBar->addAction(embed::getIcon("add_bb_track"),
                                   tr("Add beat/bassline"),
                                   Engine::getSong(),
                                   SLOT(addBBTrack()));
    */

    trackActionsToolBar->addAction(
            embed::getIcon("add_sample_track"), tr("Add sample-track"),
            m_trackContainerView, SLOT(addSampleTrack()));

    trackActionsToolBar->addAction(embed::getIcon("add_automation_track"),
                                   tr("Add automation-track"),
                                   m_trackContainerView,
                                   SLOT(addAutomationTrack()));

    /*
    QWidget* stretch = new QWidget(m_toolBar);
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    trackActionsToolBar->addWidget(stretch);
    */

    // Step actions
    DropToolBar* stepActionsToolBar = addDropToolBarToTop(tr("Step actions"));

    stepActionsToolBar->addAction(embed::getIcon("step_btn_remove"),
                                  tr("Remove steps"), m_trackContainerView,
                                  SLOT(removeSteps()));
    stepActionsToolBar->addAction(embed::getIcon("step_btn_add"),
                                  tr("Add steps"), m_trackContainerView,
                                  SLOT(addSteps()));
    stepActionsToolBar->addAction(embed::getIcon("step_btn_duplicate"),
                                  tr("Clone Steps"), m_trackContainerView,
                                  SLOT(cloneSteps()));

    stepActionsToolBar->addAction(
            embed::getIcon("arrow_left"), tr("Rotate one step left"),
            m_trackContainerView, SLOT(rotateOneStepLeft()));
    stepActionsToolBar->addAction(
            embed::getIcon("arrow_right"), tr("Rotate one step right"),
            m_trackContainerView, SLOT(rotateOneStepRight()));

    connect(&tc->m_bbComboBoxModel, SIGNAL(dataChanged()),
            m_trackContainerView, SLOT(updatePosition()));

    QAction* viewNext = new QAction(this);
    connect(viewNext, SIGNAL(triggered()), m_bbComboBox, SLOT(selectNext()));
    viewNext->setShortcut(Qt::Key_Plus);
    addAction(viewNext);

    QAction* viewPrevious = new QAction(this);
    connect(viewPrevious, SIGNAL(triggered()), m_bbComboBox,
            SLOT(selectPrevious()));
    viewPrevious->setShortcut(Qt::Key_Minus);
    addAction(viewPrevious);
}

BBWindow::~BBWindow()
{
}

QSize BBWindow::sizeHint() const
{
    return {minimumWidth() + 10, 300};
}

void BBWindow::removeBBView(int bb)
{
    m_trackContainerView->removeBBView(bb);
}

void BBWindow::play()
{
    if(Engine::getSong()->playMode() != Song::Mode_PlayBB)
        Engine::getSong()->playBB();
    else
        Engine::getSong()->togglePause();
}

void BBWindow::stop()
{
    Engine::getSong()->stop();
}

BBEditor::BBEditor(BBTrackContainer* tc) : TrackContainerView(tc), m_bbtc(tc)
{
    setModel(tc);
}

float BBEditor::pixelsPerTact() const
{
    return m_ppt > 0.f ? m_ppt : 256.f;
}

void BBEditor::addSteps()
{
    makeSteps(false);
}

void BBEditor::cloneSteps()
{
    makeSteps(true);
}

void BBEditor::removeSteps()
{
    // Tracks tl = model()->tracks();
    // for(Tracks::iterator it = tl.begin(); it != tl.end(); ++it)
    for(Track* t: model()->tracks())
    {
        if(t->type() == Track::InstrumentTrack)
        {
            Pattern* p
                    = static_cast<Pattern*>(t->getTCO(m_bbtc->currentBB()));
            p->removeBarSteps();
        }
    }
}

void BBEditor::cloneBeat()
{
    TrackContainerView* tcView    = gui->songWindow()->m_editor;
    const int           cur_bb    = m_bbtc->currentBB();
    BBTrack*            bbt       = BBTrack::findBBTrack(cur_bb);
    TrackView*          trackView = tcView->createTrackView(bbt);

    Track*     newTrack     = bbt->clone();
    TrackView* newTrackView = tcView->createTrackView(newTrack);
    int        index        = tcView->trackViews().indexOf(trackView);
    int        i            = tcView->trackViews().size();
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

    m_bbtc->setCurrentBB(static_cast<BBTrack*>(newTrack)->index());
}

void BBEditor::addInstrumentTrack()
{
    Track::create(Track::InstrumentTrack, model());
}

void BBEditor::addSampleTrack()
{
    Track::create(Track::SampleTrack, model());
}

void BBEditor::addAutomationTrack()
{
    Track::create(Track::AutomationTrack, model());
}

int BBEditor::highestStepResolution()
{
    int r = 0;
    for(const TrackView* view: trackViews())
    {
        const Track* t = view->track();
        Tiles        v = t->getTCOs();
        for(const Tile* tco: v)
            r = qMax<int>(r, tco->stepResolution());
        qInfo("BBEditor::highestStepResolution %s %d",
              qPrintable(view->track()->name()), r);
    }
    return r;
}

void BBEditor::rotateOneStepLeft()
{
    int r = highestStepResolution();
    if(r > 0)
        for(TrackView* view: trackViews())
        {
            const Track* t = view->track();
            Tiles        v = t->getTCOs();
            for(Tile* tco: v)
                tco->rotate(-MidiTime::ticksPerTact() / r);
        }
}

void BBEditor::rotateOneStepRight()
{
    int r = highestStepResolution();
    if(r > 0)
        for(TrackView* view: trackViews())
        {
            const Track* t = view->track();
            Tiles        v = t->getTCOs();
            for(Tile* tco: v)
                tco->rotate(MidiTime::ticksPerTact() / r);
        }
}

void BBEditor::removeBBView(int bb)
{
    for(TrackView* view: trackViews())
        view->getTrackContentWidget()->removeTCOView(bb);
}

void BBEditor::saveSettings(QDomDocument& doc, QDomElement& element)
{
    MainWindow::saveWidgetState(parentWidget(), element);
}

void BBEditor::loadSettings(const QDomElement& element)
{
    MainWindow::restoreWidgetState(parentWidget(), element);
}

void BBEditor::dropEvent(QDropEvent* de)
{
    QString type  = StringPairDrag::decodeKey(de);
    QString value = StringPairDrag::decodeValue(de);

    if(type.left(6) == "track_")
    {
        DataFile dataFile(value.toUtf8());
        Track* t = Track::create(dataFile.content().firstChild().toElement(),
                                 model());
        m_bbtc->updateAfterTrackAdd();
        t->createTCOsForBB(Engine::getBBTrackContainer()->numOfBBs() - 1);
        t->deleteUnusedTCOsForBB();
        m_bbtc->fixIncorrectPositions();
        de->accept();
    }
    else
    {
        TrackContainerView::dropEvent(de);
    }
}

void BBEditor::updatePosition()
{
    // realignTracks();
    emit positionChanged(m_currentPosition);
}

void BBEditor::makeSteps(bool clone)
{
    // Tracks tl = model()->tracks();
    // for(Tracks::iterator it = tl.begin(); it != tl.end(); ++it)
    for(Track* t: model()->tracks())
    {
        if(t->type() == Track::InstrumentTrack)
        {
            Pattern* p
                    = static_cast<Pattern*>(t->getTCO(m_bbtc->currentBB()));
            if(clone)
                p->cloneSteps();
            else
                p->addBarSteps();
        }
    }
}
