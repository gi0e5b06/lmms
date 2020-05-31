/*
 * FilterView.cpp - Basic filter view
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

#include "FilterView.h"

#include "ComboBox.h"
#include "Knob.h"
#include "TempoSyncKnob.h"
//#include "TabWidget.h"

#include "gui_templates.h"

#include <QGridLayout>
#include <QLabel>

FilterView::FilterView(const QString& _title,
                       QWidget*       _parent,
                       const bool     _led,
                       const bool     _arrow) :
      GroupBox(_title, _parent, _led, _arrow)
{
    /*
      this <-- m_filterGroupBox
      m_filterGroupBox->setContentWidget(filterPNL);
      envLOT->addWidget(m_filterGroupBox);
    */

    qInfo("filterPNL BEFORE");
    QWidget*     filterPNL = new QWidget(nullptr);
    QGridLayout* filterLOT = new QGridLayout(filterPNL);
    qInfo("filterPNL AFTER");
    filterLOT->setContentsMargins(3, 3, 3, 3);
    filterLOT->setColumnStretch(6, 1);
    filterLOT->setHorizontalSpacing(3);
    filterLOT->setVerticalSpacing(3);
    filterPNL->setLayout(filterLOT);

    m_filterTypeComboBox = new ComboBox(filterPNL);
    // m_filterTypeComboBox->setGeometry(3, 3, 155, 26);
    m_filterTypeComboBox->setFont(pointSize<8>(m_filterTypeComboBox->font()));
    m_filterTypeComboBox->setWhatsThis(
            tr("Here you can select the built-in filter you want to use "
               "for this instrument-track. Filters are very important "
               "for changing the characteristics of a sound."));

    m_filterPassesCMB = new ComboBox(filterPNL);
    // m_filterPassesCMB->setGeometry(3, 32, 78, 26);
    m_filterPassesCMB->setFont(pointSize<8>(m_filterPassesCMB->font()));

    m_filterCutKnob = new Knob(knobBright_26, filterPNL);
    m_filterCutKnob->setLabel(tr("FREQ"));
    m_filterCutKnob->setPointColor(Qt::green);
    // m_filterCutKnob->move(164, 3);
    m_filterCutKnob->setHintText(tr("cutoff frequency:"), " " + tr("Hz"));
    m_filterCutKnob->setWhatsThis(
            tr("Use this knob for setting the cutoff frequency for the "
               "selected filter. The cutoff frequency specifies the "
               "frequency for cutting the signal by a filter. For "
               "example a lowpass-filter cuts all frequencies above "
               "the cutoff frequency. A highpass-filter cuts all "
               "frequencies below cutoff frequency, and so on..."));

    m_filterResKnob = new Knob(knobBright_26, filterPNL);
    m_filterResKnob->setLabel(tr("RESO"));
    // m_filterResKnob->move(196, 3);
    m_filterResKnob->setHintText(tr("Resonance:"), "");
    m_filterResKnob->setWhatsThis(
            tr("Use this knob for setting Q/Resonance for the selected "
               "filter. Q/Resonance tells the filter how much it "
               "should amplify frequencies near Cutoff-frequency."));

    m_filterGainKnob = new Knob(filterPNL);
    m_filterGainKnob->setLabel(tr("GAIN"));
    m_filterGainKnob->setPointColor(Qt::red);
    // m_filterGainKnob->move(100, 43);
    m_filterGainKnob->setHintText(tr("Shelf gain:"), "");

    m_filterFeedbackAmountKnob = new Knob(filterPNL);
    m_filterFeedbackAmountKnob->setLabel(tr("FA"));
    // m_filterFeedbackAmountKnob->move(132, 43);
    m_filterFeedbackAmountKnob->setHintText(tr("Feedback amount:"), "");

    m_filterFeedbackDelayKnob = new TempoSyncKnob(filterPNL);
    m_filterFeedbackDelayKnob->setLabel(tr("FD"));
    // m_filterFeedbackDelayKnob->move(164, 43);
    m_filterFeedbackDelayKnob->setHintText(tr("Feedback delay:"), "ms");

    m_filterResponseKnob = new Knob(filterPNL);
    m_filterResponseKnob->setLabel(tr("RESP"));
    m_filterResponseKnob->setPointColor(Qt::yellow);
    // m_filterResponseKnob->move(196, 43);
    m_filterResponseKnob->setHintText(tr("Response:"), "");

    filterLOT->addWidget(m_filterTypeComboBox, 0, 0, 1, 5);
    filterLOT->addWidget(m_filterPassesCMB, 0, 5, 1, 2);
    filterLOT->addWidget(m_filterCutKnob, 1, 0);
    filterLOT->addWidget(m_filterResKnob, 1, 1);
    filterLOT->addWidget(m_filterGainKnob, 1, 2);
    filterLOT->addWidget(m_filterFeedbackAmountKnob, 1, 3);
    filterLOT->addWidget(m_filterFeedbackDelayKnob, 1, 4);
    filterLOT->addWidget(m_filterResponseKnob, 1, 5);

    filterPNL->setMinimumWidth(230);
    // filterPNL->setFixedHeight(81);  // 42;
    setContentWidget(filterPNL);
}

FilterView::~FilterView()
{
}

/*
  BasicFilters is not a Model for now.
  That makes things a bit ugly. Every thing is in SoundShaping.
*/
void FilterView::setModels(BoolModel*          _led,
                           ComboBoxModel*      _type,
                           FloatModel*         _cut,
                           FloatModel*         _res,
                           ComboBoxModel*      _passes,
                           FloatModel*         _gain,
                           FloatModel*         _response,
                           FloatModel*         _feedbackAmount,
                           TempoSyncKnobModel* _feedbackDelay)
{
    ledButton()->setModel(_led);
    m_filterTypeComboBox->setModel(_type);
    m_filterCutKnob->setModel(_cut);
    m_filterResKnob->setModel(_res);
    m_filterPassesCMB->setModel(_passes);
    m_filterGainKnob->setModel(_gain);
    m_filterResponseKnob->setModel(_response);
    m_filterFeedbackAmountKnob->setModel(_feedbackAmount);
    m_filterFeedbackDelayKnob->setModel(_feedbackDelay);
}
