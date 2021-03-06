/*
 * ConfigManager.cpp - implementation of class ConfigManager
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "ConfigManager.h"

#include "Backtrace.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "ProjectVersion.h"
#include "debug.h"
#include "lmmsversion.h"

#include <QApplication>
#include <QDir>
#include <QDomElement>
#include <QMessageBox>
#include <QTextStream>

static inline QString ensureTrailingSlash(const QString& s)
{
    if(!s.isEmpty() && !s.endsWith('/') && !s.endsWith('\\'))
        return s + '/';

    return s;
}

ConfigManager* ConfigManager::s_singleton = nullptr;

void ConfigManager::init(const char* argv0)
{
    if(s_singleton == nullptr)
    {
        QDir appPath(QFileInfo(argv0).absolutePath());
        qInfo("Notice: appPath=%s", qPrintable(appPath.absolutePath()));
        s_singleton = new ConfigManager(appPath);
    }
    else
        qWarning("Fatal: Call ConfigManager::init() only once");
}

void ConfigManager::deinit()
{
    ConfigManager* old = s_singleton;
    if(old != nullptr)
    {
        s_singleton = nullptr;
        delete old;
    }
    else
        qFatal("Fatal: Call ConfigManager::deinit() only once");
}

ConfigManager* ConfigManager::inst()
{
    if(s_singleton == nullptr)
    {
        // BACKTRACE
        qWarning("Warning: Call ConfigManager::init() first");
    }

    return s_singleton;
}

ConfigManager::ConfigManager(QDir& appPath) :
      m_lmmsRcFile(QDir::home().absolutePath() + "/.lmmsrc.xml"),
      m_workingDir(QDir::home().absolutePath() + "/lmms/"),
      m_dataDir("data:/"), m_artworkDir(defaultArtworkDir()),
      /*
      m_vstDir(m_workingDir + "vst/"),
      m_gigDir(m_workingDir + "gig/"),
      m_sf2Dir(m_workingDir + "sf2/"),
      */
      m_version(defaultVersion())
{
    // If we're in development (lmms is not installed) let's get the source
    // and binary directories by reading the CMake Cache
    // qInfo("ConfigManager: before appDirPath()");
    // QDir appPath = qApp->applicationDirPath();
    // qInfo("ConfigManager: after appDirPath()");
    // If in tests, get parent directory
    if(appPath.dirName() == "tests")
        appPath.cdUp();

    /*
    QFile cmakeCache(appPath.absoluteFilePath("CMakeCache.txt"));
    if (cmakeCache.exists()) {
            cmakeCache.open(QFile::ReadOnly);
            QTextStream stream(&cmakeCache);

            // Find the lines containing something like
    lmms_SOURCE_DIR:static=<dir>
            // and lmms_BINARY_DIR:static=<dir>
            int done = 0;
            while(! stream.atEnd())
            {
                    QString line = stream.readLine();

                    if (line.startsWith("lmms_SOURCE_DIR:")) {
                            QString srcDir = line.section('=', -1).trimmed();
                            QDir::addSearchPath("data", srcDir + "/data/");
                            done++;
                    }
                    if (line.startsWith("lmms_BINARY_DIR:")) {
                            m_lmmsRcFile = line.section('=', -1).trimmed() +
    QDir::separator() +
                                                                                     ".lmmsrc.xml";
                            done++;
                    }
                    if (done == 2)
                    {
                            break;
                    }
            }

            cmakeCache.close();
    }
    */

    /*
#ifdef LMMS_BUILD_WIN32
    QDir::addSearchPath("data", qApp->applicationDirPath() + "/data/");
#else
    QDir::addSearchPath("data", qApp->applicationDirPath().section('/', 0, -2)
+ "/share/lmms/"); #endif
    */

    QString data;
    if(!(data = QString::fromLocal8Bit(qgetenv("LMMS_DATA_DIR"))).isEmpty())
        QDir::addSearchPath("data", data);
    else if(QFileInfo(data = appPath.absolutePath() + "/data/").exists())
        QDir::addSearchPath("data", data);
#ifndef LMMS_BUILD_WIN32
    else if(QFileInfo(data = appPath.absolutePath().section('/', 0, -2)
                             + "/share/lmms/")
                    .exists())
        QDir::addSearchPath("data", data);
#endif
    else
        qFatal("Fatal: data path for resources undefined");

    QString   fp;
    QFileInfo fi;
    if(!(fp = QString::fromLocal8Bit(qgetenv("LMMS_RC_FILE"))).isEmpty()
       && (fi = QFileInfo(fp)).exists())
    {
        m_lmmsRcFile = fi.absoluteFilePath();
        qInfo("Notice: LMMSRC=%s", qPrintable(fi.absoluteFilePath()));
    }
    else if((fi = QFileInfo(appPath.absolutePath(), ".lmmsrc.xml")).exists())
    {
        m_lmmsRcFile = fi.absoluteFilePath();
        qInfo("Notice: LMMSRC=%s", qPrintable(fi.absoluteFilePath()));
    }
}

