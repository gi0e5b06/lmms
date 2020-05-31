/*
 * PianoRoll.cpp - implementation of piano roll which is used for actual
 *                  writing of melodies
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2008      Andrew Kelley <superjoe30/at/gmail/dot/com>
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

#include "PianoRoll.h"

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include "ActionGroup.h"
#include "AutomationEditor.h"
#include "BBTrackContainer.h"
#include "Clipboard.h"
#include "ComboBox.h"
//#include "ConfigManager.h"
#include "Configuration.h"
#include "DetuningHelper.h"  // REQUIRED
#include "GuiApplication.h"
#include "InstrumentTrack.h"
#include "MainWindow.h"
#include "Pattern.h"
#include "SongEditor.h"
#include "TextFloat.h"
#include "TimeLineWidget.h"

//#include "debug.h"
#include "embed.h"
#include "gui_templates.h"
#include "lmms_qt_core.h"

#include <QApplication>
#include <QClipboard>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLabel>
//#include <QLayout>
//#include <QMdiArea>
#include <QPainter>
#include <QPointer>
#include <QScrollBar>
#include <QShortcut>
#include <QSignalMapper>
#include <QStyleOption>

#include <cmath>

#if QT_VERSION < 0x040800
#define MiddleButton MidButton
#endif

typedef AutomationPattern::timeMap timeMap;

// some constants
const int SCROLLBAR_SIZE = 12;
const int PIANO_X        = 0;

const int WHITE_KEY_WIDTH = 64;
// const int BLACK_KEY_WIDTH = 41;
const int WHITE_KEY_SMALL_HEIGHT = 18;
const int WHITE_KEY_BIG_HEIGHT   = 24;
const int BLACK_KEY_HEIGHT       = 16;
// const int C_KEY_LABEL_X = WHITE_KEY_WIDTH - 19;
const int KEY_LINE_HEIGHT = 12;
const int OCTAVE_HEIGHT   = KEY_LINE_HEIGHT * KeysPerOctave;  // = 12 * 12;

const int NOTE_EDIT_RESIZE_BAR = 6;
const int NOTE_EDIT_MIN_HEIGHT = 50;
const int KEY_AREA_MIN_HEIGHT  = 100;
const int PR_BOTTOM_MARGIN     = SCROLLBAR_SIZE;
const int PR_TOP_MARGIN        = 16;
const int PR_RIGHT_MARGIN      = SCROLLBAR_SIZE;

// width of area used for resizing (the grip at the end of a note)
const int RESIZE_AREA_WIDTH = 9;

// width of line for setting volume/panning of note
const int NOTE_EDIT_LINE_WIDTH = 3;

// key where to start
const int INITIAL_START_KEY = Key_C + Octave_4 * KeysPerOctave;

// number of each note to provide in quantization and note lengths
const int NUM_EVEN_LENGTHS    = 6;
const int NUM_TRIPLET_LENGTHS = 5;

QPixmap* PianoRoll::s_whiteKeySmallPm        = nullptr;
QPixmap* PianoRoll::s_whiteKeySmallPressedPm = nullptr;
QPixmap* PianoRoll::s_whiteKeyBigPm          = nullptr;
QPixmap* PianoRoll::s_whiteKeyBigPressedPm   = nullptr;
QPixmap* PianoRoll::s_blackKeyPm             = nullptr;
QPixmap* PianoRoll::s_blackKeyPressedPm      = nullptr;
QPixmap* PianoRoll::s_toolDraw               = nullptr;
QPixmap* PianoRoll::s_toolErase              = nullptr;
QPixmap* PianoRoll::s_toolSelect             = nullptr;
QPixmap* PianoRoll::s_toolMove               = nullptr;
QPixmap* PianoRoll::s_toolOpen               = nullptr;

TextFloat* PianoRoll::s_textFloat = nullptr;

/*
static QString s_noteStrings[12]
        = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

static QString getNoteString(int key)
{
    int o = (key / KeysPerOctave) - 1;
    if(o < 0)
        return s_noteStrings[key % 12] + "❶";
    else
        return s_noteStrings[key % 12] + QString::number(o, 10);
}
*/

// used for drawing of piano
PianoRoll::PianoRollKeyTypes PianoRoll::prKeyOrder[]
        = {PR_WHITE_KEY_SMALL, PR_BLACK_KEY,       PR_WHITE_KEY_BIG,
           PR_BLACK_KEY,       PR_WHITE_KEY_SMALL, PR_WHITE_KEY_SMALL,
           PR_BLACK_KEY,       PR_WHITE_KEY_BIG,   PR_BLACK_KEY,
           PR_WHITE_KEY_BIG,   PR_BLACK_KEY,       PR_WHITE_KEY_SMALL};

const int DEFAULT_PR_PPT = (KEY_LINE_HEIGHT * DefaultStepsPerTact * 11) / 12;

// const QVector<double> PianoRoll::m_zoomLevels =
//      { 0.10f, 0.20f, 0.50f, 1.0f, 2.0f, 5.0f, 10.0f, 20.0f };
//{ 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f };

PianoRoll::PianoRoll() :
      m_nemStr(QVector<QString>()), m_noteEditMenu(nullptr),
      m_semiToneMarkerMenu(nullptr), m_zoomingXModel(nullptr, "[zoom x]"),
      m_quantizeModel(nullptr, "[quantize]"),
      m_noteLenModel(nullptr, "[note length]"),
      m_rootModel(nullptr, "[root]"), m_scaleModel(nullptr, "[scale]"),
      m_chordModel(nullptr, "[chord]"), m_pattern(nullptr),
      m_ghostPattern(nullptr), m_currentPosition(), m_recording(false),
      m_currentNote(nullptr), m_action(ActionNone),
      m_noteEditMode(NoteEditVolume), m_moveBoundaryLeft(0),
      m_moveBoundaryTop(0), m_moveBoundaryRight(0), m_moveBoundaryBottom(0),
      m_mouseDownKey(0), m_mouseDownTick(0), m_lastMouseX(0), m_lastMouseY(0),
      m_oldNotesEditHeight(100), m_notesEditHeight(100),
      m_ppt(DEFAULT_PR_PPT),
      m_lenOfNewNotes(MidiTime(0, DefaultTicksPerTact / 4)),
      m_lastNoteVolume(DefaultVolume), m_lastNotePanning(DefaultPanning),
      m_lastNoteLegato(false), m_lastNoteMarcato(false),
      m_lastNoteStaccato(false), m_startKey(INITIAL_START_KEY), m_lastKey(-1),
      m_editMode(ModeDraw), m_ctrlMode(ModeDraw), m_mouseDownRight(false),
      m_scrollBack(false), m_barLineColor(0, 0, 0), m_beatLineColor(0, 0, 0),
      m_lineColor(0, 0, 0), m_noteModeColor(0, 0, 0), m_noteColor(0, 0, 0),
      m_ghostNoteColor(0, 0, 0), m_barColor(0, 0, 0),
      m_selectedNoteColor(0, 0, 0), m_textColor(0, 0, 0),
      m_textColorLight(0, 0, 0), m_textShadow(0, 0, 0),
      m_markedSemitoneColor(0, 0, 0), m_noteOpacity(255),
      m_ghostNoteOpacity(255), m_noteBorders(true), m_ghostNoteBorders(true),
      m_backgroundShade(0, 0, 0)
{
    // gui names of edit modes
    m_nemStr.push_back(tr("Note Velocity"));
    m_nemStr.push_back(tr("Note Panning"));
    m_nemStr.push_back(tr("Note Legato"));
    m_nemStr.push_back(tr("Note Marcato"));
    m_nemStr.push_back(tr("Note Staccato"));

    QSignalMapper* signalMapper = new QSignalMapper(this);
    m_noteEditMenu              = new QMenu(this);
    m_noteEditMenu->clear();
    for(int i = 0; i < m_nemStr.size(); ++i)
    {
        QAction* act = new QAction(m_nemStr.at(i), this);
        connect(act, SIGNAL(triggered()), signalMapper, SLOT(map()));
        signalMapper->setMapping(act, i);
        m_noteEditMenu->addAction(act);
    }
    connect(signalMapper, SIGNAL(mapped(int)), this,
            SLOT(changeNoteEditMode(int)));

    signalMapper         = new QSignalMapper(this);
    m_semiToneMarkerMenu = new QMenu(this);

    QAction* markSemitoneAction
            = new QAction(tr("Mark/unmark current semitone"), this);
    QAction* markAllOctaveSemitonesAction = new QAction(
            tr("Mark/unmark all corresponding octave semitones"), this);
    QAction* markScaleAction = new QAction(tr("Mark current scale"), this);
    QAction* markChordAction = new QAction(tr("Mark current chord"), this);
    QAction* unmarkAllAction = new QAction(tr("Unmark all"), this);
    QAction* copyAllNotesAction
            = new QAction(tr("Select all notes on this key"), this);

    connect(markSemitoneAction, SIGNAL(triggered()), signalMapper,
            SLOT(map()));
    connect(markAllOctaveSemitonesAction, SIGNAL(triggered()), signalMapper,
            SLOT(map()));
    connect(markScaleAction, SIGNAL(triggered()), signalMapper, SLOT(map()));
    connect(markChordAction, SIGNAL(triggered()), signalMapper, SLOT(map()));
    connect(unmarkAllAction, SIGNAL(triggered()), signalMapper, SLOT(map()));
    connect(copyAllNotesAction, SIGNAL(triggered()), signalMapper,
            SLOT(map()));

    signalMapper->setMapping(markSemitoneAction,
                             static_cast<int>(stmaMarkCurrentSemiTone));
    signalMapper->setMapping(markAllOctaveSemitonesAction,
                             static_cast<int>(stmaMarkAllOctaveSemiTones));
    signalMapper->setMapping(markScaleAction,
                             static_cast<int>(stmaMarkCurrentScale));
    signalMapper->setMapping(markChordAction,
                             static_cast<int>(stmaMarkCurrentChord));
    signalMapper->setMapping(unmarkAllAction,
                             static_cast<int>(stmaUnmarkAll));
    signalMapper->setMapping(copyAllNotesAction,
                             static_cast<int>(stmaCopyAllNotesOnKey));

    markScaleAction->setEnabled(false);
    markChordAction->setEnabled(false);

    connect(this, SIGNAL(semiToneMarkerMenuScaleSetEnabled(bool)),
            markScaleAction, SLOT(setEnabled(bool)));
    connect(this, SIGNAL(semiToneMarkerMenuChordSetEnabled(bool)),
            markChordAction, SLOT(setEnabled(bool)));

    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(markSemiTone(int)));

    m_semiToneMarkerMenu->addAction(markSemitoneAction);
    m_semiToneMarkerMenu->addAction(markAllOctaveSemitonesAction);
    m_semiToneMarkerMenu->addAction(markScaleAction);
    m_semiToneMarkerMenu->addAction(markChordAction);
    m_semiToneMarkerMenu->addAction(unmarkAllAction);
    m_semiToneMarkerMenu->addAction(copyAllNotesAction);

    // init pixmaps
    if(s_whiteKeySmallPm == nullptr)
    {
        s_whiteKeySmallPm
                = new QPixmap(embed::getIconPixmap("pr_white_key_small"));
    }
    if(s_whiteKeySmallPressedPm == nullptr)
    {
        s_whiteKeySmallPressedPm = new QPixmap(
                embed::getIconPixmap("pr_white_key_small_pressed"));
    }
    if(s_whiteKeyBigPm == nullptr)
    {
        s_whiteKeyBigPm
                = new QPixmap(embed::getIconPixmap("pr_white_key_big"));
    }
    if(s_whiteKeyBigPressedPm == nullptr)
    {
        s_whiteKeyBigPressedPm = new QPixmap(
                embed::getIconPixmap("pr_white_key_big_pressed"));
    }
    if(s_blackKeyPm == nullptr)
    {
        s_blackKeyPm = new QPixmap(embed::getIconPixmap("pr_black_key"));
    }
    if(s_blackKeyPressedPm == nullptr)
    {
        s_blackKeyPressedPm
                = new QPixmap(embed::getIconPixmap("pr_black_key_pressed"));
    }
    if(s_toolDraw == nullptr)
    {
        s_toolDraw = new QPixmap(embed::getIconPixmap("edit_draw"));
    }
    if(s_toolErase == nullptr)
    {
        s_toolErase = new QPixmap(embed::getIconPixmap("edit_erase"));
    }
    if(s_toolSelect == nullptr)
    {
        s_toolSelect = new QPixmap(embed::getIconPixmap("edit_select"));
    }
    if(s_toolMove == nullptr)
    {
        s_toolMove = new QPixmap(embed::getIconPixmap("edit_move"));
    }
    if(s_toolOpen == nullptr)
    {
        s_toolOpen = new QPixmap(embed::getIconPixmap("automation"));
    }

    // init text-float
    if(s_textFloat == nullptr)
    {
        s_textFloat = new TextFloat;
    }

    setAttribute(Qt::WA_OpaquePaintEvent, true);

    // add time-line
    m_timeLine = new TimeLineWidget(
            WHITE_KEY_WIDTH, 0, m_ppt,
            Engine::getSong()->getPlayPos(Song::Mode_PlayPattern),
            m_currentPosition, this);
    connect(this, SIGNAL(positionChanged(const MidiTime&)), m_timeLine,
            SLOT(updatePosition(const MidiTime&)));
    connect(m_timeLine, SIGNAL(positionChanged(const MidiTime&)), this,
            SLOT(updatePosition(const MidiTime&)));

    // update timeline when in record-accompany mode
    connect(Engine::getSong()->getPlayPos(Song::Mode_PlaySong).m_timeLine,
            SIGNAL(positionChanged(const MidiTime&)), this,
            SLOT(updatePositionAccompany(const MidiTime&)));
    // TODO
    /*	connect( engine::getSong()->getPlayPos( Song::Mode_PlayBB
       ).m_timeLine, SIGNAL( positionChanged( const MidiTime & ) ), this,
                            SLOT( updatePositionAccompany( const MidiTime & )
       ) );*/

    removeSelection();

    // init scrollbars
    m_leftRightScroll = new QScrollBar(Qt::Horizontal, this);
    m_leftRightScroll->setSingleStep(1);
    connect(m_leftRightScroll, SIGNAL(valueChanged(int)), this,
            SLOT(horScrolled(int)));

    m_topBottomScroll = new QScrollBar(Qt::Vertical, this);
    m_topBottomScroll->setSingleStep(1);
    m_topBottomScroll->setPageStep(20);
    connect(m_topBottomScroll, SIGNAL(valueChanged(int)), this,
            SLOT(verScrolled(int)));

    // setup zooming-stuff
    EditorWindow::fillZoomLevels(m_zoomingXModel);
    /*
    for( float const & zoomLevel : EditorWindow::ZOOM_LEVELS )
    {
            m_zoomingXModel.addItem( QString( "%1\%" ).arg( zoomLevel * 100 )
    );
    }
    */
    m_zoomingXModel.setValue(m_zoomingXModel.findText("100%"));
    connect(&m_zoomingXModel, SIGNAL(dataChanged()), this,
            SLOT(zoomingXChanged()));

    // Set up quantization model
    m_quantizeModel.addItem(tr("Note lock"));
    EditorWindow::fillQuantizeLevels(m_quantizeModel);
    /*
    m_quantizeModel.addItem( "4/1" );
    m_quantizeModel.addItem( "2/1" );
    m_quantizeModel.addItem( "1/1" );
    m_quantizeModel.addItem( "1/2" );
    m_quantizeModel.addItem( "1/4" );
    m_quantizeModel.addItem( "1/8" );
    m_quantizeModel.addItem( "1/16" );
    m_quantizeModel.addItem( "1/32" );
    m_quantizeModel.addItem( "1/64" );
    m_quantizeModel.addItem( "1/128" );
    m_quantizeModel.addItem( "1/3" );
    m_quantizeModel.addItem( "1/6" );
    m_quantizeModel.addItem( "1/12" );
    m_quantizeModel.addItem( "1/24" );
    m_quantizeModel.addItem( "1/48" );
    m_quantizeModel.addItem( "1/96" );
    m_quantizeModel.addItem( "1/192" );
    */
    m_quantizeModel.setValue(m_quantizeModel.findText("1/16"));
    connect(&m_quantizeModel, SIGNAL(dataChanged()), this,
            SLOT(quantizeChanged()));

    // Set up note length model
    m_noteLenModel.addItem(tr("Last note"), new PixmapLoader("edit_draw"));
    EditorWindow::fillLengthLevels(m_noteLenModel);
    /*
    const QString pixmaps[] = { "whole", "half", "quarter", "eighth",
                                            "sixteenth", "thirtysecond",
    "triplethalf", "tripletquarter", "tripleteighth", "tripletsixteenth",
    "tripletthirtysecond" } ; for( int i = 0; i < NUM_EVEN_LENGTHS; ++i )
    {
            PixmapLoader *loader = new PixmapLoader( "note_" + pixmaps[i] );
            m_noteLenModel.addItem( "1/" + QString::number( 1 << i ), loader
    );
    }
    for( int i = 0; i < NUM_TRIPLET_LENGTHS; ++i )
    {
            PixmapLoader *loader = new PixmapLoader( "note_" +
    pixmaps[i+NUM_EVEN_LENGTHS] ); m_noteLenModel.addItem( "1/" +
    QString::number( (1 << i) * 3 ), loader );
    }
    */
    m_noteLenModel.setValue(0);
    // Note length change can cause a redraw if Q is set to lock
    connect(&m_noteLenModel, SIGNAL(dataChanged()), this,
            SLOT(quantizeChanged()));

    // Set up root model
    for(int key = 0; key < KeysPerOctave; key++)
        m_rootModel.addItem(Note::findNoteName(key));
    m_rootModel.setValue(0);
    connect(&m_rootModel, SIGNAL(dataChanged()), this, SLOT(rootChanged()));

    // Set up scale model
    const InstrumentFunctionNoteStacking::ChordTable& chord_table
            = InstrumentFunctionNoteStacking::ChordTable::getInstance();

    m_scaleModel.addItem(tr("No scale"));
    for(const InstrumentFunctionNoteStacking::Chord& chord: chord_table)
    {
        if(chord.isScale())
        {
            m_scaleModel.addItem(chord.getName());
        }
    }

    m_scaleModel.setValue(0);
    // change can update m_semiToneMarkerMenu
    connect(&m_scaleModel, SIGNAL(dataChanged()), this,
            SLOT(updateSemiToneMarkerMenu()));
    connect(&m_scaleModel, SIGNAL(dataChanged()), this, SLOT(scaleChanged()));

    // Set up chord model
    m_chordModel.addItem(tr("No chord"));
    for(const InstrumentFunctionNoteStacking::Chord& chord: chord_table)
    {
        if(!chord.isScale())
        {
            m_chordModel.addItem(chord.getName());
        }
    }

    m_chordModel.setValue(0);

    // change can update m_semiToneMarkerMenu
    connect(&m_chordModel, SIGNAL(dataChanged()), this,
            SLOT(updateSemiToneMarkerMenu()));

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setMouseTracking(true);

    connect(&m_scaleModel, SIGNAL(dataChanged()), this,
            SLOT(updateSemiToneMarkerMenu()));

    connect(Engine::getSong(), SIGNAL(timeSignatureChanged(int, int)), this,
            SLOT(update()));

    // connection for selecion from timeline
    connect(m_timeLine, SIGNAL(regionSelectedFromPixels(int, int)), this,
            SLOT(selectRegionFromPixels(int, int)));
}

void PianoRoll::reset()
{
    m_pattern      = nullptr;
    m_ghostPattern = nullptr;

    m_lastNoteVolume   = DefaultVolume;
    m_lastNotePanning  = DefaultPanning;
    m_lastNoteLegato   = false;
    m_lastNoteMarcato  = false;
    m_lastNoteStaccato = false;
}

