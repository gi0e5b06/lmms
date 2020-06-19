/*
 * SongEditor.cpp - basic window for song-editing
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
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

#include "SongEditor.h"

#include "AudioDevice.h"
#include "AutomatableSlider.h"
#include "AutomatableToolButton.h"
#include "AutomationPattern.h"
#include "BBTrack.h"
#include "CPULoadWidget.h"
#include "Clipboard.h"
#include "ComboBox.h"
#include "ConfigManager.h"
#include "GuiApplication.h"
#include "Knob.h"
#include "LcdSpinBox.h"
#include "MainWindow.h"
#include "MeterDialog.h"
#include "Mixer.h"
#include "Pattern.h"
#include "PianoRoll.h"
#include "SampleTrack.h"
#include "TextFloat.h"
#include "TimeDisplayWidget.h"
#include "TimeLineWidget.h"
#include "ToolTip.h"
#include "VisualizationWidget.h"
#include "embed.h"

#include <QAction>
#include <QKeyEvent>
#include <QLabel>
#include <QTimeLine>
//#include <QHBoxLayout>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QPainter>
#include <QShortcut>
#include <QVBoxLayout>

/*
positionLine::positionLine( QWidget * parent ) :
        QWidget( parent )
{
        setFixedWidth( 1 );
        setAttribute( Qt::WA_NoSystemBackground, true );
}




void positionLine::paintEvent( QPaintEvent * pe )
{
        QPainter p( this );
        p.fillRect( rect(), QColor( 255, 255, 255, 153 ) );
}
*/

