/*
 * LV2ControlView.h - widget for controlling a LV2 port
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
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

#ifndef LV2_CONTROL_VIEW_H
#define LV2_CONTROL_VIEW_H

#ifdef WANT_LV2


#include <QWidget>

#include "ModelView.h"

class LV2Control;


class EXPORT LV2ControlView : public QWidget, public ModelView
{
	Q_OBJECT
public:
	LV2ControlView( QWidget * _parent, LV2Control * _ctl );
	virtual ~LV2ControlView();

private:
	LV2Control * m_ctl;

} ;

#endif
#endif
