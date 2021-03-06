/*
 * MainWindow.cpp - implementation of LMMS-main-window
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

#include "MainWindow.h"

#include "AboutDialog.h"
#include "AudioDummy.h"
#include "AutomationEditor.h"  // REQUIRED
#include "BBEditor.h"          // REQUIRED
#include "CableOverlay.h"
#include "Configuration.h"
#include "ControllerRackView.h"
#include "Engine.h"
#include "FileBrowser.h"
#include "FileDialog.h"
#include "FxMixerView.h"
#include "GuiApplication.h"
#include "MidiClient.h"
#include "MidiMapper.h"
#include "MidiPortMenu.h"
#include "PaintManager.h"
#include "PianoRoll.h"  // REQUIRED
#include "PluginBrowser.h"
#include "PluginFactory.h"
#include "PluginView.h"
#include "ProjectJournal.h"
#include "ProjectNotes.h"  // REQUIRED
#include "RemotePlugin.h"
#include "SetupDialog.h"
#include "SideBar.h"
#include "SongEditor.h"  // REQUIRED
#include "SongMetaDataDialog.h"
#include "ToolButton.h"
#include "ToolPlugin.h"
#include "VersionedSaveDialog.h"
#include "embed.h"
#include "lmmsversion.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDomElement>
#include <QFileInfo>
#include <QMdiArea>
#include <QMenuBar>
#include <QMessageBox>
#include <QShortcut>
#include <QSplitter>
#include <QStackedLayout>
#include <QWhatsThis>

MainWindow::MainWindow() :
      m_workspace(nullptr), m_tabBar(nullptr), m_templatesMenu(nullptr),
      m_recentlyOpenedProjectsMenu(nullptr), m_toolsMenu(nullptr),
      m_autoSaveTimer(this), m_viewMenu(nullptr), m_metronomeToggle(0),
      m_session(Normal)
{
    setAttribute(Qt::WA_DeleteOnClose);

    QWidget*        root_widget = new QWidget(this);
    QStackedLayout* stlo        = new QStackedLayout(root_widget);
    stlo->setStackingMode(QStackedLayout::StackAll);

    QWidget*     main_widget = new QWidget(root_widget);
    QVBoxLayout* vbox        = new QVBoxLayout(main_widget);
    vbox->setSpacing(0);
    vbox->setMargin(0);

    QWidget* w = new QWidget(main_widget);
    // QHBoxLayout* hbox = new QHBoxLayout(w);
    QGridLayout* hbox = new QGridLayout(w);
    hbox->setSpacing(0);
    hbox->setMargin(0);

    // SideBar * sideBar = new SideBar( Qt::Vertical, w );
    SideBar* sideBar = new SideBar(Qt::Vertical,
                                   w);  // main_widget); // tmp, will be w

    QSplitter* splitter = new QSplitter(Qt::Horizontal, w);
    splitter->setChildrenCollapsible(false);

    ConfigManager* confMgr = ConfigManager::inst();

    if(!CONFIG_GET_BOOL("ui.leftsidebar"))
        m_workspace = new QMdiArea(splitter);

    emit initProgress(tr("Preparing plugin browser"));
    sideBar->appendTab(new PluginBrowser(splitter));
    emit initProgress(tr("Preparing file browsers"));
    sideBar->appendTab(new FileBrowser(
            confMgr->userProjectsDir() + "*" + confMgr->factoryProjectsDir(),
            "*.mmp *.mmpz *.xml *.mid", tr("My Projects"),
            embed::getIconPixmap("project_file")
                    .transformed(QTransform().rotate(90)),
            splitter, false, true));
    sideBar->appendTab(new FileBrowser(
            confMgr->userSamplesDir() + "*" + confMgr->factorySamplesDir(),
            "*", tr("My Samples"),
            embed::getIconPixmap("sample_file")
                    .transformed(QTransform().rotate(90)),
            splitter, false, true));
    sideBar->appendTab(new FileBrowser(
            confMgr->userPresetsDir() + "*" + confMgr->factoryPresetsDir(),
            "*.xpf *.cs.xml *.xiz", tr("My Presets"),
            embed::getIconPixmap("preset_file")
                    .transformed(QTransform().rotate(90)),
            splitter, false, true));
    sideBar->appendTab(new FileBrowser(
            QDir::homePath(), "*", tr("My Home"),
            embed::getIconPixmap("home").transformed(QTransform().rotate(90)),
            splitter, false, false));

    QStringList root_paths;
    QString     title         = tr("Root directory");
    bool        dirs_as_items = false;

#ifdef LMMS_BUILD_APPLE
    title = tr("Volumes");
    root_paths += "/Volumes";
#elif defined(LMMS_BUILD_WIN32)
    title         = tr("My Computer");
    dirs_as_items = true;
#endif

#if !defined(LMMS_BUILD_APPLE)
    QFileInfoList drives = QDir::drives();
    for(const QFileInfo& drive: drives)
    {
        root_paths += drive.absolutePath();
    }
#endif

    sideBar->appendTab(
            new FileBrowser(root_paths.join("*"), "*", title,
                            embed::getIconPixmap("computer")
                                    .transformed(QTransform().rotate(90)),
                            splitter, dirs_as_items));

    if(CONFIG_GET_BOOL("ui.leftsidebar"))
        m_workspace = new QMdiArea(splitter);

    /*
    m_workspace->setViewMode(QMdiArea::TabbedView);
    m_workspace->setTabsClosable(true);
    m_workspace->setTabsMovable(true);
    for(QObject* o : m_workspace->children())
    {
        m_tabBar = dynamic_cast<QTabBar*>(o);
        if(m_tabBar != nullptr)
        {
            m_tabBar->setUsesScrollButtons(false);
            m_tabBar->setExpanding(false);
            m_tabBar->setDrawBase(false);
            // m_tabBar->setElideMode(Qt::ElideLeft);
            QStackedLayout* stackLOT = new QStackedLayout();
            m_workspace->setLayout(stackLOT);
            m_workspace->setSizePolicy(QSizePolicy::Ignored,
                                       QSizePolicy::Ignored);
            // connect(m_tabBar, SIGNAL(currentChanged(int)), stackLOT,
            //        SLOT(setCurrentIndex(int)));
            // connect(m_tabBar, SIGNAL(currentChanged(int)), this,
            //        SLOT(toggleActiveWindow()));
            connect(m_tabBar, SIGNAL(tabCloseRequested(int)), this,
                    SLOT(closeTab(int)));
            connect(m_workspace, SIGNAL(subWindowActivated(QMdiSubWindow*)),
                    this, SLOT(toggleActiveWindow(QMdiSubWindow*)));
            break;
        }
    }
    */

    // Load background
    emit    initProgress(tr("Loading background artwork"));
    QString bgArtwork = ConfigManager::inst()->backgroundArtwork();
    QImage  bgImage;
    if(!bgArtwork.isEmpty())
    {
        bgImage = QImage(bgArtwork);
    }
    if(!bgImage.isNull())
    {
        m_workspace->setBackground(bgImage);
    }
    else
    {
        m_workspace->setBackground(Qt::NoBrush);
    }

    if(!isTabbed())
    {
        m_workspace->setOption(QMdiArea::DontMaximizeSubWindowOnActivation,
                               true);
        m_workspace->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_workspace->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
    else
    {
        m_workspace->setOption(QMdiArea::DontMaximizeSubWindowOnActivation,
                               false);
        m_workspace->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_workspace->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    if(CONFIG_GET_BOOL("ui.leftsidebar"))
    {
        splitter->setStretchFactor(1, 1);
        hbox->addWidget(sideBar, 0, 0);
        hbox->addWidget(splitter, 0, 1);
        hbox->setColumnStretch(1, 1);
    }
    else
    {
        splitter->setStretchFactor(0, 1);
        hbox->addWidget(splitter, 0, 0);
        hbox->addWidget(sideBar, 0, 1);
        hbox->setColumnStretch(0, 1);
    }
    hbox->setRowStretch(0, 1);

    // create global-toolbar at the top of our window
    m_toolBar = new QWidget(main_widget);
    m_toolBar->setObjectName("mainToolbar");
    m_toolBar->setFixedHeight(64);
    m_toolBar->move(0, 0);

    // add layout for organizing quite complex toolbar-layouting
    m_toolBarLayout = new QGridLayout(m_toolBar /*, 2, 1*/);
    m_toolBarLayout->setMargin(0);
    m_toolBarLayout->setSpacing(0);

    vbox->addWidget(m_toolBar);
    vbox->addWidget(w);

    // CableOverlay* cable_overlay = new CableOverlay(root_widget);
    // stlo->addWidget(cable_overlay);
    stlo->addWidget(main_widget);
    setCentralWidget(root_widget);

    // fps
    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_updateTimer.start(1000
                        / CONFIG_GET_INT("ui.framespersecond"));  //, this );
    PaintManager::start(this);

    requireActionUpdate();

    if(ConfigManager::inst()->value("ui", "enableautosave").toInt())
    {
        // connect auto save
        connect(&m_autoSaveTimer, SIGNAL(timeout()), this, SLOT(autoSave()));
        m_autoSaveInterval
                = ConfigManager::inst()->value("ui", "saveinterval").toInt()
                                  < 1
                          ? DEFAULT_AUTO_SAVE_INTERVAL
                          : ConfigManager::inst()
                                    ->value("ui", "saveinterval")
                                    .toInt();

        // The auto save function mustn't run until there is a project
        // to save or it will run over recover.mmp if you hesitate at the
        // recover messagebox for a minute. It is now started in main.
        // See autoSaveTimerReset() in MainWindow.h
    }

    connect(Engine::getSong(), SIGNAL(playbackStateChanged()), this,
            SLOT(updatePlayPauseIcons()));
}

MainWindow::~MainWindow()
{
    for(PluginView* view: m_tools)
    {
        delete view->model();
        //delete view;
    }
    // TODO: Close tools
    // dependencies are such that the editors must be destroyed BEFORE Song is
    // deleted in Engine::destroy
    //   see issue #2015 on github
    qInfo("delete gui->automationWindow();");
    delete gui->automationWindow();
    // qInfo("delete gui->bbWindow();");
    // delete gui->bbWindow();
    qInfo("delete gui->pianoRollWindow();");
    delete gui->pianoRollWindow();
    qInfo("delete gui->songWindow();");
    delete gui->songWindow();
    // destroy engine which will do further cleanups etc.
    Engine::destroy();
}

void MainWindow::onTimeout()
{
    QThread::yieldCurrentThread();
    if(!Engine::mixer()->warningXRuns())
        emit periodicUpdate();
}

void MainWindow::finalize()
{
    resetWindowTitle();
    setWindowIcon(embed::getIcon("icon"));

    // project-popup-menu
    QMenu* project_menu = new QMenu(this);
    menuBar()->addMenu(project_menu)->setText(tr("&File"));
    project_menu->addAction(embed::getIcon("project_new"), tr("&New"), this,
                            SLOT(createNewProject()), QKeySequence::New);

    m_templatesMenu = new QMenu(tr("New from template"), this);
    m_templatesMenu->setIcon(embed::getIcon("project_new"));
    connect(m_templatesMenu, SIGNAL(aboutToShow()),
            SLOT(fillTemplatesMenu()));
    connect(m_templatesMenu, SIGNAL(triggered(QAction*)),
            SLOT(createNewProjectFromTemplate(QAction*)));

    project_menu->addMenu(m_templatesMenu);

    project_menu->addAction(embed::getIcon("project_open"), tr("&Open..."),
                            this, SLOT(openProject()), QKeySequence::Open);

    m_recentlyOpenedProjectsMenu
            = project_menu->addMenu(embed::getIcon("project_open_recent"),
                                    tr("&Recently Opened Projects"));
    connect(m_recentlyOpenedProjectsMenu, SIGNAL(aboutToShow()), this,
            SLOT(updateRecentlyOpenedProjectsMenu()));
    connect(m_recentlyOpenedProjectsMenu, SIGNAL(triggered(QAction*)), this,
            SLOT(openRecentlyOpenedProject(QAction*)));

    project_menu->addAction(embed::getIcon("project_save"), tr("&Save"), this,
                            SLOT(saveProject()), QKeySequence::Save);
    project_menu->addAction(embed::getIcon("project_saveas"),
                            tr("Save &As..."), this, SLOT(saveProjectAs()),
                            Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    project_menu->addAction(
            embed::getIcon("project_save"), tr("Save as New &Version"), this,
            SLOT(saveProjectAsNewVersion()), Qt::CTRL + Qt::ALT + Qt::Key_S);

    project_menu->addAction(embed::getIcon("project_save"),
                            tr("Save as default template"), this,
                            SLOT(saveProjectAsDefaultTemplate()));

    project_menu->addSeparator();
    project_menu->addAction(embed::getIcon("project_export"),
                            tr("&Freeze..."), Engine::getSong(),
                            SLOT(freeze()));
    project_menu->addSeparator();
    project_menu->addAction(embed::getIcon("project_export"),
                            tr("Render Son&g..."), Engine::getSong(),
                            SLOT(exportProject()), Qt::CTRL + Qt::Key_Enter);
    project_menu->addAction(embed::getIcon("project_export"),
                            tr("Render &Channels..."), Engine::getSong(),
                            SLOT(exportProjectChannels()));
    // Qt::CTRL + Qt::SHIFT + Qt::Key_C );
    project_menu->addAction(embed::getIcon("project_export"),
                            tr("Render &Tracks..."), Engine::getSong(),
                            SLOT(exportProjectTracks()));
    // Qt::CTRL + Qt::SHIFT + Qt::Key_E );
    project_menu->addAction(embed::getIcon("project_export"),
                            tr("Render &Video line..."), Engine::getSong(),
                            SLOT(exportProjectVideoLine()));
    project_menu->addAction(embed::getIcon("project_export"),
                            tr("Render Video &Wave..."), Engine::getSong(),
                            SLOT(exportProjectVideoWave()));

    project_menu->addSeparator();
    project_menu->addAction(embed::getIcon("project_import"),
                            tr("&Import..."), Engine::getSong(),
                            SLOT(importProject()));
    project_menu->addAction(embed::getIcon("midi_file"),
                            tr("&Export MIDI..."), Engine::getSong(),
                            SLOT(exportProjectMidi()), Qt::CTRL + Qt::Key_M);
    project_menu->addAction(embed::getIcon("project_export"),
                            tr("Export to LMMS 1.&2..."), Engine::getSong(),
                            SLOT(exportProjectFormat12()));

// Prevent dangling separator at end of menu per
// https://bugreports.qt.io/browse/QTBUG-40071
#if !(defined(LMMS_BUILD_APPLE) && (QT_VERSION >= 0x050000) \
      && (QT_VERSION < 0x050600))
    project_menu->addSeparator();
#endif
    project_menu->addAction(embed::getIcon("exit"), tr("&Quit"), qApp,
                            SLOT(closeAllWindows()), Qt::CTRL + Qt::Key_Q);

    QMenu* edit_menu = new QMenu(this);
    menuBar()->addMenu(edit_menu)->setText(tr("&Edit"));
    registerAction("edit_undo",
                   edit_menu->addAction(embed::getIcon("edit_undo"),
                                        tr("Undo"), this, SLOT(undo()),
                                        QKeySequence::Undo));
    registerAction("edit_redo",
                   edit_menu->addAction(embed::getIcon("edit_redo"),
                                        tr("Redo"), this, SLOT(redo()),
                                        QKeySequence::Redo));
    // Ensure that both Ctrl+Y and Ctrl+Shift+Z activate redo shortcut
    // regardless of OS defaults
    if(QKeySequence(QKeySequence::Redo) != QKeySequence(Qt::CTRL + Qt::Key_Y))
    {
        new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y), this, SLOT(redo()));
    }
    if(QKeySequence(QKeySequence::Redo)
       != QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z))
    {
        new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z), this,
                      SLOT(redo()));
    }

    edit_menu->addSeparator();
    edit_menu->addAction(registerAction("edit_cut",
                                        embed::getIcon("edit_cut"), tr("Cut"),
                                        QKeySequence::Cut));
    edit_menu->addAction(registerAction("edit_copy",
                                        embed::getIcon("edit_copy"),
                                        tr("Copy"), QKeySequence::Copy));
    edit_menu->addAction(registerAction("edit_paste",
                                        embed::getIcon("edit_paste"),
                                        tr("Paste"), QKeySequence::Paste));

    edit_menu->addSeparator();
    edit_menu->addAction(embed::getIcon("setup_general"), tr("Settings"),
                         this, SLOT(showSettingsDialog()));
    edit_menu
            ->addAction(embed::getIcon("text_block"), tr("Song Properties"),
                        this, SLOT(showSongMetaDataDialog()))
            ->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Comma));
    connect(edit_menu, SIGNAL(aboutToShow()), this,
            SLOT(updateUndoRedoButtons()));

    m_viewMenu = new QMenu(this);
    menuBar()->addMenu(m_viewMenu)->setText(tr("&View"));
    connect(m_viewMenu, SIGNAL(aboutToShow()), this, SLOT(updateViewMenu()));
    connect(m_viewMenu, SIGNAL(triggered(QAction*)), this,
            SLOT(updateConfig(QAction*)));

    QMenu* midiMenu = new QMenu(this);
    menuBar()->addMenu(midiMenu)->setText(tr("&Midi"));
    midiMenu->addAction(tr("List"), this, SLOT(listMidiMenu()));
    midiMenu->addAction(tr("Unmap"), this, SLOT(unmapMidiMenu()));
    m_mapMidiMenu = midiMenu->addMenu(tr("Map"));
    connect(m_mapMidiMenu, SIGNAL(aboutToShow()), SLOT(updateMapMidiMenu()));
    connect(m_mapMidiMenu, SIGNAL(triggered(QAction*)), this,
            SLOT(mapMidiMenu(QAction*)));

    /*
    MidiPort mp=new MidiPort("",nullptr,
    MidiPortMenu* mpm = new MidiPortMenu(MidiPort::Input);
    mpm->setModel(mp);
    mpm->updateMenu();
    midiMenu->addMenu(mpm);
    */

    m_toolsMenu = new QMenu(this);
    for(const Plugin::Descriptor* desc:
        pluginFactory->descriptors(Plugin::Tool))
    {
        m_toolsMenu->addAction(desc->logo()->pixmap(), desc->displayName());
        m_tools.push_back(
                ToolPlugin::instantiate(desc->name(), /*this*/ nullptr)
                        ->createView(this));
    }
    if(!m_toolsMenu->isEmpty())
    {
        menuBar()->addMenu(m_toolsMenu)->setText(tr("&Tools"));
        connect(m_toolsMenu, SIGNAL(triggered(QAction*)), this,
                SLOT(showTool(QAction*)));
    }

    // help-popup-menu
    QMenu* help_menu = new QMenu(this);
    menuBar()->addMenu(help_menu)->setText(tr("&Help"));
    // May use offline help
    if(true)
    {
        help_menu->addAction(embed::getIcon("help"), tr("Online Help"), this,
                             SLOT(browseHelp()));
    }
    else
    {
        help_menu->addAction(embed::getIcon("help"), tr("Help"), this,
                             SLOT(help()));
    }
    help_menu->addAction(embed::getIcon("whatsthis"), tr("What's This?"),
                         this, SLOT(enterWhatsThisMode()));

