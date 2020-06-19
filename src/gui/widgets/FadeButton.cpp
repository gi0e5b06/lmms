/*
 * FadeButton.cpp -
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

#include "FadeButton.h"

#include "Configuration.h"
#include "embed.h"

#include <QApplication>
#include <QPainter>
#include <QTimer>

const float FadeDuration = 500;

FadeButton::FadeButton(const QColor& _normal_color,
                       const QColor& _activated_color,
                       QWidget*      _parent) :
      QAbstractButton(_parent),
      m_stateTimer(), m_normalColor(_normal_color),
      m_activatedColor(_activated_color)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    // setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

FadeButton::~FadeButton()
{
}

void FadeButton::setActiveColor(const QColor& activated_color)
{
    m_activatedColor = activated_color;
}

void FadeButton::activate()
{
    m_stateTimer.restart();
    // signalUpdate();
    update();
}

void FadeButton::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    drawWidget(p);
}

void FadeButton::drawWidget(QPainter& _p)
{
    QColor col = m_normalColor;
    if(!m_stateTimer.isNull() && m_stateTimer.elapsed() < FadeDuration)
    {
        const float state = 1 - m_stateTimer.elapsed() / FadeDuration;
        const int   r     = (int)(m_normalColor.red() * (1.0f - state)
                            + m_activatedColor.red() * state);
        const int   g     = (int)(m_normalColor.green() * (1.0f - state)
                            + m_activatedColor.green() * state);
        const int   b     = (int)(m_normalColor.blue() * (1.0f - state)
                            + m_activatedColor.blue() * state);
        col.setRgb(r, g, b);
        QTimer::singleShot(1000 / CONFIG_GET_INT("ui.framespersecond"), this,
                           SLOT(update()));
        // fps
    }

    int w = width() - 1;
    int h = height() - 1;
    //_p.fillRect(0,0,w,h,Qt::yellow);
    _p.fillRect(1, 1, w - 1, h - 1, col);

    _p.setPen(m_normalColor.darker(130));
    _p.drawLine(w, 1, w, h);
    _p.drawLine(1, h, w, h);
    _p.setPen(m_normalColor.lighter(130));
    _p.drawLine(0, 0, 0, h - 1);
    _p.drawLine(0, 0, w, 0);
}

void FadeButton::updateNow()
{
    QAbstractButton::update();
}
