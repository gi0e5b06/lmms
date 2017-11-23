/*
 * AutomatableToolButton.cpp - implementation
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

#include <QMouseEvent>

#include "debug.h"

#include "AutomatableToolButton.h"

#include "CaptionMenu.h"
#include "Engine.h"
#include "MainWindow.h"
#include "ProjectJournal.h"
#include "StringPairDrag.h"



AutomatableToolButton::AutomatableToolButton( QWidget * _parent,
					      const QString & _name ) :
	QToolButton( _parent ),
	BoolModelView( new BoolModel( false, NULL, _name, true ), this )
{
	setWindowTitle( _name );
	doConnections();
	setFocusPolicy( Qt::NoFocus );
}




AutomatableToolButton::~AutomatableToolButton()
{
}




void AutomatableToolButton::setCheckable( bool _on )
{
	QToolButton::setCheckable( _on );
	model()->setJournalling( _on );
}




void AutomatableToolButton::setChecked( bool _on )
{
	//qInfo("AutomatableToolButton::setChecked(%d)",_on);

	// QToolButton::setChecked is called in update-slot
	//QToolButton::setChecked( _on );
	model()->setValue( _on );
}




bool AutomatableToolButton::isChecked()
{
	//qInfo("AutomatableToolButton::isChecked() c=%d m=%d",QToolButton::isChecked(),model()->value());

	return model()->value();
}




void AutomatableToolButton::modelChanged()
{
	//qInfo("AutomatableToolButton::modelChanged()");

	update();
	/*
	if( QToolButton::isChecked() != model()->value() )
	{
		QToolButton::setChecked( model()->value() );
	}
	*/
}




void AutomatableToolButton::update()
{
	//qInfo("AutomatableToolButton::update() c=%d m=%d",QToolButton::isChecked(),model()->value());

	if( defaultAction()->isChecked() != model()->value() )
	{
		//qInfo("AutomatableToolButton::update() -> setChecked");
		emit triggered(defaultAction());
	}

	QToolButton::update();
}




void AutomatableToolButton::contextMenuEvent( QContextMenuEvent * _me )
{
	// for the case, the user clicked right while pressing left mouse-
	// button, the context-menu appears while mouse-cursor is still hidden
	// and it isn't shown again until user does something which causes
	// an QApplication::restoreOverrideCursor()-call...
	/*TMP mouseReleaseEvent( NULL );*/

	CaptionMenu contextMenu( model()->displayName() );
	addDefaultActions( &contextMenu );
	contextMenu.exec( QCursor::pos() );
}




void AutomatableToolButton::mousePressEvent( QMouseEvent * _me )
{
	/*
	if( _me->button() == Qt::LeftButton &&
	    ! ( _me->modifiers() & Qt::ControlModifier ) )
	{
        // User simply clicked, toggle if needed
		if( isCheckable() )
		{
			toggle();
		}
		_me->accept();
	}
	else
	{
		// Ctrl-clicked, need to prepare drag-drop
		// Otherwise, drag the standalone button
		AutomatableModelView::mousePressEvent( _me );
		QToolButton::mousePressEvent( _me );
	}
	*/

	if( _me->button() == Qt::LeftButton &&
	    ( _me->modifiers() & Qt::ControlModifier ) )
	{
		AutomatableModelView::mousePressEvent( _me );
	}

	QToolButton::mousePressEvent( _me );
}




/*
void AutomatableToolButton::mouseReleaseEvent( QMouseEvent * _me )
{
	if( _me && _me->button() == Qt::LeftButton )
	{
		emit clicked();
	}
}
*/




void AutomatableToolButton::dropEvent( QDropEvent * _de )
{
	QString type = StringPairDrag::decodeKey( _de );
	QString val = StringPairDrag::decodeValue( _de );
	if( type == "float_value" )
	{
		model()->setValue( val.toFloat() ? true : false);
		_de->accept();
	}
	else if( type == "automatable_model" )
	{
		AutomatableModel * mod = dynamic_cast<AutomatableModel *>
			( Engine::projectJournal()->journallingObject( val.toInt() ) );
		if( mod != NULL )
		{
			AutomatableModel::linkModels( model(), mod );
			mod->setValue( model()->value() );
		}
	}
}




 /*
void AutomatableToolButton::toggle()
{
       	if( isCheckable() )
	{
		model()->setValue( !model()->value() );
	}
}
*/
