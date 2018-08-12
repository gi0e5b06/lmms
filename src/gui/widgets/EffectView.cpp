/*
 * EffectView.cpp - view-component for an effect
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
 * Copyright (c) 2006-2007 Danny McRae <khjklujn/at/users.sourceforge.net>
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

//#include <QLabel>
#include <QPushButton>
//#include <QMdiArea>
//#include <QMdiSubWindow>
#include <QPainter>
#include <QWhatsThis>

#include "EffectView.h"
#include "DummyEffect.h"
#include "CaptionMenu.h"
#include "embed.h"
#include "GuiApplication.h"
#include "gui_templates.h"
#include "Knob.h"
#include "LedCheckBox.h"
#include "MainWindow.h"
#include "TempoSyncKnob.h"
#include "ToolTip.h"


EffectView::EffectView( Effect * _model, QWidget * _parent ) :
	PluginView( _model, _parent ),
	m_bg( embed::getIconPixmap( "effect_plugin" ) ),
	m_subWindow( NULL ),
	m_controlView( NULL )
{
	setFixedSize( 230, 60 );

	// Disable effects that are of type "DummyEffect"
	bool isEnabled = !dynamic_cast<DummyEffect *>( effect() );
	m_bypass = new LedCheckBox( this, "", isEnabled ? LedCheckBox::Green : LedCheckBox::Red );
	m_bypass->move( 3, 3 );
	m_bypass->setEnabled( isEnabled );
	m_bypass->setWhatsThis( tr( "Toggles the effect on or off." ) );

	ToolTip::add( m_bypass, tr( "On/Off" ) );


	m_wetDry = new Knob( knobBright_26, this );
	m_wetDry->setLabel( tr( "D-WET" ) );
	//m_autoQuit->move( 20, 5 );
	m_wetDry->setGeometry( 19,5,36,36 );
	m_wetDry->setEnabled( isEnabled );
	m_wetDry->setHintText( tr( "Wet Level:" ), "" );
	m_wetDry->setWhatsThis( tr( "The Wet/Dry knob sets the ratio between "
					"the input signal and the effect signal that "
					"forms the output." ) );


	m_autoQuit = new TempoSyncKnob( knobBright_26, this );
	m_autoQuit->setLabel( tr( "DECAY" ) );
	//m_autoQuit->move( 60, 5 );
	m_autoQuit->setGeometry( 55,5,36,36 );
	m_autoQuit->setEnabled( isEnabled );
	m_autoQuit->setHintText( tr( "Time:" ), "ms" );
	m_autoQuit->setWhatsThis( tr(
"The Decay knob controls how many buffers of silence must pass before the "
"plugin stops processing.  Smaller values will reduce the CPU overhead but "
"run the risk of clipping the tail on delay and reverb effects." ) );


	m_gate = new Knob( knobBright_26, this );
	m_gate->setLabel( tr( "GATE" ) );
	//m_gate->move( 100, 5 );
	m_gate->setGeometry( 91,5,36,36 );
	m_gate->setEnabled( isEnabled );
	m_gate->setHintText( tr( "Gate:" ), "" );
	m_gate->setWhatsThis( tr(
"The Gate knob controls the signal level that is considered to be 'silence' "
"while deciding when to stop processing signals." ) );


	m_balance = new Knob( knobBright_26, this );
	m_balance->setLabel( tr( "BAL." ) );
	//m_balance->move( 100, 5 );
	m_balance->setGeometry( 127,5,36,36 );
	m_balance->setEnabled( isEnabled );
	m_balance->setHintText( tr( "Balance:" ), "" );
	m_balance->setWhatsThis( tr(
"The Balance knob controls how ..." ) );

        m_balance->setVisible(_model->isBalanceable());
	setModel( _model );

        if(!effect())
                qWarning("EffectView: effect null");
        else if(!effect()->controls())
                qWarning("EffectView: effect controls null");
        else if(!effect()->controls()->controlCount())
                qWarning("EffectView: effect control count 0");
        else
                //if( effect()->controls()->controlCount() > 0 )
	{
		//QPushButton * ctls_btn = new QPushButton( tr( "Controls" ),this );
                QPushButton * ctls_btn = new QPushButton
                        ( embed::getIconPixmap( "trackop" ),"",this );
		QFont f = ctls_btn->font();
		ctls_btn->setFont( pointSize<8>( f ) );
		//ctls_btn->setGeometry( 140, 14, 50, 20 );
		//ctls_btn->setGeometry( 136, 4, 70, 35 );//41
                ctls_btn->setGeometry( 163+5, 5, 36, 36 );//41
		connect( ctls_btn, SIGNAL( clicked() ), this, SLOT( editControls() ) );
	}


	setWhatsThis( tr(
"Effect plugins function as a chained series of effects where the signal will "
"be processed from top to bottom.\n\n"

"The On/Off switch allows you to bypass a given plugin at any point in "
"time.\n\n"

"The Wet/Dry knob controls the balance between the input signal and the "
"effected signal that is the resulting output from the effect.  The input "
"for the stage is the output from the previous stage. So, the 'dry' signal "
"for effects lower in the chain contains all of the previous effects.\n\n"

"The Decay knob controls how long the signal will continue to be processed "
"after the notes have been released.  The effect will stop processing signals "
"when the volume has dropped below a given threshold for a given length of "
"time.  This knob sets the 'given length of time'.  Longer times will require "
"more CPU, so this number should be set low for most effects.  It needs to be "
"bumped up for effects that produce lengthy periods of silence, e.g. "
"delays.\n\n"

"The Gate knob controls the 'given threshold' for the effect's auto shutdown.  "
"The clock for the 'given length of time' will begin as soon as the processed "
"signal level drops below the level specified with this knob.\n\n"

"The Controls button opens a dialog for editing the effect's parameters.\n\n"

"Right clicking will bring up a context menu where you can change the order "
"in which the effects are processed or delete an effect altogether." ) );

	//move above vst effect view creation
	//setModel( _model );
}




EffectView::~EffectView()
{

#ifdef LMMS_BUILD_LINUX

	delete m_subWindow;
#else
	if( m_subWindow )
	{
		// otherwise on win32 build VST GUI can get lost
		m_subWindow->hide();
	}
#endif

}




void EffectView::editControls()
{
	if( m_controlView == NULL )
	{
		m_controlView = effect()->controls()->createView();
		if( m_controlView )
		{
			/*
			  m_subWindow = gui->mainWindow()->addWindowedWidget( m_controlView );
			  Qt::WindowFlags flags = m_subWindow->windowFlags();
			  flags &= ~Qt::WindowMaximizeButtonHint;
			  m_subWindow->setWindowFlags( flags );
			  m_subWindow->resize(m_subWindow->sizeHint());
			  m_subWindow->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
			  //m_subWindow->setFixedSize( m_subWindow->sizeHint() );
			m_subWindow->setWindowIcon( m_controlView->windowIcon() );
			*/
			m_subWindow = SubWindow::putWidgetOnWorkspace
				(m_controlView,false,false,false,false);

			connect( m_controlView, SIGNAL( closed() ),
				 this, SLOT( closeEffects() ) );
			m_subWindow->hide();
		}
	}

	if( m_subWindow )
	{
		if( !m_subWindow->isVisible() )
		{
			m_subWindow->show();
			m_subWindow->raise();
			//effect()->controls()->setViewVisible( true );
		}
		else
		{
			m_subWindow->hide();
			//effect()->controls()->setViewVisible( false );
		}
	}
}




