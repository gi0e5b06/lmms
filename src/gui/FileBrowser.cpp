/*
 * FileBrowser.cpp - implementation of the project-, preset- and
 *                    sample-file-browser
 *
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

#include "FileBrowser.h"

#include "BBTrackContainer.h"
#include "Backtrace.h"
#include "ConfigManager.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "ImportFilter.h"
#include "Instrument.h"
#include "InstrumentTrack.h"
#include "MainWindow.h"
#include "Mixer.h"
#include "PluginFactory.h"
#include "PresetPreviewPlayHandle.h"
#include "SamplePlayHandle.h"
#include "Song.h"
#include "StringPairDrag.h"
#include "TextFloat.h"
#include "embed.h"
//#include "gui_templates.h"

#include <QApplication>
#include <QEventLoop>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>

enum TreeWidgetItemTypes
{
    TypeFileItem = QTreeWidgetItem::UserType,
    TypeDirectoryItem
};

FileBrowser::FileBrowser(const QString& directories,
                         const QString& filter,
                         const QString& title,
                         const QPixmap& pm,
                         QWidget*       parent,
                         bool           dirs_as_items,
                         bool           recurse) :
      SideBarWidget(title, pm, parent),
      m_directories(directories), m_filter(filter),
      m_dirsAsItems(dirs_as_items), m_recurse(recurse)
{
    setWindowTitle(tr("Browser"));

    QWidget* searchWidget = new QWidget(contentParent());
    // searchWidget->setFixedHeight( 24 );

    QGridLayout* searchWidgetLayout = new QGridLayout(searchWidget);
    searchWidgetLayout->setContentsMargins(0, 0, 0, 0);
    searchWidgetLayout->setSpacing(5);

    m_filterEdit = new QLineEdit(searchWidget);
    m_filterEdit->setPlaceholderText(tr("Search"));
    //#if QT_VERSION >= 0x050000
    m_filterEdit->setClearButtonEnabled(true);
    //#endif
    connect(m_filterEdit, SIGNAL(textEdited(const QString&)), this,
            SLOT(filterItems(const QString&)));

    QPushButton* reload_btn = new QPushButton(embed::getIcon("reload"),
                                              QString::null, searchWidget);
    reload_btn->setToolTip(tr("Refresh list"));
    connect(reload_btn, SIGNAL(clicked()), this, SLOT(reloadTree()));

    searchWidgetLayout->addWidget(m_filterEdit, 0, 0);
    // searchWidgetLayout->addSpacing( 5 );
    searchWidgetLayout->addWidget(reload_btn, 0, 1);

    addContentWidget(searchWidget);

    m_fileBrowserTreeWidget = new FileBrowserTreeWidget(contentParent());
    addContentWidget(m_fileBrowserTreeWidget);

    // Whenever the FileBrowser has focus, Ctrl+F should direct focus to its
    // filter box.
    QShortcut* filterFocusShortcut
            = new QShortcut(QKeySequence(QKeySequence::Find), this,
                            SLOT(giveFocusToFilter()));
    filterFocusShortcut->setContext(Qt::WidgetWithChildrenShortcut);

    reloadTree();
    show();
}

FileBrowser::~FileBrowser()
{
}

bool FileBrowser::filterItems(const QString& filter, QTreeWidgetItem* item)
{
    // call with item=nullptr to filter the entire tree
    bool anyMatched = false;

    int numChildren = item ? item->childCount()
                           : m_fileBrowserTreeWidget->topLevelItemCount();
    for(int i = 0; i < numChildren; ++i)
    {
        QTreeWidgetItem* it = item ? item->child(i)
                                   : m_fileBrowserTreeWidget->topLevelItem(i);

        // is directory?
        if(it->childCount())
        {
            // matches filter?
            if(it->text(0).contains(filter, Qt::CaseInsensitive))
            {
                // yes, then show everything below
                it->setHidden(false);
                filterItems(QString::null, it);
                it->setExpanded(!filter.isEmpty());
                anyMatched = true;
            }
            else
            {
                // only show if item below matches filter
                bool didMatch = filterItems(filter, it);
                it->setHidden(!didMatch);
                it->setExpanded(didMatch && !filter.isEmpty());
                anyMatched = anyMatched || didMatch;
            }
        }
        // a standard item (i.e. no file or directory item?)
        else if(it->type() == QTreeWidgetItem::Type)
        {
            // hide if there's any filter
            it->setHidden(!filter.isEmpty());
        }
        else
        {
            // file matches filter?
            bool didMatch = it->text(0).contains(filter, Qt::CaseInsensitive);
            it->setHidden(!didMatch);
            anyMatched = anyMatched || didMatch;
        }
    }

    // if(item!=nullptr) item->setExpanded(anyMatched);
    // if(anyMatched) expandItems(item,true);
    return anyMatched;
}

void FileBrowser::reloadTree()
{
    const QString text = m_filterEdit->text();
    m_filterEdit->clear();
    m_fileBrowserTreeWidget->clear();
    QStringList paths = m_directories.split('*');
    for(QStringList::iterator it = paths.begin(); it != paths.end(); ++it)
    {
        addItems(*it);
    }
    expandItems();
    m_filterEdit->setText(text);
    filterItems(text);
}

void FileBrowser::expandItems(QTreeWidgetItem* _item, bool _all)
{
    const int numChildren
            = _item ? _item->childCount()
                    : m_fileBrowserTreeWidget->topLevelItemCount();
    for(int i = 0; i < numChildren; ++i)
    {
        QTreeWidgetItem* it
                = _item ? _item->child(i)
                        : m_fileBrowserTreeWidget->topLevelItem(i);
        if(m_recurse)
        {
            it->setExpanded(true);
        }
        Directory* d = dynamic_cast<Directory*>(it);
        if(d)
        {
            d->update();
            d->setExpanded(_all);
        }
        if(m_recurse && it->childCount())
        {
            expandItems(it, _all);
        }
    }
}

void FileBrowser::giveFocusToFilter()
{
    if(!m_filterEdit->hasFocus())
    {
        // give focus to filter text box and highlight its text for quick
        // editing if not previously focused
        m_filterEdit->setFocus();
        m_filterEdit->selectAll();
    }
}

void FileBrowser::addItems(const QString& path)
{
    if(m_dirsAsItems)
    {
        m_fileBrowserTreeWidget->addTopLevelItem(
                new Directory(path, QString::null, m_filter));
        return;
    }

    QDir        cdir(path);
    QStringList files = cdir.entryList(QDir::Dirs, QDir::Name);
    for(QStringList::const_iterator it = files.constBegin();
        it != files.constEnd(); ++it)
    {
        QString cur_file = *it;
        if(cur_file[0] != '.')
        {
            bool orphan = true;
            for(int i = 0; i < m_fileBrowserTreeWidget->topLevelItemCount();
                ++i)
            {
                Directory* d = dynamic_cast<Directory*>(
                        m_fileBrowserTreeWidget->topLevelItem(i));
                if(d == nullptr || cur_file < d->text(0))
                {
                    Directory* dd = new Directory(cur_file, path, m_filter);
                    m_fileBrowserTreeWidget->insertTopLevelItem(i, dd);
                    dd->update();
                    orphan = false;
                    break;
                }
                else if(cur_file == d->text(0))
                {
                    d->addDirectory(path);
                    d->update();
                    orphan = false;
                    break;
                }
            }
            if(orphan)
            {
                Directory* d = new Directory(cur_file, path, m_filter);
                d->update();
                m_fileBrowserTreeWidget->addTopLevelItem(d);
            }
        }
    }

    files = cdir.entryList(QDir::Files, QDir::Name);
    for(QStringList::const_iterator it = files.constBegin();
        it != files.constEnd(); ++it)
    {
        QString cur_file = *it;
        if(cur_file[0] != '.')
        {
            // TODO: don't insert instead of removing, order changed
            // remove existing file-items
            QList<QTreeWidgetItem*> existing
                    = m_fileBrowserTreeWidget->findItems(
                            cur_file, Qt::MatchFixedString);
            if(!existing.empty())
            {
                delete existing.front();
            }
            (void)new FileItem(m_fileBrowserTreeWidget, cur_file, path);
        }
    }
}

void FileBrowser::keyPressEvent(QKeyEvent* ke)
{
    if(ke->key() == Qt::Key_F5)
    {
        reloadTree();
    }
    else
    {
        ke->ignore();
    }
}

FileBrowserTreeWidget::FileBrowserTreeWidget(QWidget* parent) :
      QTreeWidget(parent), m_mousePressed(false), m_pressPos(),
      m_previewPlayHandle(nullptr),
      m_pphMutex(),  // tmp  QMutex::Recursive ),
      m_contextMenuItem(nullptr)
{
    setColumnCount(1);
    headerItem()->setHidden(true);
    setSortingEnabled(false);

    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)),
            SLOT(activateListItem(QTreeWidgetItem*, int)));
    connect(this, SIGNAL(itemCollapsed(QTreeWidgetItem*)),
            SLOT(updateDirectory(QTreeWidgetItem*)));
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)),
            SLOT(updateDirectory(QTreeWidgetItem*)));

    connect(Engine::mixer(), SIGNAL(playHandleDeleted(PlayHandle*)), this,
            SLOT(onPlayHandleDeleted(PlayHandle*)));
}

FileBrowserTreeWidget::~FileBrowserTreeWidget()
{
}

void FileBrowserTreeWidget::contextMenuEvent(QContextMenuEvent* e)
{
    FileItem* f = dynamic_cast<FileItem*>(itemAt(e->pos()));
    if(f != nullptr
       && (f->handling() == FileItem::LoadAsPreset
           || f->handling() == FileItem::LoadByPlugin))
    {
        m_contextMenuItem = f;
        QMenu contextMenu(this);
        contextMenu.addAction(tr("Send to active instrument-track"), this,
                              SLOT(sendToActiveInstrumentTrack()));
        contextMenu.addAction(tr("Open in new instrument-track/"
                                 "Song Editor"),
                              this, SLOT(openInNewInstrumentTrackSE()));
        contextMenu.addAction(tr("Open in new instrument-track/"
                                 "B+B Editor"),
                              this, SLOT(openInNewInstrumentTrackBBE()));
        contextMenu.exec(e->globalPos());
        m_contextMenuItem = nullptr;
    }
}

void FileBrowserTreeWidget::mousePressEvent(QMouseEvent* me)
{
    QTreeWidget::mousePressEvent(me);
    if(me->button() != Qt::LeftButton)
        return;

    if(m_previewPlayHandle != nullptr)
        return;

    // qWarning("FileBrowserTreeWidget::mousePressEvent()");
    // qWarning("P isPreviewing=%d",PresetPreviewPlayHandle::isPreviewing());
    // qWarning("P   m_mousePressed=%d",m_mousePressed);

    if(m_mousePressed || PresetPreviewPlayHandle::isPreviewing())
        return;

    QTreeWidgetItem* i = itemAt(me->pos());
    if(i)
    {
        // TODO: Restrict to visible selection
        //		if ( _me->x() > header()->cellPos(
        // header()->mapToActual( 0 ) )
        //			+ treeStepSize() * ( i->depth() + (
        // rootIsDecorated() ? 						1 : 0
        // )
        // )
        // + itemMargin() || 				_me->x() <
        // header()->cellPos(
        // header()->mapToActual( 0 ) ) )
        //		{
        m_pressPos     = me->pos();
        m_mousePressed = true;
        //		}
    }

    FileItem* f = dynamic_cast<FileItem*>(i);
    if((f != nullptr) && !PresetPreviewPlayHandle::isPreviewing())
    {
        m_pphMutex.lock();
        if(m_previewPlayHandle != nullptr)
        {
            // TMP GDX
            // Engine::mixer()->removePlayHandle(m_previewPlayHandle);
            // delete m_previewPlayHandle;
            qInfo("RELEASE m_previewPlayHandle != nullptr");
        }

        m_previewPlayHandle = nullptr;

        // in special case of sample-files we do not care about
        // handling() rather than directly creating a SamplePlayHandle
        if(f->type() == FileItem::SampleFile)
        {
            TextFloat* tf = TextFloat::displayMessage(
                    tr("Loading sample"),
                    tr("Please wait, loading sample for "
                       "preview..."),
                    embed::getIconPixmap("sample_file", 24, 24), 0);
            // qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            SamplePlayHandle* s = new SamplePlayHandle(f->fullName());
            // s->setDoneMayReturnTrue(false);
            m_previewPlayHandle = s;
            delete tf;
        }
        else if((f->extension() == "xiz" || f->extension() == "sf2"
                 || f->extension() == "gig")
                && !pluginFactory->pluginSupportingExtension(f->extension())
                            .isNull())
        {
            TextFloat* tf = TextFloat::displayMessage(
                    tr("Loading data"),
                    tr("Please wait, loading data for "
                       "preview..."),
                    embed::getIconPixmap("soundfont_file", 24, 24), 0);
            m_previewPlayHandle = new PresetPreviewPlayHandle(
                    f->fullName(), f->handling() == FileItem::LoadByPlugin);
            delete tf;
        }
        else if(f->type() != FileItem::VstPluginFile
                && (f->handling() == FileItem::LoadAsPreset
                    || f->handling() == FileItem::LoadByPlugin))
        {
            TextFloat* tf = TextFloat::displayMessage(
                    tr("Loading preset"),
                    tr("Please wait, loading preset for "
                       "preview..."),
                    embed::getIconPixmap("preset_file", 24, 24), 0);
            DataFile dataFile(f->fullName());
            if(!dataFile.validate(f->extension()))
            {
                delete tf;
                QMessageBox::warning(
                        0, tr("Error"),
                        f->fullName() + " "
                                + tr("does not appear to be a valid") + " "
                                + f->extension() + " " + tr("file"),
                        QMessageBox::Ok, QMessageBox::NoButton);
                m_pphMutex.unlock();
                return;
            }
            m_previewPlayHandle = new PresetPreviewPlayHandle(
                    f->fullName(), f->handling() == FileItem::LoadByPlugin,
                    &dataFile);
            delete tf;
        }

        if(m_previewPlayHandle != nullptr)
        {
            //m_previewPlayHandle->setAffinity(Engine::mixer()->thread());
            if(!Engine::mixer()->addPlayHandle(m_previewPlayHandle))
            {
                qInfo("FileBrowser: BAD previewPlayHandle not added");
                Engine::mixer()->removePlayHandle(m_previewPlayHandle);
                // delete m_previewPlayHandle;
                m_previewPlayHandle = nullptr;
            }
            else
                qInfo("FileBrowser: OK previewPlayHandle added");
        }

        m_pphMutex.unlock();
    }
}

void FileBrowserTreeWidget::mouseMoveEvent(QMouseEvent* me)
{
    if(m_mousePressed == true
       && (m_pressPos - me->pos()).manhattanLength()
                  > QApplication::startDragDistance())
    {
        // make sure any playback is stopped
        mouseReleaseEvent(nullptr);

        FileItem* f = dynamic_cast<FileItem*>(itemAt(m_pressPos));
        if(f != nullptr)
        {
            switch(f->type())
            {
                case FileItem::PresetFile:
                    new StringPairDrag(f->handling() == FileItem::LoadAsPreset
                                               ? "presetfile"
                                               : "pluginpresetfile",
                                       f->fullName(),
                                       embed::getIconPixmap("preset_file"),
                                       this);
                    break;

                case FileItem::SampleFile:
                    new StringPairDrag("samplefile", f->fullName(),
                                       embed::getIconPixmap("sample_file"),
                                       this);
                    break;
                case FileItem::SoundFontFile:
                    new StringPairDrag("soundfontfile", f->fullName(),
                                       embed::getIconPixmap("soundfont_file"),
                                       this);
                    break;
                case FileItem::VstPluginFile:
                    new StringPairDrag(
                            "vstpluginfile", f->fullName(),
                            embed::getIconPixmap("vst_plugin_file"), this);
                    break;
                case FileItem::MidiFile:
                    new StringPairDrag("importedproject", f->fullName(),
                                       embed::getIconPixmap("midi_file"),
                                       this);
                    break;
                case FileItem::ProjectFile:
                    new StringPairDrag("projectfile", f->fullName(),
                                       embed::getIconPixmap("project_file"),
                                       this);
                    break;

                default:
                    break;
            }
        }
    }
}

void FileBrowserTreeWidget::mouseReleaseEvent(QMouseEvent* me)
{
    // qWarning("FileBrowserTreeWidget::mouseReleaseEvent()");
    // qWarning("R isPreviewing=%d",PresetPreviewPlayHandle::isPreviewing());
    // qWarning("R   m_mousePressed=%d",m_mousePressed);
    if(!m_mousePressed)
    {
        // release for double-click/activate/drop etc
        // BACKTRACE
        return;
    }

    m_mousePressed = false;

    m_pphMutex.lock();
    if(m_previewPlayHandle != nullptr)
    {
        // if there're samples shorter than 3 seconds, we don't
        // stop them if the user releases mouse-button...
        /*
    if(m_previewPlayHandle->type() == PlayHandle::TypeSamplePlayHandle)
    {
        SamplePlayHandle* sph
                = dynamic_cast<SamplePlayHandle*>(m_previewPlayHandle);
        if(sph
           && (sph->frames() - sph->framesDone()
               <= static_cast<f_cnt_t>(
                          Engine::mixer()->processingSampleRate() * 3)))
        // total
        {
            sph->setDoneMayReturnTrue(true);
            Engine::mixer()->removePlayHandle(m_previewPlayHandle);
            delete m_previewPlayHandle;
            m_previewPlayHandle = nullptr;
            m_pphMutex.unlock();
            return;
        }
    }
        */

        qInfo("FileBrowserTreeWidget::mouseReleaseEvent 1");
        Engine::mixer()->removePlayHandle(m_previewPlayHandle);
        qInfo("FileBrowserTreeWidget::mouseReleaseEvent 2");
        // delete m_previewPlayHandle;
        qInfo("FileBrowserTreeWidget::mouseReleaseEvent 3");
        m_previewPlayHandle = nullptr;
    }
    m_pphMutex.unlock();
}

