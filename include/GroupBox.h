/*
 * GroupBox.h -
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
{
    class Top : public QWidget
    {
      public:
        Top(const QString& _title,
            GroupBox*      _parent = nullptr,
            bool           _led    = true,
            bool           _arrow  = true);

        QLabel*       m_title;
        PixmapButton* m_led;
        PixmapButton* m_arrow;
    };

    Q_OBJECT

  public:
    GroupBox(const QString& _title,
             QWidget*       _parent = nullptr,
             bool           _led    = true,
             bool           _arrow  = true);
    virtual ~GroupBox();

    virtual void addTopWidget(QWidget* _w, int _col);

    virtual bool isEnabled();
    virtual void setEnabled(bool _b);

    PixmapButton* ledButton();
    PixmapButton* arrowButton();

    QWidget* contentWidget();
    void     setContentWidget(QWidget* _w);

    QWidget* bottomWidget();
    void     setBottomWidget(QWidget* _w);

    // QString& title();
    // void setTitle(const QString& _title);

    int titleBarHeight() const;

    virtual void enterValue();

  signals:
    void enabledChanged();

  public slots:
    void togglePanel();

  protected:
    virtual void paintEvent(QPaintEvent* _pe);
    virtual void resizeEvent(QResizeEvent* _re);

    QWidget* m_panel;
    QWidget* m_bottom;

  private:
    void updatePixmap();

    Top*    m_top;
    QString m_title;
};

#endif
