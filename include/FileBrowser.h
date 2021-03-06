/*
 * FileBrowser.h - include file for FileBrowser
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

#ifndef FILE_BROWSER_H
#define FILE_BROWSER_H

#include "Mutex.h"
#include "PlayHandle.h"
#include "SideBarWidget.h"

#include <QDir>
#include <QLabel>
//#include <QMutex>
#include <QTreeWidget>

class QLineEdit;

class FileItem;
class InstrumentTrack;
class FileBrowserTreeWidget;
class TrackContainer;

class FileBrowser : public SideBarWidget
{
    Q_OBJECT

  public:
    FileBrowser(const QString& directories,
                const QString& filter,
                const QString& title,
                const QPixmap& pm,
                QWidget*       parent,
                bool           dirs_as_items = false,
                bool           recurse       = false);
    virtual ~FileBrowser();

  public slots:
    void updateInfo(QString _s);

  private slots:
    void reloadTree();
    void expandItems(QTreeWidgetItem* _item = nullptr, bool _all = false);
    // call with item=nullptr to filter the entire tree
    bool filterItems(const QString&   _filter,
                     QTreeWidgetItem* _item = nullptr);
    void giveFocusToFilter();

  private:
    virtual void keyPressEvent(QKeyEvent* ke);

    void addItems(const QString& path);

    FileBrowserTreeWidget* m_fileBrowserTreeWidget;
    QLineEdit*             m_filterEdit;
    QLabel*                m_infoBox;

    QString m_directories;
    QString m_filter;

    bool m_dirsAsItems;
    bool m_recurse;
};

class FileBrowserTreeWidget : public QTreeWidget
{
    Q_OBJECT

  public:
    FileBrowserTreeWidget(QWidget* parent);
    virtual ~FileBrowserTreeWidget();

  public slots:
    void onPlayHandleDeleted(PlayHandlePointer handle);

  signals:
    void sendInfo(QString _s);

  protected:
    void contextMenuEvent(QContextMenuEvent* cme) override;
    void mousePressEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;

  private slots:
    void activateListItem(QTreeWidgetItem* item, int column);
    void openInNewInstrumentTrackBBE(void);
    void openInNewInstrumentTrackSE(void);
    void openContainingFolder();
    void sendToActiveInstrumentTrack(void);
    void updateDirectory(QTreeWidgetItem* item);

  private:
    void handleFile(FileItem* fi, InstrumentTrack* it);
    void openInNewInstrumentTrack(TrackContainer* tc);

    bool   m_mousePressed;
    QPoint m_pressPos;

    PlayHandlePointer m_previewPlayHandle;
    Mutex             m_pphMutex;

    FileItem* m_contextMenuItem;
};

class Directory : public QTreeWidgetItem
{
  public:
    Directory(const QString& filename,
              const QString& path,
              const QString& filter);

    void update(void);

    QString fullName(QString path = QString::null)
    {
        if(path == QString::null)
            path = m_directories[0];

        if(path != QString::null)
            path += QDir::separator();

        return QDir::cleanPath(path + text(0)) + QDir::separator();
    }

    INLINE void addDirectory(const QString& dir)
    {
        m_directories.push_back(dir);
    }

  private:
    void initPixmaps(void);

    bool addItems(const QString& path);

    static QPixmap* s_folderPixmap;
    static QPixmap* s_folderOpenedPixmap;
    static QPixmap* s_folderLockedPixmap;

    QStringList m_directories;
    QString     m_filter;

    int m_dirCount;
};

class FileItem : public QTreeWidgetItem
{
  public:
    enum FileTypes
    {
        ProjectFile,
        PresetFile,
        SampleFile,
        SoundFontFile,
        PatchFile,
        MidiFile,
        VstPluginFile,
        UnknownFile,
        NumFileTypes
    };

    enum FileHandling
    {
        NotSupported,
        LoadAsProject,
        LoadAsPreset,
        LoadByPlugin,
        ImportAsProject
    };

    FileItem(QTreeWidget* parent, const QString& name, const QString& path);
    FileItem(const QString& name, const QString& path);

    QString fullName() const
    {
        return QFileInfo(m_path, text(0)).absoluteFilePath();
    }

    INLINE FileTypes type() const
    {
        return m_type;
    }

    INLINE FileHandling handling() const
    {
        return m_handling;
    }

    QString        extension();
    static QString extension(const QString& file);

  private:
    void initPixmaps(void);
    void determineFileType(void);

    static QPixmap* s_projectFilePixmap;
    static QPixmap* s_presetFilePixmap;
    static QPixmap* s_sampleFilePixmap;
    static QPixmap* s_soundfontFilePixmap;
    static QPixmap* s_vstPluginFilePixmap;
    static QPixmap* s_midiFilePixmap;
    static QPixmap* s_unknownFilePixmap;

    QString      m_path;
    FileTypes    m_type;
    FileHandling m_handling;
};

#endif
