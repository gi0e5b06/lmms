/*
 * AutomatableActionGroup.cpp - implementation
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

#include "AutomatableActionGroup.h"

AutomatableActionGroup::AutomatableActionGroup(QWidget*       _parent,
                                               const QString& _name) :
      QActionGroup(_parent),
      IntModelView(new IntModel(0, 0, 0, NULL, _name, true), _parent)
{
}

AutomatableActionGroup::~AutomatableActionGroup()
{
    /*
    for( QList<AutomatableButton *>::iterator it = m_buttons.begin();
                                    it != m_buttons.end(); ++it )
    {
            ( *it )->m_group = NULL;
    }
    */
}

/*
void AutomatableActionGroup::addButton( AutomatableButton * _btn )
{
        _btn->m_group = this;
        _btn->setCheckable( true );
        _btn->model()->setValue( false );
        // disable journalling as we're recording changes of states of
        // button-group members on our own
        _btn->model()->setJournalling( false );

        m_buttons.push_back( _btn );
        model()->setRange( 0, m_buttons.size() - 1 );
        updateButtons();
}




void AutomatableActionGroup::removeButton( AutomatableButton * _btn )
{
        m_buttons.erase( qFind( m_buttons.begin(), m_buttons.end(), _btn ) );
        _btn->m_group = NULL;
}




void AutomatableActionGroup::activateButton( AutomatableButton * _btn )
{
        if( _btn != m_buttons[model()->rawValue()] &&
                                        m_buttons.indexOf( _btn ) != -1 )
        {
                model()->setValue( m_buttons.indexOf( _btn ) );
                for( AutomatableButton * btn : m_buttons )
                {
                        btn->update();
                }
        }
}
*/

/*
void AutomatableActionGroup::triggered(QAction *action)
{
        qWarning("AutomatableActionGroup::triggered");
        QActionGroup::triggered(action);
        updateModel();
}
*/

void AutomatableActionGroup::updateModel(QAction* _a)
{
    qWarning("AutomatableActionGroup::updateModel(QAction*)");

    int const n = _a->data().toInt();
    if((n < model()->minValue()) || (n >= model()->maxValue()))
        return;

    /*
    QList<QAction*> aa=actions();

    model()->setRange( 0, aa.size() - 1 );

    int i=0;
    for( QAction* a : aa )
    {
            if(a->isChecked() && (i != model()->rawValue()))
                    model()->setValue(i);
            ++i;
    }
    */
    model()->setValue(n);
}

void AutomatableActionGroup::modelChanged()
{
    qWarning("AutomatableActionGroup::modelChanged()");
    connect(model(), SIGNAL(dataChanged()), this, SLOT(updateActions()));
    IntModelView::modelChanged();
    updateActions();
}

void AutomatableActionGroup::updateActions()
{
    QList<QAction*> aa = actions();

    model()->setRange(0, aa.size() - 1);

    int i = 0;
    for(QAction* a : aa)
    {
        if((i == model()->rawValue()) && !a->isChecked())
            a->setChecked(true);
        ++i;
    }
}

/*
void AutomatableActionGroup::execConnectionDialog()
{
        qWarning("AutomatableActionGroup::execConnectionDialog()");
}
*/
