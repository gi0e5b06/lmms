/*
 * EnvelopeAndLfoView.cpp - widget which is m_used by envelope/lfo/filter-
 *                          tab of instrument track window
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "EnvelopeAndLfoView.h"

#include "ComboBox.h"
#include "DataFile.h"
#include "Engine.h"
#include "EnvelopeAndLfoParameters.h"
#include "Knob.h"
#include "LedCheckBox.h"
#include "Mixer.h"
#include "Oscillator.h"
#include "PixmapButton.h"
#include "StringPairDrag.h"
#include "TempoSyncKnob.h"
#include "TextFloat.h"
#include "ToolTip.h"
#include "Track.h"
#include "WaveFormStandard.h"
#include "embed.h"
#include "gui_templates.h"

#include <QMouseEvent>
#include <QPainter>

// extern const float SECS_PER_ENV_SEGMENT;
// extern const float SECS_PER_LFO_OSCILLATION;

const int ENV_GRAPH_X = 4;  // 6
const int ENV_GRAPH_Y = 3;  // 6;

const int ENV_KNOBS_Y     = ENV_GRAPH_Y + 61;
const int ENV_KNOBS_LBL_Y = ENV_KNOBS_Y + 32;  // 35;
const int KNOB_X_SPACING  = 30;                // 32;
const int PREDELAY_KNOB_X = 3;                 // 6;
const int ATTACK_KNOB_X   = PREDELAY_KNOB_X + KNOB_X_SPACING;
const int HOLD_KNOB_X     = ATTACK_KNOB_X + KNOB_X_SPACING;
const int DECAY_KNOB_X    = HOLD_KNOB_X + KNOB_X_SPACING;
const int SUSTAIN_KNOB_X  = DECAY_KNOB_X + KNOB_X_SPACING;
const int RELEASE_KNOB_X  = SUSTAIN_KNOB_X + KNOB_X_SPACING;
const int AMOUNT_KNOB_X   = RELEASE_KNOB_X + KNOB_X_SPACING;

const int TIME_UNIT_WIDTH = 45;

const int LFO_GRAPH_X         = 3;                    // 6;
const int LFO_GRAPH_Y         = ENV_KNOBS_LBL_Y + 8;  // 14
const int LFO_KNOB_Y          = LFO_GRAPH_Y + 61;     //-2;
const int LFO_PREDELAY_KNOB_X = LFO_GRAPH_X;          // + 92; //100;
const int LFO_ATTACK_KNOB_X   = LFO_PREDELAY_KNOB_X + KNOB_X_SPACING;
const int LFO_SPEED_KNOB_X    = LFO_ATTACK_KNOB_X + KNOB_X_SPACING;
const int LFO_AMOUNT_KNOB_X   = LFO_SPEED_KNOB_X + KNOB_X_SPACING;
const int LFO_SHAPES_X = LFO_GRAPH_X;            // PREDELAY_KNOB_X;
const int LFO_SHAPES_Y = LFO_GRAPH_Y + 61 + 39;  // 50

EnvelopeAndLfoView::EnvelopeAndLfoView(QWidget* _parent) :
      QWidget(_parent), ModelView(NULL, this), m_params(NULL)
{
    m_envGraph = embed::getPixmap("envelope_bigger_graph");
    m_lfoGraph = embed::getPixmap("lfo_bigger_graph");

    m_predelayKnob = new TempoSyncKnob(knobBright_26, this);
    m_predelayKnob->setLabel(tr("DEL"));
    m_predelayKnob->move(PREDELAY_KNOB_X, ENV_KNOBS_Y);
    m_predelayKnob->setHintText(tr("Predelay:"), "");
    m_predelayKnob->setWhatsThis(
            tr("Use this knob for setting predelay of the current "
               "envelope. The bigger this value the longer the time "
               "before start of actual envelope."));

    m_attackKnob = new TempoSyncKnob(knobBright_26, this);
    m_attackKnob->setLabel(tr("ATT"));
    m_attackKnob->move(ATTACK_KNOB_X, ENV_KNOBS_Y);
    m_attackKnob->setHintText(tr("Attack:"), "");
    m_attackKnob->setWhatsThis(
            tr("Use this knob for setting attack-time of the current "
               "envelope. The bigger this value the longer the "
               "envelope needs to increase to attack-level. "
               "Choose a small value for instruments like pianos "
               "and a big value for strings."));

    m_holdKnob = new TempoSyncKnob(knobBright_26, this);
    m_holdKnob->setLabel(tr("HOLD"));
    m_holdKnob->move(HOLD_KNOB_X, ENV_KNOBS_Y);
    m_holdKnob->setHintText(tr("Hold:"), "");
    m_holdKnob->setWhatsThis(
            tr("Use this knob for setting hold-time of the current "
               "envelope. The bigger this value the longer the "
               "envelope holds attack-level before it begins to "
               "decrease to sustain-level."));

    m_decayKnob = new TempoSyncKnob(knobBright_26, this);
    m_decayKnob->setLabel(tr("DEC"));
    m_decayKnob->move(DECAY_KNOB_X, ENV_KNOBS_Y);
    m_decayKnob->setHintText(tr("Decay:"), "");
    m_decayKnob->setWhatsThis(
            tr("Use this knob for setting decay-time of the current "
               "envelope. The bigger this value the longer the "
               "envelope needs to decrease from attack-level to "
               "sustain-level. Choose a small value for instruments "
               "like pianos."));

    m_sustainKnob = new Knob(knobBright_26, this);
    m_sustainKnob->setLabel(tr("SUST"));
    m_sustainKnob->move(SUSTAIN_KNOB_X, ENV_KNOBS_Y);
    m_sustainKnob->setHintText(tr("Sustain:"), "");
    m_sustainKnob->setWhatsThis(
            tr("Use this knob for setting sustain-level of the current "
               "envelope. The bigger this value the higher the level "
               "on which the envelope stays before going down to "
               "zero."));

    m_releaseKnob = new TempoSyncKnob(knobBright_26, this);
    m_releaseKnob->setLabel(tr("REL"));
    m_releaseKnob->move(RELEASE_KNOB_X, ENV_KNOBS_Y);
    m_releaseKnob->setHintText(tr("Release:"), "");
    m_releaseKnob->setWhatsThis(
            tr("Use this knob for setting release-time of the current "
               "envelope. The bigger this value the longer the "
               "envelope needs to decrease from sustain-level to "
               "zero. Choose a big value for soft instruments like "
               "strings."));

    m_amountKnob = new Knob(knobBright_26, this);
    m_amountKnob->setLabel(tr("AMT"));
    m_amountKnob->move(AMOUNT_KNOB_X, ENV_KNOBS_Y);  // ENV_GRAPH_Y );
    m_amountKnob->setHintText(tr("Modulation amount:"), "");
    m_amountKnob->setWhatsThis(
            tr("Use this knob for setting modulation amount of the "
               "current envelope. The bigger this value the more the "
               "according size (e.g. volume or cutoff-frequency) "
               "will be influenced by this envelope."));

    m_lfoPredelayKnob = new Knob(knobBright_26, this);
    m_lfoPredelayKnob->setLabel(tr("PRE"));
    m_lfoPredelayKnob->move(LFO_PREDELAY_KNOB_X, LFO_KNOB_Y);
    m_lfoPredelayKnob->setHintText(tr("LFO predelay:"), "");
    m_lfoPredelayKnob->setWhatsThis(
            tr("Use this knob for setting predelay-time of the current "
               "LFO. The bigger this value the the time until the "
               "LFO starts to oscillate."));

    m_lfoAttackKnob = new Knob(knobBright_26, this);
    m_lfoAttackKnob->setLabel(tr("ATT"));
    m_lfoAttackKnob->move(LFO_ATTACK_KNOB_X, LFO_KNOB_Y);
    m_lfoAttackKnob->setHintText(tr("LFO- attack:"), "");
    m_lfoAttackKnob->setWhatsThis(
            tr("Use this knob for setting attack-time of the current LFO. "
               "The bigger this value the longer the LFO needs to "
               "increase its amplitude to maximum."));

    m_lfoSpeedKnob = new TempoSyncKnob(knobBright_26, this);
    m_lfoSpeedKnob->setLabel(tr("SPD"));
    m_lfoSpeedKnob->move(LFO_SPEED_KNOB_X, LFO_KNOB_Y);
    m_lfoSpeedKnob->setHintText(tr("LFO speed:"), "");
    m_lfoSpeedKnob->setWhatsThis(
            tr("Use this knob for setting speed of the current LFO. The "
               "bigger this value the faster the LFO oscillates and "
               "the faster will be your effect."));

    m_lfoAmountKnob = new Knob(knobBright_26, this);
    m_lfoAmountKnob->setLabel(tr("AMT"));
    m_lfoAmountKnob->move(LFO_AMOUNT_KNOB_X, LFO_KNOB_Y);
    m_lfoAmountKnob->setHintText(tr("Modulation amount:"), "");
    m_lfoAmountKnob->setWhatsThis(
            tr("Use this knob for setting modulation amount of the "
               "current LFO. The bigger this value the more the "
               "selected size (e.g. volume or cutoff-frequency) will "
               "be influenced by this LFO."));

    /*
    PixmapButton * sin_lfo_btn = new PixmapButton( this, NULL );
    sin_lfo_btn->move( LFO_SHAPES_X, LFO_SHAPES_Y );
    sin_lfo_btn->setActiveGraphic( embed::getIconPixmap(
                                                    "sin_wave_active" ) );
    sin_lfo_btn->setInactiveGraphic( embed::getIconPixmap(
                                                    "sin_wave_inactive" ) );
    sin_lfo_btn->setWhatsThis(
            tr( "Click here for a sine-wave." ) );

    PixmapButton * triangle_lfo_btn = new PixmapButton( this, NULL );
    triangle_lfo_btn->move( LFO_SHAPES_X+15, LFO_SHAPES_Y );
    triangle_lfo_btn->setActiveGraphic( embed::getIconPixmap(
                                            "triangle_wave_active" ) );
    triangle_lfo_btn->setInactiveGraphic( embed::getIconPixmap(
                                            "triangle_wave_inactive" ) );
    triangle_lfo_btn->setWhatsThis(
            tr( "Click here for a triangle-wave." ) );

    PixmapButton * saw_lfo_btn = new PixmapButton( this, NULL );
    saw_lfo_btn->move( LFO_SHAPES_X+30, LFO_SHAPES_Y );
    saw_lfo_btn->setActiveGraphic( embed::getIconPixmap(
                                                    "saw_wave_active" ) );
    saw_lfo_btn->setInactiveGraphic( embed::getIconPixmap(
                                                    "saw_wave_inactive" ) );
    saw_lfo_btn->setWhatsThis(
            tr( "Click here for a saw-wave for current." ) );

    PixmapButton * sqr_lfo_btn = new PixmapButton( this, NULL );
    sqr_lfo_btn->move( LFO_SHAPES_X+45, LFO_SHAPES_Y );
    sqr_lfo_btn->setActiveGraphic( embed::getIconPixmap(
                                            "square_wave_active" ) );
    sqr_lfo_btn->setInactiveGraphic( embed::getIconPixmap(
                                            "square_wave_inactive" ) );
    sqr_lfo_btn->setWhatsThis(
            tr( "Click here for a square-wave." ) );

    m_userLfoBtn = new PixmapButton( this, NULL );
    m_userLfoBtn->move( LFO_SHAPES_X+75, LFO_SHAPES_Y );
    m_userLfoBtn->setActiveGraphic( embed::getIconPixmap(
                                                    "usr_wave_active" ) );
    m_userLfoBtn->setInactiveGraphic( embed::getIconPixmap(
                                                    "usr_wave_inactive" ) );
    m_userLfoBtn->setWhatsThis(
            tr( "Click here for a user-defined wave. "
                    "Afterwards, drag an according sample-"
                    "file onto the LFO graph." ) );

    connect( m_userLfoBtn, SIGNAL( toggled( bool ) ),
                            this, SLOT( lfoUserWaveChanged() ) );

    PixmapButton * random_lfo_btn = new PixmapButton( this, NULL );
    random_lfo_btn->move( LFO_SHAPES_X+60, LFO_SHAPES_Y );
    random_lfo_btn->setActiveGraphic( embed::getIconPixmap(
                                            "random_wave_active" ) );
    random_lfo_btn->setInactiveGraphic( embed::getIconPixmap(
                                            "random_wave_inactive" ) );
    random_lfo_btn->setWhatsThis(
            tr( "Click here for random wave." ) );

    m_lfoWaveBtnGrp = new automatableButtonGroup( this );
    m_lfoWaveBtnGrp->addButton( sin_lfo_btn );
    m_lfoWaveBtnGrp->addButton( triangle_lfo_btn );
    m_lfoWaveBtnGrp->addButton( saw_lfo_btn );
    m_lfoWaveBtnGrp->addButton( sqr_lfo_btn );
    m_lfoWaveBtnGrp->addButton( m_userLfoBtn );
    m_lfoWaveBtnGrp->addButton( random_lfo_btn );
    */

    m_x100Cb = new LedCheckBox(tr("FREQ x 100"), this);
    m_x100Cb->setFont(pointSizeF(m_x100Cb->font(), 6.5));
    // m_x100Cb->move( LFO_PREDELAY_KNOB_X, LFO_GRAPH_Y + 36 );
    m_x100Cb->move(LFO_AMOUNT_KNOB_X + 29, LFO_KNOB_Y);
    m_x100Cb->setWhatsThis(
            tr("Click here if the frequency of this LFO should be "
               "multiplied by 100."));
    ToolTip::add(m_x100Cb, tr("multiply LFO-frequency by 100"));

    m_controlEnvAmountCb = new LedCheckBox(tr("MODULATE ENV-AMOUNT"), this);
    // m_controlEnvAmountCb->move( LFO_PREDELAY_KNOB_X, LFO_GRAPH_Y + 52 );
    m_controlEnvAmountCb->move(LFO_SHAPES_X + 94, LFO_SHAPES_Y);
    m_controlEnvAmountCb->setFont(
            pointSizeF(m_controlEnvAmountCb->font(), 6.5));
    m_controlEnvAmountCb->setWhatsThis(
            tr("Click here to make the envelope-amount controlled by this "
               "LFO."));
    ToolTip::add(m_controlEnvAmountCb,
                 tr("control envelope-amount by this LFO"));

    m_lfoWaveBankCMB = new ComboBox(this);
    m_lfoWaveBankCMB->setGeometry(LFO_SHAPES_X, LFO_SHAPES_Y, 89, 18);

    m_lfoWaveIndexCMB = new ComboBox(this);
    m_lfoWaveIndexCMB->setGeometry(LFO_SHAPES_X, LFO_SHAPES_Y + 24, 224, 18);

    setAcceptDrops(true);
}

