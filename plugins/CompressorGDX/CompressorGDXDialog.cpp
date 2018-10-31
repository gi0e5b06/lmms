/*
 * CompressorGDXDialog.cpp -
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

#include "CompressorGDXDialog.h"

#include "CompressorGDXControls.h"
#include "GroupBox.h"
#include "embed.h"

#include <QGridLayout>
#include <QLayout>

CompressorGDXDialog::CompressorGDXDialog(CompressorGDXControls* controls) :
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

    Knob* inGainKNB = new Knob(this);
    // outGainKNB->move(17, 85);
    inGainKNB->setModel(&controls->m_inGainModel);
    inGainKNB->setLabel(tr("IN"));
    inGainKNB->setHintText(tr("In gain:"), "");

    Knob* thresholdKNB = new Knob(this);
    // thresholdKNB->move(17, 35);
    thresholdKNB->setModel(&controls->m_thresholdModel);
    thresholdKNB->setLabel(tr("TRHLD"));
    thresholdKNB->setHintText(tr("Treshold:"), "");

    Knob* ratioKNB = new Knob(this);
    // ratioKNB->move(67, 35);
    ratioKNB->setModel(&controls->m_ratioModel);
    ratioKNB->setLabel(tr("RATIO"));
    ratioKNB->setHintText(tr("Ratio:"), "");

    Knob* modeKNB = new Knob(this);
    // modeKNB->move(67, 85);
    modeKNB->setModel(&controls->m_modeModel);
    modeKNB->setLabel(tr("MODE"));
    modeKNB->setHintText(tr("Mode:"), "");

    Knob* outGainKNB = new Knob(this);
    // outGainKNB->move(17, 85);
    outGainKNB->setModel(&controls->m_outGainModel);
    outGainKNB->setLabel(tr("OUT"));
    outGainKNB->setHintText(tr("Out gain:"), "");

    m_mainLayout->addWidget(inGainKNB, 0, 0,
                            Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(thresholdKNB, 0, 1,
                            Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(ratioKNB, 0, 2, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(modeKNB, 0, 3, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(outGainKNB, 0, 4,
                            Qt::AlignHCenter | Qt::AlignTop);

    m_mainLayout->setColumnStretch(5, 1);
    m_mainLayout->setRowStretch(1, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
