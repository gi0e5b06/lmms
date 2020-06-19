/*
 * AutomatableModelView.cpp - implementation of AutomatableModelView
 *
 * Copyright (c) 2011-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include <QApplication>
#include <QClipboard>
//#include <QInputDialog>
#include "AutomatableModelView.h"
#include "AutomationEditor.h"
#include "AutomationPattern.h"
#include "ControllerConnection.h"
#include "ControllerConnectionDialog.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "StringPairDrag.h"
#include "embed.h"

#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>

static float floatFromClipboard(bool* ok = nullptr);

AutomatableModelView::AutomatableModelView(::Model* model, QWidget* _this) :
      ModelView(model, _this), m_interactive(true),
      m_description(QString::null), m_unit(QString::null)
{
    widget()->setAcceptDrops(true);
    widget()->setCursor(QCursor(embed::getIconPixmap("hand"), 3, 3));
}

AutomatableModelView::~AutomatableModelView()
{
}

bool AutomatableModelView::isInteractive() const
{
    return m_interactive;
}

void AutomatableModelView::setInteractive(bool _b)
{
    if(m_interactive == _b)
        return;

    m_interactive = _b;
    widget()->setAcceptDrops(_b);
    widget()->update();
}

void AutomatableModelView::addDefaultActions(QMenu*     menu,
                                             const bool _interactive)
{
    const AutomatableModel* model = modelUntyped();
    if(!model)
        return;

    AutomatableModelViewSlots* amvSlots
            = new AutomatableModelViewSlots(this, menu);

    QAction* a;

    a = menu->addAction(embed::getIcon("edit_rename"),
                        AutomatableModel::tr("Enter &Value"), amvSlots,
                        SLOT(enterValue()));
    a->setEnabled(_interactive);

    a = menu->addAction(
            embed::getIcon("reload"),
            AutomatableModel::tr("&Reset (%1%2)")
                    .arg(model->displayValue(model->initValue<float>()))
                    .arg(m_unit),
            model, SLOT(reset()));
    a->setEnabled(_interactive);

    FloatModel* fm = dynamic_cast<FloatModel*>(modelUntyped());
    if(fm != nullptr)
        menu->addAction(embed::getIcon("edit_rename"),
                        AutomatableModel::tr("Edit &Randomization (%1%2)")
                                .arg(fm->randomRatio() * 100.)
                                .arg("%"),
                        amvSlots, SLOT(editRandomization()));

    menu->addSeparator();
    menu->addAction(embed::getIcon("edit_copy"),
                    AutomatableModel::tr("&Copy value (%1%2)")
                            .arg(model->displayValue(model->value<float>()))
                            .arg(m_unit),
                    amvSlots, SLOT(copyToClipboard()));

    bool          canPaste     = true;
    const float   valueToPaste = floatFromClipboard(&canPaste);
    const QString pasteDesc
            = canPaste ? AutomatableModel::tr("&Paste value (%1%2)")
                                 .arg(model->displayValue(valueToPaste))
                                 .arg(m_unit)
                       : AutomatableModel::tr("&Paste value");
    QAction* pasteAction
            = menu->addAction(embed::getIcon("edit_paste"), pasteDesc,
                              amvSlots, SLOT(pasteFromClipboard()));
    pasteAction->setEnabled(canPaste && _interactive);

    menu->addSeparator();
    if(model->hasLinkedModels())
    {
        menu->addAction(embed::getIcon("edit_erase"),
                        AutomatableModel::tr("Remove all linked controls"),
                        amvSlots, SLOT(unlinkAllModels()));
    }

    QString controllerTxt;
    if(model->controllerConnection())
    {
        Controller* cont = model->controllerConnection()->getController();
        if(cont)
        {
            controllerTxt = AutomatableModel::tr("Connected to %1")
                                    .arg(cont->name());
        }
        else
        {
            controllerTxt = AutomatableModel::tr("Connected to controller");
        }

        QMenu* contMenu = menu->addMenu(embed::getIconPixmap("controller"),
                                        controllerTxt);

        contMenu->addAction(embed::getIcon("controller"),
                            AutomatableModel::tr("Edit connection..."),
                            amvSlots, SLOT(execConnectionDialog()));
        contMenu->addAction(embed::getIcon("cancel"),
                            AutomatableModel::tr("Remove connection"),
                            amvSlots, SLOT(removeConnection()));
    }
    else
    {
        menu->addAction(embed::getIcon("controller"),
                        AutomatableModel::tr("Connect to controller..."),
                        amvSlots, SLOT(execConnectionDialog()));
    }

    menu->addSeparator();
    menu->addAction(embed::getIconPixmap("automation"),
                    AutomatableModel::tr("Edit song-global automation"),
                    amvSlots, SLOT(editSongGlobalAutomation()));

    menu->addAction(QPixmap(),
                    AutomatableModel::tr("Remove song-global automation"),
                    amvSlots, SLOT(removeSongGlobalAutomation()));
}

void AutomatableModelView::enterValue()
{
    qWarning("enterValue() not implemented");
}

void AutomatableModelView::editRandomization()
{
    qWarning("editRandomization() not implemented");
}

/*
void AutomatableModelView::setModel( Model* model, bool isOldModelValid )
{
        ModelView::setModel( model, isOldModelValid );
}
*/