void FileBrowserTreeWidget::onPlayHandleDeleted(PlayHandle* handle)
{
    if(m_previewPlayHandle != nullptr)
    {
        qInfo("FileBrowserTreeWidget::onPlayHandleDeleted");
        if(m_previewPlayHandle == handle)
        {
            m_previewPlayHandle = nullptr;
        }
    }
}

void FileBrowserTreeWidget::handleFile(FileItem* f, InstrumentTrack* it)
{
    Engine::mixer()->requestChangeInModel();
    switch(f->handling())
    {
        case FileItem::LoadAsProject:
            if(gui->mainWindow()->mayChangeProject(true))
            {
                Engine::getSong()->loadProject(f->fullName());
            }
            break;

        case FileItem::LoadByPlugin:
        {
            const QString e = f->extension();
            Instrument*   i = it->instrument();
            if(i == nullptr || !i->descriptor()->supportsFileType(e))
            {
                i = it->loadInstrument(
                        pluginFactory->pluginSupportingExtension(e).name());
            }
            i->loadFile(f->fullName());
            break;
        }

        case FileItem::LoadAsPreset:
        {
            DataFile dataFile(f->fullName());
            InstrumentTrack::removeMidiPortNode(dataFile);
            it->setSimpleSerializing();
            it->loadSettings(dataFile.content().toElement());
            break;
        }

        case FileItem::ImportAsProject:
            ImportFilter::import(f->fullName(), Engine::getSong());
            break;

        case FileItem::NotSupported:
        default:
            break;
    }
    Engine::mixer()->doneChangeInModel();
}

