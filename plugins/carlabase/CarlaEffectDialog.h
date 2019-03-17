/*
 * Carla for LSMM
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (C) 2014      Filipe Coelho <falktx@falktx.com>
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

#ifndef CARLA_EFFECT_DIALOG_H
#define CARLA_EFFECT_DIALOG_H

#include "CarlaEffectControls.h"
#include "CarlaNative.h"
#include "EffectControlDialog.h"
#include "Knob.h"
#include "LcdSpinBox.h"
#include "LedCheckBox.h"
#include "SubWindow.h"

#include <QMutex>
#include <QStringListModel>

class QMessageBox;
class QCompleter;
class QScrollArea;
class QGridLayout;
class QHBoxLayout;
class QLineEdit;
class QPushButton;

class MidiProcessorView;
class CarlaEffectParamsView;

class CarlaEffectDialog : public EffectControlDialog
{
    Q_OBJECT

  public:
    CarlaEffectDialog(CarlaEffectControls* controls);
    virtual ~CarlaEffectDialog();

  private slots:
    void toggleUI(bool);
    void uiClosed();
    void onDataChanged();
    void toggleParamsWindow(bool);
    void paramsWindowClosed();

  private:
    virtual void modelChanged();
    virtual void timerEvent(QTimerEvent*);

    NativePluginHandle            fHandle;
    const NativePluginDescriptor* fDescriptor;
    int                           fTimerId;

    // Knob*        m_knobs[NB_KNOBS];
    // LedCheckBox* m_leds [NB_LEDS ];
    // LcdSpinBox*  m_lcds [NB_LCDS ];

    CarlaEffect*       m_carlaEffect;
    QPushButton*       m_toggleUIButton;
    QPushButton*       m_toggleParamsWindowButton;
    MidiProcessorView* m_midiInProc;
    // MidiProcessorView*     m_midiOutProc;
    CarlaEffectParamsView* m_paramsView;
    // QWidget*         p_parent;
};

class CarlaEffectParamsView : public QWidget  // InstrumentView
{
    Q_OBJECT
  public:
    CarlaEffectParamsView(CarlaEffect* effect, QWidget* parent);
    virtual ~CarlaEffectParamsView();

    SubWindow* subWindow()
    {
        return m_subWindow;
    }

  private slots:
    void onRefreshButton();
    void onRefreshValuesButton();
    void refreshKnobs();
    void filterKnobs();
    void clearFilterText();

  private:
    virtual void modelChanged();

    void addKnob(uint32_t index);
    void clearKnobs();

    SubWindow* m_subWindow;
    // QObject*         p_subWindow;
    CarlaEffect* m_carlaEffect;
    QCompleter*  m_paramCompleter;

    QList<Knob*> m_knobs;

    uint32_t lMaxColumns;
    uint32_t lCurColumn;
    uint32_t lCurRow;

    QScrollArea* m_scrollArea;
    QGridLayout* m_scrollAreaLayout;
    QWidget*     m_scrollAreaWidgetContent;
    QHBoxLayout* m_toolBarLayout;

    QPushButton* m_refreshParamsButton;
    QPushButton* m_refreshParamValuesButton;
    QLineEdit*   m_paramsFilterLineEdit;
    QPushButton* m_clearFilterButton;
    QPushButton* m_automatedOnlyButton;
};

#endif
