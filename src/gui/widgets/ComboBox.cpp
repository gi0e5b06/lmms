/*
 * ComboBox.cpp - implementation of LMMS combobox
 *
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2008-2009 Paul Giblock <pgib/at/users.sourceforge.net>
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

#include "ComboBox.h"

#include "CaptionMenu.h"
#include "embed.h"
#include "gui_templates.h"

#include <QApplication>
#include <QInputDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionFrame>

static PixmapLoader /*ComboBox::*/ s_background("combobox_bg");
static PixmapLoader /*ComboBox::*/ s_arrow("combobox_arrow");
static PixmapLoader /*ComboBox::*/ s_arrowSelected("combobox_arrow_selected");

const int CB_ARROW_BTN_WIDTH = 20;

ComboBox::ComboBox(QWidget*       _parent,
                   const QString& _displayName,
                   const QString& _objectName) :
      QWidget(_parent),
      IntModelView(
              new ComboBoxModel(nullptr, _displayName, _objectName, true),
              this),
      m_menu(this), m_pressed(false)
{
    /*
    if(s_background == nullptr)
        s_background.reset(new QPixmap(embed::getPixmap("combobox_bg")));

    if(s_arrow == nullptr)
        s_arrow.reset(new QPixmap(embed::getPixmap("combobox_arrow")));

    if(s_arrowSelected == nullptr)
        s_arrowSelected.reset(
                new QPixmap(embed::getPixmap("combobox_arrow_selected")));
    */

    // setFont( pointSize<9>( font() ) );
    // m_menu.setFont( pointSize<8>( m_menu.font() ) );
    setMinimumSize(44, 22);  // Height(22);

    connect(&m_menu, SIGNAL(triggered(QAction*)), this,
            SLOT(setItem(QAction*)));

    setWindowTitle(_displayName);
}

ComboBox::~ComboBox()
{
}

void ComboBox::selectNext()
{
    // model()->setInitValue(model()->value() + 1);
    model()->setInitValue(model()->nextValue());
}

void ComboBox::selectPrevious()
{
    // model()->setInitValue(model()->value() - 1);
    model()->setInitValue(model()->previousValue());
}

void ComboBox::contextMenuEvent(QContextMenuEvent* event)
{
    if(model() == nullptr || event->x() <= width() - CB_ARROW_BTN_WIDTH)
    {
        QWidget::contextMenuEvent(event);
        return;
    }

    CaptionMenu contextMenu(model()->displayName());
    addDefaultActions(&contextMenu);
    contextMenu.exec(QCursor::pos());
}

void ComboBox::mousePressEvent(QMouseEvent* event)
{
    ComboBoxModel* m = model();
    if(m == nullptr)
        return;

    if(event->button() == Qt::LeftButton
       && !(event->modifiers() & Qt::ControlModifier))
    {
        if(event->x() > width() - CB_ARROW_BTN_WIDTH)
        {
            m_pressed = true;
            update();

            m_menu.clear();
            for(int i = 0; i < m->size(); ++i)
            {
                QAction* a = m_menu.addAction(
                        m->itemIcon(i) ? m->itemIcon(i)->pixmap() : QPixmap(),
                        m->itemText(i));
                a->setData(i);
            }

            QPoint gpos = mapToGlobal(QPoint(0, height()));
            if(gpos.y() + m_menu.sizeHint().height()
               < qApp->desktop()->height())
            {
                m_menu.exec(gpos);
            }
            else
            {
                m_menu.exec(mapToGlobal(QPoint(width(), 0)));
            }
            m_pressed = false;
            update();
        }
        else  // if( event->button() == Qt::LeftButton )
        {
            selectNext();
            update();
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        if(event->x() <= width() - CB_ARROW_BTN_WIDTH)
        {
            selectPrevious();
            update();
        }
    }
    else
    {
        IntModelView::mousePressEvent(event);
    }
}

void ComboBox::paintEvent(QPaintEvent* _pe)
{
    QPainter p(this);

    // p.fillRect( 2, 2, width()-2, height()-4, *s_background );
    p.fillRect(2, 2, width() - 2, height() - 4, p.background());

    QColor shadow    = palette().shadow().color();
    QColor highlight = palette().highlight().color();

    shadow.setAlpha(124);
    highlight.setAlpha(124);

    // button-separator
    p.setPen(shadow);
    p.drawLine(width() - CB_ARROW_BTN_WIDTH - 1, 1,
               width() - CB_ARROW_BTN_WIDTH - 1, height() - 3);

    p.setPen(highlight);
    p.drawLine(width() - CB_ARROW_BTN_WIDTH, 1, width() - CB_ARROW_BTN_WIDTH,
               height() - 3);

    // Border
    QStyleOptionFrame opt;
    opt.initFrom(this);
    opt.state = 0;

    style()->drawPrimitive(QStyle::PE_Frame, &opt, &p, this);

    QPixmap arrow = m_pressed ? s_arrowSelected : s_arrow;

    p.drawPixmap(width() - CB_ARROW_BTN_WIDTH + 5, (height() - 15) / 2,
                 arrow);

    ComboBoxModel* m = model();

    if(m != nullptr && m->size() > 0)
    {
        p.setFont(font());
        p.setClipRect(
                QRect(4, 2, width() - CB_ARROW_BTN_WIDTH - 8, height() - 2));
        QPixmap pm
                = m->currentIcon() ? m->currentIcon()->pixmap() : QPixmap();
        int tx = 5;
        if(!pm.isNull())
        {
            if(pm.height() > 16)
                pm = pm.scaledToHeight(16, Qt::SmoothTransformation);

            p.drawPixmap(tx, 3, pm);
            tx += pm.width() + 3;
        }
        const int y = (height() + p.fontMetrics().height()) / 2;
        p.setPen(QColor(64, 64, 64));
        p.drawText(tx + 1, y - 3, model()->currentText());
        p.setPen(QColor(224, 224, 224));
        p.drawText(tx, y - 4, model()->currentText());
    }
}

void ComboBox::wheelEvent(QWheelEvent* _we)
{
    if(_we->modifiers() & Qt::ShiftModifier)
    {
        ComboBoxModel* m = model();
        if(m != nullptr)
        {
            m->setInitValue(m->value() + ((_we->delta() < 0) ? 1 : -1));
            update();
            _we->accept();
        }
    }
}

void ComboBox::setItem(QAction* item)
{
    ComboBoxModel* m = model();
    if(m == nullptr)
        return;

    m->setInitValue(m->itemValue(item->data().toInt()));
}

void ComboBox::enterValue()
{
    ComboBoxModel* m = model();
    if(m == nullptr)
        return;

    bool ok;
    bool new_val;

    new_val = QInputDialog::getInt(this, windowTitle(),
                                   tr("Please enter a new value between "
                                      "%1 and %2:")
                                           .arg(m->minValue())
                                           .arg(m->maxValue()),
                                   m->value(), m->minValue(), m->maxValue(),
                                   0, &ok);

    if(ok)
    {
        m->setValue(new_val);
    }
}
