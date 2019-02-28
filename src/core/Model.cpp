/*
 * Model.cpp - implementation of Model base class
 *
 * Copyright (c) 2007-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "Model.h"

#include "Backtrace.h"

#include <QUuid>
#include <QHash>

QHash<QString, Model*> Model::s_models;

Model::Model(Model*         _parent,
             const QString& _displayName,
             bool           _defaultConstructed) :
      QObject(_parent),
      m_displayName(_displayName), m_defaultConstructed(_defaultConstructed),
      m_uuid("")
{
    if(_displayName.isEmpty())
    {
        BACKTRACE
        qInfo("Model::Model empty display name");
    }
    m_debug_uuid = QUuid::createUuid().toString();
}

static QHash<QString, bool> s_destructorTracker;

Model::~Model()
{
    if(hasUuid())
        s_models.remove(m_uuid);

    if(s_destructorTracker.contains(m_debug_uuid))
    {
        BACKTRACE
        qCritical("Model::~Model %s destroyed twice",
                  qPrintable(fullDisplayName()));
        return;
    }
    s_destructorTracker.insert(m_debug_uuid, true);
}

const QString Model::uuid()
{
    if(!hasUuid())
    {
        m_uuid = QUuid::createUuid().toString().replace("{", "").replace("}",
                                                                         "");
        s_models.insert(m_uuid, this);
        qInfo("Model::uuid create uuid for '%s'",
              qPrintable(fullDisplayName()));
    }
    return m_uuid;
}

bool Model::hasUuid() const
{
    return !m_uuid.isEmpty();
}

void Model::setUuid(const QString& _uuid)
{
    if(hasUuid())
    {
        qWarning("Model::setUuid '%s' already has an uuid",
                 qPrintable(fullDisplayName()));
        return;
    }

    m_uuid = _uuid;
    s_models.insert(m_uuid, this);
}

void Model::resetUuid()
{
    if(hasUuid())
    {
        s_models.remove(uuid());
        m_uuid = "";
    }
}

Model* Model::find(const QString& _uuid)
{
    return s_models.value(_uuid, nullptr);
}

void Model::setDisplayName(const QString& _displayName)
{
    if(m_displayName != _displayName)
    {
        m_displayName = _displayName;
        emit propertiesChanged();
    }
}

QString Model::fullDisplayName() const
{
    const QString& n = displayName();
    if(parentModel())
    {
        const QString p = parentModel()->fullDisplayName();
        if(n.isEmpty() && p.isEmpty())
        {
            return QString::null;
        }
        else if(p.isEmpty())
        {
            return n;
        }
        return p + ">" + n;
    }
    return n;
}

bool Model::frequentlyUpdated() const
{
    return m_frequentlyUpdated;
}

void Model::setFrequentlyUpdated(const bool _b)
{
    if(m_frequentlyUpdated != _b)
    {
        m_frequentlyUpdated = _b;
        emit propertiesChanged();
    }
}

bool Model::hasCableFrom(Model* _m) const
{
    return false;
}
