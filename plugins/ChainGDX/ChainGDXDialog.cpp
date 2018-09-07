/*
 * ChainGDXControlDialog.cpp - control dialog for chaining effect
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

#include "ChainGDXControlDialog.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLayout>

#include "ChainGDX.h"
#include "ChainGDXControls.h"
#include "EffectRackView.h"
#include "embed.h"

ChainGDXControlDialog::ChainGDXControlDialog(ChainGDXControls* controls) :
      EffectControlDialog(controls)
{
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    //pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);
    setContentsMargins(0, 0, 0, 0);
    setFixedWidth(246);
    setMinimumHeight(200);

    EffectRackView* erv
            = new EffectRackView(controls->m_effect->m_chain, this);
    erv->setFixedSize(245, 200);
}
