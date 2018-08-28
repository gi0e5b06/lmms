/*
 * ScarifierGDXDialog.cpp -
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

#include "ScarifierGDXDialog.h"

#include "ComboBox.h"
#include "Knob.h"
#include "ScarifierGDXControls.h"
#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

ScarifierGDXDialog::ScarifierGDXDialog(ScarifierGDXControls* controls) :
      EffectControlDialog(controls)
{
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    // pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setColumnStretch(3, 1);
    mainLayout->setHorizontalSpacing(6);
    mainLayout->setVerticalSpacing(6);

    ComboBox* modeCBX = new ComboBox(this);
    modeCBX->setModel(&controls->m_modeModel);

    Knob* widthKNB = new Knob(this);
    widthKNB->setModel(&controls->m_widthModel);
    widthKNB->setText("WIDTH");
    widthKNB->setPointColor(Qt::white);

    Knob* keyKNB = new Knob(this);
    keyKNB->setModel(&controls->m_keyModel);
    keyKNB->setText("KEY");
    keyKNB->setPointColor(Qt::darkCyan);

    Knob* ampKNB = new Knob(this);
    ampKNB->setModel(&controls->m_ampModel);
    ampKNB->setText("GAIN");
    ampKNB->setPointColor(Qt::red);

    mainLayout->addWidget(modeCBX, 0, 0, 1, 3,
                          Qt::AlignBottom);// | Qt::AlignHCenter);
    mainLayout->addWidget(widthKNB, 1, 0, Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(keyKNB, 1, 1, Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(ampKNB, 1, 2, Qt::AlignBottom | Qt::AlignHCenter);

    setFixedWidth(234);
}

ScarifierGDXDialog::~ScarifierGDXDialog()
{
}
