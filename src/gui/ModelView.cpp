/*
 * ModelView.cpp - implementation of ModelView
 *
 * Copyright (c) 2007-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "ModelView.h"

#include "Backtrace.h"

#include <QWidget>

class Delegate : public QObject
{
    Q_OBJECT

  public:
    Delegate(ModelView* _mv) : m_mv(_mv)
    {
    }

    virtual ~Delegate()
    {
        // qInfo("Delegate::~Delegate");
    }

  public slots:
    virtual void onModelDestroyed() final
    {
        qInfo("Delegate::onModelDestroyed");
        if(m_mv != nullptr && !m_mv->m_deleted)
            m_mv->onModelDestroyed();
    }
    virtual void onWidgetDestroyed(QObject* _o) final
    {
        qInfo("Delegate::onWidgetDestroyed");
        if(m_mv != nullptr && !m_mv->m_deleted && m_mv->m_widget == nullptr)
            m_mv->onWidgetDestroyed();
    }

  private:
    ModelView* m_mv;

    friend class ModelView;
};

void ModelView::trace(QString _s)
{
    QString mn = m_model.isNull() ? "-" : m_model->displayName();
    QString wn = m_widget.isNull() ? "-" : m_widget->objectName();
    if(mn == "-" && !m_model.isNull()
       && m_model.data() == dynamic_cast<Model*>(this))
        mn = "this";
    if(wn == "-" && !m_widget.isNull()
       && m_widget.data() == dynamic_cast<QWidget*>(this))
        wn = "this";
    qInfo("%s", qPrintable(QString("%1 m=%2 w=%3").arg(_s).arg(mn).arg(wn)));
}

ModelView::ModelView(Model* _model, QWidget* _widget) :
      m_delegate(new Delegate(this)), m_deleted(false), m_widget(_widget),
      m_model(_model), m_modelChangeAllowed(false),
      m_widgetChangeAllowed(false), m_nullModelAllowed(false),
      m_nullWidgetAllowed(false), m_debugCnx(false)
{
    if(m_widget.isNull())
    {
        BACKTRACE
        qFatal("ModelView::ModelView null widget not allowed");
    }
    if(m_model.isNull())
    {
        BACKTRACE
        qWarning("ModelView::ModelView null model not recommended");
    }
    else
    {
        allowModelChange(_model->isDefaultConstructed());
    }
    doConnections();
    modelChanged();
}

ModelView::ModelView(QWidget* _widget) :
      m_delegate(new Delegate(this)), m_deleted(false), m_widget(_widget),
      m_model(nullptr), m_modelChangeAllowed(true),
      m_widgetChangeAllowed(false), m_nullModelAllowed(true),
      m_nullWidgetAllowed(false), m_debugCnx(false)
{
    if(m_widget.isNull())
    {
        BACKTRACE
        qFatal("ModelView::ModelView this is not a widget");
    }
}

ModelView::~ModelView()
{
    m_deleted = true;
    // trace("ModelView::~ModelView START");

    undoConnections();
    m_delegate = nullptr;

    if(!m_widget.isNull())
    {
        if(m_widget.data() != dynamic_cast<QWidget*>(this))
        {
            // m_widget->blockSignals(true);
            /*
            if(!m_widget->testAttribute(Qt::WA_DeleteOnClose))
            {
                BACKTRACE
                qWarning("WA_DeleteOnClose not set for widget %s %p %p",
                         qPrintable(m_widget->objectName()), m_widget, this);
            }
            */
            if(m_widget->testAttribute(Qt::WA_DeleteOnClose))
            {
                m_widget->blockSignals(true);
                // trace("ModelView::~ModelView close widget");
                m_widget->close();
            }
        }
        else
        {
            m_widget->blockSignals(true);
            m_widget->setParent(nullptr);
        }
    }
    m_widget = nullptr;

    if(!m_model.isNull())
    {
        if(m_model.data() != dynamic_cast<Model*>(this))
        {
            if(m_model->isDefaultConstructed())
            {
                // trace("ModelView::~ModelView delete model");
                m_model->blockSignals(true);
                delete m_model;  // m_model->deleteLater();
            }
        }
        else
        {
            // m_model->blockSignals(true);
        }
    }
    m_model = nullptr;

    // trace("ModelView::~ModelView END");
}

void ModelView::setModel(Model* _model)  //, bool _isOldModelValid)
{
    if(_model == nullptr && !m_nullModelAllowed)
    {
        BACKTRACE
        trace("ModelView::setModel null not allowed");
        qFatal("--- %s",
               qPrintable(_model == nullptr ? "null" : _model->objectName()));
    }

    if(m_model.data() == _model)
    {
        BACKTRACE
        trace("ModelView::setModel same model");
        return;
    }

    if(!m_modelChangeAllowed)
    {
        BACKTRACE
        trace("ModelView::setModel not allowed, disabled");
        qFatal("--- %s",
               qPrintable(_model == nullptr ? "null" : _model->objectName()));
    }

    undoConnections();

    if(/*_isOldModelValid &&*/ !m_model.isNull())
    {
        if(m_model->isDefaultConstructed())
        {
            m_model->blockSignals(true);
            m_model->deleteLater();
        }
        /*
        else
        {
            QWidget* w = widget();
            if(w != nullptr)
                m_model->disconnect(w);
        }
        */
    }

    m_model = _model;
    doConnections();
    modelChanged();
}

