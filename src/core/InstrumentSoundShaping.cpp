/*
 * InstrumentSoundShaping.cpp - implementation of class InstrumentSoundShaping
 *
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "InstrumentSoundShaping.h"

#include "BasicFilters.h"
#include "Engine.h"
#include "EnvelopeAndLfoParameters.h"
#include "Instrument.h"
#include "InstrumentPlayHandle.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "embed.h"

#include <QDomElement>

const real_t CUT_FREQ_MULTIPLIER = 6000.;
const real_t RES_MULTIPLIER      = 2.;
const real_t RES_PRECISION       = 1000.;

// names for env- and lfo-targets - first is name being displayed to user
// and second one is used internally, e.g. for saving/restoring settings
const QString InstrumentSoundShaping::targetNames
        [InstrumentSoundShaping::NumTargets][3]
        = {{InstrumentSoundShaping::tr("VOLUME"), "vol",
            InstrumentSoundShaping::tr("Volume")},
           /*	InstrumentSoundShaping::tr( "Pan" ),
                   InstrumentSoundShaping::tr( "Pitch" ),*/
           {InstrumentSoundShaping::tr("CUTOFF"), "cut",
            InstrumentSoundShaping::tr("Cutoff frequency")},
           {InstrumentSoundShaping::tr("RESO"), "res",
            InstrumentSoundShaping::tr("Resonance")}};

InstrumentSoundShaping::InstrumentSoundShaping(
        InstrumentTrack* _instrument_track) :
      Model(_instrument_track, tr("Envelopes/LFOs")),
      m_instrumentTrack(_instrument_track), m_filterEnabledModel(false, this),
      m_filterModel(this, tr("Filter type")),
      m_filterCutModel(BasicFilters<>::maxFreq(),
                       BasicFilters<>::minFreq(),
                       BasicFilters<>::maxFreq(),
                       1.0,
                       this,
                       tr("Cutoff frequency")),
      // 14000,1,14000
      m_filterResModel(0.5,
                       BasicFilters<>::minQ(),
                       BasicFilters<>::maxQ(),
                       0.001,
                       this,
                       tr("Q/Resonance"))
{
    for(int i = 0; i < NumTargets; ++i)
    {
        real_t value_for_zero_amount = 0.0;
        if(i == Volume)
        {
            value_for_zero_amount = 1.0;
        }
        m_envLfoParameters[i]
                = new EnvelopeAndLfoParameters(value_for_zero_amount, this);
        m_envLfoParameters[i]->setDisplayName(
                tr(targetNames[i][2].toUtf8().constData()));
    }

    m_filterModel.addItem(tr("LowPass"), new PixmapLoader("filter_lp"));
    m_filterModel.addItem(tr("HiPass"), new PixmapLoader("filter_hp"));
    m_filterModel.addItem(tr("BandPass csg"), new PixmapLoader("filter_bp"));
    m_filterModel.addItem(tr("BandPass czpg"), new PixmapLoader("filter_bp"));
    m_filterModel.addItem(tr("Notch"), new PixmapLoader("filter_notch"));
    m_filterModel.addItem(tr("Allpass"), new PixmapLoader("filter_ap"));
    m_filterModel.addItem(tr("Moog"), new PixmapLoader("filter_lp"));
    m_filterModel.addItem(tr("2x LowPass"), new PixmapLoader("filter_2lp"));
    m_filterModel.addItem(tr("RC LowPass 12dB"),
                          new PixmapLoader("filter_lp"));
    m_filterModel.addItem(tr("RC BandPass 12dB"),
                          new PixmapLoader("filter_bp"));
    m_filterModel.addItem(tr("RC HighPass 12dB"),
                          new PixmapLoader("filter_hp"));
    m_filterModel.addItem(tr("RC LowPass 24dB"),
                          new PixmapLoader("filter_lp"));
    m_filterModel.addItem(tr("RC BandPass 24dB"),
                          new PixmapLoader("filter_bp"));
    m_filterModel.addItem(tr("RC HighPass 24dB"),
                          new PixmapLoader("filter_hp"));
    m_filterModel.addItem(tr("Vocal Formant Filter"),
                          new PixmapLoader("filter_hp"));
    m_filterModel.addItem(tr("2x Moog"), new PixmapLoader("filter_2lp"));
    m_filterModel.addItem(tr("SV LowPass"), new PixmapLoader("filter_lp"));
    m_filterModel.addItem(tr("SV BandPass"), new PixmapLoader("filter_bp"));
    m_filterModel.addItem(tr("SV HighPass"), new PixmapLoader("filter_hp"));
    m_filterModel.addItem(tr("SV Notch"), new PixmapLoader("filter_notch"));
    m_filterModel.addItem(tr("Fast Formant"), new PixmapLoader("filter_hp"));
    m_filterModel.addItem(tr("Tripole"), new PixmapLoader("filter_lp"));
    m_filterModel.addItem(tr("Brown"), new PixmapLoader("filter_lp"));
    m_filterModel.addItem(tr("Pink"), new PixmapLoader("filter_lp"));
}

