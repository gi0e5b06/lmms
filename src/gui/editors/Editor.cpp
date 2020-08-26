/*
 * Editor.cpp -
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2014 Lukas W <lukaswhl/at/gmail.com>
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

#include "Editor.h"

#include "ActionGroup.h"
#include "Backtrace.h"
#include "ComboBoxModel.h"
#include "EditorOverlay.h"
#include "Song.h"
#include "embed.h"
#include "lmms_qt_core.h"

#include <QAction>
#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QShortcut>
#include <QWidget>

Editor::Editor(Model*         _parent,
               const QString& _displayName,
               const QString& _objectName) :
      m_editMode(EditMode::ModeDraw),
      m_overlay(nullptr), m_editorModel(_parent, _displayName, _objectName)
{
}

Editor::~Editor()
{
}

EditorOverlay* Editor::overlay() const
{
    return m_overlay;
}

void Editor::setOverlay(EditorOverlay* _overlay)
{
    m_overlay = _overlay;
}

Editor::EditMode Editor::editMode() const
{
    return m_editMode;
}

Editor::EditMode Editor::cursorMode() const
{
    return editMode();
}

bool Editor::isEditMode(EditMode _mode) const
{
    return m_editMode == _mode;
}

void Editor::setEditMode(EditMode _mode)
{
    qInfo("::setEditMode mode=%d (was %d)", _mode, (int)m_editMode);
    if(m_editMode != _mode)
    {
        m_editMode = _mode;
        if(CONFIG_GET_BOOL("ui.toolcursor"))
            if(m_overlay != nullptr)
                m_overlay->update();
    }
}

/*
const QWidget* Editor::widget()
{
    const QWidget* w = dynamic_cast<QWidget*>(this);
    if(w == nullptr)
    {
        BACKTRACE
        qCritical("Editor must be a widget");
    }
    return w;
}
*/

/*
void Editor::drawModeCursor(QPainter& _p, QWidget& _w, EditMode _mode)
{
    static QPixmap* s_modeDraw   = nullptr;
    static QPixmap* s_modeErase  = nullptr;
    static QPixmap* s_modeSelect = nullptr;
    static QPixmap* s_modeDetune = nullptr;

    const QPixmap* cursor = nullptr;

    switch(_mode)
    {
        case ModeDraw:
            if(s_modeDraw == nullptr)
                s_modeDraw = new QPixmap(embed::getPixmap("edit_draw"));
            cursor = s_modeDraw;
            break;
        case ModeErase:
            if(s_modeErase == nullptr)
                s_modeErase = new QPixmap(embed::getPixmap("edit_erase"));
            cursor = s_modeErase;
            break;
        case ModeSelect:
            if(s_modeSelect == nullptr)
                s_modeSelect = new QPixmap(embed::getPixmap("edit_select"));
            cursor = s_modeSelect;
            break;
        case ModeDetune:
            if(s_modeDetune == nullptr)
                s_modeDetune = new QPixmap(embed::getPixmap("automation"));
            cursor = s_modeDetune;
            break;
    }

    if(cursor != nullptr)
    {
        QPoint q = _w->mapFromGlobal(QCursor::pos());
        if(_w->rect().contains(q))
            _p.drawPixmap(q + QPoint(8, 8), *cursor);
    }
}
*/

static int cursor_count = 0;

void Editor::applyOverrideCursor(Qt::CursorShape _shape)
{
    QCursor c(_shape);
    applyOverrideCursor(c);
}

void Editor::applyOverrideCursor(QCursor& _c)
{
    if(cursor_count > 0)
    {
        QApplication::changeOverrideCursor(_c);
    }
    else
    {
        QApplication::setOverrideCursor(_c);
        cursor_count++;
    }
}

void Editor::resetOverrideCursor()
{
    if(cursor_count > 0 && QApplication::overrideCursor() != nullptr)
    {
        QApplication::restoreOverrideCursor();
        cursor_count--;
    }
}

const QVector<real_t> EditorWindow::ZOOM_LEVELS
        = {0.10, 0.20, 0.50, 0.75, 1.00, 1.50, 2.0, 5.00, 10.00, 20.00};

