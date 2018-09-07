/*
 * SplitGDXControlDialog.cpp - control dialog for chaining effect
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

#include "SplitGDXControlDialog.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLayout>

#include "EffectRackView.h"
#include "SplitGDX.h"
#include "SplitGDXControls.h"
#include "embed.h"

SplitGDXControlDialog::SplitGDXControlDialog(SplitGDXControls* controls) :
      EffectControlDialog(controls)
{
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    //pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);
    setContentsMargins(0, 0, 0, 0);
    setMinimumWidth(265);
    setMinimumHeight(632);

    QVBoxLayout* vl = new QVBoxLayout(this);

    QGroupBox* splitGB = new QGroupBox(tr("Split"), this);
    QGroupBox* wetGB   = new QGroupBox(tr("Wet"), this);
    QGroupBox* remGB   = new QGroupBox(tr("Remainder"), this);
    QGroupBox* mixGB   = new QGroupBox(tr("Mixing"), this);

    EffectRackView* splitRV
            = new EffectRackView(controls->m_effect->m_splitChain, splitGB);
    EffectRackView* wetRV
            = new EffectRackView(controls->m_effect->m_wetChain, wetGB);
    EffectRackView* remRV
            = new EffectRackView(controls->m_effect->m_remChain, remGB);

    splitRV->setFixedSize(245, 152);
    wetRV->setFixedSize(245, 152);
    remRV->setFixedSize(245, 152);

    splitRV->move(1,21);
    wetRV->move(1,21);
    remRV->move(1,21);

    splitGB->setContentsMargins(5,5,5,5);

    Knob* splitKnob = new Knob(knobBright_26, mixGB);
    splitKnob->move(7, 30);
    splitKnob->setModel(&controls->m_splitModel);
    splitKnob->setLabel(tr("SUBDRY"));
    splitKnob->setHintText(tr("Sub dry:"), "");

    Knob* wetKnob = new Knob(knobBright_26, mixGB);
    wetKnob->move(47, 30);
    wetKnob->setModel(&controls->m_wetModel);
    wetKnob->setLabel(tr("SUBWET"));
    wetKnob->setHintText(tr("Sub wet:"), "");

    Knob* remKnob = new Knob(knobBright_26, mixGB);
    remKnob->move(87, 30);
    remKnob->setModel(&controls->m_remModel);
    remKnob->setLabel(tr("REMAIND"));
    remKnob->setHintText(tr("Remainder:"), "");

    mixGB->setFixedHeight(75);

    vl->addWidget(splitGB);
    vl->addWidget(wetGB);
    vl->addWidget(remGB);
    vl->addWidget(mixGB);
}