EnvelopeAndLfoView::~EnvelopeAndLfoView()
{
    // delete m_lfoWaveBtnGrp;
}

void EnvelopeAndLfoView::modelChanged()
{
    m_params = castModel<EnvelopeAndLfoParameters>();
    m_predelayKnob->setModel(&m_params->m_predelayModel);
    m_attackKnob->setModel(&m_params->m_attackModel);
    m_holdKnob->setModel(&m_params->m_holdModel);
    m_decayKnob->setModel(&m_params->m_decayModel);
    m_sustainKnob->setModel(&m_params->m_sustainModel);
    m_releaseKnob->setModel(&m_params->m_releaseModel);
    m_amountKnob->setModel(&m_params->m_amountModel);
    m_lfoPredelayKnob->setModel(&m_params->m_lfoPredelayModel);
    m_lfoAttackKnob->setModel(&m_params->m_lfoAttackModel);
    m_lfoSpeedKnob->setModel(&m_params->m_lfoSpeedModel);
    m_lfoAmountKnob->setModel(&m_params->m_lfoAmountModel);
    // m_lfoWaveBtnGrp->setModel( &m_params->m_lfoWaveModel );
    m_lfoWaveBankCMB->setModel(&m_params->m_lfoWaveBankModel);
    m_lfoWaveIndexCMB->setModel(&m_params->m_lfoWaveIndexModel);
    m_x100Cb->setModel(&m_params->m_x100Model);
    m_controlEnvAmountCb->setModel(&m_params->m_controlEnvAmountModel);

    connect(m_params, SIGNAL(dataChanged()), this, SLOT(update()));
}

