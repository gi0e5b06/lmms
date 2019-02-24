/*
 * Editor.cpp - implementation of Editor class
 *
 * Copyright (c) 2014 Lukas W <lukaswhl/at/gmail.com>
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

#include "Editor.h"

#include "ComboBoxModel.h"
#include "Song.h"
#include "embed.h"

#include <QAction>
#include <QLabel>
#include <QShortcut>

const QVector<real_t> Editor::ZOOM_LEVELS
        = {0.10, 0.20, 0.50, 1.0, 2.0, 5.0, 10.0, 20.0};

void Editor::fillZoomLevels(ComboBoxModel& _cbm)
{
    for(const real_t& zoomLevel : Editor::ZOOM_LEVELS)
    {
        _cbm.addItem(QString("%1\%").arg(zoomLevel * 100));
    }
}

const QVector<tick_t> Editor::QUANTIZE_LEVELS = {
        4 * 192,  2 * 192,   1 * 192,  192 / 2,   192 / 4, 192 / 8,
        192 / 16, 192 / 32, 192 / 64, 192 / 3, 192 / 6,
        192 / 12, 192 / 24, 192 / 48, 192 / 96,  1 };

void Editor::fillQuantizeLevels(ComboBoxModel& _cbm)
{
    _cbm.addItem("4/1");
    _cbm.addItem("2/1");
    _cbm.addItem("1/1");
    _cbm.addItem("1/2");
    _cbm.addItem("1/4");
    _cbm.addItem("1/8");
    _cbm.addItem("1/16");
    _cbm.addItem("1/32");
    _cbm.addItem("1/64");
    _cbm.addItem("1/3");
    _cbm.addItem("1/6");
    _cbm.addItem("1/12");
    _cbm.addItem("1/24");
    _cbm.addItem("1/48");
    _cbm.addItem("1/96");
    _cbm.addItem("1/192");
}

const QVector<tick_t> Editor::LENGTH_LEVELS = {
        4 * 192,  2 * 192,   1 * 192,  192 / 2,   192 / 4, 192 / 8,
        192 / 16, 192 / 32, 192 / 64, 192 / 3, 192 / 6,
        192 / 12, 192 / 24, 192 / 48, 192 / 96,  1};

void Editor::fillLengthLevels(ComboBoxModel& _cbm)
{
    _cbm.addItem("4/1", new PixmapLoader("note_four_whole"));
    _cbm.addItem("2/1", new PixmapLoader("note_double_whole"));
    _cbm.addItem("1/1", new PixmapLoader("note_whole"));
    _cbm.addItem("1/2", new PixmapLoader("note_half"));
    _cbm.addItem("1/4", new PixmapLoader("note_quarter"));
    _cbm.addItem("1/8", new PixmapLoader("note_eighth"));
    _cbm.addItem("1/16", new PixmapLoader("note_sixteenth"));
    _cbm.addItem("1/32", new PixmapLoader("note_thirtysecond"));
    _cbm.addItem("1/64", new PixmapLoader("note_sixtyfourth"));
    _cbm.addItem("1/3", new PixmapLoader("note_triplethalf"));
    _cbm.addItem("1/6", new PixmapLoader("note_tripletquarter"));
    _cbm.addItem("1/12", new PixmapLoader("note_tripleteighth"));
    _cbm.addItem("1/24", new PixmapLoader("note_tripletsixteenth"));
    _cbm.addItem("1/48", new PixmapLoader("note_tripletthirtysecond"));
    _cbm.addItem("1/96", new PixmapLoader("note_tripletsixtyfourth"));
    _cbm.addItem("1/192", new PixmapLoader("note_tick"));
}

void Editor::setPauseIcon(bool displayPauseIcon)
{
    // If we're playing, show a pause icon
    if(displayPauseIcon)
        m_playAction->setIcon(embed::getIconPixmap("pause"));
    else
        m_playAction->setIcon(embed::getIconPixmap("play"));
}

DropToolBar* Editor::addDropToolBarToTop(QString const& windowTitle)
{
    return addDropToolBar(Qt::TopToolBarArea, windowTitle);
}

DropToolBar* Editor::addDropToolBar(Qt::ToolBarArea whereToAdd,
                                    QString const&  windowTitle)
{
    return addDropToolBar(this, whereToAdd, windowTitle);
}

DropToolBar* Editor::addDropToolBar(QWidget*        parent,
                                    Qt::ToolBarArea whereToAdd,
                                    QString const&  windowTitle)
{
    DropToolBar* toolBar = new DropToolBar(parent);
    addToolBar(whereToAdd, toolBar);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    toolBar->setWindowTitle(windowTitle);

    return toolBar;
}

void Editor::togglePlayStop()
{
    if(Engine::getSong()->isPlaying())
        stop();
    else
        play();
    requireActionUpdate();
}

Editor::Editor(bool record) :
      m_toolBar(new DropToolBar(this)), m_playAction(nullptr),
      m_recordAction(nullptr), m_recordAccompanyAction(nullptr),
      m_stopAction(nullptr)
{
    m_toolBar = addDropToolBarToTop(tr("Transport controls"));

    auto addButton = [this](QAction* action, QString objectName) {
        m_toolBar->addAction(action);
        m_toolBar->widgetForAction(action)->setObjectName(objectName);
    };

    // Set up play and record actions
    m_playAction = new QAction(embed::getIconPixmap("play"),
                               tr("Play (Space)"), this);
    m_stopAction = new QAction(embed::getIconPixmap("stop"),
                               tr("Stop (Space)"), this);

    m_recordAction
            = new QAction(embed::getIconPixmap("record"), tr("Record"), this);
    m_recordAccompanyAction
            = new QAction(embed::getIconPixmap("record_accompany"),
                          tr("Record while playing"), this);

    // Set up connections
    connect(m_playAction, SIGNAL(triggered()), this, SLOT(play()));
    connect(m_recordAction, SIGNAL(triggered()), this, SLOT(record()));
    connect(m_recordAccompanyAction, SIGNAL(triggered()), this,
            SLOT(recordAccompany()));
    connect(m_stopAction, SIGNAL(triggered()), this, SLOT(stop()));
    new QShortcut(Qt::Key_Space, this, SLOT(togglePlayStop()));

    // Add actions to toolbar
    addButton(m_playAction, "playButton");
    if(record)
    {
        addButton(m_recordAction, "recordButton");
        addButton(m_recordAccompanyAction, "recordAccompanyButton");
    }
    addButton(m_stopAction, "stopButton");
}

Editor::~Editor()
{
}

QAction* Editor::playAction() const
{
    return m_playAction;
}

DropToolBar::DropToolBar(QWidget* parent) : QToolBar(parent)
{
    setAcceptDrops(true);
}

void DropToolBar::addBlank()
{
    QLabel* lbl = new QLabel(this);
    lbl->setFixedSize(34, 34);
    // lbl->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    addWidget(lbl);
}

void DropToolBar::dragEnterEvent(QDragEnterEvent* event)
{
    dragEntered(event);
}

void DropToolBar::dropEvent(QDropEvent* event)
{
    dropped(event);
}