void PianoRoll::showTextFloat(const QString& text,
                              const QPoint&  pos,
                              int            timeout)
{
    s_textFloat->setText(text);
    // show the float, offset slightly so as to not obscure anything
    s_textFloat->moveGlobal(this, pos + QPoint(4, 16));
    if(timeout == -1)
    {
        s_textFloat->show();
    }
    else
    {
        s_textFloat->setVisibilityTimeOut(timeout);
    }
}

void PianoRoll::showVolTextFloat(volume_t vol, const QPoint& pos, int timeout)
{
    //! \todo display velocity for MIDI-based instruments
    // possibly dBFS values too? not sure if it makes sense for note
    // volumes
    showTextFloat(tr("Velocity: %1%").arg(vol), pos, timeout);
}

void PianoRoll::showPanTextFloat(panning_t     pan,
                                 const QPoint& pos,
                                 int           timeout)
{
    QString text;
    if(pan < 0)
    {
        text = tr("Panning: %1% left").arg(qAbs(pan));
    }
    else if(pan > 0)
    {
        text = tr("Panning: %1% right").arg(qAbs(pan));
    }
    else
    {
        text = tr("Panning: center");
    }
    showTextFloat(text, pos, timeout);
}

void PianoRoll::showLegatoTextFloat(bool _b, const QPoint& pos, int timeout)
{
    QString text = tr("Legato: %1").arg(_b ? tr("On") : tr("Off"));
    showTextFloat(text, pos, timeout);
}

void PianoRoll::showMarcatoTextFloat(bool _b, const QPoint& pos, int timeout)
{
    QString text = tr("Marcato: %1").arg(_b ? tr("On") : tr("Off"));
    showTextFloat(text, pos, timeout);
}

void PianoRoll::showStaccatoTextFloat(bool _b, const QPoint& pos, int timeout)
{
    QString text = tr("Staccato: %1").arg(_b ? tr("On") : tr("Off"));
    showTextFloat(text, pos, timeout);
}

void PianoRoll::changeNoteEditMode(int i)
{
    m_noteEditMode = (NoteEditMode)i;
    update();  // repaint();
}

void PianoRoll::markSemiTone(int i)
{
    const int key = getKey(mapFromGlobal(m_semiToneMarkerMenu->pos()).y());
    markSemiTone(i, key, nullptr);
}

void PianoRoll::markSemiTone(
        const int                                    i,
        const int                                    key,
        const InstrumentFunctionNoteStacking::Chord* chord)
{
    switch(static_cast<SemiToneMarkerAction>(i))
    {
        case stmaUnmarkAll:
            m_markedSemiTones.clear();
            break;
        case stmaMarkCurrentSemiTone:
        {
            QList<int>::iterator it = qFind(m_markedSemiTones.begin(),
                                            m_markedSemiTones.end(), key);
            if(it != m_markedSemiTones.end())
            {
                m_markedSemiTones.erase(it);
            }
            else
            {
                m_markedSemiTones.push_back(key);
            }
            break;
        }
        case stmaMarkAllOctaveSemiTones:
        {
            QList<int> aok = getAllOctavesForKey(key);

            if(m_markedSemiTones.contains(key))
            {
                // lets erase all of the ones that match this by octave
                QList<int>::iterator i;
                for(int ix = 0; ix < aok.size(); ++ix)
                {
                    i = qFind(m_markedSemiTones.begin(),
                              m_markedSemiTones.end(), aok.at(ix));
                    m_markedSemiTones.erase(i);
                }
            }
            else
            {
                // we should add all of the ones that match this by octave
                m_markedSemiTones.append(aok);
            }

            break;
        }
        case stmaMarkCurrentScale:
            // qInfo("PianoRoll scale=%s",m_scaleModel.currentText());
            if(!chord)
            {
                chord = &InstrumentFunctionNoteStacking::ChordTable::
                                 getInstance()
                                         .getScaleByName(
                                                 m_scaleModel.currentText());
            }
        case stmaMarkCurrentChord:
        {
            if(!chord)
            {
                chord = &InstrumentFunctionNoteStacking::ChordTable::
                                 getInstance()
                                         .getChordByName(
                                                 m_chordModel.currentText());
            }

            if(chord->isEmpty())
            {
                break;
            }
            else if(chord->isScale())
            {
                m_markedSemiTones.clear();
            }

            const int first = chord->isScale() ? 0 : key;
            const int last = chord->isScale() ? NumKeys : key + chord->last();
            const int cap  = (chord->isScale() || chord->last() == 0)
                                    ? KeysPerOctave
                                    : chord->last();

            for(int i = first; i <= last; i++)
            {
                if(chord->hasSemiTone((i + cap - (key % cap)) % cap))
                {
                    m_markedSemiTones.push_back(i);
                }
            }
            break;
        }
        case stmaCopyAllNotesOnKey:
        {
            selectNotesOnKey();
            break;
        }
        default:;
    }

    qSort(m_markedSemiTones.begin(), m_markedSemiTones.end(),
          qGreater<int>());
    QList<int>::iterator new_end
            = std::unique(m_markedSemiTones.begin(), m_markedSemiTones.end());
    m_markedSemiTones.erase(new_end, m_markedSemiTones.end());
}

void PianoRoll::rootChanged()
{
    markSemiTone(stmaUnmarkAll, 0, nullptr);
    if(m_scaleModel.value() == 0)
        markSemiTone(stmaMarkAllOctaveSemiTones, m_rootModel.value(),
                     nullptr);
    else
        markSemiTone(stmaMarkCurrentScale, m_rootModel.value(), nullptr);
}

void PianoRoll::scaleChanged()
{
    markSemiTone(stmaUnmarkAll, 0, nullptr);
    if(m_scaleModel.value() == 0)
        markSemiTone(stmaMarkAllOctaveSemiTones, m_rootModel.value(),
                     nullptr);
    else
        markSemiTone(stmaMarkCurrentScale, m_rootModel.value(), nullptr);
}

void PianoRoll::guessScale()
{
    if(m_pattern == nullptr)
        return;

    uint64_t keys = 0;
    for(const Note* n: m_pattern->notes())
        keys |= 1 << (n->key() % KeysPerOctave);
    // qInfo("PianoRoll::guessScale keys=%ld", keys);

    for(int s = 1; s < m_scaleModel.size(); s++)
    {
        if(s == 19)
            continue;  // Chromatic

        const InstrumentFunctionNoteStacking::Chord* chord
                = &InstrumentFunctionNoteStacking::ChordTable::getInstance()
                           .getScaleByName(m_scaleModel.itemText(s));
        for(int root = 0; root < KeysPerOctave; root++)
        {
            uint64_t marks = 0;
            for(int k = 0; k <= KeysPerOctave; k++)  // chord->last()
            {
                int i = (k + KeysPerOctave - root) % KeysPerOctave;
                if(chord->hasSemiTone(i))
                    marks |= 1 << k;
            }
            // qInfo("PianoRoll::guessScale marks=%ld root=%d on %s", marks,
            //      root, qPrintable(m_scaleModel.itemText(s)));

            if((keys & marks) == keys)
            {
                m_rootModel.setValue(root);
                m_scaleModel.setValue(s);
                update();
                return;
            }
        }
    }

    m_rootModel.setValue(0);
    m_scaleModel.setValue(0);
    update();
}

void PianoRoll::upChord()
{
    bool   useAllNotes = !isSelection();
    tick_t start       = m_pattern->unitLength();
    tick_t end         = 0;
    Notes  notes;
    for(Note* note: m_pattern->notes())
    {
        // if none are selected, move all notes, otherwise
        // only move selected notes
        if(useAllNotes || note->selected())
        {
            notes << note;
            if(start > note->pos())
                start = note->pos();
            if(end < note->endPos())
                end = note->endPos();
        }
    }
    while(start < end)
    {
        Notes chord;
        int   lowkey  = NumKeys;
        Note* lownote = nullptr;
        for(Note* note: notes)
            if(start >= note->pos() && start < note->endPos())
            {
                chord.append(note);
                if(note->key() < lowkey)
                {
                    lowkey  = note->key();
                    lownote = note;
                }
            }
        if(lownote != nullptr)
            lownote->setKey(qBound(0, lownote->key() + 12, NumKeys - 1));
        for(Note* note: chord)
            notes.removeOne(note);
        start++;
    }

    m_pattern->rearrangeAllNotes();
    m_pattern->emit dataChanged();

    // we modified the song
    update();
    gui->songWindow()->update();
}

void PianoRoll::downChord()
{
    bool   useAllNotes = !isSelection();
    tick_t start       = m_pattern->unitLength();
    tick_t end         = 0;
    Notes  notes;
    for(Note* note: m_pattern->notes())
    {
        // if none are selected, move all notes, otherwise
        // only move selected notes
        if(useAllNotes || note->selected())
        {
            notes << note;
            if(start > note->pos())
                start = note->pos();
            if(end < note->endPos())
                end = note->endPos();
        }
    }
    while(start < end)
    {
        Notes chord;
        int   highkey  = -1;
        Note* highnote = nullptr;
        for(Note* note: notes)
            if(start >= note->pos() && start < note->endPos())
            {
                chord.append(note);
                if(note->key() > highkey)
                {
                    highkey  = note->key();
                    highnote = note;
                }
            }
        if(highnote != nullptr)
            highnote->setKey(qBound(0, highnote->key() - 12, NumKeys - 1));
        for(Note* note: chord)
            notes.removeOne(note);
        start++;
    }

    m_pattern->rearrangeAllNotes();
    m_pattern->emit dataChanged();

    // we modified the song
    update();
    gui->songWindow()->update();
}

PianoRoll::~PianoRoll()
{
}

void PianoRoll::setCurrentPattern(Pattern* pattern)
{
    const Pattern* old = currentPattern();
    if(old == pattern)
        return;

    // Disconnect our old pattern
    if(old != nullptr)
    {
        old->disconnect(this);
        old->instrumentTrack()->disconnect(this);
    }

    // force the song-editor to stop playing if it played pattern before
    if(Engine::getSong()->isPlaying()
       && Engine::getSong()->playMode() == Song::Mode_PlayPattern)
    {
        Engine::getSong()->playPattern(nullptr);
    }

    // set new data
    m_pattern         = pattern;
    m_currentPosition = 0;
    m_currentNote     = nullptr;
    m_startKey        = INITIAL_START_KEY;

    if(m_pattern == nullptr)
    {
        // resizeEvent( nullptr );
        update();
        emit currentPatternChanged();
        return;
    }

    m_leftRightScroll->setValue(0);

    // determine the central key so that we can scroll to it
    int central_key = 0;
    int total_notes = 0;
    for(const Note* note: m_pattern->notes())
    {
        if(note->length() > 0)
        {
            central_key += note->key();
            ++total_notes;
        }
    }

    if(total_notes > 0)
    {
        central_key = central_key / total_notes
                      - (NumKeys - m_totalKeysToScroll)
                                / 2;  // KeysPerOctave * NumOctaves
        m_startKey = tLimit(central_key, 0,
                            NumKeys - 1);  // NumOctaves * KeysPerOctave
    }

    // resizeEvent() does the rest for us (scrolling, range-checking
    // of start-notes and so on)
    resizeEvent(nullptr);

    // make sure to always get informed about the pattern being destroyed
    connect(m_pattern, SIGNAL(destroyedPattern(Pattern*)), this,
            SLOT(hidePattern(Pattern*)));

    connect(m_pattern->instrumentTrack(), SIGNAL(midiNoteOn(const Note&)),
            this, SLOT(startRecordNote(const Note&)));
    connect(m_pattern->instrumentTrack(), SIGNAL(midiNoteOff(const Note&)),
            this, SLOT(finishRecordNote(const Note&)));
    connect(m_pattern->instrumentTrack()->pianoModel(), SIGNAL(dataChanged()),
            this, SLOT(update()));

    update();
    emit currentPatternChanged();
}

void PianoRoll::setGhostPattern(Pattern* _pattern)
{
    if(m_ghostPattern != nullptr)
        m_ghostPattern->disconnect(this);

    m_ghostPattern = _pattern;
    if(_pattern != nullptr)
    {
        // make sure to always get informed about the pattern being destroyed
        connect(m_ghostPattern, SIGNAL(destroyedPattern(Pattern*)), this,
                SLOT(clearGhostPattern()));
        emit ghostPatternSet(true);
    }
}

void PianoRoll::hidePattern(Pattern* _pattern)
{
    if(m_pattern == _pattern)
    {
        setCurrentPattern(nullptr);
    }
}

void PianoRoll::clearGhostPattern()
{
    setGhostPattern(nullptr);
    emit ghostPatternSet(false);
    update();
}

void PianoRoll::selectRegionFromPixels(int xStart, int xEnd)
{

    xStart -= WHITE_KEY_WIDTH;
    xEnd -= WHITE_KEY_WIDTH;

    // select an area of notes
    int pos_ticks
            = xStart * MidiTime::ticksPerTact() / m_ppt + m_currentPosition;
    int key_num       = 0;
    m_selectStartTick = pos_ticks;
    m_selectedTick    = 0;
    m_selectStartKey  = key_num;
    m_selectedKeys    = 1;
    // change size of selection

    // get tick in which the cursor is posated
    pos_ticks = xEnd * MidiTime::ticksPerTact() / m_ppt + m_currentPosition;
    key_num   = 120;

    m_selectedTick = pos_ticks - m_selectStartTick;
    if((int)m_selectStartTick + m_selectedTick < 0)
    {
        m_selectedTick = -static_cast<int>(m_selectStartTick);
    }
    m_selectedKeys = key_num - m_selectStartKey;
    if(key_num <= m_selectStartKey)
    {
        --m_selectedKeys;
    }

    computeSelectedNotes(false);
}

/** \brief qproperty access implementation */

QColor PianoRoll::barLineColor() const
{
    return m_barLineColor;
}

void PianoRoll::setBarLineColor(const QColor& c)
{
    m_barLineColor = c;
}

QColor PianoRoll::beatLineColor() const
{
    return m_beatLineColor;
}

void PianoRoll::setBeatLineColor(const QColor& c)
{
    m_beatLineColor = c;
}

QColor PianoRoll::lineColor() const
{
    return m_lineColor;
}

void PianoRoll::setLineColor(const QColor& c)
{
    m_lineColor = c;
}

QColor PianoRoll::noteModeColor() const
{
    return m_noteModeColor;
}

void PianoRoll::setNoteModeColor(const QColor& c)
{
    m_noteModeColor = c;
}

QColor PianoRoll::noteColor() const
{
    return m_noteColor;
}

void PianoRoll::setNoteColor(const QColor& c)
{
    m_noteColor = c;
}

QColor PianoRoll::ghostNoteColor() const
{
    return m_ghostNoteColor;
}

void PianoRoll::setGhostNoteColor(const QColor& c)
{
    m_ghostNoteColor = c;
}
QColor PianoRoll::barColor() const
{
    return m_barColor;
}

void PianoRoll::setBarColor(const QColor& c)
{
    m_barColor = c;
}

QColor PianoRoll::selectedNoteColor() const
{
    return m_selectedNoteColor;
}

void PianoRoll::setSelectedNoteColor(const QColor& c)
{
    m_selectedNoteColor = c;
}

QColor PianoRoll::textColor() const
{
    return m_textColor;
}

void PianoRoll::setTextColor(const QColor& c)
{
    m_textColor = c;
}

QColor PianoRoll::textColorLight() const
{
    return m_textColorLight;
}

void PianoRoll::setTextColorLight(const QColor& c)
{
    m_textColorLight = c;
}

QColor PianoRoll::textShadow() const
{
    return m_textShadow;
}

void PianoRoll::setTextShadow(const QColor& c)
{
    m_textShadow = c;
}

QColor PianoRoll::markedSemitoneColor() const
{
    return m_markedSemitoneColor;
}

void PianoRoll::setMarkedSemitoneColor(const QColor& c)
{
    m_markedSemitoneColor = c;
}

int PianoRoll::noteOpacity() const
{
    return m_noteOpacity;
}

void PianoRoll::setNoteOpacity(const int i)
{
    m_noteOpacity = i;
}

int PianoRoll::ghostNoteOpacity() const
{
    return m_ghostNoteOpacity;
}

void PianoRoll::setGhostNoteOpacity(const int i)
{
    m_ghostNoteOpacity = i;
}

bool PianoRoll::noteBorders() const
{
    return m_noteBorders;
}

void PianoRoll::setNoteBorders(const bool b)
{
    m_noteBorders = b;
}

bool PianoRoll::ghostNoteBorders() const
{
    return m_ghostNoteBorders;
}

void PianoRoll::setGhostNoteBorders(const bool b)
{
    m_ghostNoteBorders = b;
}
QColor PianoRoll::backgroundShade() const
{
    return m_backgroundShade;
}

void PianoRoll::setBackgroundShade(const QColor& c)
{
    m_backgroundShade = c;
}

void PianoRoll::drawNoteRect(QPainter&     p,
                             int           x,
                             int           y,
                             int           width,
                             const Note*   n,
                             const QColor& noteCol,
                             const QColor& selCol,
                             const int     noteOpc,
                             const bool    borders,
                             const bool    drawNoteNames)
{
    ++x;
    ++y;
    width -= 2;

    if(width <= 0)
    {
        width = 2;
    }

    // Volume
    float const volumeRange = static_cast<float>(MaxVolume - MinVolume);
    float const volumeSpan  = static_cast<float>(n->getVolume() - MinVolume);
    float const volumeRatio = volumeSpan / volumeRange;
    int volVal = qMin(255, 100 + static_cast<int>(volumeRatio * 155.0f));

    // Panning
    float const panningRange = static_cast<float>(PanningRight - PanningLeft);
    float const leftPanSpan
            = static_cast<float>(PanningRight - n->getPanning());
    float const rightPanSpan
            = static_cast<float>(n->getPanning() - PanningLeft);

    float leftPercent = qMin<float>(1.0f, leftPanSpan / panningRange * 2.0f);
    float rightPercent
            = qMin<float>(1.0f, rightPanSpan / panningRange * 2.0f);

    QColor col = QColor(noteCol);
    QPen   pen;

    if(n->selected())
    {
        col = QColor(selCol);
    }

    const int borderWidth = borders ? 1 : 0;

    const int noteHeight = KEY_LINE_HEIGHT - 1 - borderWidth;
    int       noteWidth  = width + 1 - borderWidth;

    // adjust note to make it a bit faded if it has a lower volume
    // in stereo using gradients
    QColor lcol = QColor::fromHsv(col.hue(), col.saturation(),
                                  static_cast<int>(volVal * leftPercent),
                                  noteOpc);
    QColor rcol = QColor::fromHsv(col.hue(), col.saturation(),
                                  static_cast<int>(volVal * rightPercent),
                                  noteOpc);

    QLinearGradient gradient(x, y, x, y + noteHeight);
    gradient.setColorAt(0, rcol);
    gradient.setColorAt(1, lcol);
    p.setBrush(gradient);
    p.setPen(borders ? col : Qt::NoPen);
    p.drawRect(x, y, noteWidth, noteHeight);

    int ym = y;
    p.setPen(col);
    if(n->legato())
    {
        ym -= 4;
        p.drawRect(x - 8, ym, 16, 2);
    }
    if(n->staccato())
    {
        ym -= 6;
        p.drawEllipse(x + 2, ym, 4, 4);
    }
    if(n->marcato())
    {
        ym -= 8;
        p.drawLine(x + 1, ym, x + 7, ym + 3);
        p.drawLine(x + 7, ym + 4, x + 1, ym + 6);
    }

    // Draw note key text
    if(drawNoteNames)
    {
        p.save();
        int const noteTextHeight = static_cast<int>(noteHeight * 0.8);
        if(noteTextHeight > 6)
        {
            QString noteKeyString = Note::findKeyName(n->key());
            // getNoteString(n->key());

            QFont noteFont(p.font());
            noteFont.setPixelSize(noteTextHeight);
            QFontMetrics fontMetrics(noteFont);
            QSize        textSize
                    = fontMetrics.size(Qt::TextSingleLine, noteKeyString);

            const int distanceToBorder = 2;
            const int xOffset          = borderWidth + distanceToBorder;
            const int yOffset          = (noteHeight + noteTextHeight) / 2;

            if(textSize.width() < noteWidth - xOffset)
            {
                p.setPen(Qt::white);  // noteTextColor);
                p.setFont(noteFont);
                p.drawText(x + xOffset, y + yOffset, noteKeyString);
            }
        }
        p.restore();
    }

    // draw the note endmark, to hint the user to resize
    p.setBrush(col);
    if(width > 2)
    {
        const int endmarkWidth = 3 - borderWidth;
        p.drawRect(x + noteWidth - endmarkWidth, y, endmarkWidth, noteHeight);
    }
}

