/*
 * TrackContainer.h - base-class for all track-containers like Song-Editor,
 *                    BB-Editor...
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

#ifndef TRACK_CONTAINER_H
#define TRACK_CONTAINER_H

#include "JournallingObject.h"
#include "Track.h"

#include <QReadWriteLock>

class AutomationPattern;
class InstrumentTrack;
class TrackContainerView;

class EXPORT TrackContainer : public Model, public JournallingObject
{
    Q_OBJECT
  public:
    enum TrackContainerTypes
    {
        BBContainer,
        SongContainer
    };

    TrackContainer(Model*         _parent,
                   const QString& _displayName,
                   const QString& _objectName);
    virtual ~TrackContainer();

    void saveSettings(QDomDocument& _doc, QDomElement& _parent) override;
    void loadSettings(const QDomElement& _this) override;

    virtual bool isFixed() const
    {
        return false;
    }

    virtual AutomationPattern* tempoAutomationPattern()
    {
        return nullptr;
    }

    virtual bool hasTracks() const final;
    virtual int  countTracks(Track::TrackType _tt) const final;
    // = Track::NumTrackType) const;

    virtual void addTrack(Track* _track) final;
    virtual void removeTrack(Track* _track) final;

    virtual void clearAllTracks() final;

    virtual const Tracks& tracks() const final
    {
        return m_tracks;
    }

    virtual bool isEmpty() const final;

    static const QString classNodeName()
    {
        return "trackcontainer";
    }

    INLINE virtual TrackContainerTypes type() const final
    {
        return m_TrackContainerType;
    }

    INLINE virtual void setType(TrackContainerTypes newType) final
    {
        m_TrackContainerType = newType;
    }

    virtual void automatedValuesAt(MidiTime           time,
                                   int                tcoNum /*= -1*/,
                                   AutomatedValueMap& _map) const;

  signals:
    void trackAdded(Track* _track);
    void trackRemoved();

  protected:
    static void automatedValuesFromTracks(const Tracks&      tracks,
                                          MidiTime           timeStart,
                                          int                tcoNum /*= -1*/,
                                          AutomatedValueMap& _map);
    static void automatedValuesFromTrack(const Track*       _track,
                                         MidiTime           timeStart,
                                         int                tcoNum,
                                         AutomatedValueMap& _map);

    mutable QReadWriteLock m_tracksMutex;

  private:
    Tracks m_tracks;

    TrackContainerTypes m_TrackContainerType;

    friend class TrackContainerView;
    friend class Track;
};

class DummyTrackContainer : public TrackContainer
{
  public:
    DummyTrackContainer();
    virtual ~DummyTrackContainer();

    QString nodeName() const override
    {
        return "DummyTrackContainer";
    }

    InstrumentTrack* dummyInstrumentTrack()
    {
        return m_dummyInstrumentTrack;
    }

  private:
    InstrumentTrack* m_dummyInstrumentTrack;
};

#endif
