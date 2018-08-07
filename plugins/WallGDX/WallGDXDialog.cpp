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
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setHorizontalSpacing(6);
    mainLayout->setVerticalSpacing(6);

    Knob* distanceKNB = new Knob(this);
    distanceKNB->setModel(&controls->m_distanceModel);
    distanceKNB->setLabel(tr("DIST"));
    distanceKNB->setHintText(tr("Distance:"), "");

    Knob* dryKNB = new Knob(this);
    dryKNB->setModel(&controls->m_dryModel);
    dryKNB->setLabel(tr("DRY"));
    dryKNB->setHintText(tr("Dry:"), "");

    Knob* wetKNB = new Knob(this);
    wetKNB->setModel(&controls->m_wetModel);
    wetKNB->setLabel(tr("WET"));
    wetKNB->setHintText(tr("Wet:"), "");

    int col = 0, row = 0;  // first row
    mainLayout->addWidget(distanceKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(dryKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(wetKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    setFixedWidth(234);
}
