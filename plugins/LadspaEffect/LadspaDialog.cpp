/*
 * LadspaDialog.cpp - dialog for displaying and editing control port
 *                             values for LADSPA plugins
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

#include "LadspaDialog.h"

#include "LadspaControlView.h"
#include "LadspaEffect.h"
#include "LedCheckBox.h"
#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>

#include <cmath>

LadspaDialog::LadspaDialog(LadspaControls* _ctl) :
      EffectControlDialog(_ctl), m_effectLayout(NULL), m_stereoLink(NULL)
{
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getPixmap("plugin_bg"));
    // pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(3);
    mainLayout->setRowStretch(0,1);
    mainLayout->setColumnStretch(0,1);

    /*
    QScrollArea* m_scrollArea = new QScrollArea();
    m_scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    m_scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    //m_scrollArea->setPalette( QApplication::palette( m_scrollArea ) );
    m_scrollArea->setFrameStyle( QFrame::NoFrame );
    m_scrollArea->setLineWidth( 1 );
    */
    //m_scrollArea->setAutoFillBackground(true);
    //QPalette pal;
    //pal.setBrush(backgroundRole(), embed::getPixmap("plugin_bg"));
    // pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    //m_scrollArea->setPalette(pal);

    m_pane=new QWidget();
    m_effectLayout = new QGridLayout(m_pane);
    m_effectLayout->setColumnStretch(0, 1);
    m_effectLayout->setContentsMargins(3,3,3,3);
    m_effectLayout->setSpacing(3);
    m_pane->setLayout(m_effectLayout);

    //pane->setFixedWidth(234);
    ////m_scrollArea->setWidget(m_pane);
    //mainLayout->addLayout(m_effectLayout, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    ////mainLayout->addWidget(m_scrollArea, 0, 0);//, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->addWidget(m_pane, 0, 0);//, Qt::AlignLeft | Qt::AlignTop);

    updateEffectView(_ctl);

    if(_ctl->m_processors > 1)
    {
        m_stereoLink = new LedCheckBox(tr("Link Channels"), this);
        m_stereoLink->setModel(&_ctl->m_stereoLinkModel);
        mainLayout->addWidget(m_stereoLink, 1, 0,
                              Qt::AlignLeft | Qt::AlignTop);
    }

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}

LadspaDialog::~LadspaDialog()
{
}

void LadspaDialog::updateEffectView(LadspaControls* _ctl)
{
    QList<QWidget*> list = m_effectLayout->findChildren<QWidget*>();
    for(QList<QWidget*>::iterator it = list.begin(); it != list.end(); ++it)
    {
        delete *it;
    }

    m_effectControls = _ctl;

    /*	const int cols = static_cast<int>( sqrt(
            static_cast<double>( _ctl->m_controlCount /
                                            _ctl->m_processors ) ) );
    */

    bool multi = (_ctl->m_processors > 1);

    if(multi)
    {
        m_effectLayout->addWidget(new QLabel(tr("Port")), 0, 0);
        m_effectLayout->addWidget(new QLabel(tr("Left")), 0, 1);
        m_effectLayout->addWidget(new QLabel(tr("Right")), 0, 2);
    }
    else
    {
        m_effectLayout->addWidget(new QLabel(tr("Port")), 0, 0, 1, 1);
        m_effectLayout->addWidget(new QLabel(tr("Stereo")), 0, 1, 1, 2);
    }

    //int cw=0;
    int rh=0;
    for(ch_cnt_t proc = 0; proc < _ctl->m_processors; proc++)
    {
        control_list_t& controls = _ctl->m_controls[proc];
        int row = 0;
        int col = 0;

        for(control_list_t::iterator it = controls.begin();
            it != controls.end(); ++it)
        {
            if((*it)->port()->proc != proc)
            {
                qWarning("LadspaDialog::updateEffectView strange, proc=%d cp=%d",
                         proc, (*it)->port()->proc);
                continue;
            }

            if(multi)
            {
                ++row;
                if((*it)->isLink())
                {
                    QLabel* lbl = new QLabel((*it)->port()->name, this);
                    m_effectLayout->addWidget(lbl, row, 0);
                    rh=qMax<int>(rh,lbl->sizeHint().height());
                }

                col = proc+1;
                QWidget* qw = new LadspaControlView(this, *it, false);
                m_effectLayout->addWidget(qw, row, col);
                rh=qMax<int>(rh,qw->sizeHint().height());
                qInfo("add r=%d, c=%d, %s", row, col,
                      qPrintable((*it)->port()->name));
            }
            else
            {
                ++row;
                QLabel* lbl = new QLabel((*it)->port()->name, this);
                m_effectLayout->addWidget(lbl, row, 0);
                QWidget* qw = new LadspaControlView(this, *it, false);
                m_effectLayout->addWidget(qw, row, 1, 1, 2);
                //cw=qMax<int>(cw,qw->sizeHint().width());
                rh=qMax<int>(rh,lbl->sizeHint().height());
                rh=qMax<int>(rh,qw->sizeHint().height());
            }
        }

        /*
        int row = 0;
        int col = 0;

        QGroupBox * grouper;
        if( _ctl->m_processors > 1 )
        {
                grouper = new QGroupBox( tr( "Channel " ) +
                                        QString::number( proc + 1 ),
                                                        this );
        }
        else
        {
                grouper = new QGroupBox( tr( "Stereo" ), this );
        }

        QGridLayout * gl = new QGridLayout( grouper );
        grouper->setLayout( gl );
        grouper->setAlignment( Qt::Vertical );

        int cw=0,rh=0;
        for( control_list_t::iterator it = controls.begin(); it !=
        controls.end(); ++it )
        {
                if( (*it)->port()->proc == proc )
                {
                        if( last_port != NONE &&
                            (*it)->port()->data_type == TOGGLED &&
                            !( (*it)->port()->data_type == TOGGLED &&
                               last_port == TOGGLED ) )
                        {
                                ++row;
                                col = 0;
                        }
                        QWidget* qw=new LadspaControlView( grouper, *it );
                        gl->addWidget(qw,row,col);
                        cw=qMax<int>(cw,qw->sizeHint().width());
                        rh=qMax<int>(rh,qw->sizeHint().height());
                        if( ++col == cols )
                        {
                                ++row;
                                col = 0;
                        }
                        last_port = (*it)->port()->data_type;
                }
        }

        const int rows=(col==0 ? row : row+1);

        for(col=0;col<cols;col++)
        {
                gl->setColumnMinimumWidth(col,cw);
                gl->setColumnStretch(col,1.f);
        }

        for(row=0;row<rows;row++)
        {
                gl->setRowMinimumHeight(row,rh);
                gl->setRowStretch(row,1.f);
        }

        m_effectLayout->addWidget( grouper );
        */
    }

    for(int row=0;row<m_effectLayout->rowCount();row++)
    {
            m_effectLayout->setRowMinimumHeight(row,rh);
            //m_effectLayout->setRowStretch(row,1);
    }
    m_effectLayout->setColumnStretch(m_effectLayout->columnCount(),1);
    m_effectLayout->setRowStretch(m_effectLayout->rowCount(),1);

    if(multi && m_stereoLink != NULL)
    {
        m_stereoLink->setModel(&_ctl->m_stereoLinkModel);
    }

    connect(_ctl, SIGNAL(effectModelChanged(LadspaControls*)), this,
            SLOT(updateEffectView(LadspaControls*)), Qt::DirectConnection);
}
