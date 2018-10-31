/*
 * VideoWaveExport.h - support for exporting a simple video line
 *                     .mpg (M-JPEG, BPM images per 10 second)
 *
 * Copyright (c) 2018 gi0e5b06
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

#ifndef _VIDEO_WAVE_EXPORT_H
#define _VIDEO_WAVE_EXPORT_H

#include "ExportFilter.h"

#include <QString>

class VideoWaveExport : public ExportFilter
{
    // 	Q_OBJECT
  public:
    VideoWaveExport();
    ~VideoWaveExport();

    virtual PluginView* instantiateView(QWidget*)
    {
        return nullptr;
    }

    virtual bool proceed(const QString& _fileName);

  private:
    void error();
};

#endif
