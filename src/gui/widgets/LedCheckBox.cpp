/*
 * LedCheckBox.cpp - class LedCheckBox, an improved QCheckBox
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "LedCheckBox.h"

#include "GuiApplication.h"
#include "MainWindow.h"
#include "embed.h"
#include "gui_templates.h"

#include <QFontMetrics>
#include <QInputDialog>
#include <QPainter>
//#include <QStyle>
#include <QStyleOption>
#include <QTime>

static const QString names[LedCheckBox::NumColors]
        = {"led_yellow", "led_green",   "led_red",
           "led_blue",   "led_magenta", "led_white"};

LedCheckBox::LedCheckBox(const QString& _text,
                         QWidget*       _parent,
                         const QString& _name,
                         LedColors      _color) :
      AutomatableButton(_parent, _name),
      m_blinkingState(true), m_blinking(false), m_text(_text),
      m_textAnchor(Qt::AnchorLeft)
{
    initUi(_color);
}

LedCheckBox::LedCheckBox(QWidget*       _parent,
                         const QString& _name,
                         LedColors      _color) :
      LedCheckBox("", _parent, _name, _color)
{
}

LedCheckBox::LedCheckBox(QWidget* _parent, LedColors _color) :
      LedCheckBox("", _parent, "[led checkbox]", _color)
{
}

LedCheckBox::~LedCheckBox()
{
    delete m_ledOnPixmap;
    delete m_ledOffPixmap;
}

QString LedCheckBox::text() const
{
    return m_text;
}

void LedCheckBox::setText(const QString& _t)
{
    if(m_text == _t)
        return;

    m_text = _t;
    onTextUpdated();
}

Qt::AnchorPoint LedCheckBox::textAnchorPoint() const
{
    return m_textAnchor;
}

void LedCheckBox::setTextAnchorPoint(Qt::AnchorPoint _a)
{
    if(m_textAnchor == _a)
        return;

    m_textAnchor = _a;
    onTextUpdated();
}

void LedCheckBox::update()
{
    AutomatableButton::update();
}

void LedCheckBox::paintEvent(QPaintEvent* _pe)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    BoolModel* m = model();
    if(m==nullptr)
        return;

    QMargins mw = contentsMargins();
    int      xl = 0;
    int      yl = 0;

    QString t = text();
    if(!t.isEmpty())
    {
        const Qt::AnchorPoint a = textAnchorPoint();

        const QFont ft = pointSizeF(
                font(),
                (a == Qt::AnchorLeft || a == Qt::AnchorRight) ? 8.5f : 6.5f);
        p.setFont(ft);

        // const QFontMetrics mx(pointSizeF(p.font(), 6.5));
        const QFontMetrics mx = p.fontMetrics();
        // pointSizeF( font(), 7) ); //6.5
        int ww = width() - mw.left() - mw.right();
        // int hw=height() - mw.top() - mw.bottom();
        int wl = m_ledOffPixmap->width();
        int hl = m_ledOffPixmap->height();
        int wt = mx.width(t);
        int ht = mx.height();
        int xt = 1;
        int yt = ht - 1;

        if(a == Qt::AnchorLeft)
        {
            xt = wl + 3;
        }
        else if(a == Qt::AnchorRight)
        {
            xl = ww - wl;
        }
        else if(a == Qt::AnchorBottom)
        {
            xl = (ww - wl) / 2;
            xt = (ww - wt) / 2;
            yt += yl + hl;
            yt -= 6;  // tmp
        }
        else if(a == Qt::AnchorTop)
        {
            xl = (ww - wl) / 2;
            yl += yt;
            xt = (ww - wt) / 2;
        }

        // p.setPen( QColor( 64, 64, 64 ) );
        // p.drawText( m_ledOffPixmap->width() + 4, 11, text() );
        xt += mw.left();
        yt += mw.top();
        p.setPen(Qt::white);  // QColor( 255, 255, 255 ) );
        p.drawText(xt, yt, text());
    }

    if(m_blinking)
        m_blinkingState = (QTime::currentTime().msec() * 6 / 1000) % 2 == 0;

    xl += mw.left();
    yl += mw.top();
    p.drawPixmap(xl, yl,
                 m && m->rawValue() && (!m_blinking || m_blinkingState)
                         ? *m_ledOnPixmap
                         : *m_ledOffPixmap);
    // m_blinkingState=!m_blinkingState;

    // p.setPen(Qt::red);
    // p.drawRect(0,0,width()-1,height()-1);
}

void LedCheckBox::initUi(LedColors _color)
{
    setCheckable(true);

    if(_color >= NumColors || _color < Yellow)
        _color = Yellow;

    m_ledOnPixmap = new QPixmap(
            embed::getPixmap(names[_color].toUtf8().constData()));
    m_ledOffPixmap = new QPixmap(embed::getPixmap("led_off"));

    // setFont(pointSize<11>(font()));
    // setText(m_text);
    onTextUpdated();
}

void LedCheckBox::onTextUpdated()
{
    QStyleOption opt;
    opt.init(this);

    const QString& t = text();
    int            w = m_ledOffPixmap->width();
    int            h = m_ledOffPixmap->height();//-6
    if(!t.isEmpty())
    {
        const Qt::AnchorPoint a  = textAnchorPoint();
        const QFont           ft = pointSizeF(
                font(),
                (a == Qt::AnchorLeft || a == Qt::AnchorRight) ? 8.5f : 6.5f);
        const QFontMetrics mx(ft);

        if((a == Qt::AnchorLeft) || (a == Qt::AnchorRight))
        {
            w += mx.width(t) + 3;
            h = qMax<int>(h, mx.height()) + 2;
        }
        if((a == Qt::AnchorTop) || (a == Qt::AnchorBottom))
        {
            w = qMax<int>(w, qMin<int>(2 * w, mx.width(t)));
            h += mx.height() + 2;
        }
    }

    QMargins mw = contentsMargins();
    w += mw.left() + mw.right();
    h += mw.top() + mw.bottom();

    setMinimumSize(w, h);
    resize(w,h);
    update();
}

/*
void LedCheckBox::enterValue()
{
    BoolModel* m = model();
    if(!m)
        return;

    bool ok;
    bool new_val;

    new_val = QInputDialog::getInt(this, windowTitle(),
                                   tr("Please enter a new value between "
                                      "%1 and %2:")
                                           .arg(m->minValue())
                                           .arg(m->maxValue()),
                                   m->rawValue(), m->minValue(),
                                   m->maxValue(), 0, &ok);

    if(ok)
    {
        m->setValue(new_val);
    }
}
*/

bool LedCheckBox::blinking() const
{
    return m_blinking;
}

void LedCheckBox::setBlinking(bool _b)
{
    if(m_blinking != _b)
    {
        m_blinking = _b;
        if(_b)
            connect(gui->mainWindow(), SIGNAL(periodicUpdate()), this,
                    SLOT(update()));
        else
            disconnect(gui->mainWindow(), SIGNAL(periodicUpdate()), this,
                       SLOT(update()));
    }
}
