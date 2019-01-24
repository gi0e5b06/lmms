/*
 * StemMP4Import.h -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#ifndef STEM_MP4_IMPORT_H
#define STEM_MP4_IMPORT_H

#include "ImportFilter.h"

#include <QPair>
#include <QString>
#include <QVector>

class StemMP4Import : public ImportFilter
{
    Q_OBJECT

 public:
    StemMP4Import(const QString& _file);
    virtual ~StemMP4Import();

    virtual PluginView* instantiateView(QWidget*)
    {
        return (NULL);
    }

 protected:
    virtual QString pathForStem(const QFile& _file, const int _num) const;
    virtual bool tryImport(TrackContainer* _tc);
};

#endif
