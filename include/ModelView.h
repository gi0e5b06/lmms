/*
 * ModelView.h - declaration of ModelView base class
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

#ifndef MODEL_VIEW_H
#define MODEL_VIEW_H

#include "Backtrace.h"
#include "Model.h"
#include "lmms_basics.h"

#include <QColor>
#include <QLine>
//#include <QPoint>
#include <QPointer>
#include <QWidget>
class Delegate;

// template <class T : Model, class U : QWidget>
class EXPORT ModelView
{
  public:
    ModelView(Model* model, QWidget* widget);
    virtual ~ModelView();

    INLINE Model* model()  // non virtual
    {
        return m_model.data();
    }

    INLINE const Model* model() const  // non virtual
    {
        return m_model.data();
    }

    virtual void setModel(Model* model);  //, bool isOldModelValid = true);

    INLINE QWidget* widget()  // non virtual
    {
        return m_widget.data();
    }

    INLINE const QWidget* widget() const  // non virtual
    {
        return m_widget.data();
    }

    virtual void setWidget(QWidget* _w) final;

    virtual QList<ModelView*> childModelViews() const
    {
        return QList<ModelView*>();
    }

    virtual QLine  cableFrom() const;
    virtual QLine  cableTo() const;
    virtual QColor cableColor() const;

    virtual void onModelDestroyed() final;
    virtual void onWidgetDestroyed() final;

    virtual void allowModelChange(bool _allowed)
    {
        m_modelChangeAllowed = _allowed;
    }

    virtual void allowWidgetChange(bool _allowed)
    {
        m_widgetChangeAllowed = _allowed;
    }

    virtual void allowNullModel(bool _allowed)
    {
        m_nullModelAllowed = _allowed;
    }

    virtual void allowNullWidget(bool _allowed)
    {
        m_nullWidgetAllowed = _allowed;
    }

  protected:
    template <class X>
    X* castModel()
    {
        X* r = dynamic_cast<X*>(model());
        if(r == nullptr && !m_nullModelAllowed)
        {
            BACKTRACE
            qWarning("ModelView::castModel bad cast, null");
        }
        return r;
    }

    template <class X>
    const X* castModel() const
    {
        const X* r = dynamic_cast<const X*>(model());
        if(r == nullptr && !m_nullModelAllowed)
        {
            BACKTRACE
            qWarning("ModelView::castModel const bad cast, null");
        }
        return r;
    }

    template <class X>
    X* castWidget()
    {
        X* r = dynamic_cast<X*>(widget());
        if(r == nullptr && !m_nullWidgetAllowed)
        {
            BACKTRACE
            qWarning("ModelView::castWidget bad cast, null");
        }
        return r;
    }

    template <class X>
    const X* castWidget() const
    {
        const X* r = dynamic_cast<const X*>(widget());
        if(r == nullptr && !m_nullWidgetAllowed)
        {
            BACKTRACE
            qWarning("ModelView::castWidget const bad cast, null");
        }
        return r;
    }

    // secret constructor with null as model and this as widget
    ModelView(QWidget* _widget);

    virtual void doConnections();
    virtual void undoConnections();

    // sub-classes can re-implement this to track model-changes
    virtual void modelChanged();
    // sub-classes can re-implement this to track widget-changes
    virtual void widgetChanged();

  private:
    void trace(QString _s);

    QPointer<const Delegate> m_delegate;
    volatile bool            m_deleted;
    QPointer<QWidget>        m_widget;
    QPointer<Model>          m_model;
    bool                     m_modelChangeAllowed;
    bool                     m_widgetChangeAllowed;
    bool                     m_nullModelAllowed;
    bool                     m_nullWidgetAllowed;
    bool                     m_debugCnx;

    friend class Delegate;
};

class EXPORT DummyModelView : public QWidget, public ModelView
{
  public:
    DummyModelView(Model* _model, QWidget* _parent) :
          QWidget(_parent), ModelView(_model, this)
    {
        hide();
        setGeometry(0, 0, 0, 0);
    }
};

#endif
