/*
 * LcdWidget.cpp - a widget for displaying numbers in LCD style
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2008 Paul Giblock <pgllama/at/gmail.com>
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

#include "LcdWidget.h"

//#include <QApplication>
//#include <QLabel>
//#include <QMouseEvent>
#include "embed.h"
#include "gui_templates.h"

#include <QPainter>
#include <QStyleOptionFrameV2>

//#include "MainWindow.h"

LcdWidget::LcdWidget(QWidget* parent, const QString& name) :
      LcdWidget(1, "19default", parent, name)
{
}

LcdWidget::LcdWidget(int numDigits, QWidget* parent, const QString& name) :
      LcdWidget(numDigits, "19default", parent, name)
{
}

LcdWidget::LcdWidget(int            numDigits,
                     const QString& style,
                     QWidget*       parent,
                     const QString& name) :
      Widget(parent),
      m_label(), m_textColor(255, 255, 255), m_textShadowColor(64, 64, 64),
      m_numDigits(numDigits)
{
    initUi(name, style);
}

LcdWidget::~LcdWidget()
{
    delete m_lcdPixmap;
}

void LcdWidget::setValue(int value)
{
    QString s = m_textForValue[value];
    if(s.isEmpty())
    {
        s = QString::number(value);
        // TODO: if pad == true
        /*
        while( (int) s.length() < m_numDigits )
        {
                s = "0" + s;
        }
        */
    }
    if(s != m_display)
    {
        m_display = s;
        LcdWidget::update();
    }
}

QColor LcdWidget::textColor() const
{
    return m_textColor;
}

void LcdWidget::setTextColor(const QColor& c)
{
    if(m_textColor != c)
    {
        m_textColor = c;
        update();
    }
}

QColor LcdWidget::textShadowColor() const
{
    return m_textShadowColor;
}

void LcdWidget::setTextShadowColor(const QColor& c)
{
    if(m_textShadowColor != c)
    {
        m_textShadowColor = c;
        update();
    }
}

void LcdWidget::drawWidget(QPainter& _p)
{
    QSize cellSize(m_cellWidth, m_cellHeight);
    QRect cellRect(0, 0, m_cellWidth, m_cellHeight);

    int margin = 1;  // QStyle::PM_DefaultFrameWidth;
    // int lcdWidth = m_cellWidth * m_numDigits + (margin*m_marginWidth)*2;

    //_p.translate( width() / 2 - lcdWidth / 2, 0 );
    _p.save();

    _p.translate(margin, margin);

    // Left Margin
    _p.drawPixmap(cellRect, *m_lcdPixmap,
                  QRect(QPoint(charsPerPixmap * m_cellWidth,
                               isEnabled() ? 0 : m_cellHeight),
                        cellSize));

    _p.translate(m_marginWidth, 0);

    // Padding
    for(int i = 0; i < m_numDigits - m_display.length(); i++)
    {
        _p.drawPixmap(cellRect, *m_lcdPixmap,
                      QRect(QPoint(10 * m_cellWidth,
                                   isEnabled() ? 0 : m_cellHeight),
                            cellSize));
        _p.translate(m_cellWidth, 0);
    }

    // Digits
    for(int i = 0; i < m_display.length(); i++)
    {
        int val = m_display[i].digitValue();
        if(val < 0)
        {
            if(m_display[i] == '-')
                val = 11;
            else
                val = 10;
        }
        _p.drawPixmap(cellRect, *m_lcdPixmap,
                      QRect(QPoint(val * m_cellWidth,
                                   isEnabled() ? 0 : m_cellHeight),
                            cellSize));
        _p.translate(m_cellWidth, 0);
    }

    // Right Margin
    _p.drawPixmap(QRect(0, 0, m_marginWidth - 1, m_cellHeight), *m_lcdPixmap,
                  QRect(charsPerPixmap * m_cellWidth,
                        isEnabled() ? 0 : m_cellHeight, m_cellWidth / 2,
                        m_cellHeight));

    _p.restore();

    // Border
    QStyleOptionFrame opt;
    opt.initFrom(this);
    opt.state = QStyle::State_Sunken;
    opt.rect  = QRect(0, 0,
                     m_cellWidth * m_numDigits + (margin + m_marginWidth) * 2
                             - 1,
                     m_cellHeight + (margin * 2));

    style()->drawPrimitive(QStyle::PE_Frame, &opt, &_p, this);

    _p.resetTransform();

    // Label
    if(!m_label.isEmpty())
    {
        _p.setRenderHints( QPainter::Antialiasing, true );
        //_p.setFont(pointSizeF(_p.font(), 7));  // 6.5 ) );
        _p.setFont(pointSizeF(font(),7.f));
        /*
        p.setPen( textShadowColor() );
        p.drawText( width() / 2 -
                        _p.fontMetrics().width( m_label ) / 2 + 1,
                                        height(), m_label );
        */
        const QFontMetrics mx = _p.fontMetrics();
        int                xt = width() / 2 - mx.width(m_label) / 2;
        int                yt = m_cellHeight + margin + mx.height() - 4;
        _p.setPen(textColor());
        _p.drawText(xt, yt, m_label);
        _p.drawText(xt, yt, m_label); // twice for readability
    }
}

void LcdWidget::setLabel(const QString& _s)
{
    setText(_s);
}

QString LcdWidget::text() const
{
    return m_label;
}

void LcdWidget::setText(const QString& _s)
{
    if(m_label != _s)
    {
        m_label = _s;
        invalidateCache();
        updateSize();
    }
}

void LcdWidget::setMarginWidth(int _width)
{
    if(m_marginWidth != _width)
    {
        m_marginWidth = _width;
        updateSize();
    }
}

void LcdWidget::updateSize()
{
    const int margin = 1;
    int       w, h;
    if(m_label.isEmpty())
    {
        w = m_cellWidth * m_numDigits + 2 * (margin + m_marginWidth);
        h = m_cellHeight + (2 * margin);
    }
    else
    {
        w = qMax<int>(m_cellWidth * m_numDigits
                              + 2 * (margin + m_marginWidth),
                      QFontMetrics(pointSizeF(font(), 6.5)).width(m_label));
        h = m_cellHeight + (2 * margin) + 9;
    }
    if(w != width() || h != height() || !isCacheValid())
    {
        setFixedSize(w, h);
        update();
    }
}

QRect LcdWidget::displayRect()
{
    int margin = 1;
    return QRect(margin, margin,
                 m_cellWidth * m_numDigits + 2 * m_marginWidth, m_cellHeight);
}

void LcdWidget::initUi(const QString& name, const QString& style)
{
    setEnabled(true);

    setWindowTitle(name);

    // We should make a factory for these or something.
    // m_lcdPixmap = new QPixmap( embed::getIconPixmap( QString( "lcd_" +
    // style ).toUtf8().constData() ) ); m_lcdPixmap = new QPixmap(
    // embed::getIconPixmap( "lcd_19green" ) ); // TODO!!
    m_lcdPixmap = new QPixmap(embed::getIconPixmap(
            QString("lcd_" + style).toUtf8().constData()));

    m_cellWidth  = m_lcdPixmap->size().width() / LcdWidget::charsPerPixmap;
    m_cellHeight = m_lcdPixmap->size().height() / 2;

    m_marginWidth = m_cellWidth / 2;

    updateSize();
}