void EffectView::moveUp()
{
	emit moveUp( this );
}




void EffectView::moveDown()
{
	emit moveDown( this );
}



void EffectView::deletePlugin()
{
	emit deletePlugin( this );
}




void EffectView::displayHelp()
{
	QWhatsThis::showText( mapToGlobal( rect().bottomRight() ),
			      whatsThis() );
}




void EffectView::closeEffects()
{
	if( m_subWindow )
	{
		m_subWindow->hide();
	}
	//effect()->controls()->setViewVisible( false );
}



void EffectView::contextMenuEvent( QContextMenuEvent * )
{
	QPointer<CaptionMenu> contextMenu = new CaptionMenu( model()->displayName(), this );
	contextMenu->addAction( embed::getIconPixmap( "arp_up" ),
						tr( "Move &up" ),
						this, SLOT( moveUp() ) );
	contextMenu->addAction( embed::getIconPixmap( "arp_down" ),
						tr( "Move &down" ),
						this, SLOT( moveDown() ) );
	contextMenu->addSeparator();
	contextMenu->addAction( embed::getIconPixmap( "cancel" ),
						tr( "&Remove this plugin" ),
						this, SLOT( deletePlugin() ) );
	contextMenu->addSeparator();
	contextMenu->addHelpAction();
	contextMenu->exec( QCursor::pos() );
	delete contextMenu;
}




void EffectView::paintEvent( QPaintEvent * )
{
	QPainter p( this );
	p.drawPixmap( 0, 0, m_bg );

	QFont f = pointSizeF( font(), 7.5f );
	f.setBold( true );
	p.setFont( f );

	p.setPen( palette().shadow().color() );
	p.drawText( 7, 54, model()->displayName() );
	p.setPen( palette().text().color() );
	p.drawText( 6, 53, model()->displayName() );
}




void EffectView::modelChanged()
{
        Effect* e=effect();
	m_bypass->setModel( &e->m_enabledModel );
	m_wetDry->setModel( &e->m_wetDryModel );
	m_autoQuit->setModel( &e->m_autoQuitModel );
	m_gate->setModel( &e->m_gateModel );
	m_balance->setModel( &e->m_balanceModel );
        m_balance->setVisible( e->isBalanceable() );
}
