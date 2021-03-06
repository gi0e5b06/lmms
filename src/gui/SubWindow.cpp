﻿/*
 * SubWindow.cpp - Implementation of QMdiSubWindow that correctly tracks
 *   the geometry that windows should be restored to.
 *   Workaround for https://bugreports.qt.io/browse/QTBUG-256
 *
 * Copyright (c) 2015 Colin Wallace <wallace.colin.a@gmail.com>
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

//#include <QDebug>
#include "SubWindow.h"

#include "GuiApplication.h"
#include "MainWindow.h"
#include "embed.h"

#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMoveEvent>
#include <QPainter>
#include <QScrollBar>

SubWindow* SubWindow::putWidgetOnWorkspace(QWidget* _widget,
                                           bool     _deleteOnClose,
                                           bool     _ignoreCloseEvents,
                                           bool     _maximizable,
                                           bool     _frameless)
{
    SubWindow* win
            = new SubWindow(_widget, _deleteOnClose, _ignoreCloseEvents,
                            _maximizable, _frameless);
    gui->mainWindow()->workspace()->addSubWindow(win);

    // win->setOption(QMdiSubWindow::RubberBandResize, true);
    // win->setOption(QMdiSubWindow::RubberBandMove, true);

    connect(win->mdiArea(), SIGNAL(subWindowActivated(QMdiSubWindow*)), win,
            SLOT(focusChanged(QMdiSubWindow*)));
    return win;
}

SubWindow::SubWindow(QWidget* _child,
                     bool     _deleteOnClose,
                     bool     _ignoreCloseEvents,
                     bool     _maximizable,
                     bool     _frameless) :
      QMdiSubWindow(),
      m_ignoreCloseEvents(_ignoreCloseEvents), m_buttonSize(17, 17),
      m_titleBarHeight(24), m_closeBtn(NULL), m_maximizeBtn(NULL),
      m_restoreBtn(NULL), m_hasFocus(false)
{
    if(!_child)
    {
        qWarning("SubWindow::SubWindow _child is null");
        _child = new QLabel("Content", this);
    }
    // setParent( gui->mainWindow()->workspace()->viewport() );
    // setWindowFlags( _flags );
    setAttribute(Qt::WA_DeleteOnClose, _deleteOnClose);
    // setAttribute(Qt::WA_OpaquePaintEvent, true);

    if(gui->mainWindow()->isTabbed())
    {
        _maximizable     = false;
        _frameless       = true;
        m_titleBarHeight = 0;
    }

    // inits the colors
    m_activeColor     = Qt::SolidPattern;
    m_textShadowColor = Qt::black;
    m_borderColor     = Qt::black;

    if(!gui->mainWindow()->isTabbed())
    {
        // close, maximize and restore (after maximizing) buttons
        m_closeBtn = new QPushButton(embed::getIcon("close"), QString::null,
                                     this);
        // m_closeBtn->setContentsMargins(0, 0, 0, 0);
        m_closeBtn->resize(m_buttonSize);
        m_closeBtn->setFocusPolicy(Qt::NoFocus);
        m_closeBtn->setCursor(Qt::ArrowCursor);
        m_closeBtn->setAttribute(Qt::WA_NoMousePropagation);
        m_closeBtn->setToolTip(tr("Close"));
        connect(m_closeBtn, SIGNAL(clicked(bool)), this, SLOT(close()));

        if(_maximizable)
        {
            m_maximizeBtn = new QPushButton(embed::getIcon("maximize"),
                                            QString::null, this);
            // m_maximizeBtn->setContentsMargins(0, 0, 0, 0);
            m_maximizeBtn->resize(m_buttonSize);
            m_maximizeBtn->setFocusPolicy(Qt::NoFocus);
            m_maximizeBtn->setCursor(Qt::ArrowCursor);
            m_maximizeBtn->setAttribute(Qt::WA_NoMousePropagation);
            m_maximizeBtn->setToolTip(tr("Maximize"));
            connect(m_maximizeBtn, SIGNAL(clicked(bool)), this,
                    SLOT(showMaximized()));

            m_restoreBtn = new QPushButton(embed::getIcon("restore"),
                                           QString::null, this);
            // m_restoreBtn->setContentsMargins(0, 0, 0, 0);
            m_restoreBtn->resize(m_buttonSize);
            m_restoreBtn->setFocusPolicy(Qt::NoFocus);
            m_restoreBtn->setCursor(Qt::ArrowCursor);
            m_restoreBtn->setAttribute(Qt::WA_NoMousePropagation);
            m_restoreBtn->setToolTip(tr("Restore"));
            connect(m_restoreBtn, SIGNAL(clicked(bool)), this,
                    SLOT(showNormal()));
        }

        // QLabel for the window title and the shadow effect
        /*
        m_shadow = new QGraphicsDropShadowEffect();
        m_shadow->setColor( m_textShadowColor );
        m_shadow->setXOffset( 1 );
        m_shadow->setYOffset( 1 );
        */

        m_windowTitle = new QLabel(this);
        m_windowTitle->setFocusPolicy(Qt::NoFocus);
        m_windowTitle->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        // m_windowTitle->setGraphicsEffect( m_shadow );
    }

    // disable the minimize button
    Qt::WindowFlags flags = Qt::SubWindow | Qt::CustomizeWindowHint;

    if(_maximizable)
        flags |= Qt::WindowMaximizeButtonHint;
    else
        flags |= Qt::MSWindowsFixedSizeDialogHint;

    if(_frameless)
        flags |= Qt::FramelessWindowHint;
    else
        flags |= Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint
                 | Qt::WindowTitleHint;

    setWindowFlags(flags);

    setWidget(_child);

    if(gui->mainWindow()->isTabbed())
    {
        QSize sz = gui->mainWindow()->workspace()->size();
        move(0, 0);
        resize(sz);
        widget()->setGeometry(0, 0, sz.width(), sz.height());
    }
    else
    {
        QSize sz = widget()->sizeHint();
        // qWarning("Child SizeHint: %d x %d",sz.width(),sz.height());
        widget()->resize(sz);
        sz = widget()->size();
        // qWarning("Child Size: %d x %d",sz.width(),sz.height());

        if(flags & Qt::FramelessWindowHint)
            resize(sz);
        else
            resize(sz.width() + 6, sz.height() + m_titleBarHeight + 4);
    }

    // qWarning("SubWindow Size: %d x %d\n",width(),height());

    // initialize the tracked geometry to whatever Qt thinks the normal
    // geometry currently is. this should always work, since QMdiSubWindows
    // will not start as maximized
    m_trackedNormalGeom = normalGeometry();

    if(!_maximizable)
    {
        // setFixedSize(size());
        // setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        // layout()->setSizeConstraint( QLayout::SetFixedSize );

        // Hide the Size and Maximize options from the system menu
        // since the dialog size is fixed.
        QMenu* menu = systemMenu();
        menu->actions().at(2)->setVisible(false);  // Size
        menu->actions().at(4)->setVisible(false);  // Maximize
    }
}

