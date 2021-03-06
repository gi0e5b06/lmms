/*
 * LedCheckBox.h - class LedCheckBox, an improved QCheckBox
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2005-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef LED_CHECKBOX_H
#define LED_CHECKBOX_H

#include "AutomatableButton.h"

class QPixmap;

class EXPORT LedCheckBox final : public AutomatableButton
{
    Q_OBJECT

  public:
    static constexpr auto _LCSL_ = __LINE__;
    enum LedColors
    {
        Yellow,
        Green,    // param / mute
        Red,      // alert / clipping
        Blue,     // freeze
        Magenta,  // solo
        White     //,
        // NumColors
    };
    static constexpr auto LedColorCount = __LINE__ - _LCSL_ - 4;

    LedCheckBox(BoolModel* _model, QWidget* _parent);
    LedCheckBox(QWidget*       _parent      = nullptr,
                const QString& _displayName = "[led checkbox]");

    // obsolete
    LedCheckBox(const QString& _txt,
                QWidget*       _parent,
                const QString& _displayName = "[led checkbox]",
                LedColors      _color       = Yellow);
    LedCheckBox(QWidget*       _parent,
                const QString& _displayName,
                LedColors      _color = Yellow);
    LedCheckBox(QWidget* _parent, LedColors _color);

    virtual ~LedCheckBox();

    QString text() const;
    void    setText(const QString& _s);
    // Q_PROPERTY( QString text READ text WRITE setText )

    Qt::AnchorPoint textAnchorPoint() const;
    void            setTextAnchorPoint(Qt::AnchorPoint _a);

    LedColors ledColor() const;
    void      setLedColor(LedColors _color);

    bool blinking() const;
    void setBlinking(bool _b);

    // virtual void enterValue();

  public slots:
    void update() override;

  protected:
    void initUi() override;
    void onTextUpdated();
    void onLedColorUpdated();
    void paintEvent(QPaintEvent* _pe) override;

  private:
    QString         m_text;
    Qt::AnchorPoint m_textAnchor;
    LedColors       m_ledColor;
    bool            m_blinkingState;
    bool            m_blinking;

    QPixmap* m_ledOnPixmap;
    QPixmap* m_ledOffPixmap;
};

#endif