void EditorWindow::fillZoomLevels(ComboBoxModel& _model, bool _automatic)
{
    if(_automatic)
    {
        static ComboBoxModel::Items items;
        if(items.isEmpty())
        {
            items.append(ComboBoxModel::Item(-1, QObject::tr("Auto"), nullptr,
                                             -1));
            int i = 0;
            for(const real_t& zoomLevel: EditorWindow::ZOOM_LEVELS)
            {
                int     n    = int(zoomLevel * 100);
                QString text = QString("%1%").arg(n);
                items.append(ComboBoxModel::Item(i, text, nullptr, n));
                i++;
            }
        }
        _model.setItems(&items);
    }
    else
    {
        static ComboBoxModel::Items items;
        if(items.isEmpty())
        {
            int i = 0;
            for(const real_t& zoomLevel: EditorWindow::ZOOM_LEVELS)
            {
                int     n    = int(zoomLevel * 100);
                QString text = QString("%1%").arg(n);
                items.append(ComboBoxModel::Item(i, text, nullptr, n));
                i++;
            }
        }
        _model.setItems(&items);
    }
}

/*
_cbm.addItem("4/1", new PixmapLoader("note_four_whole"));
_cbm.addItem("2/1", new PixmapLoader("note_double_whole"));
_cbm.addItem("1/1", new PixmapLoader("note_whole"));
_cbm.addItem("1/2", new PixmapLoader("note_half"));
_cbm.addItem("1/4", new PixmapLoader("note_quarter"));
_cbm.addItem("1/8", new PixmapLoader("note_eighth"));
_cbm.addItem("1/16", new PixmapLoader("note_sixteenth"));
_cbm.addItem("1/32", new PixmapLoader("note_thirtysecond"));
_cbm.addItem("1/64", new PixmapLoader("note_sixtyfourth"));
_cbm.addItem("1/3", new PixmapLoader("note_triplethalf"));
_cbm.addItem("1/6", new PixmapLoader("note_tripletquarter"));
_cbm.addItem("1/12", new PixmapLoader("note_tripleteighth"));
_cbm.addItem("1/24", new PixmapLoader("note_tripletsixteenth"));
_cbm.addItem("1/48", new PixmapLoader("note_tripletthirtysecond"));
_cbm.addItem("1/96", new PixmapLoader("note_tripletsixtyfourth"));
_cbm.addItem("1/192", new PixmapLoader("note_tick"));
*/

const QVector<tick_t> EditorWindow::QUANTIZE_LEVELS
        = {8 * 192,  4 * 192,  2 * 192,  1 * 192,  192 / 2, 192 / 4,
           192 / 8,  192 / 16, 192 / 32, 192 / 64, 192 / 3, 192 / 6,
           192 / 12, 192 / 24, 192 / 48, 192 / 96, 1};

void EditorWindow::fillQuantizeLevels(ComboBoxModel& _model, bool _noteLock)
{
    /*
    _cbm.addItem("4/1");
    _cbm.addItem("2/1");
    _cbm.addItem("1/1");
    _cbm.addItem("1/2");
    _cbm.addItem("1/4");
    _cbm.addItem("1/8");
    _cbm.addItem("1/16");
    _cbm.addItem("1/32");
    _cbm.addItem("1/64");
    _cbm.addItem("1/3");
    _cbm.addItem("1/6");
    _cbm.addItem("1/12");
    _cbm.addItem("1/24");
    _cbm.addItem("1/48");
    _cbm.addItem("1/96");
    _cbm.addItem("1/192");
    */
    if(_noteLock)
    {
        static ComboBoxModel::Items items;
        if(items.isEmpty())
        {
            items.append(ComboBoxModel::Item(-1, QObject::tr("Note lock"),
                                             nullptr, -1));
            int i = 0;
            for(const tick_t& q: EditorWindow::QUANTIZE_LEVELS)
            {
                int     n    = (q >= 192 ? q / 192 : 1);
                int     d    = (q >= 192 ? 1 : 192 / q);
                QString text = QString("%1/%2").arg(n).arg(d);
                items.append(ComboBoxModel::Item(i, text, nullptr, q));
                i++;
            }
        }
        _model.setItems(&items);
    }
    else
    {
        static ComboBoxModel::Items items;
        if(items.isEmpty())
        {
            int i = 0;
            for(const tick_t& q: EditorWindow::QUANTIZE_LEVELS)
            {
                int     n    = (q >= 192 ? q / 192 : 1);
                int     d    = (q >= 192 ? 1 : 192 / q);
                QString text = QString("%1/%2").arg(n).arg(d);
                items.append(ComboBoxModel::Item(i, text, nullptr, q));
                i++;
            }
        }
        _model.setItems(&items);
    }
}

