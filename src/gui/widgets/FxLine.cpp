/*
 * FxLine.cpp - FX line widget
 *
 * Copyright (c) 2009 Andrew Kelley <superjoe30/at/gmail/dot/com>
 * Copyright (c) 2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "FxLine.h"

#include "CaptionMenu.h"
#include "FxMixer.h"
#include "FxMixerView.h"
#include "GuiApplication.h"
#include "Song.h"
#include "embed.h"
//#include "gui_templates.h"

#include <QGraphicsProxyWidget>
#include <QWhatsThis>

const int FxLine::FxLineHeight = 457;  // 287

static PixmapLoader s_sendBgArrow("send_bg_arrow", 29, 56);
static PixmapLoader s_receiveBgArrow("receive_bg_arrow", 29, 56);

FxLine::FxLine(QWidget* _parent, IntModel* _currentLine, int _channelIndex) :
      QWidget(_parent), m_currentLine(_currentLine),
      m_channelIndex(
              _channelIndex, 0, 10000, nullptr, tr("Channel"), "channel"),
      m_backgroundActive(Qt::SolidPattern), m_strokeOuterActive(0, 0, 0),
      m_strokeOuterInactive(0, 0, 0), m_strokeInnerActive(0, 0, 0),
      m_strokeInnerInactive(0, 0, 0), m_inRename(false)
{
    /*
    if(s_sendBgArrow == nullptr)
        s_sendBgArrow
                = new QPixmap(embed::getPixmap("send_bg_arrow", 29, 56));
    if(s_receiveBgArrow == nullptr)
        s_receiveBgArrow
                = new QPixmap(embed::getPixmap("receive_bg_arrow", 29, 56));
    */

    setFixedSize(33, FxLineHeight);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    // setCursor(Qt::PointingHandCursor);

    // mixer sends knob
    m_sendKnob = new Knob(knobBright_26, this, tr("Channel send amount"));
    m_sendKnob->move(3, 22);
    m_sendKnob->setVisible(false);
    m_sendKnob->allowNullModel(true);
    m_sendKnob->allowModelChange(true);

    // send button indicator
    m_sendBtn = new SendButtonIndicator(this, m_currentLine, &m_channelIndex);
    m_sendBtn->move(2, 2);

    // channel number
    m_lcd = new LcdWidget(2, this);
    m_lcd->setValue(channelIndex());
    m_lcd->move(4, 58);
    m_lcd->setMarginWidth(1);

    setWhatsThis(
            tr("The FX channel receives input from one or more instrument "
               "tracks.\n "
               "It in turn can be routed to multiple other FX channels. LMMS "
               "automatically "
               "takes care of preventing infinite loops for you and doesn't "
               "allow making "
               "a connection that would result in an infinite loop.\n\n"

               "In order to route the channel to another channel, select the "
               "FX channel "
               "and click on the \"send\" button on the channel you want to "
               "send to. "
               "The knob under the send button controls the level of signal "
               "that is sent "
               "to the channel.\n\n"

               "You can remove and move FX channels in the context menu, "
               "which is accessed "
               "by right-clicking the FX channel.\n"));

    QString name = Engine::fxMixer()->effectChannel(channelIndex())->name();
    setToolTip(name);

    m_renameLineEdit = new QLineEdit();
    m_renameLineEdit->setText(name);
    m_renameLineEdit->setFixedWidth(65);
    // m_renameLineEdit->setFont( pointSizeF( font(), 7.5f ) );
    m_renameLineEdit->setReadOnly(true);

    QGraphicsScene* scene = new QGraphicsScene();
    scene->setSceneRect(0, 0, 33, FxLineHeight);

    m_view = new QGraphicsView(this);
    m_view->setStyleSheet("border-style: none; background: transparent;");
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_view->setScene(scene);

    QGraphicsProxyWidget* proxyWidget = scene->addWidget(m_renameLineEdit);
    proxyWidget->setRotation(-90);
    proxyWidget->setPos(6, 145);  // 8

    connect(m_renameLineEdit, SIGNAL(editingFinished()), this,
            SLOT(renameFinished()));
    connect(m_sendBtn, SIGNAL(sendModelChanged(int)), this, SLOT(refresh()));
    connect(m_currentLine, SIGNAL(dataChanged()), this, SLOT(refresh()));
    connect(&m_channelIndex, SIGNAL(dataChanged()), this, SLOT(refresh()));
}

