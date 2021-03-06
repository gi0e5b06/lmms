/*
 * EnvelopeAndLfoView.h - declaration of class EnvelopeAndLfoView which
 *                        is used by envelope/lfo/filter-tab of instrument
 * track
 *
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef ENVELOPE_AND_LFO_VIEW_H
#define ENVELOPE_AND_LFO_VIEW_H

#include "ModelView.h"

#include <QWidget>

class QPaintEvent;
class QPixmap;

class EnvelopeAndLfo;

// class AutomatableButtonGroup;
class ComboBox;
class Knob;
class LedCheckBox;
class PixmapButton;
class TempoSyncKnob;

class EnvelopeAndLfoView : public QWidget, public ModelView
{
    Q_OBJECT
  public:
    EnvelopeAndLfoView(EnvelopeAndLfo* _model, QWidget* _parent);
    virtual ~EnvelopeAndLfoView();

    EnvelopeAndLfo* model()
    {
        return castModel<EnvelopeAndLfo>();
    }

    const EnvelopeAndLfo* model() const
    {
        return castModel<EnvelopeAndLfo>();
    }

  public slots:
    virtual void update();

  protected:
    virtual void modelChanged();

    virtual void dragEnterEvent(QDragEnterEvent* _dee);
    virtual void dropEvent(QDropEvent* _de);
    virtual void mousePressEvent(QMouseEvent* _me);
    virtual void paintEvent(QPaintEvent* _pe);

  protected slots:
    void lfoUserWaveChanged();

  private:
    QPixmap m_envGraph;
    QPixmap m_lfoGraph;

    EnvelopeAndLfo* m_params;

    // envelope stuff
    TempoSyncKnob* m_predelayKnob;
    TempoSyncKnob* m_attackKnob;
    TempoSyncKnob* m_holdKnob;
    TempoSyncKnob* m_decayKnob;
    Knob*          m_sustainKnob;
    TempoSyncKnob* m_releaseKnob;
    Knob*          m_amountKnob;

    // LFO stuff
    Knob*          m_lfoPredelayKnob;
    Knob*          m_lfoAttackKnob;
    TempoSyncKnob* m_lfoSpeedKnob;
    Knob*          m_lfoAmountKnob;
    PixmapButton*  m_userLfoBtn;
    // AutomatableButtonGroup * m_lfoWaveBtnGrp;
    ComboBox* m_lfoWaveBankCMB;
    ComboBox* m_lfoWaveIndexCMB;

    LedCheckBox* m_x100Cb;
    LedCheckBox* m_controlEnvAmountCb;
    Knob*        m_outKnob;
    // float m_randomGraph;
};

#endif