const QVector<tick_t> EditorWindow::LENGTH_LEVELS
        = {8 * 192,  4 * 192,  2 * 192,  1 * 192,  192 / 2, 192 / 4,
           192 / 8,  192 / 16, 192 / 32, 192 / 64, 192 / 3, 192 / 6,
           192 / 12, 192 / 24, 192 / 48, 192 / 96, 1};

void EditorWindow::fillLengthLevels(ComboBoxModel& _model, bool _lastNote)
{
    if(_lastNote)
    {
        static ComboBoxModel::Items items;
        if(items.isEmpty())
        {
            items.append(ComboBoxModel::Item(-1, QObject::tr("Last note"),
                                             new PixmapLoader("edit_draw"),
                                             -1));
            int i = 0;
            for(const tick_t& q: EditorWindow::LENGTH_LEVELS)
            {
                int     n    = (q >= 192 ? q / 192 : 1);
                int     d    = (q >= 192 ? 1 : 192 / q);
                QString text = QString("%1/%2").arg(n).arg(d);
                items.append(ComboBoxModel::Item(i, text, nullptr, q));
                i++;
            }
        }
        _model.setItems(&items);
    }
    else
    {
        static ComboBoxModel::Items items;
        if(items.isEmpty())
        {
            int i = 0;
            for(const tick_t& q: EditorWindow::LENGTH_LEVELS)
            {
                int     n    = (q >= 192 ? q / 192 : 1);
                int     d    = (q >= 192 ? 1 : 192 / q);
                QString text = QString("%1/%2").arg(n).arg(d);
                items.append(ComboBoxModel::Item(i, text, nullptr, q));
                i++;
            }
        }
        _model.setItems(&items);
    }
}

void EditorWindow::setPauseIcon(bool displayPauseIcon)
{
    // If we're playing, show a pause icon
    if(displayPauseIcon)
        m_playAction->setIcon(embed::getIcon("pause"));
    else
        m_playAction->setIcon(embed::getIcon("play"));
}

DropToolBar* EditorWindow::addDropToolBarToTop(QString const& windowTitle)
{
    return addDropToolBar(Qt::TopToolBarArea, windowTitle);
}

DropToolBar* EditorWindow::addDropToolBar(Qt::ToolBarArea whereToAdd,
                                          QString const&  windowTitle)
{
    return addDropToolBar(this, whereToAdd, windowTitle);
}

DropToolBar* EditorWindow::addDropToolBar(QWidget*        parent,
                                          Qt::ToolBarArea whereToAdd,
                                          QString const&  windowTitle)
{
    DropToolBar* toolBar = new DropToolBar(parent);
    addToolBar(whereToAdd, toolBar);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    toolBar->setWindowTitle(windowTitle);

    return toolBar;
}

void EditorWindow::togglePlayStop()
{
    if(Engine::getSong()->isPlaying())
        stop();
    else
        play();
    requireActionUpdate();
}

