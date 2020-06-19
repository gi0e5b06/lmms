/*
 * MainWindow.h - declaration of class MainWindow, the main window of LMMS
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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ActionUpdatable.h"
#include "ConfigManager.h"
#include "SubWindow.h"

//#include <QAction>
//#include <QBasicTimer>
//#include <QHash>
#include <QIcon>
#include <QList>
#include <QMainWindow>
#include <QTabBar>
//#include <QThread>
#include <QTimer>

class QAction;
class QDomElement;
class QGridLayout;
class QMdiArea;

class ConfigManager;
class PluginView;
class ToolButton;

class MainWindow : public QMainWindow, public virtual ActionUpdatable
{
    Q_OBJECT

  public:
    QMdiArea* workspace()
    {
        return m_workspace;
    }

    QWidget* toolBar()
    {
        return m_toolBar;
    }

    int addWidgetToToolBar(QWidget* _w,
                           int      _row     = -1,
                           int      _col     = -1,
                           int      _rowSpan = -1,
                           int      _colSpan = -1);
    int addSpacingToToolBar(int _size, int _col = -1);

    bool isTabbed()
    {
        return m_tabBar != nullptr;
    }

    // wrap the widget with a window decoration and add it to the workspace
    // EXPORT SubWindow* addWindowedWidget(QWidget* _w);//, Qt::WindowFlags
    // windowFlags=0);

    ///
    /// \brief	Asks whether changes made to the project are to be saved.
    ///
    /// Opens a dialog giving the user the choice to (a) confirm his choice
    /// (such as opening a new file), (b) save the current project before
    /// proceeding or (c) cancel the process.
    ///
    /// Every function that replaces the current file (e.g. creates new file,
    /// opens another file...) must call this before and may only proceed if
    /// this function returns true.
    ///
    /// \param	stopPlayback whether playback should be stopped upon
    /// prompting.  If set to false, the caller should ensure that
    /// Engine::getSong()->stop() is called before unloading/loading a song.
    ///
    /// \return	true if the user allows the software to proceed, false if they
    ///         cancel the action.
    ///
    bool mayChangeProject(bool stopPlayback);

    // Auto save timer intervals. The slider in SetupDialog.cpp wants
    // minutes and the rest milliseconds.
    static const int DEFAULT_SAVE_INTERVAL_MINUTES = 2;
    static const int DEFAULT_AUTO_SAVE_INTERVAL
            = DEFAULT_SAVE_INTERVAL_MINUTES * 60 * 1000;

    static const int m_autoSaveShortTime = 10 * 1000;  // 10s short loop

    void autoSaveTimerReset(
            int msec
            = ConfigManager::inst()->value("ui", "saveinterval").toInt() * 60
              * 1000)
    {
        if(msec < m_autoSaveShortTime)  // No 'saveinterval' in .lmmsrc.xml
        {
            msec = DEFAULT_AUTO_SAVE_INTERVAL;
        }
        m_autoSaveTimer.start(msec);
    }

    int getAutoSaveTimerInterval()
    {
        return m_autoSaveTimer.interval();
    }

    enum SessionState
    {
        Normal,
        Recover
    };

    void setSession(SessionState session)
    {
        m_session = session;
    }

    SessionState getSession()
    {
        return m_session;
    }

    void sessionCleanup();

    void clearKeyModifiers();

    bool isCtrlPressed()
    {
        return m_keyMods.m_ctrl;
    }

    bool isShiftPressed()
    {
        return m_keyMods.m_shift;
    }

    bool isAltPressed()
    {
        return m_keyMods.m_alt;
    }

    // ActionUpdatable
    void updateActions(const bool _active, QHash<QString, bool>& _table);
    void actionTriggered(QString _name);
    void requireActionUpdate();

    // ActionManager
    void     registerAction(QString _name, QAction* _action);
    QAction* registerAction(QString _name, const QString& text);
    QAction* registerAction(QString        _name,
                            const QIcon&   _icon,
                            const QString& _text);
    QAction* registerAction(QString             _name,
                            const QIcon&        _icon,
                            const QString&      _text,
                            const QKeySequence& _ks);
    QAction* action(QString _name);
    void     enableAction(QString _name);
    void     disableAction(QString _name);
    void     setActionEnabled(QString _name, bool _enabled);

    static void saveWidgetState(QWidget* _w, QDomElement& _de);
    static void restoreWidgetState(QWidget* _w, const QDomElement& _de);

  public slots:
    void emptySlot();
    void onTimeout();
    void enterWhatsThisMode();
    void createNewProject();
    void createNewProjectFromTemplate(QAction* _idx);
    void openProject();
    bool saveProject();
    bool saveProjectAs();
    bool saveProjectAsNewVersion();
    void saveProjectAsDefaultTemplate();
    void showSettingsDialog();
    void showSongMetaDataDialog();
    void aboutLMMS();
    void help();
    void closeTab(int _index);
    // void toggleActiveWindow(QMdiSubWindow* _w);
    void toggleAutomationWindow();
    void toggleBBWindow();
    void togglePianoRollWindow();
    void toggleSongWindow();
    void toggleProjectNotesWindow();
    void toggleFxMixerWindow();
    void toggleControllerRackWindow();
    void reorganizeWindows();
    void resetWindowTitle();

    void updatePlayPauseIcons();
    void updateUndoRedoButtons();
    void undo();
    void redo();

    void onActionTriggered(bool _checked);
    void onWindowStateChanged(Qt::WindowStates, Qt::WindowStates);
    // void onActionUpdateRequired();

    void autoSave();

  protected:
    virtual void closeEvent(QCloseEvent* _ce);
    virtual void focusOutEvent(QFocusEvent* _fe);
    virtual void keyPressEvent(QKeyEvent* _ke);
    virtual void keyReleaseEvent(QKeyEvent* _ke);
    // virtual void timerEvent( QTimerEvent * _ev );

  private:
    MainWindow();
    MainWindow(const MainWindow&);
    virtual ~MainWindow();

    void finalize();

    void toggleWindow(QWidget* window, bool forceShow);
    void refocus();

    QHash<QString, QAction*> m_actions;
    QHash<QAction*, QString> m_actionNames;

    QMdiArea* m_workspace;
    QTabBar*  m_tabBar;

    QWidget*     m_toolBar;
    QGridLayout* m_toolBarLayout;

    QMenu* m_templatesMenu;
    QMenu* m_recentlyOpenedProjectsMenu;
    int    m_custom_templates_count;

    struct keyModifiers
    {
        keyModifiers() : m_ctrl(false), m_shift(false), m_alt(false)
        {
        }
        bool m_ctrl;
        bool m_shift;
        bool m_alt;
    } m_keyMods;

    QMenu* m_toolsMenu;
    // QAction * m_undoAction;
    // QAction * m_redoAction;
    QList<PluginView*> m_tools;

    // QBasicTimer m_updateTimer;
    QTimer m_updateTimer;
    QTimer m_autoSaveTimer;
    int    m_autoSaveInterval;

    friend class GuiApplication;

    QMenu* m_viewMenu;

    ToolButton* m_metronomeToggle;

    SessionState m_session;

  private slots:
    void browseHelp();
    void fillTemplatesMenu();
    void openRecentlyOpenedProject(QAction* _action);
    void showTool(QAction* _idx);
    void updateRecentlyOpenedProjectsMenu();
    void updateViewMenu(void);
    void updateConfig(QAction* _who);
    void onToggleMetronome();
    void listMidiMenu();
    void mapMidiMenu();
    void unmapMidiMenu();

  signals:
    void periodicUpdate();
    void initProgress(const QString& msg);
};

#endif
