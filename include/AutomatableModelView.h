/*
 * AutomatableModelView.h - provides AutomatableModelView base class and
 * provides BoolModelView, FloatModelView, IntModelView subclasses.
 *
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

#ifndef AUTOMATABLE_MODEL_VIEW_H
#define AUTOMATABLE_MODEL_VIEW_H

#include "AutomatableModel.h"
#include "ModelView.h"

class QMenu;
class QMouseEvent;

class EXPORT AutomatableModelView : public ModelView
{
  protected:
    AutomatableModelView(AutomatableModel* _model, QWidget* _this);
    virtual ~AutomatableModelView();

  public:
    /*
    // some basic functions for convenience
    AutomatableModel* modelUntyped()
    {
        return castModel<AutomatableModel>();
    }

    const AutomatableModel* modelUntyped() const
    {
        return castModel<AutomatableModel>();
    }

    // virtual void setModel( Model* model, bool isOldModelValid = true );

    template <typename X>
    INLINE X value() const
    {
        return modelUntyped() ? modelUntyped()->value<X>() : 0;
    }
    */

    AutomatableModel* model()  // non virtual
    {
        return castModel<AutomatableModel>();
    }

    const AutomatableModel* model() const  // non virtual
    {
        return castModel<AutomatableModel>();
    }

    template <typename X>
    INLINE X value() const
    {
        return model()->AutomatableModel::value<X>();
    }

    QColor cableColor() const;

    virtual bool isInteractive() const final;
    virtual void setInteractive(bool _b) final;

    virtual void setDescription(const QString& desc) final;
    virtual void setUnit(const QString& unit) final;

    virtual void addDefaultActions(QMenu*     menu,
                                   const bool _interactive = true);
    virtual void enterValue();
    virtual void editRandomization();

  protected:
    virtual void mousePressEvent(QMouseEvent* event);

    bool    m_interactive;
    QString m_description;
    QString m_unit;
    bool    m_spaceBeforeUnit;
};

class AutomatableModelViewSlots : public QObject
{
    Q_OBJECT

  public:
    AutomatableModelViewSlots(AutomatableModelView* amv, QObject* parent);

  public slots:
    void enterValue();
    void editRandomization();
    void execConnectionDialog();
    void removeConnection();
    void editSongGlobalAutomation();
    void unlinkAllModels();
    void removeSongGlobalAutomation();

  private slots:
    /// Copy the model's value to the clipboard.
    void copyToClipboard();
    /// Paste the model's value from the clipboard.
    void pasteFromClipboard();

  protected:
    AutomatableModelView* m_amv;
};

template <class T>
class EXPORT TypedAutomatableModelView : public AutomatableModelView
{
  public:
    TypedAutomatableModelView(T* _model, QWidget* _this) :
          AutomatableModelView(_model, _this)
    {
    }

    TypedAutomatableModelView(QWidget* _this) :
          TypedAutomatableModelView(T::createDefaultConstructed(), _this)
    {
    }

    TypedAutomatableModelView(QWidget* _this, const QString& _displayName) :
          TypedAutomatableModelView(T::createDefaultConstructed(_displayName),
                                    _this)
    {
    }

    INLINE T* model()
    {
        return castModel<T>();
    }

    INLINE const T* model() const
    {
        return castModel<T>();
    }
};

using RealModelView = TypedAutomatableModelView<RealModel>;
#define FloatModelView RealModelView
using IntModelView  = TypedAutomatableModelView<IntModel>;
using BoolModelView = TypedAutomatableModelView<BoolModel>;

#endif
