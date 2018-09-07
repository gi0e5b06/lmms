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

#include "ComboBox.h"
#include "Knob.h"
#include "ShaperGDXControls.h"
#include "VisualizationWidget.h"
#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>

ShaperGDXDialog::ShaperGDXDialog(ShaperGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    // pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    ComboBox* bankCMB = new ComboBox(this);
    // bankCMB->move(17, 35);
    bankCMB->setModel(&controls->m_waveBankModel);
    // bankCMB->setText(tr("Treshold"));
    // bankCMB->setHintText(tr("Treshold:"), "");
    //bankCMB->setMinimumWidth(3 * 26 + 2*6);

    ComboBox* indexCMB = new ComboBox(this);
    // indexCMB->move(17, 85);
    indexCMB->setModel(&controls->m_waveIndexModel);
    // indexCMB->setText(tr("Treshold"));
    // indexCMB->setHintText(tr("Treshold:"), "");
    //indexCMB->setMinimumWidth(6 * 26 + 5*6);

    Knob* timeKNB = new Knob(this);
    // timeKNB->move(17, 135);
    timeKNB->setModel(&controls->m_timeModel);
    timeKNB->setText(tr("TIME"));
    timeKNB->setHintText(tr("Time:"), "");

    Knob* ratioKNB = new Knob(this);
    // ratioKNB->move(67, 135);
    ratioKNB->setModel(&controls->m_ratioModel);
    ratioKNB->setText(tr("RATIO"));
    ratioKNB->setHintText(tr("Ratio:"), "");

    Knob* outGainKNB = new Knob(this);
    // outGainKNB->move(117, 135);
    outGainKNB->setModel(&controls->m_outGainModel);
    outGainKNB->setText(tr("OUT"));
    outGainKNB->setHintText(tr("Out gain:"), "");

    Knob* modeKNB = new Knob(this);
    // modeKNB->move(167, 135);
    modeKNB->setModel(&controls->m_modeModel);
    modeKNB->setText(tr("MODE"));
    modeKNB->setHintText(tr("Mode:"), "");

    m_showWVW = new VisualizationWidget(
            embed::getIconPixmap("output_bigger_graph"), this,
            VisualizationWidget::Stereo);
    // m_showWVW->move(23, 205);

    mainLayout->addWidget(bankCMB, 0, 0, 1, 3);
    mainLayout->addWidget(indexCMB, 1, 0, 1, 7);

    mainLayout->addWidget(timeKNB, 2, 0, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(ratioKNB, 2, 1, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(outGainKNB, 2, 2, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(modeKNB, 2, 3, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(m_showWVW, 3, 0, 1, 8,
                          Qt::AlignVCenter | Qt::AlignHCenter);

    mainLayout->setColumnStretch(9, 1);
    mainLayout->setRowStretch(4, 1);

    setFixedWidth(250);
    // setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);

    connect(controls, SIGNAL(nextStereoBuffer(const sampleFrame*)), m_showWVW,
            SLOT(updateStereoBuffer(const sampleFrame*)));
}

ShaperGDXDialog::~ShaperGDXDialog()
{
    disconnect(m_effectControls, SIGNAL(nextStereoBuffer(const sampleFrame*)),
               m_showWVW, SLOT(updateStereoBuffer(const sampleFrame*)));
}