void PianoRoll::drawDetuningInfo(QPainter&   _p,
                                 const Note* _n,
                                 int         _x,
                                 int         _y) const
{
    int middle_y = _y + KEY_LINE_HEIGHT / 2;
    _p.setPen(noteColor());

    int old_x = 0;
    int old_y = 0;

    timeMap& map = _n->detuning()->automationPattern()->getTimeMap();
    for(timeMap::ConstIterator it = map.begin(); it != map.end(); ++it)
    {
        int pos_ticks = it.key();
        int pos_x     = _x + pos_ticks * m_ppt / MidiTime::ticksPerTact();

        const float level = it.value();

        int pos_y = middle_y - level * KEY_LINE_HEIGHT;

        if(old_x != 0 && old_y != 0)
        {
            switch(_n->detuning()->automationPattern()->progressionType())
            {
                case AutomationPattern::DiscreteProgression:
                    _p.drawLine(old_x, old_y, pos_x, old_y);
                    _p.drawLine(pos_x, old_y, pos_x, pos_y);
                    break;
                case AutomationPattern::CubicHermiteProgression: /* TODO */
                case AutomationPattern::ParabolicProgression:    /* TODO */
                case AutomationPattern::LinearProgression:
                    _p.drawLine(old_x, old_y, pos_x, pos_y);
                    break;
            }
        }

        _p.drawLine(pos_x - 1, pos_y, pos_x + 1, pos_y);
        _p.drawLine(pos_x, pos_y - 1, pos_x, pos_y + 1);

        old_x = pos_x;
        old_y = pos_y;
    }
}

void PianoRoll::removeSelection()
{
    m_selectStartTick = 0;
    m_selectedTick    = 0;
    m_selectStartKey  = 0;
    m_selectedKeys    = 0;
}

void PianoRoll::clearSelectedNotes()
{
    if(m_pattern != nullptr)
    {
        for(Note* note: m_pattern->notes())
        {
            note->setSelected(false);
        }
    }
}

void PianoRoll::shiftSemiTone(int amount)  // shift notes by amount semitones
{
    if(m_pattern == nullptr)
        return;
    bool useAllNotes = !isSelection();
    for(Note* note: m_pattern->notes())
    {
        // if none are selected, move all notes, otherwise
        // only move selected notes
        if(useAllNotes || note->selected())
        {
            note->setKey(note->key() + amount);
        }
    }

    m_pattern->rearrangeAllNotes();
    m_pattern->emit dataChanged();

    // we modified the song
    update();
    gui->songWindow()->update();
}

void PianoRoll::shiftPos(int amount)  // shift notes pos by amount
{
    if(m_pattern == nullptr)
        return;
    bool useAllNotes = !isSelection();

    bool first = true;
    for(Note* note: m_pattern->notes())
    {
        // if none are selected, move all notes, otherwise
        // only move selected notes
        if(note->selected() || (useAllNotes && note->length() > 0))
        {
            // don't let notes go to out of bounds
            if(first)
            {
                m_moveBoundaryLeft = note->pos();
                if(m_moveBoundaryLeft + amount < 0)
                {
                    amount += 0 - (amount + m_moveBoundaryLeft);
                }
                first = false;
            }
            note->setPos(note->pos() + amount);
        }
    }

    m_pattern->rearrangeAllNotes();
    m_pattern->updateLength();
    m_pattern->emit dataChanged();

    // we modified the song
    update();
    gui->songWindow()->update();
}

bool PianoRoll::isSelection() const  // are any notes selected?
{
    for(const Note* note: m_pattern->notes())
    {
        if(note->selected())
        {
            return true;
        }
    }

    return false;
}

int PianoRoll::selectionCount() const  // how many notes are selected?
{
    int sum = 0;

    for(const Note* note: m_pattern->notes())
    {
        if(note->selected())
        {
            ++sum;
        }
    }

    return sum;
}

void PianoRoll::keyPressEvent(QKeyEvent* ke)
{
    if(m_pattern != nullptr && ke->modifiers() == Qt::NoModifier)
    {
        const int key_num = PeripheralView::getKeyFromKeyEvent(ke)
                            + (DefaultOctave - 1) * KeysPerOctave;

        if(!ke->isAutoRepeat() && key_num > -1)
        {
            // m_pattern->instrumentTrack()->pianoModel()->handleKeyPress(
            //    key_num, MidiDefaultVelocity);
            playTestKey(key_num, MidiDefaultVelocity, 0);
            ke->accept();
        }
    }

    switch(ke->key())
    {
        case Qt::Key_Up:
        case Qt::Key_Down:
        {
            int direction = (ke->key() == Qt::Key_Up ? +1 : -1);
            if((ke->modifiers() & Qt::ControlModifier)
               && m_action == ActionNone)
            {
                // shift selection up an octave
                // if nothing selected, shift _everything_
                if(m_pattern != nullptr)
                {
                    shiftSemiTone(12 * direction);
                }
            }
            else if((ke->modifiers() & Qt::ShiftModifier)
                    && m_action == ActionNone)
            {
                // Move selected notes up by one semitone
                if(m_pattern != nullptr)
                {
                    shiftSemiTone(1 * direction);
                }
            }
            else
            {
                // scroll
                m_topBottomScroll->setValue(m_topBottomScroll->value()
                                            - cm_scrollAmtVert * direction);

                // if they are moving notes around or resizing,
                // recalculate the note/resize position
                if(m_action == ActionMoveNote || m_action == ActionResizeNote)
                {
                    dragNotes(m_lastMouseX, m_lastMouseY,
                              ke->modifiers() & Qt::AltModifier,
                              ke->modifiers() & Qt::ShiftModifier,
                              ke->modifiers() & Qt::ControlModifier);
                }
            }
            ke->accept();
            break;
        }

        case Qt::Key_Right:
        case Qt::Key_Left:
        {
            int direction = (ke->key() == Qt::Key_Right ? +1 : -1);
            if(ke->modifiers() & Qt::ControlModifier
               && m_action == ActionNone)
            {
                // Move selected notes by one bar to the left
                if(m_pattern != nullptr)
                {
                    shiftPos(direction * MidiTime::ticksPerTact());
                }
            }
            else if(ke->modifiers() & Qt::ShiftModifier
                    && m_action == ActionNone)
            {
                // move notes
                if(m_pattern != nullptr)
                {
                    bool quantized = !(ke->modifiers() & Qt::AltModifier);
                    int  amt       = quantized ? quantization() : 1;
                    shiftPos(direction * amt);
                }
            }
            else if(ke->modifiers() & Qt::AltModifier)
            {
                // switch to editing a pattern adjacent to this one in the
                // song editor
                if(m_pattern != nullptr)
                {
                    Pattern* p = direction > 0 ? m_pattern->nextPattern()
                                               : m_pattern->previousPattern();
                    if(p != nullptr)
                    {
                        setCurrentPattern(p);
                    }
                }
            }
            else
            {
                // scroll
                m_leftRightScroll->setValue(m_leftRightScroll->value()
                                            + direction * cm_scrollAmtHoriz);

                // if they are moving notes around or resizing,
                // recalculate the note/resize position
                if(m_action == ActionMoveNote || m_action == ActionResizeNote)
                {
                    dragNotes(m_lastMouseX, m_lastMouseY,
                              ke->modifiers() & Qt::AltModifier,
                              ke->modifiers() & Qt::ShiftModifier,
                              ke->modifiers() & Qt::ControlModifier);
                }
            }
            ke->accept();
            break;
        }

        case Qt::Key_A:
            if(ke->modifiers() & Qt::ControlModifier)
            {
                ke->accept();
                if(ke->modifiers() & Qt::ShiftModifier)
                {
                    // Ctrl+Shift+A = deselect all notes
                    clearSelectedNotes();
                }
                else
                {
                    // Ctrl+A = select all notes
                    selectAll();
                }
                update();
            }
            break;

        case Qt::Key_Escape:
            // Same as Ctrl+Shift+A
            clearSelectedNotes();
            break;

        case Qt::Key_Delete:
            deleteSelectedNotes();
            ke->accept();
            break;

        case Qt::Key_Home:
            m_timeLine->pos().setTicks(0);
            m_timeLine->updatePosition();
            ke->accept();
            break;

        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        {
            int len = 1 + ke->key() - Qt::Key_0;

            if(ke->modifiers() & Qt::AltModifier)
            {
                if(len == 10)
                    len = 0;
                if(len > 0)
                    len += 2;
                m_quantizeModel.setValue(len);
                ke->accept();
            }
            else  // if(ke->modifiers()
                  // & (Qt::ControlModifier | Qt::KeypadModifier))
            {
                if(len == 10)
                    len = 0;
                if(len > 0)
                    len += 2;
                m_noteLenModel.setValue(len);
                ke->accept();
            }
            break;
        }

        case Qt::Key_Control:
            // Enter selection mode if:
            // -> this window is active
            // -> shift is not pressed
            // (<S-C-drag> is shortcut for sticky note resize)
            if(!(ke->modifiers() & Qt::ShiftModifier) && isActiveWindow())
            {
                m_ctrlMode = m_editMode;
                m_editMode = ModeSelect;
                QApplication::changeOverrideCursor(Qt::ArrowCursor);
                ke->accept();
            }
            break;
        default:
            break;
    }

    update();
}

void PianoRoll::keyReleaseEvent(QKeyEvent* ke)
{
    if(m_pattern != nullptr && ke->modifiers() == Qt::NoModifier)
    {
        const int key_num = PeripheralView::getKeyFromKeyEvent(ke)
                            + (DefaultOctave - 1) * KeysPerOctave;

        if(!ke->isAutoRepeat() && key_num > -1)
        {
            stopTestKey();  // key_num);
            // m_pattern->instrumentTrack()->pianoModel()->handleKeyRelease(
            //  key_num);
            ke->accept();
        }
    }

    switch(ke->key())
    {
        case Qt::Key_Control:
            computeSelectedNotes(ke->modifiers() & Qt::ShiftModifier);
            m_editMode = m_ctrlMode;
            update();
            break;

        // update after undo/redo
        case Qt::Key_Z:
        case Qt::Key_R:
            if(m_pattern != nullptr && ke->modifiers() == Qt::ControlModifier)
            {
                update();
            }
            break;
    }

    update();
}

void PianoRoll::leaveEvent(QEvent* e)
{
    while(QApplication::overrideCursor() != nullptr)
    {
        QApplication::restoreOverrideCursor();
    }

    QWidget::leaveEvent(e);
    s_textFloat->hide();
}

int PianoRoll::noteEditTop() const
{
    return height() - PR_BOTTOM_MARGIN - m_notesEditHeight
           + NOTE_EDIT_RESIZE_BAR;
}

int PianoRoll::noteEditBottom() const
{
    return height() - PR_BOTTOM_MARGIN;
}

int PianoRoll::noteEditRight() const
{
    return width() - PR_RIGHT_MARGIN;
}

int PianoRoll::noteEditLeft() const
{
    return WHITE_KEY_WIDTH;
}

int PianoRoll::keyAreaTop() const
{
    return PR_TOP_MARGIN;
}

int PianoRoll::keyAreaBottom() const
{
    return height() - PR_BOTTOM_MARGIN - m_notesEditHeight;
}

void PianoRoll::mousePressEvent(QMouseEvent* me)
{
    m_startedWithShift = me->modifiers() & Qt::ShiftModifier;

    if(m_pattern == nullptr)
        return;
    if(m_editMode == ModeEditDetuning && noteUnderMouse())
    {
        static QPointer<AutomationPattern> detuningPattern = nullptr;
        if(detuningPattern.data() != nullptr)
        {
            detuningPattern->disconnect(this);
        }
        Note* n = noteUnderMouse();
        if(n->detuning() == nullptr)
        {
            n->createDetuning();
        }
        detuningPattern = n->detuning()->automationPattern();
        connect(detuningPattern.data(), SIGNAL(dataChanged()), this,
                SLOT(update()));
        gui->automationWindow()->open(detuningPattern);
        return;
    }

    // if holding control, go to selection mode unless shift is also pressed
    if(me->modifiers() & Qt::ControlModifier && m_editMode != ModeSelect)
    {
        m_ctrlMode = m_editMode;
        m_editMode = ModeSelect;
        QApplication::changeOverrideCursor(QCursor(Qt::ArrowCursor));
        update();
    }

    // keep track of the point where the user clicked down
    if(me->button() == Qt::LeftButton)
    {
        m_moveStartX = me->x();
        m_moveStartY = me->y();
    }

    if(me->y() > keyAreaBottom() && me->y() < noteEditTop())
    {
        // resizing the note edit area
        m_action             = ActionResizeNoteEditArea;
        m_oldNotesEditHeight = m_notesEditHeight;
        return;
    }

    if(me->y() > PR_TOP_MARGIN)
    {
        bool edit_note = (me->y() > noteEditTop());

        int key_num = getKey(me->y());

        int x = me->x();

        if(x > WHITE_KEY_WIDTH)
        {
            // set, move or resize note

            x -= WHITE_KEY_WIDTH;

            // get tick in which the user clicked
            int pos_ticks = x * MidiTime::ticksPerTact() / m_ppt
                            + m_currentPosition;

            // get note-vector of current pattern
            const Notes& notes = m_pattern->notes();

            // will be our iterator in the following loop
            Notes::ConstIterator it = notes.begin() + notes.size() - 1;

            // loop through whole note-vector
            for(int i = 0; i < notes.size(); ++i)
            {
                Note*    note = *it;
                MidiTime len  = note->length();
                if(len < 0)
                {
                    len = 4;
                }
                // and check whether the user clicked on an
                // existing note or an edit-line
                if(pos_ticks >= note->pos() && len > 0
                   && ((!edit_note && pos_ticks <= note->pos() + len
                        && note->key() == key_num)
                       || (edit_note
                           && pos_ticks
                                      <= note->pos()
                                                 + NOTE_EDIT_LINE_WIDTH
                                                           * MidiTime::
                                                                     ticksPerTact()
                                                           / m_ppt)))
                {
                    break;
                }
                --it;
            }

            // first check whether the user clicked in note-edit-
            // area
            if(edit_note)
            {
                m_pattern->addJournalCheckPoint();
                // scribble note edit changes
                mouseMoveEvent(me);
                return;
            }
            // left button??
            else if(me->button() == Qt::LeftButton && m_editMode == ModeDraw)
            {
                // whether this action creates new note(s) or not
                bool is_new_note = false;

                Note* created_new_note = nullptr;
                // did it reach end of vector because
                // there's no note??
                if(it == notes.begin() - 1)
                {
                    is_new_note = true;
                    m_pattern->addJournalCheckPoint();

                    // then set new note

                    // clear selection and select this new note
                    clearSelectedNotes();

                    // +32 to quanitize the note correctly when placing notes
                    // with the mouse.  We do this here instead of in
                    // note.quantized because live notes should still be
                    // quantized at the half.
                    MidiTime note_pos(pos_ticks - (quantization() / 2));
                    MidiTime note_len(newNoteLen());

                    Note new_note(note_len, note_pos, key_num);
                    new_note.setSelected(true);
                    new_note.setVolume(m_lastNoteVolume);
                    new_note.setPanning(m_lastNotePanning);
                    new_note.setLegato(m_lastNoteLegato);
                    new_note.setMarcato(m_lastNoteMarcato);
                    new_note.setStaccato(m_lastNoteStaccato);
                    created_new_note = m_pattern->addNote(new_note);

                    const InstrumentFunctionNoteStacking::Chord& chord
                            = InstrumentFunctionNoteStacking::ChordTable::
                                      getInstance()
                                              .getChordByName(
                                                      m_chordModel
                                                              .currentText());

                    if(!chord.isEmpty())
                    {
                        // if a chord is selected, create following notes in
                        // chord or arpeggio mode
                        const bool arpeggio
                                = me->modifiers() & Qt::ShiftModifier;
                        for(int i = 1; i < chord.size(); i++)
                        {
                            if(arpeggio)
                            {
                                note_pos += note_len;
                            }
                            Note new_note(note_len, note_pos,
                                          key_num + chord[i]);
                            new_note.setSelected(true);
                            new_note.setVolume(m_lastNoteVolume);
                            new_note.setPanning(m_lastNotePanning);
                            new_note.setLegato(m_lastNoteLegato);
                            new_note.setMarcato(m_lastNoteMarcato);
                            new_note.setStaccato(m_lastNoteStaccato);
                            m_pattern->addNote(new_note);
                        }
                    }

                    // reset it so that it can be used for
                    // ops (move, resize) after this
                    // code-block
                    it = notes.begin();
                    while(it != notes.end() && *it != created_new_note)
                    {
                        ++it;
                    }
                }

                Note* current_note = *it;
                m_currentNote      = current_note;
                m_lastNoteVolume   = current_note->getVolume();
                m_lastNotePanning  = current_note->getPanning();
                m_lastNoteLegato   = current_note->legato();
                m_lastNoteMarcato  = current_note->marcato();
                m_lastNoteStaccato = current_note->staccato();
                m_lenOfNewNotes    = current_note->length();

                // remember which key and tick we started with
                m_mouseDownKey  = m_startKey;
                m_mouseDownTick = m_currentPosition;

                bool first = true;
                for(it = notes.begin(); it != notes.end(); ++it)
                {
                    Note* note = *it;

                    // remember note starting positions
                    note->setOldKey(note->key());
                    note->setOldPos(note->pos());
                    note->setOldLength(note->length());

                    if(note->selected())
                    {

                        // figure out the bounding box of all the selected
                        // notes
                        if(first)
                        {
                            m_moveBoundaryLeft   = note->pos().getTicks();
                            m_moveBoundaryRight  = note->endPos();
                            m_moveBoundaryBottom = note->key();
                            m_moveBoundaryTop    = note->key();

                            first = false;
                        }
                        else
                        {
                            m_moveBoundaryLeft
                                    = qMin(note->pos().getTicks(),
                                           (tick_t)m_moveBoundaryLeft);
                            m_moveBoundaryRight = qMax((int)note->endPos(),
                                                       m_moveBoundaryRight);
                            m_moveBoundaryBottom
                                    = qMin(note->key(), m_moveBoundaryBottom);
                            m_moveBoundaryTop
                                    = qMax(note->key(), m_moveBoundaryTop);
                        }
                    }
                }

                // if clicked on an unselected note, remove selection
                // and select that new note
                if(!m_currentNote->selected())
                {
                    clearSelectedNotes();
                    m_currentNote->setSelected(true);
                    m_moveBoundaryLeft   = m_currentNote->pos().getTicks();
                    m_moveBoundaryRight  = m_currentNote->endPos();
                    m_moveBoundaryBottom = m_currentNote->key();
                    m_moveBoundaryTop    = m_currentNote->key();
                }

                // clicked at the "tail" of the note?
                if(pos_ticks * m_ppt / MidiTime::ticksPerTact()
                           > m_currentNote->endPos() * m_ppt
                                             / MidiTime::ticksPerTact()
                                     - RESIZE_AREA_WIDTH
                   && m_currentNote->length() > 0)
                {
                    m_pattern->addJournalCheckPoint();
                    // then resize the note
                    m_action = ActionResizeNote;

                    // set resize-cursor
                    QCursor c(Qt::SizeHorCursor);
                    QApplication::setOverrideCursor(c);
                }
                else
                {
                    if(!created_new_note)
                    {
                        m_pattern->addJournalCheckPoint();
                    }

                    // otherwise move it
                    m_action = ActionMoveNote;

                    // set move-cursor
                    QCursor c(Qt::SizeAllCursor);
                    QApplication::setOverrideCursor(c);

                    // if they're holding shift, copy all selected notes
                    if(!is_new_note && me->modifiers() & Qt::ShiftModifier)
                    {
                        // vector to hold new notes until we're through the
                        // loop
                        QVector<Note> newNotes;
                        for(Note* const& note: notes)
                        {
                            if(note->selected())
                            {
                                // copy this note
                                Note noteCopy(*note);
                                newNotes.push_back(noteCopy);
                            }
                            ++it;
                        }

                        if(newNotes.size() != 0)
                        {
                            // put notes from vector into piano roll
                            for(int i = 0; i < newNotes.size(); ++i)
                            {
                                Note* newNote = m_pattern->addNote(
                                        newNotes[i], false);
                                newNote->setSelected(false);
                            }

                            // added new notes, so must update engine, song,
                            // etc
                            Engine::getSong()->setModified();
                            update();
                            gui->songWindow()->update();
                        }
                    }

                    // play the note
                    playTestNote(m_currentNote);
                }

                Engine::getSong()->setModified();
            }
            else if((me->buttons() == Qt::RightButton
                     && m_editMode == ModeDraw)
                    || m_editMode == ModeErase)
            {
                // erase single note
                m_mouseDownRight = true;
                if(it != notes.begin() - 1)
                {
                    m_pattern->addJournalCheckPoint();
                    m_pattern->removeNote(*it);
                    Engine::getSong()->setModified();
                }
            }
            else if(me->button() == Qt::LeftButton
                    && m_editMode == ModeSelect)
            {
                // select an area of notes

                m_selectStartTick = pos_ticks;
                m_selectedTick    = 0;
                m_selectStartKey  = key_num;
                m_selectedKeys    = 1;
                m_action          = ActionSelectNotes;

                // call mousemove to fix glitch where selection
                // appears in wrong spot on mousedown
                mouseMoveEvent(me);
            }

            update();
        }
        else if(me->y() < keyAreaBottom())
        {
            // reference to last key needed for both
            // right click (used for copy all keys on note)
            // and for playing the key when left-clicked
            m_lastKey = key_num;

            // clicked on keyboard on the left
            if(me->buttons() == Qt::RightButton)
            {
                // right click, tone marker contextual menu
                m_semiToneMarkerMenu->popup(
                        mapToGlobal(QPoint(me->x(), me->y())));
            }
            else
            {
                // left click - play the note
                int v = ((float)x) / ((float)WHITE_KEY_WIDTH)
                        * MidiDefaultVelocity;
                stopTestNotes();
                stopTestKey();
                // m_pattern->instrumentTrack()->pianoModel()->handleKeyPress(
                playTestKey(key_num, v, 0);  // OK
            }
        }
        else
        {
            if(me->buttons() == Qt::LeftButton)
            {
                // clicked in the box below the keys to the left of note edit
                // area
                m_noteEditMode = (NoteEditMode)(((int)m_noteEditMode) + 1);
                if(m_noteEditMode == NoteEditCount)
                {
                    m_noteEditMode = (NoteEditMode)0;
                }
                update();  // repaint();
            }
            else if(me->buttons() == Qt::RightButton)
            {
                // pop menu asking which one they want to edit
                m_noteEditMenu->popup(mapToGlobal(QPoint(me->x(), me->y())));
            }
        }
    }
}

