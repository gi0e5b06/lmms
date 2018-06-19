/*
 * CarlaEffectView.cpp
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

#include <QLabel>
#include <QVBoxLayout>

#include "embed.h"
#include "gui_templates.h"
#include "CarlaEffect.h"
#include "CarlaEffectView.h"


CarlaEffectView::CarlaEffectView(CarlaEffect* const effect, QWidget* const parent)
    : EffectView(effect, parent),
      fHandle(effect->fHandle),
      fDescriptor(effect->fDescriptor),
      fTimerId(fHandle != NULL && fDescriptor->ui_idle != NULL ? startTimer(30) : 0)
{
    setAutoFillBackground(true);

    QPalette pal;
    pal.setBrush(backgroundRole(), effect->kIsPatchbay ? PLUGIN_NAME::getIconPixmap("artwork-patchbay") : PLUGIN_NAME::getIconPixmap("artwork-rack"));
    setPalette(pal);

    QVBoxLayout * l = new QVBoxLayout( this );
    l->setContentsMargins( 20, 180, 10, 10 );
    l->setSpacing( 10 );

    m_toggleUIButton = new QPushButton( tr( "Show GUI" ), this );
    m_toggleUIButton->setCheckable( true );
    m_toggleUIButton->setChecked( false );
    m_toggleUIButton->setIcon( embed::getIconPixmap( "zoom" ) );
    m_toggleUIButton->setFont( pointSize<8>( m_toggleUIButton->font() ) );
    connect( m_toggleUIButton, SIGNAL( clicked(bool) ), this, SLOT( toggleUI( bool ) ) );

    m_toggleUIButton->setWhatsThis(
                tr( "Click here to show or hide the graphical user interface (GUI) of Carla." ) );

    l->addWidget( m_toggleUIButton );
    l->addStretch();

    connect(effect, SIGNAL(uiClosed()), this, SLOT(uiClosed()));
}

CarlaEffectView::~CarlaEffectView()
{
    if (m_toggleUIButton->isChecked())
        toggleUI(false);
}

void CarlaEffectView::toggleUI(bool visible)
{
    if (fHandle != NULL && fDescriptor->ui_show != NULL)
        fDescriptor->ui_show(fHandle, visible);
}

void CarlaEffectView::uiClosed()
{
    m_toggleUIButton->setChecked(false);
}

void CarlaEffectView::modelChanged()
{
}

void CarlaEffectView::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == fTimerId)
        fDescriptor->ui_idle(fHandle);

    EffectView::timerEvent(event);
}

