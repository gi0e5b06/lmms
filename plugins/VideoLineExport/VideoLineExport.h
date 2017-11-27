/*
 * VideoLineExport.h - support for exporting a simple video line
 *                     .mpg (M-JPEG, BPM images per 10 second)
 *
 * Copyright (c) 2017 gi0e5b06
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

#ifndef _VIDEO_LINE_EXPORT_H
#define _VIDEO_LINE_EXPORT_H

#include <QString>

#include "ExportFilter.h"


/*
const int BUFFER_SIZE = 50*1024;
typedef VideoLineFile::VIDEO_LINETrack<BUFFER_SIZE> MTrack;

struct VideoLineNote
{
	int time;
	uint8_t pitch;
	int duration;
	uint8_t volume;

	inline bool operator<(const VideoLineNote &b) const
	{
		return this->time < b.time;
	}
} ;

typedef std::vector<VideoLineNote> VideoLineNoteVector;
typedef std::vector<VideoLineNote>::iterator VideoLineNoteIterator;
*/


class VideoLineExport: public ExportFilter
{
// 	Q_OBJECT
public:
	VideoLineExport();
	~VideoLineExport();

	virtual PluginView *instantiateView(QWidget *)
	{
		return nullptr;
	}

        /*
	virtual bool tryExport(const TrackContainer::TrackList &tracks,
                               const TrackContainer::TrackList &tracks_BB,
                               int tempo,
                               QVector<QPair<tick_t,tick_t>> loops,
                               const QString &filename);
        */
        virtual bool proceed(const QString& _fileName);

private:
        /*
	void writePattern(VideoLineNoteVector &pat, QDomNode n,
                          int base_pitch, double base_volume, int base_time);
	void writePatternToTrack(MTrack &mtrack, VideoLineNoteVector &nv);
	void writeBBPattern(VideoLineNoteVector &src, VideoLineNoteVector &dst,
                            int len, int base, int start, int end);
	void ProcessBBNotes(VideoLineNoteVector &nv, int cutPos);
        */

        int posToImg(int tempo,int pos);
        int imgToPos(int tempo,int img);
	void error();


} ;


#endif