SongEditor::SongEditor(Song* song) :
      TrackContainerView(song), m_song(song),
      m_zoomingXModel(new ComboBoxModel(nullptr, tr("Zoom X"))),
      m_zoomingYModel(new ComboBoxModel(nullptr, tr("Zoom Y"))),
      m_quantizeModel(new ComboBoxModel(nullptr, tr("Quantize"))),
      m_scrollBack(false),
      m_smoothScroll(
              ConfigManager::inst()->value("ui", "smoothscroll").toInt()),
      m_mode(DrawMode)
{
    m_zoomingXModel->setParent(this);
    m_zoomingYModel->setParent(this);
    m_quantizeModel->setParent(this);

    // create time-line
    int widgetTotal
            = ConfigManager::inst()->value("ui", "compacttrackbuttons")
                                      .toInt()
                              == 1
                      ? DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT
                                + TRACK_OP_WIDTH_COMPACT
                      : DEFAULT_SETTINGS_WIDGET_WIDTH + TRACK_OP_WIDTH;
    m_timeLine = new TimeLineWidget(widgetTotal, 32, pixelsPerTact(),
                                    m_song->m_playPos[Song::Mode_PlaySong],
                                    m_currentPosition, this);
    connect(this, SIGNAL(positionChanged(const MidiTime&)),
            m_song->m_playPos[Song::Mode_PlaySong].m_timeLine,
            SLOT(updatePosition(const MidiTime&)));
    connect(m_timeLine, SIGNAL(positionChanged(const MidiTime&)), this,
            SLOT(updatePosition(const MidiTime&)));
    connect(m_timeLine, SIGNAL(regionSelectedFromPixels(int, int)), this,
            SLOT(selectRegionFromPixels(int, int)));
    connect(m_timeLine, SIGNAL(selectionFinished()), this,
            SLOT(stopRubberBand()));

    // GDX m_positionLine = new positionLine( this );
    static_cast<QVBoxLayout*>(layout())->insertWidget(1, m_timeLine);

    // start of global toolbar
    // this part should be moved out
    // add some essential widgets to global tool-bar
    QWidget* tb = gui->mainWindow()->toolBar();

    gui->mainWindow()->addSpacingToToolBar(12);

    m_tempoSpinBox = new LcdSpinBox(3, tb, tr("Tempo"));
    m_tempoSpinBox->setModel(&m_song->m_tempoModel);
    m_tempoSpinBox->setLabel(tr("TEMPO/BPM"));
    ToolTip::add(m_tempoSpinBox, tr("tempo of song"));

    m_tempoSpinBox->setWhatsThis(
            tr("The tempo of a song is specified in beats per minute "
               "(BPM). If you want to change the tempo of your "
               "song, change this value. Every measure has four beats, "
               "so the tempo in BPM specifies, how many measures / 4 "
               "should be played within a minute (or how many measures "
               "should be played within four minutes)."));

    int col = gui->mainWindow()->addWidgetToToolBar(m_tempoSpinBox, 0);

    /*
    toolButton * hq_btn = new toolButton( embed::getIconPixmap( "hq_mode" ),
                                            tr( "High quality mode" ),
                                            nullptr, nullptr, tb );
    hq_btn->setCheckable( true );
    connect( hq_btn, SIGNAL( toggled( bool ) ),
                    this, SLOT( setHighQuality( bool ) ) );
    hq_btn->setFixedWidth( 42 );
    gui->mainWindow()->addWidgetToToolBar( hq_btn, 1, col );
    */

    m_timeSigDisplay = new MeterDialog(this, true);
    m_timeSigDisplay->setModel(&m_song->m_timeSigModel);

    gui->mainWindow()->addWidgetToToolBar(m_timeSigDisplay, 1, col);
    gui->mainWindow()->addSpacingToToolBar(12, col + 1);
    col += 2;

    // Master volume

    QLabel* masterVolumeLBL = new QLabel(tb);
    masterVolumeLBL->setPixmap(embed::getPixmap("master_volume"));
    masterVolumeLBL->setFixedWidth(26);

    m_masterVolumeKNB = new Knob(tb, tr("Master volume"));
    m_masterVolumeKNB->setVolumeKnob(true);
    m_masterVolumeKNB->setModel(&m_song->m_masterVolumeModel);
    m_masterVolumeKNB->setHintText(tr("Master volume"), "%");
    ToolTip::add(m_masterVolumeKNB, tr("Master volume"));

    m_masterVolumeTFT = new TextFloat();
    m_masterVolumeTFT->setTitle(tr("Master volume"));
    m_masterVolumeTFT->setPixmap(embed::getPixmap("master_volume"));

    gui->mainWindow()->addWidgetToToolBar(masterVolumeLBL, 0, col);
    gui->mainWindow()->addWidgetToToolBar(m_masterVolumeKNB, 1, col);
    gui->mainWindow()->addSpacingToToolBar(6, col + 1);
    col += 2;

    // Master Panning

    QLabel* masterPanningLBL = new QLabel(tb);
    masterPanningLBL->setPixmap(embed::getPixmap("master_panning"));
    masterPanningLBL->setFixedWidth(26);

    m_masterPanningKNB = new Knob(tb, tr("Master panning"));
    m_masterPanningKNB->setPointColor(Qt::magenta);
    m_masterPanningKNB->setModel(&m_song->m_masterPanningModel);
    m_masterPanningKNB->setHintText(tr("Master panning"), "%");
    ToolTip::add(m_masterPanningKNB, tr("Master panning"));

    m_masterPanningTFT = new TextFloat();
    m_masterPanningTFT->setTitle(tr("Master pan"));
    m_masterPanningTFT->setPixmap(embed::getPixmap("master_panning"));

    gui->mainWindow()->addWidgetToToolBar(masterPanningLBL, 0, col);
    gui->mainWindow()->addWidgetToToolBar(m_masterPanningKNB, 1, col);
    gui->mainWindow()->addSpacingToToolBar(12, col + 1);
    col += 2;

    // Master pitch

    QLabel* masterPitchLBL = new QLabel(tb);
    masterPitchLBL->setPixmap(embed::getPixmap("master_pitch"));
    masterPitchLBL->setFixedWidth(26);

    m_masterPitchKNB = new Knob(tb, tr("Master pitch"));
    m_masterPitchKNB->setPointColor(Qt::cyan);
    m_masterPitchKNB->setModel(&m_song->m_masterPitchModel);
    m_masterPitchKNB->setHintText(tr("Master pitch"), " " + tr("semitones"));
    ToolTip::add(m_masterPitchKNB, tr("Master pitch"));

    m_masterPitchTFT = new TextFloat();
    m_masterPitchTFT->setTitle(tr("Master pitch"));
    m_masterPitchTFT->setPixmap(embed::getPixmap("master_pitch"));

    gui->mainWindow()->addWidgetToToolBar(masterPitchLBL, 0, col);
    gui->mainWindow()->addWidgetToToolBar(m_masterPitchKNB, 1, col);
    gui->mainWindow()->addSpacingToToolBar(6, col + 1);
    col += 2;

    // Oscilloscope

    VisualizationWidget* vw
            = new VisualizationWidget(embed::getPixmap("output_bigger_graph"),
                                      tb, Engine::mixer()->displayRing());
    gui->mainWindow()->addWidgetToToolBar(vw, -1, col);
    gui->mainWindow()->addSpacingToToolBar(12, col + 1);
    col += 2;

    // Time + CPU

    CPULoadWidget* clw = new CPULoadWidget(tb, true);

    gui->mainWindow()->addWidgetToToolBar(new TimeDisplayWidget, 0, col);
    gui->mainWindow()->addWidgetToToolBar(clw, 1, col);
    gui->mainWindow()->addSpacingToToolBar(12, col + 1);
    col += 2;

    // navigator
    if(false)
    {

        const char* NAV_ICONS_1[6]
                = {"play", "pause", "forward", "", "stop", "rewind"};

        for(int i = 0; i < 6; i++)
        {
            if(QString("") != NAV_ICONS_1[i])
            {
                AutomatableToolButton* b = new AutomatableToolButton(tb);
                QAction* a = new QAction(embed::getIcon(NAV_ICONS_1[i]),
                                         NAV_ICONS_1[i], b);
                b->setDefaultAction(a);
                a->setData(QVariant(i));
                // a->setCheckable(false);
                // a->setShortcut((char)(65+i));
                gui->mainWindow()->addWidgetToToolBar(b, i / 3, col + i % 3);
            }
        }

        gui->mainWindow()->addSpacingToToolBar(12, col + 3);
        col += 4;

        const char* NAV_ICONS_2[2] = {"record", "record_accompany"};

        for(int i = 0; i < 2; i++)
        {
            AutomatableToolButton* b = new AutomatableToolButton(tb);
            QAction* a = new QAction(embed::getIcon(NAV_ICONS_2[i]),
                                     NAV_ICONS_2[i], b);
            b->setDefaultAction(a);
            a->setData(QVariant(i));
            // a->setCheckable(false);
            // a->setShortcut((char)(65+i));
            gui->mainWindow()->addWidgetToToolBar(b, i / 1, col + i % 1);
        }

        gui->mainWindow()->addSpacingToToolBar(12, col + 1);
        col += 2;

        const char* NAV_ICONS_3[12]
                = {"playpos_songstart",     "playpos_laststart",
                   "playpos_hyperbarstart", "playpos_loopstart",
                   "playpos_barstart",      "playpos_beatstart",

                   "playpos_songend",       "playpos_lastend",
                   "playpos_hyperbarend",   "playpos_loopend",
                   "playpos_barend",        "playpos_beatend"};

        for(int i = 0; i < 12; i++)
        {
            AutomatableToolButton* b = new AutomatableToolButton(tb);
            QAction* a = new QAction(embed::getIcon(NAV_ICONS_3[i]),
                                     NAV_ICONS_3[i], b);
            b->setDefaultAction(a);
            a->setData(QVariant(i));
            // a->setCheckable(false);
            // a->setShortcut((char)(65+i));
            gui->mainWindow()->addWidgetToToolBar(b, i / 6, col + i % 6);
        }

        col += 6;
    }

    // end of global toolbar

    static_cast<QVBoxLayout*>(layout())->insertWidget(0, m_timeLine);

    m_leftRightScroll = new QScrollBar(Qt::Horizontal, this);
    m_leftRightScroll->setMinimum(0);
    m_leftRightScroll->setMaximum(0);
    m_leftRightScroll->setSingleStep(1);
    m_leftRightScroll->setPageStep(20);
    static_cast<QVBoxLayout*>(layout())->addWidget(m_leftRightScroll);
    connect(m_leftRightScroll, SIGNAL(valueChanged(int)), this,
            SLOT(scrolled(int)));
    connect(m_song, SIGNAL(lengthChanged(int)), this,
            SLOT(updateScrollBar(int)));

    // Set up zooming x model
    EditorWindow::fillZoomLevels(*m_zoomingXModel);
    /*
    for(const float& zoomLevel : EditorWindow::ZOOM_LEVELS)
        m_zoomingXModel->addItem(QString("%1%").arg(zoomLevel * 100));
    */
    m_zoomingXModel->setInitValue(m_zoomingXModel->findText("100%"));
    connect(m_zoomingXModel, SIGNAL(dataChanged()), this,
            SLOT(zoomingXChanged()));

    // Set up zooming y model
    EditorWindow::fillZoomLevels(*m_zoomingYModel);
    /*
    for(const float& zoomLevel : EditorWindow::ZOOM_LEVELS)
        m_zoomingYModel->addItem(QString("%1%").arg(zoomLevel * 100));
    */
    m_zoomingYModel->setInitValue(m_zoomingYModel->findText("100%"));
    connect(m_zoomingYModel, SIGNAL(dataChanged()), this,
            SLOT(zoomingYChanged()));

    // Quantize
    EditorWindow::fillQuantizeLevels(*m_quantizeModel);
    m_quantizeModel->setInitValue(m_quantizeModel->findText("1/1"));

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

SongEditor::~SongEditor()
{
}

void SongEditor::saveSettings(QDomDocument& doc, QDomElement& element)
{
    MainWindow::saveWidgetState(parentWidget(), element);
}

void SongEditor::loadSettings(const QDomElement& element)
{
    MainWindow::restoreWidgetState(parentWidget(), element);
}

void SongEditor::setHighQuality(bool hq)
{
    Engine::mixer()->changeQuality(Mixer::qualitySettings(
            hq ? Mixer::qualitySettings::Mode_HighQuality
               : Mixer::qualitySettings::Mode_Draft));
}

void SongEditor::scrolled(int new_pos)
{
    m_currentPosition = MidiTime(new_pos, 0);
    update();
    emit positionChanged(m_currentPosition);
}

void SongEditor::setEditMode(EditMode mode)
{
    m_mode = mode;
}

void SongEditor::setEditModeDraw()
{
    setEditMode(DrawMode);
}

void SongEditor::setEditModeSelect()
{
    setEditMode(SelectMode);
}

void SongEditor::keyPressEvent(QKeyEvent* ke)
{
    // qInfo("SongEditor::keyPressEvent key=%d",ke->key());

    if(ke->modifiers() & Qt::ShiftModifier && ke->key() == Qt::Key_Insert)
    {
        m_song->insertBar();
    }
    else if(ke->modifiers() & Qt::ShiftModifier
            && ke->key() == Qt::Key_Delete)
    {
        m_song->removeBar();
    }
    else if(ke->key() == Qt::Key_Left)
    {
        tick_t t = m_song->currentTick() - MidiTime::ticksPerTact();
        if(t >= 0)
        {
            m_song->setPlayPos(t, Song::Mode_PlaySong);
        }
    }
    else if(ke->key() == Qt::Key_Right)
    {
        tick_t t = m_song->currentTick() + MidiTime::ticksPerTact();
        if(t < MaxSongLength)
        {
            m_song->setPlayPos(t, Song::Mode_PlaySong);
        }
    }
    else if(ke->key() == Qt::Key_Home)
    {
        m_song->setPlayPos(0, Song::Mode_PlaySong);
    }
    else if(ke->key() == Qt::Key_Delete)
    {
        deleteSelection();
        /*
        QVector<TrackContentObjectView *> tcoViews;
        QVector<SelectableObject *> so = selectedObjects();
        for( QVector<SelectableObject *>::iterator it = so.begin();
                        it != so.end(); ++it )
        {
                TrackContentObjectView * tcov =
                        dynamic_cast<TrackContentObjectView *>( *it );
                tcov->remove();
        }
        */
    }
    else if(ke->modifiers() & Qt::ControlModifier && ke->key() == Qt::Key_A)
    {
        selectAllTcos(!(ke->modifiers() & Qt::ShiftModifier));
    }
    else if(ke->key() == Qt::Key_Escape)
    {
        selectAllTcos(false);
    }
    else if(ke->modifiers() & Qt::ControlModifier && ke->key() == Qt::Key_U)
    {
        unitePatterns();
    }
    else if(ke->modifiers() & Qt::ControlModifier && ke->key() == Qt::Key_D)
    {
        dividePatterns();
    }
    else
    {
        // qInfo("SongEditor: shift=%d key=%d",
        //  bool(ke->modifiers() & Qt::ShiftModifier), ke->key());
        QWidget::keyPressEvent(ke);
    }

    requireActionUpdate();
}

tick_t SongEditor::quantization() const
{
    int v = m_quantizeModel->value();
    return EditorWindow::QUANTIZE_LEVELS[v];
}

void SongEditor::unitePatterns()
{
    // Unite multiple selected patterns horizontally
    QVector<SelectableObject*> so = selectedObjects();
    qSort(so.begin(), so.end(), SelectableObject::lessThan);
    for(Track* t: model()->tracks())
    {
        if(t->type() == Track::InstrumentTrack)
        {
            // qInfo("Unite: instrument track %p",t);
            Pattern* newp = nullptr;
            for(QVector<SelectableObject*>::iterator it = so.begin();
                it != so.end(); ++it)
            {
                TrackContentObjectView* tcov
                        = dynamic_cast<TrackContentObjectView*>(*it);
                // tcov->remove();
                if(!tcov)
                {
                    qCritical("Unite: tcov null");
                    continue;
                }
                TrackContentObject* tco = tcov->getTrackContentObject();
                if(!tco)
                {
                    qCritical("Unite: tco null");
                    continue;
                }
                if(tco->track() != t)
                    continue;
                Pattern* p = dynamic_cast<Pattern*>(tco);
                if(!p)
                {
                    qCritical("Unite: p null");
                    continue;
                }
                if(newp == nullptr)
                {
                    t->addJournalCheckPoint();
                    newp = new Pattern(*p);
                    newp->setJournalling(false);
                    // newp->setAutoResize(true);
                    newp->movePosition(p->startPosition());
                    newp->changeLength(p->length());
                }
                else
                {
                    for(Note* n: p->notes())
                    {
                        Note* newn = new Note(*n);
                        newn->setPos(newn->pos() + p->startPosition()
                                     - newp->startPosition());
                        newp->addNote(*newn, false);
                    }
                    newp->changeLength(qMax<int>(
                            newp->length(), p->startPosition()
                                                    - newp->startPosition()
                                                    + p->length()));
                }
                tcov->remove();
            }
            if(newp)
            {
                newp->rearrangeAllNotes();
                newp->setJournalling(true);
            }
            // qInfo("  end of instrument track");
        }
        else if(t->type() == Track::AutomationTrack)
        {
            // qInfo("Unite: automation track %p",t);
            AutomationPattern* newp = nullptr;
            MidiTime           endPos;
            for(QVector<SelectableObject*>::iterator it = so.begin();
                it != so.end(); ++it)
            {
                TrackContentObjectView* tcov
                        = dynamic_cast<TrackContentObjectView*>(*it);
                // tcov->remove();
                if(!tcov)
                {
                    qCritical("Unite: tcov null");
                    continue;
                }
                TrackContentObject* tco = tcov->getTrackContentObject();
                if(!tco)
                {
                    qCritical("Unite: tco null");
                    continue;
                }
                if(tco->track() != t)
                    continue;
                AutomationPattern* p = dynamic_cast<AutomationPattern*>(tco);
                if(!p)
                {
                    qCritical("Unite: p null");
                    continue;
                }
                if(newp == nullptr)
                {
                    t->addJournalCheckPoint();
                    newp = new AutomationPattern(*p);
                    newp->setJournalling(false);
                    // newp->cleanObjects();
                    newp->movePosition(p->startPosition());
                    endPos = p->endPosition();
                }
                else
                {
                    AutomationPattern::timeMap& map = p->getTimeMap();
                    for(int t: map.keys())
                    {
                        float v    = map.value(t);
                        int   newt = t + p->startPosition()
                                   - newp->startPosition();
                        newp->getTimeMap().insert(newt, v);
                    }
                    for(QPointer<AutomatableModel> o: p->objects())
                        newp->addObject(o);
                    endPos = qMax(endPos, p->endPosition());
                }
                tcov->remove();
            }
            if(newp)
            {
                newp->changeLength(endPos - newp->startPosition());
                newp->setJournalling(true);
                newp->emit dataChanged();
            }
            // qInfo("  end of automation track");
        }
        else if(t->type() == Track::BBTrack)
        {
            // qInfo("Unite: bb track %p",t);
            BBTCO*   newp = nullptr;
            MidiTime endPos;
            for(QVector<SelectableObject*>::iterator it = so.begin();
                it != so.end(); ++it)
            {
                TrackContentObjectView* tcov
                        = dynamic_cast<TrackContentObjectView*>(*it);
                // tcov->remove();
                if(!tcov)
                {
                    qCritical("Unite: tcov null");
                    continue;
                }
                TrackContentObject* tco = tcov->getTrackContentObject();
                if(!tco)
                {
                    qCritical("Unite: tco null");
                    continue;
                }
                if(tco->track() != t)
                    continue;
                BBTCO* p = dynamic_cast<BBTCO*>(tco);
                if(!p)
                {
                    qCritical("Unite: p null");
                    continue;
                }
                if(newp == nullptr)
                {
                    t->addJournalCheckPoint();
                    newp = new BBTCO(*p);
                    newp->setJournalling(false);
                    // newp->cleanObjects();
                    newp->movePosition(p->startPosition());
                    endPos = p->endPosition();
                }
                else
                {
                    endPos = qMax(endPos, p->endPosition());
                }
                tcov->remove();
            }
            if(newp)
            {
                newp->changeLength(endPos - newp->startPosition());
                newp->setJournalling(true);
                newp->emit dataChanged();
            }
            // qInfo("  end of bb track");
        }
        else if(t->type() == Track::SampleTrack)
        {
            // Currently not imlementable without creating a new sample
        }
    }
}

void SongEditor::dividePatterns()
{
    // qInfo("SongEditor::dividePatterns");
    // Split multiple selected patterns at the current position
    QVector<SelectableObject*> so = selectedObjects();
    qSort(so.begin(), so.end(), SelectableObject::lessThan);
    Song* song = Engine::getSong();
    if(song->isPlaying())
        return;

    int  splitPos = song->getPlayPos(Song::Mode_PlaySong);
    bool first    = true;
    for(Track* t: model()->tracks())
    {
        if(t->type() == Track::InstrumentTrack)
        {
            // qInfo("Divide: instrument track %p",t);
            for(QVector<SelectableObject*>::iterator it = so.begin();
                it != so.end(); ++it)
            {
                TrackContentObjectView* tcov
                        = dynamic_cast<TrackContentObjectView*>(*it);
                if(!tcov)
                {
                    qCritical("Divide: tcov null");
                    continue;
                }
                TrackContentObject* tco = tcov->getTrackContentObject();
                if(!tco)
                {
                    qCritical("Divide: tco null");
                    continue;
                }
                if(tco->track() != t)
                    continue;
                Pattern* p = dynamic_cast<Pattern*>(tco);
                if(!p)
                {
                    qCritical("Divide: p null");
                    continue;
                }

                // qInfo("pos: s=%d e=%d r=%d",
                //  (int)p->startPosition(),(int)p->endPosition(),splitPos);
                if(p->startPosition() >= splitPos)
                    continue;
                if(p->endPosition() <= splitPos)
                    continue;

                if(first)
                {
                    t->addJournalCheckPoint();
                    first = false;
                }

                Pattern* newp1 = new Pattern(*p);  // 1 left, before
                newp1->setJournalling(false);
                newp1->movePosition(p->startPosition());
                for(Note* n: newp1->notes())
                {
                    if(newp1->startPosition() + n->pos() < splitPos)
                        continue;
                    // qInfo("p1 pos: s=%d n=%d r=%d remove note",
                    //  (int)newp1->startPosition(),(int)n->pos(),splitPos);
                    newp1->removeNote(n);
                }
                newp1->changeLength(splitPos - p->startPosition());

                Pattern* newp2 = new Pattern(*p);  // 2 right, after
                newp2->setJournalling(false);
                newp2->movePosition(p->startPosition());

                if(!newp2->isEmpty())
                {
                    if(newp2->autoRepeat())
                        newp2->rotate(-(splitPos - p->startPosition())
                                      % newp2->unitLength());

                    Notes todelete;
                    for(Note* n: newp2->notes())
                    {
                        if(newp2->autoRepeat()
                           || (newp2->startPosition() + n->pos() >= splitPos))
                        {
                            // qInfo("p2 pos: s=%d n=%d r=%d k=%d move note",
                            //  (int)newp2->startPosition(),(int)n->pos(),splitPos,n->key());
                            n->setPos(n->pos() + newp2->startPosition()
                                      - splitPos);
                            continue;
                        }
                        // qInfo("p2 pos: s=%d n=%d r=%d k=%d remove note",
                        //  (int)newp2->startPosition(),(int)n->pos(),splitPos,n->key());
                        todelete << n;
                    }
                    for(Note* n: todelete)
                        newp2->removeNote(n);
                }
                newp2->movePosition(splitPos);
                newp2->changeLength(p->endPosition() - splitPos);

                tcov->remove();

                newp1->rearrangeAllNotes();
                newp2->rearrangeAllNotes();
                newp1->setJournalling(true);
                newp2->setJournalling(true);
                newp1->emit dataChanged();
                newp2->emit dataChanged();
                // qInfo("  end of instrument track");
            }
        }
        else if(t->type() == Track::AutomationTrack)
        {
            // qInfo("Divide: automation track %p",t);
            for(QVector<SelectableObject*>::iterator it = so.begin();
                it != so.end(); ++it)
            {
                TrackContentObjectView* tcov
                        = dynamic_cast<TrackContentObjectView*>(*it);
                if(!tcov)
                {
                    qCritical("Unite: tcov null");
                    continue;
                }
                TrackContentObject* tco = tcov->getTrackContentObject();
                if(!tco)
                {
                    qCritical("Unite: tco null");
                    continue;
                }
                if(tco->track() != t)
                    continue;
                AutomationPattern* p = dynamic_cast<AutomationPattern*>(tco);
                if(!p)
                {
                    qCritical("Divide: p null");
                    continue;
                }

                // qInfo("pos: s=%d e=%d r=%d",
                //  (int)p->startPosition(),(int)p->endPosition(),splitPos);
                if(p->startPosition() >= splitPos)
                    continue;
                if(p->endPosition() <= splitPos)
                    continue;

                if(first)
                {
                    t->addJournalCheckPoint();
                    first = false;
                }

                AutomationPattern* newp1
                        = new AutomationPattern(*p);  // 1 left, before
                newp1->setJournalling(false);
                newp1->movePosition(p->startPosition());
                AutomationPattern::timeMap& map1 = newp1->getTimeMap();
                for(int t: map1.keys())
                {
                    // qInfo(" val: t=%d",t);
                    if(t >= splitPos - p->startPosition())
                        map1.remove(t);
                }
                if(!map1.contains(splitPos - p->startPosition()))
                    map1.insert(splitPos - p->startPosition(),
                                p->valueAt(splitPos));

                // for(QPointer<AutomatableModel> o: p->objects())
                //              newp->addObject(o);

                newp1->changeLength(splitPos - p->startPosition());

                AutomationPattern* newp2
                        = new AutomationPattern(*p);  // 2 right, after
                newp2->setJournalling(false);
                newp2->movePosition(p->startPosition());
                AutomationPattern::timeMap& map2 = newp2->getTimeMap();
                if(!map2.contains(0))
                    map2.insert(0, p->valueAt(splitPos));
                for(int t: map2.keys())
                {
                    float v = map2.value(t);
                    newp2->removeValue(t);
                    if(t >= splitPos - p->startPosition())
                        map2.insert(t - splitPos + p->startPosition(), v);
                }

                // for(QPointer<AutomatableModel> o: p->objects())
                //              newp->addObject(o);

                newp2->changeLength(p->endPosition() - splitPos);
                newp2->movePosition(splitPos);

                tcov->remove();

                newp1->setJournalling(true);
                newp2->setJournalling(true);
                newp1->emit dataChanged();
                newp2->emit dataChanged();
                // qInfo("  end of automation track");
            }
        }
        else if(t->type() == Track::BBTrack)
        {
            // qInfo("Divide: bb track %p",t);
            for(QVector<SelectableObject*>::iterator it = so.begin();
                it != so.end(); ++it)
            {
                TrackContentObjectView* tcov
                        = dynamic_cast<TrackContentObjectView*>(*it);
                // tcov->remove();
                if(!tcov)
                {
                    qCritical("Divide: tcov null");
                    continue;
                }
                TrackContentObject* tco = tcov->getTrackContentObject();
                if(!tco)
                {
                    qCritical("Divide: tco null");
                    continue;
                }
                if(tco->track() != t)
                    continue;
                BBTCO* p = dynamic_cast<BBTCO*>(tco);
                if(!p)
                {
                    qCritical("Divide: p null");
                    continue;
                }

                // qInfo("pos: s=%d e=%d r=%d",
                //  (int)p->startPosition(),(int)p->endPosition(),splitPos);
                if(p->startPosition() >= splitPos)
                    continue;
                if(p->endPosition() <= splitPos)
                    continue;

                if(first)
                {
                    t->addJournalCheckPoint();
                    first = false;
                }

                BBTCO* newp1 = new BBTCO(*p);  // 1 left, before
                newp1->setJournalling(false);
                newp1->movePosition(p->startPosition());
                newp1->changeLength(splitPos - p->startPosition());

                qInfo("SE: newp1->track=%p (t=%p)", newp1->track(), t);
                qInfo("SE: origp: p=%d l=%d", p->startPosition().getTicks(),
                      p->length().getTicks());
                qInfo("SE: newp1: p=%d l=%d",
                      newp1->startPosition().getTicks(),
                      newp1->length().getTicks());

                BBTCO* newp2 = new BBTCO(*p);  // 2 right, after
                newp2->setJournalling(false);
                newp2->movePosition(splitPos);
                newp2->changeLength(p->endPosition() - splitPos);

                // p->deleteLater instead?
                tcov->remove();

                newp1->setJournalling(true);
                newp2->setJournalling(true);
                newp1->emit dataChanged();
                newp2->emit dataChanged();
            }
            // qInfo("  end of bb track");
        }
        else if(t->type() == Track::SampleTrack)
        {
            qInfo("Divide: sample track %p", t);
            for(QVector<SelectableObject*>::iterator it = so.begin();
                it != so.end(); ++it)
            {
                TrackContentObjectView* tcov
                        = dynamic_cast<TrackContentObjectView*>(*it);
                // tcov->remove();
                if(!tcov)
                {
                    qCritical("Divide: tcov null");
                    continue;
                }
                TrackContentObject* tco = tcov->getTrackContentObject();
                if(!tco)
                {
                    qCritical("Divide: tco null");
                    continue;
                }
                if(tco->track() != t)
                    continue;
                SampleTCO* p = dynamic_cast<SampleTCO*>(tco);
                if(!p)
                {
                    qCritical("Divide: p null");
                    continue;
                }

                qInfo("pos: s=%d e=%d r=%d", (int)p->startPosition(),
                      (int)p->endPosition(), splitPos);
                if(p->startPosition() >= splitPos)
                    continue;
                if(p->endPosition() <= splitPos)
                    continue;

                if(first)
                {
                    t->addJournalCheckPoint();
                    first = false;
                }

                SampleTCO* newp1 = new SampleTCO(*p);  // 1 left, before
                newp1->setJournalling(false);
                newp1->movePosition(p->startPosition());
                newp1->changeLength(splitPos - p->startPosition());

                SampleTCO* newp2 = new SampleTCO(*p);  // 2 right, after
                newp2->setJournalling(false);
                newp2->movePosition(splitPos);
                newp2->changeLength(p->endPosition() - splitPos);
                newp2->setInitialPlayTick(newp2->initialPlayTick()
                                          + (splitPos - p->startPosition()));

                tcov->remove();

                newp1->setJournalling(true);
                newp2->setJournalling(true);
                newp1->emit dataChanged();
                newp2->emit dataChanged();
            }
            qInfo("Divide: end of sample track");
        }
    }
}

void SongEditor::wheelEvent(QWheelEvent* we)
{
    /*
    if( we->modifiers() & Qt::ControlModifier )
    {
            int z = m_zoomingXModel->value();

            if( we->delta() > 0 )
            {
                    z++;
            }
            else if( we->delta() < 0 )
            {
                    z--;
            }
            z = qBound( 0, z, m_zoomingXModel->size() - 1 );
            // update combobox with zooming-factor
            m_zoomingXModel->setValue( z );

            // update timeline
            m_song->m_playPos[Song::Mode_PlaySong].m_timeLine->
                                    setPixelsPerTact( pixelsPerTact() );
            // and make sure, all TCO's are resized and relocated
            realignTracks();
    }
    else
    */
    if(we->modifiers() & Qt::ShiftModifier
       || we->orientation() == Qt::Horizontal)
    {
        m_leftRightScroll->setValue(m_leftRightScroll->value()
                                    - we->delta() / 30);
    }
    else
    {
        we->ignore();
        return;
    }
    we->accept();
}

void SongEditor::closeEvent(QCloseEvent* ce)
{
    if(parentWidget())
    {
        parentWidget()->hide();
    }
    else
    {
        hide();
    }
    ce->ignore();
}

void SongEditor::setMasterVolume(real_t _newVal)
{
    /*
    updateMasterVolumeFloat( _newVal );
    if( !m_masterVolumeTFT->isVisible() && !m_song->m_loadingProject
    ) && m_masterVolumeKNB->showTFT()
    {
            m_masterVolumeTFT->moveGlobal( m_masterVolumeKNB,
                    QPoint( m_masterVolumeKNB->width() + 2, -2 ) );
            m_masterVolumeTFT->setVisibilityTimeOut( 1000 );
    }
    */
    Engine::mixer()->setMasterVolumeGain(_newVal / 100.0f);
}

/*
void SongEditor::showMasterVolumeFloat( void )
{
        m_masterVolumeTFT->moveGlobal( m_masterVolumeKNB,
                        QPoint( m_masterVolumeKNB->width() + 2, -2 ) );
        m_masterVolumeTFT->show();
        updateMasterVolumeFloat( m_song->m_masterVolumeModel.value() );
}

void SongEditor::updateMasterVolumeFloat( int _newVal )
{
        m_masterVolumeTFT->setText( tr( "Value: %1%" ).arg( _newVal ) );
}

void SongEditor::hideMasterVolumeFloat( void )
{
        m_masterVolumeTFT->hide();
}
*/

void SongEditor::setMasterPitch(real_t _newVal)
{
    /*
    updateMasterPitchFloat( _newVal );
    if( m_masterPitchTFT->isVisible() == false && m_song->m_loadingProject ==
    false ) //&& m_masterPitchKNB->showTFT()
    {
            m_masterPitchTFT->moveGlobal( m_masterPitchKNB,
                    QPoint( m_masterPitchKNB->width() + 2, -2 ) );
            m_masterPitchTFT->setVisibilityTimeOut( 1000 );
    }
    */
    // Engine::mixer()->setMasterPitchGain( _newVal );
}

/*
void SongEditor::showMasterPitchFloat( void )
{
        m_masterPitchTFT->moveGlobal( m_masterPitchKNB,
                        QPoint( m_masterPitchKNB->width() + 2, -2 ) );
        m_masterPitchTFT->show();
        updateMasterPitchFloat( m_song->m_masterPitchModel.value() );
}

void SongEditor::updateMasterPitchFloat( int _newVal )
{
        m_masterPitchTFT->setText( tr( "Value: %1 semitones").arg( _newVal )
);

}

void SongEditor::hideMasterPitchFloat( void )
{
        m_masterPitchTFT->hide();
}
*/

void SongEditor::setMasterPanning(real_t _newVal)
{
    /*
    updateMasterPanFloat( _newVal );
    if( m_masterPanningTFT->isVisible() == false && m_song->m_loadingProject
    == false ) //&& m_masterPanningKNB->showTFT()
    {
            m_masterPanningTFT->moveGlobal( m_masterPanningKNB,
                    QPoint( m_masterPanningKNB->width() + 2, -2 ) );
            m_masterPanningTFT->setVisibilityTimeOut( 1000 );
    }
    */
    Engine::mixer()->setMasterPanningGain(_newVal);
}

/*
void SongEditor::showMasterPanFloat( void )
{
        m_masterPanningTFT->moveGlobal( m_masterPanningKNB,
                        QPoint( m_masterPanningKNB->width() + 2, -2 ) );
        m_masterPanningTFT->show();
        updateMasterPanFloat( m_song->m_masterPanningModel.value() );
}

void SongEditor::updateMasterPanFloat( int _newVal )
{
        m_masterPanningTFT->setText( tr( "Value: %1 semitones").arg( _newVal )
);

}

void SongEditor::hideMasterPanFloat( void )
{
        m_masterPanningTFT->hide();
}
*/

void SongEditor::updateScrollBar(int len)
{
    m_leftRightScroll->setMaximum(len);
}

static inline void
        animateScroll(QScrollBar* scrollBar, int newVal, bool smoothScroll)
{
    if(smoothScroll == false)
    {
        scrollBar->setValue(newVal);
    }
    else if(scrollBar->value() != newVal)
    {
        // do smooth scroll animation using QTimeLine
        QTimeLine* t = scrollBar->findChild<QTimeLine*>();
        if(t == nullptr)
        {
            t = new QTimeLine(500, scrollBar);
            t->setFrameRange(scrollBar->value(), newVal);
            t->connect(t, SIGNAL(finished()), SLOT(deleteLater()));

            scrollBar->connect(t, SIGNAL(frameChanged(int)),
                               SLOT(setValue(int)));

            t->start();
        }
        else
        {
            // smooth scrolling is still active, therefore just update the end
            // frame
            t->setEndFrame(newVal);
        }
    }
}

void SongEditor::updatePosition(const MidiTime& t)
{
    int widgetWidth, trackOpWidth;
    if(ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt())
    {
        widgetWidth  = DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT;
        trackOpWidth = TRACK_OP_WIDTH_COMPACT;
    }
    else
    {
        widgetWidth  = DEFAULT_SETTINGS_WIDGET_WIDTH;
        trackOpWidth = TRACK_OP_WIDTH;
    }

    if((m_song->isPlaying() && m_song->m_playMode == Song::Mode_PlaySong
        && m_timeLine->autoScroll() == TimeLineWidget::AutoScrollEnabled)
       || m_scrollBack == true)
    {
        m_smoothScroll
                = ConfigManager::inst()->value("ui", "smoothscroll").toInt();

        const int w = width() - 1 - widgetWidth - trackOpWidth
                      - contentWidget()
                                ->verticalScrollBar()
                                ->width();  // width of right scrollbar
        const MidiTime e
                = m_currentPosition
                  + qRound(w * MidiTime::ticksPerTact() / pixelsPerTact());
        if(t >= e - 2)
        {
            animateScroll(m_leftRightScroll, t.getTact(), m_smoothScroll);
        }
        else if(t < m_currentPosition)
        {
            animateScroll(m_leftRightScroll, t.getTact(), m_smoothScroll);
        }
        m_scrollBack = false;
    }

    const int x
            = m_song->m_playPos[Song::Mode_PlaySong].m_timeLine->markerX(t)
              + 8;
    if(x >= trackOpWidth + widgetWidth
       && x < width() - contentWidget()->verticalScrollBar()->width())
    {
        // GDX m_positionLine->show();
        // GDX m_positionLine->move( x, m_timeLine->height() );
    }
    else
    {
        // GDX m_positionLine->hide();
    }

    /*
      GDX
      m_positionLine->setFixedHeight
            ( height()
              - m_timeLine->height()
              - contentWidget()->horizontalScrollBar()->height() );
    */
}

void SongEditor::updatePositionLine()
{
    // m_positionLine->setFixedHeight( height() );
}

void SongEditor::zoomingXChanged()
{
    setPixelsPerTact(EditorWindow::ZOOM_LEVELS[m_zoomingXModel->value()]
                     * 16.f);

    m_song->m_playPos[Song::Mode_PlaySong].m_timeLine->setPixelsPerTact(
            pixelsPerTact());
    realignTracks();
}

void SongEditor::zoomingYChanged()
{
    const float f = EditorWindow::ZOOM_LEVELS[m_zoomingYModel->value()];

    for(TrackView* tv: trackViews())
    {
        const int ht = qMax<int>(qRound(f * DEFAULT_TRACK_HEIGHT),
                                 MINIMAL_TRACK_HEIGHT);
        tv->setFixedHeight(ht);
    }
    realignTracks();
    for(TrackView* tv: trackViews())
        tv->track()->setHeight(height());
}

void SongEditor::selectAllTcos(bool select)
{
    QVector<SelectableObject*> so = select ? rubberBand()->selectableObjects()
                                           : rubberBand()->selectedObjects();
    for(int i = 0; i < so.count(); ++i)
    {
        so.at(i)->setSelected(select);
    }

    // qInfo("SongEditor::selectAllTcos");
    requireActionUpdate();
}

bool SongEditor::allowRubberband() const
{
    return m_mode == SelectMode;
}

ComboBoxModel* SongEditor::zoomingXModel() const
{
    return m_zoomingXModel;
}

ComboBoxModel* SongEditor::zoomingYModel() const
{
    return m_zoomingYModel;
}

ComboBoxModel* SongEditor::quantizeModel() const
{
    return m_quantizeModel;
}

SongWindow::SongWindow(Song* song) :
      EditorWindow(Engine::mixer()->audioDev()->supportsCapture()),
      m_editor(new SongEditor(song))
//, m_crtlAction(nullptr)
{
    setWindowTitle(tr("Song"));
    setWindowIcon(embed::getIconPixmap("songeditor"));

    setCentralWidget(m_editor);
    setAcceptDrops(true);
    m_toolBar->setAcceptDrops(true);
    connect(m_toolBar, SIGNAL(dragEntered(QDragEnterEvent*)), m_editor,
            SLOT(dragEnterEvent(QDragEnterEvent*)));
    connect(m_toolBar, SIGNAL(dropped(QDropEvent*)), m_editor,
            SLOT(dropEvent(QDropEvent*)));

    // Set up buttons
    m_playAction->setToolTip(tr("Play song (Space)"));
    m_recordAction->setToolTip(tr("Record samples from Audio-device"));
    m_recordAccompanyAction->setToolTip(
            tr("Record samples from Audio-device while playing song or BB "
               "track"));
    m_stopAction->setToolTip(tr("Stop song (Space)"));

    m_playAction->setWhatsThis(
            tr("Click here, if you want to play your whole song. "
               "Playing will be started at the song-position-marker (green). "
               "You can also move it while playing."));
    m_stopAction->setWhatsThis(
            tr("Click here, if you want to stop playing of your song. "
               "The song-position-marker will be set to the start of your "
               "song."));

    // Track actions
    /*
    DropToolBar* trackActionsToolBar = addDropToolBarToTop(tr("Track
    actions"));

    m_addBBTrackAction = new QAction(embed::getIconPixmap("add_bb_track"),
                                     tr("Add beat/bassline"), this);

    m_addSampleTrackAction = new
    QAction(embed::getIconPixmap("add_sample_track"), tr("Add sample-track"),
    this);

    m_addAutomationTrackAction = new
    QAction(embed::getIconPixmap("add_automation"), tr("Add
    automation-track"), this);

    connect(m_addBBTrackAction, SIGNAL(triggered()), m_editor->m_song,
    SLOT(addBBTrack())); connect(m_addSampleTrackAction, SIGNAL(triggered()),
    m_editor->m_song, SLOT(addSampleTrack()));
    connect(m_addAutomationTrackAction, SIGNAL(triggered()), m_editor->m_song,
    SLOT(addAutomationTrack()));

    trackActionsToolBar->addAction( m_addBBTrackAction );
    trackActionsToolBar->addAction( m_addSampleTrackAction );
    trackActionsToolBar->addAction( m_addAutomationTrackAction );
    */

    DropToolBar* trackActionsToolBar
            = addDropToolBarToTop(tr("Track actions"));

    trackActionsToolBar->addAction(
            embed::getIconPixmap("add_instrument_track"),
            tr("Add instrument-track"), Engine::getSong(),
            SLOT(addInstrumentTrack()));

    trackActionsToolBar->addAction(embed::getIconPixmap("add_bb_track"),
                                   tr("Add beat/bassline"), Engine::getSong(),
                                   SLOT(addBBTrack()));

    trackActionsToolBar->addAction(embed::getIconPixmap("add_sample_track"),
                                   tr("Add sample-track"), Engine::getSong(),
                                   SLOT(addSampleTrack()));

    trackActionsToolBar->addAction(
            embed::getIconPixmap("add_automation_track"),
            tr("Add automation-track"), Engine::getSong(),
            SLOT(addAutomationTrack()));

    // Edit actions
    DropToolBar* editActionsToolBar = addDropToolBarToTop(tr("Edit actions"));

    m_editModeGroup  = new ActionGroup(this);
    m_drawModeAction = m_editModeGroup->addAction(embed::getIcon("edit_draw"),
                                                  tr("Draw mode"));
    m_drawModeAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_D));
    m_selectModeAction = m_editModeGroup->addAction(
            embed::getIcon("edit_select"), tr("Edit mode (select and move)"));
    m_drawModeAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_S));

    editActionsToolBar->addSeparator();
    editActionsToolBar->addAction(m_drawModeAction);
    editActionsToolBar->addAction(m_selectModeAction);

    connect(m_drawModeAction, SIGNAL(triggered()), m_editor,
            SLOT(setEditModeDraw()));
    connect(m_selectModeAction, SIGNAL(triggered()), m_editor,
            SLOT(setEditModeSelect()));
    m_drawModeAction->setChecked(true);

    // Loop mark actions
    DropToolBar* loopMarkToolBar = addDropToolBarToTop(tr("Loop marks"));
    loopMarkToolBar->addBlank();
    loopMarkToolBar->addBlank();
    loopMarkToolBar->addBlank();
    loopMarkToolBar->addBlank();
    loopMarkToolBar->addBlank();
    loopMarkToolBar->addSeparator();
    m_editor->m_timeLine->addLoopMarkButtons(loopMarkToolBar);

    addToolBarBreak();

    // Zoom actions
    DropToolBar* zoomTLB = addDropToolBarToTop(tr("Zoom controls"));

    QLabel* zoomXLBL = new QLabel(zoomTLB);
    zoomXLBL->setPixmap(embed::getIconPixmap("zoom_x"));
    zoomXLBL->setFixedSize(32, 32);
    zoomXLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_zoomingXComboBox = new ComboBox(zoomTLB);
    m_zoomingXComboBox->setFixedSize(70, 32);
    // m_zoomingXComboBox->move( 580, 4 );
    m_zoomingXComboBox->setModel(m_editor->m_zoomingXModel);

    zoomTLB->addWidget(zoomXLBL);
    zoomTLB->addWidget(m_zoomingXComboBox);

    new QShortcut(Qt::Key_Minus, m_zoomingXComboBox, SLOT(selectPrevious()));
    new QShortcut(Qt::Key_Plus, m_zoomingXComboBox, SLOT(selectNext()));

    QLabel* zoomYLBL = new QLabel(zoomTLB);
    zoomYLBL->setPixmap(embed::getIconPixmap("zoom_y"));
    zoomYLBL->setFixedSize(32, 32);
    zoomYLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_zoomingYComboBox = new ComboBox(zoomTLB);
    m_zoomingYComboBox->setFixedSize(70, 32);
    // m_zoomingXComboBox->move( 580, 4 );
    m_zoomingYComboBox->setModel(m_editor->m_zoomingYModel);

    zoomTLB->addWidget(zoomYLBL);
    zoomTLB->addWidget(m_zoomingYComboBox);

    new QShortcut(Qt::SHIFT | Qt::Key_Minus, m_zoomingYComboBox,
                  SLOT(selectPrevious()));
    new QShortcut(Qt::SHIFT | Qt::Key_Plus, m_zoomingYComboBox,
                  SLOT(selectNext()));

    // Quantize actions
    DropToolBar* quantizeTLB = addDropToolBarToTop(tr("Quantize controls"));

    QLabel* quantizeLBL = new QLabel(quantizeTLB);
    quantizeLBL->setPixmap(embed::getPixmap("quantize"));
    quantizeLBL->setFixedSize(32, 32);
    quantizeLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_quantizeComboBox = new ComboBox(quantizeTLB);
    m_quantizeComboBox->setFixedSize(70, 32);
    // m_quantizeComboBox->move( 580, 4 );
    m_quantizeComboBox->setModel(m_editor->m_quantizeModel);

    quantizeTLB->addWidget(quantizeLBL);
    quantizeTLB->addWidget(m_quantizeComboBox);

    // Timeline actions
    DropToolBar* timeLineToolBar
            = addDropToolBarToTop(tr("Timeline controls"));
    timeLineToolBar->addSeparator();
    m_editor->m_timeLine->addToolButtons(timeLineToolBar);

    // Loop size actions
    DropToolBar* loopSizeToolBar = addDropToolBarToTop(tr("Loop sizes"));
    loopSizeToolBar->addSeparator();
    m_editor->m_timeLine->addLoopSizeButtons(loopSizeToolBar);

    connect(song, SIGNAL(projectLoaded()), this,
            SLOT(adjustUiAfterProjectLoad()));
    connect(this, SIGNAL(resized()), m_editor, SLOT(updatePositionLine()));
}