FxLine::~FxLine()
{
    DELETE_HELPER(m_sendKnob);
    DELETE_HELPER(m_sendBtn);
    DELETE_HELPER(m_lcd);
}

int FxLine::currentLine()
{
    return m_currentLine->value();
}

void FxLine::setCurrentLine(int index)
{
    m_currentLine->setValue(index);
}

int FxLine::channelIndex()
{
    return m_channelIndex.value();
}

void FxLine::setChannelIndex(int index)
{
    m_channelIndex.setValue(index);
    m_lcd->setValue(channelIndex());
    // m_lcd->update();
}

void FxLine::drawFxLine(QPainter*     p,
                        const FxLine* fxLine,
                        bool          isActive,
                        bool          sendToThis,
                        bool          receiveFromThis)
{
    QString name = Engine::fxMixer()->effectChannel(channelIndex())->name();
    QString elidedName = elideName(name);

    if(!m_inRename && m_renameLineEdit->text() != elidedName)
        m_renameLineEdit->setText(elidedName);

    int width  = fxLine->rect().width();
    int height = fxLine->rect().height();

    p->fillRect(fxLine->rect(),
                isActive ? fxLine->backgroundActive() : p->background());

    // inner border
    p->setPen(isActive ? fxLine->strokeInnerActive()
                       : fxLine->strokeInnerInactive());
    p->drawRect(1, 1, width - 3, height - 3);

    // outer border
    p->setPen(isActive ? fxLine->strokeOuterActive()
                       : fxLine->strokeOuterInactive());
    p->drawRect(0, 0, width - 1, height - 1);

    // draw the mixer send background
    if(sendToThis)
        p->drawPixmap(2, 0, 29, 56, s_sendBgArrow);
    else if(receiveFromThis)
        p->drawPixmap(2, 0, 29, 56, s_receiveBgArrow);
}

QString FxLine::elideName(const QString& name)
{
    const int    maxTextHeight = 60;
    QFontMetrics metrics(m_renameLineEdit->font());
    QString      elidedName
            = metrics.elidedText(name, Qt::ElideRight, maxTextHeight);
    return elidedName;
}

void FxLine::paintEvent(QPaintEvent*)
{
    bool sendToThis = Engine::fxMixer()->channelSendModel(currentLine(),
                                                          channelIndex())
                      != nullptr;
    bool receiveFromThis = Engine::fxMixer()->channelSendModel(channelIndex(),
                                                               currentLine())
                           != nullptr;
    QPainter p(this);
    drawFxLine(&p, this, currentLine() == channelIndex(), sendToThis,
               receiveFromThis);
}

void FxLine::mousePressEvent(QMouseEvent*)
{
    setCurrentLine(channelIndex());
}

void FxLine::mouseDoubleClickEvent(QMouseEvent*)
{
    renameChannel();
}

void FxLine::contextMenuEvent(QContextMenuEvent*)
{
    QPointer<CaptionMenu> contextMenu = new CaptionMenu(
            Engine::fxMixer()->effectChannel(channelIndex())->name(), this);
    if(channelIndex() != 0)  // no move-options in master
    {
        contextMenu->addAction(tr("Move &left"), this,
                               SLOT(moveChannelLeft()));
        contextMenu->addAction(tr("Move &right"), this,
                               SLOT(moveChannelRight()));
    }
    contextMenu->addAction(tr("Rename &channel"), this,
                           SLOT(renameChannel()));
    contextMenu->addSeparator();

    if(channelIndex() != 0)  // no remove-option in master
    {
        contextMenu->addAction(embed::getIcon("cancel"),
                               tr("R&emove channel"), this,
                               SLOT(removeChannel()));
        contextMenu->addSeparator();
    }
    contextMenu->addAction(embed::getIcon("cancel"),
                           tr("Remove &unused channels"), this,
                           SLOT(removeUnusedChannels()));

    contextMenu->addHelpAction();
    contextMenu->exec(QCursor::pos());
    delete contextMenu;
}