// Prevent dangling separator at end of menu per
// https://bugreports.qt.io/browse/QTBUG-40071
#if !(defined(LMMS_BUILD_APPLE) && (QT_VERSION >= 0x050000) \
      && (QT_VERSION < 0x050600))
    help_menu->addSeparator();
#endif
    help_menu->addAction(embed::getIcon("icon"), tr("About"), this,
                         SLOT(aboutLMMS()));

    // create tool-buttons
    ToolButton* project_new = new ToolButton(
            embed::getIcon("project_new"), tr("Create new project"), this,
            SLOT(createNewProject()), m_toolBar);

    ToolButton* project_new_from_template
            = new ToolButton(embed::getIcon("project_new_from_template"),
                             tr("Create new project from template"), this,
                             SLOT(emptySlot()), m_toolBar);
    project_new_from_template->setMenu(m_templatesMenu);
    project_new_from_template->setPopupMode(ToolButton::InstantPopup);

    ToolButton* project_open = new ToolButton(
            embed::getIcon("project_open"), tr("Open existing project"), this,
            SLOT(openProject()), m_toolBar);

    ToolButton* project_open_recent
            = new ToolButton(embed::getIcon("project_open_recent"),
                             tr("Recently opened projects"), this,
                             SLOT(emptySlot()), m_toolBar);
    project_open_recent->setMenu(m_recentlyOpenedProjectsMenu);
    project_open_recent->setPopupMode(ToolButton::InstantPopup);

    ToolButton* project_save = new ToolButton(
            embed::getIcon("project_save"), tr("Save current project"), this,
            SLOT(saveProject()), m_toolBar);

    ToolButton* project_export = new ToolButton(
            embed::getIcon("project_export"), tr("Export current project"),
            Engine::getSong(), SLOT(exportProject()), m_toolBar);

    ToolButton* whatsthis
            = new ToolButton(embed::getIcon("whatsthis"), tr("What's this?"),
                             this, SLOT(enterWhatsThisMode()), m_toolBar);

    m_metronomeToggle = new ToolButton(embed::getIcon("metronome"),
                                       tr("Toggle metronome"), this,
                                       SLOT(onToggleMetronome()), m_toolBar);
    m_metronomeToggle->setCheckable(true);
    m_metronomeToggle->setChecked(Engine::mixer()->isMetronomeActive());

    m_toolBarLayout->setColumnMinimumWidth(0, 5);
    m_toolBarLayout->addWidget(project_new, 0, 1);
    m_toolBarLayout->addWidget(project_new_from_template, 0, 2);
    m_toolBarLayout->addWidget(project_open, 0, 3);
    m_toolBarLayout->addWidget(project_open_recent, 0, 4);
    m_toolBarLayout->addWidget(project_save, 0, 5);
    m_toolBarLayout->addWidget(project_export, 0, 6);
    m_toolBarLayout->addWidget(whatsthis, 0, 7);
    m_toolBarLayout->addWidget(m_metronomeToggle, 0, 8);

    // window-toolbar
    ToolButton* bbWindowBTN
            = new ToolButton(embed::getIcon("bb_track_btn"),
                             tr("Show/hide the beat editor") + " (F6)", this,
                             SLOT(toggleBBWindow()), m_toolBar);
    bbWindowBTN->setShortcut(Qt::Key_F6);
    bbWindowBTN->setWhatsThis(
            tr("By pressing this button, you can show or hide the "
               "beat editor. The beat editor is used "
               "for creating beats and basslines."));

    ToolButton* songWindowBTN
            = new ToolButton(embed::getIcon("songeditor"),
                             tr("Show/hide the song editor") + " (F5)", this,
                             SLOT(toggleSongWindow()), m_toolBar);
    songWindowBTN->setShortcut(Qt::Key_F5);
    songWindowBTN->setWhatsThis(
            tr("By pressing this button, you can show or hide the "
               "song editor. The song editor is used for creating the "
               "tracks and how they should be played."));

    ToolButton* pianoRollWindowBTN = new ToolButton(
            embed::getIcon("piano"), tr("Show/hide the piano roll") + " (F7)",
            this, SLOT(togglePianoRollWindow()), m_toolBar);
    pianoRollWindowBTN->setShortcut(Qt::Key_F7);
    pianoRollWindowBTN->setWhatsThis(
            tr("By pressing this button, you can show or hide the "
               "piano roll. The piano roll is used for creating "
               "patterns of notes, especially for the melodies."));

    ToolButton* automationWindowBTN
            = new ToolButton(embed::getIcon("automation"),
                             tr("Show/hide the automation editor") + " (F8)",
                             this, SLOT(toggleAutomationWindow()), m_toolBar);
    automationWindowBTN->setShortcut(Qt::Key_F8);
    automationWindowBTN->setWhatsThis(
            tr("By pressing this button, you can show or hide the "
               "automation editor. The automation editor is used "
               "for creating curves of dynamic values for any parameter."));

    ToolButton* fx_mixer_window = new ToolButton(
            embed::getIcon("fx_mixer"), tr("Show/hide the mixer") + " (F9)",
            this, SLOT(toggleFxMixerWindow()), m_toolBar);
    fx_mixer_window->setShortcut(Qt::Key_F9);
    fx_mixer_window->setWhatsThis(
            tr("By pressing this button, you can show or hide the "
               "mixer. The mixer is a powerful tool "
               "for managing effects for your song. You can insert "
               "effects into different channels."));

    ToolButton* project_notes_window = new ToolButton(
            embed::getIcon("project_notes"),
            tr("Show/hide the project notes") + " (F10)", this,
            SLOT(toggleProjectNotesWindow()), m_toolBar);
    project_notes_window->setShortcut(Qt::Key_F10);
    project_notes_window->setWhatsThis(
            tr("By pressing this button, you can show or hide the "
               "the editor for the project notes. You can write here "
               "anything related to your song."));

    ToolButton* controllers_window = new ToolButton(
            embed::getIcon("controller"),
            tr("Show/hide the controller rack") + " (F11)", this,
            SLOT(toggleControllerRackWindow()), m_toolBar);
    controllers_window->setShortcut(Qt::Key_F11);

    ToolButton* reorganize_windows = new ToolButton(
            embed::getIcon("reorganize_windows"), tr("Reorganize Windows"),
            this, SLOT(reorganizeWindows()), m_toolBar);
    reorganize_windows->setShortcut(Qt::Key_F12);

    m_toolBarLayout->addWidget(songWindowBTN, 1, 1);
    m_toolBarLayout->addWidget(bbWindowBTN, 1, 2);
    m_toolBarLayout->addWidget(pianoRollWindowBTN, 1, 3);
    m_toolBarLayout->addWidget(automationWindowBTN, 1, 4);
    m_toolBarLayout->addWidget(fx_mixer_window, 1, 5);
    m_toolBarLayout->addWidget(project_notes_window, 1, 6);
    m_toolBarLayout->addWidget(controllers_window, 1, 7);
    m_toolBarLayout->addWidget(reorganize_windows, 1, 8);

    m_toolBarLayout->setColumnStretch(100, 1);

    // setup-dialog opened before?
    if(!ConfigManager::inst()->value("app", "configured").toInt())
    {
        ConfigManager::inst()->setValue("app", "configured", "1");
        // no, so show it that user can setup everything
        SetupDialog sd;
        sd.exec();
    }
    // look whether mixer failed to start the audio device selected by the
    // user and is using AudioDummy as a fallback
    else if(Engine::mixer()->audioDevStartFailed())
    {
        // if so, offer the audio settings section of the setup dialog
        SetupDialog sd(SetupDialog::AudioSettings);
        sd.exec();
    }

    // Add editor subwindows
    for(EditorWindow* ew: std::list<EditorWindow*>{
                gui->songWindow(), gui->bbWindow(), gui->pianoRollWindow(),
                gui->automationWindow()})
    {
        // QMdiSubWindow* window = addWindowedWidget(widget);
        // window->setAttribute(Qt::WA_DeleteOnClose, false);
        // window->resize(widget->sizeHint());
        SubWindow* win
                = SubWindow::putWidgetOnWorkspace(ew, false, false, true);
        win->setWindowIcon(ew->windowIcon());
        toggleWindow(ew, false);
    }

    for(QWidget* widget:
        std::list<QWidget*>{gui->fxMixerView(), gui->getProjectNotes(),
                            gui->getControllerRackView()})
        toggleWindow(widget, false);

    toggleWindow(gui->songWindow(), true);
    // reorganizeWindows();

    // reset window title every time we change the state of a subwindow to
    // show the correct title
    for(const QMdiSubWindow* subWindow: workspace()->subWindowList())
    {
        connect(subWindow,
                SIGNAL(windowStateChanged(Qt::WindowStates,
                                          Qt::WindowStates)),
                this,
                SLOT(onWindowStateChanged(Qt::WindowStates,
                                          Qt::WindowStates)));
    }
}

