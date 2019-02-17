/*
 * SpaceGDXDialog.cpp - control dialog for wall effect
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

#include "SpaceGDXDialog.h"

#include <QGridLayout>

#include "SpaceGDXControls.h"
#include "embed.h"

SpaceGDXDialog::SpaceGDXDialog(SpaceGDXControls* controls) :
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

    Knob* rightPhaseKNB = new Knob(this);
    rightPhaseKNB->setModel(&controls->m_rightPhaseModel);
    rightPhaseKNB->setText(tr("PHASE"));
    rightPhaseKNB->setHintText(tr("Right phase:"), "");

    Knob* rightGainKNB = new Knob(this);
    rightGainKNB->setModel(&controls->m_rightGainModel);
    rightGainKNB->setText(tr("GAIN"));
    rightGainKNB->setHintText(tr("Right gain:"), "");

    Knob* rightLowKNB = new Knob(this);
    rightLowKNB->setModel(&controls->m_rightLowModel);
    rightLowKNB->setText(tr("LOW"));
    rightLowKNB->setHintText(tr("Right low pass:"), "");

    Knob* rightHighKNB = new Knob(this);
    rightHighKNB->setModel(&controls->m_rightHighModel);
    rightHighKNB->setText(tr("HIGH"));
    rightHighKNB->setHintText(tr("Right low pass:"), "");

    Knob* leftPhaseKNB = new Knob(this);
    leftPhaseKNB->setModel(&controls->m_leftPhaseModel);
    leftPhaseKNB->setText(tr("PHASE"));
    leftPhaseKNB->setHintText(tr("Left phase:"), "");

    Knob* leftGainKNB = new Knob(this);
    leftGainKNB->setModel(&controls->m_leftGainModel);
    leftGainKNB->setText(tr("GAIN"));
    leftGainKNB->setHintText(tr("Left gain:"), "");

    Knob* leftLowKNB = new Knob(this);
    leftLowKNB->setModel(&controls->m_leftLowModel);
    leftLowKNB->setText(tr("LOW"));
    leftLowKNB->setHintText(tr("Left low pass:"), "");

    Knob* leftHighKNB = new Knob(this);
    leftHighKNB->setModel(&controls->m_leftHighModel);
    leftHighKNB->setText(tr("HIGH"));
    leftHighKNB->setHintText(tr("Left low pass:"), "");

    Knob* dispersionKNB = new Knob(this);
    dispersionKNB->setModel(&controls->m_dispersionModel);
    dispersionKNB->setText(tr("DISP"));
    dispersionKNB->setHintText(tr("Dispersion:"), "");

    Knob* amountKNB = new Knob(this);
    amountKNB->setModel(&controls->m_amountModel);
    amountKNB->setText(tr("AMNT"));
    amountKNB->setHintText(tr("Amount:"), "");

    int col = 0, row = 0;  // first row
    mainLOT->addWidget(leftPhaseKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(leftGainKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(leftLowKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(leftHighKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    col=0; row++;
    mainLOT->addWidget(rightPhaseKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(rightGainKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(rightLowKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(rightHighKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    col=0; row++;
    mainLOT->addWidget(dispersionKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(amountKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLOT->setColumnStretch(5, 1);
    mainLOT->setRowStretch(3, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
