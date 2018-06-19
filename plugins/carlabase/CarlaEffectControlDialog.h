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

#ifndef CARLA_EFFECT_CONTROL_DIALOG_H
#define CARLA_EFFECT_CONTROL_DIALOG_H

#include <QPushButton>

#include "CarlaNative.h"

#include "EffectControlDialog.h"
//#include "CarlaEffect.h"


class CarlaEffectControls;

class CarlaEffectControlDialog : public EffectControlDialog
{
        Q_OBJECT
 public:
        CarlaEffectControlDialog(CarlaEffectControls* controls);
        virtual ~CarlaEffectControlDialog();

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
} ;


#endif