void EnvelopeAndLfoView::update()
{
    // qInfo("EnvelopeAndLfoView::update");
    QWidget::update();
}

void EnvelopeAndLfoView::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() != Qt::LeftButton)
    {
        return;
    }

    if(QRect(ENV_GRAPH_X, ENV_GRAPH_Y, m_envGraph.width(),
             m_envGraph.height())
               .contains(_me->pos())
       == true)
    {
        if(m_params->m_amountModel.value() < 1.0f)
        {
            m_params->m_amountModel.setValue(1.0f);
        }
        else
        {
            m_params->m_amountModel.setValue(0.0f);
        }
    }
    else if(QRect(LFO_GRAPH_X, LFO_GRAPH_Y, m_lfoGraph.width(),
                  m_lfoGraph.height())
                    .contains(_me->pos())
            == true)
    {
        if(m_params->m_lfoAmountModel.value() < 1.0f)
        {
            m_params->m_lfoAmountModel.setValue(1.0f);
        }
        else
        {
            m_params->m_lfoAmountModel.setValue(0.0f);
        }
    }
}

void EnvelopeAndLfoView::dragEnterEvent(QDragEnterEvent* _dee)
{
    StringPairDrag::processDragEnterEvent(
            _dee, QString("samplefile,tco_%1").arg(Track::SampleTrack));
}