void PianoRoll::mouseDoubleClickEvent(QMouseEvent* me)
{
    if(m_pattern == nullptr)
        return;
    // if they clicked in the note edit area, enter value for the volume bar
    if(me->x() > noteEditLeft() && me->x() < noteEditRight()
       && me->y() > noteEditTop() && me->y() < noteEditBottom())
    {
        // get values for going through notes
        int       pixel_range = 4;
        int       x           = me->x() - WHITE_KEY_WIDTH;
        const int ticks_start
                = (x - pixel_range / 2) * MidiTime::ticksPerTact() / m_ppt
                  + m_currentPosition;
        const int ticks_end
                = (x + pixel_range / 2) * MidiTime::ticksPerTact() / m_ppt
                  + m_currentPosition;
        const int ticks_middle
                = x * MidiTime::ticksPerTact() / m_ppt + m_currentPosition;

        // go through notes to figure out which one we want to change
        bool  altPressed = me->modifiers() & Qt::AltModifier;
        Notes nv;
        for(Note* i: m_pattern->notes())
        {
            if(i->withinRange(ticks_start, ticks_end)
               || (i->selected() && !altPressed))
            {
                nv += i;
            }
        }
        // make sure we're on a note
        if(nv.size() > 0)
        {
            const Note* closest      = nullptr;
            int         closest_dist = 9999999;
            // if we caught multiple notes, find the closest...
            if(nv.size() > 1)
            {
                for(const Note* i: nv)
                {
                    const int dist = qAbs(i->pos().getTicks() - ticks_middle);
                    if(dist < closest_dist)
                    {
                        closest      = i;
                        closest_dist = dist;
                    }
                }
                // ... then remove all notes from the vector that aren't on
                // the same exact time
                Notes::Iterator it = nv.begin();
                while(it != nv.end())
                {
                    const Note* note = *it;
                    if(note->pos().getTicks() != closest->pos().getTicks())
                    {
                        it = nv.erase(it);
                    }
                    else
                    {
                        it++;
                    }
                }
            }
            enterValue(nv);
        }
    }
}

void PianoRoll::stopTestNotes()
{
    if(m_pattern != nullptr)
        for(Note* n: m_pattern->notes())
            stopTestNote(n);

    // m_pattern->instrumentTrack()->pianoModel()->reset();
}

void PianoRoll::stopTestNote(Note* n)
{
    if(n->isPlaying())
    {
        qInfo("PianoRoll::stopTestNote n=%d", n->key());
        n->setIsPlaying(false);
        m_pattern->instrumentTrack()->pianoModel()->handleKeyRelease(
                n->key() - m_pattern->instrumentTrack()->baseNote()
                + DefaultKey);
    }
}

void PianoRoll::playTestNote(Note* n)
{
    // m_lastKey = n->key();

    if(!n->isPlaying() && !m_recording)
    {
        qInfo("PianoRoll::playTestNote n=%d", n->key());
        n->setIsPlaying(true);

        qInfo("PianoRoll::playTestNote 1");
        const int baseVelocity
                = m_pattern->instrumentTrack()->midiPort()->baseVelocity();
        m_pattern->instrumentTrack()->pianoModel()->handleKeyPress(
                n->key() - m_pattern->instrumentTrack()->baseNote()
                        + DefaultKey,
                n->midiVelocity(baseVelocity));

        qInfo("PianoRoll::playTestNote 2");
        MidiEvent event(MidiMetaEvent, -1,
                        n->key() - m_pattern->instrumentTrack()->baseNote()
                                + DefaultKey,
                        panningToMidi(n->getPanning()));
        event.setMetaEvent(MidiNotePanning);
        m_pattern->instrumentTrack()->processInEvent(event, 0);
        qInfo("PianoRoll::playTestNote 3");
    }
}

void PianoRoll::resumeTestNotes()
{
    for(Note* n: m_pattern->notes())
        if(n->isPlaying())
        {
            n->setIsPlaying(false);
            playTestNote(n);
        }
}

void PianoRoll::suspendTestNotes()
{
    for(Note* n: m_pattern->notes())
    {
        if(n->isPlaying())
        {
            stopTestNote(n);
            n->setIsPlaying(true);
        }
    }
}

void PianoRoll::playTestKey(int key, int midiVelocity, int midiPanning)
{
    // turn off old key
    //    if(m_lastKey >= 0)  stopTestKey(m_lastKey);

    // remember which one we're playing
    m_lastKey = key;

    // play new key
    m_pattern->instrumentTrack()->pianoModel()->handleKeyPress(
            key - m_pattern->instrumentTrack()->baseNote() + DefaultKey,
            midiVelocity);

    MidiEvent event(MidiMetaEvent, -1,
                    key - m_pattern->instrumentTrack()->baseNote()
                            + DefaultKey,
                    midiPanning);
    event.setMetaEvent(MidiNotePanning);
    m_pattern->instrumentTrack()->processInEvent(event, 0);
}

void PianoRoll::stopTestKey()
{
    if(m_lastKey >= 0)
    {
        m_pattern->instrumentTrack()->pianoModel()->handleKeyRelease(
                m_lastKey - m_pattern->instrumentTrack()->baseNote()
                + DefaultKey);
        m_lastKey = -1;
    }
}

void PianoRoll::computeSelectedNotes(bool shift)
{
    if(m_selectStartTick == 0 && m_selectedTick == 0 && m_selectStartKey == 0
       && m_selectedKeys == 0)
    {
        // don't bother, there's no selection
        return;
    }

    // setup selection-vars
    int sel_pos_start = m_selectStartTick;
    int sel_pos_end   = m_selectStartTick + m_selectedTick;
    if(sel_pos_start > sel_pos_end)
    {
        qSwap<int>(sel_pos_start, sel_pos_end);
    }

    int sel_key_start = m_selectStartKey - m_startKey + 1;
    int sel_key_end   = sel_key_start + m_selectedKeys;
    if(sel_key_start > sel_key_end)
    {
        qSwap<int>(sel_key_start, sel_key_end);
    }

    // int y_base = noteEditTop() - 1;
    if(m_pattern != nullptr)
    {
        for(Note* note: m_pattern->notes())
        {
            // make a new selection unless they're holding shift
            if(!shift)
            {
                note->setSelected(false);
            }

            int len_ticks = note->length();

            if(len_ticks == 0)
            {
                continue;
            }
            else if(len_ticks < 0)
            {
                len_ticks = 4;
            }

            const int key = note->key() - m_startKey + 1;

            int pos_ticks = note->pos();

            // if the selection even barely overlaps the note
            if(key > sel_key_start && key <= sel_key_end
               && pos_ticks + len_ticks > sel_pos_start
               && pos_ticks < sel_pos_end)
            {
                // remove from selection when holding shift
                bool selected = shift && note->selected();
                note->setSelected(!selected);
            }
        }
    }

    removeSelection();
    update();
}

void PianoRoll::mouseReleaseEvent(QMouseEvent* me)
{
    bool mustRepaint = false;

    s_textFloat->hide();

    if(me->button() & Qt::LeftButton)
    {
        mustRepaint = true;

        if(m_action == ActionSelectNotes && m_editMode == ModeSelect)
        {
            // select the notes within the selection rectangle and
            // then destroy the selection rectangle
            computeSelectedNotes(me->modifiers() & Qt::ShiftModifier);
        }
        else if(m_action == ActionMoveNote)
        {
            // we moved one or more notes so they have to be
            // moved properly according to new starting-
            // time in the note-array of pattern
            m_pattern->rearrangeAllNotes();
        }

        if(m_action == ActionMoveNote || m_action == ActionResizeNote)
        {
            // if we only moved one note, deselect it so we can
            // edit the notes in the note edit area
            if(selectionCount() == 1)
            {
                clearSelectedNotes();
            }
        }
    }

    if(me->button() & Qt::RightButton)
    {
        m_mouseDownRight = false;
        mustRepaint      = true;
    }

    if(m_pattern != nullptr)
    {
        // turn off all notes that are playing
        stopTestNotes();
        /*
        for(Note* note: m_pattern->notes())
        {
            if(note->isPlaying())
            {
                m_pattern->instrumentTrack()->pianoModel()->handleKeyRelease(
                        note->key());
                note->setIsPlaying(false);
            }
        }
        */

        // stop playing keys that we let go of
        // m_pattern->instrumentTrack()->pianoModel()->handleKeyRelease(
        //   m_lastKey);
    }
    stopTestKey();

    // if(m_currentNote != nullptr)
    //  stopTestNote(m_currentNote);
    m_currentNote = nullptr;
    m_action      = ActionNone;

    if(m_editMode == ModeDraw)
    {
        QApplication::restoreOverrideCursor();
    }

    if(mustRepaint)
    {
        update();  // repaint()
    }
}

void PianoRoll::mouseMoveEvent(QMouseEvent* me)
{
    if(m_pattern == nullptr)
    {
        update();
        return;
    }

    if(m_action == ActionNone && me->buttons() == 0)
    {
        if(me->y() > keyAreaBottom() && me->y() < noteEditTop())
        {
            QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
            return;
        }
    }
    else if(m_action == ActionResizeNoteEditArea)
    {
        // change m_notesEditHeight and then repaint
        m_notesEditHeight = tLimit<int>(
                m_oldNotesEditHeight - (me->y() - m_moveStartY),
                NOTE_EDIT_MIN_HEIGHT,
                height() - PR_TOP_MARGIN - NOTE_EDIT_RESIZE_BAR
                        - PR_BOTTOM_MARGIN - KEY_AREA_MIN_HEIGHT);
        update();  // repaint()
        return;
    }

    if(me->y() > PR_TOP_MARGIN || m_action != ActionNone)
    {
        bool edit_note
                = (me->y() > noteEditTop()) && m_action != ActionSelectNotes;

        int key_num = getKey(me->y());
        int x       = me->x();

        // see if they clicked on the keyboard on the left
        if(x < WHITE_KEY_WIDTH && m_action == ActionNone && !edit_note
           && key_num != m_lastKey && me->buttons() & Qt::LeftButton)
        {
            // clicked on a key, play the note
            stopTestNotes();
            stopTestKey();
            // m_pattern->instrumentTrack()->pianoModel()->handleKeyPress(
            playTestKey(key_num,
                        ((float)x) / ((float)WHITE_KEY_WIDTH)
                                * MidiDefaultVelocity,
                        0);
            update();
            return;
        }

        x -= WHITE_KEY_WIDTH;

        if(me->buttons() & Qt::LeftButton && m_editMode == ModeDraw
           && (m_action == ActionMoveNote || m_action == ActionResizeNote))
        {
            // handle moving notes and resizing them
            bool replay_note
                    = key_num != m_lastKey && m_action == ActionMoveNote;

            if(replay_note
               || (m_action == ActionMoveNote
                   && (me->modifiers() & Qt::ShiftModifier)
                   && !m_startedWithShift))
            {
                //suspendTestNotes();
            }

            dragNotes(me->x(), me->y(), me->modifiers() & Qt::AltModifier,
                      me->modifiers() & Qt::ShiftModifier,
                      me->modifiers() & Qt::ControlModifier);

            if(replay_note && m_action == ActionMoveNote
               && !((me->modifiers() & Qt::ShiftModifier)
                    && !m_startedWithShift))
            {
                if(m_pattern != nullptr)
                {
                    if(m_pattern != nullptr
                       && m_pattern->instrumentTrack() != nullptr)
                    {
                        m_pattern->instrumentTrack()->silenceAllNotes();
                        m_pattern->instrumentTrack()->pianoModel()->reset();
                    }
                }
                //resumeTestNotes();
            }
        }
        else if(m_editMode != ModeErase
                && (edit_note || m_action == ActionChangeNoteProperty)
                && (me->buttons() & Qt::LeftButton
                    || me->buttons() & Qt::MiddleButton
                    || (me->buttons() & Qt::RightButton
                        && me->modifiers() & Qt::ShiftModifier)))
        {
            // editing note properties

            // Change notes within a certain pixel range of where
            // the mouse cursor is
            int pixel_range = 14;

            // convert to ticks so that we can check which notes
            // are in the range
            int ticks_start
                    = (x - pixel_range / 2) * MidiTime::ticksPerTact() / m_ppt
                      + m_currentPosition;
            int ticks_end
                    = (x + pixel_range / 2) * MidiTime::ticksPerTact() / m_ppt
                      + m_currentPosition;

            // get note-vector of current pattern
            const Notes& notes = m_pattern->notes();

            // determine what volume/panning to set note to
            // if middle-click, set to defaults
            volume_t  vol  = DefaultVolume;
            panning_t pan  = DefaultPanning;
            bool      bval = false;

            if(me->buttons() & Qt::LeftButton)
            {
                vol = tLimit<int>(
                        MinVolume
                                + (((float)noteEditBottom())
                                   - ((float)me->y()))
                                          / ((float)(noteEditBottom()
                                                     - noteEditTop()))
                                          * (MaxVolume - MinVolume),
                        MinVolume, MaxVolume);
                pan = tLimit<int>(
                        PanningLeft
                                + ((float)(noteEditBottom() - me->y()))
                                          / ((float)(noteEditBottom()
                                                     - noteEditTop()))
                                          * ((float)(PanningRight
                                                     - PanningLeft)),
                        PanningLeft, PanningRight);
                bval = ((float)(noteEditBottom() - me->y()))
                                               / ((float)(noteEditBottom()
                                                          - noteEditTop()))
                                       >= 0.5f
                               ? true
                               : false;
            }

            if(m_noteEditMode == NoteEditVolume)
            {
                m_lastNoteVolume = vol;
                showVolTextFloat(vol, me->pos());
            }
            else if(m_noteEditMode == NoteEditPanning)
            {
                m_lastNotePanning = pan;
                showPanTextFloat(pan, me->pos());
            }
            else if(m_noteEditMode == NoteEditLegato)
            {
                m_lastNoteLegato = bval;
                showLegatoTextFloat(bval, me->pos());
            }
            else if(m_noteEditMode == NoteEditMarcato)
            {
                m_lastNoteMarcato = bval;
                showMarcatoTextFloat(bval, me->pos());
            }
            else if(m_noteEditMode == NoteEditStaccato)
            {
                m_lastNoteStaccato = bval;
                showStaccatoTextFloat(bval, me->pos());
            }

            // When alt is pressed we only edit the note under the cursor
            bool altPressed = me->modifiers() & Qt::AltModifier;
            // We iterate from last note in pattern to the first,
            // chronologically
            Notes::ConstIterator it = notes.begin() + notes.size() - 1;
            for(int i = 0; i < notes.size(); ++i)
            {
                Note* n = *it;

                bool isUnderPosition = n->withinRange(ticks_start, ticks_end);
                // Play note under the cursor
                if(isUnderPosition)
                {
                    stopTestNotes();
                    playTestNote(n);
                }
                // If note is:
                // Under the cursor, when there is no selection
                // Selected, and alt is not pressed
                // Under the cursor, selected, and alt is pressed
                if((isUnderPosition && !isSelection())
                   || (n->selected() && !altPressed)
                   || (isUnderPosition && n->selected() && altPressed))
                {
                    if(m_noteEditMode == NoteEditVolume)
                    {
                        n->setVolume(vol);

                        const int baseVelocity = m_pattern->instrumentTrack()
                                                         ->midiPort()
                                                         ->baseVelocity();

                        m_pattern->instrumentTrack()->processInEvent(
                                MidiEvent(MidiKeyPressure, -1, n->key(),
                                          n->midiVelocity(baseVelocity)));
                    }
                    else if(m_noteEditMode == NoteEditPanning)
                    {
                        n->setPanning(pan);

                        MidiEvent evt(MidiMetaEvent, -1, n->key(),
                                      panningToMidi(pan));
                        evt.setMetaEvent(MidiNotePanning);
                        m_pattern->instrumentTrack()->processInEvent(evt);
                    }
                    else if(m_noteEditMode == NoteEditLegato)
                    {
                        n->setLegato(bval);
                    }
                    else if(m_noteEditMode == NoteEditMarcato)
                    {
                        n->setMarcato(bval);
                    }
                    else if(m_noteEditMode == NoteEditStaccato)
                    {
                        n->setStaccato(bval);
                    }
                }
                else if(n->isPlaying())
                {
                    // mouse not over this note, stop playing it.
                    /*
                    m_pattern->instrumentTrack()
                            ->pianoModel()
                            ->handleKeyRelease(n->key());
                    n->setIsPlaying(false);
                    */
                    stopTestNote(n);
                }

                --it;
            }

            // Emit pattern has changed
            m_pattern->emit dataChanged();
        }

        else if(me->buttons() == Qt::NoButton && m_editMode == ModeDraw)
        {
            // set move- or resize-cursor

            // get tick in which the cursor is posated
            int pos_ticks = (x * MidiTime::ticksPerTact()) / m_ppt
                            + m_currentPosition;

            // get note-vector of current pattern
            const Notes& notes = m_pattern->notes();

            // will be our iterator in the following loop
            Notes::ConstIterator it = notes.begin() + notes.size() - 1;

            // loop through whole note-vector
            for(int i = 0; i < notes.size(); ++i)
            {
                Note* note = *it;
                // and check whether the cursor is over an
                // existing note
                if(pos_ticks >= note->pos()
                   && pos_ticks <= note->pos() + note->length()
                   && note->key() == key_num && note->length() > 0)
                {
                    break;
                }
                --it;
            }

            // did it reach end of vector because there's
            // no note??
            if(it != notes.begin() - 1)
            {
                Note* note = *it;
                // x coordinate of the right edge of the note
                int noteRightX
                        = (note->pos() + note->length() - m_currentPosition)
                          * m_ppt / MidiTime::ticksPerTact();
                // cursor at the "tail" of the note?
                bool atTail = note->length() > 0
                              && x > noteRightX - RESIZE_AREA_WIDTH;
                Qt::CursorShape cursorShape
                        = atTail ? Qt::SizeHorCursor : Qt::SizeAllCursor;
                if(QApplication::overrideCursor() != nullptr)
                {
                    while(QApplication::overrideCursor() != nullptr
                          && QApplication::overrideCursor()->shape() != cursorShape)
                        QApplication::restoreOverrideCursor();
                }

                if(QApplication::overrideCursor() == nullptr)
                {
                    QCursor c(cursorShape);
                    QApplication::setOverrideCursor(c);
                }
            }
            else
            {
                // the cursor is over no note, so restore cursor
                while(QApplication::overrideCursor() != nullptr)
                {
                    QApplication::restoreOverrideCursor();
                }
            }
        }
        else if(me->buttons() & Qt::LeftButton && m_editMode == ModeSelect
                && m_action == ActionSelectNotes)
        {

            // change size of selection

            // get tick in which the cursor is posated
            int pos_ticks = x * MidiTime::ticksPerTact() / m_ppt
                            + m_currentPosition;

            m_selectedTick = pos_ticks - m_selectStartTick;
            if((int)m_selectStartTick + m_selectedTick < 0)
            {
                m_selectedTick = -static_cast<int>(m_selectStartTick);
            }
            m_selectedKeys = key_num - m_selectStartKey;
            if(key_num <= m_selectStartKey)
            {
                --m_selectedKeys;
            }
        }
        else if((m_editMode == ModeDraw && me->buttons() & Qt::RightButton)
                || (m_editMode == ModeErase && me->buttons()))
        {
            // holding down right-click to delete notes or holding down
            // any key if in erase mode

            // get tick in which the user clicked
            int pos_ticks = x * MidiTime::ticksPerTact() / m_ppt
                            + m_currentPosition;

            // get note-vector of current pattern
            const Notes& notes = m_pattern->notes();

            // will be our iterator in the following loop
            Notes::ConstIterator it = notes.begin();

            // loop through whole note-vector
            while(it != notes.end())
            {
                Note*    note = *it;
                MidiTime len  = note->length();
                if(len < 0)
                {
                    len = 4;
                }
                // and check whether the user clicked on an
                // existing note or an edit-line
                if(pos_ticks >= note->pos() && len > 0
                   && ((!edit_note && pos_ticks <= note->pos() + len
                        && note->key() == key_num)
                       || (edit_note
                           && pos_ticks
                                      <= note->pos()
                                                 + NOTE_EDIT_LINE_WIDTH
                                                           * MidiTime::
                                                                     ticksPerTact()
                                                           / m_ppt)))
                {
                    // delete this note
                    m_pattern->removeNote(note);
                    Engine::getSong()->setModified();
                }
                else
                {
                    ++it;
                }
            }
        }
    }
    else
    {
        if(me->buttons() & Qt::LeftButton && m_editMode == ModeSelect
           && m_action == ActionSelectNotes)
        {

            int x = me->x() - WHITE_KEY_WIDTH;
            if(x < 0 && m_currentPosition > 0)
            {
                x = 0;
                QCursor::setPos(
                        mapToGlobal(QPoint(WHITE_KEY_WIDTH, me->y())));
                if(m_currentPosition >= 4)
                {
                    m_leftRightScroll->setValue(m_currentPosition - 4);
                }
                else
                {
                    m_leftRightScroll->setValue(0);
                }
            }
            else if(x > width() - WHITE_KEY_WIDTH)
            {
                x = width() - WHITE_KEY_WIDTH;
                QCursor::setPos(mapToGlobal(QPoint(width(), me->y())));
                m_leftRightScroll->setValue(m_currentPosition + 4);
            }

            // get tick in which the cursor is posated
            int pos_ticks = x * MidiTime::ticksPerTact() / m_ppt
                            + m_currentPosition;

            m_selectedTick = pos_ticks - m_selectStartTick;
            if((int)m_selectStartTick + m_selectedTick < 0)
            {
                m_selectedTick = -static_cast<int>(m_selectStartTick);
            }

            int key_num      = getKey(me->y());
            int visible_keys = (height() - PR_TOP_MARGIN - PR_BOTTOM_MARGIN
                                - m_notesEditHeight)
                                       / KEY_LINE_HEIGHT
                               + 2;
            const int s_key = m_startKey - 1;

            if(key_num <= s_key)
            {
                QCursor::setPos(
                        mapToGlobal(QPoint(me->x(), keyAreaBottom())));
                m_topBottomScroll->setValue(m_topBottomScroll->value() + 1);
                key_num = s_key;
            }
            else if(key_num >= s_key + visible_keys)
            {
                QCursor::setPos(mapToGlobal(QPoint(me->x(), PR_TOP_MARGIN)));
                m_topBottomScroll->setValue(m_topBottomScroll->value() - 1);
                key_num = s_key + visible_keys;
            }

            m_selectedKeys = key_num - m_selectStartKey;
            if(key_num <= m_selectStartKey)
            {
                --m_selectedKeys;
            }
        }
        QApplication::restoreOverrideCursor();
    }

    m_lastMouseX = me->x();
    m_lastMouseY = me->y();

    update();
}

