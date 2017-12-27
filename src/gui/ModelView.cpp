/*
 * ModelView.cpp - implementation of ModelView
 *
 * Copyright (c) 2007-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include <QWidget>

#include "ModelView.h"

#include "Backtrace.h"

ModelView::ModelView( Model* model, QWidget* widget ) :
	m_widget( widget ),
	m_model( model )
{
        doConnections();
}




ModelView::~ModelView()
{
	if( m_model != NULL && m_model->isDefaultConstructed() )
	{
		delete m_model;
	}
}




void ModelView::setModel( Model* model, bool isOldModelValid )
{
        if(!model) qFatal("ModelView::setModel null not allowed");

        if(model!=m_model)
        {
                if( isOldModelValid && m_model != NULL )
                {
                        if( m_model->isDefaultConstructed() )
                                delete m_model;
                        else
                                m_model->disconnect( widget() );
		}
                m_model=model;
        }

        doConnections();
	modelChanged();
}


void ModelView::modelChanged()
{
        //qWarning("ModelView::setModel widget=%p ->update()",widget());
        widget()->update();
}


void ModelView::doConnections()
{
        //qWarning("ModelView::doConnections m_model=%p",m_model);
	if( m_model != NULL )
	{
                //m_model->disconnect(widget());
		QObject::connect( m_model, SIGNAL( dataChanged() ), widget(), SLOT( update() ) );
		QObject::connect( m_model, SIGNAL( propertiesChanged() ), widget(), SLOT( update() ) );
	}
}