void MainWindow::onWindowStateChanged(Qt::WindowStates _old,
                                      Qt::WindowStates _new)
{
    resetWindowTitle();

    // if((_old & Qt::WindowActive)==(_new & Qt::WindowActive))
    requireActionUpdate();
}

int MainWindow::addWidgetToToolBar(
        QWidget* _w, int _row, int _col, int _rowSpan, int _colSpan)
{
    if(_row == -1)
        _row = 0;
    if(_col == -1)
        _col = m_toolBarLayout->columnCount() + 8;

    if(_rowSpan == -1)
    {
        _rowSpan = 1;
        if(_w->height() > 32)
        {
            _rowSpan = 2;
            _row     = 0;
        }
    }
    if(_colSpan == -1)
    {
        _colSpan = 1;
    }

    m_toolBarLayout->addWidget(_w, _row, _col, _rowSpan, _colSpan);
    return _col;
}

int MainWindow::addSpacingToToolBar(int _size, int _col)
{
    if(_col == -1)
        _col = m_toolBarLayout->columnCount() + 8;

    QWidget* w = new QWidget(m_toolBar);
    w->setFixedWidth(_size);
    m_toolBarLayout->addWidget(w, 0, _col, 2, 1);
    return _col;
}

void MainWindow::resetWindowTitle()
{
    QString title = "";

    if(Engine::getSong()->projectFileName() != "")
        title = QFileInfo(Engine::getSong()->projectFileName())
                        .completeBaseName();

    if(title == "")
        title = tr("Untitled");

    if(Engine::getSong()->isModified())
        title += '*';

    if(getSession() == Recover)
        title += " - " + tr("Recover session. Please save your work!");

    setWindowTitle(title + " - " + tr("LMMS %1").arg(LMMS_VERSION));
}

