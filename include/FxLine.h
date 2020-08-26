/*
 * FxLine.h - FX line widget
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

#ifndef FX_LINE_H
#define FX_LINE_H

#include "Knob.h"
#include "LcdWidget.h"
#include "SendButtonIndicator.h"

#include <QGraphicsView>
#include <QLineEdit>
#include <QPointer>
#include <QWidget>

class FxMixerView;
class SendButtonIndicator;

class FxLine : public QWidget
{
    Q_OBJECT

  public:
    Q_PROPERTY(QBrush backgroundActive READ backgroundActive WRITE
                       setBackgroundActive)
    Q_PROPERTY(QColor strokeOuterActive READ strokeOuterActive WRITE
                       setStrokeOuterActive)
    Q_PROPERTY(QColor strokeOuterInactive READ strokeOuterInactive WRITE
                       setStrokeOuterInactive)
    Q_PROPERTY(QColor strokeInnerActive READ strokeInnerActive WRITE
                       setStrokeInnerActive)
    Q_PROPERTY(QColor strokeInnerInactive READ strokeInnerInactive WRITE
                       setStrokeInnerInactive)

    FxLine(QWidget* _parent, IntModel* _currentLine, int _channelIndex);
    virtual ~FxLine();

    int  currentLine();
    void setCurrentLine(int index);
    int  channelIndex();
    void setChannelIndex(int index);

    QPointer<Knob>                m_sendKnob;
    QPointer<SendButtonIndicator> m_sendBtn;

    QBrush backgroundActive() const;
    void   setBackgroundActive(const QBrush& c);

    QColor strokeOuterActive() const;
    void   setStrokeOuterActive(const QColor& c);

    QColor strokeOuterInactive() const;
    void   setStrokeOuterInactive(const QColor& c);

    QColor strokeInnerActive() const;
    void   setStrokeInnerActive(const QColor& c);

    QColor strokeInnerInactive() const;
    void   setStrokeInnerInactive(const QColor& c);

    static const int FxLineHeight;

  public slots:
    void refresh();

  protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;

  private:
    void drawFxLine(QPainter*     p,
                    const FxLine* fxLine,
                    bool          isActive,
                    bool          sendToThis,
                    bool          receiveFromThis);

    QString elideName(const QString& name);

    QPointer<IntModel>  m_currentLine;
    IntModel            m_channelIndex;
    QPointer<LcdWidget> m_lcd;
    QBrush              m_backgroundActive;
    QColor              m_strokeOuterActive;
    QColor              m_strokeOuterInactive;
    QColor              m_strokeInnerActive;
    QColor              m_strokeInnerInactive;
    bool                m_inRename;
    QLineEdit*          m_renameLineEdit;
    QGraphicsView*      m_view;

    // static QPixmap* s_sendBgArrow;
    // static QPixmap* s_receiveBgArrow;

  private slots:
    void renameChannel();
    void renameFinished();
    void removeChannel();
    void removeUnusedChannels();
    void moveChannelLeft();
    void moveChannelRight();
    void displayHelp();
};

#endif  // FXLINE_H
