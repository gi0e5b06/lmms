/*
 * LadspaControlView.cpp - widget for controlling a LADSPA port
 *
 * Copyright (c) 2006-2008 Danny McRae <khjklujn/at/users.sourceforge.net>
 * Copyright (c) 2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "LadspaControlView.h"

#include "Knob.h"
#include "LadspaBase.h"  // REQUIRED
#include "LadspaControl.h"
#include "LedCheckBox.h"
#include "TempoSyncKnob.h"
#include "ToolTip.h"

#include <QHBoxLayout>

LadspaControlView::LadspaControlView(QWidget*       _parent,
                                     LadspaControl* _ctl,
                                     bool           _named) :
      QWidget(_parent),
      ModelView(_ctl, this), m_ctl(_ctl)
{
    setObjectName(
            QString("LadspaControlView-%1").arg((unsigned long)this, 16));
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setStretch(2, 1);

    if(m_ctl->m_link)
    {
        LedCheckBox* link = new LedCheckBox("", this);
        link->setFixedWidth(20);
        link->setModel(&m_ctl->m_linkEnabledModel);
        ToolTip::add(link, tr("Link channels"));
        layout->addWidget(link, 0, Qt::AlignHCenter | Qt::AlignBottom);
        connect(m_ctl, SIGNAL(linkChanged(int, bool)), this, SLOT(update()));
    }
    else
    {
        QWidget* w = new QWidget(this);
        w->setFixedWidth(20);
        layout->addWidget(w, 0, Qt::AlignHCenter | Qt::AlignBottom);
        connect(m_ctl, SIGNAL(linkChanged(int, bool)), this, SLOT(update()));
    }

    Knob* knb = NULL;

    switch(m_ctl->port()->data_type)
    {
        case TOGGLED:
        {
            LedCheckBox* toggle = new LedCheckBox(
                    (_named ? m_ctl->port()->name : ""), this,
                    m_ctl->port()->name,  // QString::null
                    LedCheckBox::Green);
            // qInfo("LadspaControlView: LedCheckBox %p
            // name=%s",toggle,qPrintable(m_ctl->port()->name));
            toggle->setTextAnchorPoint(Qt::AnchorBottom);
            toggle->setModel(m_ctl->toggledModel());
            layout->addWidget(toggle, 1, Qt::AlignHCenter | Qt::AlignBottom);
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
            knb = new Knob(/*knobBright_26,*/ this,
                           _named ? m_ctl->port()->name : "");
            // qInfo("LadspaControlView: Knob %p
            // name=%s",knb,qPrintable(m_ctl->port()->name));
            break;

        case TIME:
            knb = new TempoSyncKnob(/*knobBright_26,*/ this,
                                    _named ? m_ctl->port()->name : "");
            // qInfo("LadspaControlView: TempoSyncKnob %p
            // name=%s",knb,qPrintable(m_ctl->port()->name));
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
        // knb->setText( m_ctl->port()->name );
        knb->setHintText(tr("Value:"), "");
        knb->setWhatsThis(tr("Sorry, no help available."));
        layout->addWidget(knb, 1, Qt::AlignHCenter | Qt::AlignBottom);
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

LadspaControlView::~LadspaControlView()
{
}

void LadspaControlView::update()
{
    /*
      qInfo("update p=%p", this);

      QWidget* p = parentWidget();
if(p)
{
    QList<Widget*> list = p->findChildren<Widget*>();
    for(QList<Widget*>::iterator it = list.begin(); it != list.end(); ++it)
        (*it)->invalidateCache();
    p->QWidget::update();
}
else
    */
        
    QWidget::update();

    /*
    QWidget* w=this;
    QWidget* p=w->parentWidget();
    if(p) p->update();
    else  w->update();
    */
}
