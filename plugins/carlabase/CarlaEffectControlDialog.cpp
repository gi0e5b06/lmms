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
#include "Song.h"

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
	setMinimumSize(250,310);

	//QLabel* lb=new QLabel("Controls",this);
	//lb->setGeometry(10,10,130,140);

        QVBoxLayout* l = new QVBoxLayout( this );
        l->setContentsMargins( 8, 66, 8, 8 );
        l->setSpacing( 10 );

        m_toggleUIButton = new QPushButton( tr( "Show GUI" ), this );
        //m_toggleUIButton->setGeometry(10,10,130,140);
        m_toggleUIButton->setCheckable( true );
        m_toggleUIButton->setChecked( false );
        m_toggleUIButton->setIcon( embed::getIconPixmap( "zoom" ) );
        m_toggleUIButton->setFont( pointSize<8>( m_toggleUIButton->font() ) );
        connect( m_toggleUIButton, SIGNAL( clicked(bool) ), this, SLOT( toggleUI( bool ) ) );
        m_toggleUIButton->setWhatsThis
                ( tr( "Click here to show or hide the graphical user interface (GUI) of Carla." ) );
        l->addWidget( m_toggleUIButton );

        QWidget* kg=new QWidget(this);
        kg->setFixedSize(228,117+21+34);
        for(int i=0;i<NB_KNOBS;i++)
        {
                m_knobs[i]=new Knob(knobBright_26,kg);
                m_knobs[i]->setText(QString("CC %1").arg(NB_KNOB_START+i));
                m_knobs[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
                                        .arg(MIDI_CH).arg(NB_KNOB_START+i),"");
                m_knobs[i]->setWhatsThis("");
                m_knobs[i]->setGeometry(39*(i%6),39*(i/6),39,39);
                m_knobs[i]->setModel(controls->m_knobs[i]);
                connect(controls->m_knobs[i], SIGNAL(dataChanged()),
                        this, SLOT(onDataChanged()));
        }
        for(int i=0;i<NB_LEDS;i++)
        {
                m_leds[i]=new LedCheckBox(kg);
                m_leds[i]->setText(QString("CC %1").arg(NB_LED_START+i));
                m_leds[i]->setTextAnchorPoint(Qt::AnchorBottom);
                //m_leds[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
                //                        .arg(MIDI_CH).arg(NB_LED_START+i),"");
                //m_leds[i]->setWhatsThis("");
                m_leds[i]->setGeometry(29*(i%8),117+21*(i/8),29,21);
                m_leds[i]->setModel(controls->m_leds[i]);
                connect(controls->m_leds[i], SIGNAL(dataChanged()),
                        this, SLOT(onDataChanged()));
        }
        for(int i=0;i<NB_LCDS;i++)
        {
                m_lcds[i]=new LcdSpinBox(3,kg);
                m_lcds[i]->setText(QString("CC %1").arg(NB_LCD_START+i));
                //m_lcds[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
                //                        .arg(MIDI_CH).arg(NB_LCD_START+i),"");
                //m_lcds[i]->setWhatsThis("");
                m_lcds[i]->setGeometry(58*(i%4),117+21+34*(i/4),58,34);
                m_lcds[i]->setModel(controls->m_lcds[i]);
                connect(controls->m_lcds[i],SIGNAL(dataChanged()),
                        this, SLOT(onDataChanged()));
        }
        l->addWidget(kg);

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

void CarlaEffectControlDialog::onDataChanged()
{
        QObject* o=sender();
        if(!o) qInfo("no sender");
        AutomatableModel* m=dynamic_cast<AutomatableModel*>(sender());
        if(m)
        {
                int cc=-1;
                int v =-1;
                if(cc==-1)
                        for(int i=0;i<NB_KNOBS;i++)
                                if(m_knobs[i]->model()==m)
                                {
                                        cc=NB_KNOB_START+i;
                                        v =m_knobs[i]->model()->value();
                                }
                if(cc==-1)
                        for(int i=0;i<NB_LEDS;i++)
                                if(m_leds[i]->model()==m)
                                {
                                        cc=NB_LED_START+i;
                                        v =m_leds[i]->model()->value();
                                }

                if(cc==-1)
                        for(int i=0;i<NB_LCDS;i++)
                                if(m_lcds[i]->model()==m)
                                {
                                        cc=NB_LCD_START+i;
                                        v =m_lcds[i]->model()->value();
                                }
                if(cc!=-1)
                {
                        qInfo("cc %d: data changed: %d",cc,v);
                        CarlaEffect* fx=dynamic_cast<CarlaEffect*>(controls()->effect());
                        if(fx)
                        {
                                MidiEvent ev(MidiEventTypes::MidiControlChange,
                                             MIDI_CH-1,
                                             cc,
                                             v);
                                Song::PlayPos pos=Engine::getSong()->getPlayPos();
                                //qInfo("sending midi event");
                                fx->handleMidiEvent(ev,pos,pos.currentFrame());
                        }
                        else qInfo("fx is null");
                }
                else qInfo("cc model not found");
        }
        else qInfo("sender but no model");
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

