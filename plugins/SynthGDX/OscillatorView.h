/*
 * OscillatorView.h - SynthGDX 8 oscillators
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

#ifndef OSCILLATOR_VIEW_H
#define OSCILLATOR_VIEW_H

#include "MemoryManager.h"

#include <QWidget>

class OscillatorObject;
class VisualizationWidget;

class OscillatorView : public QWidget
{
    MM_OPERATORS
    Q_OBJECT

  public:
    OscillatorView(OscillatorObject* _osc, const int _idx, QWidget* _parent);
    ~OscillatorView();

  public slots:
    void updateWave1IndexModel();
    void updateWave2IndexModel();
    void updateVisualizationWidget();

  private:
    OscillatorObject*    m_osc;
    VisualizationWidget* m_showWave;
};

#endif
