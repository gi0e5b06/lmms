/*
 * AutomatableButton.cpp - implementation of class AutomatableButton and
 *                          AutomatableButtonGroup
 *
 * Copyright (c) 2006-2011 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "AutomatableButton.h"

#include "CaptionMenu.h"
#include "Engine.h"
#include "ProjectJournal.h"
#include "StringPairDrag.h"

#include <QInputDialog>
#include <QMouseEvent>

AutomatableButton::AutomatableButton(BoolModel* _model, QWidget* _parent) :
      QPushButton(_parent), BoolModelView(_model, this), m_group(nullptr)
{
    initUi();
}

AutomatableButton::AutomatableButton(QWidget*       _parent,
                                     const QString& _displayName) :
      QPushButton(_parent),
      BoolModelView(this, _displayName),
      // new BoolModel(false, nullptr, _displayName, _objectName, true),
      // this),
      m_group(nullptr)
{
    initUi();
}

AutomatableButton::~AutomatableButton()
{
    if(m_group != nullptr)
        m_group->removeButton(this);
}

void AutomatableButton::initUi()
{
    BoolModel* m = castModel<BoolModel>();
    setObjectName(m->objectName() + "Button");
    setWindowTitle(m->displayName());
    // setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
}

void AutomatableButton::modelChanged()
{
    BoolModel* m = model();
    // if(m == nullptr) return;

    setWindowTitle(m->displayName());
    if(QPushButton::isChecked() != m->rawValue())
        QPushButton::setChecked(m->rawValue());
}

void AutomatableButton::update()
{
    BoolModel* m = model();
    // if(m == nullptr) return;

    if(QPushButton::isChecked() != m->rawValue())
        QPushButton::setChecked(m->rawValue());

    QPushButton::update();
}

void AutomatableButton::setChecked(bool _on)
{
    BoolModel* m = model();
    // if(m == nullptr) return;

    // QPushButton::setChecked is called in update-slot
    m->setValue(_on);
}

void AutomatableButton::contextMenuEvent(QContextMenuEvent* _me)
{
    // for the case, the user clicked right while pressing left mouse-
    // button, the context-menu appears while mouse-cursor is still hidden
    // and it isn't shown again until user does something which causes
    // an QApplication::restoreOverride Cursor()-call...
    mouseReleaseEvent(nullptr);

    if(m_group != nullptr)
    {
        CaptionMenu contextMenu(m_group->model()->displayName());
        m_group->addDefaultActions(&contextMenu);
        contextMenu.exec(QCursor::pos());
    }
    else
    {
        CaptionMenu contextMenu(model()->displayName());
        addDefaultActions(&contextMenu);
        contextMenu.exec(QCursor::pos());
    }
}

void AutomatableButton::mousePressEvent(QMouseEvent* _me)
{
    if(!isInteractive())
        return;

    if(_me->button() == Qt::LeftButton
       && !(_me->modifiers() & Qt::ControlModifier))
    {
        // User simply clicked, toggle if needed
        if(isCheckable())
        {
            toggle();
        }
        _me->accept();
    }
    else
    {
        // Ctrl-clicked, need to prepare drag-drop
        if(m_group != nullptr)
        {
            // A group, we must get process it instead
            AutomatableModelView* groupView = (AutomatableModelView*)m_group;
            new StringPairDrag("automatable_model",
                               QString::number(groupView->model()->id()),
                               QPixmap(), widget());
            // TODO: ^^ Maybe use a predefined icon instead of the button they
            // happened to select
            _me->accept();
        }
        else
        {
            // Otherwise, drag the standalone button
            AutomatableModelView::mousePressEvent(_me);
            QPushButton::mousePressEvent(_me);
        }
    }
}

void AutomatableButton::mouseReleaseEvent(QMouseEvent* _me)
{
    if(!isInteractive())
        return;

    if(_me && _me->button() == Qt::LeftButton)
        emit clicked();
}

void AutomatableButton::dropEvent(QDropEvent* _de)
{
    BoolModel* m = model();
    // if(m == nullptr) return;

    QString type = StringPairDrag::decodeKey(_de);
    QString val  = StringPairDrag::decodeValue(_de);
    if(type == "float_value")
    {
        m->addJournalCheckPoint();
        m->setValue(val.toFloat());
        _de->accept();
    }
    else if(type == "automatable_model")
    {
        AutomatableModel* mod = dynamic_cast<AutomatableModel*>(
                Engine::projectJournal()->journallingObject(val.toInt()));
        if(mod != nullptr)
        {
            AutomatableModel::linkModels(m, mod);
            mod->setValue(m->rawValue());
        }
    }
}

void AutomatableButton::toggle()
{
    BoolModel* m = model();
    // if(m == nullptr) return;

    if(isCheckable() && m_group != nullptr)
    {
        if(m->rawValue() == false)
        {
            m_group->activateButton(this);
        }
    }
    else
    {
        m->addJournalCheckPoint();
        m->setValue(!m->rawValue());
    }
}

void AutomatableButton::enterValue()
{
    BoolModel* m = model();
    // if(m == nullptr) return;
    if(!isInteractive())
        return;

    bool ok;
    bool new_val;

    new_val = QInputDialog::getInt(this, windowTitle(),
                                   tr("Please enter a new value between "
                                      "%1 and %2:")
                                           .arg(m->minValue())
                                           .arg(m->maxValue()),
                                   m->rawValue(),  // getRoundedValue(),
                                   m->minValue(), m->maxValue(),
                                   0,  // m->getDigitCount(),
                                   &ok);

    if(ok)
    {
        m->addJournalCheckPoint();
        m->setValue(new_val);
    }
}

AutomatableButtonGroup::AutomatableButtonGroup(QWidget*       _parent,
                                               const QString& _displayName,
                                               const QString& _objectName) :
      QWidget(_parent),
      IntModelView(this, _displayName)
// new IntModel(0, 0, 0, nullptr, _displayName, _objectName, true),
//      this)
{
    hide();
    setWindowTitle(_displayName);
}

AutomatableButtonGroup::~AutomatableButtonGroup()
{
    /*
    for(QList<AutomatableButton*>::iterator it = m_buttons.begin();
        it != m_buttons.end(); ++it)
    {
        (*it)->m_group = nullptr;
    }
    */
    for(AutomatableButton* btn: m_buttons)
        btn->m_group = nullptr;
}