InstrumentSoundShaping::~InstrumentSoundShaping()
{
}

real_t InstrumentSoundShaping::volumeLevel(NotePlayHandle* n,
                                           const f_cnt_t   frame)
{
    f_cnt_t envReleaseBegin
            = frame - n->releaseFramesDone() + n->framesBeforeRelease();

    if(n->isReleased() == false)
    {
        envReleaseBegin += Engine::mixer()->framesPerPeriod();
    }

    real_t level;
    m_envLfoParameters[Volume]->fillLevel(&level, frame, envReleaseBegin, 1);

    return level;
}

void InstrumentSoundShaping::processAudioBuffer(sampleFrame*    buffer,
                                                const fpp_t     frames,
                                                NotePlayHandle* nph)
{
    const f_cnt_t envTotalFrames  = nph->totalFramesPlayed();
    f_cnt_t       envReleaseBegin = envTotalFrames - nph->releaseFramesDone()
                              + nph->framesBeforeRelease();

    if(!nph->isReleased()
       || (nph->instrumentTrack()->isSustainPedalPressed()
           && !nph->isReleaseStarted()))
    {
        envReleaseBegin += frames;
    }

    if(nph->m_filter == nullptr && m_filterEnabledModel.value())
    {
        nph->m_filter
                = new BasicFilters<>(Engine::mixer()->processingSampleRate());
    }
    processAudioBuffer(buffer, frames, nph->m_filter, envTotalFrames,
                       envReleaseBegin);
}

void InstrumentSoundShaping::processAudioBuffer(sampleFrame*          buffer,
                                                const fpp_t           frames,
                                                InstrumentPlayHandle* iph)
{
    const f_cnt_t envTotalFrames  = 0;  // iph->totalFramesPlayed();
    f_cnt_t       envReleaseBegin = 1000000;
    // envTotalFrames - iph->releaseFramesDone() + iph->framesBeforeRelease();

    /*
if(!iph->isReleased()
   || (iph->instrumentTrack()->isSustainPedalPressed()
       && !iph->isReleaseStarted()))
{
    envReleaseBegin += frames;
}
    */
    if(iph->m_filter == nullptr && m_filterEnabledModel.value())
    {
        iph->m_filter
                = new BasicFilters<>(Engine::mixer()->processingSampleRate());
    }
    processAudioBuffer(buffer, frames, iph->m_filter, envTotalFrames,
                       envReleaseBegin);
}

