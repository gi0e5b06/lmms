/*
 * CarlaEffectControlDialog.cpp
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

#include <QVBoxLayout>

#include "CarlaEffectControlDialog.h"
#include "CarlaEffectControls.h"
#include "CarlaEffect.h"
#include "embed.h"
#include "gui_templates.h"



CarlaEffectControlDialog::CarlaEffectControlDialog( CarlaEffectControls* controls ) :
	EffectControlDialog( controls ),
        fHandle(controls->m_effect->fHandle),
        fDescriptor(controls->m_effect->fDescriptor),
        fTimerId(fHandle != NULL && fDescriptor->ui_idle != NULL ? startTimer(30) : 0)
{
        /*
        if (QWidget* const window = parent->window())
                controls->m_effect->fHost.uiParentId = window->winId();
        else
        */
        controls->m_effect->fHost.uiParentId = 0;
        std::free((char*)controls->m_effect->fHost.uiName);
        // TODO - get plugin instance name
        //fHost.uiName = strdup(parent->windowTitle().toUtf8().constData());
        controls->m_effect->fHost.uiName=
                strdup(controls->m_effect->kIsPatchbay
                       ? "CarlaEffectBay-LMMS"
                       : "CarlaEffectRack-LMMS");

        //return new CarlaEffectView(this, parent);

        setAutoFillBackground( true );
	QPalette pal;
        pal.setBrush(backgroundRole(),
                     controls->m_effect->kIsPatchbay
                     ? PLUGIN_NAME::getIconPixmap("artwork-patchbay")
                     : PLUGIN_NAME::getIconPixmap("artwork-rack"));
	setPalette( pal );
	setFixedSize( 250, 250 );

	//QLabel* lb=new QLabel("Controls",this);
	//lb->setGeometry(10,10,130,140);

    QVBoxLayout * l = new QVBoxLayout( this );
    l->setContentsMargins( 10, 160, 10, 10 );
    l->setSpacing( 10 );

    m_toggleUIButton = new QPushButton( tr( "Show GUI" ), this );
    //m_toggleUIButton->setGeometry(10,10,130,140);
    m_toggleUIButton->setCheckable( true );
    m_toggleUIButton->setChecked( false );
    m_toggleUIButton->setIcon( embed::getIconPixmap( "zoom" ) );
    m_toggleUIButton->setFont( pointSize<8>( m_toggleUIButton->font() ) );
    connect( m_toggleUIButton, SIGNAL( clicked(bool) ), this, SLOT( toggleUI( bool ) ) );

    m_toggleUIButton->setWhatsThis(
                tr( "Click here to show or hide the graphical user interface (GUI) of Carla." ) );

    l->addWidget( m_toggleUIButton );
    l->addStretch();

    connect(controls->m_effect, SIGNAL(uiClosed()), this, SLOT(uiClosed()));
}


CarlaEffectControlDialog::~CarlaEffectControlDialog()
{
    if (m_toggleUIButton->isChecked())
        toggleUI(false);
}


void CarlaEffectControlDialog::toggleUI(bool visible)
{
    if (fHandle != NULL && fDescriptor->ui_show != NULL)
        fDescriptor->ui_show(fHandle, visible);
}

void CarlaEffectControlDialog::uiClosed()
{
    m_toggleUIButton->setChecked(false);
}

void CarlaEffectControlDialog::modelChanged()
{
}

void CarlaEffectControlDialog::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == fTimerId)
        fDescriptor->ui_idle(fHandle);

    EffectControlDialog::timerEvent(event);
}

