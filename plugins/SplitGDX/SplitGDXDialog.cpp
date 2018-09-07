/*
 * SplitGDXDialog.cpp - control dialog for chaining effect
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

#include "SplitGDXDialog.h"

#include "EffectRackView.h"
#include "GroupBox.h"
#include "SplitGDX.h"
#include "SplitGDXControls.h"
#include "embed.h"

#include <QGridLayout>
//#include <QGroupBox>
#include <QHBoxLayout>
//#include <QLayout>

SplitGDXDialog::SplitGDXDialog(SplitGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    setPalette(pal);

    QVBoxLayout* mainLOT = new QVBoxLayout(this);
    mainLOT->setContentsMargins(0, 0, 0, 0);
    mainLOT->setSpacing(0);

    EffectRackView* splitRV = new EffectRackView(
            controls->m_effect->m_splitChain, this, tr("Split"));
    EffectRackView* wetRV = new EffectRackView(controls->m_effect->m_wetChain,
                                               this, tr("Wet"));
    EffectRackView* remRV = new EffectRackView(controls->m_effect->m_remChain,
                                               this, tr("Remainder"));

    // GroupBox* splitGB = new GroupBox(tr("Split"), this, false, true, true);
    // GroupBox* wetGB   = new GroupBox(tr("Wet"), this, false, true, true);
    // GroupBox* remGB   = new GroupBox(tr("Remainder"), this, false, true,
    // true);
    GroupBox* mixGB = new GroupBox(tr("Mixing"), this, false, false);

    // splitGB->setContentsMargins(5,5,5,5);

    QWidget*     mixPNL = new QWidget(mixGB);
    QGridLayout* mixLOT = new QGridLayout(mixPNL);
    mixLOT->setContentsMargins(3, 3, 3, 3);
    mixLOT->setSpacing(3);
    mixPNL->setLayout(mixLOT);
    mixGB->setContentWidget(mixPNL);

    Knob* splitKNB = new Knob(knobBright_26, mixPNL);
    // splitKNB->move(7, 30);
    splitKNB->setModel(&controls->m_splitModel);
    splitKNB->setLabel(tr("SUBDRY"));
    splitKNB->setHintText(tr("Sub dry:"), "");

    Knob* wetKNB = new Knob(knobBright_26, mixPNL);
    // wetKNB->move(47, 30);
    wetKNB->setModel(&controls->m_wetModel);
    wetKNB->setLabel(tr("SUBWET"));
    wetKNB->setHintText(tr("Sub wet:"), "");

    Knob* remKNB = new Knob(knobBright_26, mixPNL);
    // remKNB->move(87, 30);
    remKNB->setModel(&controls->m_remModel);
    remKNB->setLabel(tr("REMAIND"));
    remKNB->setHintText(tr("Remainder:"), "");

    mixLOT->addWidget(splitKNB, 0, 0, Qt::AlignHCenter | Qt::AlignTop);
    mixLOT->addWidget(wetKNB, 0, 1, Qt::AlignHCenter | Qt::AlignTop);
    mixLOT->addWidget(remKNB, 0, 2, Qt::AlignHCenter | Qt::AlignTop);

    mixLOT->setColumnStretch(3, 1);

    mainLOT->addWidget(splitRV);
    mainLOT->addWidget(wetRV);
    mainLOT->addWidget(remRV);
    mainLOT->addWidget(mixGB);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