ConfigManager::~ConfigManager()
{
    saveConfigFile();
}

void ConfigManager::upgrade_1_1_90()
{
    // Remove trailing " (bad latency!)" string which was once saved with
    // PulseAudio
    if(value("mixer", "audiodev").startsWith("PulseAudio ("))
    {
        setValue("mixer", "audiodev", "PulseAudio");
    }

    // MidiAlsaRaw used to store the device info as "Device" instead of
    // "device"
    if(value("MidiAlsaRaw", "device").isNull())
    {
        // copy "device" = "Device" and then delete the old "Device" (further
        // down)
        QString oldDevice = value("MidiAlsaRaw", "Device");
        setValue("MidiAlsaRaw", "device", oldDevice);
    }

    if(!value("MidiAlsaRaw", "device").isNull())
    {
        // delete the old "Device" in the case that we just copied it to
        // "device"
        //   or if the user somehow set both the "Device" and "device" fields
        deleteValue("MidiAlsaRaw", "Device");
    }
}

void ConfigManager::upgrade_1_1_91()
{
    // rename displaydbv to displaydbfs
    if(!value("app", "displaydbv").isNull())
    {
        setValue("app", "displaydbfs", value("app", "displaydbv"));
        deleteValue("app", "displaydbv");
    }
}

void ConfigManager::upgrade()
{
    // Skip the upgrade if versions match
    if(m_version == LMMS_VERSION)
        return;

    ProjectVersion createdWith = m_version;

    if(createdWith.setCompareType(ProjectVersion::Build) < "1.1.90")
        upgrade_1_1_90();

    if(createdWith.setCompareType(ProjectVersion::Build) < "1.1.91")
        upgrade_1_1_91();

    // Don't use old themes as they break the UI (i.e. 0.4 != 1.0, etc)
    if(createdWith.setCompareType(ProjectVersion::Minor) != LMMS_VERSION)
        m_artworkDir = defaultArtworkDir();

    // Bump the version, now that we are upgraded
    m_version = LMMS_VERSION;
}

QString ConfigManager::defaultVersion() const
{
    return LMMS_VERSION;
}

void ConfigManager::setArtworkDir(const QString& _ad)
{
    m_artworkDir = ensureTrailingSlash(_ad);
}

void ConfigManager::setBackgroundArtwork(const QString& _ba)
{
    m_backgroundArtwork = _ba;
}

bool ConfigManager::hasWorkingDir() const
{
    return QDir(m_workingDir).exists();
}

void ConfigManager::setWorkingDir(const QString& wd)
{
    m_workingDir = ensureTrailingSlash(QDir::cleanPath(wd));
}

void ConfigManager::setLADSPADir(const QString& _dir)
{
    m_ladDir = ensureTrailingSlash(_dir);
}

void ConfigManager::setLV2Dir(const QString& _dir)
{
    m_lv2Dir = ensureTrailingSlash(_dir);
}

void ConfigManager::setVSTDir(const QString& _dir)
{
    m_vstDir = ensureTrailingSlash(_dir);
}

