/*
 * InstrumentFunctionView.h - views for instrument-functions-tab
 *
 * Copyright (c) 2017-2018 gi0e5b06
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

#ifndef INSTRUMENT_FUNCTION_VIEW_H
#define INSTRUMENT_FUNCTION_VIEW_H

#include <QWidget>

#include "InstrumentFunction.h"
#include "ModelView.h"

class QLabel;
class ComboBox;
class GroupBox;
class Knob;
class TempoSyncKnob;
class LedCheckBox;

class InstrumentFunctionView
      : public QWidget
      , public ModelView
{
    Q_OBJECT

  public:
    virtual ~InstrumentFunctionView();

  protected:
    InstrumentFunctionView(InstrumentFunction* cc,
                           const QString&      _caption,
                           QWidget*            _parent = NULL);

    GroupBox* m_groupBox;
};

class InstrumentFunctionNoteStackingView : public InstrumentFunctionView
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteStackingView(InstrumentFunctionNoteStacking* cc,
                                       QWidget* parent = NULL);
    virtual ~InstrumentFunctionNoteStackingView();

  public slots:
    virtual void modelChanged();

  private:
    InstrumentFunctionNoteStacking* m_cc;
    ComboBox*                       m_chordsComboBox;
    Knob*                           m_chordRangeKnob;
};

class InstrumentFunctionArpeggioView : public InstrumentFunctionView
{
    Q_OBJECT

  public:
    InstrumentFunctionArpeggioView(InstrumentFunctionArpeggio* arp,
                                   QWidget*                    parent = NULL);
    virtual ~InstrumentFunctionArpeggioView();

  public slots:
    virtual void modelChanged();

  private:
    InstrumentFunctionArpeggio* m_cc;
    ComboBox*                   m_arpComboBox;
    Knob*                       m_arpRangeKnob;
    Knob*                       m_arpCycleKnob;
    Knob*                       m_arpSkipKnob;
    Knob*                       m_arpMissKnob;
    TempoSyncKnob*              m_arpTimeKnob;
    Knob*                       m_arpGateKnob;

    ComboBox* m_arpDirectionComboBox;
    ComboBox* m_arpModeComboBox;
};

class InstrumentFunctionNoteHumanizingView : public InstrumentFunctionView
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteHumanizingView(InstrumentFunctionNoteHumanizing* cc,
                                         QWidget* parent = NULL);
    virtual ~InstrumentFunctionNoteHumanizingView();

  public slots:
    virtual void modelChanged();

  private:
    InstrumentFunctionNoteHumanizing* m_cc;
    Knob*                             m_volumeRangeKnob;
    Knob*                             m_panRangeKnob;
    Knob*                             m_tuneRangeKnob;
    Knob*                             m_offsetRangeKnob;
    Knob*                             m_shortenRangeKnob;
};

class InstrumentFunctionNoteDuplicatesRemovingView
      : public InstrumentFunctionView
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteDuplicatesRemovingView(
            InstrumentFunctionNoteDuplicatesRemoving* cc,
            QWidget*                                  parent = NULL);
    virtual ~InstrumentFunctionNoteDuplicatesRemovingView();

  public slots:
    virtual void modelChanged();

  private:
    InstrumentFunctionNoteDuplicatesRemoving* m_cc;
};

class InstrumentFunctionNoteFilteringView : public InstrumentFunctionView
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteFilteringView(InstrumentFunctionNoteFiltering* cc,
                                        QWidget* parent = NULL);
    virtual ~InstrumentFunctionNoteFilteringView();

  public slots:
    virtual void modelChanged();

  private:
    InstrumentFunctionNoteFiltering* m_cc;
    ComboBox*                        m_configComboBox;
    ComboBox*                        m_actionComboBox;
    LedCheckBox*                     m_noteSelectionLed[12];
};

class InstrumentFunctionNoteKeyingView : public InstrumentFunctionView
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteKeyingView(InstrumentFunctionNoteKeying* cc,
                                     QWidget* parent = NULL);
    virtual ~InstrumentFunctionNoteKeyingView();

  public slots:
    virtual void modelChanged();

  private:
    InstrumentFunctionNoteKeying* m_cc;
    Knob*                         m_volumeRangeKnob;
    Knob*                         m_volumeBaseKnob;
    Knob*                         m_volumeMinKnob;
    Knob*                         m_volumeMaxKnob;
    Knob*                         m_panRangeKnob;
    Knob*                         m_panBaseKnob;
    Knob*                         m_panMinKnob;
    Knob*                         m_panMaxKnob;
};

class InstrumentFunctionNoteOuttingView : public InstrumentFunctionView
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteOuttingView(InstrumentFunctionNoteOutting* cc,
                                      QWidget* parent = NULL);
    virtual ~InstrumentFunctionNoteOuttingView();

  public slots:
    virtual void modelChanged();

  private:
    InstrumentFunctionNoteOutting* m_cc;
    Knob*                          m_keyKnob;
    Knob*                          m_volumeKnob;
    Knob*                          m_panKnob;
};

class InstrumentFunctionGlissandoView : public InstrumentFunctionView
{
    Q_OBJECT

  public:
    InstrumentFunctionGlissandoView(InstrumentFunctionGlissando* arp,
                                    QWidget* parent = NULL);
    virtual ~InstrumentFunctionGlissandoView();

  public slots:
    virtual void modelChanged();

  private:
    InstrumentFunctionGlissando* m_cc;
    TempoSyncKnob*               m_gliTimeKnob;
    Knob*                        m_gliGateKnob;
    Knob*                        m_gliAttenuationKnob;
    ComboBox*                    m_gliUpModeComboBox;
    ComboBox*                    m_gliDownModeComboBox;
};

#endif