QSize SongWindow::sizeHint() const
{
    return {900, 280};
}

void SongWindow::resizeEvent(QResizeEvent* _event)
{
    EditorWindow ::resizeEvent(_event);
    emit resized();
}

void SongWindow::play()
{
    emit playTriggered();

    if(Engine::getSong()->isPlaying())
        Engine::transport()->transportStop();
    else
        Engine::transport()->transportStart();

    /*
    if( Engine::getSong()->playMode() != Song::Mode_PlaySong )
    {
            Engine::getSong()->playSong();
    }
    else
    {
            Engine::getSong()->togglePause();
    }
    */
    requireActionUpdate();
}

void SongWindow::record()
{
    m_editor->m_song->record();
    requireActionUpdate();
}

void SongWindow::recordAccompany()
{
    m_editor->m_song->playAndRecord();
    requireActionUpdate();
}

void SongWindow::stop()
{
    Engine::transport()->transportStop();
    m_editor->m_song->stop();
    gui->pianoRollWindow()->stopRecording();
    requireActionUpdate();
}

void SongWindow::lostFocus()
{
    /*
    if(m_crtlAction)
    {
        m_crtlAction->setChecked(true);
        m_crtlAction->trigger();
    }
    */
    m_drawModeAction->setChecked(true);
    requireActionUpdate();
}