void AutomatableButtonGroup::doConnections()
{
    IntModelView::doConnections();
    IntModel* m = model();
    connect(m, SIGNAL(dataChanged()), this, SLOT(updateButtons()));
}

void AutomatableButtonGroup::undoConnections()
{
    IntModelView::undoConnections();
    IntModel* m = model();
    disconnect(m, SIGNAL(dataChanged()), this, SLOT(updateButtons()));
}

void AutomatableButtonGroup::addButton(AutomatableButton* _btn)
{
    _btn->m_group = this;
    _btn->setCheckable(true);
    _btn->model()->setValue(false);
    // disable journalling as we're recording changes of states of
    // button-group members on our own
    _btn->model()->setJournalling(false);
    m_buttons.append(_btn);

    IntModel* m = model();
    // if(m != nullptr)
    m->setRange(0, m_buttons.size() - 1);
    updateButtons();
}

void AutomatableButtonGroup::removeButton(AutomatableButton* _btn)
{
    m_buttons.erase(qFind(m_buttons.begin(), m_buttons.end(), _btn));
    _btn->m_group = nullptr;
}

void AutomatableButtonGroup::activateButton(AutomatableButton* _btn)
{
    IntModel* m = model();
    // if(m == nullptr) return;

    if(_btn != m_buttons[m->rawValue()] && m_buttons.indexOf(_btn) != -1)
    {
        m->setValue(m_buttons.indexOf(_btn));
        for(AutomatableButton* btn: m_buttons)
            btn->update();
    }
}

void AutomatableButtonGroup::modelChanged()
{
    IntModel* m = model();
    // if(m != nullptr)
    //{
    setWindowTitle(m->displayName());
    //}
    IntModelView::modelChanged();
    updateButtons();
}

void AutomatableButtonGroup::updateButtons()
{
    IntModel* m = model();
    if(m == nullptr)
        return;

    m->setRange(0, m_buttons.size() - 1);
    int i = 0;
    for(AutomatableButton* btn: m_buttons)
    {
        btn->model()->setValue(i == m->rawValue());
        ++i;
    }
}

void AutomatableButtonGroup::enterValue()
{
    IntModel* m = model();
    if(m == nullptr)
        return;

    bool ok;
    bool new_val;

    new_val = QInputDialog::getInt(this, windowTitle(),
                                   tr("Please enter a new value between "
                                      "%1 and %2:")
                                           .arg(m->minValue())
                                           .arg(m->maxValue()),
                                   m->rawValue(),  // m->getRoundedValue(),
                                   m->minValue(), m->maxValue(),
                                   0,  // m->getDigitCount(),
                                   &ok);

    if(ok)
        m->setValue(new_val);
}
