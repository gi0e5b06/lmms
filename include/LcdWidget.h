/*
 * LcdWidget.h - a widget for displaying numbers in LCD style
 *
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

#ifndef LCD_WIDGET_H
#define LCD_WIDGET_H

#include "Widget.h"
#include "export.h"
#include "lmms_basics.h"

#include <QMap>
//#include <QWidget>

class EXPORT LcdWidget : public Widget
{
    Q_OBJECT

    // theming qproperties
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QColor textShadowColor READ textShadowColor WRITE
                       setTextShadowColor)

  public:
    LcdWidget(QWidget* parent, const QString& name = "[lcd widget]");
    LcdWidget(int            numDigits,
              QWidget*       parent,
              const QString& name = "[lcd widget]");
    LcdWidget(int            numDigits,
              const QString& style,
              QWidget*       parent,
              const QString& name = "[lcd widget]");

    virtual ~LcdWidget();

    void setValue(int value);

    void setLabel(const QString& label);  // deprecated

    QString text() const;
    void    setText(const QString& _s);

    void addTextForValue(int value, const QString& text)
    {
        m_textForValue[value] = text;
        update();
    }

    Q_PROPERTY(int numDigits READ numDigits WRITE setNumDigits)

    INLINE int numDigits() const
    {
        return m_numDigits;
    }

    INLINE void setNumDigits(int n)
    {
        m_numDigits = n;
        updateSize();
    }

    QColor textColor() const;
    void   setTextColor(const QColor& c);

    QColor textShadowColor() const;
    void   setTextShadowColor(const QColor& c);

  public slots:
    virtual void setMarginWidth(int width);

  protected:
    virtual void drawWidget(QPainter& _p);
    // virtual void paintEvent( QPaintEvent * pe );

    virtual void  updateSize();
    virtual QRect displayRect();
    /*
    int cellHeight() const
    {
            return m_cellHeight;
    }
    */

  private:
    static const int charsPerPixmap = 12;

    QMap<int, QString> m_textForValue;

    QString m_display;

    QString  m_label;
    QPixmap* m_lcdPixmap;

    QColor m_textColor;
    QColor m_textShadowColor;

    int m_cellWidth;
    int m_cellHeight;
    int m_numDigits;
    int m_marginWidth;

    void initUi(const QString& name, const QString& style);
};

#endif
