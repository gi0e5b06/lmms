/*
 * Configuration.cpp -
 *
 * Copyright (c) 2018 gi0e5b06
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "ConfigManager.h"
#include "debug.h"

#include <QVariant>

/*
#define CONFIG_BOOL(s) Configuration::toBool(s)
#define CONFIG_INT(s) Configuration::toInt(s)
#define CONFIG_FLOAT(s) Configuration::toFloat(s)
#define CONFIG_STRING(s) Configuration::toString(s)
#define CONFIG_DEFAULT(s, v) Configuration::putDefault(s, v)
#define CONFIG_PUT(s, v) Configuration::put(s, v)
*/

#define CONFIG_GET_BOOL(s) Configuration<bool>::get(s)
#define CONFIG_GET_INT(s) Configuration<int>::get(s)
#define CONFIG_GET_FLOAT(s) Configuration<float>::get(s)
#define CONFIG_GET_STRING(s) Configuration<QString>::get(s)

#define CONFIG_SET_BOOL(s, v) Configuration<bool>::set(s, v)
#define CONFIG_SET_INT(s, v) Configuration<int>::set(s, v)
#define CONFIG_SET_FLOAT(s, v) Configuration<float>::set(s, v)
#define CONFIG_SET_STRING(s, v) Configuration<QString>::set(s, v)

#define DEFAULT_BOOL(s, v) Configuration<bool>::setDefault(s, v)
#define DEFAULT_INT(s, v) Configuration<int>::setDefault(s, v)
#define DEFAULT_FLOAT(s, v) Configuration<float>::setDefault(s, v)
#define DEFAULT_STRING(s, v) Configuration<QString>::setDefault(s, v)

void lmms_default_configuration();

template <typename T>
class Configuration
{
  public:
    static T    get(const QString& _s);
    static void set(const QString& _s, T _v);
    static T    getDefault(const QString& _s);
    static void setDefault(const QString& _s, T _v);

  private:
    // Configuration();
    // virtual ~Configuration();

    QHash<QString, T> m_table;

    static Configuration<T> s_instance;
    static Configuration<T> s_default;
};

template <typename T>
Configuration<T> Configuration<T>::s_instance;
template <typename T>
Configuration<T> Configuration<T>::s_default;

template <typename T>
T Configuration<T>::get(const QString& _s)
{
    if(s_instance.m_table.contains(_s))
        return s_instance.m_table.value(_s);

    const int p = _s.indexOf(QChar('.'));
    if(p < 0)
    {
        qWarning("Configuration: invalid key: %s", qPrintable(_s));
    }
    else
    {
        const QString c = _s.left(p);
        const QString k = _s.mid(p + 1);
        // qInfo("Configuration::get c='%s'
        // k='%s'",qPrintable(c),qPrintable(k));
        QString v = ConfigManager::inst()->value(c, k, "");
        if(v != "")
        {
            T r = QVariant(v).value<T>();
            s_instance.m_table.insert(_s, r);
            return r;
        }
    }

    if(s_default.m_table.contains(_s))
    {
        T r = s_default.m_table.value(_s);
        s_instance.m_table.insert(_s, r);
        return r;
    }

    {
        qWarning("Configuration: key not found: %s", qPrintable(_s));
        return 0;
    }
}

template <typename T>
void Configuration<T>::set(const QString& _s, T _v)
{
    s_instance.m_table.insert(_s, _v);

    const int p = _s.indexOf(QChar('.'));
    if(p < 0)
    {
        qWarning("Configuration: invalid key: %s", qPrintable(_s));
    }
    else
    {
        const QString c = _s.left(p);
        const QString k = _s.mid(p + 1);
        ConfigManager::inst()->setValue(c, k, QVariant(_v).toString());
    }
}

template <typename T>
T Configuration<T>::getDefault(const QString& _s)
{
    return s_default.m_table.value(_s);
}

template <typename T>
void Configuration<T>::setDefault(const QString& _s, T _v)
{
    s_default.m_table.insert(_s, _v);
}

/*
class Configuration
{
  public:
    static bool           toBool(const QString& _s);
    static int            toInt(const QString& _s);
    static float          toFloat(const QString& _s);
    static const QString& toString(const QString& _s);

    static void put(const QString& _s, bool _v);
    static void put(const QString& _s, int _v);
    static void put(const QString& _s, float _v);
    static void put(const QString& _s, const QString& _v);

    static void putDefault(const QString& _s, bool _v);
    static void putDefault(const QString& _s, int _v);
    static void putDefault(const QString& _s, float _v);
    static void putDefault(const QString& _s, const QString& _v);

  private:
    Configuration();
    virtual ~Configuration();

    QHash<QString, bool>    m_bools;
    QHash<QString, int>     m_ints;
    QHash<QString, float>   m_floats;
    QHash<QString, QString> m_strings;

    static Configuration s_instance;
    static Configuration s_default;
};
*/

#endif