void AutomatableModelView::mousePressEvent(QMouseEvent* event)
{
    AutomatableModel* m = modelUntyped();
    if(!m || !isInteractive())
        return;

    if(event->button() == Qt::LeftButton
       && event->modifiers() & Qt::ControlModifier)
    {
        new StringPairDrag("automatable_model", QString::number(m->id()),
                           QPixmap(), widget());
        event->accept();
    }
    else if(event->button() == Qt::MidButton)
    {
        m->reset();
    }
}

QColor AutomatableModelView::cableColor() const
{
    const AutomatableModel* m = modelUntyped();
    QColor                  r(255, 255, 255);

    if(!m)
        r = QColor(128, 128, 128);  // no model...
    else if(m->isAutomated())
        r = QColor(128, 128, 255);  // automation pattern
    else if(m->isControlled())
        r = QColor(255, 192, 0);  // midi
    else if(m->hasLinkedModels())
        r = QColor(128, 128, 255);  // link

    return r;
}

AutomatableModelViewSlots::AutomatableModelViewSlots(
        AutomatableModelView* amv, QObject* parent) :
      QObject(),
      m_amv(amv)
{
    connect(parent, SIGNAL(destroyed()), this, SLOT(deleteLater()),
            Qt::QueuedConnection);
}

void AutomatableModelViewSlots::enterValue()
{
    m_amv->enterValue();
}

void AutomatableModelViewSlots::editRandomization()
{
    m_amv->editRandomization();
}

void AutomatableModelViewSlots::execConnectionDialog()
{
    // TODO[pg]: Display a dialog with list of controllers currently in the
    // song in addition to any system MIDI controllers
    AutomatableModel* m = m_amv->modelUntyped();

    m->displayName();
    ControllerConnectionDialog d(gui->mainWindow(), m);

    if(d.exec() == 1)
    {
        // Actually chose something
        if(d.chosenController())
        {
            // Update
            if(m->controllerConnection())
            {
                m->controllerConnection()->setController(
                        d.chosenController());
            }
            // New
            else
            {
                ControllerConnection* cc
                        = new ControllerConnection(d.chosenController());
                m->setControllerConnection(cc);
                // cc->setTargetName( m->displayName() );
            }
        }
        // no controller, so delete existing connection
        else
        {
            removeConnection();
        }
    }
}

void AutomatableModelViewSlots::removeConnection()
{
    AutomatableModel* m = m_amv->modelUntyped();

    if(m->controllerConnection())
    {
        delete m->controllerConnection();
        m->setControllerConnection(nullptr);
    }
}

void AutomatableModelViewSlots::editSongGlobalAutomation()
{
    gui->automationWindow()->open(AutomationPattern::globalAutomationPattern(
            m_amv->modelUntyped()));
}

void AutomatableModelViewSlots::removeSongGlobalAutomation()
{
    delete AutomationPattern::globalAutomationPattern(m_amv->modelUntyped());
}

void AutomatableModelViewSlots::unlinkAllModels()
{
    m_amv->modelUntyped()->unlinkAllModels();
}

void AutomatableModelViewSlots::copyToClipboard()
{
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(m_amv->value<float>()));
}

void AutomatableModelViewSlots::pasteFromClipboard()
{
    bool        isNumber = false;
    const float number   = floatFromClipboard(&isNumber);
    if(isNumber)
    {
        m_amv->modelUntyped()->setValue(number);
    }
}

/// Attempt to parse a float from the clipboard
static float floatFromClipboard(bool* ok)
{
    const QClipboard* clipboard = QApplication::clipboard();
    return clipboard->text().toFloat(ok);
}
