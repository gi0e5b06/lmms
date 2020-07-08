/*
 * SongMetaDataDialog.h - dialog for setting song properties/tags/metadata
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
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

#ifndef SONG_METADATA_DIALOG_H
#define SONG_METADATA_DIALOG_H

#include "ui_song_metadata.h"

#include <QDialog>

#include <vector>

class SongMetaDataDialog : public QDialog, public Ui::SongMetaDataDialog
{
    Q_OBJECT
    
  public:
    SongMetaDataDialog(QWidget* _parent = nullptr);
    virtual ~SongMetaDataDialog();

  protected:
    virtual void reject(void);
    virtual void closeEvent(QCloseEvent* _ce);

  private slots:
    void accept();

  private:
    static const QString GENRES[];
    static const QString STRUCTURES[];
};

#endif
