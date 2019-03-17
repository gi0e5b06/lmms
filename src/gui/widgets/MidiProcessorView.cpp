/*
 * MidiProcessorView.cpp -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#include "MidiProcessorView.h"

#include "MidiEventProcessor.h"
#include "Model.h"
#include "StringPairDrag.h"
#include "embed.h"
#include "gui_templates.h"

#include <QWhatsThis>

MidiProcessorView::MidiProcessorView(bool                _in,
                                     MidiEventProcessor* _processor,
                                     QWidget*            _parent,
                                     const QString&      _name) :
      Widget(_parent),
      m_in(_in), m_processor(_processor)
{
    setAcceptDrops(!m_in);
    setCursor(Qt::PointingHandCursor);
    // setFocusPolicy(Qt::ClickFocus);

    m_pixmap = embed::getPixmap(m_in ? "midi_in" : "midi_out");
    setText(m_in ? "IN" : "OUT");
}

MidiProcessorView::~MidiProcessorView()
{
}

QString MidiProcessorView::text() const
{
    return m_text;
}

void MidiProcessorView::setText(const QString& txt)
{
    if(m_text == txt)
        return;

    int w = m_pixmap.width();
    int h = m_pixmap.height();
    if(!txt.isEmpty())
    {
        QFontMetrics mx(pointSizeF(font(), 7.f));  // 6.5));
        w = qMax<int>(w, qMin<int>(2 * w, mx.width(txt)));
        h += 7;  // 10;
        // h = qMin(h, 36);
    }
    // w = qMin(w, 26);
    setMinimumSize(w, h);
    resize(w, h);

    m_text = txt;
    update();
}

void MidiProcessorView::drawText(QPainter& _p)
{
    if(!m_text.isEmpty())
    {
        _p.setFont(pointSizeF(font(), 7.f));
        _p.setPen(Qt::white);  // textColor());
        QFontMetrics metrix = _p.fontMetrics();
        QString text = metrix.elidedText(m_text, Qt::ElideRight, width());
        int     x    = width() / 2 - metrix.width(text) / 2;
        int     y    = height() - 1;
        _p.drawText(x, y, text);
        _p.drawText(x, y, text);  // twice for aa
    }
}

void MidiProcessorView::drawWidget(QPainter& _p)
{
    _p.setRenderHints(QPainter::Antialiasing, true);
    drawConnector(_p);
    drawText(_p);
}

void MidiProcessorView::drawConnector(QPainter& _p)
{
    _p.drawPixmap(QPointF((width() - m_pixmap.width()) / 2.f, 0.f), m_pixmap);
}

void MidiProcessorView::dragEnterEvent(QDragEnterEvent* _dee)
{
    StringPairDrag::processDragEnterEvent(_dee, m_in ? "midi_out_processor"
                                                     : "midi_in_processor");
}

void MidiProcessorView::dropEvent(QDropEvent* _de)
{
    QString type = StringPairDrag::decodeKey(_de);
    QString val  = StringPairDrag::decodeValue(_de);

    qInfo("MidiProcessorView::dropEvent type=%s val=%s", qPrintable(type),
          qPrintable(val));

    if(m_in && type == "midi_out_processor")
    {
        MidiEventProcessor* proc
                = dynamic_cast<MidiEventProcessor*>(Model::find(val));
        if(proc != nullptr)
        {
            Model* m = dynamic_cast<Model*>(m_processor);
            qInfo("MidiProcessorView::dropEvent proc=%s",
                  qPrintable(m->uuid()));
            proc->linkMidiOutProcessor(m->uuid());
            _de->accept();
            // mod->emit propertiesChanged();
            update();
        }
    }
    else if(!m_in && type == "midi_in_processor")
    {
        MidiEventProcessor* proc
                = dynamic_cast<MidiEventProcessor*>(Model::find(val));
        if(proc != nullptr)
        {
            Model* m = dynamic_cast<Model*>(proc);
            qInfo("MidiProcessorView::dropEvent proc=%s",
                  qPrintable(m->uuid()));
            m_processor->linkMidiOutProcessor(m->uuid());
            _de->accept();
            // mod->emit propertiesChanged();
            update();
        }
    }
}

void MidiProcessorView::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::LeftButton
       && _me->modifiers() & Qt::ControlModifier)
    {

        Model* m = dynamic_cast<Model*>(m_processor);
        if(m != nullptr)
        {
            new StringPairDrag(m_in ? "midi_in_processor"
                                    : "midi_out_processor",
                               m->uuid(), m_pixmap, this);
            _me->accept();
        }
    }
}

void MidiProcessorView::displayHelp()
{
    QWhatsThis::showText(mapToGlobal(rect().bottomRight()), whatsThis());
}

QLine MidiProcessorView::cableFrom() const
{
    int    w = m_pixmap.width();
    int    h = m_pixmap.height();
    QPoint p(w / 2, h / 2);
    return QLine(p, p + QPoint(0, 50));
}

QLine MidiProcessorView::cableTo() const
{
    int    w = m_pixmap.width() + 1;
    int    h = m_pixmap.height() + 1;
    QPoint p(w / 2, h / 2);
    return QLine(p, p + QPoint(0, 60));
}

QColor MidiProcessorView::cableColor() const
{
    return Qt::black;
}
