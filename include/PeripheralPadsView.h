/*
 * PeripheralPadsView.h - declaration of PeripheralPadsView, an interactive
 * pads widget (like the MPD218)
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2010 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef PERIPHERAL_PADS_VIEW_H
#define PERIPHERAL_PADS_VIEW_H

#include <QPixmap>
#include <QScrollBar>

//#include "ModelView.h"
#include "PeripheralView.h"

class Piano;

class PeripheralPadsView : public PeripheralView
// public QWidget, public ModelView
{
    Q_OBJECT

  public:
    PeripheralPadsView(Piano* _piano, QWidget* _parent);
    virtual ~PeripheralPadsView();

    void keyPressEvent(QKeyEvent* ke) override;
    void keyReleaseEvent(QKeyEvent* ke) override;

  protected:
    void contextMenuEvent(QContextMenuEvent* _me) override;
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* me) override;
    void mouseReleaseEvent(QMouseEvent* me) override;
    void mouseMoveEvent(QMouseEvent* me) override;
    void focusOutEvent(QFocusEvent* _fe) override;
    void resizeEvent(QResizeEvent* _event) override;

  private:
    int getKeyFromMouse(const QPoint& _p) const;
    // int getKeyX( int _key_num ) const;
    QRect getBankRect(int _b) const;
    QRect getPadRect(int _k) const;

    // static QPixmap * s_padPm;
    // static QPixmap * s_padPressedPm;

    QScrollBar* m_pianoScroll;
    int         m_startKey;  // first key when drawing
    int         m_lastKey;

  private slots:
    void pianoScrolled(int _newPos);

  signals:
    void keyPressed(int);
    void baseNoteChanged();
};

#endif
