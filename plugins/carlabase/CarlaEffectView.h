/*
 * Carla for LMMS
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

#ifndef CARLA_EFFECT_VIEW_H
#define CARLA_EFFECT_VIEW_H

#include <QPushButton>

#include "CarlaEffect.h"
#include "EffectView.h"


class CarlaEffectView : public EffectView
{
    Q_OBJECT

public:
    CarlaEffectView(CarlaEffect* const effect, QWidget* const parent);
    virtual ~CarlaEffectView();

private slots:
    void toggleUI(bool);
    void uiClosed();

private:
    virtual void modelChanged();
    virtual void timerEvent(QTimerEvent*);

    NativePluginHandle fHandle;
    const NativePluginDescriptor* fDescriptor;
    int fTimerId;

    QPushButton * m_toggleUIButton;
};

#endif
