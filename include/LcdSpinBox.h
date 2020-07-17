/*
 * LcdSpinBox.h - class LcdSpinBox, an improved QLCDNumber
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef LCD_SPINBOX_H
#define LCD_SPINBOX_H

#include "AutomatableModelView.h"
#include "LcdWidget.h"

class EXPORT LcdSpinBox : public LcdWidget, public IntModelView
{
    Q_OBJECT
  public:
    LcdSpinBox(int            _numDigits,
               QWidget*       _parent,
               const QString& _displayName = "[lcd spinbox]",
               const QString& _objectName  = QString::null);

    LcdSpinBox(int            _numDigits,
               const QString& _style,
               QWidget*       _parent,
               const QString& _displayName = "[lcd spinbox]",
               const QString& _objectName  = QString::null);

    virtual ~LcdSpinBox();

    virtual void modelChanged();

    /*! Sets an offset which is always added to value of model so we can
        display values in a user-friendly way if they internally start at 0 */
    /*
      void setDisplayOffset( int offset )
    {
            m_displayOffset = offset;
    }
    */

    /*! \brief Returns internal offset for displaying values */
    /*
      int displayOffset() const
    {
            return m_displayOffset;
    }
    */
  public slots:
    virtual void update();

  protected:
    virtual void contextMenuEvent(QContextMenuEvent* _me);
    virtual void mousePressEvent(QMouseEvent* _me);
    virtual void mouseMoveEvent(QMouseEvent* _me);
    virtual void mouseReleaseEvent(QMouseEvent* _me);
    virtual void wheelEvent(QWheelEvent* _we);
    virtual void mouseDoubleClickEvent(QMouseEvent* _me);

    virtual void convert(const QPoint& _p, real_t& value_, real_t& dist_);
    virtual void setPosition(const QPoint& _p, bool _shift);

  private slots:
    virtual void enterValue();
    // void displayHelp();
    // void friendlyUpdate();
    // void toggleScale();

  private:
    real_t m_pressValue;  // model value when left button pressed
    QPoint m_pressPos;    // mouse pos when left button pressed
    bool   m_pressLeft;   // true when left button pressed

    // bool m_mouseMoving;
    // QPoint m_origMousePos;
    // int m_displayOffset;
    // void enterValue();

  signals:
    void manualChange();
};

typedef IntModel LcdSpinBoxModel;

#endif