void FileBrowserTreeWidget::activateListItem(QTreeWidgetItem* item,
                                             int              column)
{
    // qWarning("FileBrowserTreeWidget::activateListItem()");
    // qWarning("A isPreviewing=%d",PresetPreviewPlayHandle::isPreviewing());
    // qWarning("A   m_mousePressed=%d",m_mousePressed);

    // make sure any playback is stopped
    // mouseReleaseEvent( nullptr );
    // qWarning("A isPreviewing=%d",PresetPreviewPlayHandle::isPreviewing());
    // qWarning("A   m_mousePressed=%d",m_mousePressed);

    FileItem* f = dynamic_cast<FileItem*>(item);
    if(f == nullptr)
    {
        m_pphMutex.unlock();
        return;
    }

    if(f->handling() == FileItem::LoadAsProject
       || f->handling() == FileItem::ImportAsProject)
    {
        handleFile(f, nullptr);
    }
    else if(f->handling() != FileItem::NotSupported)
    {
        m_contextMenuItem = f;
        sendToActiveInstrumentTrack();
        /*
          InstrumentTrack * it = dynamic_cast<InstrumentTrack *>(
          Track::create( Track::InstrumentTrack,
          Engine::getBBTrackContainer() ) );
          handleFile( f, it );
        */
    }
}

