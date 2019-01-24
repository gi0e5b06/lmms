/*
 * NormalizeGDXDialog.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include "NormalizeGDXDialog.h"

#include "NormalizeGDXControls.h"
#include "Knob.h"
#include "LedCheckBox.h"
#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>

NormalizeGDXDialog::NormalizeGDXDialog(NormalizeGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    // pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    LedCheckBox* skewLCB = new LedCheckBox(this);
    skewLCB->setModel(&controls->m_skewEnabledModel);
    //skewLCB->setText(tr("SKEW"));
    //skewLCB->setHintText(tr("Skew:"), "");

    LedCheckBox* volumeLCB = new LedCheckBox(this);
    volumeLCB->setModel(&controls->m_volumeEnabledModel);
    //volumeLCB->setText(tr("VOLUME"));
    //volumeLCB->setHintText(tr("Volume:"), "");

    LedCheckBox* balanceLCB = new LedCheckBox(this);
    balanceLCB->setModel(&controls->m_balanceEnabledModel);
    //balanceLCB->setText(tr("BALANCE"));
    //balanceLCB->setHintText(tr("Balance:"), "");

    Knob* skewKNB = new Knob(this);
    skewKNB->setModel(&controls->m_skewSpeedModel);
    skewKNB->setText(tr("SKEW"));
    skewKNB->setHintText(tr("Skew:"), "");

    Knob* volumeUpKNB = new Knob(this);
    volumeUpKNB->setModel(&controls->m_volumeUpSpeedModel);
    volumeUpKNB->setText(tr("VOL UP"));
    volumeUpKNB->setHintText(tr("Volume up:"), "");

    Knob* volumeDownKNB = new Knob(this);
    volumeDownKNB->setModel(&controls->m_volumeDownSpeedModel);
    volumeDownKNB->setText(tr("DOWN"));
    volumeDownKNB->setHintText(tr("Volume down:"), "");

    Knob* balanceKNB = new Knob(this);
    balanceKNB->setModel(&controls->m_balanceSpeedModel);
    balanceKNB->setText(tr("BAL"));
    balanceKNB->setHintText(tr("Balance:"), "");

    Knob* outGainKNB = new Knob(this);
    outGainKNB->setModel(&controls->m_outGainModel);
    outGainKNB->setText(tr("OUT"));
    outGainKNB->setHintText(tr("Out gain:"), "");

    mainLayout->addWidget(skewLCB, 0, 1, 1, 1,
                          Qt::AlignBottom | Qt::AlignRight);
    mainLayout->addWidget(volumeLCB, 0, 2, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(balanceLCB, 0, 3, 1, 1,
                          Qt::AlignBottom | Qt::AlignLeft);

    mainLayout->addWidget(skewKNB, 1, 1, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(volumeUpKNB, 1, 2, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(balanceKNB, 1, 3, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLayout->addWidget(volumeDownKNB, 2, 2, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLayout->addWidget(outGainKNB, 3, 2, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLayout->setColumnStretch(4, 1);
    mainLayout->setRowStretch(4, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}

NormalizeGDXDialog::~NormalizeGDXDialog()
{
}
