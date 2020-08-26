/*
 * FilterView.h - Basic filter view
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

#ifndef FILTER_VIEW_H
#define FILTER_VIEW_H

#include "GroupBox.h"

#include <QString>

class QWidget;

class TempoSyncKnobModel;
class ComboBox;
class ComboBoxModel;
// class GroupBox;
class Knob;
class TempoSyncKnob;
class TempoSyncKnobModel;

class FilterView : public GroupBox
{
    Q_OBJECT

  public:
    FilterView(const QString& _title,
               QWidget*       _parent,
               const bool     _led,
               const bool     _arrow);
    ~FilterView();

    void setModels(BoolModel*          _led,
                   ComboBoxModel*      _type,
                   RealModel*          _cut,
                   RealModel*          _res,
                   ComboBoxModel*      _passes,
                   RealModel*          _gain,
                   RealModel*          _response,
                   RealModel*          _feedbackAmount,
                   TempoSyncKnobModel* _feedbackDelay);

  private:
    ComboBox*      m_filterTypeComboBox;
    Knob*          m_filterCutKnob;
    Knob*          m_filterResKnob;  // Q, BW
    ComboBox*      m_filterPassesCMB;
    Knob*          m_filterGainKnob;
    Knob*          m_filterResponseKnob;
    Knob*          m_filterFeedbackAmountKnob;
    TempoSyncKnob* m_filterFeedbackDelayKnob;
};

#endif