void FileBrowserTreeWidget::openInNewInstrumentTrack(TrackContainer* tc)
{
    if(m_contextMenuItem->handling() == FileItem::LoadAsPreset
       || m_contextMenuItem->handling() == FileItem::LoadByPlugin)
    {
        InstrumentTrack* it = dynamic_cast<InstrumentTrack*>(
                Track::create(Track::InstrumentTrack, tc));
        handleFile(m_contextMenuItem, it);
    }
}

void FileBrowserTreeWidget::openInNewInstrumentTrackBBE(void)
{
    openInNewInstrumentTrack(Engine::getBBTrackContainer());
}

void FileBrowserTreeWidget::openInNewInstrumentTrackSE(void)
{
    openInNewInstrumentTrack(Engine::getSong());
}

void FileBrowserTreeWidget::sendToActiveInstrumentTrack(void)
{
    // qWarning("FileBrowserTreeWidget::sendToActiveInstrumentTrack");

    QMdiSubWindow* sw = gui->mainWindow()->workspace()->activeSubWindow();

    if(!sw->isVisible())
        return;

    if(sw->widget()->inherits("InstrumentTrackWindow"))
    {
        // qWarning("
        // windowTitle()=%s",qPrintable(w.peekPrevious()->windowTitle()));
        InstrumentTrackWindow* w
                = qobject_cast<InstrumentTrackWindow*>(sw->widget());
        handleFile(m_contextMenuItem, w->model());
    }
    // else SongEditor, BBTEditor, ...
}

