/*
 * Editor.h -
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

#ifndef EDITOR_WINDOW_H
#define EDITOR_WINDOW_H

#include "ActionUpdatable.h"
#include "Model.h"
#include "lmms_basics.h"

#include <QMainWindow>
//#include <QHash>
#include <QCloseEvent>
#include <QToolBar>

class ActionGroup;
class ComboBoxModel;
class DropToolBar;
class EditorOverlay;
class QAction;

class Editor
{
  public:
    enum EditMode
    {
        ModeDraw,
        ModeErase,
        ModeSelect,
        ModeMove,
        ModeJoin,   // Unite
        ModeSplit,  // Divide
        ModeDetune,
    };

    Editor(Model*         _parent,
           const QString& _displayName,
           const QString& _objectName);
    virtual ~Editor();

    virtual EditorOverlay* overlay() const;
    virtual void           setOverlay(EditorOverlay* _overlay);

    // static void drawModeCursor(QPainter& _p, QWidget& _w, EditMode _mode);
    static void applyOverrideCursor(Qt::CursorShape _shape);
    static void applyOverrideCursor(QCursor& _c);
    static void resetOverrideCursor();

  protected:
    // const QWidget* widget(); // this as a widget

    Model* editorModel()
    {
        return &m_editorModel;
    }

    virtual bool     isEditMode(EditMode _mode) const;
    virtual EditMode editMode() const;
    virtual EditMode cursorMode() const;
    virtual void     setEditMode(EditMode _mode);

  private:
    EditMode       m_editMode;
    EditorOverlay* m_overlay;
    Model          m_editorModel;

    friend class EditorOverlay;
};

/// \brief The base window for windows that conaint an editor.
/// Provides toolbars and transport.
/// Those editors include the Song Editor, the Automation Editor, B&B Editor,
/// and the Piano Roll.
class EditorWindow : public QMainWindow, public virtual ActionUpdatable
{
    Q_OBJECT

  public:
    void     setPauseIcon(bool displayPauseIcon = true);
    QAction* playAction() const;

    static const QVector<real_t> ZOOM_LEVELS;
    static const QVector<tick_t> QUANTIZE_LEVELS;
    static const QVector<tick_t> LENGTH_LEVELS;

    static void fillZoomLevels(ComboBoxModel& _cbm, bool _automatic);
    static void fillQuantizeLevels(ComboBoxModel& _cbm, bool _noteLock);
    static void fillLengthLevels(ComboBoxModel& _cbm, bool _lastNote);

  public slots:
    virtual void setEditMode(int _mode) = 0;

  protected:
    /// \brief	Constructor.
    ///
    /// \param	record	If set true, the editor's toolbar will contain record
    ///					buttons in addition to the play and
    /// stop buttons.
    EditorWindow(bool record = false);
    virtual ~EditorWindow();

    DropToolBar* addDropToolBarToTop(QString const& windowTitle);
    DropToolBar* addDropToolBar(Qt::ToolBarArea whereToAdd,
                                QString const&  windowTitle);
    DropToolBar* addDropToolBar(QWidget*        parent,
                                Qt::ToolBarArea whereToAdd,
                                QString const&  windowTitle);

    virtual void closeEvent(QCloseEvent* _ce);

    virtual void buildModeActions(DropToolBar* _toolBar);

    ActionGroup* m_editModeGroup;
    QAction*     m_drawModeAction;
    QAction*     m_eraseModeAction;
    QAction*     m_selectModeAction;
    QAction*     m_moveModeAction;
    QAction*     m_splitModeAction;  // Unite
    QAction*     m_joinModeAction;   // Divide
    QAction*     m_detuneModeAction;
    QAction*     m_previousEditAction;

  protected slots:
    virtual void play()
    {
    }
    virtual void record()
    {
    }
    virtual void recordAccompany()
    {
    }
    virtual void stop()
    {
    }

  private slots:
    /// Called by pressing the space key. Plays or stops.
    void togglePlayStop();

  protected:
    DropToolBar* m_toolBar;

    QAction* m_playAction;
    QAction* m_recordAction;
    QAction* m_recordAccompanyAction;
    QAction* m_stopAction;
};

/// Small helper class: A QToolBar that accepts and exposes drop events as
/// signals
class DropToolBar : public QToolBar
{
    Q_OBJECT

  public:
    DropToolBar(QWidget* parent = nullptr);

    void addBlank();
    /*
  signals:
    void dragEntered(QDragEnterEvent* event);
    void dropped(QDropEvent* event);

  protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);
    */
};

#endif