void FxLine::renameChannel()
{
    m_inRename = true;
    setToolTip("");
    m_renameLineEdit->setReadOnly(false);
    m_lcd->hide();
    m_renameLineEdit->setFixedWidth(135);
    m_renameLineEdit->setText(
            Engine::fxMixer()->effectChannel(channelIndex())->name());
    m_view->setFocus();
    m_renameLineEdit->selectAll();
    m_renameLineEdit->setFocus();
}

void FxLine::renameFinished()
{
    m_inRename = false;
    m_renameLineEdit->deselect();
    m_renameLineEdit->setReadOnly(true);
    m_renameLineEdit->setFixedWidth(65);
    m_lcd->show();
    QString newName = m_renameLineEdit->text();
    setFocus();
    if(!newName.isEmpty()
       && Engine::fxMixer()->effectChannel(channelIndex())->name() != newName)
    {
        Engine::fxMixer()->effectChannel(channelIndex())->setName(newName);
        m_renameLineEdit->setText(elideName(newName));
        Engine::getSong()->setModified();
    }
    QString name = Engine::fxMixer()->effectChannel(channelIndex())->name();
    setToolTip(name);
}

void FxLine::removeChannel()
{
    FxMixerView* mix = gui->fxMixerView();
    mix->deleteChannel(channelIndex());
}

void FxLine::removeUnusedChannels()
{
    FxMixerView* mix = gui->fxMixerView();
    mix->deleteUnusedChannels();
}

void FxLine::moveChannelLeft()
{
    FxMixerView* mix = gui->fxMixerView();
    mix->moveChannelLeft(channelIndex());
}

void FxLine::moveChannelRight()
{
    FxMixerView* mix = gui->fxMixerView();
    mix->moveChannelRight(channelIndex());
}

void FxLine::displayHelp()
{
    QWhatsThis::showText(mapToGlobal(rect().bottomRight()), whatsThis());
}

QBrush FxLine::backgroundActive() const
{
    return m_backgroundActive;
}

void FxLine::setBackgroundActive(const QBrush& c)
{
    m_backgroundActive = c;
}

QColor FxLine::strokeOuterActive() const
{
    return m_strokeOuterActive;
}

void FxLine::setStrokeOuterActive(const QColor& c)
{
    m_strokeOuterActive = c;
}

QColor FxLine::strokeOuterInactive() const
{
    return m_strokeOuterInactive;
}

void FxLine::setStrokeOuterInactive(const QColor& c)
{
    m_strokeOuterInactive = c;
}

QColor FxLine::strokeInnerActive() const
{
    return m_strokeInnerActive;
}

void FxLine::setStrokeInnerActive(const QColor& c)
{
    m_strokeInnerActive = c;
}

QColor FxLine::strokeInnerInactive() const
{
    return m_strokeInnerInactive;
}

void FxLine::setStrokeInnerInactive(const QColor& c)
{
    m_strokeInnerInactive = c;
}

void FxLine::refresh()
{
    qInfo("FxLine::refresh current=%d index=%d START", currentLine(),
          channelIndex());

    if(m_sendKnob == nullptr)
        qWarning("FxLine::refresh m_sendKnob is null");
    if(m_sendBtn == nullptr)
        qWarning("FxLine::refresh m_sendBtn is null");

    FxMixer*   mix = Engine::fxMixer();
    RealModel* sendModel
            = mix->channelSendModel(currentLine(), channelIndex());
    if(sendModel == nullptr)
    {
        // does not send, hide send knob
        m_sendKnob->setVisible(false);
        m_sendKnob->setModel(RealModel::createDefaultConstructed());
    }
    else
    {
        // it does send, show knob and connect
        m_sendKnob->setVisible(true);
        if(m_sendKnob->model() != sendModel)
            m_sendKnob->setModel(sendModel);
    }

    // disable the send button if it would cause an infinite loop
    m_sendBtn->setVisible(
            !mix->isInfiniteLoop(currentLine(), channelIndex()));
    m_sendBtn->updateLightStatus();
    update();

    qInfo("FxLine::refresh END");
}