void InstrumentSoundShaping::processAudioBuffer(sampleFrame*    buffer,
                                                const fpp_t     frames,
                                                BasicFilters<>* filter,
                                                f_cnt_t envTotalFrames,
                                                f_cnt_t envReleaseBegin)
{
    // because of optimizations, there's special code for several cases:
    // 	- cut- and res-lfo/envelope active
    // 	- cut-lfo/envelope active
    // 	- res-lfo/envelope active
    //	- no lfo/envelope active but filter is used

    // only use filter, if it is really needed

    if(m_filterEnabledModel.value())
    {
        real_t cutBuffer[frames];
        real_t resBuffer[frames];

        int old_filter_cut = 0;
        int old_filter_res = 0;

        filter->setFilterType(m_filterModel.value());

        if(m_envLfoParameters[Cut]->isUsed())
        {
            m_envLfoParameters[Cut]->fillLevel(cutBuffer, envTotalFrames,
                                               envReleaseBegin, frames);
        }
        if(m_envLfoParameters[Resonance]->isUsed())
        {
            m_envLfoParameters[Resonance]->fillLevel(
                    resBuffer, envTotalFrames, envReleaseBegin, frames);
        }

        const real_t fcv = m_filterCutModel.value();
        const real_t frv = m_filterResModel.value();

        if(m_envLfoParameters[Cut]->isUsed()
           && m_envLfoParameters[Resonance]->isUsed())
        {
            for(fpp_t frame = 0; frame < frames; ++frame)
            {
                const real_t new_cut_val
                        = EnvelopeAndLfoParameters::expKnobVal(
                                  cutBuffer[frame])
                                  * CUT_FREQ_MULTIPLIER
                          + fcv;

                const real_t new_res_val
                        = frv + RES_MULTIPLIER * resBuffer[frame];

                if(static_cast<int>(new_cut_val) != old_filter_cut
                   || static_cast<int>(new_res_val * RES_PRECISION)
                              != old_filter_res)
                {
                    // qInfo("CF=%f RQ=%f", new_cut_val, new_res_val);
                    filter->calcFilterCoeffs(new_cut_val, new_res_val);
                    old_filter_cut = static_cast<int>(new_cut_val);
                    old_filter_res
                            = static_cast<int>(new_res_val * RES_PRECISION);
                }

                buffer[frame][0] = filter->update(buffer[frame][0], 0);
                buffer[frame][1] = filter->update(buffer[frame][1], 1);
            }
        }
        else if(m_envLfoParameters[Cut]->isUsed())
        {
            for(fpp_t frame = 0; frame < frames; ++frame)
            {
                real_t new_cut_val = EnvelopeAndLfoParameters::expKnobVal(
                                             cutBuffer[frame])
                                             * CUT_FREQ_MULTIPLIER
                                     + fcv;

                if(static_cast<int>(new_cut_val) != old_filter_cut)
                {
                    // qInfo("CF=%f FRV=%f", new_cut_val, frv);
                    filter->calcFilterCoeffs(new_cut_val, frv);
                    old_filter_cut = static_cast<int>(new_cut_val);
                }

                buffer[frame][0] = filter->update(buffer[frame][0], 0);
                buffer[frame][1] = filter->update(buffer[frame][1], 1);
            }
        }
        else if(m_envLfoParameters[Resonance]->isUsed())
        {
            for(fpp_t frame = 0; frame < frames; ++frame)
            {
                real_t new_res_val = frv + RES_MULTIPLIER * resBuffer[frame];

                if(static_cast<int>(new_res_val * RES_PRECISION)
                   != old_filter_res)
                {
                    // qInfo("FCV=%f RQ=%f", fcv, new_res_val);
                    filter->calcFilterCoeffs(fcv, new_res_val);
                    old_filter_res
                            = static_cast<int>(new_res_val * RES_PRECISION);
                }

                buffer[frame][0] = filter->update(buffer[frame][0], 0);
                buffer[frame][1] = filter->update(buffer[frame][1], 1);
            }
        }
        else
        {
            filter->calcFilterCoeffs(fcv, frv);

            for(fpp_t frame = 0; frame < frames; ++frame)
            {
                buffer[frame][0] = filter->update(buffer[frame][0], 0);
                buffer[frame][1] = filter->update(buffer[frame][1], 1);
            }
        }
    }

    if(m_envLfoParameters[Volume]->isUsed())
    {
        real_t volBuffer[frames];
        m_envLfoParameters[Volume]->fillLevel(volBuffer, envTotalFrames,
                                              envReleaseBegin, frames);

        for(fpp_t frame = 0; frame < frames; ++frame)
        {
            real_t vol_level = volBuffer[frame];
            vol_level        = vol_level * vol_level;
            buffer[frame][0] = vol_level * buffer[frame][0];
            buffer[frame][1] = vol_level * buffer[frame][1];
        }
    }

    /*	else if( m_envLfoParameters[Volume]->isUsed() == false &&
       m_envLfoParameters[PANNING]->isUsed() )
            {
                    // only use panning-envelope...
                    for( fpp_t frame = 0; frame < frames; ++frame )
                    {
                            real_t vol_level = pan_buf[frame];
                            vol_level = vol_level*vol_level;
                            for( ch_cnt_t chnl = 0; chnl < DEFAULT_CHANNELS;
       ++chnl )
                            {
                                    buffer[frame][chnl] = vol_level *
       buffer[frame][chnl];
                            }
                    }
            }*/
}

