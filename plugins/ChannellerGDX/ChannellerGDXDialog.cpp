/*
 * ChannellerGDXDialog.cpp -
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

#include "ChannellerGDXDialog.h"

#include "ComboBox.h"
#include "ChannellerGDXControls.h"
#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

ChannellerGDXDialog::ChannellerGDXDialog(ChannellerGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setHorizontalSpacing(6);
    mainLayout->setVerticalSpacing(6);

    ComboBox* operationCMB=new ComboBox(this);
    operationCMB->setModel(&controls->m_operationModel);

    mainLayout->addWidget(operationCMB, 0, 0,
                          Qt::AlignBottom);

    mainLayout->setColumnStretch(0, 1);
    mainLayout->setRowStretch(1, 1);

    setFixedWidth(250);
    // setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}

ChannellerGDXDialog::~ChannellerGDXDialog()
{
}
