/*
 * EditorOverlay.cpp -
 *
 * Copyright (c) 2020-2020 gi0e5b06 (on github.com)
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

#include "EditorOverlay.h"

#include "Backtrace.h"
#include "Configuration.h"
#include "embed.h"
//#include "gui_templates.h"
#include "GuiApplication.h"
#include "MainWindow.h"

#include <QApplication>
#include <QCursor>
#include <QPainter>
#include <QWidget>

EditorOverlay::EditorOverlay(QWidget* _parent, Editor* _editor) :
      Widget(_parent), m_editor(_editor), m_vlxs(-1), m_vlxe(-1), m_hlys(-1),
      m_hlye(-1)
{
    setGeometry(10, 10, 100, 100);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    if(CONFIG_GET_BOOL("ui.toolcursor"))
        connect(gui->mainWindow(), SIGNAL(periodicUpdate()), this,
                SLOT(check()), Qt::UniqueConnection);
}

void EditorOverlay::check()
{
    // qInfo("EditorOverlay::check");
    static QPoint old(-1, -1);
    static bool   in = true;

    QPoint q = mapFromGlobal(QCursor::pos());
    if(q != old || in)
    {
        in = rect().contains(q);
        update();
        old = q;
    }
}

void EditorOverlay::verticalLine(int _xs, int _xe)
{
    static bool in = false;
    if(_xs != m_vlxs || _xe != m_vlxe || in)
    {
        in     = (_xs >= 0 && _xs < width()) || (_xe >= 0 && _xe < width());
        m_vlxs = _xs;
        m_vlxe = _xe;
        update();
    }
}

void EditorOverlay::horizontalLine(int _ys, int _ye)
{
    static bool in = false;
    if(_ys != m_hlys || _ye != m_hlye || in)
    {
        in     = (_ys >= 0 && _ys < height()) || (_ye >= 0 && _ye < height());
        m_hlys = _ys;
        m_hlye = _ye;
        update();
    }
}

void EditorOverlay::drawModeCursor(QPainter&        _p,
                                   QWidget&         _w,
                                   Editor::EditMode _mode)
{
    static QPixmap* s_modeDraw   = nullptr;
    static QPixmap* s_modeErase  = nullptr;
    static QPixmap* s_modeSelect = nullptr;
    static QPixmap* s_modeMove   = nullptr;
    static QPixmap* s_modeSplit  = nullptr;
    static QPixmap* s_modeJoin   = nullptr;
    static QPixmap* s_modeDetune = nullptr;

    const QPixmap* cursor = nullptr;

    switch(_mode)
    {
        case Editor::ModeDraw:
            if(s_modeDraw == nullptr)
                s_modeDraw = new QPixmap(embed::getPixmap("edit_draw"));
            cursor = s_modeDraw;
            break;
        case Editor::ModeErase:
            if(s_modeErase == nullptr)
                s_modeErase = new QPixmap(embed::getPixmap("edit_erase"));
            cursor = s_modeErase;
            break;
        case Editor::ModeSelect:
            if(s_modeSelect == nullptr)
                s_modeSelect = new QPixmap(embed::getPixmap("edit_select"));
            cursor = s_modeSelect;
            break;
        case Editor::ModeMove:
            if(s_modeMove == nullptr)
                s_modeMove = new QPixmap(embed::getPixmap("edit_move"));
            cursor = s_modeMove;
            break;
        case Editor::ModeSplit:
            if(s_modeSplit == nullptr)
                s_modeSplit = new QPixmap(embed::getPixmap("edit_split"));
            cursor = s_modeSplit;
            break;
        case Editor::ModeJoin:
            if(s_modeJoin == nullptr)
                s_modeJoin = new QPixmap(embed::getPixmap("edit_join"));
            cursor = s_modeJoin;
            break;
        case Editor::ModeDetune:
            if(s_modeDetune == nullptr)
                s_modeDetune = new QPixmap(embed::getPixmap("automation"));
            cursor = s_modeDetune;
            break;
    }

    if(cursor != nullptr)
    {
        QPoint q = _w.mapFromGlobal(QCursor::pos());
        if(_w.rect().contains(q))
        {
            // static QPoint old(-1, -1);
            // int           d = (q - old).manhattanLength();
            // if(d !=< 2)
            _p.drawPixmap(q + QPoint(8, 8), *cursor);
            // else
            // old = q;
        }
        //_w.setCursor(QCursor(*cursor));
    }
}

void EditorOverlay::drawModeCursor(QPainter& _p, Editor::EditMode _mode)
{
    drawModeCursor(_p, *this, _mode);
}

void EditorOverlay::drawVerticalLine(QPainter& _p)
{
    if((m_vlxs >= 0 && m_vlxs < width()) || (m_vlxe >= 0 && m_vlxe < width()))
    {
        if(m_vlxe > m_vlxs)
        {
            int alpha = 64 + 192 / (m_vlxe - m_vlxs + 1);
            _p.setBrush(QColor(255, 255, 255, alpha));
            _p.setPen(QColor(0, 0, 0, qMin(255, 2 * alpha)));
            //_p.drawLine(m_xVerticalLine, 0, m_xVerticalLine, height() - 1);
            _p.drawRect(m_vlxs, 0, m_vlxe - m_vlxs, height());
        }
        else
        {
            _p.setBrush(Qt::NoBrush);
            _p.setPen(QColor(255, 255, 255, 192));
            _p.drawLine(m_vlxs, 0, m_vlxe, height() - 1);
        }
    }
}

void EditorOverlay::drawHorizontalLine(QPainter& _p)
{
}

void EditorOverlay::drawWidget(QPainter& _p)
{
    // QPainter p(this);
    drawModeCursor(_p, m_editor->cursorMode());
    drawHorizontalLine(_p);
    drawVerticalLine(_p);

    /*
    _p.setBrush(Qt::NoBrush);
    _p.setPen(Qt::red);
    _p.drawText(10, 20, "Overlay");
    _p.drawRect(0, 0, width() - 1, height() - 1);
    _p.drawLine(width() - 1, 0, 0, height() - 1);
    */
}
