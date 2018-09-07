/*
 * WallGDXDialog.cpp - control dialog for wall effect
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

#include "WallGDXDialog.h"

#include <QGridLayout>

#include "WallGDXControls.h"
#include "embed.h"

WallGDXDialog::WallGDXDialog(WallGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    setPalette(pal);

    QGridLayout* mainLOT = new QGridLayout(this);
    mainLOT->setContentsMargins(6, 6, 6, 6);
    mainLOT->setSpacing(6);

    Knob* distanceKNB = new Knob(this);
    distanceKNB->setModel(&controls->m_distanceModel);
    distanceKNB->setText(tr("DIST"));
    distanceKNB->setHintText(tr("Distance:"), "");

    Knob* dryKNB = new Knob(this);
    dryKNB->setModel(&controls->m_dryModel);
    dryKNB->setText(tr("DRY"));
    dryKNB->setHintText(tr("Dry:"), "");

    Knob* wetKNB = new Knob(this);
    wetKNB->setModel(&controls->m_wetModel);
    wetKNB->setText(tr("WET"));
    wetKNB->setHintText(tr("Wet:"), "");

    int col = 0, row = 0;  // first row
    mainLOT->addWidget(distanceKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(dryKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(wetKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLOT->setColumnStretch(4, 1);
    mainLOT->setRowStretch(1, 1);

    setFixedWidth(250);
    //setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
