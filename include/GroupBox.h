/*
 * GroupBox.h - LMMS-groupbox
 *
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

#ifndef GROUP_BOX_H
#define GROUP_BOX_H

#include "AutomatableModelView.h"
#include "PixmapButton.h"

#include <QWidget>

class QLabel;
class QPixmap;

class GroupBox : public QWidget
//, public BoolModelView
{
    class Top : public QWidget
    {
      public:
        Top(const QString& _caption,
            GroupBox*      _parent = NULL,
            bool           _led    = true,
            bool           _arrow  = true);

        QLabel*       m_title;
        PixmapButton* m_led;
        PixmapButton* m_arrow;
    };

    Q_OBJECT

  public:
    GroupBox(const QString& _caption,
             QWidget*       _parent = NULL,
             bool           _led    = true,
             bool           _arrow  = true,
             bool           _panel  = true);
    virtual ~GroupBox();

    virtual void addTopWidget(QWidget* _w, int _col);
    // virtual BoolModel* model();
    // virtual void setModel(BoolModel* _model);

    virtual bool isEnabled();
    virtual void setEnabled(bool _b);

    QWidget*      contentWidget();
    PixmapButton* ledButton();
    int           titleBarHeight() const;

    virtual void enterValue();

  signals:
    void enabledChanged();

  public slots:
    void togglePanel();

  protected:
    // virtual void mousePressEvent(QMouseEvent* _me);
    virtual void paintEvent(QPaintEvent* _pe);
    virtual void resizeEvent(QResizeEvent* _re);

    QWidget* m_panel;

  private:
    void updatePixmap();

    Top*    m_top;
    QString m_caption;
};

typedef BoolModel groupBoxModel;

#endif
