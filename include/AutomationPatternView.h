/*
 * AutomationPatternView.h - declaration of class AutomationPatternView
 *
 * Copyright (c) 2008-2010 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef AUTOMATION_PATTERN_VIEW_H
#define AUTOMATION_PATTERN_VIEW_H

#include "Track.h"

#include <QStaticText>

class AutomationPattern;

class AutomationPatternView : public TileView
{
    Q_OBJECT

  public:
    AutomationPatternView(AutomationPattern* _pat, TrackView* _parent);
    virtual ~AutomationPatternView();

  public slots:
    /// Opens this view's pattern in the global automation editor
    void         openInAutomationEditor();
    virtual void update();

  protected slots:
    // void resetName();
    // void changeName();
    void disconnectObject(QAction* _a);
    void toggleRecording();
    void flipY();
    void flipX();

  protected:
    virtual QMenu* buildContextMenu();

    virtual void mouseDoubleClickEvent(QMouseEvent* me);
    virtual void paintEvent(QPaintEvent* pe);
    virtual void dragEnterEvent(QDragEnterEvent* _dee);
    virtual void dropEvent(QDropEvent* _de);

  private:
    AutomationPattern* m_pat;
    QPixmap            m_paintPixmap;
    QPixmap            m_pat_rec;
    QStaticText        m_staticTextName;

    void scaleTimemapToFit(real_t oldMin, real_t oldMax);
};

#endif
