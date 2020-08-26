/*
 * InstrumentSoundShapingView.h - Sound shaping view in the instrument window
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef INSTRUMENT_SOUND_SHAPING_VIEW_H
#define INSTRUMENT_SOUND_SHAPING_VIEW_H

#include "InstrumentSoundShaping.h"
#include "ModelView.h"

#include <QScrollArea>

class QLabel;

class EnvelopeAndLfoView;
class FilterView;
class ComboBox;
class GroupBox;
class Knob;
class TempoSyncKnob;
class TabWidget;

class InstrumentSoundShapingView :
      public QScrollArea /*QWidget*/,
      public ModelView
{
    Q_OBJECT
  public:
    InstrumentSoundShapingView(InstrumentSoundShaping* _model,
                               QWidget*                _parent);
    virtual ~InstrumentSoundShapingView();

    InstrumentSoundShaping* model()
    {
        return castModel<InstrumentSoundShaping>();
    }

    const InstrumentSoundShaping* model() const
    {
        return castModel<InstrumentSoundShaping>();
    }

    void setFunctionsHidden(bool hidden);

  private:
    virtual void modelChanged();

    // InstrumentSoundShaping* m_ss;

    // TabWidget * m_targetsTabWidget;
    EnvelopeAndLfoView* m_envLfoViews[InstrumentSoundShaping::NumTargets];

    // filter-stuff
    FilterView* m_filter1View;
    FilterView* m_filter2View;
    /*
    GroupBox*      m_filterGroupBox;
    ComboBox*      m_filterTypeComboBox;
    Knob*          m_filterCutKnob;
    Knob*          m_filterResKnob;  // Q, BW
    ComboBox*      m_filterPassesCMB;
    Knob*          m_filterGainKnob;
    Knob*          m_filterResponseKnob;
    Knob*          m_filterFeedbackAmountKnob;
    TempoSyncKnob* m_filterFeedbackDelayKnob;
    */
    // QLabel* m_singleStreamInfoLabel;
};

#endif
