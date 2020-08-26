/*
 * EffectChainView.cpp - view for effectChain model
 *
 * Copyright (c) 2006-2007 Danny McRae <khjklujn@netscape.net>
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "EffectChainView.h"

#include "Clipboard.h"
#include "EffectSelectDialog.h"
#include "EffectView.h"
#include "GroupBox.h"

#include <QApplication>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

EffectChainView::EffectChainView(EffectChain* _model,
                                 QWidget*     _parent,
                                 QString      _title) :
      GroupBox(_title == "" ? tr("AUDIO EFFECTS CHAIN") : _title,
               _parent,
               true,
               false),
      ModelView(_model, this)
{
    qRegisterMetaType<EffectView*>("EffectView*");
    allowModelChange(true);

    QWidget* effectsView = new QWidget();
    effectsView->setContentsMargins(0, 0, 0, 12);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setPalette(QApplication::palette(m_scrollArea));
    // m_scrollArea->setFrameStyle( QFrame::Box );
    // m_scrollArea->setLineWidth( 1 );
    m_scrollArea->setFrameStyle(QFrame::NoFrame);
    m_scrollArea->setLineWidth(1);
    m_scrollArea->setWidget(effectsView);
    setContentWidget(m_scrollArea);

    QPushButton* addButton = new QPushButton(tr("Add audio effect"), this);
    setBottomWidget(addButton);
    connect(addButton, SIGNAL(clicked()), this, SLOT(addEffect()));

    m_lastY = 0;

    // setModel(model);
    setFixedWidth(250);
    modelChanged();
}

EffectChainView::~EffectChainView()
{
    qWarning("********** delete chain view *************");
    clearViews();
}

void EffectChainView::clearViews()
{
    while(!m_effectViews.isEmpty()) //size())
    {
        // EffectView* e = m_effectViews[m_effectViews.size() - 1];
        // m_effectViews.pop_back();
        EffectView* e = m_effectViews.takeLast();
        // delete e;
        e->deleteLater();
    }
}

void EffectChainView::moveUp(EffectView* _view)
{
    model()->moveUp(_view->model());
    // if(_view != m_effectViews.first())
    {
        /*
        int i = 0;
        for( QVector<EffectView *>::Iterator it = m_effectViews.begin();
                                it != m_effectViews.end(); it++, i++ )
        {
                if( *it == view )
                {
                        break;
                }
        }

        EffectView * temp = m_effectViews[ i - 1 ];

        m_effectViews[i - 1] = view;
        m_effectViews[i] = temp;
        */
        /*
        QVector<EffectView*>::Iterator it
                = qFind(m_effectViews.begin(), m_effectViews.end(), _view);
        it = m_effectViews.erase(it);
        m_effectViews.insert(it - 1, _view);
        */
        int index = m_effectViews.indexOf(_view);
        if(index > 0)
        {
            m_effectViews.remove(index);
            m_effectViews.insert(index - 1, _view);
            update();
        }
        else if(index < 0)
        {
            qWarning("EffectChainView::moveUp effectView not found");
        }
    }
}

void EffectChainView::moveDown(EffectView* _view)
{
    model()->moveDown(_view->model());
    // if(_view != m_effectViews.last())
    {
        /*
        QVector<EffectView*>::Iterator it
                = qFind(m_effectViews.begin(), m_effectViews.end(), _view);
        it = m_effectViews.erase(it);
        m_effectViews.insert(it + 1, _view);
        update();
        */
        int index = m_effectViews.indexOf(_view);
        if(index < m_effectViews.size() - 1)
        {
            m_effectViews.remove(index);
            m_effectViews.insert(index + 1, _view);
            update();
        }
        else if(index < 0)
        {
            qWarning("EffectChainView::moveDown effectView not found");
        }
    }
}

void EffectChainView::moveTop(EffectView* view)
{
    model()->moveTop(view->model());
    if(view != m_effectViews.first())
    {
        m_effectViews.removeOne(view);
        m_effectViews.prepend(view);
        update();
    }
}

void EffectChainView::moveBottom(EffectView* view)
{
    model()->moveBottom(view->model());
    if(view != m_effectViews.last())
    {
        m_effectViews.removeOne(view);
        m_effectViews.append(view);
        update();
    }
}

void EffectChainView::removeEffect(EffectView* _view)
{
    Effect* e = _view->model();
    // m_effectViews.erase(
    //  qFind(m_effectViews.begin(), m_effectViews.end(), view));
    m_effectViews.removeOne(_view);
    delete _view;
    model()->removeEffect(e);
    e->deleteLater();
    update();
}

