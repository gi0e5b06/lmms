/*
 * AutomationTrack.cpp - AutomationTrack handles automation of objects without
 *                       a track
 *
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2006-2008 Javier Serrano Polo
 * <jasp00/at/users.sourceforge.net>
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

#include "AutomationTrack.h"

#include "AutomationPattern.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "ProjectJournal.h"
#include "StringPairDrag.h"
#include "TrackContainerView.h"
#include "TrackLabelButton.h"
#include "embed.h"

AutomationTrack::AutomationTrack(TrackContainer* tc, bool _hidden) :
      Track(_hidden ? HiddenAutomationTrack : Track::AutomationTrack, tc)
{
    setName(tr("Automation track"));
    setColor(QColor("#5F30A4"));
}

AutomationTrack::~AutomationTrack()
{
    qInfo("AutomationTrack::~AutomationTrack");
    if(type() == HiddenAutomationTrack)
        qInfo("~AutomationTrack HIDDEN");
}

QString AutomationTrack::defaultName() const
{
    QString r = "";
    for(const Tile* tco: getTCOs())
        if(!tco->isEmpty())
        {
            r = tco->name();
            r = r.right(r.indexOf(QChar('>')));
            break;
        }
    if(r.isEmpty())
        r = tr("Automation track");
    return r;
}

bool AutomationTrack::play(const MidiTime& time_start,
                           const fpp_t     _frames,
                           const f_cnt_t   _frame_base,
                           int             _tco_num)
{
    return false;
}

TrackView* AutomationTrack::createView(TrackContainerView* tcv)
{
    return new AutomationTrackView(this, tcv);
}

Tile* AutomationTrack::createTCO()
{
    return new AutomationPattern(this);
}

void AutomationTrack::saveTrackSpecificSettings(QDomDocument& _doc,
                                                QDomElement&  _this)
{
}

void AutomationTrack::loadTrackSpecificSettings(const QDomElement& _this)
{
    // just in case something somehow wrent wrong...
    if(type() == HiddenAutomationTrack)
    {
        setMuted(false);
    }
}

AutomationTrackView::AutomationTrackView(AutomationTrack*    _at,
                                         TrackContainerView* tcv) :
      TrackView(_at, tcv)
{
    setFixedHeight(32);
    TrackLabelButton* tlb
            = new TrackLabelButton(this, getTrackSettingsWidget());
    tlb->setIcon(embed::getIcon("automation_track"));
    tlb->move(3, 1);
    tlb->show();
    connect(tlb, SIGNAL(clicked(bool)), this, SLOT(clickedTrackLabel()));
    // setModel(_at);
}

AutomationTrackView::~AutomationTrackView()
{
}

void AutomationTrackView::addSpecificMenu(QMenu* _cm, bool _enabled)
{
    _cm->addAction(tr("Turn all recording on"), this, SLOT(recordingOn()));
    _cm->addAction(tr("Turn all recording off"), this, SLOT(recordingOff()));
}

void AutomationTrackView::recordingOn()
{
    const Tiles& tcov = track()->getTCOs();
    for(Tiles::const_iterator it = tcov.begin(); it != tcov.end(); ++it)
    {
        AutomationPattern* ap = qobject_cast<AutomationPattern*>(*it);
        if(ap != nullptr)

            ap->setRecording(true);
    }
    update();
}

void AutomationTrackView::recordingOff()
{
    const Tiles& tcov = track()->getTCOs();
    for(Tiles::const_iterator it = tcov.begin(); it != tcov.end(); ++it)
    {
        AutomationPattern* ap = qobject_cast<AutomationPattern*>(*it);
        if(ap != nullptr)
            ap->setRecording(false);
    }
    update();
}

void AutomationTrackView::dragEnterEvent(QDragEnterEvent* _dee)
{
    StringPairDrag::processDragEnterEvent(_dee, "automatable_model");
}

void AutomationTrackView::dropEvent(QDropEvent* _de)
{
    QString type = StringPairDrag::decodeKey(_de);
    QString val  = StringPairDrag::decodeValue(_de);
    if(type == "automatable_model")
    {
        _de->accept();
        AutomatableModel* mod = dynamic_cast<AutomatableModel*>(
                Engine::projectJournal()->journallingObject(val.toInt()));
        if(mod != nullptr)
        {
            MidiTime pos
                    = MidiTime(trackContainerView()->currentPosition()
                               + (_de->pos().x()
                                  - getTrackContentWidget()->x())
                                         * MidiTime::ticksPerTact()
                                         / static_cast<int>(
                                                 trackContainerView()
                                                         ->pixelsPerTact()))
                              .toAbsoluteTact();

            if(pos.getTicks() < 0)
            {
                pos.setTicks(0);
            }

            Tile*              tco = track()->createTCO();
            AutomationPattern* pat = dynamic_cast<AutomationPattern*>(tco);
            pat->addObject(mod);
            pat->movePosition(pos);

            // mod->emit propertiesChanged();
            mod->emit
                    dataChanged();  // controlledValueChanged(mod->currentValue(0));
        }
    }

    update();
}

void AutomationTrackView::clickedTrackLabel()
{
    gui->mainWindow()->toggleAutomationWindow();
}
