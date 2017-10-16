/*
 * BufferManager.cpp - A buffer caching/memory management system
 *
 * Copyright (c) 2017 Lukas W <lukaswhl/at/gmail.com>
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "BufferManager.h"

#include "Engine.h"
#include "Mixer.h"
#include "MemoryManager.h"

fpp_t BufferManager::s_framesPerPeriod = 0;

void BufferManager::init( fpp_t framesPerPeriod )
{
	s_framesPerPeriod=framesPerPeriod;
}


sampleFrame * BufferManager::acquire()
{
	if(s_framesPerPeriod<=0)
		qFatal("invalid framesPerPeriod %s:%d",__FILE__,__LINE__);

	sampleFrame * r=MM_ALLOC(sampleFrame,s_framesPerPeriod);
	clear(r);
	return r;
}


void BufferManager::clear( sampleFrame * ab )
{
	memset( ab, 0, sizeof(sampleFrame) * s_framesPerPeriod );
}


void BufferManager::clear( sampleFrame * ab, const f_cnt_t frames,
			   const f_cnt_t offset )
{
	if((offset<0)||(frames<=0)||
	   (offset+frames*sizeof(sampleFrame)>s_framesPerPeriod*sizeof(sampleFrame)))
		qFatal("strange clear ffp=%d nbf=%d offset=%d %s:%d",
		       s_framesPerPeriod,frames,offset,__FILE__,__LINE__);

	memset( ab + offset, 0, sizeof(sampleFrame) * frames );
}


#ifndef LMMS_DISABLE_SURROUND
void BufferManager::clear( surroundSampleFrame * ab, const f_cnt_t frames,
			   const f_cnt_t offset )
{
	qFatal("BufferManager::clear no surround");
	/*
	if(offset+frames*sizeof(surroundSampleFrame)>s_framesPerPeriod*sizeof(surroundSampleFrame))
		qFatal("strange clear ffp=%d nbf=%d offset=%d %s:%d",
	memset( ab + offset, 0, sizeof(surroundSampleFrame) * frames );
	*/
}
#endif


void BufferManager::release( sampleFrame * buf )
{
	MM_FREE(buf);
}


void BufferManager::refresh() // non-threadsafe, hence it's called periodically from mixer at a time when no other threads can interfere
{
}