void SubWindow::closeEvent(QCloseEvent* _evt)
{
    if(m_ignoreCloseEvents)
    {
        // ignore close-events - for some reason otherwise the VST GUI
        // remains hidden when re-opening
        hide();
        _evt->ignore();
    }
    else
    {
        QMdiSubWindow::closeEvent(_evt);
    }

    emit closed();
}

void SubWindow::paintEvent(QPaintEvent*)
{
    if(gui->mainWindow()->isTabbed())
        return;

    QPainter p(this);
    QRect    rect(0, 0, width(), m_titleBarHeight);
    bool     isActive = (mdiArea() && (mdiArea()->activeSubWindow() == this));

    p.fillRect(rect, isActive ? activeColor() : p.pen().brush());

    // window border
    p.setPen(isActive ? activeColor().color() : borderColor());

    // bottom, left, and right lines
    p.drawLine(0, height() - 1, width() - 1, height() - 1);
    p.drawLine(0, m_titleBarHeight, 0, height() - 1);
    p.drawLine(width() - 1, m_titleBarHeight, width() - 1, height() - 1);

    // test thicker
    p.drawLine(1, height() - 2, width() - 2, height() - 2);
    p.drawLine(1, m_titleBarHeight, 1, height() - 2);
    p.drawLine(width() - 2, m_titleBarHeight, width() - 2, height() - 2);

    // window icon

    if(!widget())
        qWarning("SubWindow::paintEvent widget() is NULL");
    else
    {
        QPixmap winicon(widget()->windowIcon().pixmap(m_buttonSize));
        p.drawPixmap(3, 3, m_buttonSize.width(), m_buttonSize.height(),
                     winicon);
    }
}

void SubWindow::changeEvent(QEvent* event)
{
    QMdiSubWindow::changeEvent(event);

    if(event->type() == QEvent::WindowTitleChange)
    {
        adjustTitleBar();
    }
}

void SubWindow::elideText(QLabel* label, QString text)
{
    QFontMetrics metrix(label->font());
    int          width       = label->width() - 2;
    QString      clippedText = metrix.elidedText(text, Qt::ElideRight, width);
    label->setText(clippedText);
}

bool SubWindow::isMaximized()
{
#ifdef LMMS_BUILD_APPLE
    // check if subwindow size is identical to the MdiArea size, accounting
    // for scrollbars
    int hScrollBarHeight
            = mdiArea()->horizontalScrollBar()->isVisible()
                      ? mdiArea()->horizontalScrollBar()->size().height()
                      : 0;
    int vScrollBarWidth
            = mdiArea()->verticalScrollBar()->isVisible()
                      ? mdiArea()->verticalScrollBar()->size().width()
                      : 0;
    QSize areaSize(mdiArea()->size().width() - vScrollBarWidth,
                   mdiArea()->size().height() - hScrollBarHeight);

    return areaSize == size();
#else
    return QMdiSubWindow::isMaximized();
#endif
}