void PianoRoll::dragNotes(int x, int y, bool alt, bool shift, bool ctrl)
{
    // dragging one or more notes around

    // convert pixels to ticks and keys
    int off_x     = x - m_moveStartX;
    int off_ticks = off_x * MidiTime::ticksPerTact() / m_ppt;
    int off_key   = getKey(y) - getKey(m_moveStartY);

    // handle scroll changes while dragging
    off_ticks -= m_mouseDownTick - m_currentPosition;
    off_key -= m_mouseDownKey - m_startKey;

    // if they're not holding alt, quantize the offset
    if(!alt)
    {
        off_ticks = floor(off_ticks / quantization()) * quantization();
    }

    // make sure notes won't go outside boundary conditions
    if(m_action == ActionMoveNote && !(shift && !m_startedWithShift))
    {
        if(m_moveBoundaryLeft + off_ticks < 0)
        {
            off_ticks -= (off_ticks + m_moveBoundaryLeft);
        }
        if(m_moveBoundaryTop + off_key > NumKeys)
        {
            off_key -= NumKeys - (m_moveBoundaryTop + off_key);
        }
        if(m_moveBoundaryBottom + off_key < 0)
        {
            off_key -= (m_moveBoundaryBottom + off_key);
        }
    }

    // get note-vector of current pattern
    const Notes& notes = m_pattern->notes();

    if(m_action == ActionMoveNote)
    {
        for(Note* note: notes)
        {
            if(note->selected())
            {
                if(shift && !m_startedWithShift)
                {
                    // quick resize, toggled by holding shift after starting a
                    // note move, but not before
                    int ticks_new = note->oldLength().getTicks() + off_ticks;
                    if(ticks_new <= 0)
                    {
                        ticks_new = 1;
                    }
                    note->setLength(MidiTime(ticks_new));
                    m_lenOfNewNotes = note->length();
                }
                else
                {
                    // moving note
                    int pos_ticks = note->oldPos().getTicks() + off_ticks;
                    int key_num   = note->oldKey() + off_key;

                    // ticks can't be negative
                    pos_ticks = qMax(0, pos_ticks);
                    // upper/lower bound checks on key_num
                    key_num = qMax(0, key_num);
                    key_num = qMin(key_num, NumKeys - 1);

                    note->setPos(MidiTime(pos_ticks));
                    note->setKey(key_num);
                }
            }
        }
    }
    else if(m_action == ActionResizeNote)
    {
        // When resizing notes:
        // If shift is not pressed, resize the selected notes but do not
        // rearrange them If shift is pressed we resize and rearrange only the
        // selected notes If shift + ctrl then we also rearrange all posterior
        // notes (sticky) If shift is pressed but only one note is selected,
        // apply sticky

        if(shift)
        {
            // Algorithm:
            // Relative to the starting point of the left-most selected note,
            //   all selected note start-points and *endpoints* (not length)
            //   should be scaled by a calculated factor.
            // This factor is such that the endpoint of the note whose handle
            // is being dragged should lie under the cursor. first, determine
            // the start-point of the left-most selected note:
            int stretchStartTick = -1;
            for(const Note* note: notes)
            {
                if(note->selected()
                   && (stretchStartTick < 0
                       || note->oldPos().getTicks() < stretchStartTick))
                {
                    stretchStartTick = note->oldPos().getTicks();
                }
            }
            // determine the ending tick of the right-most selected note
            const Note* posteriorNote = nullptr;
            for(const Note* note: notes)
            {
                if(note->selected()
                   && (posteriorNote == nullptr
                       || note->oldPos().getTicks()
                                          + note->oldLength().getTicks()
                                  > posteriorNote->oldPos().getTicks()
                                            + posteriorNote->oldLength()
                                                      .getTicks()))
                {
                    posteriorNote = note;
                }
            }
            int posteriorEndTick = posteriorNote->pos().getTicks()
                                   + posteriorNote->length().getTicks();
            // end-point of the note whose handle is being dragged:
            int stretchEndTick = m_currentNote->oldPos().getTicks()
                                 + m_currentNote->oldLength().getTicks();
            // Calculate factor by which to scale the start-point and
            // end-point of all selected notes
            float scaleFactor
                    = (float)(stretchEndTick - stretchStartTick + off_ticks)
                      / qMax(1, stretchEndTick - stretchStartTick);
            scaleFactor = qMax(0.0f, scaleFactor);

            // process all selected notes & determine how much the endpoint of
            // the right-most note was shifted
            int posteriorDeltaThisFrame = 0;
            for(Note* note: notes)
            {
                if(note->selected())
                {
                    // scale relative start and end positions by scaleFactor
                    int newStart = stretchStartTick
                                   + scaleFactor
                                             * (note->oldPos().getTicks()
                                                - stretchStartTick);
                    int newEnd = stretchStartTick
                                 + scaleFactor
                                           * (note->oldPos().getTicks()
                                              + note->oldLength().getTicks()
                                              - stretchStartTick);
                    // if  not holding alt, quantize the offsets
                    if(!alt)
                    {
                        // quantize start time
                        int oldStart  = note->oldPos().getTicks();
                        int startDiff = newStart - oldStart;
                        startDiff     = floor(startDiff / quantization())
                                    * quantization();
                        newStart = oldStart + startDiff;
                        // quantize end time
                        int oldEnd  = oldStart + note->oldLength().getTicks();
                        int endDiff = newEnd - oldEnd;
                        endDiff     = floor(endDiff / quantization())
                                  * quantization();
                        newEnd = oldEnd + endDiff;
                    }
                    int newLength = qMax(1, newEnd - newStart);
                    if(note == posteriorNote)
                    {
                        posteriorDeltaThisFrame
                                = (newStart + newLength)
                                  - (note->pos().getTicks()
                                     + note->length().getTicks());
                    }
                    note->setLength(MidiTime(newLength));
                    note->setPos(MidiTime(newStart));

                    m_lenOfNewNotes = note->length();
                }
            }
            if(ctrl || selectionCount() == 1)
            {
                // if holding ctrl or only one note is selected, reposition
                // posterior notes
                for(Note* note: notes)
                {
                    if(!note->selected()
                       && note->pos().getTicks() >= posteriorEndTick)
                    {
                        int newStart = note->pos().getTicks()
                                       + posteriorDeltaThisFrame;
                        note->setPos(MidiTime(newStart));
                    }
                }
            }
        }
        else
        {
            // shift is not pressed; stretch length of selected notes but not
            // their position
            for(Note* note: notes)
            {
                if(note->selected())
                {
                    int newLength = note->oldLength() + off_ticks;
                    newLength     = qMax(1, newLength);
                    note->setLength(MidiTime(newLength));

                    m_lenOfNewNotes = note->length();
                }
            }
        }
    }

    m_pattern->updateLength();
    m_pattern->emit dataChanged();
    Engine::getSong()->setModified();
}

int PianoRoll::xCoordOfTick(int tick)
{
    return WHITE_KEY_WIDTH
           + ((tick - m_currentPosition) * m_ppt / MidiTime::ticksPerTact());
}