void EnvelopeAndLfoView::dropEvent(QDropEvent* _de)
{
    QString type  = StringPairDrag::decodeKey(_de);
    QString value = StringPairDrag::decodeValue(_de);
    if(type == "samplefile")
    {
        m_params->m_userWave.setAudioFile(StringPairDrag::decodeValue(_de));
        // m_userLfoBtn->model()->setValue( true );
        // m_params->m_lfoWaveModel.setValue(EnvelopeAndLfoParameters::UserDefinedWave);
        m_params->m_lfoWaveBankModel.setValue(0);
        m_params->m_lfoWaveIndexModel.setValue(
                EnvelopeAndLfoParameters::UserDefinedWave);
        _de->accept();
        update();
    }
    else if(type == QString("tco_%1").arg(Track::SampleTrack))
    {
        DataFile dataFile(value.toUtf8());
        m_params->m_userWave.setAudioFile(dataFile.content()
                                                  .firstChildElement()
                                                  .firstChildElement()
                                                  .firstChildElement()
                                                  .attribute("src"));
        // m_userLfoBtn->model()->setValue( true );
        // m_params->m_lfoWaveModel.setValue(EnvelopeAndLfoParameters::UserDefinedWave);
        m_params->m_lfoWaveBankModel.setValue(0);
        m_params->m_lfoWaveIndexModel.setValue(
                EnvelopeAndLfoParameters::UserDefinedWave);
        _de->accept();
        update();
    }
}

