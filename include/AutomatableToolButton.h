/*
 * AutomatableToolButton.h - 
 *
 * Copyright (c) 2017
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


#ifndef AUTOMATABLE_TOOL_BUTTON_H
#define AUTOMATABLE_TOOL_BUTTON_H

#include <QToolButton>

#include "AutomatableModelView.h"




class EXPORT AutomatableToolButton : public QToolButton, public BoolModelView
{
	Q_OBJECT
public:
	AutomatableToolButton( QWidget * _parent, const QString & _name
			= QString::null );
	virtual ~AutomatableToolButton();

	virtual bool isChecked();
	virtual void setCheckable( bool _on );

public slots:
	virtual void modelChanged();
	virtual void update();
	virtual void setChecked( bool _on );
	//virtual void toggle();



protected:
	virtual void contextMenuEvent( QContextMenuEvent * _me );
	virtual void dropEvent( QDropEvent * _de );
	virtual void mousePressEvent( QMouseEvent * _me );
	//virtual void mouseReleaseEvent( QMouseEvent * _me );
} ;

#endif