f_cnt_t InstrumentSoundShaping::envFrames(const bool _only_vol) const
{
    f_cnt_t ret_val = m_envLfoParameters[Volume]->PAHD_Frames();

    if(_only_vol == false)
    {
        for(int i = Volume + 1; i < NumTargets; ++i)
        {
            if(m_envLfoParameters[i]->isUsed()
               && m_envLfoParameters[i]->PAHD_Frames() > ret_val)
            {
                ret_val = m_envLfoParameters[i]->PAHD_Frames();
            }
        }
    }
    return ret_val;
}

f_cnt_t InstrumentSoundShaping::releaseFrames() const
{
    if(m_envLfoParameters[Volume]->isUsed())
    {
        return m_envLfoParameters[Volume]->releaseFrames();
    }
    f_cnt_t ret_val = m_instrumentTrack->instrument()
                              ? m_instrumentTrack->instrument()
                                        ->desiredReleaseFrames()
                              : 0;

    for(int i = Volume + 1; i < NumTargets; ++i)
    {
        if(m_envLfoParameters[i]->isUsed())
        {
            ret_val = qMax(ret_val, m_envLfoParameters[i]->releaseFrames());
        }
    }
    return ret_val;
}

void InstrumentSoundShaping::saveSettings(QDomDocument& _doc,
                                          QDomElement&  _this)
{
    m_filterModel.saveSettings(_doc, _this, "ftype");
    m_filterCutModel.saveSettings(_doc, _this, "fcut");
    m_filterResModel.saveSettings(_doc, _this, "fres");
    m_filterEnabledModel.saveSettings(_doc, _this, "fwet");

    for(int i = 0; i < NumTargets; ++i)
    {
        m_envLfoParameters[i]
                ->saveState(_doc, _this)
                .setTagName(m_envLfoParameters[i]->nodeName()
                            + QString(targetNames[i][1]).toLower());
    }
}

void InstrumentSoundShaping::loadSettings(const QDomElement& _this)
{
    m_filterModel.loadSettings(_this, "ftype");
    m_filterCutModel.loadSettings(_this, "fcut");
    m_filterResModel.loadSettings(_this, "fres");
    m_filterEnabledModel.loadSettings(_this, "fwet");

    QDomNode node = _this.firstChild();
    while(!node.isNull())
    {
        if(node.isElement())
        {
            for(int i = 0; i < NumTargets; ++i)
            {
                if(node.nodeName()
                   == m_envLfoParameters[i]->nodeName()
                              + QString(targetNames[i][1]).toLower())
                {
                    m_envLfoParameters[i]->restoreState(node.toElement());
                }
            }
        }
        node = node.nextSibling();
    }
}
