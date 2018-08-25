/*
 * MidiEffectRackView.cpp - view for effectChain model
 *
 * Copyright (c) 2006-2007 Danny McRae <khjklujn@netscape.net>
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "MidiEffectRackView.h"

#include "MidiEffectSelectDialog.h"
#include "MidiEffectView.h"
#include "GroupBox.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QScrollArea>


MidiEffectRackView::MidiEffectRackView( MidiEffectChain* model, QWidget* parent ) :
	QWidget( parent ),
	ModelView( NULL, this )
{
	QVBoxLayout* mainLayout = new QVBoxLayout( this );
	mainLayout->setMargin( 0 );//5

	m_effectsGroupBox = new GroupBox( tr( "EFFECTS CHAIN" ) );
	mainLayout->addWidget( m_effectsGroupBox );

	QVBoxLayout* effectsLayout = new QVBoxLayout( m_effectsGroupBox );
	effectsLayout->setSpacing( 0 );
	//effectsLayout->setContentsMargins( 2, m_effectsGroupBox->titleBarHeight() + 4, 2, 2 );
	effectsLayout->setContentsMargins( 1, m_effectsGroupBox->titleBarHeight() + 4, 1, 1 );

	m_scrollArea = new QScrollArea;
	m_scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
	m_scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	m_scrollArea->setPalette( QApplication::palette( m_scrollArea ) );
	m_scrollArea->setFrameStyle( QFrame::NoFrame );
	m_scrollArea->setWidget( new QWidget );

	effectsLayout->addWidget( m_scrollArea );

	QPushButton* addButton = new QPushButton;
	addButton->setText( tr( "Add effect" ) );

	effectsLayout->addWidget( addButton );

	connect( addButton, SIGNAL( clicked() ), this, SLOT( addEffect() ) );


	m_lastY = 0;

	setModel( model );
}



MidiEffectRackView::~MidiEffectRackView()
{
	clearViews();
}





void MidiEffectRackView::clearViews()
{
	while( m_effectViews.size() )
	{
		MidiEffectView * e = m_effectViews[m_effectViews.size() - 1];
		m_effectViews.pop_back();
		delete e;
	}
}




void MidiEffectRackView::moveUp( MidiEffectView* view )
{
	fxChain()->moveUp( view->effect() );
	if( view != m_effectViews.first() )
	{
		int i = 0;
		for( QVector<MidiEffectView *>::Iterator it = m_effectViews.begin(); 
					it != m_effectViews.end(); it++, i++ )
		{
			if( *it == view )
			{
				break;
			}
		}

		MidiEffectView * temp = m_effectViews[ i - 1 ];

		m_effectViews[i - 1] = view;
		m_effectViews[i] = temp;

		update();
	}
}




void MidiEffectRackView::moveDown( MidiEffectView* view )
{
	if( view != m_effectViews.last() )
	{
		// moving next effect up is the same
		moveUp( *( qFind( m_effectViews.begin(), m_effectViews.end(), view ) + 1 ) );
	}
}




void MidiEffectRackView::deletePlugin( MidiEffectView* view )
{
	MidiEffect * e = view->effect();
	m_effectViews.erase( qFind( m_effectViews.begin(), m_effectViews.end(), view ) );
	delete view;
	fxChain()->removeEffect( e );
	e->deleteLater();
	update();
}




void MidiEffectRackView::update()
{
	QWidget * w = m_scrollArea->widget();
	QVector<bool> view_map( qMax<int>( fxChain()->m_effects.size(),
						m_effectViews.size() ), false );

	for( QVector<MidiEffect *>::Iterator it = fxChain()->m_effects.begin();
					it != fxChain()->m_effects.end(); ++it )
	{
		int i = 0;
		for( QVector<MidiEffectView *>::Iterator vit = m_effectViews.begin();
				vit != m_effectViews.end(); ++vit, ++i )
		{
			if( ( *vit )->model() == *it )
			{
				view_map[i] = true;
				break;
			}
		}
		if( i >= m_effectViews.size() )
		{
			MidiEffectView * view = new MidiEffectView( *it, w );
			connect( view, SIGNAL( moveUp( MidiEffectView * ) ), 
					this, SLOT( moveUp( MidiEffectView * ) ) );
			connect( view, SIGNAL( moveDown( MidiEffectView * ) ),
				this, SLOT( moveDown( MidiEffectView * ) ) );
			connect( view, SIGNAL( deletePlugin( MidiEffectView * ) ),
				this, SLOT( deletePlugin( MidiEffectView * ) ),
							Qt::QueuedConnection );
			view->show();
			m_effectViews.append( view );
			if( i < view_map.size() )
			{
				view_map[i] = true;
			}
			else
			{
				view_map.append( true );
			}

		}
	}

	int i = 0, nView = 0;

	const int MidiEffectViewMargin = 3;
	m_lastY = MidiEffectViewMargin;

	for( QVector<MidiEffectView *>::Iterator it = m_effectViews.begin(); 
					it != m_effectViews.end(); i++ )
	{
		if( i < view_map.size() && view_map[i] == false )
		{
			delete m_effectViews[nView];
			it = m_effectViews.erase( it );
		}
		else
		{
			( *it )->move( MidiEffectViewMargin, m_lastY );
			m_lastY += ( *it )->height();
			++nView;
			++it;
		}
	}

	w->setFixedSize( 210 + 2*MidiEffectViewMargin, m_lastY );

	QWidget::update();
}




void MidiEffectRackView::addEffect()
{
	MidiEffectSelectDialog esd( this );
	esd.exec();

	if( esd.result() == QDialog::Rejected )
	{
		return;
	}

	MidiEffect * fx = esd.instantiateSelectedPlugin( fxChain() );

	fxChain()->m_enabledModel.setValue( true );
	fxChain()->appendEffect( fx );
	update();

	// Find the effectView, and show the controls
	for( QVector<MidiEffectView *>::Iterator vit = m_effectViews.begin();
					vit != m_effectViews.end(); ++vit )
	{
		if( ( *vit )->effect() == fx )
		{
			( *vit )->editControls();

			break;
		}
	}


}




void MidiEffectRackView::modelChanged()
{
	//clearViews();
	m_effectsGroupBox->ledButton()->setModel( &fxChain()->m_enabledModel );
	connect( fxChain(), SIGNAL( aboutToClear() ), this, SLOT( clearViews() ) );
	update();
}





