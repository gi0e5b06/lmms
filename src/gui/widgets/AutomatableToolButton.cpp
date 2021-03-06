/*
 * AutomatableToolButton.cpp - implementation
 *
 * Copyright (c) 2017-2018 gi0e5b06 (on github.com)
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

#include "AutomatableToolButton.h"

#include "CaptionMenu.h"
#include "Engine.h"
//#include "MainWindow.h"
#include "ProjectJournal.h"
#include "StringPairDrag.h"

#include <QMouseEvent>

//#include "debug.h"

AutomatableToolButton::AutomatableToolButton(QWidget*       _parent,
                                             const QString& _displayName,
                                             const QString& _objectName) :
      QToolButton(_parent),
      BoolModelView(this, _displayName)
// new BoolModel(false, nullptr, _displayName, _objectName, true),
// this)
{
    setWindowTitle(_displayName);
    // doConnections();
    setFocusPolicy(Qt::NoFocus);
}

AutomatableToolButton::~AutomatableToolButton()
{
}

void AutomatableToolButton::setCheckable(bool _on)
{
    QToolButton::setCheckable(_on);
    model()->setJournalling(_on);
}

void AutomatableToolButton::setChecked(bool _on)
{
    // qInfo("AutomatableToolButton::setChecked(%d)",_on);

    // QToolButton::setChecked is called in update-slot
    // QToolButton::setChecked( _on );
    model()->setValue(_on);
}

bool AutomatableToolButton::isChecked()
{
    // qInfo("AutomatableToolButton::isChecked() c=%d
    // m=%d",QToolButton::isChecked(),model()->rawValue());

    return model()->castValue<bool>(model()->rawValue());
}

void AutomatableToolButton::modelChanged()
{
    // qInfo("AutomatableToolButton::modelChanged()");

    update();
    /*
    if( QToolButton::isChecked() != model()->rawValue() )
    {
            QToolButton::setChecked( model()->rawValue() );
    }
    */
}

void AutomatableToolButton::update()
{
    // qInfo("AutomatableToolButton::update() c=%d
    // m=%d",QToolButton::isChecked(),model()->rawValue());

    if(defaultAction()->isChecked() != model()->rawValue())
    {
        // qInfo("AutomatableToolButton::update() -> setChecked");
        emit triggered(defaultAction());
    }

    QToolButton::update();
}

void AutomatableToolButton::contextMenuEvent(QContextMenuEvent* _me)
{
    // for the case, the user clicked right while pressing left mouse-
    // button, the context-menu appears while mouse-cursor is still hidden
    // and it isn't shown again until user does something which causes
    // an QApplication::restoreOverride Cursor()-call...
    /*TMP mouseReleaseEvent( NULL );*/

    CaptionMenu contextMenu(model()->displayName());
    addDefaultActions(&contextMenu);
    contextMenu.exec(QCursor::pos());
}

void AutomatableToolButton::mousePressEvent(QMouseEvent* _me)
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

    if(_me->button() == Qt::LeftButton
       && (_me->modifiers() & Qt::ControlModifier))
    {
        AutomatableModelView::mousePressEvent(_me);
    }

    QToolButton::mousePressEvent(_me);
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

void AutomatableToolButton::dropEvent(QDropEvent* _de)
{
    QString type = StringPairDrag::decodeKey(_de);
    QString val  = StringPairDrag::decodeValue(_de);
    if(type == "float_value")
    {
        model()->setValue(val.toFloat());
        _de->accept();
    }
    else if(type == "automatable_model")
    {
        AutomatableModel* mod = dynamic_cast<AutomatableModel*>(
                Engine::projectJournal()->journallingObject(val.toInt()));
        if(mod != NULL)
        {
            AutomatableModel::linkModels(model(), mod);
            mod->setValue(model()->rawValue());
        }
    }
}

/*
void AutomatableToolButton::toggle()
{
       if( isCheckable() )
       {
               model()->setValue( !model()->rawValue() );
       }
}
*/