#ifdef LMMS_HAVE_STK
void ConfigManager::setSTKDir(const QString& _dir)
{
    m_stkDir = ensureTrailingSlash(_dir);
}
#endif

void ConfigManager::setGIGDir(const QString& _dir)
{
    m_gigDir = ensureTrailingSlash(_dir);
}

void ConfigManager::setSF2Dir(const QString& _dir)
{
    m_sf2Dir = ensureTrailingSlash(_dir);
}

void ConfigManager::setDefaultSoundfont(const QString& _sf)
{
#ifdef LMMS_HAVE_FLUIDSYNTH
    m_defaultSoundfont = _sf;
#endif
}

void ConfigManager::createWorkingDir()
{
    QDir().mkpath(m_workingDir);

    QDir().mkpath(userPresetsDir());
    QDir().mkpath(userProjectsDir());
    QDir().mkpath(userTemplateDir());
    QDir().mkpath(userSamplesDir());
    QDir().mkpath(userWaveFormsDir());

    // QDir().mkpath(userGigDir());
    // QDir().mkpath(userSf2Dir());
    // QDir().mkpath(userVstDir());
    // QDir().mkpath(userLadspaDir());
}

void ConfigManager::addRecentlyOpenedProject(const QString& file)
{
    QFileInfo recentFile(file);
    if(recentFile.suffix().toLower() == "mmp"
       || recentFile.suffix().toLower() == "mmpz"
       || recentFile.suffix().toLower() == "mpt")
    {
        m_recentlyOpenedProjects.removeAll(file);
        if(m_recentlyOpenedProjects.size() > 50)
            m_recentlyOpenedProjects.removeLast();

        m_recentlyOpenedProjects.prepend(file);
        ConfigManager::inst()->saveConfigFile();
    }
}

const QString& ConfigManager::value(const QString& cls,
                                    const QString& attribute) const
{
    if(m_settings.contains(cls))
    {
        for(StringPairs::const_iterator it = m_settings[cls].begin();
            it != m_settings[cls].end(); ++it)
            if((*it).first == attribute)
                return (*it).second;
    }

    static QString empty;
    return empty;
}

const QString& ConfigManager::value(const QString& cls,
                                    const QString& attribute,
                                    const QString& defaultVal) const
{
    const QString& val = value(cls, attribute);
    return val.isEmpty() ? defaultVal : val;
}

void ConfigManager::setValue(const QString& cls,
                             const QString& attribute,
                             const QString& value)
{
    if(m_settings.contains(cls))
    {
        for(StringPairs::iterator it = m_settings[cls].begin();
            it != m_settings[cls].end(); ++it)
            if((*it).first == attribute)
            {
                (*it).second = value;
                return;
            }
    }

    // not in map yet, so we have to add it...
    m_settings[cls].append(qMakePair(attribute, value));
}

void ConfigManager::deleteValue(const QString& cls, const QString& attribute)
{
    if(m_settings.contains(cls))
    {
        for(StringPairs::iterator it = m_settings[cls].begin();
            it != m_settings[cls].end(); ++it)
            if((*it).first == attribute)
            {
                m_settings[cls].erase(it);
                return;
            }
    }
}

