/*
 * FadeButton.h -
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
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

#ifndef FADE_BUTTON_H
#define FADE_BUTTON_H

#include "PaintCacheable.h"

#include <QAbstractButton>
#include <QColor>
#include <QTime>

class FadeButton : public QAbstractButton, virtual public PaintCacheable
{
    Q_OBJECT

  public:
    FadeButton(const QColor& _normal_color,
               const QColor& _activated_color,
               QWidget*      _parent);

    virtual ~FadeButton();
    void setActiveColor(const QColor& activated_color);

    using PaintCacheable::update;
    virtual void updateNow();

  public slots:
    void activate();

  protected:
    virtual void drawWidget(QPainter& _p);
    virtual void paintEvent(QPaintEvent* _pe);

  private:
    QTime  m_stateTimer;
    QColor m_normalColor;
    QColor m_activatedColor;
};

#endif
