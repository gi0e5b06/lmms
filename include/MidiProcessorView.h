/*
 * MidiProcessorView.h - powerful knob-widget
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
 *
 */

#ifndef MIDI_PROCESSOR_VIEW_H
#define MIDI_PROCESSOR_VIEW_H

#include "Widget.h"

#include <QPainter>
//#include <QDropEvent>

class MidiEventProcessor;
class QPixmap;

class /*EXPORT*/ MidiProcessorView : public Widget
{
    Q_OBJECT

  public:
    MidiProcessorView(bool                _in,
                      MidiEventProcessor* _processor,
                      QWidget*            _parent = nullptr,
                      const QString&      _name   = "[midi connector]");
    virtual ~MidiProcessorView();

    MidiEventProcessor* processor()
    {
        return m_processor;
    }

    void setProcessor(MidiEventProcessor* _processor)
    {
        m_processor = _processor;
    }

    QString text() const;
    void    setText(const QString& _s);

    virtual QLine  cableFrom() const;
    virtual QLine  cableTo() const;
    virtual QColor cableColor() const;

  public slots:
    void displayHelp();

  protected:
    void dragEnterEvent(QDragEnterEvent* _dee);
    void dropEvent(QDropEvent* _de);
    void mousePressEvent(QMouseEvent* event);

  private:
    void drawWidget(QPainter& _p);
    void drawConnector(QPainter& _p);
    void drawText(QPainter& _p);

    bool                m_in;
    MidiEventProcessor* m_processor;
    QString             m_text;
    QPixmap             m_pixmap;
};

#endif
