/*
 * GroupBox.cpp - groupbox for LMMS
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
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

GroupBox::Top::Top(const QString& _title,
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
        m_led = new PixmapButton(this, _title);
        m_led->setMinimumWidth(14);
        m_led->setCheckable(true);
        m_led->setActiveGraphic(embed::getPixmap("led_green"));
        m_led->setInactiveGraphic(embed::getPixmap("led_off"));
        gbl->addWidget(m_led, 0, 0, Qt::AlignHCenter | Qt::AlignVCenter);
        connect(m_led->model(), SIGNAL(dataChanged()), _parent,
                SIGNAL(enabledChanged()));
    }
    else
    {
        m_led = nullptr;
    }

    m_title = new QLabel(_title, this);
    m_title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_title->setFixedHeight(14);
    // m_title->setFont(pointSize<6>(m_title->font()));
    m_title->setStyleSheet(
            "font-size: 10px; font-weight: bold; padding-left: 2px; "
            "background-color: #002040;");
    // m_title->setFixedHeight(14);
    gbl->addWidget(m_title, 0, 1);

    if(_arrow)
    {
        m_arrow = new PixmapButton(this, _title);
        m_arrow->setMinimumWidth(14);
        m_arrow->setCheckable(true);
        m_arrow->setActiveGraphic(embed::getPixmap("combobox_arrow"));
        m_arrow->setInactiveGraphic(
                embed::getPixmap("combobox_arrow_selected"));
        gbl->addWidget(m_arrow, 0, 2, Qt::AlignHCenter | Qt::AlignVCenter);
    }
    else
    {
        m_arrow = nullptr;
    }
}

GroupBox::GroupBox(const QString& _title,
                   QWidget*       _parent,
                   bool           _led,
                   bool           _arrow) :
      QWidget(_parent)
{
    // setObjectName(_title);

    QGridLayout* vl = new QGridLayout(this);
    vl->setSpacing(0);
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setRowStretch(0, 0);
    vl->setRowStretch(1, 1);
    vl->setRowStretch(2, 0);
    vl->setColumnStretch(0, 1);
    // qInfo("GroupBox::GroupBox setLayout(vl); BEFORE");
    setLayout(vl);
    // qInfo("GroupBox::GroupBox setLayout(vl); AFTER");

    m_top    = new Top(_title, this, _led, _arrow);
    m_panel  = nullptr;
    m_bottom = nullptr;
    vl->addWidget(m_top, 0, 0, Qt::AlignTop);

    setAutoFillBackground(true);
    unsetCursor();

    if(_arrow)
        connect(m_top->m_arrow->model(), SIGNAL(dataChanged()), this,
                SLOT(togglePanel()));
}

GroupBox::~GroupBox()
{
}

void GroupBox::togglePanel()
{
    if(!m_panel)
        return;

    m_panel->setVisible(!m_panel->isVisible());
    adjustSize();
    parentWidget()->adjustSize();
}

void GroupBox::addTopWidget(QWidget* _w, int _col)
{
    dynamic_cast<QGridLayout*>(m_top->layout())->addWidget(_w, 0, _col);
}

int GroupBox::titleBarHeight() const
{
    return m_top->height();
}

QWidget* GroupBox::contentWidget()
{
    return m_panel;
}

void GroupBox::setContentWidget(QWidget* _w)
{
    if(m_panel == _w)
        return;

    bool visible = false;
    if(m_panel && m_panel->isVisible())
    {
        togglePanel();
        visible         = true;
        QGridLayout* vl = dynamic_cast<QGridLayout*>(layout());
        vl->removeWidget(m_panel);
    }
    m_panel = _w;
    if(m_panel)
    {
        QGridLayout* vl = dynamic_cast<QGridLayout*>(layout());
        vl->addWidget(m_panel, 1, 0);

        if(visible && m_panel->isVisible())
        {
            togglePanel();
        }
    }
    adjustSize();
    if(parentWidget())
        parentWidget()->adjustSize();
}

QWidget* GroupBox::bottomWidget()
{
    return m_bottom;
}

void GroupBox::setBottomWidget(QWidget* _w)
{
    if(m_bottom == _w)
        return;

    m_bottom = _w;
    if(m_bottom)
    {
        QGridLayout* vl = dynamic_cast<QGridLayout*>(layout());
        vl->addWidget(m_bottom, 2, 0);
    }
    adjustSize();
    if(parentWidget())
        parentWidget()->adjustSize();
}

PixmapButton* GroupBox::ledButton()
{
    return m_top->m_led;
}

PixmapButton* GroupBox::arrowButton()
{
    return m_top->m_arrow;
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
        // emit enabledChanged();
    }
}

void GroupBox::resizeEvent(QResizeEvent* _re)
{
    m_top->setFixedWidth(width());
    m_top->setFixedHeight(14);
}

void GroupBox::paintEvent(QPaintEvent* _pe)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
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