void EffectChainView::update()
{
    QWidget*      w = m_scrollArea->widget();
    QVector<bool> view_map(
            qMax<int>(model()->m_effects.size(), m_effectViews.size()),
            false);

    // for(QVector<Effect*>::Iterator it = model()->m_effects.begin();
    //    it != model()->m_effects.end(); ++it)

    model()->m_effects.map([this, &view_map, w](Effect* effect) {
        int i = 0;
        for(EffectView* ev: m_effectViews)
        {
            if(ev->model() == effect)  //*it)
            {
                view_map[i] = true;
                break;
            }
            i++;
        }
        if(i >= m_effectViews.size())
        {
            EffectView* view = new EffectView(effect, w);
            connect(view, SIGNAL(moveUp(EffectView*)), this,
                    SLOT(moveUp(EffectView*)));
            connect(view, SIGNAL(moveDown(EffectView*)), this,
                    SLOT(moveDown(EffectView*)));
            connect(view, SIGNAL(moveTop(EffectView*)), this,
                    SLOT(moveTop(EffectView*)));
            connect(view, SIGNAL(moveBottom(EffectView*)), this,
                    SLOT(moveBottom(EffectView*)));
            connect(view, SIGNAL(removeEffect(EffectView*)), this,
                    SLOT(removeEffect(EffectView*)), Qt::QueuedConnection);
            view->show();
            m_effectViews.append(view);
            if(i < view_map.size())
                view_map[i] = true;
            else
                view_map.append(true);
        }
    });

    int i = 0, nView = 0;

    const int MARGIN = 3;
    m_lastY          = MARGIN;

    for(EffectView* ev: m_effectViews)
    {
        if(i < view_map.size() && view_map[i] == false)
        {
            delete m_effectViews[nView];
            // it = m_effectViews.erase(it);
            m_effectViews.remove(i);
        }
        else
        {
            ev->move(MARGIN, m_lastY);
            m_lastY += ev->height();
            m_lastY += MARGIN;
            ++nView;
        }
        i++;
    }

    // EFFECT_WIDTH
    m_lastY += 18;  // small bottom empty space
    w->setFixedSize(229 + 2 * MARGIN, m_lastY);

    adjustSize();
    QWidget::update();
}

void EffectChainView::addEffect()
{
    EffectSelectDialog esd(this);
    esd.exec();

    if(esd.result() == QDialog::Rejected)
        return;

    Effect* fx = esd.instantiateSelectedPlugin(model());
    if(!fx->isOkay())
    {
        qWarning("EffectChainView::addEffect effect is not okay");
        delete fx;
        return;
    }

    addEffect(fx);
}

void EffectChainView::addEffect(Effect* fx)
{
    qInfo("EffectChainView::addEffect() 0");
    model()->m_enabledModel.setValue(true);
    qInfo("EffectChainView::addEffect() 1");
    model()->appendEffect(fx);
    qInfo("EffectChainView::addEffect() 2");
    update();
    qInfo("EffectChainView::addEffect() 3 %p", fx->controls());

    // Find the effectView, and show the controls
    /*
    for(QVector<EffectView*>::Iterator vit = m_effectViews.begin();
        vit != m_effectViews.end(); ++vit)
    {
        if((*vit)->model() == fx)
        {
            qInfo("EffectChainView::addEffect() 4");
            (*vit)->openControls();
            qInfo("EffectChainView::addEffect() 5");
            break;
        }
    }
    */
}

void EffectChainView::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::MiddleButton)
    {
        // qInfo("MIDDLE PRESS");
        _me->accept();
    }
}

void EffectChainView::mouseReleaseEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::MiddleButton)
    {
        _me->accept();
        // qInfo("MIDDLE RELEASE");
        if(rect().contains(_me->pos()))
        {
            bool ok = Selection::has(Effect::classNodeName());
            if(ok)
            {
                QDomElement effectData
                        = Selection::dom(Effect::classNodeName());
                if(!effectData.isNull())
                {
                    EffectKey key(effectData.elementsByTagName("key")
                                          .item(0)
                                          .toElement());
                    QString   name(effectData.attribute("name"));

                    Effect* e = Effect::instantiate(name, model(), &key);

                    if(e != NULL && e->isOkay()
                       && e->nodeName() == Effect::classNodeName())
                    {
                        e->restoreState(effectData);
                        addEffect(e);
                    }
                    else
                    {
                        ok = false;
                        delete e;
                        // e = new DummyEffect( parentModel(), effectData );
                    }
                }
                else
                    ok = false;
            }
            if(!ok)
            {
                qWarning(
                        "Warning: invalid effect data in the system "
                        "selection");
            }
        }
    }
}

void EffectChainView::modelChanged()
{
    // clearViews();
    ledButton()->setModel(&model()->m_enabledModel);
    connect(model(), SIGNAL(aboutToClear()), this, SLOT(clearViews()));
    connect(model(), SIGNAL(dataChanged()), this, SLOT(update()));
    update();
}
