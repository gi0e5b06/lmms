/*
 * Engine.h - engine-system of LMMS
 *
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef ENGINE_H
#define ENGINE_H

#include "export.h"
#include "lmms_basics.h"

#include <QObject>
#include <QString>

class BBTrackContainer;
class DummyTrackContainer;
class FxMixer;
class ProjectJournal;
class Mixer;
class Song;
class Ladspa2LMMS;
class LV22LMMS;
class Transportable;

// Note: This class is called 'LmmsCore' instead of 'Engine' because of naming
// conflicts caused by ZynAddSubFX. See
// https://github.com/LMMS/lmms/issues/2269 and
// https://github.com/LMMS/lmms/pull/2118 for more details.
//
// The workaround was to rename Lmms' Engine so that it has a different symbol
// name in the object files, but typedef it back to 'Engine' and keep it
// inside of Engine.h so that the rest of the codebase can be oblivious to
// this issue (and it could be fixed without changing every single file).

class LmmsCore;
typedef LmmsCore Engine;

class EXPORT LmmsCore : public QObject
{
    Q_OBJECT

  public:
    static void init(bool renderOnly);
    static void destroy();

    static LmmsCore*            singleton();
    static Mixer*               mixer();
    static FxMixer*             fxMixer();
    static Song*                song();
    static Song*                getSong();  // Obsolete
    static Transportable*       transport();
    static BBTrackContainer*    getBBTrackContainer();
    static DummyTrackContainer* dummyTrackContainer();
    static ProjectJournal*      projectJournal();
    static Ladspa2LMMS*         getLADSPAManager();

#ifdef LMMS_HAVE_LILV
    static LV22LMMS* getLV2Manager();
#endif

    static real_t framesPerTick()
    {
        return s_framesPerTick;
    }
    static void updateFramesPerTick();

  signals:
    void initProgress(const QString& msg);

  private:
    static void init1();
    static void init1b();
    static void init2();
    static void init3a();
    static void init3b();
    static void init3c();
    static void init3d();
    static void init4(bool _renderOnly);
    static void init5();
    static void init7();

    static real_t         s_framesPerTick;
    static Transportable* s_transport;

    // core
    /*
    static Mixer*               s_mixer;
    static FxMixer*             s_fxMixer;
    static Song*                s_song;
    static BBTrackContainer*    s_bbTrackContainer;
    static ProjectJournal*      s_projectJournal;
    static DummyTrackContainer* s_dummyTC;

    static Ladspa2LMMS* s_ladspaManager;

#ifdef LMMS_HAVE_LILV
    static LV22LMMS* s_lv2Manager;
#endif

    // even though most methods are static, an instance is needed for Qt
    // slots/signals
    static LmmsCore* s_instanceOfMe;
    */

    friend class GuiApplication;
    friend class AudioJack;
};

#endif