void SongWindow::adjustUiAfterProjectLoad()
{
    // make sure to bring us to front as the song editor is the central
    // widget in a song and when just opening a song in order to listen to
    // it, it's very annyoing to manually bring up the song editor each time
    gui->mainWindow()->workspace()->setActiveSubWindow(
            qobject_cast<QMdiSubWindow*>(parentWidget()));
    connect(qobject_cast<SubWindow*>(parentWidget()), SIGNAL(focusLost()),
            this, SLOT(lostFocus()));

    m_editor->realignTracks();
    m_editor->scrolled(0);
    requireActionUpdate();
}

void SongWindow::keyPressEvent(QKeyEvent* ke)
{
    if(ke->key() == Qt::Key_Control)
    {
        if(m_selectModeAction->isChecked())
        {
            m_drawModeAction->setChecked(true);
            m_drawModeAction->trigger();
        }
        else if(m_drawModeAction->isChecked())
        {
            m_selectModeAction->setChecked(true);
            m_selectModeAction->trigger();
        }
        /*
        m_crtlAction = m_editModeGroup->checkedAction();
        m_selectModeAction->setChecked(true);
        m_selectModeAction->trigger();
        */
    }
}

void SongWindow::keyReleaseEvent(QKeyEvent* ke)
{
    if(ke->key() == Qt::Key_Control)
    {
        if(m_selectModeAction->isChecked())
        {
            m_drawModeAction->setChecked(true);
            m_drawModeAction->trigger();
        }
        else if(m_drawModeAction->isChecked())
        {
            m_selectModeAction->setChecked(true);
            m_selectModeAction->trigger();
        }
        /*
        if(m_crtlAction)
        {
            m_crtlAction->setChecked(true);
            m_crtlAction->trigger();
        }
        */
    }
}