void FileBrowserTreeWidget::updateDirectory(QTreeWidgetItem* item)
{
    Directory* dir = dynamic_cast<Directory*>(item);
    if(dir != nullptr)
    {
        dir->update();
    }
}

QPixmap* Directory::s_folderPixmap       = nullptr;
QPixmap* Directory::s_folderOpenedPixmap = nullptr;
QPixmap* Directory::s_folderLockedPixmap = nullptr;

Directory::Directory(const QString& filename,
                     const QString& path,
                     const QString& filter) :
      QTreeWidgetItem(QStringList(filename), TypeDirectoryItem),
      m_directories(path), m_filter(filter), m_dirCount(0)
{
    initPixmaps();

    setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

    if(!QDir(fullName()).isReadable())
    {
        setIcon(0, *s_folderLockedPixmap);
    }
    else
    {
        setIcon(0, *s_folderPixmap);
    }
}

void Directory::initPixmaps(void)
{
    if(s_folderPixmap == nullptr)
    {
        s_folderPixmap = new QPixmap(embed::getIconPixmap("folder"));
    }

    if(s_folderOpenedPixmap == nullptr)
    {
        s_folderOpenedPixmap
                = new QPixmap(embed::getIconPixmap("folder_opened"));
    }

    if(s_folderLockedPixmap == nullptr)
    {
        s_folderLockedPixmap
                = new QPixmap(embed::getIconPixmap("folder_locked"));
    }
}

