/*
 * PluginFactory.cpp
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

#include "PluginFactory.h"

#include "ConfigManager.h"
#include "Engine.h"
#include "LV22LMMS.h"

#include <QApplication>
//#include <QCoreApplication>
//#include <QDebug>
#include <QDir>
#include <QLibrary>
#include <QSet>

#ifdef LMMS_BUILD_WIN32
QStringList nameFilters("*.dll");
#else
QStringList nameFilters("lib*.so");
#endif

qint64 qHash(const QFileInfo& fi)
{
    return qHash(fi.absoluteFilePath());
}

std::unique_ptr<PluginFactory> PluginFactory::s_instance;

PluginFactory::PluginFactory()
{
    // Adds a search path relative to the main executable if the path exists.
    auto addRelativeIfExists = [](const QString& path) {
        QDir dir(qApp->applicationDirPath());
        if(!path.isEmpty() && dir.cd(path))
        {
            QDir::addSearchPath("plugins", dir.absolutePath());
        }
    };

    // We're either running LMMS installed on an Unixoid or we're running a
    // portable version like we do on Windows.
    // We want to find our plugins in both cases:
    //  (a) Installed (Unix):
    //      e.g. binary at /usr/bin/lmms - plugin dir at /usr/lib/lmms/
    //  (b) Portable:
    //      e.g. binary at "C:/Program Files/LMMS/lmms.exe"
    //           plugins at "C:/Program Files/LMMS/plugins/"

#ifndef LMMS_BUILD_WIN32
    addRelativeIfExists("../lib/lmms");  // Installed
#endif
    addRelativeIfExists("plugins");  // Portable
#ifdef PLUGIN_DIR  // We may also have received a relative directory via a
                   // define
    addRelativeIfExists(PLUGIN_DIR);
#endif
    // Or via an environment variable:
    QString env_path;
    if(!(env_path = qgetenv("LMMS_PLUGIN_DIR")).isEmpty())
        QDir::addSearchPath("plugins", env_path);

    QDir::addSearchPath("plugins",
                        ConfigManager::inst()->workingDir() + "plugins");

    // discoverLmmsPlugins();
}

PluginFactory::~PluginFactory()
{
}

PluginFactory* PluginFactory::instance()
{
    if(s_instance == nullptr)
        s_instance.reset(new PluginFactory());

    return s_instance.get();
}

const Plugin::DescriptorList PluginFactory::descriptors() const
{
    return m_descriptors.values();
}

const Plugin::DescriptorList
        PluginFactory::descriptors(Plugin::PluginTypes type) const
{
    return m_descriptors.values(type);
}

const PluginFactory::PluginInfoList PluginFactory::pluginInfos() const
{
    return m_infos.values();
}

const PluginFactory::PluginInfo
        PluginFactory::pluginSupportingExtension(const QString& ext)
{
    return m_pluginByExt.value(ext, PluginInfo());
}

const PluginFactory::PluginInfo PluginFactory::pluginInfo(
        const QString& name) const  // const char* name)
{
    for(const PluginInfo& info: m_infos)
    {
        // if(qstrcmp(info.descriptor->name, name) == 0)
        if(info.descriptor->name() == name)
            return info;
    }
    return PluginInfo();
}

QString PluginFactory::errorString(QString pluginName) const
{
    static QString notfound
            = qApp->translate("PluginFactory", "Plugin not found.");
    return m_errors.value(pluginName, notfound);
}

void PluginFactory::discoverLmmsPlugins()
{
    // DescriptorMap descriptors;
    // PluginInfoList pluginInfos;
    // m_pluginByExt.clear();

    QSet<QFileInfo> files;
    for(const QString& searchPath: QDir::searchPaths("plugins"))
    {
        files.unite(QDir(searchPath).entryInfoList(nameFilters).toSet());
    }

    // Cheap dependency handling: zynaddsubfx needs ZynAddSubFxCore. By
    // loading all libraries twice we ensure that libZynAddSubFxCore is found.
    for(const QFileInfo& file: files)
    {
        QLibrary(file.absoluteFilePath()).load();
    }

    for(const QFileInfo& file: files)
    {
        QString ap = file.absoluteFilePath();
        if(m_infos.contains(ap))
            continue;

        QString descriptorName = file.baseName() + "_plugin_descriptor";
        if(descriptorName.left(3) == "lib")
        {
            descriptorName = descriptorName.mid(3);
        }

        if(descriptorName == "carlabase_plugin_descriptor"
           || descriptorName == "vstbase_plugin_descriptor"
           || descriptorName == "jalv_plugin_descriptor"
           || descriptorName == "ZynAddSubFxCore_plugin_descriptor")
            continue;

        auto library = std::make_shared<QLibrary>(ap);

        if(library == nullptr || !library->load())
        {
            m_errors[file.baseName()] = library->errorString();
            qWarning("Warning: %s", qPrintable(library->errorString()));
            continue;
        }

        // qInfo("LMMS: descriptorName=%s", qPrintable(descriptorName));

        if(library->resolve("lmms_plugin_main") == nullptr)
        {
            qWarning("Warning: not a plugin: %s",
                     qPrintable(library->errorString()));
            continue;
        }

        Plugin::Descriptor* pluginDescriptor
                = reinterpret_cast<Plugin::Descriptor*>(library->resolve(
                        descriptorName.toUtf8().constData()));
        if(pluginDescriptor == nullptr)
        {
            qWarning("Warning: %s",
                     qPrintable(QString("LMMS plugin %1 does not have a "
                                        "plugin descriptor named %2")
                                        .arg(ap)
                                        .arg(descriptorName)));
            continue;
        }

        qInfo("LMMS:   '%s' %s [%s]",
              qPrintable(pluginDescriptor->displayName()), qPrintable(ap),
              "_type_");  // TYPES[plugIn->type]);

        // qInfo("Factory: lmms: %s", pluginDescriptor->name);
        m_descriptors.insert(pluginDescriptor->type(), pluginDescriptor);

        PluginInfo info;
        info.file       = file;
        info.library    = library;
        info.descriptor = pluginDescriptor;

        m_infos.insert(ap, info);

        for(const QString& ext:
            info.descriptor->supportedFileTypes().split(','))
        {
            // tmp: only keep the first one
            if(!m_pluginByExt.contains(ext))
                m_pluginByExt.insert(ext, info);
        }
    }

    // m_infos = pluginInfos;
    // m_descriptors = descriptors;
}

#ifdef LMMS_HAVE_LILV
void PluginFactory::discoverLV2Plugins()
{
    LV22LMMS* m = Engine::getLV2Manager();
    for(lv2_key_t key: m->getValidEffects())
    {
        if(m_infos.contains(key))
        {
            qInfo("Notice: already registered: %s", qPrintable(key));
            continue;
        }

        /*
        Plugin::Descriptor* d = new Plugin::Descriptor();
        d->name               = key;//QString(key.constData());
        d->displayName        = QString(m->getName(key).constData())+" 1";
        d->description        = QString(m->getLabel(key).constData())+" 11";
        d->author             = QString(m->getMaker(key).constData());
        d->version            = 0;
        d->type               = Plugin::Effect;
        d->logo               = nullptr;
        d->supportedFileTypes = nullptr;

        qInfo("Factory: lv2: {%s} ''%s'' ''%s''", qPrintable(key),
        qPrintable(d->name), qPrintable(d->displayName));  //
        qPrintable(key));
        //qInfo("*******: lv2: {%s} ''%s'' ''%s''", qPrintable(key),
        //      qPrintable(m->getName(key)), qPrintable(m->getLabel(key)));
        m_descriptors.insert(Plugin::Effect, d);

        PluginInfo info;
        info.file       = key;
        info.library    = nullptr;  //_p.get_library_uri().as_uri();
        info.descriptor = d;

        m_infos.insert(key, info);
        */
    }
}
#endif

const QString PluginFactory::PluginInfo::name() const
{
    return descriptor ? descriptor->name() : QString();
}