// ActionUpdatable //

void SongWindow::updateActions(const bool            _active,
                               QHash<QString, bool>& _table) const
{
    // qInfo("SongWindow::updateActions() active=%d",_active);
    m_editor->updateActions(_active, _table);
}

void SongWindow::actionTriggered(QString _name)
{
    m_editor->actionTriggered(_name);
}

void SongEditor::updateActions(const bool            _active,
                               QHash<QString, bool>& _table) const
{
    // qInfo("SongEditor::updateActions() active=%d",_active);
    bool hasSelection = _active && selectedObjects().size() > 0;
    // qInfo("SongWindow::updateActions() active=%d selection=%d",
    //      _active,hasSelection);
    _table.insert("edit_cut", hasSelection);
    _table.insert("edit_copy", hasSelection);
    _table.insert("edit_paste", _active);
}

void SongEditor::actionTriggered(QString _name)
{
    qInfo("SongEditor::actionTriggered() name=%s", qPrintable(_name));
    if(_name == "edit_cut")
        cutSelection();
    else if(_name == "edit_copy")
        copySelection();
    else if(_name == "edit_paste")
        pasteSelection();
}

// Selection //

void SongEditor::deleteSelection()
{
    QVector<SelectableObject*> so = selectedObjects();
    for(QVector<SelectableObject*>::iterator it = so.begin(); it != so.end();
        ++it)
    {
        TrackContentObjectView* tcov
                = dynamic_cast<TrackContentObjectView*>(*it);
        tcov->remove();
    }
}

void SongEditor::cutSelection()
{
    copySelection();
    deleteSelection();
}

void SongEditor::copySelection()
{
    QVector<TrackContentObject*> so = selectedTCOs();
    if(so.length() > 0)
        Clipboard::copy(so.at(0));
}

void SongEditor::pasteSelection()
{
    // Clipboard::paste(vso)
}
