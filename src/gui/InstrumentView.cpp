/*
 * InstrumentView.cpp - base-class for views of all Instruments
 *
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "InstrumentView.h"

#include "InstrumentTrack.h"  // REQUIRED
//#include "StringPairDrag.h"

#include "embed.h"  // REQUIRED

#include <QIcon>  // REQUIRED

InstrumentView::InstrumentView(Instrument* _instrument, QWidget* _parent) :
      PluginView(_instrument, _parent)
{
    setFixedSize(250, 250);
    setAttribute(Qt::WA_DeleteOnClose, true);
    // setModel(_instrument);
    modelChanged();
}

InstrumentView::~InstrumentView()
{
    InstrumentTrackWindow* w = instrumentTrackWindow();
    if(w != nullptr)
        w->m_instrumentView = nullptr;
}

/*
void InstrumentView::setModel(Model* _model)
{
    if(dynamic_cast<Instrument*>(_model) != nullptr)
    {
        ModelView::setModel(_model);
        instrumentTrackWindow()->setWindowIcon(
                model()->descriptor()->logo()->pixmap());
        // connect(model(), SIGNAL(destroyed(QObject*)), this, SLOT(close()));
    }
}
*/

void InstrumentView::modelChanged()
{
    Instrument* m=model();

    if(m != nullptr)
        instrumentTrackWindow()->setWindowIcon(
                model()->descriptor()->logo()->pixmap());
}

InstrumentTrackWindow* InstrumentView::instrumentTrackWindow()
{
    return dynamic_cast<InstrumentTrackWindow*>(
            parentWidget()->parentWidget());
}

QLine InstrumentView::cableTo() const
{
    const QWidget* w = widget();
    if(w == nullptr)
        return QLine();

    QPoint p(w->width() / 2, -100);
    return QLine(p, p + QPoint(0, -50));
}

QLine InstrumentView::cableFrom() const
{
    const QWidget* w = widget();
    if(w == nullptr)
        return QLine();

    QPoint p(w->width() / 2, w->height());
    return QLine(p, p + QPoint(0, 50));
}

QColor InstrumentView::cableColor() const
{
    const Instrument*      m = model();
    const InstrumentTrack* t = m->instrumentTrack();
    return t->color();
}
