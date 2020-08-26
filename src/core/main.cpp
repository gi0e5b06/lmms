/*
 * main.cpp - just main.cpp which is starting up app...
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2012-2013 Paul Giblock    <p/at/pgiblock.net>
 *
 * This file is part of LSMM -
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "lmmsconfig.h"

#include <csignal>

#ifdef LMMS_BUILD_WIN32
#include <windows.h>
#endif

#ifdef LMMS_HAVE_SCHED_H
#include "sched.h"
#endif

#ifdef LMMS_HAVE_PROCESS_H
#include <process.h>
#endif

#ifdef LMMS_HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef LMMS_DEBUG_FPE
#include <execinfo.h>  // For backtrace and backtrace_symbols_fd
#include <fenv.h>      // For feenableexcept
#include <unistd.h>    // For STDERR_FILENO
//#include <csignal> // To register the signal handler
#endif

#include "MainApplication.h"
#include "MemoryManager.h"
//#include "ConfigManager.h"
#include "Configuration.h"
//#include "NotePlayHandle.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "ImportFilter.h"
#include "MainWindow.h"
#include "OutputSettings.h"
#include "ProjectRenderer.h"
#include "RenderManager.h"
#include "Song.h"
//#include "SetupDialog.h"

#include "PerfLog.h"
#include "debug.h"
#include "denormals.h"
#include "embed.h"
#include "lmmsconfig.h"
#include "lmmsversion.h"
#include "versioninfo.h"

#include <QFileInfo>
#include <QLocale>
#include <QTimer>
#include <QTranslator>
//#include <QApplication>
#include <QMessageBox>
#include <QPushButton>
//#include <QTemporaryFile>
#include <QTextStream>

#ifdef LMMS_DEBUG_FPE
void onSignalFPE(int signum)
{
    static void* s_previous = 0;

    // Get a back trace
    void*  array[50];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 50);

    if(s_previous == array[2])
        return;
    s_previous = array[2];

    // fprintf(stderr,"Critical: signal %d received\n",signum);
    qCritical("Critical: signal %d received", signum);
    // /usr/lib/x86_64-linux-gnu/libQt5Core.so.5(_Z9qIsFinited+0x10)[0x7f5a964eeed0]
    // if(array[2]==(void*)0x7f5a964eeed0) return;

    backtrace_symbols_fd(array, size, STDERR_FILENO);

    // cleanup and close up stuff here
    // terminate program
    /*
    if(signum!=SIGFPE)
    {
            fprintf(stderr,"Fatal: exiting on signal %d",signum);
            exit(signum);
    }
    */
}
#endif

static inline QString baseName(const QString& file)
{
    return QFileInfo(file).absolutePath() + "/"
           + QFileInfo(file).completeBaseName();
}

inline void loadTranslation(const QString& tname,
                            const QString& dir
                            = ConfigManager::inst()->localeDir())
{
    QTranslator* t    = new QTranslator(QCoreApplication::instance());
    QString      name = tname + ".qm";

    t->load(name, dir);

    QCoreApplication::instance()->installTranslator(t);
}

void printVersion(char* executableName)
{
    printf("LMMS %s\n(%s %s, Qt %s, %s)\n\n"
           "Copyright (c) %s\n\n"
           "This program is free software; you can redistribute it and/or\n"
           "modify it under the terms of the GNU General Public\n"
           "License as published by the Free Software Foundation; either\n"
           "version 2 of the License, or (at your option) any later "
           "version.\n\n"
           "Try \"%s --help\" for more information.\n\n",
           LMMS_VERSION, PLATFORM, MACHINE, QT_VERSION_STR, GCC_VERSION,
           LMMS_PROJECT_COPYRIGHT, executableName);
}

