/*
 * CompressorGDXDialog.cpp - control dialog for click remover effect
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

#include "CompressorGDXDialog.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>

#include "CompressorGDXControls.h"
#include "embed.h"

CompressorGDXDialog::CompressorGDXDialog(
        CompressorGDXControls* controls) :
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

    Knob* thresholdKnob = new Knob(knobBright_26, ctlGB);
    thresholdKnob->move(17, 35);
    thresholdKnob->setModel(&controls->m_thresholdModel);
    thresholdKnob->setLabel(tr("Treshold"));
    thresholdKnob->setHintText(tr("Treshold:"), "");

    Knob* ratioKnob = new Knob(knobBright_26, ctlGB);
    ratioKnob->move(67, 35);
    ratioKnob->setModel(&controls->m_ratioModel);
    ratioKnob->setLabel(tr("Ratio"));
    ratioKnob->setHintText(tr("Ratio:"), "");

    Knob* outGainKnob = new Knob(knobBright_26, ctlGB);
    outGainKnob->move(17, 85);
    outGainKnob->setModel(&controls->m_outGainModel);
    outGainKnob->setLabel(tr("Out"));
    outGainKnob->setHintText(tr("Out gain:"), "");

    /*
    Knob * modeKnob = new Knob( knobBright_26, ctlGB);
    modeKnob -> move( 67, 85 );
    modeKnob->setModel( &controls->m_modeModel );
    modeKnob->setLabel( tr( "Delay" ) );
    modeKnob->setHintText( tr( "Delay:" ) , "" );
    */
}