void ConfigManager::loadConfigFile(const QString& configFile)
{
    // read the XML file and create DOM tree
    // Allow configuration file override through --config commandline option
    if(!configFile.isEmpty())
        m_lmmsRcFile = configFile;

    QFile        cfg_file(m_lmmsRcFile);
    QDomDocument dom_tree;

    if(cfg_file.open(QIODevice::ReadOnly))
    {
        QString errorString;
        int     errorLine, errorCol;
        if(dom_tree.setContent(&cfg_file, false, &errorString, &errorLine,
                               &errorCol))
        {
            // get the head information from the DOM
            QDomElement root = dom_tree.documentElement();

            QDomNode node = root.firstChild();

            // Cache the config version for upgrade()
            if(!root.attribute("version").isNull())
            {
                m_version = root.attribute("version");
            }

            // create the settings-map out of the DOM
            while(!node.isNull())
            {
                if(node.isElement() && node.toElement().hasAttributes())
                {
                    StringPairs attr;
                    QDomNamedNodeMap node_attr
                            = node.toElement().attributes();
                    for(int i = 0; i < node_attr.count(); ++i)
                    {
                        QDomNode n = node_attr.item(i);
                        if(n.isAttr())
                        {
                            attr.push_back(qMakePair(n.toAttr().name(),
                                                     n.toAttr().value()));
                        }
                    }
                    m_settings[node.nodeName()] = attr;
                }
                else if(node.nodeName() == "recentfiles")
                {
                    m_recentlyOpenedProjects.clear();
                    QDomNode n = node.firstChild();
                    while(!n.isNull())
                    {
                        if(n.isElement() && n.toElement().hasAttributes())
                        {
                            m_recentlyOpenedProjects
                                    << n.toElement().attribute("path");
                        }
                        n = n.nextSibling();
                    }
                }
                node = node.nextSibling();
            }

            if(value("paths", "artwork") != "")
            {
                m_artworkDir = value("paths", "artwork");
#ifdef LMMS_BUILD_WIN32
                // Detect a QDir/QFile hang on Windows
                // see issue #3417 on github
                bool badPath = (m_artworkDir == "/" || m_artworkDir == "\\");
#else
                bool badPath = false;
#endif

                if(badPath || !QDir(m_artworkDir).exists()
                   || !QFile(m_artworkDir + "/style.css").exists())
                {
                    m_artworkDir = defaultArtworkDir();
                }
                m_artworkDir = ensureTrailingSlash(m_artworkDir);
            }
            setWorkingDir(value("paths", "workingdir"));

            setGIGDir(value("paths", "gigdir") == ""
                              ? gigDir()
                              : value("paths", "gigdir"));
            setSF2Dir(value("paths", "sf2dir") == ""
                              ? sf2Dir()
                              : value("paths", "sf2dir"));
            setVSTDir(value("paths", "vstdir"));
            setLADSPADir(value("paths", "laddir"));
#ifdef LMMS_HAVE_STK
            setSTKDir(value("paths", "stkdir"));
#endif
#ifdef LMMS_HAVE_FLUIDSYNTH
            setDefaultSoundfont(value("paths", "defaultsf2"));
#endif
            setBackgroundArtwork(value("paths", "backgroundartwork"));
        }
        else if(gui)
        {
            QMessageBox::warning(
                    nullptr, MainWindow::tr("Configuration file"),
                    MainWindow::tr("Error while parsing configuration file "
                                   "at line %1:%2: %3")
                            .arg(errorLine)
                            .arg(errorCol)
                            .arg(errorString));
        }
        cfg_file.close();
    }

    if(m_ladDir.isEmpty() || m_ladDir == QDir::separator() || m_ladDir == "/"
       || !QDir(m_ladDir).exists())
    {
        m_ladDir = "";
#ifdef LMMS_BUILD_LINUX
        QString d=QDir::home().absolutePath() + "/.ladspa/";
        if(QFile(d).exists())
            m_ladDir = d;
            // /usr/lib/ladspa/
#endif
        if(m_ladDir.isEmpty())
            m_ladDir = userPluginsDir();
    }

    if(m_lv2Dir.isEmpty() || m_lv2Dir == QDir::separator() || m_lv2Dir == "/"
       || !QDir(m_lv2Dir).exists())
    {
        m_lv2Dir = "";
#ifdef LMMS_BUILD_LINUX
        QString d=QDir::home().absolutePath() + "/.lv2/";
        if(QFile(d).exists())
            m_lv2Dir = d;
            // /usr/lib/lv2/
#endif
        if(m_lv2Dir.isEmpty())
            m_lv2Dir = userPluginsDir();
    }

    if(m_vstDir.isEmpty() || m_vstDir == QDir::separator() || m_vstDir == "/"
       || !QDir(m_vstDir).exists())
    {
        m_vstDir = "";
#ifdef LMMS_BUILD_WIN32
        QString programFiles = QString::fromLocal8Bit(getenv("ProgramFiles"));
        m_vstDir             = programFiles + "/VstPlugins/";
#endif
#ifdef LMMS_BUILD_LINUX
        QString d=QDir::home().absolutePath() + "/.vst/";
        if(QFile(d).exists())
            m_vstDir = d;
            // .vst3 /usr/lib/vst/
#endif
        if(m_vstDir.isEmpty())
            m_vstDir = userPluginsDir();
    }

#ifdef LMMS_HAVE_STK
    if(m_stkDir.isEmpty() || m_stkDir == QDir::separator() || m_stkDir == "/"
       || !QDir(m_stkDir).exists())
    {
#if defined(LMMS_BUILD_WIN32)
        m_stkDir = m_dataDir + "stk/rawwaves/";
#elif defined(LMMS_BUILD_APPLE)
        m_stkDir = qApp->applicationDirPath() + "/../share/stk/rawwaves/";
#else
        m_stkDir = "/usr/share/stk/rawwaves/";
#endif
    }
#endif

    upgrade();

    QStringList searchPaths;
    if(!qgetenv("LMMS_THEME_PATH").isNull())
        searchPaths << qgetenv("LMMS_THEME_PATH");
    searchPaths << artworkDir() << defaultArtworkDir();
    QDir::setSearchPaths("resources", searchPaths);

    // Create any missing subdirectories in the working dir, but only if the
    // working dir exists
    if(hasWorkingDir())
        createWorkingDir();
}