void PianoRoll::paintEvent(QPaintEvent* pe)
{
    bool drawNoteNames
            = ConfigManager::inst()->value("ui", "printnotelabels").toInt();

    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QBrush bgColor = p.background();

    // fill with bg color
    p.fillRect(0, 0, width(), height(), bgColor);

    // set font-size to 8
    p.setFont(pointSize<8>(p.font()));

    // y_offset is used to align the piano-keys on the key-lines
    int y_offset = 0;

    // calculate y_offset according to first key
    switch(prKeyOrder[m_startKey % KeysPerOctave])
    {
        case PR_BLACK_KEY:
            y_offset = KEY_LINE_HEIGHT / 4;
            break;
        case PR_WHITE_KEY_BIG:
            y_offset = KEY_LINE_HEIGHT / 2;
            break;
        case PR_WHITE_KEY_SMALL:
            if(prKeyOrder[((m_startKey + 1) % KeysPerOctave)] != PR_BLACK_KEY)
            {
                y_offset = KEY_LINE_HEIGHT / 2;
            }
            break;
    }
    // start drawing at the bottom
    int key_line_y = keyAreaBottom() - 1;
    // used for aligning black-keys later
    int first_white_key_height = WHITE_KEY_SMALL_HEIGHT;
    // key-counter - only needed for finding out whether the processed
    // key is the first one
    int keys_processed = 0;

    int key = m_startKey;

    // display note marks before drawing other lines
    for(int i = 0; i < m_markedSemiTones.size(); i++)
    {
        const int key_num = m_markedSemiTones.at(i);
        const int y       = keyAreaBottom() + 5
                      - KEY_LINE_HEIGHT * (key_num - m_startKey + 1);

        if(y > keyAreaBottom())
        {
            break;
        }

        p.fillRect(WHITE_KEY_WIDTH + 1, y - KEY_LINE_HEIGHT / 2, width() - 10,
                   KEY_LINE_HEIGHT, markedSemitoneColor());
    }

    Piano* pianoModel = nullptr;
    if(m_pattern != nullptr)
    {
        pianoModel = m_pattern->instrumentTrack()->pianoModel();
        // if(pianoModel != nullptr)
        //    pianoModel->lock();
    }

    // draw all white keys
    for(int y = key_line_y + 1 + y_offset; y > PR_TOP_MARGIN;
        key_line_y -= KEY_LINE_HEIGHT, ++keys_processed)
    {
        // check for white key that is only half visible on the
        // bottom of piano-roll
        if(keys_processed == 0
           && prKeyOrder[m_startKey % KeysPerOctave] == PR_BLACK_KEY)
        {
            // draw it!
            p.drawPixmap(PIANO_X, y - WHITE_KEY_SMALL_HEIGHT,
                         *s_whiteKeySmallPm);
            // update y-pos
            y -= WHITE_KEY_SMALL_HEIGHT / 2;
            // move first black key down (we didn't draw whole
            // white key so black key needs to be lifted down)
            // (default for first_white_key_height =
            // WHITE_KEY_SMALL_HEIGHT, so WHITE_KEY_SMALL_HEIGHT/2
            // is smaller)
            first_white_key_height = WHITE_KEY_SMALL_HEIGHT / 2;
        }
        // check whether to draw a big or a small white key
        if(prKeyOrder[key % KeysPerOctave] == PR_WHITE_KEY_SMALL)
        {
            // draw a small one while checking if it is pressed or not
            if(pianoModel != nullptr && pianoModel->isKeyPressed(key))
            {
                p.drawPixmap(PIANO_X, y - WHITE_KEY_SMALL_HEIGHT,
                             *s_whiteKeySmallPressedPm);
            }
            else
            {
                p.drawPixmap(PIANO_X, y - WHITE_KEY_SMALL_HEIGHT,
                             *s_whiteKeySmallPm);
            }
            // update y-pos
            y -= WHITE_KEY_SMALL_HEIGHT;
        }
        else if(prKeyOrder[key % KeysPerOctave] == PR_WHITE_KEY_BIG)
        {
            // draw a big one while checking if it is pressed or not
            if(pianoModel != nullptr && pianoModel->isKeyPressed(key))
            {
                p.drawPixmap(PIANO_X, y - WHITE_KEY_BIG_HEIGHT,
                             *s_whiteKeyBigPressedPm);
            }
            else
            {
                p.drawPixmap(PIANO_X, y - WHITE_KEY_BIG_HEIGHT,
                             *s_whiteKeyBigPm);
            }
            // if a big white key has been the first key,
            // black keys needs to be lifted up
            if(keys_processed == 0)
            {
                first_white_key_height = WHITE_KEY_BIG_HEIGHT;
            }
            // update y-pos
            y -= WHITE_KEY_BIG_HEIGHT;
        }

        // Compute the corrections for the note names
        int yCorrectionForNoteLabels = 0;

        int keyCode = key % KeysPerOctave;
        switch(keyCode)
        {
            case 0:
            case 5:
                yCorrectionForNoteLabels = -4;
                break;
            case 2:
            case 7:
            case 9:
                yCorrectionForNoteLabels = -2;
                break;
            case 4:
            case 11:
                yCorrectionForNoteLabels = 2;
                break;
        }

        if(Piano::isWhiteKey(key))
        {
            // Draw note names if activated in the preferences, C notes are
            // always drawn
            if(key % 12 == 0 || drawNoteNames)
            {
                QString noteString = Note::findKeyName(key);
                // getNoteString(key);

                QPoint textStart(WHITE_KEY_WIDTH - 20, key_line_y);
                textStart += QPoint(0, yCorrectionForNoteLabels);

                p.setPen(textShadow());
                p.drawText(textStart + QPoint(1, 1), noteString);
                // The C key is painted darker than the other ones
                if(key % 12 == 0)
                {
                    p.setPen(textColor());
                }
                else
                {
                    p.setPen(textColorLight());
                }
                p.drawText(textStart, noteString);
            }
        }
        ++key;
        if(key >= NumKeys)
            break;
    }

    // reset all values, because now we're going to draw all black keys
    key            = m_startKey;
    keys_processed = 0;
    int white_cnt  = 0;

    // and go!
    for(int y = keyAreaBottom() + y_offset; y > PR_TOP_MARGIN;
        ++keys_processed)
    {
        // check for black key that is only half visible on the bottom
        // of piano-roll
        if(keys_processed == 0
           // current key may not be a black one
           && prKeyOrder[key % KeysPerOctave] != PR_BLACK_KEY
           // but the previous one must be black (we must check this
           // because there might be two white keys (E-F)
           && prKeyOrder[(key - 1) % KeysPerOctave] == PR_BLACK_KEY)
        {
            // draw the black key!
            p.drawPixmap(PIANO_X, y - BLACK_KEY_HEIGHT / 2, *s_blackKeyPm);
            // is the one after the start-note a black key??
            if(prKeyOrder[(key + 1) % KeysPerOctave] != PR_BLACK_KEY)
            {
                // no, then move it up!
                y -= KEY_LINE_HEIGHT / 2;
            }
        }
        // current key black?
        if(prKeyOrder[key % KeysPerOctave] == PR_BLACK_KEY)
        {
            // then draw it (calculation of y very complicated,
            // but that's the only working solution, sorry)
            // check if the key is pressed or not
            if(pianoModel != nullptr && pianoModel->isKeyPressed(key))
            {
                p.drawPixmap(
                        PIANO_X,
                        y - (first_white_key_height - WHITE_KEY_SMALL_HEIGHT)
                                - WHITE_KEY_SMALL_HEIGHT / 2 - 1
                                - BLACK_KEY_HEIGHT,
                        *s_blackKeyPressedPm);
            }
            else
            {
                p.drawPixmap(
                        PIANO_X,
                        y - (first_white_key_height - WHITE_KEY_SMALL_HEIGHT)
                                - WHITE_KEY_SMALL_HEIGHT / 2 - 1
                                - BLACK_KEY_HEIGHT,
                        *s_blackKeyPm);
            }
            // update y-pos
            y -= WHITE_KEY_BIG_HEIGHT;
            // reset white-counter
            white_cnt = 0;
        }
        else
        {
            // simple workaround for increasing x if there were
            // two white keys (e.g. between E and F)
            ++white_cnt;
            if(white_cnt > 1)
            {
                y -= WHITE_KEY_BIG_HEIGHT / 2;
            }
        }

        ++key;
        if(key >= NumKeys)
            break;
    }

    // if(pianoModel != nullptr)
    //    pianoModel->unlock();

    // qInfo("paintEvent key=%d top=%d",key,PR_TOP_MARGIN);
    bool redline = (key >= NumKeys);

    // erase the area below the piano, because there might be keys that
    // should be only half-visible
    p.fillRect(QRect(0, keyAreaBottom(), WHITE_KEY_WIDTH,
                     noteEditBottom() - keyAreaBottom()),
               bgColor);

    // display note editing info
    QFont f = p.font();
    f.setBold(false);
    p.setFont(pointSize<10>(f));
    p.setPen(noteModeColor());
    p.drawText(QRect(0, keyAreaBottom(), WHITE_KEY_WIDTH,
                     noteEditBottom() - keyAreaBottom()),
               Qt::AlignCenter | Qt::TextWordWrap,
               m_nemStr.at(m_noteEditMode) + ":");

    // set clipping area, because we are not allowed to paint over
    // keyboard
    p.setClipRect(WHITE_KEY_WIDTH, PR_TOP_MARGIN, width() - WHITE_KEY_WIDTH,
                  height() - PR_TOP_MARGIN - PR_BOTTOM_MARGIN);

    // draw the grid
    if(m_pattern != nullptr)
    {
        int q, x, tick;

        q = quantization();
        if(m_zoomingXModel.value() <= 3)
        {
            if(q % 3 != 0)
            {
                // If we're under 100% zoom, we allow quantization grid up to
                // 1/24 for triplets to ensure a dense doesn't fill out the
                // background
                if(q < 8)
                    q = 8;  // ticks
            }
            else
            {
                // If we're under 100% zoom, we allow quantization grid up to
                // 1/32 for normal notes
                if(q < 6)
                    q = 6;  // ticks
            }
        }

        // First we draw the vertical quantization lines
        for(tick = m_currentPosition - m_currentPosition % q,
        x        = xCoordOfTick(tick);
            x <= width(); tick += q, x = xCoordOfTick(tick))
        {
            p.setPen(lineColor());
            p.drawLine(x, PR_TOP_MARGIN, x, height() - PR_BOTTOM_MARGIN);
        }

        // Draw horizontal lines
        key = m_startKey;
        for(int y = keyAreaBottom() - 1; y > PR_TOP_MARGIN;
            y -= KEY_LINE_HEIGHT)
        {
            if(static_cast<Keys>(key % KeysPerOctave) == Key_C)
            {
                // C note gets accented
                p.setPen(beatLineColor());
            }
            else
            {
                p.setPen(lineColor());
            }
            p.drawLine(WHITE_KEY_WIDTH, y, width(), y);
            ++key;
        }

        // Draw alternating shades on bars
        float timeSignature
                = static_cast<float>(
                          Engine::getSong()->getTimeSigModel().getNumerator())
                  / static_cast<float>(Engine::getSong()
                                               ->getTimeSigModel()
                                               .getDenominator());
        float zoomFactor = EditorWindow::ZOOM_LEVELS[m_zoomingXModel.value()];
        // the bars which disappears at the left side by scrolling
        int leftBars
                = m_currentPosition * zoomFactor / MidiTime::ticksPerTact();

        // iterates the visible bars and draw the shading on uneven bars
        for(int x = WHITE_KEY_WIDTH, barCount = leftBars;
            x < width() + m_currentPosition * zoomFactor / timeSignature;
            x += m_ppt, ++barCount)
        {
            if((barCount + leftBars) % 2 != 0)
            {
                p.fillRect(x - m_currentPosition * zoomFactor / timeSignature,
                           PR_TOP_MARGIN, m_ppt,
                           height() - (PR_BOTTOM_MARGIN + PR_TOP_MARGIN),
                           backgroundShade());
            }
        }

        // Draw the vertical beat lines
        int ticksPerBeat
                = DefaultTicksPerTact
                  / Engine::getSong()->getTimeSigModel().getDenominator();

        for(tick = m_currentPosition - m_currentPosition % ticksPerBeat,
        x        = xCoordOfTick(tick);
            x <= width(); tick += ticksPerBeat, x = xCoordOfTick(tick))
        {
            p.setPen(beatLineColor());
            p.drawLine(x, PR_TOP_MARGIN, x, height() - PR_BOTTOM_MARGIN);
        }

        // Draw the vertical bar lines
        for(tick = m_currentPosition
                   - m_currentPosition % MidiTime::ticksPerTact(),
        x = xCoordOfTick(tick);
            x <= width();
            tick += MidiTime::ticksPerTact(), x = xCoordOfTick(tick))
        {
            p.setPen(barLineColor());
            p.drawLine(x, PR_TOP_MARGIN, x, height() - PR_BOTTOM_MARGIN);
        }
    }

    // following code draws all notes in visible area
    // and the note editing stuff (volume, panning, etc)

    // setup selection-vars
    int sel_pos_start = m_selectStartTick;
    int sel_pos_end   = m_selectStartTick + m_selectedTick;
    if(sel_pos_start > sel_pos_end)
    {
        qSwap<int>(sel_pos_start, sel_pos_end);
    }

    int sel_key_start = m_selectStartKey - m_startKey + 1;
    int sel_key_end   = sel_key_start + m_selectedKeys;
    if(sel_key_start > sel_key_end)
    {
        qSwap<int>(sel_key_start, sel_key_end);
    }

    int y_base = keyAreaBottom() - 1;
    if(m_pattern != nullptr)
    {
        p.setClipRect(WHITE_KEY_WIDTH, PR_TOP_MARGIN,
                      width() - WHITE_KEY_WIDTH, height() - PR_TOP_MARGIN);

        const int visible_keys
                = (keyAreaBottom() - keyAreaTop()) / KEY_LINE_HEIGHT + 2;

        QPolygonF editHandles;

        // -- Begin ghost pattern
        if(m_ghostPattern != nullptr && m_ghostPattern != m_pattern)
        {
            for(const Note* note: m_ghostPattern->notes())
            {
                int len_ticks = note->length();

                if(len_ticks == 0)
                {
                    continue;
                }
                else if(len_ticks < 0)
                {
                    len_ticks = 4;
                }
                const int key = note->key() - m_startKey + 1;

                int pos_ticks = note->pos();

                int note_width = len_ticks * m_ppt / MidiTime::ticksPerTact();
                const int x    = (pos_ticks - m_currentPosition) * m_ppt
                              / MidiTime::ticksPerTact();
                // skip this note if not in visible area at all
                if(!(x + note_width >= 0 && x <= width() - WHITE_KEY_WIDTH))
                {
                    continue;
                }

                // is the note in visible area?
                if(key > 0 && key <= visible_keys)
                {

                    // we've done and checked all, let's draw the
                    // note
                    drawNoteRect(p, x + WHITE_KEY_WIDTH,
                                 y_base - key * KEY_LINE_HEIGHT, note_width,
                                 note,
                                 ghostNoteColor(),  // ghostNoteTextColor(),
                                 ghostNoteColor(), ghostNoteOpacity(),
                                 ghostNoteBorders(), drawNoteNames);
                }
            }
        }
        // -- End ghost pattern

        for(const Note* note: m_pattern->notes())
        {
            int len_ticks = note->length();

            if(len_ticks == 0)
            {
                continue;
            }
            else if(len_ticks < 0)
            {
                len_ticks = 4;
            }

            const int key = note->key() - m_startKey + 1;

            int pos_ticks = note->pos();

            int note_width = len_ticks * m_ppt / MidiTime::ticksPerTact();
            const int x    = (pos_ticks - m_currentPosition) * m_ppt
                          / MidiTime::ticksPerTact();
            // skip this note if not in visible area at all
            if(!(x + note_width >= 0 && x <= width() - WHITE_KEY_WIDTH))
            {
                continue;
            }

            // is the note in visible area?
            if(key > 0 && key <= visible_keys)
            {

                // we've done and checked all, let's draw the
                // note
                drawNoteRect(p, x + WHITE_KEY_WIDTH,
                             y_base - key * KEY_LINE_HEIGHT, note_width, note,
                             noteColor(), selectedNoteColor(), noteOpacity(),
                             noteBorders(), drawNoteNames);
            }

            // draw note editing stuff
            int editHandleTop = 0;
            if(m_noteEditMode == NoteEditVolume)
            {
                QColor color = barColor().lighter(
                        30 + (note->getVolume() * 90 / MaxVolume));
                if(note->selected())
                {
                    color = selectedNoteColor();
                }
                p.setPen(QPen(color, NOTE_EDIT_LINE_WIDTH));

                editHandleTop = noteEditBottom()
                                - ((float)(note->getVolume() - MinVolume))
                                          / ((float)(MaxVolume - MinVolume))
                                          * ((float)(noteEditBottom()
                                                     - noteEditTop()));

                p.drawLine(QLineF(
                        noteEditLeft() + x + 0.5, editHandleTop + 0.5,
                        noteEditLeft() + x + 0.5, noteEditBottom() + 0.5));
            }
            else if(m_noteEditMode == NoteEditPanning)
            {
                QColor color = noteColor();
                if(note->selected())
                {
                    color = selectedNoteColor();
                }

                p.setPen(QPen(color, NOTE_EDIT_LINE_WIDTH));

                editHandleTop
                        = noteEditBottom()
                          - ((float)(note->getPanning() - PanningLeft))
                                    / ((float)((PanningRight - PanningLeft)))
                                    * ((float)(noteEditBottom()
                                               - noteEditTop()));

                p.drawLine(QLine(
                        noteEditLeft() + x,
                        noteEditTop()
                                + ((float)(noteEditBottom() - noteEditTop()))
                                          / 2.0f,
                        noteEditLeft() + x, editHandleTop));
            }
            else  // bool values
            {
                bool b = false;
                if(m_noteEditMode == NoteEditLegato)
                    b = note->legato();
                else if(m_noteEditMode == NoteEditMarcato)
                    b = note->marcato();
                else if(m_noteEditMode == NoteEditStaccato)
                    b = note->staccato();

                QColor color = note->selected() ? selectedNoteColor()
                                                : noteColor();
                p.setPen(QPen(color, NOTE_EDIT_LINE_WIDTH));

                editHandleTop = noteEditBottom()
                                - (b ? 0.75f : 0.25f)
                                          * ((float)(noteEditBottom()
                                                     - noteEditTop()));

                p.drawLine(QLine(
                        noteEditLeft() + x,
                        noteEditTop()
                                + ((float)(noteEditBottom() - noteEditTop()))
                                          / 2.0f,
                        noteEditLeft() + x, editHandleTop));
            }

            editHandles << QPoint(x + noteEditLeft(), editHandleTop);

            if(note->hasDetuningInfo())
            {
                drawDetuningInfo(p, note, x + WHITE_KEY_WIDTH,
                                 y_base - key * KEY_LINE_HEIGHT);
            }
        }

        p.setPen(QPen(noteColor(), NOTE_EDIT_LINE_WIDTH + 2));
        p.drawPoints(editHandles);
    }
    else
    {
        QFont f = p.font();
        f.setBold(true);
        p.setFont(pointSize<14>(f));
        p.setPen(QApplication::palette().color(QPalette::Active,
                                               QPalette::BrightText));
        p.drawText(WHITE_KEY_WIDTH + 20, PR_TOP_MARGIN + 40,
                   tr("Open a score tile by double-clicking on it."));
    }

    p.setClipRect(WHITE_KEY_WIDTH, PR_TOP_MARGIN, width() - WHITE_KEY_WIDTH,
                  height() - PR_TOP_MARGIN - m_notesEditHeight
                          - PR_BOTTOM_MARGIN);

    // now draw selection-frame
    int x = ((sel_pos_start - m_currentPosition) * m_ppt)
            / MidiTime::ticksPerTact();
    int w = (((sel_pos_end - m_currentPosition) * m_ppt)
             / MidiTime::ticksPerTact())
            - x;
    int y = (int)y_base - sel_key_start * KEY_LINE_HEIGHT;
    int h = (int)y_base - sel_key_end * KEY_LINE_HEIGHT - y;
    p.setPen(selectedNoteColor());
    p.setBrush(Qt::NoBrush);
    p.drawRect(x + WHITE_KEY_WIDTH, y, w, h);

    // TODO: Get this out of paint event
    int l = (m_pattern != nullptr) ? (int)m_pattern->length() : 0;

    // reset scroll-range
    if(m_leftRightScroll->maximum() != l)
    {
        m_leftRightScroll->setRange(0, l);
        m_leftRightScroll->setPageStep(l);
    }

    // set line colors
    QColor editAreaCol   = QColor(lineColor());
    QColor currentKeyCol = QColor(beatLineColor());

    editAreaCol.setAlpha(64);
    currentKeyCol.setAlpha(64);

    // horizontal line for the key under the cursor
    if(m_pattern != nullptr)
    {
        int key_num = getKey(mapFromGlobal(QCursor::pos()).y());
        p.fillRect(10,
                   keyAreaBottom() + 3
                           - KEY_LINE_HEIGHT * (key_num - m_startKey + 1),
                   width() - 10, KEY_LINE_HEIGHT - 7, currentKeyCol);
    }

    // bar to resize note edit area
    p.setClipRect(0, 0, width(), height());
    p.fillRect(QRect(0, keyAreaBottom(), width() - PR_RIGHT_MARGIN,
                     NOTE_EDIT_RESIZE_BAR),
               editAreaCol);

    if(redline)
        p.fillRect(0, PR_TOP_MARGIN, width() - 1, KEY_LINE_HEIGHT - 2,
                   Qt::red);

    const QPixmap* cursor = nullptr;
    // draw current edit-mode-icon below the cursor
    switch(m_editMode)
    {
        case ModeDraw:
            if(m_mouseDownRight)
            {
                cursor = s_toolErase;
            }
            else if(m_action == ActionMoveNote)
            {
                cursor = s_toolMove;
            }
            else
            {
                cursor = s_toolDraw;
            }
            break;
        case ModeErase:
            cursor = s_toolErase;
            break;
        case ModeSelect:
            cursor = s_toolSelect;
            break;
        case ModeEditDetuning:
            cursor = s_toolOpen;
            break;
    }
    if(cursor != nullptr)
    {
        p.drawPixmap(mapFromGlobal(QCursor::pos()) + QPoint(8, 8), *cursor);
    }
}

// responsible for moving/resizing scrollbars after window-resizing
void PianoRoll::resizeEvent(QResizeEvent* re)
{
    m_leftRightScroll->setGeometry(WHITE_KEY_WIDTH, height() - SCROLLBAR_SIZE,
                                   width() - WHITE_KEY_WIDTH, SCROLLBAR_SIZE);
    m_topBottomScroll->setGeometry(width() - SCROLLBAR_SIZE, PR_TOP_MARGIN,
                                   SCROLLBAR_SIZE,
                                   height() - PR_TOP_MARGIN - SCROLLBAR_SIZE);

    int total_pixels
            = KEY_LINE_HEIGHT * (NumKeys + 1)  // OCTAVE_HEIGHT * NumOctaves
              - (height() - PR_TOP_MARGIN - PR_BOTTOM_MARGIN
                 - m_notesEditHeight);
    m_totalKeysToScroll
            = total_pixels
              / KEY_LINE_HEIGHT;  // KeysPerOctave / OCTAVE_HEIGHT;

    m_topBottomScroll->setRange(0, m_totalKeysToScroll);

    if(m_startKey > m_totalKeysToScroll)
    {
        m_startKey = m_totalKeysToScroll;
    }
    m_topBottomScroll->setValue(m_totalKeysToScroll - m_startKey);

    Engine::getSong()
            ->getPlayPos(Song::Mode_PlayPattern)
            .m_timeLine->setFixedWidth(width());

    update();
}

