/*
 * InstrumentSoundShapingView.cpp - Sound shaping view in the instrument
 * window
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
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

#include "InstrumentSoundShapingView.h"

#include "ComboBox.h"
#include "EnvelopeAndLfoParameters.h"  // REQUIRED
#include "EnvelopeAndLfoView.h"
#include "FilterView.h"
#include "GroupBox.h"
#include "Knob.h"
#include "TempoSyncKnob.h"
//#include "TabWidget.h"

#include "gui_templates.h"

#include <QGridLayout>
#include <QLabel>

/*
const int TARGETS_TABWIDGET_X      = 4;
const int TARGETS_TABWIDGET_Y      = 5;
const int TARGETS_TABWIDGET_WIDTH  = 242;
const int TARGETS_TABWIDGET_HEIGTH = 175;

const int FILTER_GROUPBOX_X = TARGETS_TABWIDGET_X;
const int FILTER_GROUPBOX_Y
        = TARGETS_TABWIDGET_Y + TARGETS_TABWIDGET_HEIGTH + 5;
const int FILTER_GROUPBOX_WIDTH  = TARGETS_TABWIDGET_WIDTH;
const int FILTER_GROUPBOX_HEIGHT = 245 - FILTER_GROUPBOX_Y;
*/

InstrumentSoundShapingView::InstrumentSoundShapingView(QWidget* _parent) :
      QScrollArea(_parent), ModelView(NULL, this), m_ss(NULL)
{
    /*
    m_targetsTabWidget = new TabWidget( tr( "TARGET" ), this );
    m_targetsTabWidget->setGeometry( TARGETS_TABWIDGET_X,
                                            TARGETS_TABWIDGET_Y,
                                            TARGETS_TABWIDGET_WIDTH,
                                            TARGETS_TABWIDGET_HEIGTH );
    m_targetsTabWidget->setWhatsThis(
            tr( "These tabs contain envelopes. They're very important for "
                    "modifying a sound, in that they are almost "
                    "always necessary for substractive synthesis. For "
                    "example if you have a volume envelope, you can set "
                    "when the sound should have a specific volume. "
                    "If you want to create some soft strings then your "
                    "sound has to fade in and out very softly. This can be "
                    "done by setting large attack and release times. "
                    "It's the same for other envelope targets like "
                    "panning, cutoff frequency for the used filter and so on.
    " "Just monkey around with it! You can really make cool " "sounds out of a
    saw-wave with just some " "envelopes...!" ) );

    for( int i = 0; i < InstrumentSoundShaping::NumTargets; ++i )
    {
            m_envLfoViews[i] = new EnvelopeAndLfoView( m_targetsTabWidget );
            m_targetsTabWidget->addTab( m_envLfoViews[i],
                                            tr(
    InstrumentSoundShaping::targetNames[i][0].toUtf8().constData() ), NULL );
    }
    */

    // E&L tab
    QScrollArea* envSCA = this;  // new QScrollArea( _tabWidget );
    envSCA->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    envSCA->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    envSCA->setFrameStyle(QFrame::NoFrame);

    QWidget*     envPNL = new QWidget(envSCA);
    QVBoxLayout* envLOT = new QVBoxLayout(envPNL);
    envLOT->setContentsMargins(3, 3, 3, 3);  // 2,3,2,5);

    m_filter1View
            = new FilterView(tr("FILTER 1 (modulated)"), envPNL, true, true);
    envLOT->addWidget(m_filter1View);

    m_filter2View = new FilterView(tr("FILTER 2 (not modulated)"), envPNL,
                                   true, true);
    envLOT->addWidget(m_filter2View);

    for(int i = 0; i < InstrumentSoundShaping::NumTargets; ++i)
    {
        GroupBox* g
                = new GroupBox(tr(InstrumentSoundShaping::targetNames[i][0]
                                          .toUtf8()
                                          .constData()),
                               envPNL, false, true);
        m_envLfoViews[i] = new EnvelopeAndLfoView(g);
        g->setContentWidget(m_envLfoViews[i]);
        envLOT->addWidget(g);
    }

    /*
    m_filterGroupBox       = new GroupBox(tr("FILTER"), envPNL, true, true);
    QWidget*     filterPNL = new QWidget(m_filterGroupBox);
    QGridLayout* filterLOT = new QGridLayout(filterPNL);
    filterLOT->setContentsMargins(6, 3, 3, 3);
    filterLOT->setColumnStretch(7, 1);
    filterLOT->setHorizontalSpacing(6);
    filterLOT->setVerticalSpacing(3);

    filterPNL->setMinimumWidth(230);
    // filterPNL->setFixedHeight(81);  // 42;
    m_filterGroupBox->setContentWidget(filterPNL);
    envLOT->addWidget(m_filterGroupBox);

    // m_filterGroupBox->setGeometry( FILTER_GROUPBOX_X, FILTER_GROUPBOX_Y,
    //				FILTER_GROUPBOX_WIDTH,
    //				FILTER_GROUPBOX_HEIGHT );

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

    // envLOT->addWidget(m_filterGroupBox);
    // envLOT->addStretch(1);
    // setLayout(envLOT);
    */

    envSCA->setWidgetResizable(true);
    envSCA->setWidget(envPNL);

    /*
    m_singleStreamInfoLabel = new QLabel( tr( "Envelopes, LFOs and filters are
    not supported by the current instrument." ), this );
    m_singleStreamInfoLabel->setWordWrap( true );
    m_singleStreamInfoLabel->setFont( pointSize<8>(
    m_singleStreamInfoLabel->font() ) );
    m_singleStreamInfoLabel->setFixedWidth(230);
    m_singleStreamInfoLabel->setVisible(false);
    */

    /*
    m_singleStreamInfoLabel->setGeometry( TARGETS_TABWIDGET_X,
                                          TARGETS_TABWIDGET_Y,
                                          TARGETS_TABWIDGET_WIDTH,
                                          TARGETS_TABWIDGET_HEIGTH );
    */
}

