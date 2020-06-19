/*
 * Model.h - declaration of Model base class
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

#ifndef MODEL_H
#define MODEL_H

#include "export.h"

#include <QHash>
#include <QObject>
#include <QString>

class EXPORT Model : public QObject
{
    Q_OBJECT

  public:
    Model(Model*         _parent,
          const QString& _displayName,
          bool           _defaultConstructed = false);
    virtual ~Model();

    bool isDefaultConstructed()
    {
        return m_defaultConstructed;
    }

    Model* parentModel() const
    {
        return dynamic_cast<Model*>(parent());
    }

    virtual const QString uuid() final;
    virtual bool          hasUuid() const final;
    virtual void          setUuid(const QString& _uuid) final;
    virtual void          resetUuid() final;

    virtual QString objectName() const;
    virtual QString fullObjectName() const;

    virtual QString displayName() const
    {
        return m_displayName;
    }

    virtual void setDisplayName(const QString& displayName);

    virtual QString fullDisplayName() const;

    virtual bool frequentlyUpdated() const;
    virtual void setFrequentlyUpdated(const bool _b);

    virtual bool hasCableFrom(Model* _m) const;

    static Model* find(const QString& _uuid);

  signals:
    // emitted if actual data of the model (e.g. values) have changed
    void dataChanged();
    // emitted in case new data was not set as it's been equal to old data
    void dataUnchanged();
    // emitted if properties of the model (e.g. ranges) have changed
    void propertiesChanged();

  private:
    QString m_displayName;
    bool    m_defaultConstructed;
    bool    m_frequentlyUpdated;
    QString m_uuid;
    QString m_debug_uuid;

    static QHash<QString, Model*> s_models;
};

#endif
