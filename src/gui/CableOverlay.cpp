
#include "CableOverlay.h"

#include "CableView.h"
#include "GuiApplication.h"
#include "MainWindow.h"

#include <QList>
#include <QMdiArea>
#include <QPaintEvent>
#include <QPainter>

CableOverlay::CableOverlay(QWidget* _parent) : QWidget(_parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void CableOverlay::collect(QWidget* _w, QHash<ModelView*, Model*>& _table)
{
    if(_w == nullptr || _w == this)
        return;

    // if(!_w->isVisible()) return;
    int oldsize = _table.size();

    ModelView* mv = dynamic_cast<ModelView*>(_w);
    if(mv != nullptr)
    {
        Model* m = dynamic_cast<Model*>(mv->model());
        if(m != nullptr)
            _table.insert(mv, m);

        for(ModelView* cmv: mv->childModelViews())
        {
            Model* cm = dynamic_cast<Model*>(cmv->model());
            if(cm != nullptr)
                _table.insert(cmv, cm);
        }
    }

    if(_w->isVisible())
        for(QObject* o: _w->children())
        {
            QWidget* w = dynamic_cast<QWidget*>(o);
            if(w == nullptr)
                continue;
            collect(w, _table);
        }

    int newsize = _table.size();
    if(oldsize != newsize)
        _w->installEventFilter(this);
}

bool CableOverlay::eventFilter(QObject* obj, QEvent* event)
{
    if(event->type() == QEvent::Move || event->type() == QEvent::Resize
       || event->type() == QEvent::Show || event->type() == QEvent::Hide
       || event->type() == QEvent::ChildAdded
       || event->type() == QEvent::ChildRemoved
       || event->type() == QEvent::Close
       || event->type() == QEvent::ZOrderChange)
    {
        // qInfo("CableOverlay::eventFilter");
        update();
    }
    return false;
}

void CableOverlay::paintEvent(QPaintEvent* _pe)
{
    /*
const QRect& clip = _pe->rect();
if(clip != rect())
{
    update();
    return;
}
    */

    QHash<ModelView*, Model*> table;
    collect(gui->mainWindow(), table);
    // qInfo("CableOverlay::paintEvent %d widgets", table.size());
    gui->mainWindow()->workspace()->installEventFilter(this);

    QPainter p(this);

    p.setRenderHints(QPainter::Antialiasing, true);
    for(ModelView* mv1: table.keys())
    {
        for(ModelView* mv2: table.keys())
        {
            if(mv1 == mv2)
                continue;

            Model* m1 = table.value(mv1);
            Model* m2 = table.value(mv2);

            if(m2->hasCableFrom(m1))
            {
                CableView c(mv1, mv2);
                c.drawCableView(this, &p);
            }
        }
    }
    p.setRenderHints(QPainter::Antialiasing, false);
}