void EnvelopeAndLfoView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    QString  s;

    // draw envelope-graph
    p.drawPixmap(ENV_GRAPH_X, ENV_GRAPH_Y, m_envGraph);
    // draw LFO-graph
    p.drawPixmap(LFO_GRAPH_X, LFO_GRAPH_Y, m_lfoGraph);

    p.setRenderHint(QPainter::Antialiasing);

    p.setFont(pointSize<29>(p.font()));
    p.setPen(Qt::black);  // Color(0,0,0,160));
    p.drawText(ENV_GRAPH_X + 3, ENV_GRAPH_Y + 40, tr("Envelope"));
    p.drawText(LFO_GRAPH_X + 3, LFO_GRAPH_Y + 40, tr("LFO"));

    /*
    p.setPen(Qt::black);
    p.drawText( ENV_GRAPH_X + m_envGraph.width() - 50, ENV_GRAPH_Y+11,
    tr("Envelope")); p.setPen(Qt::white); p.drawText( ENV_GRAPH_X +
    m_envGraph.width() - 50, ENV_GRAPH_Y+11, tr("Envelope"));

    p.setPen(Qt::black);
    p.drawText( LFO_GRAPH_X + m_lfoGraph.width() - 23, LFO_GRAPH_Y+10,
    tr("LFO")); p.setPen(Qt::white); p.drawText( LFO_GRAPH_X +
    m_lfoGraph.width() - 23, LFO_GRAPH_Y+10, tr("LFO"));
    */

    const real_t env_gray_amount
            = 1.0f - fabsf(m_amountKnob->value<real_t>());
    p.setPen(QPen(
            QColor(static_cast<int>(96 * env_gray_amount),
                   static_cast<int>(180 - 84 * env_gray_amount),    // 255,159
                   static_cast<int>(255 - 159 * env_gray_amount)),  // 128,32
            1));

    const QColor end_points_color(0x99, 0xAF, 0xFF);
    const QColor end_points_bg_color(0, 0, 2);

    const int y_base       = ENV_GRAPH_Y + m_envGraph.height() - 3;
    const int avail_height = m_envGraph.height() - 6;

    int x1 = static_cast<int>(m_predelayKnob->value<real_t>()
                              * TIME_UNIT_WIDTH);
    int x2 = x1
             + static_cast<int>(m_attackKnob->value<real_t>()
                                * TIME_UNIT_WIDTH);
    int x3 = x2
             + static_cast<int>(m_holdKnob->value<real_t>()
                                * TIME_UNIT_WIDTH);
    int x4 = x3
             + static_cast<int>((m_decayKnob->value<real_t>()
                                 * (1 - m_sustainKnob->value<real_t>()))
                                * TIME_UNIT_WIDTH);
    int x5 = x4
             + static_cast<int>(m_releaseKnob->value<real_t>()
                                * TIME_UNIT_WIDTH);

    if(x5 > 174)
    {
        x1 = (x1 * 174) / x5;
        x2 = (x2 * 174) / x5;
        x3 = (x3 * 174) / x5;
        x4 = (x4 * 174) / x5;
        x5 = (x5 * 174) / x5;
    }
    x1 += ENV_GRAPH_X + 2;
    x2 += ENV_GRAPH_X + 2;
    x3 += ENV_GRAPH_X + 2;
    x4 += ENV_GRAPH_X + 2;
    x5 += ENV_GRAPH_X + 2;

    p.drawLine(x1, y_base, x2, y_base - avail_height);
    p.fillRect(x1 - 1, y_base - 2, 4, 4, end_points_bg_color);
    p.fillRect(x1, y_base - 1, 2, 2, end_points_color);

    p.drawLine(x2, y_base - avail_height, x3, y_base - avail_height);
    p.fillRect(x2 - 1, y_base - 2 - avail_height, 4, 4, end_points_bg_color);
    p.fillRect(x2, y_base - 1 - avail_height, 2, 2, end_points_color);

    p.drawLine(x3, y_base - avail_height, x4,
               static_cast<int>(y_base - avail_height
                                + (1 - m_sustainKnob->value<real_t>())
                                          * avail_height));
    p.fillRect(x3 - 1, y_base - 2 - avail_height, 4, 4, end_points_bg_color);
    p.fillRect(x3, y_base - 1 - avail_height, 2, 2, end_points_color);

    p.drawLine(x4,
               static_cast<int>(y_base - avail_height
                                + (1 - m_sustainKnob->value<real_t>())
                                          * avail_height),
               x5, y_base);
    p.fillRect(x4 - 1,
               static_cast<int>(y_base - avail_height
                                + (1 - m_sustainKnob->value<real_t>())
                                          * avail_height)
                       - 2,
               4, 4, end_points_bg_color);
    p.fillRect(x4,
               static_cast<int>(y_base - avail_height
                                + (1 - m_sustainKnob->value<real_t>())
                                          * avail_height)
                       - 1,
               2, 2, end_points_color);
    p.fillRect(x5 - 1, y_base - 2, 4, 4, end_points_bg_color);
    p.fillRect(x5, y_base - 1, 2, 2, end_points_color);

    p.setFont(pointSize<6>(p.font()));

    int lx = ENV_GRAPH_X + 182;
    int dx = 0;
    int ly = ENV_GRAPH_Y + 10;  //+m_envGraph.height()-5;
    int dy = 9;

    s = QString("P %1 ms").arg(int(1000. * m_predelayKnob->value<real_t>()));
    p.setPen(Qt::black);
    p.drawText(lx + 1, ly + 1, s);
    p.setPen(Qt::white);
    p.drawText(lx, ly, s);

    lx += dx;
    ly += dy;
    s = QString("A %1 ms").arg(int(1000. * m_attackKnob->value<real_t>()));
    p.setPen(Qt::black);
    p.drawText(lx + 1, ly + 1, s);
    p.setPen(Qt::white);
    p.drawText(lx, ly, s);

    lx += dx;
    ly += dy;
    s = QString("H %1 ms").arg(int(1000. * m_holdKnob->value<real_t>()));
    p.setPen(Qt::black);
    p.drawText(lx + 1, ly + 1, s);
    p.setPen(Qt::white);
    p.drawText(lx, ly, s);

    lx += dx;
    ly += dy;
    s = QString("D %1 ms").arg(int(1000. * m_decayKnob->value<real_t>()
                                   * (1. - m_sustainKnob->value<real_t>())));
    p.setPen(Qt::black);
    p.drawText(lx + 1, ly + 1, s);
    p.setPen(Qt::white);
    p.drawText(lx, ly, s);

    lx += dx;
    ly += dy;
    s = QString("S %1%").arg(int(100. * m_sustainKnob->value<real_t>()));
    p.setPen(Qt::black);
    p.drawText(lx + 1, ly + 1, s);
    p.setPen(Qt::white);
    p.drawText(lx, ly, s);

    lx += dx;
    ly += dy;
    s = QString("R %1 ms").arg(int(1000. * m_releaseKnob->value<real_t>()));
    p.setPen(Qt::black);
    p.drawText(lx + 1, ly + 1, s);
    p.setPen(Qt::white);
    p.drawText(lx, ly, s);

    int LFO_GRAPH_W  = m_lfoGraph.width() - 6;   // substract border
    int LFO_GRAPH_H  = m_lfoGraph.height() - 7;  // substract border
    int graph_x_base = LFO_GRAPH_X + 3;
    int graph_y_base = LFO_GRAPH_Y + 3 + LFO_GRAPH_H / 2;

    const real_t frames_for_graph
            = EnvelopeAndLfoParameters::SECS_PER_LFO_OSCILLATION
              * Engine::mixer()->baseSampleRate() / 10.;

    const real_t lfo_gray_amount
            = 1.0f - fabsf(m_lfoAmountKnob->value<real_t>());
    p.setPen(QPen(
            QColor(static_cast<int>(96 * lfo_gray_amount),
                   static_cast<int>(180 - 84 * lfo_gray_amount),    // 255,159
                   static_cast<int>(255 - 159 * lfo_gray_amount)),  // 128,32
            1));

    real_t osc_frames = m_params->m_lfoOscillationFrames;
    if(m_params->m_x100Model.value())
        osc_frames *= 100.0f;
    osc_frames = bound(1.E-5, osc_frames, 1.E+10);

    // userWaveSample() may be used, called out of loop for efficiency
    m_params->m_userWave.dataReadLock();
    real_t    old_y = 0;
    const int xstep = (LFO_GRAPH_W % 3 == 0 ? 3 : 2);
    for(int x = 0; x <= LFO_GRAPH_W; x += xstep)
    {
        real_t val        = 0.0;
        real_t cur_sample = x * frames_for_graph / LFO_GRAPH_W;
        if(static_cast<f_cnt_t>(cur_sample) > m_params->m_lfoPredelayFrames)
        {
            cur_sample -= m_params->m_lfoPredelayFrames;
            const real_t phase = positivefraction(cur_sample / osc_frames);
            /*
            switch( m_params->m_lfoWaveModel.value() )
            {
                    case EnvelopeAndLfoParameters::SineWave:
                            val = WaveForm::sine(phase);
                            //Oscillator::sinSample( phase );
                            break;
                    case EnvelopeAndLfoParameters::TriangleWave:
                            val = WaveForm::triangle(phase);
                            //Oscillator::triangleSample( phase );
                            break;
                    case EnvelopeAndLfoParameters::SawWave:
                            val = WaveForm::sawtooth(phase);
                            //Oscillator::sawSample( phase );
                            break;
                    case EnvelopeAndLfoParameters::SquareWave:
                            val = WaveForm::square(phase);
                            //Oscillator::squareSample( phase );
                            break;
                    case EnvelopeAndLfoParameters::RandomWave:
                    //if( x % (int)( 900 * m_lfoSpeedKnob->value<real_t>() + 1
            ) == 0 )
                    //{
                    //m_randomGraph = Oscillator::noiseSample( 0.0f );
                    //}
                    //val = m_randomGraph;
                            val = WaveForm::whitenoise(phase);
                            break;
                    case EnvelopeAndLfoParameters::UserDefinedWave:
                            val = m_params->m_userWave.
                                            userWaveSample( phase );
                            break;
            }
            */
            val = WaveFormStandard::get(m_params->m_lfoWaveBankModel.value(),
                                        m_params->m_lfoWaveIndexModel.value())
                          ->f(phase);

            if(m_params->m_lfoAttackFrames > 0
               && cur_sample <= m_params->m_lfoAttackFrames)
            {
                val *= cur_sample / m_params->m_lfoAttackFrames;
            }
        }
        real_t cur_y = -LFO_GRAPH_H / 2.0f * val;
        p.drawLine(QLineF(graph_x_base + x - 1, graph_y_base + old_y,
                          graph_x_base + x, graph_y_base + cur_y));
        old_y = cur_y;
    }
    m_params->m_userWave.dataUnlock();

    QString unit = "ms";
    int     prec = 0;
    real_t  t    = EnvelopeAndLfoParameters::SECS_PER_LFO_OSCILLATION
               * m_params->m_lfoSpeedModel.value();

    if(m_params->m_x100Model.value())
        t /= 100.;

    uint32_t f = t * Engine::mixer()->baseSampleRate();
    t *= 1000.;

    if(t < 5.)
    {
        t *= 1000.;
        unit = "µs";
        prec = 0;
    }
    else if(t >= 500.)
    {
        t /= 1000.;
        unit = "s";
        prec = 3;
    }
    else
    {
        t    = roundf(t * 2.) / 2.;
        prec = 1;
    }

    /*
    p.setPen( QColor( 201, 201, 225 ) );
    p.drawText( LFO_GRAPH_X + 4, LFO_GRAPH_Y + m_lfoGraph.height() - 6,
                                                    tr( "ms/LFO:" ) );
    p.drawText( LFO_GRAPH_X + 52, LFO_GRAPH_Y + m_lfoGraph.height() - 6,
                                            QString::number( ms_per_osc ) );
    */

    s = QString("%1 %2/LFO, %3 frames").arg(t, 0, 'f', prec).arg(unit).arg(f);
    p.setFont(pointSize<6>(p.font()));
    p.setPen(Qt::black);
    p.drawText(LFO_GRAPH_X + 4, LFO_GRAPH_Y + m_lfoGraph.height() - 2, s);
    p.setPen(Qt::white);
    p.drawText(LFO_GRAPH_X + 3, LFO_GRAPH_Y + m_lfoGraph.height() - 3, s);
}

void EnvelopeAndLfoView::lfoUserWaveChanged()
{
    if(m_params->m_lfoWaveIndexModel.value()
       == EnvelopeAndLfoParameters::UserDefinedWave)
    {
        if(m_params->m_userWave.frames() <= 1)
        {
            TextFloat::displayMessage(
                    tr("Hint"),
                    tr("Drag a sample from somewhere and drop "
                       "it in this window."),
                    embed::getIconPixmap("hint"), 3000);
        }
    }
}
