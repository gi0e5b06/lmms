/*
 * AudioFileFlac.cpp - Audio device which encodes a wave stream into
 *                     a FLAC file (Implementation).
 *
 * Copyright (c) 2018 gi0e5b06       <on github.com>
 * Copyright (c) 2017 Levin Oehlmann <irrenhaus3/at/gmail[dot]com> et al.
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

#include "AudioFileFlac.h"

#include "Engine.h"
#include "Mixer.h"
#include "Song.h"
#include "endian_handling.h"
#include "lmmsversion.h"

#include <memory>

AudioFileDevice* AudioFileFlac::getInst(const QString&        outputFilename,
                                        const OutputSettings& outputSettings,
                                        const ch_cnt_t        channels,
                                        Mixer*                mixer,
                                        bool&                 successful)
{
    AudioFileFlac* r = new AudioFileFlac(outputSettings, channels, successful,
                                         outputFilename, mixer);
    r->initOutputFile();
    r->openOutputFile();

    successful = r->outputFileOpened() && r->startEncoding();

    return r;
}

AudioFileFlac::AudioFileFlac(const OutputSettings& outputSettings,
                             const ch_cnt_t        channels,
                             bool&                 successful,
                             const QString&        file,
                             Mixer*                mixer) :
      AudioFileDevice(outputSettings, channels, file, mixer),
      m_sf(nullptr)
{
}

AudioFileFlac::~AudioFileFlac()
{
    finishEncoding();
}

bool AudioFileFlac::startEncoding()
{
    m_si.samplerate = sampleRate();
    m_si.channels   = channels();
    m_si.frames     = mixer()->framesPerPeriod();
    m_si.sections   = 1;
    m_si.seekable   = 0;

    m_si.format = SF_FORMAT_FLAC;

    switch(getOutputSettings().getBitDepth())
    {
        case OutputSettings::Depth_F64:
            m_si.format |= SF_FORMAT_DOUBLE;
            break;
        case OutputSettings::Depth_F32:
            m_si.format |= SF_FORMAT_FLOAT;
            break;
        case OutputSettings::Depth_S32:
            m_si.format |= SF_FORMAT_PCM_32;
            break;
        case OutputSettings::Depth_S24:
            m_si.format |= SF_FORMAT_PCM_24;
            break;
        case OutputSettings::Depth_S8:
            m_si.format |= SF_FORMAT_PCM_S8;
            break;
        case OutputSettings::Depth_S16:
        default:
            m_si.format |= SF_FORMAT_PCM_16;
            break;
            /*
            case OutputSettings::Depth_S24:
            case OutputSettings::Depth_S32:
                    // FLAC does not support 32bit sampling, so take it as 24.
                    m_si.format |= SF_FORMAT_PCM_24;
                    break;
            default:
                    m_si.format |= SF_FORMAT_PCM_16;
            */
    }

#ifdef LMMS_HAVE_SF_COMPLEVEL
    double compression = getOutputSettings().getCompressionLevel();
    sf_command(m_sf, SFC_SET_COMPRESSION_LEVEL, &compression, sizeof(double));
#endif

    m_sf = sf_open(
#ifdef LMMS_BUILD_WIN32
            outputFile().toLocal8Bit().constData(),
#else
            outputFile().toUtf8().constData(),
#endif
            SFM_WRITE, &m_si);

    sf_command(m_sf, SFC_SET_CLIPPING, nullptr, SF_TRUE);

    Song* song = Engine::getSong();

    sf_set_string(m_sf, SF_STR_TITLE,
                  song->songMetaData("SongTitle").toUtf8().constData());
    sf_set_string(m_sf, SF_STR_COPYRIGHT,
                  song->songMetaData("Copyright").toUtf8().constData());
    sf_set_string(m_sf, SF_STR_SOFTWARE,
                  QString("LSMM %1").arg(LMMS_VERSION).toUtf8().constData());
    sf_set_string(m_sf, SF_STR_ARTIST,
                  song->songMetaData("Artist").toUtf8().constData());
    sf_set_string(m_sf, SF_STR_DATE,
                  song->songMetaData("ReleaseDate").toUtf8().constData());
    sf_set_string(m_sf, SF_STR_ALBUM,
                  song->songMetaData("AlbumTitle").toUtf8().constData());
    sf_set_string(m_sf, SF_STR_LICENSE,
                  song->songMetaData("License").toUtf8().constData());
    sf_set_string(m_sf, SF_STR_TRACKNUMBER,
                  song->songMetaData("TrackNumber").toUtf8().constData());
    sf_set_string(m_sf, SF_STR_GENRE,
                  song->songMetaData("Genre").toUtf8().constData());

    QString comment(
            "BPM: %1\n"
            "IRCS: %2\n"
            "Subgenre: %3\n"
            "Website: %4\n"
            "Label: %5\n");
    comment = comment.arg(QString("%1/%2")
                                  .arg(song->getTimeSigModel().getNumerator())
                                  .arg(song->getTimeSigModel()
                                               .getDenominator()))
                      .arg(song->songMetaData("IRCS"))
                      .arg(song->songMetaData("Subgenre"))
                      .arg(song->songMetaData("ArtistWebsite"))
                      .arg(song->songMetaData("LabelWebsite"));
    sf_set_string(m_sf, SF_STR_COMMENT, comment.toUtf8().constData());

    return true;
}

void AudioFileFlac::writeBuffer(surroundSampleFrame const* _ab,
                                fpp_t const                _frames)
{
    Q_ASSERT(sizeof(sample_t) == sizeof(float));
    sf_write_float(m_sf, (const float*)_ab, _frames * channels());
}

void AudioFileFlac::finishEncoding()
{
    if(m_sf)
    {
        sf_write_sync(m_sf);
        sf_close(m_sf);
    }
}