bool MainWindow::mayChangeProject(bool stopPlayback)
{
    if(stopPlayback)
        Engine::getSong()->stop();

    if(!Engine::getSong()->isModified() && getSession() != Recover)
        return true;

    // Separate message strings for modified and recovered files
    QString messageTitleRecovered = tr("Recovered project not saved");
    QString messageRecovered
            = tr("This project was recovered from the "
                 "previous session. It is currently "
                 "unsaved and will be lost if you don't "
                 "save it. Do you want to save it now?");

    QString messageTitleUnsaved = tr("Project not saved");
    QString messageUnsaved
            = tr("The current project was modified since "
                 "last saving. Do you want to save it "
                 "now?");

    QMessageBox mb(
            (getSession() == Recover ? messageTitleRecovered
                                     : messageTitleUnsaved),
            (getSession() == Recover ? messageRecovered : messageUnsaved),
            QMessageBox::Question, QMessageBox::Save, QMessageBox::Discard,
            QMessageBox::Cancel, this);
    int answer = mb.exec();

    if(answer == QMessageBox::Save)
    {
        return saveProject();
    }
    else if(answer == QMessageBox::Discard)
    {
        if(getSession() == Recover)
            sessionCleanup();

        return true;
    }

    return false;
}

void MainWindow::clearKeyModifiers()
{
    m_keyMods.m_ctrl  = false;
    m_keyMods.m_shift = false;
    m_keyMods.m_alt   = false;
}

