/*
 * ScatterGDXDialog.cpp -
 *
 * Copyright (c) 2017 gi0e5b06 (on github.com)
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

#include "ScatterGDXDialog.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>

#include "ScatterGDXControls.h"
#include "embed.h"

ScatterGDXDialog::ScatterGDXDialog(
        ScatterGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    //pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    QGridLayout* m_mainLayout = new QGridLayout(this);
    m_mainLayout->setContentsMargins(6, 6, 6, 6);
    m_mainLayout->setSpacing(6);

    Knob* pwrKNB = new Knob(this);
    //pwrKNB->move(10, 10);
    pwrKNB->setModel(&controls->m_pwrModel);
    pwrKNB->setText(tr("POWER"));
    pwrKNB->setHintText(tr("Power:"), "/bar");

    Knob* spdKNB = new Knob(this);
    //spdKNB->move(80, 10);
    spdKNB->setModel(&controls->m_spdModel);
    spdKNB->setText(tr("SPEED"));
    spdKNB->setHintText(tr("Speed:"), "");

    Knob* frcKNB = new Knob(this);
    //frcKNB->move(10, 60);
    frcKNB->setModel(&controls->m_frcModel);
    frcKNB->setText(tr("FRCTN"));
    frcKNB->setHintText(tr("Fraction:"), "");

    Knob* ovrKNB = new Knob(this);
    //ovrKNB->move(80, 60);
    ovrKNB->setModel(&controls->m_ovrModel);
    ovrKNB->setText(tr("OVRRD"));
    ovrKNB->setHintText(tr("Override:"), "");

    Knob* strKNB = new Knob(this);
    //strKNB->move(80, 120);
    strKNB->setModel(&controls->m_strModel);
    strKNB->setText(tr("START"));
    strKNB->setHintText(tr("Start:"), "");

    m_mainLayout->addWidget(pwrKNB, 0, 0, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(spdKNB, 0, 1, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(frcKNB, 0, 2, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(ovrKNB, 0, 3, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(strKNB, 0, 4, Qt::AlignHCenter | Qt::AlignTop);

    m_mainLayout->setColumnStretch(5, 1);
    m_mainLayout->setRowStretch(1, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