void printHelp()
{
    printf("LMMS %s\n"
           "Copyright (c) %s\n\n"
           "Usage: lmms [ -a ]\n"
           "            [ -b <bitrate> ]\n"
           "            [ -c <configuration_file> ]\n"
           "            [ -d <in> ]\n"
           "            [ -f <format> ]\n"
           "            [ --geometry <geometry> ]\n"
           "            [ -h ]\n"
           "            [ -i <method> ]\n"
           "            [ --import <file> [-e]]\n"
           "            [ -l ]\n"
           "            [ -m <mode>]\n"
           "            [ -o <path> ]\n"
           "            [ -p ]\n"
           "            [ --profile <out> ]\n"
           "            [ -r <project_file> ] [ options ]\n"
           "            [ -s <samplerate> ]\n"
           "            [ -u <in> <out> ]\n"
           "            [ -v ]\n"
           "            [ -x <value> ]\n"
           "            [ <project_file> ]\n\n"
           "-a, --float                   32bit float bit depth\n"  // REQUIRED
           "-b, --bitrate <bitrate>       Specify output bitrate in KBit/s\n"
           "       Default: 160.\n"
           "-c, --config <configfile>     Get the configuration from "
           "<configfile>\n"
           "-d, --dump <in>               Dump XML of compressed file <in>\n"
           "-f, --format <format>         Specify format of render-output "
           "where\n"
           "       Format is either 'wav', 'flac', 'ogg' or 'mp3'.\n"
           "    --geometry <geometry>     Specify the size and position of "
           "the main window\n"
           "       geometry is <xsizexysize+xoffset+yoffsety>.\n"
           "-h, --help                    Show this usage information and "
           "exit.\n"
           "-i, --interpolation <method>  Specify interpolation method\n"
           "       Possible values:\n"
           "          - linear\n"
           "          - sincfastest (default)\n"
           "          - sincmedium\n"
           "          - sincbest\n"
           "    --import <in> [-e]        Import MIDI file <in>.\n"
           "       If -e is specified lmms exits after importing the file.\n"
           "-l, --loop                    Render as a loop\n"
           "-m, --mode                    Stereo mode used for MP3 export\n"
           "       Possible values: s, j, m\n"
           "         s: Stereo\n"
           "         j: Joint Stereo\n"
           "         m: Mono\n"
           "       Default: j\n"
           "-o, --output <path>           Render into <path>\n"
           "       For rendering a song, provide a file path\n"
           "       For rendering channels or tracks, provide a directory "
           "path\n"
           "-p, --play                    Play given project file\n"
           "    --profile <out>           Dump profiling information to file "
           "<out>\n"
           "-r, --render (--song|--channels|--tracks) <project>\n"
           "                              Render given project file\n"
           "-s, --samplerate <samplerate> Specify output samplerate in Hz\n"
           "       Range: 44100 (default) to 192000\n"
           "--song\n"
           "--channels\n"
           "-t, --tracks\n"
           "-u, --upgrade <in> [out]      Upgrade file <in> and save as "
           "<out>\n"
           "       Standard out is used if no output file is specifed\n"
           "-v, --version                 Show version information and "
           "exit.\n"
           "    --allowroot               Bypass root user startup check "
           "(use with caution).\n"
           "-x, --oversampling <value>    Specify oversampling\n"
           "       Possible values: 1, 2, 4, 8\n"
           "       Default: 2\n\n",
           LMMS_VERSION, LMMS_PROJECT_COPYRIGHT);
}

