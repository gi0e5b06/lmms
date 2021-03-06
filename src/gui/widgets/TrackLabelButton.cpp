/*
 * TrackLabelButton.cpp - implementation of class trackLabelButton, a label
 *                          which is renamable by double-clicking it
 *
 * Copyright (c) 2004-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "TrackLabelButton.h"

#include "ConfigManager.h"
#include "Engine.h"
#include "Instrument.h"
#include "InstrumentTrack.h"
#include "RenameDialog.h"
#include "Song.h"
#include "TrackRenameLineEdit.h"
#include "embed.h"

//#include <QApplication>
#include <QMouseEvent>

TrackLabelButton::TrackLabelButton(TrackView* _tv, QWidget* _parent) :
    QToolButton(_parent), m_trackView(_tv)//, m_iconName()
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAcceptDrops(true);
    setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_renameLineEdit = new TrackRenameLineEdit(this);
    m_renameLineEdit->hide();

    if(ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt())
    {
        setFixedSize(30, 29);
    }
    else
    {
        setFixedSize(148, 29);  // 160
        m_renameLineEdit->move(
                29, (height() / 2)
                            - (m_renameLineEdit->sizeHint().height() / 2)
                            + 2);                       //+0
        m_renameLineEdit->setFixedWidth(width() - 29);  // 32
        connect(m_renameLineEdit, SIGNAL(editingFinished()), this,
                SLOT(renameFinished()));
    }

    setIconSize(QSize(20, 20));
    Track* t = m_trackView->track();
    connect(t, SIGNAL(dataChanged()), this, SLOT(update()));
    connect(t, SIGNAL(nameChanged()), this, SLOT(onNameChanged()));
    connect(t, SIGNAL(iconChanged()), this, SLOT(onIconChanged()));
}

TrackLabelButton::~TrackLabelButton()
{
}

void TrackLabelButton::rename()
{
    if(ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt())
    {
        QString      txt = m_trackView->track()->name();
        RenameDialog renameDlg(txt);
        renameDlg.exec();
        if(txt != text())
        {
            m_trackView->track()->setName(txt);
            Engine::getSong()->setModified();
        }
    }
    else
    {
        QString txt = m_trackView->track()->name();
        m_renameLineEdit->show();
        m_renameLineEdit->setText(txt);
        m_renameLineEdit->selectAll();
        m_renameLineEdit->setFocus();
    }
}

void TrackLabelButton::renameFinished()
{
    if(!(ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt()))
    {
        m_renameLineEdit->hide();
        if(m_renameLineEdit->text() != "")
        {
            if(m_renameLineEdit->text() != m_trackView->track()->name())
            {
                setText(elideName(m_renameLineEdit->text()));
                m_trackView->track()->setName(m_renameLineEdit->text());
                Engine::getSong()->setModified();
            }
        }
    }
}

void TrackLabelButton::onIconChanged()
{
    // QIcon icon=QToolButton::icon();
    // setIcon(icon);

    InstrumentTrack* it
            = dynamic_cast<InstrumentTrack*>(m_trackView->track());
    const PixmapLoader* pl;
    if(it != nullptr && it->instrument() != nullptr
       && it->instrument()->descriptor()
       && (pl = it->instrument()->descriptor()->logo()))
    {
        if(pl != nullptr)  // pl->name() != m_iconName)
        {
            //m_iconName = pl->name();
            setIcon(pl->pixmap());
        }
    }
}

void TrackLabelButton::onNameChanged()
{
    setText(elideName(m_trackView->track()->name()));
}

void TrackLabelButton::dragEnterEvent(QDragEnterEvent* _dee)
{
    m_trackView->dragEnterEvent(_dee);
}

void TrackLabelButton::dropEvent(QDropEvent* _de)
{
    m_trackView->dropEvent(_de);
    setChecked(true);
}

void TrackLabelButton::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::RightButton)
    {
        rename();
    }
    else
    {
        m_buttonRect = QRect(this->mapToGlobal(pos()), size());
        _me->ignore();
    }
}

void TrackLabelButton::mouseDoubleClickEvent(QMouseEvent* _me)
{
    rename();
}

void TrackLabelButton::mouseReleaseEvent(QMouseEvent* _me)
{
    if(m_buttonRect.contains(_me->globalPos(), true)
       && m_renameLineEdit->isHidden())
    {
        QToolButton::mousePressEvent(_me);
    }
    QToolButton::mouseReleaseEvent(_me);
    _me->ignore();
}

void TrackLabelButton::paintEvent(QPaintEvent* _pe)
{
    /*
    if(m_trackView->track()->type() == Track::InstrumentTrack)
    {
        InstrumentTrack* it
                = dynamic_cast<InstrumentTrack*>(m_trackView->track());
        const PixmapLoader* pl;
        if(it && it->instrument() && it->instrument()->descriptor()
           && (pl = it->instrument()->descriptor()->logo()))
        {
            if(pl->name() != m_iconName)
            {
                m_iconName = pl->name();
                setIcon(pl->pixmap());
            }
        }
    }
    */
    QToolButton::paintEvent(_pe);
}

void TrackLabelButton::resizeEvent(QResizeEvent* _re)
{
    setText(elideName(m_trackView->track()->displayName()));
}

QString TrackLabelButton::elideName(const QString& name)
{
    const int spacing      = 6;  // 16
    const int maxTextWidth = width() - spacing - iconSize().width();
    if(maxTextWidth < 1)
    {
        setToolTip(m_trackView->track()->displayName());
        return QString(" ");
    }
    setToolTip("");
    QFontMetrics metrics(font());
    QString      elidedName
            = metrics.elidedText(name, Qt::ElideRight, maxTextWidth);
    return elidedName;
}
