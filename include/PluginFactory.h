/*
 * PluginFactory.h
 *
 * Copyright (c) 2015 Lukas W <lukaswhl/at/gmail.com>
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

#ifndef PLUGINFACTORY_H
#define PLUGINFACTORY_H

#include "Plugin.h"
#include "export.h"

#include <QFileInfo>
#include <QHash>
#include <QList>
#include <QMap>

#include <memory>

class QLibrary;

class EXPORT PluginFactory
{
  public:
    struct PluginInfo
    {
        PluginInfo() : library(nullptr), descriptor(nullptr)
        {
        }

        QFileInfo                 file;
        std::shared_ptr<QLibrary> library;
        Plugin::Descriptor*       descriptor;

        const QString name() const;

        bool isNull() const
        {
            return library==nullptr || descriptor==nullptr;
        }
    };

    typedef QList<PluginInfo> PluginInfoList;

    typedef QHash<QString, PluginInfo> PluginInfos;

    typedef QMultiMap<Plugin::PluginTypes, Plugin::Descriptor*>
            PluginDescriptors;

    PluginFactory();
    virtual ~PluginFactory();

    /// Returns the singleton instance of PluginFactory. You won't need to
    /// call this directly, use pluginFactory instead.
    static PluginFactory* instance();

    /// Returns a list of all found plugins' descriptors.
    const Plugin::DescriptorList descriptors() const;
    const Plugin::DescriptorList descriptors(Plugin::PluginTypes type) const;

    /// Returns a list of all found plugins' PluginFactory::PluginInfo
    /// objects.
    const PluginInfoList pluginInfos() const;
    /// Returns a plugin that support the given file extension
    const PluginInfo pluginSupportingExtension(const QString& ext);

    /// Returns the PluginInfo object of the plugin with the given name.
    /// If the plugin is not found, an empty PluginInfo is returned (use
    /// PluginInfo::isNull() to check this).
    const PluginInfo pluginInfo(const QString& name) const;

    /// When loading a library fails during discovery, the error string is
    /// saved. It can be retrieved by calling this function.
    QString errorString(QString pluginName) const;

  public slots:
    void discoverLmmsPlugins();
    // void discoverLadspaPlugins();
#ifdef LMMS_HAVE_LILV
    void discoverLV2Plugins();
#endif

  private:
    PluginDescriptors         m_descriptors;
    PluginInfos               m_infos;
    QMap<QString, PluginInfo> m_pluginByExt;
    QHash<QString, QString>   m_errors;

    static std::unique_ptr<PluginFactory> s_instance;
};

#define pluginFactory PluginFactory::instance()

#endif  // PLUGINFACTORY_H
