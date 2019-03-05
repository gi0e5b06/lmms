/*
 * Engine.cpp - implementation of LMMS' engine-system
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

#include "Engine.h"

#include "BBTrackContainer.h"
#include "BandLimitedWave.h"
#include "ConfigManager.h"
#include "FxMixer.h"
#include "Ladspa2LMMS.h"
#include "Mixer.h"
#include "PluginFactory.h"
#include "PresetPreviewPlayHandle.h"
#include "ProjectJournal.h"
#include "Song.h"
#include "lmmsconfig.h"
//#include "Backtrace.h"

#include <QFuture>
#include <QtConcurrent>

real_t               LmmsCore::s_framesPerTick;
Mixer*               LmmsCore::s_mixer            = nullptr;
FxMixer*             LmmsCore::s_fxMixer          = nullptr;
Song*                LmmsCore::s_song             = nullptr;
Transportable*       LmmsCore::s_transport        = nullptr;
BBTrackContainer*    LmmsCore::s_bbTrackContainer = nullptr;
ProjectJournal*      LmmsCore::s_projectJournal   = nullptr;
Ladspa2LMMS*         LmmsCore::s_ladspaManager    = nullptr;
DummyTrackContainer* LmmsCore::s_dummyTC          = nullptr;

#ifdef LMMS_HAVE_LILV
#include "LV22LMMS.h"
LV22LMMS* LmmsCore::s_lv2Manager = nullptr;
#endif

void LmmsCore::init(bool renderOnly)
{
    qRegisterMetaType<tact_t>("tact_t");
    qRegisterMetaType<tick_t>("tick_t");
    qRegisterMetaType<real_t>("real_t");
    qRegisterMetaType<volume_t>("volume_t");
    qRegisterMetaType<panning_t>("panning_t");
    qRegisterMetaType<pitch_t>("pitch_t");
    qRegisterMetaType<frequency_t>("frequency_t");

    qRegisterMetaType<sample_t>("sample_t");
    qRegisterMetaType<sampleS16_t>("sampleS16_t");
    qRegisterMetaType<sample_rate_t>("sample_rate_t");
    qRegisterMetaType<f_cnt_t>("f_cnt_t");
    qRegisterMetaType<ch_cnt_t>("ch_cnt_t");
    qRegisterMetaType<bpm_t>("bpm_t");
    qRegisterMetaType<bitrate_t>("bitrate_t");
    qRegisterMetaType<fx_ch_t>("fx_ch_t");
    qRegisterMetaType<jo_id_t>("jo_id_t");

    qRegisterMetaType<const sampleFrame*>("const sampleFrame*");
    qRegisterMetaType<const surroundSampleFrame*>(
            "const surroundSampleFrame*");
    qRegisterMetaType<MidiTime>("MidiTime");
    qRegisterMetaType<const ValueBuffer*>("const ValueBuffer*");

    LmmsCore* engine = inst();

    QFuture<void> t1  = QtConcurrent::run(init1);
    QFuture<void> t1b = QtConcurrent::run(init1b);
    QFuture<void> t2  = QtConcurrent::run(init2);

    emit engine->initProgress(tr("Generating wavetables"));
    t1.waitForFinished();
    emit engine->initProgress(tr("Generating math functions"));
    t1b.waitForFinished();
    emit engine->initProgress(tr("Initializing data structures"));
    t2.waitForFinished();

    QFuture<void> t3a = QtConcurrent::run(init3a);
    QFuture<void> t3b = QtConcurrent::run(init3b);
#ifdef LMMS_HAVE_LILV
    emit engine->initProgress(tr("Initializing LV2 effects"));
#endif
    emit engine->initProgress(tr("Initializing Ladspa effects"));
    t3a.waitForFinished();
    t3b.waitForFinished();

    QFuture<void> t3c = QtConcurrent::run(init3c);
    emit          engine->initProgress(tr("Initializing Lmms effects"));
    t3c.waitForFinished();

    emit          engine->initProgress(tr("Adding effects"));
    QFuture<void> t3d = QtConcurrent::run(init3d);
    t3d.waitForFinished();

    QFuture<void> t4 = QtConcurrent::run(init4, renderOnly);
    emit          engine->initProgress(tr("Initializing Mixer"));
    t4.waitForFinished();

    emit engine->initProgress(tr("Initializing Song"));
    init5();
    // QFuture<void> t5 = QtConcurrent::run(init5);
    // t5.waitForFinished();

    emit engine->initProgress(tr("Initializing FX Mixer"));
    s_fxMixer = new FxMixer();

    emit          engine->initProgress(tr("Initializing BB"));
    QFuture<void> t7 = QtConcurrent::run(init7);

    t7.waitForFinished();

    s_projectJournal->setJournalling(true);

    emit engine->initProgress(tr("Opening audio and midi devices"));
    s_mixer->initDevices();

    PresetPreviewPlayHandle::init();
    s_dummyTC = new DummyTrackContainer();

    emit engine->initProgress(tr("Launching mixer threads"));
    s_mixer->startProcessing();
}

void LmmsCore::init1()
{
    // generate (load from file) bandlimited wavetables
    BandLimitedWave::generateWaves();
}

void LmmsCore::init1b()
{
    /*
    init_fastsqrtf01();
    init_fastnsinf01();
    init_fasttrianglef01();
    init_fastsawtoothf01();
    init_fastsquaref01();
    init_fastharshsawf01();
    init_fastpeakexpf01();
    init_fastrandf01();
    */
}

void LmmsCore::init2()
{
    s_projectJournal = new ProjectJournal();
}

void LmmsCore::init3a()
{
#ifdef LMMS_HAVE_LILV
    qInfo("Notice: loading LV2 descriptions");
    s_lv2Manager = new LV22LMMS();
#endif
}

void LmmsCore::init3b()
{
    s_ladspaManager = new Ladspa2LMMS();
}

void LmmsCore::init3c()
{
    qInfo("Notice: add Lmms plugins");
    PluginFactory::instance()->discoverLmmsPlugins();
}

void LmmsCore::init3d()
{
#ifdef LMMS_HAVE_LILV
    qInfo("Notice: add LV2 plugins");
    PluginFactory::instance()->discoverLV2Plugins();
#endif
}

void LmmsCore::init4(bool _renderOnly)
{
    s_mixer = new Mixer(_renderOnly);
}

void LmmsCore::init5()
{
    s_song      = new Song();
    s_transport = s_song;
}

void LmmsCore::init7()
{
    s_bbTrackContainer = new BBTrackContainer();
}

void LmmsCore::destroy()
{
    qWarning("Engine::destroy START");

    s_projectJournal->stopAllJournalling();
    transport()->transportStop();
    s_mixer->stopProcessing();
    qWarning("Engine::destroy processing stopped");

    PresetPreviewPlayHandle::cleanup();

    s_song->clearProject();

    DELETE_HELPER(s_bbTrackContainer);
    DELETE_HELPER(s_dummyTC);
    DELETE_HELPER(s_fxMixer);
    DELETE_HELPER(s_mixer);
    DELETE_HELPER(s_ladspaManager);
    // delete ConfigManager::inst();
    DELETE_HELPER(s_projectJournal);
    DELETE_HELPER(s_song);

    qInfo("ConfigManager::deinit");
    // delete ConfigManager::inst();
    ConfigManager::deinit();

    qInfo("Engine::destroy END");
}

void LmmsCore::updateFramesPerTick()
{
    s_framesPerTick = s_mixer->processingSampleRate() * 60. * 4.
                      / DefaultTicksPerTact / s_song->getTempo();
}

LmmsCore* LmmsCore::s_instanceOfMe = nullptr;
