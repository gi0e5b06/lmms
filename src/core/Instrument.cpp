/*
 * Instrument.cpp - base-class for all instrument-plugins (synths, samplers
 * etc)
 *
 * Copyright (c) 2005-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "Instrument.h"

#include "DummyInstrument.h"
#include "InstrumentTrack.h"
#include <QCoreApplication>

Instrument::Instrument(InstrumentTrack*  _instrumentTrack,
                       const Descriptor* _descriptor) :
      Plugin(_descriptor, nullptr /* _instrumentTrack*/),
      m_instrumentTrack(_instrumentTrack), m_okay(true), m_noRun(false)
{
}

Instrument::~Instrument()
{
    qInfo("Instrument::~Instrument [%s] START", qPrintable(displayName()));

    Engine::mixer()->emit playHandlesForInstrumentToRemove(this);
    qInfo("Instrument::~Instrument wait");
    QCoreApplication::sendPostedEvents();
    // QThread::yieldCurrentThread();
    Engine::mixer()->waitUntilNoPlayHandle(this);

    m_instrumentTrack = nullptr;
    qInfo("Instrument::~Instrument [%s] END", qPrintable(displayName()));
}

void Instrument::play(sampleFrame*)
{
}

void Instrument::deleteNotePluginData(NotePlayHandle*)
{
}

f_cnt_t Instrument::beatLen(NotePlayHandle*) const
{
    return 0;
}

Instrument* Instrument::instantiate(const QString&   _pluginName,
                                    InstrumentTrack* _instrumentTrack)
{
    Plugin* p = Plugin::instantiate(_pluginName, _instrumentTrack,
                                    _instrumentTrack);
    // check whether instantiated plugin is an instrument
    if(dynamic_cast<Instrument*>(p) != nullptr)
    {
        // everything ok, so return pointer
        return dynamic_cast<Instrument*>(p);
    }

    // not quite... so delete plugin and return dummy instrument
    delete p;
    return new DummyInstrument(_instrumentTrack);
}

bool Instrument::isFromTrack(const Track* _track) const
{
    // qWarning("Instrument::isFromTrack %p %p",m_instrumentTrack,_track);
    return m_instrumentTrack != nullptr && m_instrumentTrack == _track;
}

void Instrument::applyRelease(sampleFrame* buf, const NotePlayHandle* _n)
{
    const fpp_t   frames = _n->framesLeftForCurrentPeriod();
    const fpp_t   fpp    = Engine::mixer()->framesPerPeriod();
    const f_cnt_t fl     = _n->framesLeft();
    if(fl <= desiredReleaseFrames() + fpp)
    {
        for(fpp_t f = (fpp_t)((fl > desiredReleaseFrames())
                                      ? (qMax(fpp - desiredReleaseFrames(), 0)
                                         + fl % fpp)
                                      : 0);
            f < frames; ++f)
        {
            const float fac = (float)(fl - f - 1) / desiredReleaseFrames();
            for(ch_cnt_t ch = 0; ch < DEFAULT_CHANNELS; ++ch)
            {
                buf[f][ch] *= fac;
            }
        }
    }
}

QString Instrument::fullDisplayName() const
{
    return instrumentTrack()->displayName() + ">" + displayName();
}