void MainWindow::saveWidgetState(QWidget* _w, QDomElement& _de)
{
    // If our widget is the main content of a window (e.g. piano roll,
    // FxMixer, etc), we really care about the position of the *window* - not
    // the position of the widget within its window
    if(_w->parentWidget() != nullptr
       && _w->parentWidget()->inherits("QMdiSubWindow"))
    {
        _w = _w->parentWidget();
    }

    // If the widget is a SubWindow, then we can make use of the
    // getTrueNormalGeometry() method that performs the same as
    // normalGeometry, but isn't broken on X11 ( see
    // https://bugreports.qt.io/browse/QTBUG-256 )
    SubWindow* asSubWindow = qobject_cast<SubWindow*>(_w);
    QRect      normalGeom  = asSubWindow != nullptr
                               ? asSubWindow->getTrueNormalGeometry()
                               : _w->normalGeometry();

    bool visible = _w->isVisible();
    _de.setAttribute("visible", visible);
    _de.setAttribute("minimized", _w->isMinimized());
    _de.setAttribute("maximized", _w->isMaximized());

    _de.setAttribute("x", normalGeom.x());
    _de.setAttribute("y", normalGeom.y());

    QSize sizeToStore = normalGeom.size();
    _de.setAttribute("width", sizeToStore.width());
    _de.setAttribute("height", sizeToStore.height());
}

void MainWindow::restoreWidgetState(QWidget* _w, const QDomElement& _de)
{
    QRect r(qMax(1, _de.attribute("x").toInt()),
            qMax(1, _de.attribute("y").toInt()),
            qMax(_w->sizeHint().width(), _de.attribute("width").toInt()),
            qMax(_w->minimumHeight(), _de.attribute("height").toInt()));
    if(_de.hasAttribute("visible") && !r.isNull())
    {
        // If our widget is the main content of a window (e.g. piano roll,
        // FxMixer, etc), we really care about the position of the *window* -
        // not the position of the widget within its window
        if(_w->parentWidget() != nullptr
           && _w->parentWidget()->inherits("QMdiSubWindow"))
        {
            _w = _w->parentWidget();
        }
        // first restore the window, as attempting to resize a maximized
        // window causes graphics glitching
        _w->setWindowState(_w->windowState()
                           & ~(Qt::WindowMaximized | Qt::WindowMinimized));

        // Check isEmpty() to work around corrupt project files with empty
        // size
        if(!r.size().isEmpty())
            _w->resize(r.size());

        _w->move(r.topLeft());

        // set the window to its correct minimized/maximized/restored state
        Qt::WindowStates flags = _w->windowState();
        flags                  = _de.attribute("minimized").toInt()
                        ? (flags | Qt::WindowMinimized)
                        : (flags & ~Qt::WindowMinimized);
        flags = _de.attribute("maximized").toInt()
                        ? (flags | Qt::WindowMaximized)
                        : (flags & ~Qt::WindowMaximized);
        _w->setWindowState(flags);

        _w->setVisible(_de.attribute("visible").toInt());
    }
}

void MainWindow::emptySlot()
{
}

void MainWindow::enterWhatsThisMode()
{
    QWhatsThis::enterWhatsThisMode();
}

void MainWindow::createNewProject()
{
    if(mayChangeProject(true))
    {
        Engine::getSong()->createNewProject();
        requireActionUpdate();
    }
}

void MainWindow::createNewProjectFromTemplate(QAction* _idx)
{
    if(m_templatesMenu && mayChangeProject(true))
    {
        int  indexOfTemplate   = m_templatesMenu->actions().indexOf(_idx);
        bool isFactoryTemplate = indexOfTemplate >= m_custom_templates_count;
        QString dirBase
                = isFactoryTemplate
                          ? ConfigManager::inst()->factoryTemplatesDir()
                          : ConfigManager::inst()->userTemplateDir();

        Engine::getSong()->createNewProjectFromTemplate(
                dirBase + _idx->text().replace("&&", "&") + ".mpt");
        requireActionUpdate();
    }
}

void MainWindow::openProject()
{
    if(mayChangeProject(false))
    {
        FileDialog ofd(this, tr("Open Project"), "",
                       tr("LMMS (*.mmp *.mmpz)"));

        ofd.setDirectory(ConfigManager::inst()->userProjectsDir());
        ofd.setFileMode(FileDialog::AnyFile);  // ExistingFile);
        if(ofd.exec() == QDialog::Accepted && !ofd.selectedFiles().isEmpty())
        {
            Song* song = Engine::getSong();

            song->stop();
            setCursor(Qt::WaitCursor);
            song->loadProject(ofd.selectedFiles()[0]);
            setCursor(Qt::ArrowCursor);
            requireActionUpdate();
        }
    }
}

void MainWindow::updateRecentlyOpenedProjectsMenu()
{
    m_recentlyOpenedProjectsMenu->clear();
    QStringList rup = ConfigManager::inst()->recentlyOpenedProjects();

    //	The file history goes 50 deep but we only show the 15
    //	most recent ones that we can open and omit .mpt files.
    int shownInMenu = 0;
    for(QStringList::iterator it = rup.begin(); it != rup.end(); ++it)
    {
        QFileInfo recentFile(*it);
        if(recentFile.exists()
           && *it != ConfigManager::inst()->recoveryFile())
        {
            if(recentFile.suffix().toLower() == "mpt")
            {
                continue;
            }

            m_recentlyOpenedProjectsMenu->addAction(
                    embed::getIcon("project_file"), it->replace("&", "&&"));
#ifdef LMMS_BUILD_APPLE
            m_recentlyOpenedProjectsMenu->actions()
                    .last()
                    ->setIconVisibleInMenu(false);  // QTBUG-44565 workaround
            m_recentlyOpenedProjectsMenu->actions()
                    .last()
                    ->setIconVisibleInMenu(true);
#endif
            shownInMenu++;
            if(shownInMenu >= 23)
            {
                return;
            }
        }
    }
}

void MainWindow::openRecentlyOpenedProject(QAction* _action)
{
    if(mayChangeProject(true))
    {
        QString f = _action->text();
        // workaround for KDE
        QFileInfo recentFile(f);
        if(!recentFile.exists() && (f.indexOf('&') >= 0))
        {
            f = f.replace("&&", "£lukas-w closed my PR£");
            f = f.replace("&", "");
            f = f.replace("£lukas-w closed my PR£", "&");
        }
        // end
        setCursor(Qt::WaitCursor);
        Engine::getSong()->loadProject(f);
        setCursor(Qt::ArrowCursor);
    }
}

bool MainWindow::saveProject()
{
    if(Engine::getSong()->projectFileName() == "")
    {
        return saveProjectAs();
    }
    else if(Engine::getSong()->guiSaveProject())
    {
        if(getSession() == Recover)
            sessionCleanup();

        return true;
    }

    return false;
}

bool MainWindow::saveProjectAs()
{
    VersionedSaveDialog sfd(this, tr("Save Project"), "",
                            tr("LMMS Project") + " (*.mmpz *.mmp);;"
                                    + tr("LMMS Project Template")
                                    + " (*.mpt)");
    QString             f = Engine::getSong()->projectFileName();
    if(f != "")
    {
        sfd.setDirectory(QFileInfo(f).absolutePath());
        sfd.selectFile(QFileInfo(f).fileName());
    }
    else
    {
        sfd.setDirectory(ConfigManager::inst()->userProjectsDir());
    }

    // Don't write over file with suffix if no suffix is provided.
    QString suffix
            = ConfigManager::inst()->value("app", "nommpz").toInt() == 0
                      ? "mmpz"
                      : "mmp";
    sfd.setDefaultSuffix(suffix);

    if(sfd.exec() == FileDialog::Accepted && !sfd.selectedFiles().isEmpty()
       && sfd.selectedFiles()[0] != "")
    {
        QString fname = sfd.selectedFiles()[0];
        if(sfd.selectedNameFilter().contains("(*.mpt)"))
        {
            // Remove the default suffix
            fname.remove("." + suffix);
            if(!sfd.selectedFiles()[0].endsWith(".mpt"))
            {
                if(VersionedSaveDialog::fileExistsQuery(
                           fname + ".mpt", tr("Save project template")))
                {
                    fname += ".mpt";
                }
            }
        }
        if(Engine::getSong()->guiSaveProjectAs(fname))
        {
            if(getSession() == Recover)
            {
                sessionCleanup();
            }
            return true;
        }
    }
    return false;
}