void Directory::update(void)
{
    if(!isExpanded())
    {
        setIcon(0, *s_folderPixmap);
        return;
    }

    setIcon(0, *s_folderOpenedPixmap);
    if(!childCount())
    {
        m_dirCount = 0;
        for(QStringList::iterator it = m_directories.begin();
            it != m_directories.end(); ++it)
        {
            int top_index = childCount();
            if(addItems(fullName(*it))
               && (*it).contains(ConfigManager::inst()->dataDir()))
            {
                QTreeWidgetItem* sep = new QTreeWidgetItem;
                sep->setText(
                        0, FileBrowserTreeWidget::tr("--- Factory files ---")
                                   .replace("-", "—"));
                sep->setIcon(0, embed::getIconPixmap("factory_files"));
                insertChild(m_dirCount + top_index, sep);
            }
        }
    }
}

bool Directory::addItems(const QString& path)
{
    QDir thisDir(path);
    if(!thisDir.isReadable())
    {
        return false;
    }

    treeWidget()->setUpdatesEnabled(false);

    bool added_something = false;

    QStringList files = thisDir.entryList(QDir::Dirs, QDir::Name);
    for(QStringList::const_iterator it = files.constBegin();
        it != files.constEnd(); ++it)
    {
        QString cur_file = *it;
        if(cur_file[0] != '.')
        {
            bool orphan = true;
            for(int i = 0; i < childCount(); ++i)
            {
                Directory* d = dynamic_cast<Directory*>(child(i));
                if(d == nullptr || cur_file < d->text(0))
                {
                    insertChild(i, new Directory(cur_file, path, m_filter));
                    orphan = false;
                    m_dirCount++;
                    break;
                }
                else if(cur_file == d->text(0))
                {
                    d->addDirectory(path);
                    orphan = false;
                    break;
                }
            }
            if(orphan)
            {
                addChild(new Directory(cur_file, path, m_filter));
                m_dirCount++;
            }

            added_something = true;
        }
    }

    QList<QTreeWidgetItem*> items;
    files = thisDir.entryList(QDir::Files, QDir::Name);
    for(QStringList::const_iterator it = files.constBegin();
        it != files.constEnd(); ++it)
    {
        QString cur_file = *it;
        if(cur_file[0] != '.' && thisDir.match(m_filter, cur_file.toLower())
           && !thisDir.match("*.prp", cur_file.toLower()))
        {
            items << new FileItem(cur_file, path);
            added_something = true;
        }
    }
    addChildren(items);

    treeWidget()->setUpdatesEnabled(true);

    return added_something;
}

