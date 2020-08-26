/*
 * CableView.h -
 *
 */

#ifndef CABLE_VIEW_H
#define CABLE_VIEW_H

//#include "ModelView.h"

#include <QColor>
#include <QLine>
//#include <QPoint>

class ModelView;
class QPainter;
class QWidget;

class CableView
{
  public:
    CableView(ModelView* _fromView, ModelView* _toView);

    /*
    void setFrom(const QLine& _fromLine);
    void setTo(const QLine _toLine);
    void setColor(const QColor& _color);
    */

    void drawCableView(QWidget* _overlay, QPainter* _p);

  private:
    ModelView* m_fromView;
    ModelView* m_toView;
    QLine      m_fromLine, m_toLine;
    QColor     m_color;
};

#endif
