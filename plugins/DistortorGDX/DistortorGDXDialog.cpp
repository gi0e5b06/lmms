/*
 * DistortorGDXDialog.cpp -
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

#include "DistortorGDXDialog.h"

#include "DistortorGDXControls.h"
#include "GroupBox.h"
#include "embed.h"

#include <QGridLayout>
#include <QLayout>

DistortorGDXDialog::DistortorGDXDialog(DistortorGDXControls* controls) :
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

    Knob* simpleKNB = new Knob(this);
    // outGainKNB->move(17, 85);
    simpleKNB->setModel(&controls->m_simpleModel);
    simpleKNB->setLabel(tr("Simple"));
    simpleKNB->setHintText(tr("Simple:"), "");

    Knob* foldoverKNB = new Knob(this);
    // foldoverKNB->move(17, 35);
    foldoverKNB->setModel(&controls->m_foldoverModel);
    foldoverKNB->setLabel(tr("Foldover"));
    foldoverKNB->setHintText(tr("Treshold:"), "");

    Knob* modulatorKNB = new Knob(this);
    // modulatorKNB->move(67, 35);
    modulatorKNB->setModel(&controls->m_modulatorModel);
    modulatorKNB->setLabel(tr("Modulator"));
    modulatorKNB->setHintText(tr("Modulator:"), "");

    Knob* crossoverKNB = new Knob(this);
    // crossoverKNB->move(67, 85);
    crossoverKNB->setModel(&controls->m_crossoverModel);
    crossoverKNB->setLabel(tr("Crossover"));
    crossoverKNB->setHintText(tr("Crossover:"), "");

    Knob* outGainKNB = new Knob(this);
    // outGainKNB->move(17, 85);
    outGainKNB->setModel(&controls->m_outGainModel);
    outGainKNB->setLabel(tr("Out"));
    outGainKNB->setHintText(tr("Out gain:"), "");

    m_mainLayout->addWidget(simpleKNB, 0, 0, Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(foldoverKNB, 0, 1,
                            Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(modulatorKNB, 0, 2,
                            Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(crossoverKNB, 0, 3,
                            Qt::AlignHCenter | Qt::AlignTop);
    m_mainLayout->addWidget(outGainKNB, 0, 4,
                            Qt::AlignHCenter | Qt::AlignTop);

    m_mainLayout->setColumnStretch(5, 1);
    m_mainLayout->setRowStretch(1, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
