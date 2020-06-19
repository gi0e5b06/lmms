/*
 * LV2ControlView.cpp - widget for controlling a LV2 port
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

#include "LV2ControlView.h"

#include "LV2Control.h"

#include <QHBoxLayout>
//#include "LV2Base.h"
#include "LedCheckBox.h"
#include "TempoSyncKnob.h"
#include "ToolTip.h"

LV2ControlView::LV2ControlView(QWidget* _parent, LV2Control* _ctl) :
      QWidget(_parent), ModelView(_ctl, this), m_ctl(_ctl)
{
    setObjectName(QString("lv2ControlView%1").arg((unsigned long)this, 16));
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);

    LedCheckBox* link = NULL;

    if(m_ctl->m_link)
    {
        link = new LedCheckBox("", this);
        link->setModel(&m_ctl->m_linkEnabledModel);
        ToolTip::add(link, tr("Link channels"));
        layout->addWidget(link);
    }

    Knob* knb = NULL;

    switch(m_ctl->port()->data_type)
    {
        case TOGGLED:
        {
            LedCheckBox* toggle
                    = new LedCheckBox(m_ctl->port()->name, this,
                                      m_ctl->port()->name,  // QString::null
                                      LedCheckBox::Green);
            qWarning("LV2ControlView: LedCheckBox %p name=%s", toggle,
                     qPrintable(m_ctl->port()->name));
            toggle->setModel(m_ctl->toggledModel());
            layout->addWidget(toggle);
            /*
            if( link != NULL )
            {
                    setFixedSize( link->width() + toggle->width(),
                                            toggle->height() );
            }
            else
            {
                    setFixedSize( toggle->width(),
                                            toggle->height() );
            }
            */
            break;
        }

        case INTEGER:
        case FLOATING:
            knb = new Knob(knobBright_26, this, m_ctl->port()->name);
            qWarning("LV2ControlView: Knob %p name=%s", knb,
                     qPrintable(m_ctl->port()->name));
            break;

        case TIME:
            knb = new TempoSyncKnob(knobBright_26, this, m_ctl->port()->name);
            qWarning("LV2ControlView: TempoSyncKnob %p name=%s", knb,
                     qPrintable(m_ctl->port()->name));
            break;

        default:
            break;
    }

    if(knb != NULL)
    {
        if(m_ctl->port()->data_type != TIME)
        {
            knb->setModel(m_ctl->knobModel());
        }
        else
        {
            knb->setModel(m_ctl->tempoSyncKnobModel());
        }
        knb->setLabel(m_ctl->port()->name);
        knb->setHintText(tr("Value:"), "");
        knb->setWhatsThis(tr("Sorry, no help available."));
        layout->addWidget(knb);
        /*
        if( link != NULL )
        {
                setFixedSize( link->width() + knb->width(),
                                        knb->height() );
        }
        else
        {
                setFixedSize( knb->width(), knb->height() );
        }
        */
    }
}

LV2ControlView::~LV2ControlView()
{
}
