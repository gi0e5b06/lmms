/*
 * PixmapButton.cpp - A pixmap-based button
 * (often used as "themed" checkboxes/radiobuttons etc)
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2013 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "PixmapButton.h"

//#include "MainWindow.h"
#include "Backtrace.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "embed.h"

#include <QFontMetrics>
#include <QInputDialog>
#include <QPainter>
//#include <QStyle>
#include <QStyleOption>
#include <QTime>

PixmapButton::PixmapButton(QWidget* _parent, const QString& _name) :
      AutomatableButton(_parent, _name), m_activePixmap(), m_inactivePixmap(),
      m_pressed(false), m_blinkingState(true), m_blinking(false)
{
    setActiveGraphic(embed::getPixmap("led_yellow"));
    setInactiveGraphic(embed::getPixmap("led_off"), false);
}

PixmapButton::~PixmapButton()
{
}

void PixmapButton::update()
{
    AutomatableButton::update();
}

void PixmapButton::paintEvent(QPaintEvent*)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    BoolModel* m = model();
    if(!m)
        return;

    if(m_blinking)
        m_blinkingState = (QTime::currentTime().msec() * 6 / 1000) % 2 == 0;

    if((m != nullptr && m->rawValue()
        && (!m_blinking || m_blinkingState || (isCheckable() && m_pressed)))
       || (!isCheckable() && m_pressed))
    {
        if(!m_activePixmap.isNull())
        {
            // p.drawPixmap( 0, 0, m_activePixmap.scaled
            //	      (size().boundedTo(m_activePixmap.size())));
            p.drawPixmap(0, 0, m_activePixmap.scaled(size()));
        }
    }
    else if(!m_inactivePixmap.isNull())
    {
        // p.drawPixmap( 0, 0, m_inactivePixmap.scaled
        //	      (size().boundedTo(m_activePixmap.size())));
        p.drawPixmap(0, 0, m_inactivePixmap.scaled(size()));
    }

    // m_blinkingState=!m_blinkingState;

    // p.setPen(Qt::red);
    // p.drawRect(0,0,width()-1,height()-1);
}

void PixmapButton::resizeEvent(QResizeEvent* _re)
{
    // qInfo("Knob::resizeEvent()");
    // BACKTRACE
    QSize sh = sizeHint();
    // qInfo("PixmapButton::resizeEvent %dx%d ->
    // %dx%d",width(),height(),sh.width(),sh.height());
    if(sh != size())
    {
        setFixedSize(sh);
        resize(sh);
    }
}

void PixmapButton::mousePressEvent(QMouseEvent* _me)
{
    // Show pressing graphics if this isn't checkable
    m_pressed = true;
    if(!isCheckable())
        update();
    AutomatableButton::mousePressEvent(_me);
}

void PixmapButton::mouseReleaseEvent(QMouseEvent* _me)
{
    AutomatableButton::mouseReleaseEvent(_me);
    m_pressed = false;
    if(!isCheckable())
        update();
}

void PixmapButton::mouseDoubleClickEvent(QMouseEvent* _me)
{
    emit doubleClicked();
    _me->accept();
}

void PixmapButton::setActiveGraphic(const QPixmap& _pm, bool _update)
{
    m_activePixmap = _pm;
    setFixedSize(
            sizeHint());  // resize( sizeHint() );//m_activePixmap.width(),
                          // m_activePixmap.height() );
    if(_update)
        update();
}

void PixmapButton::setInactiveGraphic(const QPixmap& _pm, bool _update)
{
    m_inactivePixmap = _pm;
    setFixedSize(sizeHint());  // resize( sizeHint() );
    if(_update)
        update();
}

QSize PixmapButton::sizeHint() const
{
    /*
    if( ( model() != nullptr && model()->rawValue() ) || m_pressed )
    {
            //return
    minimumSize().expandedTo(m_activePixmap.size()).boundedTo(maximumSize());
            return m_activePixmap.size();
    }
    else
    {
            //return
    minimumSize().expandedTo(m_inactivePixmap.size()).boundedTo(maximumSize());
            return m_inactivePixmap.size();
    }
    */

    QSize r = QSize(0, 0);
    if(!m_activePixmap.isNull())
        r = r.expandedTo(m_activePixmap.size());
    if(!m_inactivePixmap.isNull())
        r = r.expandedTo(m_inactivePixmap.size());
    return r;
}

/*
void PixmapButton::enterValue()
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

bool PixmapButton::blinking() const
{
    return m_blinking;
}

void PixmapButton::setBlinking(bool _b)
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
