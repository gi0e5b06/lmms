/*
 * OscilloscopeGDXDialog.cpp -
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

#include "OscilloscopeGDXDialog.h"

#include "ComboBox.h"
#include "Knob.h"
//#include "OscilloscopeGDX.h"
#include "OscilloscopeGDXControls.h"
#include "VisualizationWidget.h"
#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>

OscilloscopeGDXDialog::OscilloscopeGDXDialog(
        OscilloscopeGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    // pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(6);

    // m_background = QPixmap(250, 250);
    // m_background.fill(Qt::black);

    m_showWVW = new VisualizationWidget(250, 250, this, controls->ring());

    mainLayout->addWidget(m_showWVW, 0, 0);

    // setFixedWidth(250);
    // setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);

    // connect(controls, SIGNAL(nextStereoBuffer(const sampleFrame*)),
    // m_showWVW,
    //        SLOT(updateStereoBuffer(const sampleFrame*)));
}

OscilloscopeGDXDialog::~OscilloscopeGDXDialog()
{
    // disconnect(m_effectControls, SIGNAL(nextStereoBuffer(const
    // sampleFrame*)),
    //     m_showWVW, SLOT(updateStereoBuffer(const sampleFrame*)));
}
