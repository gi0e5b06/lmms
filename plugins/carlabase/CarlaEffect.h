/*
 * Carla for LMMS
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

#ifndef CARLA_EFFECT_H
#define CARLA_EFFECT_H

#include "CarlaNative.h"
#include "EffectControls.h"
#include "MidiEvent.h"
#include "MidiEventProcessor.h"
#include "Plugin.h"

#include <QMutex>
#include <QStringListModel>
//#include "CarlaEffectDialog.h"
class CarlaEffectControls;
class CarlaEffectDialog;
class CarlaEffectParamsView;

class PLUGIN_EXPORT CarlaEffect :
      public Effect  // , public MidiEventProcessor
{
    Q_OBJECT

  public:
    static const uint32_t kMaxMidiEvents = 512;

    CarlaEffect(Model*                          parent,
                const Plugin::Descriptor* const descriptor,
                const bool                      isPatchbay);
    virtual ~CarlaEffect();

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
    // virtual Flags flags() const;
    // virtual void saveSettings(QDomDocument& doc, QDomElement& parent);
    // virtual void loadSettings(const QDomElement& elem);
    // virtual QString nodeName() const;
    // virtual void play(sampleFrame* workingBuffer);
    virtual bool handleMidiEvent(const MidiEvent& event,
                                 const MidiTime&  time,
                                 f_cnt_t          offset);

    virtual bool processAudioBuffer(sampleFrame* _buf, const fpp_t _frames);

    virtual EffectControls* controls();

    // PluginView* createNativeView(QWidget* parent);

    virtual bool hasMidiOut()
    {
        return false;
    }

    virtual void processInEvent(const MidiEvent& event,
                                const MidiTime&  time   = MidiTime(),
                                f_cnt_t          offset = 0)
    {
        qInfo("CarlaEffect: receive midi event %d", event.type());
        handleMidiEvent(event, time, offset);
    }

    virtual void processOutEvent(const MidiEvent& event,
                                 const MidiTime&  time   = MidiTime(),
                                 f_cnt_t          offset = 0)
    {
        qInfo("CarlaEffect: processOutEvent not implemented");
        // propagateMidiOutEvent(event, time, offset);
    }

  signals:
    void uiClosed();

  private slots:
    void sampleRateChanged();
    void refreshParams(bool valuesOnly, const QDomElement* elem = nullptr);
    void clearParamModels();
    void paramModelChanged(uint32_t index);

  private:
    CarlaEffectControls* m_gdxControls;

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
    QStringList        m_completerList;

    friend class CarlaEffectControls;
    friend class CarlaEffectDialog;
    friend class CarlaEffectParamsView;
};

#endif
