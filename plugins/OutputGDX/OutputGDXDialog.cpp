/*
 * OutputGDXDialog.cpp - control dialog for audio output properties
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
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

#include "OutputGDXDialog.h"

#include "OutputGDXControls.h"
#include "embed.h"

#include <QGridLayout>

OutputGDXDialog::OutputGDXDialog(OutputGDXControls* controls) :
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

    Knob* leftKNB = new Knob(this);
    leftKNB->setModel(&controls->m_leftModel);
    leftKNB->setPointColor(Qt::white);
    leftKNB->setInteractive(false);
    leftKNB->setText(tr("LEFT"));
    leftKNB->setHintText(tr("Left:"), "");

    Knob* rightKNB = new Knob(this);
    rightKNB->setModel(&controls->m_rightModel);
    rightKNB->setPointColor(Qt::white);
    rightKNB->setInteractive(false);
    rightKNB->setText(tr("RIGHT"));
    rightKNB->setHintText(tr("Right:"), "");

    Knob* rmsKNB = new Knob(this);
    rmsKNB->setModel(&controls->m_rmsModel);
    rmsKNB->setPointColor(Qt::red);
    rmsKNB->setInteractive(false);
    rmsKNB->setText(tr("RMS"));
    rmsKNB->setHintText(tr("Rms:"), "");

    Knob* volKNB = new Knob(this);
    volKNB->setModel(&controls->m_volModel);
    volKNB->setPointColor(Qt::red);
    volKNB->setInteractive(false);
    volKNB->setText(tr("VOL"));
    volKNB->setHintText(tr("Vol:"), "");

    Knob* panKNB = new Knob(this);
    panKNB->setModel(&controls->m_panModel);
    panKNB->setPointColor(Qt::magenta);
    panKNB->setInteractive(false);
    panKNB->setText(tr("PAN"));
    panKNB->setHintText(tr("Pan:"), "");

    int col = 0, row = 0;  // first row
    mainLayout->addWidget(leftKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(rightKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(rmsKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(volKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(panKNB, row, ++col, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLayout->setColumnStretch(6, 1);
    mainLayout->setRowStretch(1, 1);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
