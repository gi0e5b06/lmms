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

#include <QHash>
#include <QUuid>

#include <typeinfo>

QHash<QString, Model*> Model::s_models;

Model::Model(Model*         _parent,
             const QString& _displayName,
             const QString& _objectName,
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
    setObjectName(_objectName.isEmpty() ? normalizeObjectName(_displayName)
                                        : _objectName);
    m_debug_uuid = QUuid::createUuid().toString();
}

static QHash<QString, bool> s_destructorTracker;

Model::~Model()
{
    // qInfo("Model::~Model START %s", qPrintable(m_displayName));

    if(hasUuid())
        s_models.remove(m_uuid);

    if(s_destructorTracker.contains(m_debug_uuid))
    {
        BACKTRACE
        qCritical("Model::~Model %s destroyed twice",
                  qPrintable(m_displayName));
        return;
    }
    s_destructorTracker.insert(m_debug_uuid, true);

    /*
    qInfo("Model::~Model CHILDREN");
    for(QObject* o: children())
        qInfo("  - child %s %s", qPrintable(o->objectName()),
              typeid(o).name());
    */

    emit modelDestroyed();
    // qInfo("Model::~Model END %s", qPrintable(m_displayName));
}

const QString Model::uuid() const
{
    if(!hasUuid())
    {
        Model* m  = const_cast<Model*>(this);
        m->m_uuid = QUuid::createUuid().toString().replace("{", "").replace(
                "}", "");
        s_models.insert(m->m_uuid, m);
        BACKTRACE
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

QString Model::objectName() const
{
    QString r = QObject::objectName();
    if(r.isEmpty())
        r = normalizeObjectName(displayName());  // fullDisplayName();
    return r;
}

QString Model::normalizeObjectName(const QString& _s) const
{
    QString r(_s);
    r.replace('/', '_');
    // r.replace('>', '/');
    r.replace(QRegExp("[#]"), "");
    r.replace(QRegExp("[(].*[)]"), "");

    QRegExp rx1("(^|/)[A-Z]+", Qt::CaseSensitive);
    int     p = 0;
    while(p >= 0 && p < r.length())
    {
        // qInfo("on: p=%d r=%s", p, qPrintable(r));
        p = rx1.indexIn(r, p);
        if(p < 0)
            break;

        int n = rx1.matchedLength();
        // qInfo("    matches %s", qPrintable(r.mid(p, n)));
        r = r.left(p) + r.mid(p, n).toLower() + r.mid(p + n);
        p++;  //= n;
    }

    QRegExp rx2(" +.");
    p = 0;
    while(p >= 0 && p < r.length())
    {
        // qInfo("on: p=%d r=%s", p, qPrintable(r));
        p = rx2.indexIn(r, p);
        if(p < 0)
            break;

        int n = rx2.matchedLength();
        r     = r.left(p) + r.mid(p + n - 1, 1).toUpper() + r.mid(p + n);
        p++;
        // if(r.length() > 50) break;
    }

    r.replace(QRegExp("[ -]"), "");
    // r = QString("{") + r + "}";
    return r;
}

QString Model::fullObjectName() const
{
    const QString& n = objectName();
    const Model*   m = parentModel();

    if(m != nullptr)
    {
        const QString p = m->fullObjectName();
        if(p.isEmpty())
        {
            if(n.isEmpty())
                return QString::null;
            else
                return n;
        }

        return p + "." + n;
    }

    return n;
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
    const Model*   m = parentModel();

    if(m != nullptr)
    {
        const QString p = m->fullDisplayName();
        if(p.isEmpty())
        {
            if(n.isEmpty())
                return QString::null;
            else
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