QPixmap* FileItem::s_projectFilePixmap   = nullptr;
QPixmap* FileItem::s_presetFilePixmap    = nullptr;
QPixmap* FileItem::s_sampleFilePixmap    = nullptr;
QPixmap* FileItem::s_soundfontFilePixmap = nullptr;
QPixmap* FileItem::s_vstPluginFilePixmap = nullptr;
QPixmap* FileItem::s_midiFilePixmap      = nullptr;
QPixmap* FileItem::s_unknownFilePixmap   = nullptr;

FileItem::FileItem(QTreeWidget*   parent,
                   const QString& name,
                   const QString& path) :
      QTreeWidgetItem(parent, QStringList(name), TypeFileItem),
      m_path(path)
{
    determineFileType();
    initPixmaps();
}

FileItem::FileItem(const QString& name, const QString& path) :
      QTreeWidgetItem(QStringList(name), TypeFileItem), m_path(path)
{
    determineFileType();
    initPixmaps();
}

void FileItem::initPixmaps(void)
{
    if(s_projectFilePixmap == nullptr)
    {
        s_projectFilePixmap
                = new QPixmap(embed::getPixmap("project_file", 16, 16));
    }

    if(s_presetFilePixmap == nullptr)
    {
        s_presetFilePixmap
                = new QPixmap(embed::getPixmap("preset_file", 16, 16));
    }

    if(s_sampleFilePixmap == nullptr)
    {
        s_sampleFilePixmap
                = new QPixmap(embed::getPixmap("sample_file", 16, 16));
    }

    if(s_soundfontFilePixmap == nullptr)
    {
        s_soundfontFilePixmap
                = new QPixmap(embed::getPixmap("soundfont_file", 16, 16));
    }

    if(s_vstPluginFilePixmap == nullptr)
    {
        s_vstPluginFilePixmap
                = new QPixmap(embed::getPixmap("vst_plugin_file", 16, 16));
    }

    if(s_midiFilePixmap == nullptr)
    {
        s_midiFilePixmap = new QPixmap(embed::getPixmap("midi_file", 16, 16));
    }

    if(s_unknownFilePixmap == nullptr)
    {
        s_unknownFilePixmap = new QPixmap(embed::getPixmap("unknown_file"));
    }

    switch(m_type)
    {
        case ProjectFile:
            setIcon(0, *s_projectFilePixmap);
            break;
        case PresetFile:
            setIcon(0, *s_presetFilePixmap);
            break;
        case SoundFontFile:
            setIcon(0, *s_soundfontFilePixmap);
            break;
        case VstPluginFile:
            setIcon(0, *s_vstPluginFilePixmap);
            break;
        case SampleFile:
        case PatchFile:  // TODO
            setIcon(0, *s_sampleFilePixmap);
            break;
        case MidiFile:
            setIcon(0, *s_midiFilePixmap);
            break;
        case UnknownFile:
        default:
            setIcon(0, *s_unknownFilePixmap);
            break;
    }
}

