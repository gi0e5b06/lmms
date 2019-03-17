/*
 * MidiEventProcessor.h - base-class for midi-processing classes
 *
 * Copyright (c) 2019      gi0e5b06 (on github.com)
 * Copyright (c) 2005-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef MIDI_EVENT_PROCESSOR_H
#define MIDI_EVENT_PROCESSOR_H

// #include "MemoryManager.h"
#include "MidiEvent.h"
#include "MidiTime.h"
#include "Model.h"
#include "SafeList.h"

// All classes being able to process MIDI-events should inherit from this
class MidiEventProcessor
{
    // MM_OPERATORS

  public:
    MidiEventProcessor()
    {
    }

    virtual ~MidiEventProcessor()
    {
    }

    virtual bool hasMidiIn()
    {
        return true;
    }

    virtual bool hasMidiOut()
    {
        return true;
    }

    // to be implemented by inheriting classes
    virtual void processInEvent(const MidiEvent& event,
                                const MidiTime&  time   = MidiTime(),
                                f_cnt_t          offset = 0)
            = 0;
    virtual void processOutEvent(const MidiEvent& event,
                                 const MidiTime&  time   = MidiTime(),
                                 f_cnt_t          offset = 0)
            = 0;

    virtual void linkMidiOutProcessor(QString _uuid) final
    {
        if(!detectLoop(_uuid))
            m_midiOutProcessors.append(_uuid);
    }

    virtual void unlinkMidiOutProcessor(QString _uuid) final
    {
        m_midiOutProcessors.removeOne(_uuid);
    }

  protected:
    virtual void propagateMidiOutEvent(const MidiEvent& event,
                                       const MidiTime&  time   = MidiTime(),
                                       f_cnt_t          offset = 0) final
    {
        m_midiOutProcessors.map([event, time, offset](QString _uuid) {
            MidiEventProcessor* p
                    = dynamic_cast<MidiEventProcessor*>(Model::find(_uuid));
            qInfo("sending event to %s (%p)", qPrintable(_uuid), p);
            if(p != nullptr)
                p->processInEvent(event, time, offset);
        });
    }

    virtual bool detectLoop(QString _uuid) final
    {
        QStringList a;
        a.append(_uuid);
        return detectLoop(a);
    }

    virtual bool detectLoop(QStringList& _a) final
    {
        Model* m = dynamic_cast<Model*>(this);
        if(m == nullptr)
            return false;
        QString u = m->uuid();
        if(_a.contains(u))
        {
            return true;
        }
        else
        {
            _a.append(u);
            bool r = false;
            m_midiOutProcessors.map([&r, &_a](QString _uuid) {
                if(r == false)
                {
                    Model*              m = Model::find(_uuid);
                    MidiEventProcessor* p
                            = dynamic_cast<MidiEventProcessor*>(m);
                    r = p->detectLoop(_a);
                }
            });
            return r;
        }
    }

  private:
    SafeList<QString> m_midiOutProcessors;
};

#endif
