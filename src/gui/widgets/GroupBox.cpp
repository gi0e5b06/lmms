/*
 * GroupBox.cpp - groupbox for LMMS
 *
 * Copyright (c) 2005-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include "GroupBox.h"

#include "embed.h"
#include "gui_templates.h"

//#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QVBoxLayout>

GroupBox::Top::Top(const QString& _caption,
                   GroupBox*      _parent,
                   bool           _led,
                   bool           _arrow) :
      QWidget(_parent)
{
    setFixedHeight(14);
    setFixedWidth(50);

    QGridLayout* gbl = new QGridLayout(this);
    gbl->setSpacing(2);
    gbl->setContentsMargins(2, 0, 2, 0);
    gbl->setColumnStretch(1, 1);

    if(_led)
    {
        m_led = new PixmapButton(this, _caption);
        m_led->setMinimumWidth(14);
        m_led->setCheckable(true);
        // m_led->move( 3, 0 );
        m_led->setActiveGraphic(embed::getIconPixmap("led_green"));
        m_led->setInactiveGraphic(embed::getIconPixmap("led_off"));
        gbl->addWidget(m_led, 0, 0, Qt::AlignHCenter | Qt::AlignVCenter);
    }
    else
        m_led = NULL;

    m_title = new QLabel(_caption, this);
    m_title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_title->setFixedHeight(14);
    // m_title->setFont(pointSize<6>(m_title->font()));
    m_title->setStyleSheet(
            "font-size: 10px; font-weight: bold; padding-left: 2px; "
            "background-color: #001f40;");
    // m_title->setFixedHeight(14);
    gbl->addWidget(m_title, 0, 1);

    if(_arrow)
    {
        m_arrow = new PixmapButton(this, _caption);
        m_arrow->setMinimumWidth(14);
        m_arrow->setCheckable(true);
        // m_arrow->move( 3, 0 );
        m_arrow->setActiveGraphic(embed::getIconPixmap("combobox_arrow"));
        m_arrow->setInactiveGraphic(
                embed::getIconPixmap("combobox_arrow_selected"));
        gbl->addWidget(m_arrow, 0, 2, Qt::AlignHCenter | Qt::AlignVCenter);
    }
    else
        m_arrow = NULL;
}

GroupBox::GroupBox(const QString& _caption,
                   QWidget*       _parent,
                   bool           _led,
                   bool           _arrow,
                   bool           _panel) :
      QWidget(_parent)
// BoolModelView(new BoolModel(false, NULL, _caption, true),
// m_caption(_caption)
{
    setObjectName(_caption);
    // setStyleSheet("background-color:#ffff00;");
    m_top   = new Top(_caption, this, _led, _arrow);
    m_panel = new QWidget(this);
    // m_panel->setContentsMargins(0, 0, 0, 0);
    // m_panel->setStyleSheet("background-color:#ffffff;");

    QVBoxLayout* vl = new QVBoxLayout(this);
    vl->setSpacing(0);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->addWidget(m_top, 0, Qt::AlignTop);
    vl->setStretch(0, 0);
    if(_panel)
    {
        vl->addWidget(m_panel, 1);
        vl->setStretch(1, 1);
    }

    setContentsMargins(0, 0, 0, 0);  // m_top->height(), 0, 0);

    // setModel(new BoolModel(false, NULL, _caption, true));
    setAutoFillBackground(true);
    unsetCursor();

    if(_arrow)
        connect(m_top->m_arrow->model(), SIGNAL(dataChanged()), this,
                SLOT(togglePanel()));
}

GroupBox::~GroupBox()
{
    // delete m_led;
}

void GroupBox::togglePanel()
{
    m_panel->setVisible(!m_panel->isVisible());
    // update();
    // QSize sz = sizeHint();
    // qInfo("GroupBox::togglePanel %dx%d", sz.width(), sz.height());
    // resize(sz);
    adjustSize();
    parentWidget()->adjustSize();
    // parentWidget()->parentWidget()->adjustSize();
    // parentWidget()->parentWidget()->parentWidget()->adjustSize();
}

void GroupBox::addTopWidget(QWidget* _w, int _col)
{
    dynamic_cast<QGridLayout*>(m_top->layout())->addWidget(_w, 0, _col);
}

int GroupBox::titleBarHeight() const
{
    return m_top->height();  // m_titleBarHeight;
}

QWidget* GroupBox::contentWidget()
{
    return m_panel;
}

PixmapButton* GroupBox::ledButton()
{
    return m_top->m_led;
}

bool GroupBox::isEnabled()
{
    return m_top->m_led ? m_top->m_led->model()->value() : true;
}

void GroupBox::setEnabled(bool _b)
{
    if(m_top->m_led && m_top->m_led->model()->value() != _b)
    {
        m_top->m_led->model()->setValue(_b);
        emit enabledChanged();
    }
}

/*
BoolModel* GroupBox::model()
{
    if(m_top->m_led)
        return m_top->m_led->model();
    else
        return NULL;
}

void GroupBox::setModel(BoolModel* _model)
{
    if(m_top->m_led)
        m_top->m_led->setModel(_model);
}
*/

/*
void GroupBox::mousePressEvent(QMouseEvent* _me)
{
    if(_me->y() > 1 && _me->y() < 13 && _me->button() == Qt::LeftButton)
    {
        model()->setValue(!model()->value());
    }
}
*/

void GroupBox::resizeEvent(QResizeEvent* _re)
{
    m_top->setFixedWidth(width());
    m_top->setFixedHeight(14);
    // MaximumHeight(m_top->sizeHint().height());
    // setContentsMargins(0, m_top->height(), 0, 0);
    // QWidget::resizeEvent(_re);
}

void GroupBox::paintEvent(QPaintEvent* _pe)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    /*
    QPainter p( this );

    // Draw background
    //p.fillRect( 0, 0, width() - 1, height() - 1, p.background() );
    //p.fillRect(0,0,width(),height(),p.background());
    //p.fillRect(0,0,width(),height(),p.background().color().darker(150));

    // outer rect
    p.setPen(p.background().color().dark(150));
    p.drawRect( 0, 0, width() - 1, height() - 1 );

    // draw line below titlebar
    //p.fillRect( 1, 1, width() - 2, m_titleBarHeight + 1,
    p.background().color().darker( 150 ) );
    p.fillRect(0,0,width()-1,m_titleBarHeight-1,p.background().color().darker(150));

    // draw text
    p.setPen( palette().color( QPalette::Active, QPalette::Text ) );
    p.setFont( pointSize<8>( font() ) );
    p.drawText( 22, m_titleBarHeight-3, m_caption );
    */
}

void GroupBox::enterValue()
{
    if(!m_top || !m_top->m_led)
        return;

    BoolModel* m = m_top->m_led->model();
    bool       ok;
    bool       new_val;

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

//#include "GroupBox.moc"
