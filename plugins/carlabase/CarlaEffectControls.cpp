/*
 * CarlaEffectControls.cpp - controls for click remover effect
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


#include <QDomElement>

#include "CarlaEffectControlDialog.h"
#include "CarlaEffectControls.h"
#include "CarlaEffect.h"
//#include "Engine.h"
//#include "Song.h"


CarlaEffectControls::CarlaEffectControls( CarlaEffect* effect ) :
	EffectControls( effect ),
	m_effect( effect )
{
    for(int i=0;i<NB_KNOBS;i++)
    {
            m_knobs[i]=new FloatModel
                    (0.f,0.f,127.f,1.f,
                     NULL,QString("KNB%1").arg(NB_KNOB_START+i),false);
    }
    for(int i=0;i<NB_LEDS;i++)
    {
            m_leds[i]=new BoolModel
                    (false,NULL,QString("LED%1").arg(NB_LED_START+i),false);
    }
    for(int i=0;i<NB_LCDS;i++)
    {
            m_lcds[i]=new IntModel
                    (0,0,127,
                     NULL,QString("LCD%1").arg(NB_LCD_START+i),false);
    }
}


void CarlaEffectControls::changeControl()
{
}


void CarlaEffectControls::loadSettings( const QDomElement& elem )
{
        qInfo("CarlaEffectControls::loadSettings start");

        for(int i=0;i<NB_KNOBS;i++)
                m_knobs[i]->loadSettings( elem, QString("knob%1").arg(i) );
        for(int i=0;i<NB_LEDS;i++)
                m_leds[i]->loadSettings( elem, QString("led%1").arg(i) );
        for(int i=0;i<NB_LCDS;i++)
                m_lcds[i]->loadSettings( elem, QString("lcd%1").arg(i) );

        if (m_effect->fHandle == NULL ||
            m_effect->fDescriptor->set_state == NULL)
        {
                qWarning("CarlaEffectControls::loadSettings fHandle or fDescriptor is null");
                return;
        }

	QDomNode node=elem.firstChild();
        if( node.isNull() )
        {
                qWarning("elem first child is null");
        }
	while( !node.isNull() )
	{
                if( !node.isElement() )
                {
                        qWarning("elem not an element: %s",qPrintable(node.nodeName()));
                }
		if( node.isElement() )
		{
                        //qInfo("node name: %s",qPrintable(node.nodeName()));
			if( node.nodeName() == "CARLA-PROJECT" )
			{
                                QDomDocument carlaDoc("carla");
                                carlaDoc.appendChild(carlaDoc.importNode(node, true ));
                                m_effect->fDescriptor->set_state(m_effect->fHandle,
                                                                 carlaDoc.toString(0).toUtf8().constData());
			}
		}
		node = node.nextSibling();
	}

        qInfo("CarlaEffectControls::loadSettings end");
}


void CarlaEffectControls::saveSettings( QDomDocument& doc, QDomElement& parent )
{
        for(int i=0;i<NB_KNOBS;i++)
                m_knobs[i]->saveSettings( doc, parent, QString("knob%1").arg(i) );
        for(int i=0;i<NB_LEDS;i++)
                m_leds[i]->saveSettings( doc, parent, QString("led%1").arg(i) );
        for(int i=0;i<NB_LCDS;i++)
                m_lcds[i]->saveSettings( doc, parent, QString("lcd%1").arg(i) );

        if (m_effect->fHandle == NULL ||
            m_effect->fDescriptor->get_state == NULL)
        {
                qWarning("CarlaEffectControls::saveSettings fHandle or fDescriptor is null");
                return;
        }

        char* const state = m_effect->fDescriptor->get_state(m_effect->fHandle);

        if (state == NULL)
        {
                qWarning("CarlaEffectControls: retrieved state is null");
                return;
        }

        QDomDocument carlaDoc("carla");

        if (carlaDoc.setContent(QString(state)))
        {
                QDomNode n = doc.importNode(carlaDoc.documentElement(), true);
                parent.appendChild(n);
        }

        std::free(state);
}


EffectControlDialog* CarlaEffectControls::createView()
{
        return new CarlaEffectControlDialog(this);
}
