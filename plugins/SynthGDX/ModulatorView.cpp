/*
 * ModulatorView.cpp - SynthGDX 8 oscillators
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

#include "ModulatorView.h"

#include <QGridLayout>
#include <QLabel>

#include "ComboBox.h"
#include "Knob.h"
#include "LedCheckBox.h"
#include "SynthGDX.h"

ModulatorView::ModulatorView(ModulatorObject* _mod,
                             const int        _idx,
                             QWidget*         _parent) :
      QWidget(_parent),
      m_mod(_mod)
{
    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);  // 6, 6, 6, 6);
    mainLayout->setColumnStretch(2, 1);
    // mainLayout->setRowStretch(1, 1);
    mainLayout->setHorizontalSpacing(2);  // 6);
    mainLayout->setVerticalSpacing(2);

    // QLabel* numLBL = new QLabel(QString("M%1").arg(_idx + 1), this);
    // numLBL->setAutoFillBackground(true);
    // QPalette pal(numLBL->palette());
    // pal.setBrush(backgroundRole(), QColor(96,96,128));
    // numLBL->setPalette(pal);

    LedCheckBox* enabledLCB = new LedCheckBox(this,LedCheckBox::Green);
    enabledLCB->setModel(&_mod->m_enabledModel);
    enabledLCB->setText(QString("M%1").arg(_idx + 1));
    enabledLCB->setTextAnchorPoint(Qt::AnchorBottom);
    /*
    QPalette pal=enabledLCB->palette();
    pal.setColor(QPalette::Background, QColor(128, 96, 96));
    enabledLCB->setAutoFillBackground(true);
    enabledLCB->setPalette(pal);
    */
    enabledLCB->setStyleSheet("background-color:#606080;");
    enabledLCB->setMinimumSize(10, 20);

    ComboBox* algoCBX = new ComboBox(this);
    algoCBX->setModel(&_mod->m_algoModel);
    algoCBX->setMinimumWidth(4*27+6+6);

    ComboBox* modulatedCBX = new ComboBox(this);
    modulatedCBX->setModel(&_mod->m_modulatedModel);
    modulatedCBX->setMinimumWidth(39);

    ComboBox* modulatorCBX = new ComboBox(this);
    modulatorCBX->setModel(&_mod->m_modulatorModel);
    modulatorCBX->setMinimumWidth(39);

    // mainLayout->addWidget(numLBL, 0, 0, 1, 1);
    mainLayout->addWidget(enabledLCB, 0, 0, 1, 1,
                          Qt::AlignTop | Qt::AlignHCenter);
    int row = 0;  // first row
    mainLayout->addWidget(modulatedCBX, row, 1, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(algoCBX, row, 2, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(modulatorCBX, row, 3, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);

    // setLayout(mainLayout);
    setFixedWidth(234);
}

ModulatorView::~ModulatorView()
{
}
