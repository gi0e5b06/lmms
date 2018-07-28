/*
 * RandomGDXControlDialog.cpp - control dialog for randomizing effect
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

#include "RandomGDXControlDialog.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>

#include "RandomGDXControls.h"
#include "embed.h"

RandomGDXControlDialog::RandomGDXControlDialog(RandomGDXControls* controls) :
      EffectControlDialog(controls)
{
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    //pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);
    setFixedSize(150, 160);

    QGroupBox* ctlGB = new QGroupBox(tr("Controls"), this);
    ctlGB->setGeometry(10, 10, 130, 140);

    Knob* rndAmpKnob = new Knob(knobBright_26, ctlGB);
    rndAmpKnob->move(17, 35);
    rndAmpKnob->setModel(&controls->m_rndAmpModel);
    rndAmpKnob->setLabel(tr("Ramp"));
    rndAmpKnob->setHintText(tr("Random Amp:"), "");

    Knob* fixAmpKnob = new Knob(knobBright_26, ctlGB);
    fixAmpKnob->move(67, 35);
    fixAmpKnob->setModel(&controls->m_fixAmpModel);
    fixAmpKnob->setLabel(tr("Famp"));
    fixAmpKnob->setHintText(tr("Base Amp:"), "");

    Knob* sngPosKnob = new Knob(knobBright_26, ctlGB);
    sngPosKnob->move(17, 85);
    sngPosKnob->setModel(&controls->m_sngPosModel);
    sngPosKnob->setLabel(tr("Sing"));
    sngPosKnob->setHintText(tr("Singularity:"), "");

    Knob* delPosKnob = new Knob(knobBright_26, ctlGB);
    delPosKnob->move(67, 85);
    delPosKnob->setModel(&controls->m_delPosModel);
    delPosKnob->setLabel(tr("Delay"));
    delPosKnob->setHintText(tr("Delay:"), "");
}
