/*
 * carla.h - Carla for LMMS
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

#ifndef CARLA_H
#define CARLA_H

#include "CarlaNative.h"
#include "Instrument.h"
#include "InstrumentView.h"
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

class CarlaInstrumentParamsView;

//#define NB_MIDI_KNOBS 128
//#define NB_MIDI_LEDS        8
//#define NB_MIDI_LCDS        0
//#define NB_MIDI_KNOB_START 0
//#define NB_MIDI_LED_START  60
//#define NB_MIDI_LCD_START  70
//#define MIDI_CH 1

class PLUGIN_EXPORT CarlaInstrument : public Instrument
{
    Q_OBJECT

  public:
    static const uint32_t kMaxMidiEvents = 512;

    CarlaInstrument(InstrumentTrack* const  instrumentTrack,
                    const Descriptor* const descriptor,
                    const bool              isPatchbay);
    virtual ~CarlaInstrument();

    // CarlaNative functions
    uint32_t              handleGetBufferSize() const;
    double                handleGetSampleRate() const;
    bool                  handleIsOffline() const;
    const NativeTimeInfo* handleGetTimeInfo() const;
    void                  handleUiParameterChanged(const uint32_t index,
                                                   const float    value) const;
    void                  handleUiClosed();
    intptr_t handleDispatcher(const NativeHostDispatcherOpcode opcode,
                              const int32_t                    index,
                              const intptr_t                   value,
                              void* const                      ptr,
                              const float                      opt);

    // LMMS functions
    virtual Flags   flags() const;
    virtual f_cnt_t desiredReleaseFrames() const;
    // virtual QString     nodeName() const;
    virtual void        saveSettings(QDomDocument& doc, QDomElement& parent);
    virtual void        loadSettings(const QDomElement& elem);
    virtual void        play(sampleFrame* workingBuffer);
    virtual bool        handleMidiEvent(const MidiEvent& event,
                                        const MidiTime&  time,
                                        f_cnt_t          offset);
    virtual PluginView* instantiateView(QWidget* parent);

  signals:
    void uiClosed();

  private slots:
    void sampleRateChanged();
    void refreshParams(bool valuesOnly, const QDomElement* elem = nullptr);
    void clearParamModels();
    void paramModelChanged(uint32_t index);

  private:
    const bool kIsPatchbay;

    NativePluginHandle            fHandle;
    NativeHostDescriptor          fHost;
    const NativePluginDescriptor* fDescriptor;

    uint32_t        fMidiEventCount;
    NativeMidiEvent fMidiEvents[kMaxMidiEvents];
    NativeTimeInfo  fTimeInfo;

    // this is only needed because note-offs are being sent during play
    QMutex fMutex;

    QList<FloatModel*> paramModels;
    // QDomElement settingsElem;
    QStringList m_completerList;
    // QStringListModel m_completerModel;

    // FloatModel* m_midiKnobs[NB_MIDI_KNOBS];
    // BoolModel*   m_midiLeds [NB_MIDI_LEDS ];
    // IntModel*    m_midiLcds [NB_MIDI_LCDS ];

    friend class CarlaInstrumentView;
    friend class CarlaInstrumentParamsView;
};

class CarlaInstrumentView : public InstrumentView
{
    Q_OBJECT

  public:
    CarlaInstrumentView(CarlaInstrument* instrument, QWidget* parent);
    virtual ~CarlaInstrumentView();

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

    QPushButton* m_toggleUIButton;

    // Knob* m_midiKnobs[NB_MIDI_KNOBS];
    // LedCheckBox* m_midiLeds [NB_MIDI_LEDS ];
    // LcdSpinBox*  m_midiLcds [NB_MIDI_LCDS ];

    CarlaInstrument*           m_carlaInstrument;
    QPushButton*               m_toggleParamsWindowButton;
    CarlaInstrumentParamsView* m_paramsView;
    // QWidget*         p_parent;
};

class CarlaInstrumentParamsView : public QWidget  // InstrumentView
{
    Q_OBJECT
  public:
    CarlaInstrumentParamsView(CarlaInstrument* instrument, QWidget* parent);
    virtual ~CarlaInstrumentParamsView();

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
    CarlaInstrument* m_carlaInstrument;
    QCompleter*      m_paramCompleter;

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