void ConfigManager::saveConfigFile()
{
    setValue("paths", "artwork", m_artworkDir);
    setValue("paths", "workingdir", m_workingDir);

    setValue("paths", "gigdir", m_gigDir);
    setValue("paths", "sf2dir", m_sf2Dir);
#ifdef LMMS_HAVE_STK
    setValue("paths", "stkdir", m_stkDir);
#endif

    setValue("paths", "laddir", m_ladDir);
    setValue("paths", "lv2dir", m_lv2Dir);
    setValue("paths", "vstdir", m_vstDir);

#ifdef LMMS_HAVE_FLUIDSYNTH
    setValue("paths", "defaultsf2", m_defaultSoundfont);
#endif

    setValue("paths", "backgroundartwork", m_backgroundArtwork);

    QDomDocument doc("lmms-config-file");

    QDomElement lmms_config = doc.createElement("lmms");
    lmms_config.setAttribute("version", m_version);
    doc.appendChild(lmms_config);

    for(SettingsMap::iterator it = m_settings.begin(); it != m_settings.end();
        ++it)
    {
        QDomElement n = doc.createElement(it.key());
        for(StringPairs::iterator it2 = (*it).begin();
            it2 != (*it).end(); ++it2)
            n.setAttribute((*it2).first, (*it2).second);

        lmms_config.appendChild(n);
    }

    QDomElement recent_files = doc.createElement("recentfiles");

    for(QStringList::iterator it = m_recentlyOpenedProjects.begin();
        it != m_recentlyOpenedProjects.end(); ++it)
    {
        QDomElement n = doc.createElement("file");
        n.setAttribute("path", *it);
        recent_files.appendChild(n);
    }

    lmms_config.appendChild(recent_files);

    QString xml = "<?xml version=\"1.0\"?>\n" + doc.toString(2);

    QFile outfile(m_lmmsRcFile);
    if(!outfile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        if(gui)
        {
            QString title, message;
            title   = MainWindow::tr("Could not open file");
            message = MainWindow::tr(
                              "Could not open file %1 "
                              "for writing.\nPlease make "
                              "sure you have write "
                              "permission to the file and "
                              "the directory containing the "
                              "file and try again!")
                              .arg(m_lmmsRcFile);
            QMessageBox::critical(nullptr, title, message, QMessageBox::Ok,
                                  QMessageBox::NoButton);
        }

        return;
    }

    outfile.write(xml.toUtf8());
    outfile.close();
}