bool MainWindow::saveProjectAsNewVersion()
{
    QString fileName = Engine::getSong()->projectFileName();
    if(fileName == "")
    {
        return saveProjectAs();
    }
    else
    {
        do
            VersionedSaveDialog::changeFileNameVersion(fileName, true);
        while(QFile(fileName).exists());

        return Engine::getSong()->guiSaveProjectAs(fileName);
    }
}

void MainWindow::saveProjectAsDefaultTemplate()
{
    QString defaultTemplate
            = ConfigManager::inst()->userTemplateDir() + "default.mpt";

    QFileInfo fileInfo(defaultTemplate);
    if(fileInfo.exists())
    {
        if(QMessageBox::warning(
                   this, tr("Overwrite default template?"),
                   tr("This will overwrite your current default template."),
                   QMessageBox::Ok, QMessageBox::Cancel)
           != QMessageBox::Ok)
        {
            return;
        }
    }

    Engine::getSong()->saveProjectFile(defaultTemplate);
}

void MainWindow::showSettingsDialog()
{
    SetupDialog sd;
    sd.exec();
}

void MainWindow::showSongMetaDataDialog()
{
    SongMetaDataDialog sd;
    sd.exec();
}

void MainWindow::aboutLMMS()
{
    AboutDialog(this).exec();
}

void MainWindow::help()
{
    QMessageBox::information(this, tr("Help not available"),
                             tr("Currently there's no help "
                                "available in LMMS.\n"
                                "Please visit "
                                "http://lmms.sf.net/wiki "
                                "for documentation on LMMS."),
                             QMessageBox::Ok);
}

void MainWindow::closeTab(int _index)
{
    /*
    QMdiSubWindow* win = m_workspace->currentSubWindow();
    qInfo("MainWindow::closeTab(%d) win=%p",_index,win);
    if(win != nullptr)
    {
        bool doc = win->testAttribute(Qt::WA_DeleteOnClose);
        win->close();
        if(!doc && win->isHidden())
            m_workspace->removeSubWindow(win);
    }
    */
    m_tabBar->removeTab(_index);
    // reorganizeWindows();
}

void MainWindow::toggleWindow(QWidget* window, bool forceShow)
{
    if(window == nullptr)
        return;

    QWidget* parent = window;
    if(!parent->inherits("QMdiSubWindow") && parent->parentWidget() != nullptr
       && parent->parentWidget()->inherits("QMdiSubWindow"))
        parent = parent->parentWidget();

    if(forceShow || m_workspace->activeSubWindow() != parent
       || parent->isHidden())
    {
        // qInfo("parent %dx%d", parent->width(), parent->height());
        // qInfo("window %dx%d", window->width(), window->height());
        if(gui->mainWindow()->isTabbed())
        {
            // for(QMdiSubWindow* subWindow : workspace()->subWindowList())
            //  subWindow->hide();
            // adjustSize();
            // m_workspace->adjustSize();
            QSize size = m_workspace->size();
            qInfo("workspace: %dx%d", size.width(), size.height());
            parent->setFixedSize(size);
            parent->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
            parent->setGeometry(QRect(QPoint(0, 0), size));
            if(parent->parentWidget() == nullptr)
                m_workspace->addSubWindow(parent);
            qInfo("parent after: %dx%d", parent->width(), parent->height());
        }
        else if(parent->width() < 50 || parent->height() < 50)
        {
            parent->resize(parent->sizeHint());
            qInfo("parent after: %dx%d", parent->width(), parent->height());
        }
        parent->show();
        window->show();
        window->setFocus();
    }
    else if(!isTabbed())
    {
        parent->hide();
        refocus();
    }

    // Workaround for Qt Bug #260116
    m_workspace->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_workspace->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if(!isTabbed())
    {
        m_workspace->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_workspace->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }
}

/*
 * When an editor window with focus is toggled off, attempt to set focus
 * to the next visible editor window, or if none are visible, set focus
 * to the parent window.
 */
void MainWindow::refocus()
{
    QList<QWidget*> windows;
    windows << gui->songWindow()->parentWidget()
            << gui->bbWindow()->parentWidget()
            << gui->pianoRollWindow()->parentWidget()
            << gui->automationWindow()->parentWidget();

    bool                      found = false;
    QList<QWidget*>::Iterator window;
    for(window = windows.begin(); window != windows.end(); ++window)
    {
        if(!(*window)->isHidden())
        {
            (*window)->setFocus();
            found = true;
            break;
        }
    }

    if(!found)
        this->setFocus();
}

/*
void MainWindow::toggleActiveWindow(QMdiSubWindow* _w)
{
    if(_w)
        qInfo("MainWindow::toggleActiveWindow() %s",
              qPrintable(_w->windowTitle()));
    toggleWindow(_w, true);
}
*/

void MainWindow::toggleBBWindow()
{
    toggleWindow(gui->bbWindow(), true);
}

void MainWindow::toggleSongWindow()
{
    toggleWindow(gui->songWindow(), true);
}

void MainWindow::togglePianoRollWindow()
{
    toggleWindow(gui->pianoRollWindow(), true);
}

void MainWindow::toggleAutomationWindow()
{
    toggleWindow(gui->automationWindow(), true);
}

void MainWindow::toggleControllerRackWindow()
{
    toggleWindow(gui->getControllerRackView(), true);
}

void MainWindow::toggleFxMixerWindow()
{
    toggleWindow(gui->fxMixerView(), true);
}

void MainWindow::toggleProjectNotesWindow()
{
    toggleWindow(gui->getProjectNotes(), true);
}

void MainWindow::updateViewMenu()
{
    m_viewMenu->clear();
    // TODO: get current visibility for these and indicate in menu?
    // Not that it's straight visible <-> invisible, more like
    // not on top -> top <-> invisible
    m_viewMenu->addAction(embed::getIcon("songeditor"),
                          tr("Song Editor") + " (F5)", this,
                          SLOT(toggleSongWindow()));
    m_viewMenu->addAction(embed::getIcon("bb_track"),
                          tr("Beat+Bassline Editor") + " (F6)", this,
                          SLOT(toggleBBWindow()));
    m_viewMenu->addAction(embed::getIcon("piano"), tr("Piano Roll") + " (F7)",
                          this, SLOT(togglePianoRollWindow()));
    m_viewMenu->addAction(embed::getIcon("automation"),
                          tr("Automation Editor") + " (F8)", this,
                          SLOT(toggleAutomationWindow()));
    m_viewMenu->addAction(embed::getIcon("fx_mixer"),
                          tr("FX Mixer") + " (F9)", this,
                          SLOT(toggleFxMixerWindow()));
    m_viewMenu->addAction(embed::getIcon("project_notes"),
                          tr("Project Notes") + " (F10)", this,
                          SLOT(toggleProjectNotesWindow()));
    m_viewMenu->addAction(embed::getIcon("controller"),
                          tr("Controller Rack") + " (F11)", this,
                          SLOT(toggleControllerRackWindow()));

    m_viewMenu->addSeparator();

    // Here we should put all look&feel -stuff from configmanager
    // that is safe to change on the fly. There is probably some
    // more elegant way to do this.
    QAction* qa;
    qa = new QAction(tr("Volume as dBFS"), this);
    qa->setData("app.displaydbfs");
    qa->setCheckable(true);
    qa->setChecked(CONFIG_GET_BOOL("app.displaydbfs"));
    m_viewMenu->addAction(qa);

    qa = new QAction(tr("Smooth scroll"), this);
    qa->setData("ui.smoothscroll");
    qa->setCheckable(true);
    qa->setChecked(CONFIG_GET_BOOL("ui.smoothscroll"));
    m_viewMenu->addAction(qa);

    // Not yet.
    /* qa = new QAction(tr( "One instrument track window" ), this);
    qa->setData("ui.oneinstrumenttrackwindow");
    qa->setCheckable( true );
    qa->setChecked( ConfigManager::inst()->value( "ui",
    "oneinstrumenttrackwindow" ).toInt() ); m_viewMenu->addAction(qa);
    */

    qa = new QAction(tr("Enable note labels in piano roll"), this);
    qa->setData("ui.printnotelabels");
    qa->setCheckable(true);
    qa->setChecked(CONFIG_GET_BOOL("ui.printnotelabels"));
    m_viewMenu->addAction(qa);
}

