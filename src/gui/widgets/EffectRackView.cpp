/*
 * EffectRackView.cpp - view for effectChain model
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

#include "EffectRackView.h"

#include "Clipboard.h"
#include "EffectSelectDialog.h"
#include "EffectView.h"
#include "GroupBox.h"

#include <QApplication>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

EffectRackView::EffectRackView(EffectChain* model,
                               QWidget*     parent,
                               QString      _title) :
      GroupBox(_title == "" ? tr("AUDIO EFFECTS CHAIN") : _title,
               parent,
               true,
               false),
      ModelView(NULL, this)
{
    qRegisterMetaType<EffectView*>("EffectView*");

    QWidget* effectsView = new QWidget();

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

    setModel(model);
    setFixedWidth(250);
}

EffectRackView::~EffectRackView()
{
    clearViews();
}

void EffectRackView::clearViews()
{
    while(m_effectViews.size())
    {
        EffectView* e = m_effectViews[m_effectViews.size() - 1];
        m_effectViews.pop_back();
        delete e;
    }
}

void EffectRackView::moveUp(EffectView* _view)
{
    fxChain()->moveUp(_view->effect());
    if(_view != m_effectViews.first())
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
        QVector<EffectView*>::Iterator it
                = qFind(m_effectViews.begin(), m_effectViews.end(), _view);
        it = m_effectViews.erase(it);
        m_effectViews.insert(it - 1, _view);
        update();
    }
}

void EffectRackView::moveDown(EffectView* _view)
{
    fxChain()->moveDown(_view->effect());
    if(_view != m_effectViews.last())
    {
        // moving next effect up is the same
        // moveUp( *( qFind( m_effectViews.begin(), m_effectViews.end(), view
        // ) + 1 ) );

        QVector<EffectView*>::Iterator it
                = qFind(m_effectViews.begin(), m_effectViews.end(), _view);
        it = m_effectViews.erase(it);
        m_effectViews.insert(it + 1, _view);
        update();
    }
}

void EffectRackView::moveTop(EffectView* view)
{
    fxChain()->moveTop(view->effect());
    if(view != m_effectViews.first())
    {
        m_effectViews.removeOne(view);
        m_effectViews.prepend(view);
        update();
    }
}

void EffectRackView::moveBottom(EffectView* view)
{
    fxChain()->moveBottom(view->effect());
    if(view != m_effectViews.last())
    {
        m_effectViews.removeOne(view);
        m_effectViews.append(view);
        update();
    }
}

void EffectRackView::deletePlugin(EffectView* view)
{
    Effect* e = view->effect();
    m_effectViews.erase(
            qFind(m_effectViews.begin(), m_effectViews.end(), view));
    delete view;
    fxChain()->removeEffect(e);
    e->deleteLater();
    update();
}

void EffectRackView::update()
{
    QWidget*      w = m_scrollArea->widget();
    QVector<bool> view_map(
            qMax<int>(fxChain()->m_effects.size(), m_effectViews.size()),
            false);

    for(QVector<Effect*>::Iterator it = fxChain()->m_effects.begin();
        it != fxChain()->m_effects.end(); ++it)
    {
        int i = 0;
        for(QVector<EffectView*>::Iterator vit = m_effectViews.begin();
            vit != m_effectViews.end(); ++vit, ++i)
        {
            if((*vit)->model() == *it)
            {
                view_map[i] = true;
                break;
            }
        }
        if(i >= m_effectViews.size())
        {
            EffectView* view = new EffectView(*it, w);
            connect(view, SIGNAL(moveUp(EffectView*)), this,
                    SLOT(moveUp(EffectView*)));
            connect(view, SIGNAL(moveDown(EffectView*)), this,
                    SLOT(moveDown(EffectView*)));
            connect(view, SIGNAL(moveTop(EffectView*)), this,
                    SLOT(moveTop(EffectView*)));
            connect(view, SIGNAL(moveBottom(EffectView*)), this,
                    SLOT(moveBottom(EffectView*)));
            connect(view, SIGNAL(deletePlugin(EffectView*)), this,
                    SLOT(deletePlugin(EffectView*)), Qt::QueuedConnection);
            view->show();
            m_effectViews.append(view);
            if(i < view_map.size())
            {
                view_map[i] = true;
            }
            else
            {
                view_map.append(true);
            }
        }
    }

    int i = 0, nView = 0;

    const int EffectViewMargin = 3;
    m_lastY                    = EffectViewMargin;

    for(QVector<EffectView*>::Iterator it = m_effectViews.begin();
        it != m_effectViews.end(); i++)
    {
        if(i < view_map.size() && view_map[i] == false)
        {
            delete m_effectViews[nView];
            it = m_effectViews.erase(it);
        }
        else
        {
            (*it)->move(EffectViewMargin, m_lastY);
            m_lastY += (*it)->height();
            m_lastY += EffectViewMargin;
            ++nView;
            ++it;
        }
    }

    // EFFECT_WIDTH
    w->setFixedSize(229 + 2 * EffectViewMargin, m_lastY);

    QWidget::update();
}

void EffectRackView::addEffect()
{
    EffectSelectDialog esd(this);
    esd.exec();

    if(esd.result() == QDialog::Rejected)
    {
        return;
    }

    Effect* fx = esd.instantiateSelectedPlugin(fxChain());
    if(!fx->isOkay())
    {
        qWarning("EffectRackView::addEffect effect is not okay");
        delete fx;
        return;
    }

    addEffect(fx);
}

void EffectRackView::addEffect(Effect* fx)
{
    qInfo("EffectRackView::addEffect() 0");
    fxChain()->m_enabledModel.setValue(true);
    qInfo("EffectRackView::addEffect() 1");
    fxChain()->appendEffect(fx);
    qInfo("EffectRackView::addEffect() 2");
    update();
    qInfo("EffectRackView::addEffect() 3 %p", fx->controls());

    // Find the effectView, and show the controls
    /*
    for(QVector<EffectView*>::Iterator vit = m_effectViews.begin();
        vit != m_effectViews.end(); ++vit)
    {
        if((*vit)->effect() == fx)
        {
            qInfo("EffectRackView::addEffect() 4");
            (*vit)->editControls();
            qInfo("EffectRackView::addEffect() 5");
            break;
        }
    }
    */
}

void EffectRackView::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::MiddleButton)
    {
        qInfo("MIDDLE PRESS");
        _me->accept();
    }
}

void EffectRackView::mouseReleaseEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::MiddleButton)
    {
        _me->accept();
        qInfo("MIDDLE RELEASE");
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
                    QString name(effectData.attribute("name"));

                    Effect* e
                            = Effect::instantiate(name, fxChain(), &key);

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

void EffectRackView::modelChanged()
{
    // clearViews();
    ledButton()->setModel(&fxChain()->m_enabledModel);
    connect(fxChain(), SIGNAL(aboutToClear()), this, SLOT(clearViews()));
    update();
}