void FileItem::determineFileType(void)
{
    m_handling = NotSupported;

    const QString ext = extension();
    if(ext == "mmp" || ext == "mpt" || ext == "mmpz")
    {
        m_type     = ProjectFile;
        m_handling = LoadAsProject;
    }
    else if(ext == "xpf" || ext == "xml")
    {
        m_type     = PresetFile;
        m_handling = LoadAsPreset;
    }
    else if(ext == "xiz"
            && !pluginFactory->pluginSupportingExtension(ext).isNull())
    {
        m_type     = PresetFile;
        m_handling = LoadByPlugin;
    }
    else if(ext == "sf2")
    {
        m_type = SoundFontFile;
    }
    else if(ext == "sfz"
            && !pluginFactory->pluginSupportingExtension(ext).isNull())
    {
        m_type = SoundFontFile;
    }
    else if(ext == "pat")
    {
        m_type = PatchFile;
    }
    else if(ext == "mid")
    {
        m_type     = MidiFile;
        m_handling = ImportAsProject;
    }
    else if(ext == "dll")
    {
        m_type     = VstPluginFile;
        m_handling = LoadByPlugin;
    }
    else
    {
        m_type = UnknownFile;
    }

    if(m_handling == NotSupported && !ext.isEmpty()
       && !pluginFactory->pluginSupportingExtension(ext).isNull())
    {
        m_handling = LoadByPlugin;
        // classify as sample if not classified by anything yet but can
        // be handled by a certain plugin
        if(m_type == UnknownFile)
        {
            m_type = SampleFile;
        }
    }
}

QString FileItem::extension()
{
    return extension(fullName());
}

QString FileItem::extension(const QString& _file)
{
    return QFileInfo(_file).suffix().toLower();
}