void PianoRoll::wheelEvent(QWheelEvent* we)
{
    we->accept();
    // handle wheel events for note edit area - for editing note vol/pan with
    // mousewheel
    if(we->x() > noteEditLeft() && we->x() < noteEditRight()
       && we->y() > noteEditTop() && we->y() < noteEditBottom())
    {
        if(m_pattern == nullptr)
        {
            return;
        }
        // get values for going through notes
        int pixel_range = 8;
        int x           = we->x() - WHITE_KEY_WIDTH;
        int ticks_start
                = (x - pixel_range / 2) * MidiTime::ticksPerTact() / m_ppt
                  + m_currentPosition;
        int ticks_end
                = (x + pixel_range / 2) * MidiTime::ticksPerTact() / m_ppt
                  + m_currentPosition;

        // When alt is pressed we only edit the note under the cursor
        bool altPressed = we->modifiers() & Qt::AltModifier;
        // go through notes to figure out which one we want to change
        Notes nv;
        for(Note* i: m_pattern->notes())
        {
            if(i->withinRange(ticks_start, ticks_end)
               || (i->selected() && !altPressed))
            {
                nv += i;
            }
        }
        if(nv.size() > 0)
        {
            const int step = we->delta() > 0 ? 1.0 : -1.0;
            if(m_noteEditMode == NoteEditVolume)
            {
                for(Note* n: nv)
                {
                    volume_t vol = tLimit<int>(n->getVolume() + step,
                                               MinVolume, MaxVolume);
                    n->setVolume(vol);
                }
                bool allVolumesEqual = std::all_of(
                        nv.begin(), nv.end(), [nv](const Note* note) {
                            return note->getVolume() == nv[0]->getVolume();
                        });
                if(allVolumesEqual)
                {
                    // show the volume hover-text only if all notes have the
                    // same volume
                    showVolTextFloat(nv[0]->getVolume(), we->pos(), 1000);
                }
            }
            else if(m_noteEditMode == NoteEditPanning)
            {
                for(Note* n: nv)
                {
                    panning_t pan = tLimit<int>(n->getPanning() + step,
                                                PanningLeft, PanningRight);
                    n->setPanning(pan);
                }
                bool allPansEqual = std::all_of(
                        nv.begin(), nv.end(), [nv](const Note* note) {
                            return note->getPanning() == nv[0]->getPanning();
                        });
                if(allPansEqual)
                {
                    // show the pan hover-text only if all notes have the same
                    // panning
                    showPanTextFloat(nv[0]->getPanning(), we->pos(), 1000);
                }
            }
            else  // bool values
            {
                for(Note* n: nv)
                {
                    bool b = false;
                    if(m_noteEditMode == NoteEditLegato)
                        b = n->legato();
                    else if(m_noteEditMode == NoteEditMarcato)
                        b = n->marcato();
                    else if(m_noteEditMode == NoteEditStaccato)
                        b = n->staccato();

                    b = ((b ? 0.75f : 0.25f) + step / 2.f) < 0.5f ? false
                                                                  : true;
                    if(m_noteEditMode == NoteEditLegato)
                        n->setLegato(b);
                    else if(m_noteEditMode == NoteEditMarcato)
                        n->setMarcato(b);
                    else if(m_noteEditMode == NoteEditStaccato)
                        n->setStaccato(b);
                }
                /*
                bool allLegsEqual = std::all_of(
                        nv.begin(), nv.end(), [nv](const Note* note) {
                            return note->legato() == nv[0]->legato();
                        });
                if(allLegsEqual)
                {
                    // show the pan hover-text only if all notes have the same
                    // panning
                    showPanTextFloat(nv[0]->legato(), we->pos(), 1000);
                }
                */
            }
            update();
        }
    }

    // not in note edit area, so handle scrolling/zooming and quantization
    // change
    else if(we->modifiers() & Qt::ControlModifier
            && we->modifiers() & Qt::AltModifier)
    {
        int q = m_quantizeModel.value();
        if(we->delta() > 0)
        {
            q--;
        }
        else if(we->delta() < 0)
        {
            q++;
        }
        q = qBound(0, q, m_quantizeModel.size() - 1);
        m_quantizeModel.setValue(q);
    }
    else if(we->modifiers() & Qt::ControlModifier
            && we->modifiers() & Qt::ShiftModifier)
    {
        int l = m_noteLenModel.value();
        if(we->delta() > 0)
        {
            l--;
        }
        else if(we->delta() < 0)
        {
            l++;
        }
        l = qBound(0, l, m_noteLenModel.size() - 1);
        m_noteLenModel.setValue(l);
    }
    else if(we->modifiers() & Qt::ControlModifier)
    {
        int z = m_zoomingXModel.value();
        if(we->delta() > 0)
        {
            z++;
        }
        else if(we->delta() < 0)
        {
            z--;
        }
        z = qBound(0, z, m_zoomingXModel.size() - 1);
        // update combobox with zooming-factor
        m_zoomingXModel.setValue(z);
    }
    else if(we->modifiers() & Qt::ShiftModifier
            || we->orientation() == Qt::Horizontal)
    {
        m_leftRightScroll->setValue(m_leftRightScroll->value()
                                    - we->delta() * 2 / 15);
    }
    else
    {
        m_topBottomScroll->setValue(m_topBottomScroll->value()
                                    - we->delta() / 30);
    }
}

void PianoRoll::focusOutEvent(QFocusEvent*)
{
    /*
    if(m_pattern!=nullptr)
    {
        if(m_pattern != nullptr && m_pattern->instrumentTrack() != nullptr)
        {
            // m_pattern->instrumentTrack()->silenceAllNotes();
            m_pattern->instrumentTrack()->pianoModel()->reset();
        }
    }
    */
    // stopTestNotes();
    stopTestKey();

    m_editMode = m_ctrlMode;
    update();
}

int PianoRoll::getKey(int y) const
{
    int key_line_y = keyAreaBottom() - 1;
    // pressed key on piano
    int key_num = (key_line_y - y) / KEY_LINE_HEIGHT;
    key_num += m_startKey;

    // some range-checking-stuff
    if(key_num < 0)
    {
        key_num = 0;
    }

    if(key_num >= NumKeys)  // KeysPerOctave * NumOctaves
    {
        key_num = NumKeys - 1;  // KeysPerOctave * NumOctaves
    }

    return key_num;
}

QList<int> PianoRoll::getAllOctavesForKey(int keyToMirror) const
{
    QList<int> keys;

    for(int i = keyToMirror % KeysPerOctave; i < NumKeys; i += KeysPerOctave)
    {
        keys.append(i);
    }

    return keys;
}

Song::PlayModes PianoRoll::desiredPlayModeForAccompany() const
{
    if(m_pattern->getTrack()->trackContainer()
       == Engine::getBBTrackContainer())
    {
        return Song::Mode_PlayBB;
    }
    return Song::Mode_PlaySong;
}

void PianoRoll::play()
{
    if(m_pattern == nullptr)
        return;
    if(Engine::getSong()->playMode() != Song::Mode_PlayPattern)
    {
        Engine::getSong()->playPattern(m_pattern);
    }
    else
    {
        Engine::getSong()->togglePause();
    }
}

void PianoRoll::record()
{
    if(Engine::getSong()->isPlaying())
    {
        stop();
    }
    if(m_recording || m_pattern == nullptr)
        return;
    m_pattern->addJournalCheckPoint();
    m_recording = true;

    Engine::getSong()->playPattern(m_pattern, false);
}

void PianoRoll::recordAccompany()
{
    if(Engine::getSong()->isPlaying())
    {
        stop();
    }
    if(m_recording || m_pattern == nullptr)
        return;
    m_pattern->addJournalCheckPoint();
    m_recording = true;

    if(m_pattern->getTrack()->trackContainer() == Engine::getSong())
    {
        Engine::getSong()->playSong();
    }
    else
    {
        Engine::getSong()->playBB();
    }
}

void PianoRoll::stop()
{
    Engine::getSong()->stop();
    m_recording = false;
    m_scrollBack
            = (m_timeLine->autoScroll() == TimeLineWidget::AutoScrollEnabled);
}

void PianoRoll::startRecordNote(const Note& n)
{
    if(m_recording && m_pattern != nullptr && Engine::getSong()->isPlaying()
       && (Engine::getSong()->playMode() == desiredPlayModeForAccompany()
           || Engine::getSong()->playMode() == Song::Mode_PlayPattern))
    {
        MidiTime sub;
        if(Engine::getSong()->playMode() == Song::Mode_PlaySong)
        {
            sub = m_pattern->startPosition();
        }
        Note n1(1,
                Engine::getSong()->getPlayPos(Engine::getSong()->playMode())
                        - sub,
                n.key(), n.getVolume(), n.getPanning());
        n1.setLegato(n.legato());
        n1.setMarcato(n.marcato());
        n1.setStaccato(n.staccato());
        if(n1.pos() >= 0)
        {
            m_recordingNotes << n1;
        }
    }
}

void PianoRoll::finishRecordNote(const Note& n)
{
    if(m_recording && m_pattern != nullptr && Engine::getSong()->isPlaying()
       && (Engine::getSong()->playMode() == desiredPlayModeForAccompany()
           || Engine::getSong()->playMode() == Song::Mode_PlayPattern))
    {
        for(QList<Note>::Iterator it = m_recordingNotes.begin();
            it != m_recordingNotes.end(); ++it)
        {
            if(it->key() == n.key())
            {
                Note n1(n.length(), it->pos(), it->key(), it->getVolume(),
                        it->getPanning());
                n1.setLegato(it->legato());
                n1.setMarcato(it->marcato());
                n1.setStaccato(it->staccato());
                n1.quantizeLength(quantization());
                m_pattern->addNote(n1);
                update();
                m_recordingNotes.erase(it);
                break;
            }
        }
    }
}

void PianoRoll::horScrolled(int new_pos)
{
    m_currentPosition = new_pos;
    emit positionChanged(m_currentPosition);
    update();
}

void PianoRoll::verScrolled(int new_pos)
{
    // revert value
    m_startKey = m_totalKeysToScroll - new_pos;

    update();
}

void PianoRoll::setEditMode(int mode)
{
    m_ctrlMode = m_editMode = (EditModes)mode;
}

void PianoRoll::selectAll()
{
    if(m_pattern == nullptr)
        return;
    // if first_time = true, we HAVE to set the vars for select
    bool first_time = true;

    for(const Note* note: m_pattern->notes())
    {
        int len_ticks = static_cast<int>(note->length()) > 0
                                ? static_cast<int>(note->length())
                                : 1;

        const int key = note->key();

        int pos_ticks = note->pos();
        if(key <= m_selectStartKey || first_time)
        {
            // if we move start-key down, we have to add
            // the difference between old and new start-key
            // to m_selectedKeys, otherwise the selection
            // is just moved down
            m_selectedKeys += m_selectStartKey - (key - 1);
            m_selectStartKey = key - 1;
        }
        if(key >= m_selectedKeys + m_selectStartKey || first_time)
        {
            m_selectedKeys = key - m_selectStartKey;
        }
        if(pos_ticks < m_selectStartTick || first_time)
        {
            m_selectStartTick = pos_ticks;
        }
        if(pos_ticks + len_ticks > m_selectStartTick + m_selectedTick
           || first_time)
        {
            m_selectedTick = pos_ticks + len_ticks - m_selectStartTick;
        }
        first_time = false;
    }
}

// returns vector with pointers to all selected notes
Notes PianoRoll::getSelectedNotes()
{
    Notes selectedNotes;

    if(m_pattern != nullptr)
    {
        for(Note* note: m_pattern->notes())
        {
            if(note->selected())
            {
                selectedNotes.push_back(note);
            }
        }
    }
    return selectedNotes;
}

// selects all notess associated with m_lastKey
void PianoRoll::selectNotesOnKey()
{
    if(m_pattern != nullptr)
    {
        for(Note* note: m_pattern->notes())
        {
            if(note->key() == m_lastKey)
            {
                note->setSelected(true);
            }
        }
    }
}

void PianoRoll::enterValue(Notes& _notes)
{

    if(m_noteEditMode == NoteEditVolume)
    {
        bool ok;
        int  new_val;
        new_val = QInputDialog::getInt(
                this, "Piano roll: note velocity",
                tr("Please enter a new value between %1 and %2:")
                        .arg(MinVolume)
                        .arg(MaxVolume),
                _notes[0]->getVolume(), MinVolume, MaxVolume, 1, &ok);

        if(ok)
        {
            for(Note* n: _notes)
            {
                n->setVolume(new_val);
            }
            m_lastNoteVolume = new_val;
        }
    }
    else if(m_noteEditMode == NoteEditPanning)
    {
        bool ok;
        int  new_val;
        new_val = QInputDialog::getInt(
                this, "Piano roll: note panning",
                tr("Please enter a new value between %1 and %2:")
                        .arg(PanningLeft)
                        .arg(PanningRight),
                _notes[0]->getPanning(), PanningLeft, PanningRight, 1, &ok);

        if(ok)
        {
            for(Note* n: _notes)
            {
                n->setPanning(new_val);
            }
            m_lastNotePanning = new_val;
        }
    }
    else if(m_noteEditMode == NoteEditLegato)
    {
        bool ok;
        bool new_val;
        new_val = QInputDialog::getInt(
                this, "Piano roll: note legato",
                tr("Please enter a new value between %1 and %2:")
                        .arg(0)
                        .arg(1),
                _notes[0]->legato() ? 0 : 1, 0, 1, 1, &ok);

        if(ok)
        {
            m_lastNoteLegato = new_val;
            for(Note* n: _notes)
                n->setLegato(new_val);
        }
    }
    else if(m_noteEditMode == NoteEditMarcato)
    {
        bool ok;
        bool new_val;
        new_val = QInputDialog::getInt(
                this, "Piano roll: note marcato",
                tr("Please enter a new value between %1 and %2:")
                        .arg(0)
                        .arg(1),
                _notes[0]->marcato() ? 0 : 1, 0, 1, 1, &ok);

        if(ok)
        {
            m_lastNoteMarcato = new_val;
            for(Note* n: _notes)
                n->setMarcato(new_val);
        }
    }
    else if(m_noteEditMode == NoteEditStaccato)
    {
        bool ok;
        bool new_val;
        new_val = QInputDialog::getInt(
                this, "Piano roll: note staccato",
                tr("Please enter a new value between %1 and %2:")
                        .arg(0)
                        .arg(1),
                _notes[0]->staccato() ? 0 : 1, 0, 1, 1, &ok);

        if(ok)
        {
            m_lastNoteStaccato = new_val;
            for(Note* n: _notes)
                n->setStaccato(new_val);
        }
    }
}

void PianoRoll::copyToClipboard(const Notes& notes) const
{
    DataFile    dataFile(DataFile::ClipboardData);
    QDomElement note_list = dataFile.createElement("note-list");
    dataFile.content().appendChild(note_list);

    // MidiTime start_pos(notes.front()->pos().getTact(), 0);
    tick_t       start_pos = notes.front()->pos();
    const tick_t q         = quantization();
    if(q > 0)
        start_pos -= start_pos % q;

    for(const Note* note: notes)
    {
        Note clip_note(*note);
        clip_note.setPos(clip_note.pos() - start_pos);
        clip_note.saveState(dataFile, note_list);
    }

    QMimeData* clip_content = new QMimeData;
    clip_content->setData("text/x-note-list", dataFile.toString().toUtf8());
    QApplication::clipboard()->setMimeData(clip_content,
                                           QClipboard::Clipboard);
}

void PianoRoll::copySelectedNotes()
{
    Notes selected_notes = getSelectedNotes();

    if(!selected_notes.empty())
    {
        copyToClipboard(selected_notes);
    }
}

void PianoRoll::cutSelectedNotes()
{
    if(m_pattern == nullptr)
        return;
    Notes selected_notes = getSelectedNotes();

    if(!selected_notes.empty())
    {
        copyToClipboard(selected_notes);

        Engine::getSong()->setModified();

        for(Note* note: selected_notes)
        {
            // note (the memory of it) is also deleted by
            // pattern::removeNote(...) so we don't have to do that
            m_pattern->removeNote(note);
        }
    }

    update();
    gui->songWindow()->update();
}

void PianoRoll::pasteNotes()
{
    if(m_pattern == nullptr)
        return;
    QString value = QApplication::clipboard()
                            ->mimeData(QClipboard::Clipboard)
                            ->data("text/x-note-list");

    if(!value.isEmpty())
    {
        DataFile dataFile(value.toUtf8());

        QDomNodeList list = dataFile.elementsByTagName(Note::classNodeName());

        // remove selection and select the newly pasted notes
        clearSelectedNotes();

        if(!list.isEmpty())
        {
            m_pattern->addJournalCheckPoint();
        }

        tick_t delta = m_currentPosition;
        QPoint mouse = mapFromGlobal(QCursor::pos());
        if(mouse.x() <= WHITE_KEY_WIDTH
           || mouse.x() > width() - SCROLLBAR_SIZE
           || mouse.y() < PR_TOP_MARGIN || mouse.y() > keyAreaBottom())
        {
            // width of the piano roll in ticks
            const tick_t wt = (width() - WHITE_KEY_WIDTH - SCROLLBAR_SIZE)
                              * MidiTime::ticksPerTact() / m_ppt;

            if(m_timeLine->pos() < m_currentPosition
               || m_timeLine->pos() >= m_currentPosition + wt)
                delta += wt / 2;
            else
                delta = m_timeLine->pos();
        }
        else
        {
            delta += (mouse.x() - WHITE_KEY_WIDTH) * MidiTime::ticksPerTact()
                     / m_ppt;
        }
        const tick_t q = quantization();
        qInfo("PianoRoll::pasteNotes cp=%d delta=%d q=%d",
              m_currentPosition.getTicks(), delta, q);
        if(q > 0)
        {
            delta -= delta % q;
            if(delta < m_currentPosition)
                delta += q;
        }

        bool modified = false;
        for(int i = 0; !list.item(i).isNull(); ++i)
        {
            // create the note
            Note cur_note;
            cur_note.restoreState(list.item(i).toElement());
            cur_note.setPos(cur_note.pos() + delta);  // m_timeLine->pos());

            // select it
            cur_note.setSelected(true);

            // add to pattern
            m_pattern->addNote(cur_note, false);
            modified = true;
        }

        if(modified)
        {
            // we only have to do the following lines if we pasted at
            // least one note
            Engine::getSong()->setModified();
            update();
            gui->songWindow()->update();
        }
    }
}

void PianoRoll::deleteSelectedNotes()
{
    if(m_pattern == nullptr)
        return;

    bool update_after_delete = false;

    m_pattern->addJournalCheckPoint();

    // get note-vector of current pattern
    const Notes& notes = m_pattern->notes();

    // will be our iterator in the following loop
    Notes::ConstIterator it = notes.begin();
    while(it != notes.end())
    {
        Note* note = *it;
        if(note->selected())
        {
            // delete this note
            m_pattern->removeNote(note);
            update_after_delete = true;

            // start over, make sure we get all the notes
            it = notes.begin();
        }
        else
        {
            ++it;
        }
    }

    if(update_after_delete)
    {
        Engine::getSong()->setModified();
        update();
        gui->songWindow()->update();
    }
}

void PianoRoll::autoScroll(const MidiTime& t)
{
    const int w = width() - WHITE_KEY_WIDTH;
    if(t > m_currentPosition + w * MidiTime::ticksPerTact() / m_ppt)
    {
        m_leftRightScroll->setValue(t.getTact() * MidiTime::ticksPerTact());
    }
    else if(t < m_currentPosition)
    {
        MidiTime t2 = qMax<MidiTime>(
                t
                        - w * MidiTime::ticksPerTact()
                                  * MidiTime::ticksPerTact() / m_ppt,
                0);
        m_leftRightScroll->setValue(t2.getTact() * MidiTime::ticksPerTact());
    }
    m_scrollBack = false;
}

void PianoRoll::updatePosition(const MidiTime& t)
{
    if((Engine::getSong()->isPlaying()
        && Engine::getSong()->playMode() == Song::Mode_PlayPattern
        && m_timeLine->autoScroll() == TimeLineWidget::AutoScrollEnabled)
       || m_scrollBack)
    {
        autoScroll(t);
    }
}

void PianoRoll::updatePositionAccompany(const MidiTime& t)
{
    Song* s = Engine::getSong();

    if(m_recording && m_pattern != nullptr
       && s->playMode() != Song::Mode_PlayPattern)
    {
        MidiTime pos = t;
        if(s->playMode() != Song::Mode_PlayBB)
        {
            pos -= m_pattern->startPosition();
        }
        if((int)pos > 0)
        {
            s->getPlayPos(Song::Mode_PlayPattern).setTicks(pos);
            autoScroll(pos);
        }
    }
}

void PianoRoll::zoomingXChanged()
{
    m_ppt = EditorWindow::ZOOM_LEVELS[m_zoomingXModel.value()] * DEFAULT_PR_PPT;

    Q_ASSERT(m_ppt > 0.f);

    m_timeLine->setPixelsPerTact(m_ppt);
    update();
}

void PianoRoll::quantizeChanged()
{
    update();
}

tick_t PianoRoll::quantization() const
{
    int v = m_quantizeModel.value();
    if(v == 0)
        return (m_noteLenModel.value() > 0)
                       ? newNoteLen().getTicks()
                       : tick_t(DefaultTicksPerTact / 16);

    // QString text = m_quantizeModel.currentText();
    // return DefaultTicksPerTact / text.right( text.length() - 2
    // ).toInt();

    return EditorWindow::QUANTIZE_LEVELS[v - 1];
}

