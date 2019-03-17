/*
 * DualFilterControlDialog.cpp - control dialog for dual filter effect
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "DualFilterControlDialog.h"

#include "ComboBox.h"
#include "DualFilterControls.h"
#include "LedCheckBox.h"
#include "ToolTip.h"
#include "embed.h"
#include "gui_templates.h"

#include <QLayout>

#define makeknob(name, x, y, model, label, hint, unit) \
    Knob* name = new Knob(knobBright_26, this);        \
    name->move(x, y);                                  \
    name->setModel(&controls->model);                  \
    name->setLabel(label);                             \
    name->setHintText(hint, unit)

DualFilterControlDialog::DualFilterControlDialog(
        DualFilterControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    // pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    setFixedWidth(250);
    setMinimumHeight(109);

    makeknob(cut1Knob, 6, 16, m_cut1Model, tr("FREQ"), tr("Cutoff frequency"),
             "Hz");
    makeknob(res1Knob, 36, 16, m_res1Model, tr("RESO"), tr("Resonance"), "");
    makeknob(gain1Knob, 66, 16, m_gain1Model, tr("VOL"), tr("Volume"), "%");
    makeknob(mixKnob, 125 - 13, 16, m_mixModel, tr("MIX"), tr("Mix"), "");
    makeknob(cut2Knob, 224 - 66, 16, m_cut2Model, tr("FREQ"),
             tr("Cutoff frequency"), "Hz");
    makeknob(res2Knob, 224 - 36, 16, m_res2Model, tr("RESO"), tr("Resonance"),
             "");
    makeknob(gain2Knob, 224 - 6, 16, m_gain2Model, tr("VOL"), tr("Volume"),
             "%");

    cut1Knob->setPointColor(Qt::green);
    cut2Knob->setPointColor(Qt::green);
    gain1Knob->setVolumeKnob(true);
    gain2Knob->setVolumeKnob(true);

    LedCheckBox* enabled1Toggle = new LedCheckBox(
            "", this, tr("Filter 1 enabled"), LedCheckBox::Green);
    LedCheckBox* enabled2Toggle = new LedCheckBox(
            "", this, tr("Filter 2 enabled"), LedCheckBox::Green);

    enabled1Toggle->move(6, 0);
    enabled1Toggle->setModel(&controls->m_enabled1Model);
    ToolTip::add(enabled1Toggle, tr("Click to enable/disable Filter 1"));
    enabled2Toggle->move(224 - 66, 0);
    enabled2Toggle->setModel(&controls->m_enabled2Model);
    ToolTip::add(enabled2Toggle, tr("Click to enable/disable Filter 2"));

    ComboBox* m_filter1ComboBox = new ComboBox(this);
    m_filter1ComboBox->setGeometry(6, 55, 151, 22);
    m_filter1ComboBox->setFont(pointSize<8>(m_filter1ComboBox->font()));
    m_filter1ComboBox->setModel(&controls->m_filter1Model);

    ComboBox* m_filter2ComboBox = new ComboBox(this);
    m_filter2ComboBox->setGeometry(93, 80, 151, 22);
    m_filter2ComboBox->setFont(pointSize<8>(m_filter2ComboBox->font()));
    m_filter2ComboBox->setModel(&controls->m_filter2Model);
}
