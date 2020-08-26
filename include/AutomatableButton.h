/*
 * AutomatableButton.h - class automatableButton, the base for all buttons
 *
 * Copyright (c) 2006-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef AUTOMATABLE_BUTTON_H
#define AUTOMATABLE_BUTTON_H

#include "AutomatableModelView.h"

#include <QPushButton>

class AutomatableButtonGroup;

class EXPORT AutomatableButton : public QPushButton, public BoolModelView
{
    Q_OBJECT

  public:
    AutomatableButton(BoolModel* _model, QWidget* _parent);
    AutomatableButton(QWidget*       _parent      = nullptr,
                      const QString& _displayName = "[led checkbox]");

    virtual ~AutomatableButton();

    INLINE virtual void setCheckable(bool _on)
    {
        QPushButton::setCheckable(_on);
        model()->setJournalling(_on);
    }

    virtual void enterValue();

    void modelChanged() override;

  public slots:
    virtual void update();
    virtual void toggle();
    virtual void setChecked(bool _on);

  protected:
    virtual void initUi();

    void contextMenuEvent(QContextMenuEvent* _me) override;
    void dropEvent(QDropEvent* _de) override;
    void mousePressEvent(QMouseEvent* _me) override;
    void mouseReleaseEvent(QMouseEvent* _me) override;

  private:
    AutomatableButtonGroup* m_group;

    friend class AutomatableButtonGroup;

    using QPushButton::isChecked;
    using QPushButton::setChecked;
};

class EXPORT AutomatableButtonGroup final :
      public QWidget,
      public IntModelView
{
    Q_OBJECT

  public:
    AutomatableButtonGroup(QWidget*       _parent,
                           const QString& _displayName = "[button group]",
                           const QString& _objectName  = QString::null);
    virtual ~AutomatableButtonGroup();

    virtual void addButton(AutomatableButton* _btn);
    virtual void removeButton(AutomatableButton* _btn);
    virtual void activateButton(AutomatableButton* _btn);

    virtual void enterValue();

    void modelChanged() override;

  protected:
    void doConnections() override;
    void undoConnections() override;

  private slots:
    virtual void updateButtons();

  private:
    QList<AutomatableButton*> m_buttons;
};

#endif
