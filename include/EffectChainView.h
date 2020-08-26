/*
 * EffectChainView.h -
 *
 * Copyright (c) 2006-2007 Danny McRae <khjklujn@netscape.net>
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

#ifndef EFFECT_CHAIN_VIEW_H
#define EFFECT_CHAIN_VIEW_H

#include "EffectChain.h"
#include "GroupBox.h"
#include "ModelView.h"

#include <QMouseEvent>
#include <QPointer>
#include <QWidget>

//#include "lmms_basics.h"

class QScrollArea;
class QVBoxLayout;

class EffectView;
class GroupBox;

class EffectChainView : public GroupBox, public ModelView
{
    Q_OBJECT

  public:
    EffectChainView(EffectChain* model,
                    QWidget*     parent = nullptr,
                    QString      _title = "");
    virtual ~EffectChainView();

  public slots:
    void clearViews();
    void moveUp(EffectView* view);
    void moveDown(EffectView* view);
    void moveTop(EffectView* view);
    void moveBottom(EffectView* view);
    void removeEffect(EffectView* view);

  protected:
    virtual void addEffect(Effect* _effect);

    void mousePressEvent(QMouseEvent* _me) override;
    void mouseReleaseEvent(QMouseEvent* _me) override;

  private slots:
    virtual void update();
    virtual void addEffect();

  private:
    virtual void modelChanged();

    INLINE EffectChain* model()
    {
        return castModel<EffectChain>();
    }
    INLINE const EffectChain* model() const
    {
        return castModel<EffectChain>();
    }

    QVector<QPointer<EffectView>> m_effectViews;
    QScrollArea*                  m_scrollArea;
    int                           m_lastY;
};

#endif
