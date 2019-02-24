
#include "CableView.h"

#include "GuiApplication.h"
#include "MainWindow.h"
#include "ModelView.h"

#include <QList>
#include <QMdiArea>
#include <QPainter>
#include <QWidget>

CableView::CableView(ModelView* _fromView, ModelView* _toView) :
      m_fromView(_fromView), m_toView(_toView),
      m_fromLine(_fromView->cableFrom()), m_toLine(_toView->cableTo()),
      m_color(_fromView->cableColor())
{
}

void CableView::drawCableView(QWidget* _overlay, QPainter* _p)
{
    QWidget* w1 = dynamic_cast<QWidget*>(m_fromView->widget());
    if(w1 == nullptr)
        return;
    QWidget* w2 = dynamic_cast<QWidget*>(m_toView->widget());
    if(w2 == nullptr)
        return;

    /*
    QRegion region1=w1->visibleRegion();
    QRegion region2=w2->visibleRegion();
    QPoint p1=_overlay->mapFromGlobal(w1->mapToGlobal(QPoint(0,0)));
    QPoint p2=_overlay->mapFromGlobal(w2->mapToGlobal(QPoint(0,0)));
    region1=region1.translated(p1);
    region2=region1.translated(p2);
    _p->setClipRegion(region1.united(region2));
    */
    QMdiArea*             ws = gui->mainWindow()->workspace();
    QRegion               region;
    QList<QMdiSubWindow*> windows
            = ws->subWindowList(QMdiArea::StackingOrder);
    int count = 0;
    for(QMdiSubWindow* sw: windows)
    {
        bool b1 = sw->isAncestorOf(w1);
        bool b2 = sw->isAncestorOf(w2);

        if(count >= 2)
        {
            QRegion swr = sw->rect();  // visibleRegion();
            QPoint  swp = ws->mapFromGlobal(sw->mapToGlobal(QPoint(0, 0)));
            region      = region.united(swr.translated(swp));
        }

        if(b1)
            count++;
        if(b2)
            count++;
    }

    region     = QRegion(ws->rect()).subtracted(region);  // visibleRegion();
    QPoint pws = _overlay->mapFromGlobal(ws->mapToGlobal(QPoint(0, 0)));
    region     = region.translated(pws);
    _p->setClipRegion(region);

    const int nbp = 8;
    QPointF   points[nbp];

    points[0] = _overlay->mapFromGlobal(w1->mapToGlobal(m_fromLine.p1()));
    points[7] = _overlay->mapFromGlobal(w2->mapToGlobal(m_toLine.p1()));

    QPoint fromDir = m_fromLine.p2() - m_fromLine.p1();
    QPoint toDir   = m_toLine.p2() - m_toLine.p1();

    QPointF q((points[7].x() - points[0].x()) / 35.f,
              (points[7].y() - points[0].y()) / 35.f);

    points[1] = points[0] + fromDir;
    points[2] = points[1] + fromDir + fromDir + q;
    points[3] = points[2] + fromDir + fromDir + fromDir + q + q + q + q;

    points[6] = points[7] + toDir;
    points[5] = points[6] + toDir + toDir - q;
    points[4] = points[5] + toDir + toDir + toDir - q - q - q - q;

    QPainterPath path;
    path.moveTo(points[0]);
    // path.lineTo(points[1]);
    for(int i = 1; i < nbp; i++)
    {
        QPointF& a = (i >= 2 ? points[i - 2] : points[0]);
        QPointF& b = points[i - 1];
        QPointF& c = points[i];
        QPointF& d = (i < nbp - 1 ? points[i + 1] : points[nbp - 1]);
        QPointF  c1(b.x() + (c.x() - a.x()) / 4.f,
                   b.y() + (c.y() - a.y()) / 4.f);
        QPointF  c2(c.x() + (b.x() - d.x()) / 4.f,
                   c.y() + (b.y() - d.y()) / 4.f);
        path.cubicTo(c1, c2, c);
    }

    _p->setPen(QPen(m_color, 3.f));
    _p->drawPath(path);

    QPointF xy(
            (points[7].x() + 2 * toDir.x() + points[0].x() + 2 * fromDir.x())
                    / 2.f,
            (points[7].y() + 2 * toDir.y() + points[0].y() + 2 * fromDir.y())
                    / 2.f);
    QString s1 = m_fromView->model()->displayName();
    QString s2 = m_toView->model()->displayName();
    if(s1 == "")
        s1 = m_fromView->model()->fullDisplayName();
    if(s2 == "")
        s2 = m_toView->model()->fullDisplayName();
    if(s1 == "")
        s1 = "???";
    if(s2 == "")
        s2 = "???";
    _p->drawText(xy, s1 + " --> " + s2);
}
