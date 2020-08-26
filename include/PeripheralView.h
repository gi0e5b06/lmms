/*
 * PeripheralView.h - declaration of PeripheralView, an interactive
 * peripheral/keyboard-widget
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

#ifndef PERIPHERAL_VIEW_H
#define PERIPHERAL_VIEW_H

#include "ModelView.h"
#include "Piano.h"

#include <QPixmap>
#include <QScrollBar>

class Piano;

class PeripheralView : public QWidget, public ModelView
{
    Q_OBJECT

  public:
    PeripheralView(Piano* _piano, QWidget* _parent);
    virtual ~PeripheralView();

    Piano* model()
    {
        return castModel<Piano>();
    }

    const Piano* model() const
    {
        return castModel<Piano>();
    }

    // void setModel(Model* _model) override;

    virtual void keyPressEvent(QKeyEvent* ke)   = 0;
    virtual void keyReleaseEvent(QKeyEvent* ke) = 0;

    static int getKeyFromKeyEvent(QKeyEvent* _ke);

  signals:
    void keyPressed(int);
    void baseNoteChanged();

  protected slots:
    virtual void modelChanged();

  protected:
    virtual void contextMenuEvent(QContextMenuEvent* _me);
    virtual void paintEvent(QPaintEvent*)           = 0;
    virtual void mousePressEvent(QMouseEvent* me)   = 0;
    virtual void mouseReleaseEvent(QMouseEvent* me) = 0;
    virtual void mouseMoveEvent(QMouseEvent* me)    = 0;
    virtual void focusOutEvent(QFocusEvent* _fe)    = 0;
    virtual void resizeEvent(QResizeEvent* _event)  = 0;

    // QPointer<Piano> m_piano;

    /*
    private:
    int getKeyFromMouse( const QPoint & _p ) const;
    int getKeyX( int _key_num ) const;

    static QPixmap* s_whiteKeyPm;
    static QPixmap* s_blackKeyPm;
    static QPixmap* s_whiteKeyPressedPm;
    static QPixmap* s_blackKeyPressedPm;

    QScrollBar* m_peripheralScroll;
    int m_startKey;			// first key when drawing
    int m_lastKey;
    */

    // private slots:
    // void peripheralScrolled( int _new_pos );
};

#endif