void fileCheck(QString& file)
{
    QFileInfo fileToCheck(file);

    if(!fileToCheck.size())
    {
        qCritical("The file %s does not have any content.\n",
                  file.toUtf8().constData());
        exit(EXIT_FAILURE);
    }
    else if(fileToCheck.isDir())
    {
        qCritical("%s is a directory.\n", file.toUtf8().constData());
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv)
{
    QThread::currentThread()->setObjectName("mainThread");

// /lib/x86_64-linux-gnu/libc.so.6(+0x3ef20)[0x7f34e1286f20]
// /usr/lib/x86_64-linux-gnu/libQt5Core.so.5(_Z9qIsFinited+0x10)[0x7f34e1a5bed0]
#undef LMMS_DEBUG_FPE

#ifdef LMMS_DEBUG_FPE
    // Enable exceptions for certain floating point results
    feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW | FE_UNDERFLOW);

    // Install the trap handler
    // register signal SIGFPE and signal handler
    signal(SIGFPE, onSignalFPE);
#endif

#ifdef LMMS_BUILD_WIN32
    if(!freopen("lmms-err.txt", "a", stderr))
        qWarning("Warning: Could not redirect stderr to lmms-err.txt");
#endif

    // initialize memory managers
    MM_INIT  // MemoryManager::init();
            ConfigManager::init(argv[0]);
    lmms_default_configuration();
    // NotePlayHandleManager::init();

    // intialize RNG
    srand(getpid() + time(0));

    disable_denormals();

    bool    coreOnly        = false;
    bool    fullscreen      = true;
    bool    exitAfterImport = false;
    bool    allowRoot       = false;
    bool    renderLoop      = false;
    bool    renderChannels  = false;
    bool    renderTracks    = false;
    QString fileToLoad, fileToImport, playOut, renderOut, testOut,
            profilerOutputFile, configFile;

    // first of two command-line parsing stages
    for(int i = 1; i < argc; ++i)
    {
        QString arg = argv[i];

        if(arg == "--help" || arg == "-h" || arg == "--version" || arg == "-v"
           || arg == "--test"
           // arg == "--play"    || arg == "-p" ||
           || arg == "--render" || arg == "-r")
        {
            coreOnly = true;
        }
        else if(arg == "--allowroot")
        {
            allowRoot = true;
        }
        else if(arg == "--geometry" || arg == "-geometry")
        {
            if(arg == "--geometry")
            {
                // Delete the first "-" so Qt recognize the option
                strcpy(argv[i], "-geometry");
            }
            // option -geometry is filtered by Qt later,
            // so we need to check its presence now to
            // determine, if the application should run in
            // fullscreen mode (default, no -geometry given).
            fullscreen = false;
        }
    }

#if !defined(LMMS_BUILD_WIN32) && !defined(LMMS_BUILD_HAIKU)
    if((getuid() == 0 || geteuid() == 0) && !allowRoot)
    {
        qCritical(
                "LMMS cannot be run as root.\nUse \"--allowroot\" to "
                "override.\n");
        return EXIT_FAILURE;
    }
#endif

    if(coreOnly)
        qInfo("Notice: core only (no gui)");
    else
        qInfo("Notice: gui available");

    QCoreApplication* app = coreOnly ? new QCoreApplication(argc, argv)
                                     : new MainApplication(argc, argv);

    Mixer::qualitySettings qs(Mixer::qualitySettings::Mode_HighQuality);
    // OutputSettings os( 44100, OutputSettings::BitRateSettings(160, false),
    // OutputSettings::Depth_16Bit, OutputSettings::StereoMode_JointStereo );
    OutputSettings os(44100, OutputSettings::BitRateSettings(320, false),
                      OutputSettings::Depth_F32,
                      OutputSettings::StereoMode_JointStereo);
    ProjectRenderer::ExportFileFormats eff = ProjectRenderer::WaveFile;

    // second of two command-line parsing stages
    for(int i = 1; i < argc; ++i)
    {
        QString arg = argv[i];

        if(arg == "--version" || arg == "-v")
        {
            printVersion(argv[0]);
            return EXIT_SUCCESS;
        }
        else if(arg == "--help" || arg == "-h")
        {
            printHelp();
            return EXIT_SUCCESS;
        }
        else if(arg == "--upgrade" || arg == "-u")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No input file specified.\n"
                        "       Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            DataFile dataFile(QString::fromLocal8Bit(argv[i]));

            if(argc > i + 1)  // output file specified
            {
                dataFile.writeFile(QString::fromLocal8Bit(argv[i + 1]));
            }
            else  // no output file specified; use stdout
            {
                QTextStream ts(stdout);
                dataFile.write(ts);
                fflush(stdout);
            }

            return EXIT_SUCCESS;
        }
        else if(arg == "--allowroot" || arg == "--allow-root")
        {
            // Ignore, processed earlier
#ifdef LMMS_BUILD_WIN32
            if(allowRoot)
            {
                qNotice("Notice: Option \"--allowroot\" is ignored on this "
                        "platform.");
            }
#endif
        }
        else if(arg == "--dump" || arg == "-d")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No input file specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            QFile f(QString::fromLocal8Bit(argv[i]));
            f.open(QIODevice::ReadOnly);
            QString d = qUncompress(f.readAll());
            printf("%s\n", d.toUtf8().constData());

            return EXIT_SUCCESS;
        }
        else if(arg == "--play" || arg == "-p")
        {
            /*
            ++i;

            if( i == argc )
            {
                    qWarning( "Error: No input file specified.\n"
                              "     : Try \"%s --help\" for more
            information.",argv[0]); return EXIT_FAILURE;
            }

            fileToLoad = QString::fromLocal8Bit( argv[i] );
            */
            playOut = "yes";
        }
        else if(arg == "--test")
        {
            testOut = "yes";
        }
        else if(arg == "--render" || arg == "-r")
        {
            /*
            ++i;

            if( i == argc )
            {
                    qWarning("Error: No input file specified.\n"
                             "     : Try \"%s --help\" for more
            information.",argv[0]); return EXIT_FAILURE;
            }
            fileToLoad = QString::fromLocal8Bit( argv[i] );
            */
            renderOut = "yes";  // fileToLoad;
        }
        else if(arg == "--loop" || arg == "-l")
        {
            renderLoop = true;
        }
        else if(arg == "--channels")
        {
            renderChannels = true;
            renderTracks   = false;
        }
        else if(arg == "--song")
        {
            renderChannels = false;
            renderTracks   = false;
        }
        else if(arg == "--tracks" || arg == "-t")
        {
            renderChannels = false;
            renderTracks   = true;
        }
        else if(arg == "--output" || arg == "-o")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No output file specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            renderOut = QString::fromLocal8Bit(argv[i]);
        }
        else if(arg == "--format" || arg == "-f")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No output format specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            const QString ext = QString(argv[i]);

            if(ext == "wav")
            {
                eff = ProjectRenderer::WaveFile;
            }
