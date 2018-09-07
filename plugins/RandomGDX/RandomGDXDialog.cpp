/*
 * RandomGDXDialog.cpp -
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

#include "RandomGDXDialog.h"

#include "RandomGDXControls.h"
#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>

RandomGDXDialog::RandomGDXDialog(RandomGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    setPalette(pal);

    QGridLayout* m_mainLayout = new QGridLayout(this);
    m_mainLayout->setContentsMargins(6, 6, 6, 6);
    m_mainLayout->setSpacing(6);

    Knob* rndAmpKNB = new Knob(this);
    // rndAmpKNB->move(17, 35);
    rndAmpKNB->setModel(&controls->m_rndAmpModel);
    rndAmpKNB->setText(tr("RAND"));
    rndAmpKNB->setHintText(tr("Random Amp:"), "");

    Knob* fixAmpKNB = new Knob(this);
    // fixAmpKNB->move(67, 35);
    fixAmpKNB->setModel(&controls->m_fixAmpModel);
    fixAmpKNB->setText(tr("BASE"));
    fixAmpKNB->setHintText(tr("Base Amp:"), "");

    Knob* sngPosKNB = new Knob(this);
    // sngPosKNB->move(17, 85);
    sngPosKNB->setModel(&controls->m_sngPosModel);
    sngPosKNB->setText(tr("SINGL"));
    sngPosKNB->setHintText(tr("Singularity:"), "");

    Knob* delPosKNB = new Knob(this);
    // delPosKNB->move(67, 85);
    delPosKNB->setModel(&controls->m_delPosModel);
    delPosKNB->setText(tr("DELAY"));
    delPosKNB->setHintText(tr("Delay:"), "");

    m_mainLayout->addWidget(rndAmpKNB, 0, 0, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(fixAmpKNB, 0, 1, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(sngPosKNB, 0, 2, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(delPosKNB, 0, 3, Qt::AlignHCenter | Qt::AlignTop);

    m_mainLayout->setColumnStretch(4, 1);
    m_mainLayout->setRowStretch(1, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
