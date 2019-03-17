/*
 * OutputGDXDialog.h - control dialog for audio output properties
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef OUTPUTGDX_DIALOG_H
#define OUTPUTGDX_DIALOG_H

#include "EffectControlDialog.h"

class OutputGDXControls;

class OutputGDXDialog : public EffectControlDialog
{
    Q_OBJECT

  public:
    OutputGDXDialog(OutputGDXControls* controls);
    virtual ~OutputGDXDialog()
    {
    }
};

#endif
