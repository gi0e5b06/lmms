/*
 * FrequencyGDXDialog.h -
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

#ifndef FREQUENCYGDX_DIALOG_H
#define FREQUENCYGDX_DIALOG_H

#include "EffectControlDialog.h"

class FrequencyGDXControls;
// class VisualizationWidget;
class QLabel;

class FrequencyGDXDialog : public EffectControlDialog
{
    Q_OBJECT

  public:
    FrequencyGDXDialog(FrequencyGDXControls* controls);
    virtual ~FrequencyGDXDialog();

  public slots:
    void updateTopFrequency();
    void updateAvgFrequency();
    void updateMainFrequency();

    void updateTopKey();
    void updateAvgKey();
    void updateMainKey();

    void updateTopNote();
    void updateAvgNote();
    void updateMainNote();

  protected:
    QLabel* m_topFrequencyLBL;
    QLabel* m_avgFrequencyLBL;
    QLabel* m_mainFrequencyLBL;

    QLabel* m_topKeyLBL;
    QLabel* m_avgKeyLBL;
    QLabel* m_mainKeyLBL;

    QLabel* m_topNoteLBL;
    QLabel* m_avgNoteLBL;
    QLabel* m_mainNoteLBL;
};

#endif
