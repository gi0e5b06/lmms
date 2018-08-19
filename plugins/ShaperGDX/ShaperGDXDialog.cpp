/*
 * ShaperGDXDialog.cpp -
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

#include "ShaperGDXDialog.h"

#include "ShaperGDXControls.h"
#include "ComboBox.h"
#include "Knob.h"

#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>

ShaperGDXDialog::ShaperGDXDialog(
        ShaperGDXControls* controls) :
      EffectControlDialog(controls)
{
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    //pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);
    setFixedSize(234, 220);

    QGroupBox* ctlGB = new QGroupBox(tr("Controls"), this);
    ctlGB->setGeometry(10, 10, 214, 200);

    ComboBox* bankCMB = new ComboBox( ctlGB);
    bankCMB->move(17, 35);
    bankCMB->setModel(&controls->m_waveBankModel);
    //bankCMB->setLabel(tr("Treshold"));
    //bankCMB->setHintText(tr("Treshold:"), "");
    bankCMB->setMinimumWidth(3 * 27 + 4);

    ComboBox* indexCMB = new ComboBox( ctlGB);
    indexCMB->move(17, 85);
    indexCMB->setModel(&controls->m_waveIndexModel);
    //indexCMB->setLabel(tr("Treshold"));
    //indexCMB->setHintText(tr("Treshold:"), "");
    indexCMB->setMinimumWidth(6 * 27 + 12);

    Knob* timeKNB = new Knob(knobBright_26, ctlGB);
    timeKNB->move(17, 135);
    timeKNB->setModel(&controls->m_timeModel);
    timeKNB->setLabel(tr("TIME"));
    timeKNB->setHintText(tr("Time:"), "");

    Knob* ratioKNB = new Knob(knobBright_26, ctlGB);
    ratioKNB->move(67, 135);
    ratioKNB->setModel(&controls->m_ratioModel);
    ratioKNB->setLabel(tr("RATIO"));
    ratioKNB->setHintText(tr("Ratio:"), "");

    Knob* outGainKNB = new Knob(knobBright_26, ctlGB);
    outGainKNB->move(117, 135);
    outGainKNB->setModel(&controls->m_outGainModel);
    outGainKNB->setLabel(tr("OUT"));
    outGainKNB->setHintText(tr("Out gain:"), "");

    Knob * modeKNB = new Knob( knobBright_26, ctlGB);
    modeKNB -> move( 167, 135 );
    modeKNB->setModel( &controls->m_modeModel );
    modeKNB->setLabel( tr( "MODE" ) );
    modeKNB->setHintText( tr( "Mode:" ) , "" );
}
