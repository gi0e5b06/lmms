/*
 * ShifterGDXDialog.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include "ShifterGDXDialog.h"

#include "ComboBox.h"
#include "Knob.h"
#include "ShifterGDXControls.h"
#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

ShifterGDXDialog::ShifterGDXDialog(ShifterGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_alpha_bg"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setHorizontalSpacing(6);
    mainLayout->setVerticalSpacing(6);

    Knob* deltaFrequencyKNB = new Knob(this);
    deltaFrequencyKNB->setModel(&controls->m_deltaFrequencyModel);
    deltaFrequencyKNB->setText("DF");
    deltaFrequencyKNB->setPointColor(Qt::green);

    Knob* lowFrequencyKNB = new Knob(this);
    lowFrequencyKNB->setModel(&controls->m_lowFrequencyModel);
    lowFrequencyKNB->setText("LF");
    lowFrequencyKNB->setPointColor(Qt::green);

    Knob* highFrequencyKNB = new Knob(this);
    highFrequencyKNB->setModel(&controls->m_highFrequencyModel);
    highFrequencyKNB->setText("HF");
    highFrequencyKNB->setPointColor(Qt::green);

    Knob* factorFrequencyKNB = new Knob(this);
    factorFrequencyKNB->setModel(&controls->m_factorFrequencyModel);
    factorFrequencyKNB->setText("XF");
    factorFrequencyKNB->setPointColor(Qt::green);

    Knob* deltaVolumeKNB = new Knob(this);
    deltaVolumeKNB->setModel(&controls->m_deltaVolumeModel);
    deltaVolumeKNB->setText("DV");
    deltaVolumeKNB->setPointColor(Qt::red);

    Knob* lowVolumeKNB = new Knob(this);
    lowVolumeKNB->setModel(&controls->m_lowVolumeModel);
    lowVolumeKNB->setText("LV");
    lowVolumeKNB->setPointColor(Qt::red);

    Knob* highVolumeKNB = new Knob(this);
    highVolumeKNB->setModel(&controls->m_highVolumeModel);
    highVolumeKNB->setText("HV");
    highVolumeKNB->setPointColor(Qt::red);

    mainLayout->addWidget(deltaFrequencyKNB, 0, 0,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(lowFrequencyKNB, 0, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(highFrequencyKNB, 0, 2,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(factorFrequencyKNB, 0, 3,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(deltaVolumeKNB, 1, 0,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(lowVolumeKNB, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(highVolumeKNB, 1, 2,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLayout->setColumnStretch(4, 1);
    mainLayout->setRowStretch(2, 1);

    setFixedWidth(250);
    // setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}

ShifterGDXDialog::~ShifterGDXDialog()
{
}
