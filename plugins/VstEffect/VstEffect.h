/*
 * VstEffect.h - class for handling VST effect plugins
 *
 * Copyright (c) 2019      gi0e5b06 (on github.com)
 * Copyright (c) 2006-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef _VST_EFFECT_H
#define _VST_EFFECT_H

#include "Effect.h"
#include "MidiEventProcessor.h"
#include "VstEffectControlDialog.h"
#include "VstEffectControls.h"
#include "VstPlugin.h"

#include <QMutex>

class VstEffect : public Effect  // , public MidiEventProcessor
{
  public:
    VstEffect(Model* _parent, const Descriptor::SubPluginFeatures::Key* _key);
    virtual ~VstEffect();

    virtual bool handleMidiEvent(const MidiEvent& event,
                                 const MidiTime&  time,
                                 f_cnt_t          offset);

    virtual bool processAudioBuffer(sampleFrame* _buf, const fpp_t _frames);

    virtual EffectControls* controls()
    {
        return &m_vstControls;
    }

    virtual inline QString publicName() const
    {
        return m_plugin->name();
    }

    virtual QString displayName() const
    {
        return Model::displayName();
    }

    virtual bool hasMidiOut()
    {
        return false;
    }

    virtual void processInEvent(const MidiEvent& event,
                                const MidiTime&  time   = MidiTime(),
                                f_cnt_t          offset = 0)
    {
        qInfo("VstEffect: receive midi event %d", event.type());
        handleMidiEvent(event, time, offset);
    }

    virtual void processOutEvent(const MidiEvent& event,
                                 const MidiTime&  time   = MidiTime(),
                                 f_cnt_t          offset = 0)
    {
        qInfo("VstEffect: processOutEvent not implemented");
        // propagateMidiOutEvent(event, time, offset);
    }

  private:
    void openPlugin(const QString& _plugin);
    void closePlugin();

    VstPlugin* m_plugin;
    QMutex     m_pluginMutex;
    EffectKey  m_key;

    VstEffectControls m_vstControls;

    friend class VstEffectControls;
    friend class VstEffectControlDialog;
    friend class manageVSTEffectView;
};

#endif
