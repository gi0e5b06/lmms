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

#include "Configuration.h"

#include <QLocale>

void lmms_default_configuration()
{
    DEFAULT_BOOL("app.disablebackup", false);
    DEFAULT_BOOL("app.nommpz", true);
    DEFAULT_STRING("app.language", QLocale::system().name().left(2));
    DEFAULT_BOOL("app.nomsgaftersetup", false);
    DEFAULT_BOOL("app.displaydbfs", false);
    DEFAULT_BOOL("app.openlastproject", false);

    DEFAULT_BOOL("ui.compacttrackbuttons", false);
    DEFAULT_BOOL("ui.disableautoquit", false);
    DEFAULT_BOOL("ui.enableautosave", true);
    DEFAULT_INT("ui.saveinterval", 120000);
    DEFAULT_BOOL("ui.syncvstplugins", true);
    DEFAULT_BOOL("ui.oneinstrumenttrackwindow", false);
    DEFAULT_BOOL("ui.smoothscroll", false);
    DEFAULT_INT("ui.enablerunningautosave", 0);
    DEFAULT_BOOL("ui.animateafp", true);
    DEFAULT_BOOL("ui.printnotelabels", false);
    DEFAULT_BOOL("ui.displaywaveform", true);
    DEFAULT_INT("ui.framespersecond", 10);
    DEFAULT_BOOL("ui.leftsidebar", false);

    DEFAULT_BOOL("tooltips.disabled", false);

    DEFAULT_INT("mixer.samplerate",48000);//44100);
    DEFAULT_INT("mixer.framesperaudiobuffer", 1024);
    DEFAULT_BOOL("mixer.hqaudio", false);
    DEFAULT_STRING("mixer.audiodev","");
    DEFAULT_STRING("mixer.mididev","");
}

/*
#include "ConfigManager.h"

bool  Configuration::toBool(const QString& _s);
int   Configuration::toInt(const QString& _s);
float Configuration::toFloat(const QString& _s)
{
    if(s_instance.m_floats.contains(_s))
        return s_instance.m_floats.value(_s);

    const int p = _s.indexOf(QChar('.'));
    if(p < 0)
        qWarning("Configuration: invalid float key: %s", qPrintable(s));
    const QString c = _s.left(p);
    const QString k = _s.mid(p+1);
    QString       v = ConfigManager::inst()->value(c, k, "");
    if(v != "")
    {
        bool  ok = false;
        float r  = v.toFloat(ok);
        if(ok)
        {
            s_instance.m_floats.insert(_s, r);
            return r;
        }
    }

    if(s_default.m_default.contains(_s))
    {
        float r = s_default.m_floats.value(_s);
        s_instance.m_floats.insert(_s, r);
        return r;
    }

    qWarning("Configuration: float key not found: %s", qPrintable(s));
    return 0.f;
}

const QString& Configuration::toString(const QString& _s)
{
    if(m_strings.contains(_s))
        return m_strings.value(_s);

    QString d = "";
    if(s_default.m_strings.contains(_s))
        d = s_default.m_strings.value(_s);

    const int p = _s.indexOf(QChar('.'));
    if(p < 0)
        qWarning("Warning: invalid configuration key: %s", qPrintable(s));
    const QString c = _s.left(p);
    const QString k = _s.mid(p+1);
    return ConfigManager::inst()->value(c, k, d);
}

void    Configuration::put(const QString& _s, bool _v);
int     Configuration::put(const QString& _s, int _v);
float   Configuration::put(const QString& _s, float _v);
QString Configuration::put(const QString& _s, const QString& _v);

void    Configuration::putDefault(const QString& _s, bool _v);
int     Configuration::putDefault(const QString& _s, int _v);
float   Configuration::putDefault(const QString& _s, float _v);
QString Configuration::putDefault(const QString& _s, const QString& _v);
*/
