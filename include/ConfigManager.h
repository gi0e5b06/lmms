/*
 * ConfigManager.h - class ConfigManager, a class for managing
 * LMMS-configuration
 *
 * Copyright (c) 2005-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "MemoryManager.h"
#include "export.h"
#include "lmmsconfig.h"

#include <QDir>
#include <QMap>
#include <QPair>
#include <QStringList>
#include <QVector>
//#include "Backtrace.h"

// class LmmsCore;

const QString PROJECTS_PATH  = "projects/";
const QString TEMPLATES_PATH = "templates/";
const QString PRESETS_PATH   = "presets/";
const QString SAMPLES_PATH   = "samples/";
// const QString GIG_PATH           = "soundfonts/gig/";
// const QString SF2_PATH           = "soundfonts/sf2/";
const QString SOUNDFONTS_PATH = "soundfonts/";
const QString PLUGINS_PATH    = "plugins/";
// const QString LADSPA_PATH        = "plugins/ladspa/";
// const QString LV2_PATH           = "plugins/lv2/";
// const QString SFZ_PATH           = "soundfonts/sfz/";
// const QString VST_PATH           = "plugins/vst/";
const QString WAVEFORMS_PATH = "waveforms/";

const QString DEFAULT_THEME_PATH = "themes/default/";
// const QString TRACK_ICON_PATH    = "track_icons/";
const QString LOCALE_PATH = "locale/";

class EXPORT ConfigManager
{
    MM_OPERATORS
  public:
    static void           init(const char* arg0);
    static void           deinit();
    static ConfigManager* inst();

    const QString& dataDir() const
    {
        return m_dataDir;
    }

    const QString& workingDir() const
    {
        return m_workingDir;
    }

    QString userPresetsDir() const
    {
        return workingDir() + PRESETS_PATH;
    }

    QString userProjectsDir() const
    {
        return workingDir() + PROJECTS_PATH;
    }

    QString userTemplateDir() const
    {
        return workingDir() + TEMPLATES_PATH;
    }

    QString userSamplesDir() const
    {
        return workingDir() + SAMPLES_PATH;
    }

    QString userSoundfontsDir() const
    {
        return workingDir() + SOUNDFONTS_PATH;
    }

    QString userWaveFormsDir() const
    {
        return workingDir() + WAVEFORMS_PATH;
    }

    QString userPluginsDir() const
    {
        return workingDir() + PLUGINS_PATH;
    }

    /*
    QString userGigDir() const
    {
        return workingDir() + SOUNDFONTS_PATH;//GIG_PATH;
    }

    QString userSf2Dir() const
    {
        return workingDir() + SOUNDFONTS_PATH;//SF2_PATH;
    }

    QString userLadspaDir() const
    {
        return workingDir() + PLUGINS_PATH;//LADSPA_PATH;
    }

    QString userLV2Dir() const
    {
        return workingDir() + PLUGINS_PATH;//LV2_PATH;
    }

    QString userVstDir() const
    {
        return workingDir() + PLUGINS_PATH;//VST_PATH; m_vstDir;
    }
    */

    QString factoryPresetsDir() const
    {
        return dataDir() + PRESETS_PATH;
    }

    QString factoryProjectsDir() const
    {
        return dataDir() + PROJECTS_PATH;
    }

    QString factoryTemplatesDir() const
    {
        return factoryProjectsDir() + TEMPLATES_PATH;
    }

    QString factorySamplesDir() const
    {
        return dataDir() + SAMPLES_PATH;
    }

    QString factoryWaveFormsDir() const
    {
        return factorySamplesDir() + WAVEFORMS_PATH;
    }

    QString defaultVersion() const;

    QString defaultArtworkDir() const
    {
        return m_dataDir + DEFAULT_THEME_PATH;
    }

    QString artworkDir() const
    {
        return m_artworkDir;
    }

    /*
    QString trackIconsDir() const
    {
        return m_dataDir + TRACK_ICON_PATH;
    }
    */

    QString localeDir() const
    {
        return m_dataDir + LOCALE_PATH;
    }

    // GIG
    const QString& gigDir() const
    {
        return m_gigDir;
    }
    void setGIGDir(const QString& gd);

    // SF2
    const QString& sf2Dir() const
    {
        return m_sf2Dir;
    }

    void setSF2Dir(const QString& sfd);

#ifdef LMMS_HAVE_STK
    const QString& stkDir() const
    {
        return m_stkDir;
    }
    void setSTKDir(const QString& _fd);
#endif

    // LADSPA
    const QString& ladspaDir() const
    {
        return m_ladDir;
    }
    void setLADSPADir(const QString& _fd);

    // LV2
    const QString& lv2Dir() const
    {
        return m_lv2Dir;
    }
    void setLV2Dir(const QString& _fd);

    // VST
    const QString& vstDir() const
    {
        return m_vstDir;
    }
    void setVSTDir(const QString& _vd);

    const QString recoveryFile() const
    {
        return m_workingDir + "recover.mmp";
    }

    const QString& version() const
    {
        return m_version;
    }

#ifdef LMMS_HAVE_FLUIDSYNTH
    const QString& defaultSoundfont() const
    {
        return m_defaultSoundfont;
    }
#endif

    const QString& backgroundArtwork() const
    {
        return m_backgroundArtwork;
    }

    inline const QStringList& recentlyOpenedProjects() const
    {
        return m_recentlyOpenedProjects;
    }

    // returns true if the working dir (e.g. $HOME/lmms) exists on disk
    bool hasWorkingDir() const;

    void addRecentlyOpenedProject(const QString& _file);

    const QString& value(const QString& cls, const QString& attribute) const;
    const QString& value(const QString& cls,
                         const QString& attribute,
                         const QString& defaultVal) const;

    void setValue(const QString& cls,
                  const QString& attribute,
                  const QString& value);
    void deleteValue(const QString& cls, const QString& attribute);

    void loadConfigFile(const QString& configFile = "");
    void saveConfigFile();

    void setWorkingDir(const QString& _wd);
    void setArtworkDir(const QString& _ad);

    void setVersion(const QString& _cv);
    void setDefaultSoundfont(const QString& _sf);
    void setBackgroundArtwork(const QString& _ba);

    // creates the working directory & subdirectories on disk.
    void createWorkingDir();

  private:
    static ConfigManager* s_instanceOfMe;

    ConfigManager(QDir& _appPath);
    // ConfigManager( const ConfigManager & _c );
    virtual ~ConfigManager();

    void upgrade_1_1_90();
    void upgrade_1_1_91();
    void upgrade();

    QString m_lmmsRcFile;
    QString m_workingDir;
    QString m_dataDir;
    QString m_artworkDir;
    QString m_vstDir;
    QString m_ladDir;
    QString m_lv2Dir;
    QString m_gigDir;
    QString m_sf2Dir;
    QString m_version;
#ifdef LMMS_HAVE_STK
    QString m_stkDir;
#endif
#ifdef LMMS_HAVE_FLUIDSYNTH
    QString m_defaultSoundfont;
#endif
    QString     m_backgroundArtwork;
    QStringList m_recentlyOpenedProjects;

    typedef QVector<QPair<QString, QString>> stringPairVector;
    typedef QMap<QString, stringPairVector>  settingsMap;
    settingsMap                              m_settings;

    // friend class LmmsCore;
};

#endif