EditorWindow::EditorWindow(bool record) :
      m_toolBar(new DropToolBar(this)), m_playAction(nullptr),
      m_recordAction(nullptr), m_recordAccompanyAction(nullptr),
      m_stopAction(nullptr)
{
    // setAttribute(Qt::WA_DontCreateNativeAncestors);
    // setAttribute(Qt::WA_NativeWindow);

    m_toolBar = addDropToolBarToTop(tr("Transport controls"));

    auto addButton = [this](QAction* action, QString objectName) {
        m_toolBar->addAction(action);
        m_toolBar->widgetForAction(action)->setObjectName(objectName);
    };

    // Set up play and record actions
    m_playAction
            = new QAction(embed::getIcon("play"), tr("Play (Space)"), this);
    m_stopAction
            = new QAction(embed::getIcon("stop"), tr("Stop (Space)"), this);

    m_recordAction
            = new QAction(embed::getIcon("record"), tr("Record"), this);
    m_recordAccompanyAction = new QAction(embed::getIcon("record_accompany"),
                                          tr("Record while playing"), this);

    // Set up connections
    connect(m_playAction, SIGNAL(triggered()), this, SLOT(play()));
    connect(m_recordAction, SIGNAL(triggered()), this, SLOT(record()));
    connect(m_recordAccompanyAction, SIGNAL(triggered()), this,
            SLOT(recordAccompany()));
    connect(m_stopAction, SIGNAL(triggered()), this, SLOT(stop()));
    new QShortcut(Qt::Key_Space, this, SLOT(togglePlayStop()));

    // Add actions to toolbar
    addButton(m_playAction, "playButton");
    if(record)
    {
        addButton(m_recordAction, "recordButton");
        addButton(m_recordAccompanyAction, "recordAccompanyButton");
    }
    addButton(m_stopAction, "stopButton");
}

EditorWindow::~EditorWindow()
{
    qInfo("EditorWindow::~EditorWindow");
}

void EditorWindow::buildModeActions(DropToolBar* _toolBar)
{
    m_editModeGroup  = new ActionGroup(_toolBar);
    m_drawModeAction = m_editModeGroup->addAction(embed::getIcon("edit_draw"),
                                                  TR("Draw mode (Shift+D)"));
    m_eraseModeAction = m_editModeGroup->addAction(
            embed::getIcon("edit_erase"), TR("Erase mode (Shift+E)"));
    m_selectModeAction = m_editModeGroup->addAction(
            embed::getIcon("edit_select"), TR("Select mode (Shift+S)"));
    m_moveModeAction = m_editModeGroup->addAction(embed::getIcon("edit_move"),
                                                  TR("Move mode (Shift+M)"));
    m_splitModeAction = m_editModeGroup->addAction(
            embed::getIcon("edit_split"), TR("Split mode (Shift+K)"));
    m_joinModeAction = m_editModeGroup->addAction(embed::getIcon("edit_join"),
                                                  TR("Split mode (Shift+J)"));
    m_detuneModeAction = m_editModeGroup->addAction(
            embed::getIcon("automation"), TR("Detune mode (Shift+T)"));

    _toolBar->addSeparator();
    _toolBar->addAction(m_drawModeAction);
    _toolBar->addAction(m_eraseModeAction);
    _toolBar->addAction(m_selectModeAction);
    _toolBar->addAction(m_moveModeAction);
    _toolBar->addAction(m_splitModeAction);
    _toolBar->addAction(m_joinModeAction);
    _toolBar->addAction(m_detuneModeAction);

    m_drawModeAction->setShortcut(Qt::SHIFT | Qt::Key_D);
    m_eraseModeAction->setShortcut(Qt::SHIFT | Qt::Key_E);
    m_selectModeAction->setShortcut(Qt::SHIFT | Qt::Key_S);
    m_moveModeAction->setShortcut(Qt::SHIFT | Qt::Key_M);
    m_splitModeAction->setShortcut(Qt::SHIFT | Qt::Key_K);
    m_joinModeAction->setShortcut(Qt::SHIFT | Qt::Key_J);
    m_detuneModeAction->setShortcut(Qt::SHIFT | Qt::Key_T);

    m_previousEditAction = nullptr;

    m_drawModeAction->setChecked(true);
}

QAction* EditorWindow::playAction() const
{
    return m_playAction;
}

void EditorWindow::closeEvent(QCloseEvent* _ce)
{
    QWidget* p = parentWidget();
    if(p != nullptr)
        p->hide();
    else
        hide();

    _ce->ignore();
}

DropToolBar::DropToolBar(QWidget* parent) : QToolBar(parent)
{
    // setAcceptDrops(true);
}

void DropToolBar::addBlank()
{
    QLabel* lbl = new QLabel(this);
    lbl->setFixedSize(34, 34);
    // lbl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    addWidget(lbl);
}

/*
void DropToolBar::dragEnterEvent(QDragEnterEvent* event)
{
    emit dragEntered(event);
}

void DropToolBar::dropEvent(QDropEvent* event)
{
    emit dropped(event);
}
*/