void MainWindow::updateConfig(QAction* _who)
{
    QString tag     = _who->data().toString();
    bool    checked = _who->isChecked();

    if(tag == "app.displaydbfs")
        CONFIG_SET_BOOL("app.displaydbfs", checked);
    else if(tag == "ui.smoothscroll")
        CONFIG_SET_BOOL("ui.smoothscroll", checked);
    // else if(tag == "ui.oneinstrumenttrackwindow")
    //    CONFIG_SET_BOOL("ui.oneinstrumenttrackwindow", checked);
    else if(tag == "ui.printnotelabels")
        CONFIG_SET_BOOL("ui.printnotelabels", checked);

    // TODO: CONFIG_SET_BOOL(tag, checked);
}

void MainWindow::updateMapMidiMenu()
{
    m_mapMidiMenu->clear();
    MidiClient* client = Engine::mixer()->midiClient();
    for(const QString& p: client->readablePorts())
    {
        QString  s = p.mid(p.indexOf("from ") + 5);
        QAction* a = m_mapMidiMenu->addAction(s);
        a->setData(p);
        // a->setCheckable(false);
    }
}

void MainWindow::listMidiMenu()
{
    MidiMapper::list(m_workspace->activeSubWindow());
}

void MainWindow::mapMidiMenu(QAction* a)
{
    qInfo("MainWindow::mapMidiMenu %s", qPrintable(a->data().toString()));
    MidiMapper::map(m_workspace->activeSubWindow(), a->data().toString());
    //"128:28 LMMS:in from MIDI Mix");
}

void MainWindow::unmapMidiMenu()
{
    MidiMapper::unmap(m_workspace->activeSubWindow());
}

void MainWindow::onToggleMetronome()
{
    Mixer* mixer = Engine::mixer();

    mixer->setMetronomeActive(m_metronomeToggle->isChecked());
}

void MainWindow::reorganizeWindows()
{
    if(isTabbed())
    {
        for(QMdiSubWindow* win: m_workspace->subWindowList())
            if(win->isHidden())
            {
                bool doc = win->testAttribute(Qt::WA_DeleteOnClose);
                win->close();
                if(!doc && win->isHidden())
                    m_workspace->removeSubWindow(win);
            }
        return;
    }

    // total space
    int wt = width() - 44;    // sidebar m_workspace->width();
    int ht = height() - 144;  // toolbar m_workspace->height();
    // 2nd row
    int h2 = gui->fxMixerView()->height();
    int w2 = wt - 262;  // tmp instr.
    // 1st row
    int h1 = ht - h2 + 8;
    int w1 = gui->fxMixerView()->width() + 6;  // tmp
    int h3 = gui->getControllerRackView()->height();
    int y3 = ht - h3 + 8;

    /*
      toggleWindow( gui->bbWindow(), forceShow );
      toggleWindow( gui->songWindow() );
      toggleWindow( gui->getProjectNotes() );
      toggleWindow( gui->pianoRoll() );
      toggleWindow( gui->automationWindow() );
      toggleWindow( gui->fxMixerView() );
      toggleWindow( gui->getControllerRackView() );
    */

    int yt = 0;
    for(QMdiSubWindow* win: m_workspace->subWindowList())
    {
        if(win->isHidden())
            continue;

        if(win == gui->songWindow()->parentWidget())
            win->setGeometry(0, 0, w2, h1);
        else if(win == gui->bbWindow()->parentWidget())
            win->setGeometry(w1 + 2, h1, w2 - w1, ht - h1 - h3 - 2);
        else if(win == gui->getProjectNotes()->parentWidget())
            win->setGeometry(20, 20, wt - 80, ht - 80);  // 0,0,w2,h1);
        else if(win == gui->pianoRollWindow()->parentWidget())
            win->setGeometry(40, 40, wt - 80, ht - 80);  // 0,0,w2,h1);
        else if(win == gui->automationWindow()->parentWidget())
            win->setGeometry(60, 60, wt - 80, ht - 80);  // 0,0,w2,h1);
        else if(win == gui->fxMixerView()->parentWidget())
            win->move(0, h1);
        else if(win == gui->getControllerRackView()->parentWidget())
            win->move(w1 + 2, y3);
        else
        {
            win->move(w2 + 2, yt);
            yt += win->height() + 2;  // 24;
        }
    }
}

void MainWindow::updatePlayPauseIcons()
{
    gui->songWindow()->setPauseIcon(false);
    gui->automationWindow()->setPauseIcon(false);
    gui->bbWindow()->setPauseIcon(false);
    gui->pianoRollWindow()->setPauseIcon(false);

    if(Engine::getSong()->isPlaying())
    {
        switch(Engine::getSong()->playMode())
        {
            case Song::Mode_PlaySong:
                gui->songWindow()->setPauseIcon(true);
                break;

            case Song::Mode_PlayAutomation:
                gui->automationWindow()->setPauseIcon(true);
                break;

            case Song::Mode_PlayBB:
                gui->bbWindow()->setPauseIcon(true);
                break;

            case Song::Mode_PlayPattern:
                gui->pianoRollWindow()->setPauseIcon(true);
                break;

            default:
                break;
        }
    }
}

void MainWindow::updateUndoRedoButtons()
{
    // when the edit menu is shown, grey out the undo/redo buttons if there's
    // nothing to undo/redo else, un-grey them
    // m_undoAction->setEnabled(Engine::projectJournal()->canUndo());
    // m_redoAction->setEnabled(Engine::projectJournal()->canRedo());
    setActionEnabled("edit_undo", Engine::projectJournal()->canUndo());
    setActionEnabled("edit_redo", Engine::projectJournal()->canRedo());
}

void MainWindow::undo()
{
    Engine::projectJournal()->undo();
    requireActionUpdate();
}

void MainWindow::redo()
{
    Engine::projectJournal()->redo();
    requireActionUpdate();
}

void MainWindow::closeEvent(QCloseEvent* _ce)
{
    if(mayChangeProject(true))
    {
        // delete recovery file
        if(ConfigManager::inst()->value("ui", "enableautosave").toInt())
        {
            sessionCleanup();
        }
        _ce->accept();
    }
    else
    {
        _ce->ignore();
    }
}

void MainWindow::sessionCleanup()
{
    // delete recover session files
    QFile::remove(ConfigManager::inst()->recoveryFile());
    setSession(Normal);
}

void MainWindow::focusOutEvent(QFocusEvent* _fe)
{
    // when loosing focus we do not receive key-(release!)-events anymore,
    // so we might miss release-events of one the modifiers we're watching!
    clearKeyModifiers();
    QMainWindow::leaveEvent(_fe);
}

