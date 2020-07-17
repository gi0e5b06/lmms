/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 * AmplifierGDXDialog.cpp -
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "AmplifierGDXDialog.h"

#include "AmplifierGDXControls.h"
#include "embed.h"

#include <QGridLayout>
#include <QLabel>

AmplifierGDXDialog::AmplifierGDXDialog(AmplifierGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getPixmap("plugin_bg"));
    // PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    QGridLayout* m_mainLayout = new QGridLayout(this);
    m_mainLayout->setContentsMargins(6, 6, 6, 6);
    m_mainLayout->setSpacing(12);

    VolumeKnob* volumeKNB = new VolumeKnob(this);
    // volumeKNB->setVolumeKnob(true);
    volumeKNB->setModel(&controls->m_volumeModel);
    // volumeKNB->setText(tr("VOL"));
    // volumeKNB->setHintText(tr("Volume:"), "%");

    Knob* balanceKNB = new Knob(this);
    balanceKNB->setModel(&controls->m_balanceModel);
    balanceKNB->setPointColor(Qt::magenta);
    balanceKNB->setText(tr("BAL"));
    balanceKNB->setHintText(tr("Balance:"), "");

    VolumeKnob* leftVolumeKNB = new VolumeKnob(this);
    // leftVolumeKNB->setVolumeKnob(true);
    leftVolumeKNB->setModel(&controls->m_leftVolumeModel);
    leftVolumeKNB->setText(tr("LEFT"));
    leftVolumeKNB->setHintText(tr("Left volume:"), "%");

    VolumeKnob* rightVolumeKNB = new VolumeKnob(this);
    // rightVolumeKNB->setVolumeKnob(true);
    rightVolumeKNB->setModel(&controls->m_rightVolumeModel);
    rightVolumeKNB->setText(tr("RIGHT"));
    rightVolumeKNB->setHintText(tr("Right volume:"), "%");

    Knob* widthKNB = new Knob(this);
    widthKNB->setModel(&controls->m_widthModel);
    widthKNB->setText(tr("WIDTH"));
    widthKNB->setHintText(tr("Width:"), "%");

    Knob* leftPanningKNB = new Knob(this);
    leftPanningKNB->setPointColor(Qt::magenta);
    leftPanningKNB->setModel(&controls->m_leftPanningModel);
    leftPanningKNB->setText(tr("LEFT"));
    leftPanningKNB->setHintText(tr("Left panning:"), "");

    Knob* rightPanningKNB = new Knob(this);
    rightPanningKNB->setPointColor(Qt::magenta);
    rightPanningKNB->setModel(&controls->m_rightPanningModel);
    rightPanningKNB->setText(tr("RIGHT"));
    rightPanningKNB->setHintText(tr("Right panning:"), "");

    Knob* responseKNB = new Knob(this);
    responseKNB->setPointColor(Qt::yellow);
    responseKNB->setModel(&controls->m_responseModel);
    responseKNB->setText(tr("RESP"));
    responseKNB->setHintText(tr("Response:"), "");

    m_mainLayout->addWidget(new QLabel("VOL"), 0, 2,
                            Qt::AlignHCenter | Qt::AlignVCenter);
    m_mainLayout->addWidget(new QLabel("PAN"), 0, 4,
                            Qt::AlignHCenter | Qt::AlignVCenter);

    m_mainLayout->addWidget(volumeKNB, 1, 0, 2, 1,
                            Qt::AlignHCenter | Qt::AlignVCenter);
    m_mainLayout->addWidget(balanceKNB, 1, 1, 2, 1,
                            Qt::AlignHCenter | Qt::AlignVCenter);
    m_mainLayout->addWidget(leftVolumeKNB, 1, 2, 1, 1,
                            Qt::AlignHCenter | Qt::AlignVCenter);
    m_mainLayout->addWidget(rightVolumeKNB, 2, 2, 1, 1,
                            Qt::AlignHCenter | Qt::AlignVCenter);
    m_mainLayout->addWidget(widthKNB, 1, 3, 2, 1,
                            Qt::AlignHCenter | Qt::AlignVCenter);
    m_mainLayout->addWidget(leftPanningKNB, 1, 4, 1, 1,
                            Qt::AlignHCenter | Qt::AlignVCenter);
    m_mainLayout->addWidget(rightPanningKNB, 2, 4, 1, 1,
                            Qt::AlignHCenter | Qt::AlignVCenter);
    m_mainLayout->addWidget(responseKNB, 1, 5, 2, 1,
                            Qt::AlignHCenter | Qt::AlignVCenter);

    m_mainLayout->setColumnStretch(6, 1);
    m_mainLayout->setRowStretch(3, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