QRect SubWindow::getTrueNormalGeometry() const
{
    return m_trackedNormalGeom;
}

QBrush SubWindow::activeColor() const
{
    return m_activeColor;
}

QColor SubWindow::textShadowColor() const
{
    return m_textShadowColor;
}

QColor SubWindow::borderColor() const
{
    return m_borderColor;
}

void SubWindow::setActiveColor(const QBrush& b)
{
    m_activeColor = b;
}

void SubWindow::setTextShadowColor(const QColor& c)
{
    m_textShadowColor = c;
}

void SubWindow::setBorderColor(const QColor& c)
{
    m_borderColor = c;
}

void SubWindow::adjustTitleBar()
{
    if(gui->mainWindow()->isTabbed())
        return;

    // button adjustments
    if(m_maximizeBtn)
        m_maximizeBtn->hide();
    if(m_restoreBtn)
        m_restoreBtn->hide();

    const int rightSpace      = 3;
    const int buttonGap       = 1;
    const int menuButtonSpace = 24;

    QPoint rightButtonPos(width() - rightSpace - m_buttonSize.width(), 3);
    QPoint middleButtonPos(
            width() - rightSpace - (2 * m_buttonSize.width()) - buttonGap, 3);
    QPoint leftButtonPos(width() - rightSpace - (3 * m_buttonSize.width())
                                 - (2 * buttonGap),
                         3);

    // the buttonBarWidth depends on the number of buttons.
    // we need it to calculate the width of window title label
    int buttonBarWidth = rightSpace + m_buttonSize.width();

    // set the buttons on their positions.
    // the close button is always needed and on the rightButtonPos
    if(m_closeBtn)
        m_closeBtn->move(rightButtonPos);

    // here we ask: is the Subwindow maximizable and
    // then we set the buttons and show them if needed
    if(windowFlags() & Qt::WindowMaximizeButtonHint)
    {
        buttonBarWidth = buttonBarWidth + m_buttonSize.width() + buttonGap;
        if(m_maximizeBtn)
            m_maximizeBtn->move(middleButtonPos);
        if(m_restoreBtn)
            m_restoreBtn->move(middleButtonPos);
        if(m_maximizeBtn)
            m_maximizeBtn->setHidden(isMaximized());
    }

    // we're keeping the restore button around if we open projects
    // from older versions that have saved minimized windows
    if(m_restoreBtn)
        m_restoreBtn->setVisible(isMaximized() || isMinimized());

    // title QLabel adjustments
    QWidget* w = widget();

    m_windowTitle->setAlignment(Qt::AlignHCenter);
    m_windowTitle->setFixedWidth(qMax(
            0, w == nullptr
                       ? 0
                       : w->width() - (menuButtonSpace + buttonBarWidth)));
    m_windowTitle->move(menuButtonSpace,
                        (m_titleBarHeight / 2)
                                - (m_windowTitle->sizeHint().height() / 2)
                                - 1);

    // if minimized we can't use widget()->width(). We have to hard code the
    // width, as the width of all minimized windows is the same.
    if(isMinimized() && m_restoreBtn)
    {
        m_restoreBtn->move(m_maximizeBtn && m_maximizeBtn->isHidden()
                                   ? middleButtonPos
                                   : leftButtonPos);
        m_windowTitle->setFixedWidth(120);
    }

    // truncate the label string if the window is to small. Adds "..."
    elideText(m_windowTitle, w==nullptr ? "" : w->windowTitle());
    m_windowTitle->setTextInteractionFlags(Qt::NoTextInteraction);
    m_windowTitle->adjustSize();
}

void SubWindow::focusChanged(QMdiSubWindow* subWindow)
{
    if(m_hasFocus && subWindow != this)
    {
        m_hasFocus = false;
        emit focusLost();
    }
    else if(subWindow == this)
    {
        m_hasFocus = true;
    }
}

void SubWindow::moveEvent(QMoveEvent* event)
{
    /*
if(gui->mainWindow()->isTabbed())
{
    event->accept();
    QRect r(QPoint(0, 0), mdiArea()->size());
    if(r != geometry())
        setGeometry(r);
}
else
    */
    QMdiSubWindow::moveEvent(event);

    // if the window was moved and ISN'T minimized/maximized/fullscreen,
    // then save the current position
    if(!isMaximized() && !isMinimized() && !isFullScreen())
    {
        // const QRect& r=geometry();
        m_trackedNormalGeom.moveTopLeft(event->pos());
    }
}

void SubWindow::resizeEvent(QResizeEvent* event)
{
    adjustTitleBar();

    /*
    if(gui->mainWindow()->isTabbed())
    {
        event->accept();
        QRect r(QPoint(0, 0), mdiArea()->size());
        if(r != geometry())
            setGeometry(r);
    }
    else
    */
    QMdiSubWindow::resizeEvent(event);

    // if the window was resized and ISN'T minimized/maximized/fullscreen,
    // then save the current size
    if(!isMaximized() && !isMinimized() && !isFullScreen())
    {
        m_trackedNormalGeom.setSize(event->size());
    }
}