void PianoRoll::quantizeNotes()
{
    if(m_pattern == nullptr)
        return;

    m_pattern->addJournalCheckPoint();

    Notes notes = getSelectedNotes();

    if(notes.empty())
    {
        for(Note* n: m_pattern->notes())
        {
            notes.push_back(n);
        }
    }

    for(Note* n: notes)
    {
        if(n->length() == MidiTime(0))
        {
            continue;
        }

        Note copy(*n);
        m_pattern->removeNote(n);
        copy.quantizePos(quantization());
        m_pattern->addNote(copy);
    }

    update();
    gui->songWindow()->update();
    Engine::getSong()->setModified();
}

void PianoRoll::updateSemiToneMarkerMenu()
{
    const InstrumentFunctionNoteStacking::ChordTable& chord_table
            = InstrumentFunctionNoteStacking::ChordTable::getInstance();
    const InstrumentFunctionNoteStacking::Chord& scale
            = chord_table.getScaleByName(m_scaleModel.currentText());
    const InstrumentFunctionNoteStacking::Chord& chord
            = chord_table.getChordByName(m_chordModel.currentText());

    emit semiToneMarkerMenuScaleSetEnabled(!scale.isEmpty());
    emit semiToneMarkerMenuChordSetEnabled(!chord.isEmpty());
}

MidiTime PianoRoll::newNoteLen() const
{
    int v = m_noteLenModel.value();
    if(v == 0)
        return m_lenOfNewNotes;

    // QString text = m_noteLenModel.currentText();
    // return DefaultTicksPerTact / text.right(text.length() - 2).toInt();
    return EditorWindow::LENGTH_LEVELS[v - 1];
}

bool PianoRoll::mouseOverNote()
{
    return m_pattern != nullptr && noteUnderMouse() != nullptr;
}

Note* PianoRoll::noteUnderMouse()
{
    QPoint pos = mapFromGlobal(QCursor::pos());

    if(pos.x() <= WHITE_KEY_WIDTH || pos.x() > width() - SCROLLBAR_SIZE
       || pos.y() < PR_TOP_MARGIN || pos.y() > keyAreaBottom())
    {
        return nullptr;
    }

    int key_num = getKey(pos.y());
    int pos_ticks
            = (pos.x() - WHITE_KEY_WIDTH) * MidiTime::ticksPerTact() / m_ppt
              + m_currentPosition;

    // loop through whole note-vector
    for(Note* const& note: m_pattern->notes())
    {
        // and check whether the cursor is over an
        // existing note
        if(pos_ticks >= note->pos() && pos_ticks <= note->endPos()
           && note->key() == key_num && note->length() > 0)
        {
            return note;
        }
    }

    return nullptr;
}

PianoRollWindow::PianoRollWindow() :
      EditorWindow(true), m_editor(new PianoRoll())
{
    setCentralWidget(m_editor);

    m_playAction->setToolTip(tr("Play/pause current pattern (Space)"));
    m_recordAction->setToolTip(
            tr("Record notes from MIDI-device/channel-piano"));
    m_recordAccompanyAction->setToolTip(
            tr("Record notes from MIDI-device/channel-piano while playing "
               "song or BB track"));
    m_stopAction->setToolTip(tr("Stop playing of current pattern (Space)"));

    m_playAction->setWhatsThis(
            tr("Click here to play the current pattern. "
               "This is useful while editing it. The pattern is "
               "automatically looped when its end is reached."));
    m_recordAction->setWhatsThis(
            tr("Click here to record notes from a MIDI-"
               "device or the virtual test-piano of the according "
               "channel-window to the current pattern. When recording "
               "all notes you play will be written to this pattern "
               "and you can play and edit them afterwards."));
    m_recordAccompanyAction->setWhatsThis(
            tr("Click here to record notes from a MIDI-"
               "device or the virtual test-piano of the according "
               "channel-window to the current pattern. When recording "
               "all notes you play will be written to this pattern "
               "and you will hear the song or BB track in the background."));
    m_stopAction->setWhatsThis(
            tr("Click here to stop playback of current pattern."));

    DropToolBar* notesActionsToolBar
            = addDropToolBarToTop(tr("Edit actions"));

    // init edit-buttons at the top
    ActionGroup* editModeGroup = new ActionGroup(this);
    QAction*     drawAction    = editModeGroup->addAction(
            embed::getIconPixmap("edit_draw"), TR("Draw mode (Shift+D)"));
    QAction* eraseAction = editModeGroup->addAction(
            embed::getIconPixmap("edit_erase"), TR("Erase mode (Shift+E)"));
    QAction* selectAction = editModeGroup->addAction(
            embed::getIconPixmap("edit_select"), TR("Select mode (Shift+S)"));
    QAction* detuneAction = editModeGroup->addAction(
            embed::getIconPixmap("automation"), TR("Detune mode (Shift+T)"));

    drawAction->setChecked(true);

    drawAction->setShortcut(Qt::SHIFT | Qt::Key_D);
    eraseAction->setShortcut(Qt::SHIFT | Qt::Key_E);
    selectAction->setShortcut(Qt::SHIFT | Qt::Key_S);
    detuneAction->setShortcut(Qt::SHIFT | Qt::Key_T);

    drawAction->setWhatsThis(
            tr("Click here and draw mode will be activated. In this "
               "mode you can add, resize and move notes. This "
               "is the default mode which is used most of the time. "
               "You can also press 'Shift+D' on your keyboard to "
               "activate this mode. In this mode, hold %1 to "
               "temporarily go into select mode.")
                    .arg(UI_CTRL_KEY));
    eraseAction->setWhatsThis(
            tr("Click here and erase mode will be activated. In this "
               "mode you can erase notes. You can also press "
               "'Shift+E' on your keyboard to activate this mode."));
    selectAction->setWhatsThis(
            tr("Click here and select mode will be activated. "
               "In this mode you can select notes. Alternatively, "
               "you can hold '%1' in draw mode to temporarily use "
               "select mode.")
                    .arg(UI_CTRL_KEY));
    detuneAction->setWhatsThis(
            tr("Click here and detune mode will be activated. "
               "In this mode you can click a note to open its "
               "automation detuning. You can utilize this to slide "
               "notes from one to another. You can also press "
               "'Shift+T' on your keyboard to activate this mode."));

    connect(editModeGroup, SIGNAL(triggered(int)), m_editor,
            SLOT(setEditMode(int)));

    QAction* quantizeAction
            = new QAction(embed::getIcon("quantize"), tr("Quantize"), this);
    connect(quantizeAction, SIGNAL(triggered()), m_editor,
            SLOT(quantizeNotes()));

    notesActionsToolBar->addAction(drawAction);
    notesActionsToolBar->addAction(eraseAction);
    notesActionsToolBar->addAction(selectAction);
    notesActionsToolBar->addAction(detuneAction);

    notesActionsToolBar->addSeparator();
    notesActionsToolBar->addAction(quantizeAction);

    // Copy + paste actions
    DropToolBar* copyPasteActionsToolBar
            = addDropToolBarToTop(tr("Copy paste controls"));
    copyPasteActionsToolBar->addSeparator();

    QAction* cutAction = new QAction(
            embed::getIconPixmap("edit_cut"),
            tr("Cut selected notes (%1+X)").arg(UI_CTRL_KEY), this);

    QAction* copyAction = new QAction(
            embed::getIconPixmap("edit_copy"),
            tr("Copy selected notes (%1+C)").arg(UI_CTRL_KEY), this);

    QAction* pasteAction = new QAction(
            embed::getIconPixmap("edit_paste"),
            tr("Paste notes from clipboard (%1+V)").arg(UI_CTRL_KEY), this);

    cutAction->setWhatsThis(
            tr("Click here and the selected notes will be cut into the "
               "clipboard. You can paste them anywhere in any pattern "
               "by clicking on the paste button."));
    copyAction->setWhatsThis(
            tr("Click here and the selected notes will be copied into the "
               "clipboard. You can paste them anywhere in any pattern "
               "by clicking on the paste button."));
    pasteAction->setWhatsThis(
            tr("Click here and the notes from the clipboard will be "
               "pasted at the first visible measure."));

    cutAction->setShortcut(Qt::CTRL | Qt::Key_X);
    copyAction->setShortcut(Qt::CTRL | Qt::Key_C);
    pasteAction->setShortcut(Qt::CTRL | Qt::Key_V);

    connect(cutAction, SIGNAL(triggered()), m_editor,
            SLOT(cutSelectedNotes()));
    connect(copyAction, SIGNAL(triggered()), m_editor,
            SLOT(copySelectedNotes()));
    connect(pasteAction, SIGNAL(triggered()), m_editor, SLOT(pasteNotes()));

    copyPasteActionsToolBar->addAction(cutAction);
    copyPasteActionsToolBar->addAction(copyAction);
    copyPasteActionsToolBar->addAction(pasteAction);

    /*DropToolBar *timeLineToolBar =*/addDropToolBarToTop(
            tr("Timeline controls"));
    // m_editor->m_timeLine->addToolButtons( timeLineToolBar );

    addToolBarBreak();

    DropToolBar* zoomAndNotesToolBar
            = addDropToolBarToTop(tr("Zoom and note controls"));

    // setup zoom-stuff
    QLabel* zoomLBL = new QLabel(m_toolBar);
    zoomLBL->setPixmap(embed::getIconPixmap("zoom_x"));
    zoomLBL->setFixedSize(32, 32);
    zoomLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_zoomingXComboBox = new ComboBox(m_toolBar, "[zoom x]");
    m_zoomingXComboBox->setModel(&m_editor->m_zoomingXModel);
    m_zoomingXComboBox->setFixedSize(70, 32);

    new QShortcut(Qt::Key_Minus, m_zoomingXComboBox, SLOT(selectPrevious()));
    new QShortcut(Qt::Key_Plus, m_zoomingXComboBox, SLOT(selectNext()));

    // setup quantize-stuff
    QLabel* quantizeLBL = new QLabel(m_toolBar);
    quantizeLBL->setPixmap(embed::getIconPixmap("quantize"));
    quantizeLBL->setFixedSize(32, 32);
    quantizeLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_quantizeComboBox = new ComboBox(m_toolBar);
    m_quantizeComboBox->setModel(&m_editor->m_quantizeModel);
    m_quantizeComboBox->setFixedSize(70, 32);

    // setup note-len-stuff
    QLabel* note_lenLBL = new QLabel(m_toolBar);
    note_lenLBL->setPixmap(embed::getIconPixmap("note"));
    note_lenLBL->setFixedSize(32, 32);
    note_lenLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_noteLenComboBox = new ComboBox(m_toolBar, "[note length]");
    m_noteLenComboBox->setModel(&m_editor->m_noteLenModel);
    m_noteLenComboBox->setFixedSize(105, 32);

    // setup root-stuff
    QLabel* rootLBL = new QLabel(m_toolBar);
    rootLBL->setPixmap(embed::getIconPixmap("chord_root"));
    rootLBL->setFixedSize(32, 32);
    rootLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_rootComboBox = new ComboBox(m_toolBar, "[root note]");
    m_rootComboBox->setModel(&m_editor->m_rootModel);
    m_rootComboBox->setFixedSize(70, 32);

    // setup scale-stuff
    QLabel* scaleLBL = new QLabel(m_toolBar);
    scaleLBL->setPixmap(embed::getIconPixmap("scale"));
    scaleLBL->setFixedSize(32, 32);
    scaleLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_scaleComboBox = new ComboBox(m_toolBar, "[scale]");
    m_scaleComboBox->setModel(&m_editor->m_scaleModel);
    m_scaleComboBox->setFixedSize(105, 32);

    // Guess scale
    m_guessScaleButton = new QToolButton(m_toolBar);
    m_guessScaleButton->setIcon(embed::getIconPixmap("guess_scale"));
    m_guessScaleButton->setToolTip(tr("Guess scale"));
    // m_guessScaleButton->setFixedSize(32, 32);
    // m_guessScaleButton->setEnabled(false);
    connect(m_guessScaleButton, SIGNAL(clicked()), m_editor,
            SLOT(guessScale()));

    // setup chord-stuff
    QLabel* chordLBL = new QLabel(m_toolBar);
    chordLBL->setPixmap(embed::getIconPixmap("chord"));
    chordLBL->setFixedSize(32, 32);
    chordLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_chordComboBox = new ComboBox(m_toolBar, "[chord]");
    m_chordComboBox->setModel(&m_editor->m_chordModel);
    m_chordComboBox->setFixedSize(105, 32);

    // -- Clear ghost pattern button
    m_clearGhostButton = new QToolButton(m_toolBar);
    m_clearGhostButton->setIcon(embed::getIconPixmap("clear_ghost_note"));
    m_clearGhostButton->setToolTip(tr("Clear ghost notes"));
    // m_clearGhostButton->setFixedSize(105, 32);
    m_clearGhostButton->setEnabled(false);
    connect(m_clearGhostButton, SIGNAL(clicked()), m_editor,
            SLOT(clearGhostPattern()));
    connect(m_editor, SIGNAL(ghostPatternSet(bool)), this,
            SLOT(ghostPatternSet(bool)));

    QToolButton* downChordBTN = new QToolButton(m_toolBar);
    downChordBTN->setIcon(embed::getIcon("arrow_down"));
    downChordBTN->setToolTip(tr("Down the selection"));
    connect(downChordBTN, SIGNAL(clicked()), m_editor, SLOT(downChord()));

    QToolButton* upChordBTN = new QToolButton(m_toolBar);
    upChordBTN->setIcon(embed::getIcon("arrow_up"));
    upChordBTN->setToolTip(tr("Up the selection"));
    connect(upChordBTN, SIGNAL(clicked()), m_editor, SLOT(upChord()));

    zoomAndNotesToolBar->addWidget(zoomLBL);
    zoomAndNotesToolBar->addWidget(m_zoomingXComboBox);

    // zoomAndNotesToolBar->addSeparator();
    zoomAndNotesToolBar->addWidget(quantizeLBL);
    zoomAndNotesToolBar->addWidget(m_quantizeComboBox);

    // zoomAndNotesToolBar->addSeparator();
    zoomAndNotesToolBar->addWidget(note_lenLBL);
    zoomAndNotesToolBar->addWidget(m_noteLenComboBox);

    // zoomAndNotesToolBar->addSeparator();
    zoomAndNotesToolBar->addWidget(rootLBL);
    zoomAndNotesToolBar->addWidget(m_rootComboBox);

    // zoomAndNotesToolBar->addSeparator();
    zoomAndNotesToolBar->addWidget(scaleLBL);
    zoomAndNotesToolBar->addWidget(m_scaleComboBox);
    // zoomAndNotesToolBar->addWidget(m_guessScaleButton);
    notesActionsToolBar->addWidget(m_guessScaleButton);
    notesActionsToolBar->addWidget(m_clearGhostButton);

    // zoomAndNotesToolBar->addSeparator();
    zoomAndNotesToolBar->addWidget(chordLBL);
    zoomAndNotesToolBar->addWidget(m_chordComboBox);
    zoomAndNotesToolBar->addSeparator();
    zoomAndNotesToolBar->addWidget(upChordBTN);
    zoomAndNotesToolBar->addWidget(downChordBTN);

    m_zoomingXComboBox->setWhatsThis(
            tr("This controls the magnification of an axis. "
               "It can be helpful to choose magnification for a specific "
               "task. For ordinary editing, the magnification should be "
               "fitted to your smallest notes. "));

    m_quantizeComboBox->setWhatsThis(
            tr("The 'Q' stands for quantization, and controls the grid size "
               "notes and control points snap to. "
               "With smaller quantization values, you can draw shorter "
               "notes "
               "in Piano Roll, and more exact control points in the "
               "Automation Editor."

               ));

    m_noteLenComboBox->setWhatsThis(
            tr("This lets you select the length of new notes. "
               "'Last Note' means that LMMS will use the note length of "
               "the note you last edited"));

    m_scaleComboBox->setWhatsThis(
            tr("The feature is directly connected to the context-menu "
               "on the virtual keyboard, to the left in Piano Roll. "
               "After you have chosen the scale you want "
               "in this drop-down menu, "
               "you can right click on a desired key in the virtual "
               "keyboard, "
               "and then choose 'Mark current Scale'. "
               "LMMS will highlight all notes that belongs to the chosen "
               "scale, "
               "and in the key you have selected!"));

    m_chordComboBox->setWhatsThis(
            tr("Let you select a chord which LMMS then can draw or "
               "highlight."
               "You can find the most common chords in this drop-down menu. "
               "After you have selected a chord, click anywhere to place "
               "the "
               "chord, and right "
               "click on the virtual keyboard to open context menu and "
               "highlight the chord. "
               "To return to single note placement, you need to choose 'No "
               "chord' "
               "in this drop-down menu."));

    // setup our actual window
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setWindowIcon(embed::getIconPixmap("piano"));
    setCurrentPattern(nullptr);

    // Connections
    connect(m_editor, SIGNAL(currentPatternChanged()), this,
            SIGNAL(currentPatternChanged()));
    connect(m_editor, SIGNAL(currentPatternChanged()), this,
            SLOT(updateWindowTitle()));
}

const Pattern* PianoRollWindow::currentPattern() const
{
    return m_editor->currentPattern();
}

const Pattern* PianoRollWindow::ghostPattern() const
{
    return m_editor->ghostPattern();
}

void PianoRollWindow::setCurrentPattern(Pattern* pattern)
{
    m_editor->setCurrentPattern(pattern);

    if(pattern)
    {
        connect(pattern->instrumentTrack(), SIGNAL(nameChanged()), this,
                SLOT(updateWindowTitle()));
        connect(pattern, SIGNAL(dataChanged()), this,
                SLOT(updateWindowTitle()));
    }

    updateWindowTitle();
}

void PianoRollWindow::setGhostPattern(Pattern* pattern)
{
    m_editor->setGhostPattern(pattern);
}

bool PianoRollWindow::isRecording() const
{
    return m_editor->isRecording();
}

int PianoRollWindow::quantization() const
{
    return m_editor->quantization();
}

void PianoRollWindow::play()
{
    m_editor->play();
}

void PianoRollWindow::stop()
{
    m_editor->stop();
}

void PianoRollWindow::record()
{
    m_editor->record();
}

void PianoRollWindow::recordAccompany()
{
    m_editor->recordAccompany();
}

void PianoRollWindow::stopRecording()
{
    m_editor->stopRecording();
}

void PianoRollWindow::reset()
{
    m_editor->reset();
}

void PianoRollWindow::saveSettings(QDomDocument& doc, QDomElement& de)
{
    MainWindow::saveWidgetState(this, de);
}

void PianoRollWindow::loadSettings(const QDomElement& de)
{
    MainWindow::restoreWidgetState(this, de);
}

QSize PianoRollWindow::sizeHint() const
{
    // const int INITIAL_PIANOROLL_WIDTH  = 860;
    // const int INITIAL_PIANOROLL_HEIGHT = 480;
    // return {INITIAL_PIANOROLL_WIDTH, INITIAL_PIANOROLL_HEIGHT};
    return {860, 480};
}

void PianoRollWindow::updateWindowTitle()
{
    const Pattern* p = currentPattern();
    setWindowTitle(p != nullptr ? tr("Piano Roll - %1").arg(p->name())
                                : tr("Piano Roll - no tile"));
}

void PianoRollWindow::ghostPatternSet(bool state)
{
    m_clearGhostButton->setEnabled(state);
}

void PianoRollWindow::focusInEvent(QFocusEvent* event)
{
    // when the window is given focus, also give focus to the actual piano
    // roll
    m_editor->setFocus(event->reason());
}
