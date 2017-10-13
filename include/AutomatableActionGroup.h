/*
 * AutomatableActionGroup.h - class automatableButton, the base for all buttons
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


#ifndef AUTOMATABLE_ACTION_GROUP_H
#define AUTOMATABLE_ACTION_GROUP_H

#include <QAction>
#include <QActionGroup>
#include <QWidget>

#include "AutomatableModelView.h"

class EXPORT AutomatableActionGroup : public QActionGroup, public IntModelView
{
	Q_OBJECT
 public:
	AutomatableActionGroup( QWidget * _parent,
				const QString & _name = QString::null  );
	virtual ~AutomatableActionGroup();

	/*
	void addButton( AutomatableButton * _btn );
	void removeButton( AutomatableButton * _btn );

	void activateButton( AutomatableButton * _btn );
	*/

	virtual void modelChanged();

signals:
	void triggered(QAction *action);

 public slots:
	 //void execConnectionDialog();
	void updateModel(QAction*);

 private slots:
	void updateActions();

private:
	//QList<AutomatableButton *> m_buttons;

} ;



#endif
