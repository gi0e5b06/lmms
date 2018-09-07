/*
 * ClickGDXDialog.cpp - control dialog for click remover effect
 *
 * Copyright (c) 2017-2018 gi0e5b06 (on github.com)
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

#include "ClickGDXDialog.h"

#include <QGridLayout>
//#include <QGroupBox>
#include <QLabel>

#include "ClickGDXControls.h"
#include "embed.h"

ClickGDXDialog::ClickGDXDialog(ClickGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    // pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    QGridLayout* m_mainLayout = new QGridLayout(this);
    m_mainLayout->setContentsMargins(6, 6, 6, 6);
    m_mainLayout->setSpacing(6);

    m_mainLayout->addWidget(new QLabel(tr("Attack"), this), 1, 0);
    m_mainLayout->addWidget(new QLabel(tr("Descent"), this), 2, 0);
    m_mainLayout->addWidget(new QLabel(tr("Panning"), this), 3, 0);

    m_mainLayout->addWidget(new QLabel(tr("Time"), this), 0, 1);
    m_mainLayout->addWidget(new QLabel(tr("Type"), this), 0, 2);
    m_mainLayout->addWidget(new QLabel(tr("Tempo"), this), 0, 3);

    Knob* attTimeKNB = new Knob(this);
    attTimeKNB->setModel(&controls->m_attackTimeModel);
    attTimeKNB->setHintText(tr("Attack Time:"), " beat");

    Knob* desTimeKNB = new Knob(this);
    desTimeKNB->setModel(&controls->m_descentTimeModel);
    desTimeKNB->setHintText(tr("Descent Time:"), " beat");

    Knob* attTypeKNB = new Knob(this);
    attTypeKNB->setModel(&controls->m_attackTypeModel);
    attTypeKNB->setHintText(tr("Attack Type:"), "");

    Knob* desTypeKNB = new Knob(this);
    desTypeKNB->setModel(&controls->m_descentTypeModel);
    desTypeKNB->setHintText(tr("Descent Type:"), "");

    Knob* attTempoKNB = new Knob(this);
    attTempoKNB->setModel(&controls->m_attackTempoModel);
    attTempoKNB->setHintText(tr("Attack Tempo:"), "");

    Knob* desTempoKNB = new Knob(this);
    desTempoKNB->setModel(&controls->m_descentTempoModel);
    desTempoKNB->setHintText(tr("Descent Tempo:"), "");

    Knob* panTimeKNB = new Knob(this);
    panTimeKNB->setModel(&controls->m_panTimeModel);
    panTimeKNB->setHintText(tr("Pan Time:"), "");

    Knob* panTypeKNB = new Knob(this);
    panTypeKNB->setModel(&controls->m_panTypeModel);
    panTypeKNB->setHintText(tr("Pan Type:"), "");

    Knob* panTempoKNB = new Knob(this);
    panTempoKNB->setModel(&controls->m_panTempoModel);
    panTempoKNB->setHintText(tr("Pan Tempo:"), "");

    m_mainLayout->addWidget(attTimeKNB, 1, 1,
                            Qt::AlignHCenter | Qt::AlignBottom);
    m_mainLayout->addWidget(desTimeKNB, 1, 2,
                            Qt::AlignHCenter | Qt::AlignBottom);
    m_mainLayout->addWidget(panTimeKNB, 1, 3,
                            Qt::AlignHCenter | Qt::AlignBottom);

    m_mainLayout->addWidget(attTypeKNB, 2, 1,
                            Qt::AlignHCenter | Qt::AlignBottom);
    m_mainLayout->addWidget(desTypeKNB, 2, 2,
                            Qt::AlignHCenter | Qt::AlignBottom);
    m_mainLayout->addWidget(panTypeKNB, 2, 3,
                            Qt::AlignHCenter | Qt::AlignBottom);

    m_mainLayout->addWidget(attTempoKNB, 3, 1,
                            Qt::AlignHCenter | Qt::AlignBottom);
    m_mainLayout->addWidget(desTempoKNB, 3, 2,
                            Qt::AlignHCenter | Qt::AlignBottom);
    m_mainLayout->addWidget(panTempoKNB, 3, 3,
                            Qt::AlignHCenter | Qt::AlignBottom);

    m_mainLayout->setColumnStretch(5, 1);
    m_mainLayout->setRowStretch(4, 1);

    //int wc=m_mainLayout->columnWidth(3);
    //m_mainLayout->setColumnWidth(1,wc);
    //m_mainLayout->setColumnWidth(2,wc);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