#ifdef LMMS_HAVE_OGGVORBIS
            else if(ext == "ogg")
            {
                eff = ProjectRenderer::OggFile;
            }
#endif
#ifdef LMMS_HAVE_MP3LAME
            else if(ext == "mp3")
            {
                eff = ProjectRenderer::MP3File;
            }
#endif
            else if(ext == "flac")
            {
                eff = ProjectRenderer::FlacFile;
            }
            else if(ext == "au")
            {
                eff = ProjectRenderer::AUFile;
            }
            else
            {
                qWarning(
                        "Error: Invalid output format %s.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[i], argv[0]);
                return EXIT_FAILURE;
            }
        }
        else if(arg == "--samplerate" || arg == "-s")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No samplerate specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            sample_rate_t sr = QString(argv[i]).toUInt();
            if(sr >= 44100 && sr <= 192000)
            {
                os.setSampleRate(sr);
            }
            else
            {
                qWarning(
                        "Error: Invalid samplerate %s.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[i], argv[0]);
                return EXIT_FAILURE;
            }
        }
        else if(arg == "--bitrate" || arg == "-b")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No bitrate specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            int br = QString(argv[i]).toUInt();

            if(br >= 64 && br <= 384)
            {
                OutputSettings::BitRateSettings bitRateSettings
                        = os.getBitRateSettings();
                bitRateSettings.setBitRate(br);
                os.setBitRateSettings(bitRateSettings);
            }
            else
            {
                qWarning(
                        "Error: Invalid bitrate %s.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[i], argv[0]);
                return EXIT_FAILURE;
            }
        }
        else if(arg == "--mode" || arg == "-m")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No stereo mode specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            QString const mode(argv[i]);

            if(mode == "s" || mode == "stereo")
            {
                os.setStereoMode(OutputSettings::StereoMode_Stereo);
            }
            else if(mode == "j" || mode == "joint")
            {
                os.setStereoMode(OutputSettings::StereoMode_JointStereo);
            }
            else if(mode == "m" || mode == "mono")
            {
                os.setStereoMode(OutputSettings::StereoMode_Mono);
            }
            else
            {
                qWarning(
                        "Error: Invalid stereo mode %s.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[i], argv[0]);
                return EXIT_FAILURE;
            }
        }
        else if(arg == "--float" || arg == "-a") // REQUIRED
        {
            os.setBitDepth(OutputSettings::Depth_F32);
        }
        else if(arg == "--interpolation" || arg == "-i")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No interpolation method specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            const QString ip = QString(argv[i]);

            if(ip == "linear")
            {
                qs.interpolation
                        = Mixer::qualitySettings::Interpolation_Linear;
            }
            else if(ip == "sincfastest")
            {
                qs.interpolation
                        = Mixer::qualitySettings::Interpolation_SincFastest;
            }
            else if(ip == "sincmedium")
            {
                qs.interpolation
                        = Mixer::qualitySettings::Interpolation_SincMedium;
            }
            else if(ip == "sincbest")
            {
                qs.interpolation
                        = Mixer::qualitySettings::Interpolation_SincBest;
            }
            else
            {
                qWarning(
                        "Error: Invalid interpolation method %s.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[i], argv[0]);
                return EXIT_FAILURE;
            }
        }
        else if(arg == "--oversampling" || arg == "-x")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No oversampling specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            int o = QString(argv[i]).toUInt();

            switch(o)
            {
                case 1:
                    qs.oversampling
                            = Mixer::qualitySettings::Oversampling_None;
                    break;
                case 2:
                    qs.oversampling = Mixer::qualitySettings::Oversampling_2x;
                    break;
                case 4:
                    qs.oversampling = Mixer::qualitySettings::Oversampling_4x;
                    break;
                case 8:
                    qs.oversampling = Mixer::qualitySettings::Oversampling_8x;
                    break;
                default:
                    qWarning(
                            "Error: Invalid oversampling %s.\n"
                            "     : Try \"%s --help\" for more information.",
                            argv[i], argv[0]);
                    return EXIT_FAILURE;
            }
        }
        else if(arg == "--import")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No file specified for importing.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            fileToImport = QString::fromLocal8Bit(argv[i]);

            // exit after import? (only for debugging)
            if(QString(argv[i + 1]) == "-e")
            {
                exitAfterImport = true;
                ++i;
            }
        }
        else if(arg == "--profile")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No profile specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            profilerOutputFile = QString::fromLocal8Bit(argv[i]);
        }
        else if(arg == "--config" || arg == "-c")
        {
            ++i;

            if(i == argc)
            {
                qWarning(
                        "Error: No configuration file specified.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[0]);
                return EXIT_FAILURE;
            }

            configFile = QString::fromLocal8Bit(argv[i]);
        }
        else
        {
            if(argv[i][0] == '-')
            {
                qWarning(
                        "Error: Invalid option %s.\n"
                        "     : Try \"%s --help\" for more information.",
                        argv[i], argv[0]);
                return EXIT_FAILURE;
            }
            fileToLoad = QString::fromLocal8Bit(argv[i]);
        }
    }

    if(testOut == "yes")
    {
        if(!fileToLoad.isEmpty())
            testOut = fileToLoad;
    }
    if(renderOut == "yes")
    {
        renderOut = fileToLoad;
    }
    if(playOut == "yes")
    {
        playOut = fileToLoad;
    }

    // Test file argument before continuing
    if(!fileToLoad.isEmpty())
    {
        fileCheck(fileToLoad);
    }
    else if(!fileToImport.isEmpty())
    {
        fileCheck(fileToImport);
    }

    ConfigManager::inst()->loadConfigFile(configFile);

    // set language
    QString pos = ConfigManager::inst()->value("app", "language");
    if(pos.isEmpty())
    {
        pos = QLocale::system().name().left(2);
    }