void MainWindow::keyPressEvent(QKeyEvent* _ke)
{
    switch(_ke->key())
    {
        case Qt::Key_Control:
            m_keyMods.m_ctrl = true;
            break;
        case Qt::Key_Shift:
            m_keyMods.m_shift = true;
            break;
        case Qt::Key_Alt:
            m_keyMods.m_alt = true;
            break;
        default:
        {
            InstrumentTrackWindow* itw
                    = InstrumentTrackView::topLevelInstrumentTrackWindow();
            if(itw != nullptr)
            {
                itw->peripheralView()->keyPressEvent(_ke);
            }
            if(!_ke->isAccepted())
            {
                QMainWindow::keyPressEvent(_ke);
            }
        }
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent* _ke)
{
    switch(_ke->key())
    {
        case Qt::Key_Control:
            m_keyMods.m_ctrl = false;
            break;
        case Qt::Key_Shift:
            m_keyMods.m_shift = false;
            break;
        case Qt::Key_Alt:
            m_keyMods.m_alt = false;
            break;
        default:
            InstrumentTrackWindow* itw
                    = InstrumentTrackView::topLevelInstrumentTrackWindow();
            if(itw != nullptr)
            {
                itw->peripheralView()->keyReleaseEvent(_ke);
            }
            if(!_ke->isAccepted())
            {
                QMainWindow::keyReleaseEvent(_ke);
            }
    }
}

/*
void MainWindow::timerEvent( QTimerEvent * _te)
{
        emit periodicUpdate();
}
*/

void MainWindow::fillTemplatesMenu()
{
    m_templatesMenu->clear();

    QDir        user_d(ConfigManager::inst()->userTemplateDir());
    QStringList templates = user_d.entryList(QStringList("*.mpt"),
                                             QDir::Files | QDir::Readable);

    m_custom_templates_count = templates.count();
    for(QStringList::iterator it = templates.begin(); it != templates.end();
        ++it)
    {
        m_templatesMenu->addAction(embed::getIcon("project_file"),
                                   (*it).left((*it).length() - 4));
#ifdef LMMS_BUILD_APPLE
        m_templatesMenu->actions().last()->setIconVisibleInMenu(
                false);  // QTBUG-44565 workaround
        m_templatesMenu->actions().last()->setIconVisibleInMenu(true);
#endif
    }

    QDir d(ConfigManager::inst()->factoryProjectsDir() + "templates");
    templates
            = d.entryList(QStringList("*.mpt"), QDir::Files | QDir::Readable);

    if(m_custom_templates_count > 0 && !templates.isEmpty())
    {
        m_templatesMenu->addSeparator();
    }
    for(QStringList::iterator it = templates.begin(); it != templates.end();
        ++it)
    {
        m_templatesMenu->addAction(embed::getIcon("project_file"),
                                   (*it).left((*it).length() - 4));
#ifdef LMMS_BUILD_APPLE
        m_templatesMenu->actions().last()->setIconVisibleInMenu(
                false);  // QTBUG-44565 workaround
        m_templatesMenu->actions().last()->setIconVisibleInMenu(true);
#endif
    }
}

void MainWindow::showTool(QAction* _idx)
{
    PluginView* p = m_tools[m_toolsMenu->actions().indexOf(_idx)];
    p->show();
    p->parentWidget()->show();
    p->setFocus();
}

void MainWindow::browseHelp()
{
    // file:// alternative for offline help
    QString url = "https://lmms.io/documentation/";
    QDesktopServices::openUrl(url);
    // TODO: Handle error
}

void MainWindow::autoSave()
{
    if(!Engine::getSong()->isExporting()
       && !Engine::getSong()->isLoadingProject()
       && !RemotePluginBase::isMainThreadWaiting()
       && !QApplication::mouseButtons()
       && (ConfigManager::inst()->value("ui", "enablerunningautosave").toInt()
           || !Engine::getSong()->isPlaying()))
    {
        Engine::getSong()->saveProjectFile(
                ConfigManager::inst()->recoveryFile());
        autoSaveTimerReset();  // Reset timer
    }
    else
    {
        // try again in 10 seconds
        if(getAutoSaveTimerInterval() != m_autoSaveShortTime)
        {
            autoSaveTimerReset(m_autoSaveShortTime);
        }
    }
}

QAction* MainWindow::registerAction(QString _name, const QString& _text)
{
    QAction* a = new QAction(_text, this);
    registerAction(_name, a);
    return a;
}

QAction* MainWindow::registerAction(QString        _name,
                                    const QIcon&   _icon,
                                    const QString& _text)
{
    QAction* a = new QAction(_icon, _text, this);
    registerAction(_name, a);
    return a;
}

QAction* MainWindow::registerAction(QString             _name,
                                    const QIcon&        _icon,
                                    const QString&      _text,
                                    const QKeySequence& _ks)
{
    QAction* a = new QAction(_icon, _text, this);
    a->setShortcut(_ks);
    registerAction(_name, a);
    return a;
}

void MainWindow::registerAction(QString _name, QAction* _action)
{
    if(m_actions.contains(_name))
    {
        qWarning("Warning: action name '%s' is already registered",
                 qPrintable(_name));
        return;
    }

    if(m_actionNames.contains(_action))
    {
        qWarning("Warning: action '%s' is already registered with '%s'",
                 qPrintable(_name), qPrintable(m_actionNames.value(_action)));
        return;
    }

    m_actions.insert(_name, _action);
    m_actionNames.insert(_action, _name);

    connect(_action, SIGNAL(triggered(bool)), this,
            SLOT(onActionTriggered(bool)));
}

QAction* MainWindow::action(QString _name)
{
    return m_actions.value(_name, nullptr);
}

void MainWindow::enableAction(QString _name)
{
    QAction* a = action(_name);
    if(a)
        a->setEnabled(true);
}

void MainWindow::disableAction(QString _name)
{
    QAction* a = action(_name);
    if(a)
        a->setEnabled(false);
}

void MainWindow::setActionEnabled(QString _name, bool _enabled)
{
    QAction* a = action(_name);
    if(a)
        a->setEnabled(_enabled);
}

void MainWindow::onActionTriggered(bool _checked)
{
    QAction* a = dynamic_cast<QAction*>(QObject::sender());
    if(a == nullptr)
    {
        qCritical("MainWindow: null action triggered");
        return;
    }

    QString n = m_actionNames.value(a, "<unknown>");
    qInfo("MainWindow: action triggered: %s", qPrintable(n));

    QMdiSubWindow* asw = workspace()->currentSubWindow();
    if(asw == nullptr)
    {
        qCritical("MainWindow: no active subwindow");
        return;
    }
    EditorWindow* aew = dynamic_cast<EditorWindow*>(asw->widget());
    if(aew == nullptr)
    {
        qCritical("MainWindow: no active editor window");
        return;
    }
    aew->actionTriggered(n);
    requireActionUpdate();
}

// ActionUpdatable //

void MainWindow::updateActions(const bool            _active,
                               QHash<QString, bool>& _table)
{
    QMdiSubWindow* asw = workspace()->currentSubWindow();
    for(const QMdiSubWindow* subWindow: workspace()->subWindowList())
    {
        if(asw == subWindow)
            continue;
        QWidget*      w = subWindow->widget();
        EditorWindow* e = dynamic_cast<EditorWindow*>(w);
        // qInfo("e1=%p (%s)",e,qPrintable(subWindow->windowTitle()));
        if(e)
            e->updateActions(false, _table);
    }
    if(asw)
    {
        QWidget*      w = asw->widget();
        EditorWindow* e = dynamic_cast<EditorWindow*>(w);
        // qInfo("e2=%p (%s)",e,qPrintable(asw->windowTitle()));
        if(e)
            e->updateActions(_active, _table);
    }
    for(const QString& s: _table.keys())
    {
        QAction* a = action(s);
        if(a)
        {
            bool e = _table.value(s);
            if(e != a->isEnabled())
                a->setEnabled(e);
        }
        else
            qWarning("Warning: action '%s' not registered", qPrintable(s));
    }
}

void MainWindow::actionTriggered(QString _name)
{
    qInfo("MainWindow::actionTriggered() name=%s", qPrintable(_name));
}

void MainWindow::requireActionUpdate()
{
    // qInfo("MainWindow::requireActionUpdate");
    QHash<QString, bool> table;
    updateActions(true, table);
}
