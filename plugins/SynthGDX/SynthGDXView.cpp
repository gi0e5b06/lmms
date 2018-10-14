/*
 * SynthGDXView.cpp - modular synth
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

#include "SynthGDXView.h"

//#include <QBitmap>
#include <QGridLayout>
//#include <QPainter>
#include <QScrollArea>

#include "Knob.h"
#include "ModulatorView.h"
#include "OscillatorView.h"
#include "ToolTip.h"
#include "debug.h"
#include "embed.h"

SynthGDXView::SynthGDXView(Instrument* _instrument, QWidget* _parent) :
      InstrumentView(_instrument, _parent)
{
    setAutoFillBackground(true);
    // QPalette pal;
    // pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("artwork"));
    // setPalette(pal);

    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    // scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // scrollArea->setPalette(QApplication::palette(scrollArea));
    scrollArea->setFrameStyle(QFrame::NoFrame);
    scrollArea->setLineWidth(1);
    scrollArea->setContentsMargins(0, 0, 0, 0);
    scrollArea->setWidget(new QWidget);

    QVBoxLayout* saLayout = new QVBoxLayout(this);
    setLayout(saLayout);
    saLayout->setContentsMargins(0, 0, 0, 0);
    saLayout->addWidget(scrollArea);

    QWidget* panel = new QWidget(scrollArea);
    // panel->setContentsMargins(0, 0, 0, 0);

    scrollArea->setWidget(panel);

    QGridLayout* mainLayout = new QGridLayout(panel);
    mainLayout->setContentsMargins(0, 6, 0, 6);  // 6, 6, 6, 6);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setHorizontalSpacing(1);  // 6);
    mainLayout->setVerticalSpacing(6);

    SynthGDX* o = castModel<SynthGDX>();

    for(int i = 0; i < NB_OSCILLATORS; ++i)
    {
        m_oscView[i] = new OscillatorView(o->m_osc[i], i, panel);
        mainLayout->addWidget(m_oscView[i], i, 0);
    }

    for(int i = 0; i < NB_MODULATORS; ++i)
    {
        m_modView[i] = new ModulatorView(o->m_mod[i], i, panel);
        mainLayout->addWidget(m_modView[i], NB_OSCILLATORS + i, 0);
    }

    /*
    Graph* g     = new Graph(panel, Graph::LinearStyle, 256, 256);
    g->setModel(m_GraphModel);
    mainLayout->addWidget(NB_OSCILLATORS + NB_MODULATORS);
    */

    panel->setFixedSize(panel->sizeHint());
}

SynthGDXView::~SynthGDXView()
{
}

void SynthGDXView::modelChanged()
{
    // update();
}