#ifdef LMMS_BUILD_WIN32
#undef QT_TRANSLATIONS_DIR
#define QT_TRANSLATIONS_DIR ConfigManager::inst()->localeDir()
#endif

#ifdef QT_TRANSLATIONS_DIR
    // load translation for Qt-widgets/-dialogs
    loadTranslation(QString("qt_") + pos, QString(QT_TRANSLATIONS_DIR));
#endif
    // load actual translation for LMMS
    loadTranslation(pos);

    // try to set realtime priority
#ifdef LMMS_BUILD_LINUX
#ifdef LMMS_HAVE_SCHED_H
#ifndef __OpenBSD__
    struct sched_param sparam;
    sparam.sched_priority = (sched_get_priority_max(SCHED_FIFO)
                             + sched_get_priority_min(SCHED_FIFO))
                            / 2;
    if(sched_setscheduler(0, SCHED_FIFO, &sparam) == -1)
    {
        qNotice("Notice: Could not set realtime priority.");
    }
#endif
#endif
#endif

#ifdef LMMS_BUILD_WIN32
    if(!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
    {
        qNotice("Notice: Could not set high priority.");
    }
#endif

#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags   = SA_SIGINFO;

    if(sigemptyset(&sa.sa_mask))
        qWarning("Error: Signal initialization failed.");

    if(sigaction(SIGPIPE, &sa, nullptr))
        qWarning("Error: Signal initialization failed.");
#endif

    bool destroyEngine = false;
    // qWarning("Test: OP '%s'", qPrintable(testOut));

    if(!testOut.isEmpty())
    {
        // qWarning("Test: Engine::init");
        Engine::init(true);
        destroyEngine = true;

        if(testOut != "yes")
        {
            qWarning("Test: Loading project... '%s'", qPrintable(testOut));
            PL_BEGIN("Project Loading")
            Engine::getSong()->loadProject(testOut);
            PL_END("Project Loading")
            qWarning("Test: Project loaded!");

            if(Engine::getSong()->isEmpty())
            {
                qCritical("Error: project '%s' is empty, aborting!",
                          fileToLoad.toUtf8().constData());
                exit(EXIT_FAILURE);
            }
        }

        qWarning("Test: Done. Waiting 5s.");
        QTimer::singleShot(5000, QCoreApplication::instance(), SLOT(quit()));
    }
    else if(!renderOut.isEmpty())
    {
        // if we have an output file for rendering, just render the song
        // without starting the GUI.

        // When rendering multiple tracks, renderOut is a directory
        // otherwise, it is a file, so we need to append the file extension
        if(!renderChannels && !renderTracks)
        {
            if(renderOut != "-")
            {
                renderOut
                        = baseName(renderOut)
                          + ProjectRenderer::getFileExtensionFromFormat(eff);
                QFileInfo fi(renderOut);
                // to check if(!fi.isWritable()) ...
            }
        }
        else
        {
            QFileInfo fi(renderOut);
            if(!fi.exists()
               || !fi.isDir())  // to check !fi.isWritable()||!fi.isWritable()
            {
                qCritical("Error: invalid output directory %s, aborting!",
                          renderOut.toUtf8().constData());
                exit(EXIT_FAILURE);
            }
        }

        Engine::init(true);
        destroyEngine = true;

        // qWarning( "Loading project..." );
        PL_BEGIN("Project Loading")
        Engine::getSong()->loadProject(fileToLoad);
        PL_END("Project Loading")

        if(Engine::getSong()->isEmpty())
        {
            qCritical("Error: project %s is empty, aborting!",
                      fileToLoad.toUtf8().constData());
            exit(EXIT_FAILURE);
        }
        qWarning("Done");

        Engine::getSong()->setExportLoop(renderLoop);

        // create renderer
        RenderManager* r = new RenderManager(qs, os, eff, renderOut);
        QCoreApplication::instance()->connect(r, SIGNAL(finished()),
                                              SLOT(quit()));

        // timer for progress-updates
        QTimer* t = new QTimer(r);
        r->connect(t, SIGNAL(timeout()), SLOT(updateConsoleProgress()));
        t->start(200);

        if(profilerOutputFile.isEmpty() == false)
        {
            Engine::mixer()->profiler().setOutputFile(profilerOutputFile);
        }

        // start now!
        if(renderChannels)
        {
            PL_BEGIN("Channels Rendering")
            r->renderChannels();
        }
        else if(renderTracks)
        {
            PL_BEGIN("Tracks Rendering")
            r->renderTracks();
        }
        else
        {
            PL_BEGIN("Project Rendering")
            r->renderProject();
        }
    }
    // if we have playOut, just play the song
    // without starting the GUI
    /*
    else if( !playOut.isEmpty() )
    {
            Engine::init( true );
            destroyEngine = true;

            qWarning( "Loading project..." );
            Engine::getSong()->loadProject( fileToLoad );
            if( Engine::getSong()->isEmpty() )
            {
                    qCritical( "Error: project %s is empty, aborting!",
    fileToLoad.toUtf8().constData() ); exit( EXIT_FAILURE );
            }
            qWarning( "Done" );

            if( profilerOutputFile.isEmpty() == false )
            {
                    Engine::mixer()->profiler().setOutputFile(
    profilerOutputFile );
            }

            //r->renderProject();
            //emit playTriggered();

            qWarning( "Playing project..." );
            //if( Engine::getSong()->playMode() != Song::Mode_PlaySong )
            Engine::getSong()->playSong();
            //else
            //Engine::getSong()->togglePause();
            qWarning( "Done" );
    }
    */
    else  // otherwise, start the GUI
    {
        // qInfo("main: initialize gui");
        new GuiApplication(playOut.isEmpty());

        // re-intialize RNG - shared libraries might have srand() or
        // srandom() calls in their init procedure
        srand(getpid() + time(0));

        // recover a file?
        QString recoveryFile = ConfigManager::inst()->recoveryFile();

        bool recoveryFilePresent = QFileInfo(recoveryFile).exists()
                                   && QFileInfo(recoveryFile).isFile();
        bool autoSaveEnabled = ConfigManager::inst()
                                       ->value("ui", "enableautosave")
                                       .toInt();

        if(!playOut.isEmpty())
        {
            recoveryFile        = "";
            recoveryFilePresent = false;
            autoSaveEnabled     = false;
        }

        if(recoveryFilePresent)
        {
            QMessageBox mb;
            mb.setWindowTitle(MainWindow::tr("Project recovery"));
            mb.setText(
                    QString("<html>"
                            "<p style=\"margin-left:6\">%1</p>"
                            "<table cellpadding=\"3\">"
                            "  <tr>"
                            "    <td><b>%2</b></td>"
                            "    <td>%3</td>"
                            "  </tr>"
                            "  <tr>"
                            "    <td><b>%4</b></td>"
                            "    <td>%5</td>"
                            "  </tr>"
                            "</table>"
                            "</html>")
                            .arg(MainWindow::tr(
                                         "There is a recovery file present. "
                                         "It looks like the last session did "
                                         "not end "
                                         "properly or another instance of "
                                         "LMMS is "
                                         "already running. Do you want to "
                                         "recover the "
                                         "project of this session?"),
                                 MainWindow::tr("Recover"),
                                 MainWindow::tr(
                                         "Recover the file. Please don't run "
                                         "multiple instances of LMMS when "
                                         "you do this."),
                                 MainWindow::tr("Discard"),
                                 MainWindow::tr("Launch a default session "
                                                "and delete "
                                                "the restored files. This is "
                                                "not reversible.")));

            mb.setIcon(QMessageBox::Warning);
            mb.setWindowIcon(embed::getIcon("icon"));
            mb.setWindowFlags(Qt::WindowCloseButtonHint);

            QPushButton* recover;
            QPushButton* discard;
            QPushButton* exit;

#if QT_VERSION >= 0x050000
            // setting all buttons to the same roles allows us
            // to have a custom layout
            discard = mb.addButton(MainWindow::tr("Discard"),
                                   QMessageBox::AcceptRole);
            recover = mb.addButton(MainWindow::tr("Recover"),
                                   QMessageBox::AcceptRole);

#else
            // in qt4 the button order is reversed
            recover = mb.addButton(MainWindow::tr("Recover"),
                                   QMessageBox::AcceptRole);
            discard = mb.addButton(MainWindow::tr("Discard"),
                                   QMessageBox::AcceptRole);

#endif

            // have a hidden exit button
            exit = mb.addButton("", QMessageBox::RejectRole);
            exit->setVisible(false);

            // set icons
            recover->setIcon(embed::getIcon("recover"));
            discard->setIcon(embed::getIcon("discard"));

            mb.setDefaultButton(recover);
            mb.setEscapeButton(exit);

            mb.exec();
            if(mb.clickedButton() == discard)
            {
                gui->mainWindow()->sessionCleanup();
            }
            else if(mb.clickedButton() == recover)  // Recover
            {
                fileToLoad = recoveryFile;
                gui->mainWindow()->setSession(
                        MainWindow::SessionState::Recover);
            }
            else  // Exit
            {
                return 0;
            }
        }

        if(!playOut.isEmpty())
        {
        }
        else
        {
            // first show the Main Window and
            // then try to load given file

            // [Settel] workaround: showMaximized() doesn't work with
            // FVWM2 unless the window is already visible -> show() first
            // qInfo("main window shows");
            gui->mainWindow()->show();
            if(fullscreen)
            {
                gui->mainWindow()->showMaximized();
            }
        }

        // Handle macOS-style FileOpen QEvents
        QString queuedFile = static_cast<MainApplication*>(app)->queuedFile();
        if(!queuedFile.isEmpty())
        {
            fileToLoad = queuedFile;
        }

        if(!fileToLoad.isEmpty())
        {
            if(fileToLoad == recoveryFile)
            {
                Engine::getSong()->createNewProjectFromTemplate(fileToLoad);
            }
            else
            {
                Engine::getSong()->loadProject(fileToLoad);
            }
        }
        else if(!fileToImport.isEmpty())
        {
            ImportFilter::import(fileToImport, Engine::getSong());
            if(exitAfterImport)
            {
                return EXIT_SUCCESS;
            }
        }
        // If enabled, open last project if there is one. Else, create
        // a new one.
        else if(ConfigManager::inst()->value("app", "openlastproject").toInt()
                && !ConfigManager::inst()->recentlyOpenedProjects().isEmpty()
                && !recoveryFilePresent)
        {
            QString f
                    = ConfigManager::inst()->recentlyOpenedProjects().first();
            QFileInfo recentFile(f);

            if(recentFile.exists() && recentFile.suffix().toLower() != "mpt")
            {
                Engine::getSong()->loadProject(f);
            }
            else
            {
                Engine::getSong()->createNewProject();
            }
        }
        else
        {
            Engine::getSong()->createNewProject();
        }

        if(!playOut.isEmpty())
        {
            Engine::getSong()->updateLength();
            QTimer* t1 = new QTimer(app);
            t1->setSingleShot(true);
            Engine::getSong()->connect(t1, SIGNAL(timeout()),
                                       SLOT(playSong()));
            t1->start(100);

            QTimer* t2 = new QTimer(app);
            gui->connect(t2, SIGNAL(timeout()), SLOT(hasSongFinished()));
            t2->start(100);
        }
        else if(autoSaveEnabled)
        {
            // Finally we start the auto save timer and also trigger the
            // autosave one time as recover.mmp is a signal to possible
            // other instances of LMMS.
            gui->mainWindow()->autoSaveTimerReset();
        }
    }

    // QThread::currentThread()->setPriority(QThread::LowPriority);

    // DEBUG_THREAD_PRINT
    const int ret = app->exec();
    delete app;

    if(!renderOut.isEmpty())
    {
        if(renderChannels)
        {
            PL_END("Channels Rendering")
        }
        else if(renderTracks)
        {
            PL_END("Tracks Rendering")
        }
        else
        {
            PL_END("Project Rendering")
        }

        // ProjectRenderer::updateConsoleProgress() doesn't return line after
        // render
        if(coreOnly)
        {
            printf("\n");
        }
    }

    // TODO: use aboutToQuit() signal

    if(destroyEngine)
        Engine::destroy();

    qInfo("Cleanup Start");
    // cleanup memory managers
    MM_CLEANUP
    // MemoryManager::cleanup();
    qInfo("Cleanup End");
    return ret;
}