InstrumentSoundShapingView::~InstrumentSoundShapingView()
{
    // delete m_targetsTabWidget;
}

void InstrumentSoundShapingView::setFunctionsHidden(bool hidden)
{
    // m_targetsTabWidget->setHidden( hidden );
    // m_filterGroupBox->setHidden( hidden );
    // m_singleStreamInfoLabel->setVisible( !hidden );
    setVisible(hidden);
}

void InstrumentSoundShapingView::modelChanged()
{
    m_ss = castModel<InstrumentSoundShaping>();

    /*
    m_filterGroupBox->ledButton()->setModel(&m_ss->m_filterEnabledModel);
    m_filterTypeComboBox->setModel(&m_ss->m_filterTypeModel);
    m_filterCutKnob->setModel(&m_ss->m_filterCutModel);
    m_filterResKnob->setModel(&m_ss->m_filterResModel);
    m_filterPassesCMB->setModel(&m_ss->m_filterPassesModel);
    m_filterGainKnob->setModel(&m_ss->m_filterGainModel);
    m_filterResponseKnob->setModel(&m_ss->m_filterResponseModel);
    m_filterFeedbackAmountKnob->setModel(&m_ss->m_filterFeedbackAmountModel);
    m_filterFeedbackDelayKnob->setModel(&m_ss->m_filterFeedbackDelayModel);
    */

    m_filter1View->setModels(
            &m_ss->m_filter1EnabledModel, &m_ss->m_filter1TypeModel,
            &m_ss->m_filter1CutModel, &m_ss->m_filter1ResModel,
            &m_ss->m_filter1PassesModel, &m_ss->m_filter1GainModel,
            &m_ss->m_filter1ResponseModel,
            &m_ss->m_filter1FeedbackAmountModel,
            &m_ss->m_filter1FeedbackDelayModel);

    m_filter2View->setModels(
            &m_ss->m_filter2EnabledModel, &m_ss->m_filter2TypeModel,
            &m_ss->m_filter2CutModel, &m_ss->m_filter2ResModel,
            &m_ss->m_filter2PassesModel, &m_ss->m_filter2GainModel,
            &m_ss->m_filter2ResponseModel,
            &m_ss->m_filter2FeedbackAmountModel,
            &m_ss->m_filter2FeedbackDelayModel);

    for(int i = 0; i < InstrumentSoundShaping::NumTargets; ++i)
    {
        m_envLfoViews[i]->setModel(m_ss->m_envLfoParameters[i]);
    }
}
