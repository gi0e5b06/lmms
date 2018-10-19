/*
 * InputGDXDialog.cpp -
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

#include "InputGDXDialog.h"

#include "InputGDXControls.h"
#include "embed.h"

#include <QGridLayout>

InputGDXDialog::InputGDXDialog(InputGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    Knob* leftSignalKNB = new Knob(this);
    leftSignalKNB->setModel(&controls->m_leftSignalModel);
    leftSignalKNB->setPointColor(Qt::white);
    leftSignalKNB->setText(tr("LEFT"));
    leftSignalKNB->setHintText(tr("Left signal:"), "");

    Knob* rightSignalKNB = new Knob(this);
    rightSignalKNB->setModel(&controls->m_rightSignalModel);
    rightSignalKNB->setPointColor(Qt::white);
    rightSignalKNB->setText(tr("RIGHT"));
    rightSignalKNB->setHintText(tr("Right signal:"), "");

    Knob* volumeKNB = new Knob(this);
    volumeKNB->setModel(&controls->m_volumeModel);
    volumeKNB->setPointColor(Qt::red);
    volumeKNB->setText(tr("VOL"));
    volumeKNB->setHintText(tr("Vol:"), "");

    Knob* balanceKNB = new Knob(this);
    balanceKNB->setModel(&controls->m_balanceModel);
    balanceKNB->setPointColor(Qt::magenta);
    balanceKNB->setText(tr("BAL"));
    balanceKNB->setHintText(tr("Balance:"), "");

    Knob* mixingKNB = new Knob(this);
    mixingKNB->setModel(&controls->m_mixingModel);
    mixingKNB->setPointColor(Qt::red);
    mixingKNB->setText(tr("MIX"));
    mixingKNB->setHintText(tr("Mixing:"), "");

    Knob* deltaKNB = new Knob(this);
    deltaKNB->setModel(&controls->m_deltaModel);
    // deltaKNB->setPointColor(Qt::red);
    deltaKNB->setText(tr("\u0394"));
    deltaKNB->setHintText(tr("Delta:"), "");

    int col = 0, row = 0;  // first row
    mainLayout->addWidget(leftSignalKNB, row, col++, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(rightSignalKNB, row, col++, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(volumeKNB, row, col++, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(balanceKNB, row, col++, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    row++;
    col = 0;
    mainLayout->addWidget(mixingKNB, row, col = 0, 1, 2,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(deltaKNB, row, col = 2, 1, 2,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLayout->setColumnStretch(4, 1);
    mainLayout->setRowStretch(2, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