void ModelView::setWidget(QWidget* _w)
{
    if(_w == nullptr && !m_nullWidgetAllowed)
    {
        BACKTRACE
        qFatal("ModelView::setWidget null not allowed");
    }

    if(m_widget.data() == _w)
    {
        BACKTRACE
        qWarning("ModelView::setWidget same widget: %s",
                 qPrintable(_w != nullptr ? _w->objectName() : "null"));
        return;
    }

    if(!m_widgetChangeAllowed)
    {
        BACKTRACE
        qFatal("ModelView::setWidget not allowed, disabled: %s",
               qPrintable(_w != nullptr ? _w->objectName() : "null"));
    }

    undoConnections();
    m_widget = _w;
    doConnections();
    widgetChanged();
}

void ModelView::modelChanged()
{
    // qWarning("ModelView::setModel widget=%p ->update()",widget());
    /*
    QWidget* w = widget();
    if(w != nullptr)
        w->update();
    */
}

void ModelView::widgetChanged()
{
    // qWarning("ModelView::setModel widget=%p ->update()",widget());
    QWidget* w = widget();
    if(w != nullptr)
        w->update();
}

void ModelView::onModelDestroyed()
{
    // trace("ModelView::onModelDestroyed");
    m_model = nullptr;
    if(m_deleted)
        return;

    if(m_nullModelAllowed)
    {
        modelChanged();  // setModel(nullptr);
    }
    else
    {
        // trace("ModelView::onModelDestroyed delete mvc this");
        m_deleted = true;
        delete this;
    }
}

void ModelView::onWidgetDestroyed()
{
    // trace("ModelView::onWidgetDestroyed");
    m_widget = nullptr;
    if(m_deleted)
        return;

    if(m_nullWidgetAllowed)
    {
        // widgetChanged();  // setWidget(nullptr);
    }
    else
    {
        // trace("ModelView::onWidgetDestroyed delete mvc this");
        m_deleted = true;
        delete this;
    }
}

void ModelView::doConnections()
{
    // qInfo("ModelView::doConnections");
    const Model*   m = m_model.data();
    const QWidget* w = m_widget.data();
    if(m != nullptr && w != nullptr)
    {
        /*
        if(m_debugCnx)
        {
            BACKTRACE
            qWarning("ModelView::doConnections twice called");
        }
        */
        m_debugCnx = true;
        // qWarning("ModelView::doConnections m_model=%p",m_model);
        // m_model->disconnect(widget());
        QObject::connect(m, SIGNAL(dataChanged()), w, SLOT(update()),
                         Qt::UniqueConnection);
        QObject::connect(m, SIGNAL(propertiesChanged()), w, SLOT(update()),
                         Qt::UniqueConnection);
    }
    if(m_delegate != nullptr)
    {
        if(m != nullptr && m != dynamic_cast<Model*>(this))
        {
            const bool connected = QObject::connect(
                    m, SIGNAL(modelDestroyed()), m_delegate,
                    SLOT(onModelDestroyed()), Qt::DirectConnection);
            Q_ASSERT(connected);
            Q_UNUSED(connected);
        }
        if(w != nullptr && w != dynamic_cast<QWidget*>(this))
        {
            const bool connected = QObject::connect(
                    w, SIGNAL(destroyed(QObject*)), m_delegate,
                    SLOT(onWidgetDestroyed(QObject*)), Qt::DirectConnection);
            Q_ASSERT(connected);
            Q_UNUSED(connected);
        }
    }
}

void ModelView::undoConnections()
{
    const Model*   m = m_model.data();
    const QWidget* w = m_widget.data();
    if(m != nullptr && w != nullptr)
    {
        if(!m_debugCnx)
        {
            BACKTRACE
            qWarning("ModelView::undoConnections twice called");
        }
        m_debugCnx = false;
        // qWarning("ModelView::doConnections m_model=%p",m_model);
        // m_model->disconnect(widget());
        QObject::disconnect(m, SIGNAL(dataChanged()), w, SLOT(update()));
        QObject::disconnect(m, SIGNAL(propertiesChanged()), w,
                            SLOT(update()));
    }
    if(m_delegate != nullptr)
    {
        if(m != nullptr && m != dynamic_cast<Model*>(this))
            QObject::disconnect(m, SIGNAL(modelDestroyed()), m_delegate,
                                SLOT(onModelDestroyed()));
        if(w != nullptr && w != dynamic_cast<QWidget*>(this))
            QObject::disconnect(w, SIGNAL(destroyed(QObject*)), m_delegate,
                                SLOT(onWidgetDestroyed(QObject*)));
    }
}

QLine ModelView::cableFrom() const
{
    const QWidget* w = widget();
    if(w == nullptr)
        return QLine();

    QPoint p(w->width() - 1, w->height() / 2);
    return QLine(p, p + QPoint(50, 0));
}

QLine ModelView::cableTo() const
{
    const QWidget* w = widget();
    if(w == nullptr)
        return QLine();

    QPoint p(0, w->height() / 2);
    return QLine(p, p + QPoint(-50, 0));
}

QColor ModelView::cableColor() const
{
    return Qt::yellow;
}

#include "ModelView.moc"
