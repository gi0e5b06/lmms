/*
 * AmplifierGDXDialog.cpp -
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

    Knob* volumeKNB = new Knob(this);
    volumeKNB->setVolumeKnob(true);
    volumeKNB->setModel(&controls->m_volumeModel);
    volumeKNB->setText(tr("VOL"));
    volumeKNB->setHintText(tr("Volume:"), "%");

    Knob* balanceKNB = new Knob(this);
    balanceKNB->setModel(&controls->m_balanceModel);
    balanceKNB->setPointColor(Qt::magenta);
    balanceKNB->setText(tr("BAL"));
    balanceKNB->setHintText(tr("Balance:"), "");

    Knob* leftVolumeKNB = new Knob(this);
    leftVolumeKNB->setVolumeKnob(true);
    leftVolumeKNB->setModel(&controls->m_leftVolumeModel);
    leftVolumeKNB->setText(tr("LEFT"));
    leftVolumeKNB->setHintText(tr("Left volume:"), "%");

    Knob* rightVolumeKNB = new Knob(this);
    rightVolumeKNB->setVolumeKnob(true);
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

    m_mainLayout->setColumnStretch(5, 1);
    m_mainLayout->setRowStretch(3, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
