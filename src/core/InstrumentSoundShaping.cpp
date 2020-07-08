/*
 * InstrumentSoundShaping.cpp -
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "InstrumentSoundShaping.h"

#include "BasicFilters.h"
#include "Engine.h"
#include "EnvelopeAndLfoParameters.h"
#include "Instrument.h"
#include "InstrumentPlayHandle.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "ProjectJournal.h"
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
      Model(_instrument_track, tr("Sound shaping")),
      m_instrumentTrack(_instrument_track),
      m_filter1EnabledModel(false, this, tr("Filter 1 enabled")),
      m_filter1TypeModel(this, tr("Filter 1 type")),
      m_filter1CutModel(BasicFilters<>::maxFreq(),
                        BasicFilters<>::minFreq(),
                        BasicFilters<>::maxFreq(),
                        1.0,
                        this,
                        tr("Cutoff frequency")),
      // 14000,1,14000
      m_filter1ResModel(0.5,
                        BasicFilters<>::minQ(),
                        BasicFilters<>::maxQ(),
                        0.001,
                        this,
                        tr("Q/Resonance")),
      m_filter1PassesModel(this, tr("Passes")),
      m_filter1GainModel(0., -1., 1., 0.001, this, tr("Shelf gain")),
      m_filter1ResponseModel(0., -1., 1., 0.001, this, tr("Response")),
      m_filter1FeedbackAmountModel(
              0., -1., 1., 0.01, this, tr("Feedback amount")),
      m_filter1FeedbackDelayModel(
              0.1, 0.1, 1000., 0.1, 1000., this, tr("Feedback delay")),
      m_filter2EnabledModel(false, this, tr("Filter 2 enabled")),
      m_filter2TypeModel(this, tr("Filter 2 type")),
      m_filter2CutModel(BasicFilters<>::maxFreq(),
                        BasicFilters<>::minFreq(),
                        BasicFilters<>::maxFreq(),
                        1.0,
                        this,
                        tr("Cutoff frequency")),
      // 14000,1,14000
      m_filter2ResModel(0.5,
                        BasicFilters<>::minQ(),
                        BasicFilters<>::maxQ(),
                        0.001,
                        this,
                        tr("Q/Resonance")),
      m_filter2PassesModel(this, tr("Passes")),
      m_filter2GainModel(0., -1., 1., 0.001, this, tr("Shelf gain")),
      m_filter2ResponseModel(0., -1., 1., 0.001, this, tr("Response")),
      m_filter2FeedbackAmountModel(
              0., -1., 1., 0.01, this, tr("Feedback amount")),
      m_filter2FeedbackDelayModel(
              0.1, 0.1, 1000., 0.1, 1000., this, tr("Feedback delay"))
{
    // Reduce the amount of computing when automated
    m_filter1CutModel.setStrictStepSize(true);
    m_filter1ResModel.setStrictStepSize(true);
    m_filter2CutModel.setStrictStepSize(true);
    m_filter2ResModel.setStrictStepSize(true);

    m_filter1PassesModel.addItem("once");
    m_filter1PassesModel.addItem("twice");
    m_filter1PassesModel.addItem("thrice");
    m_filter2PassesModel.addItem("once");
    m_filter2PassesModel.addItem("twice");
    m_filter2PassesModel.addItem("thrice");

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

    m_filter1TypeModel.addItem(tr("LowPass"), new PixmapLoader("filter_lp"));
    m_filter1TypeModel.addItem(tr("HiPass"), new PixmapLoader("filter_hp"));
    m_filter1TypeModel.addItem(tr("BandPass csg"),
                               new PixmapLoader("filter_bp"));
    m_filter1TypeModel.addItem(tr("BandPass czpg"),
                               new PixmapLoader("filter_bp"));
    m_filter1TypeModel.addItem(tr("Notch"), new PixmapLoader("filter_notch"));
    m_filter1TypeModel.addItem(tr("Allpass"), new PixmapLoader("filter_ap"));
    m_filter1TypeModel.addItem(tr("Moog"), new PixmapLoader("filter_lp"));
    m_filter1TypeModel.addItem(tr("2x LowPass"),
                               new PixmapLoader("filter_2lp"));
    m_filter1TypeModel.addItem(tr("RC12 LowPass"),
                               new PixmapLoader("filter_lp"));
    m_filter1TypeModel.addItem(tr("RC12 BandPass"),
                               new PixmapLoader("filter_bp"));
    m_filter1TypeModel.addItem(tr("RC12 HighPass"),
                               new PixmapLoader("filter_hp"));
    m_filter1TypeModel.addItem(tr("RC24 LowPass"),
                               new PixmapLoader("filter_lp"));
    m_filter1TypeModel.addItem(tr("RC24 BandPass"),
                               new PixmapLoader("filter_bp"));
    m_filter1TypeModel.addItem(tr("RC24 HighPass"),
                               new PixmapLoader("filter_hp"));
    m_filter1TypeModel.addItem(tr("Vocal Formant"),
                               new PixmapLoader("filter_hp"));
    m_filter1TypeModel.addItem(tr("2x Moog"), new PixmapLoader("filter_2lp"));
    m_filter1TypeModel.addItem(tr("SV LowPass"),
                               new PixmapLoader("filter_lp"));
    m_filter1TypeModel.addItem(tr("SV BandPass"),
                               new PixmapLoader("filter_bp"));
    m_filter1TypeModel.addItem(tr("SV HighPass"),
                               new PixmapLoader("filter_hp"));
    m_filter1TypeModel.addItem(tr("SV Notch"),
                               new PixmapLoader("filter_notch"));
    m_filter1TypeModel.addItem(tr("Fast Formant"),
                               new PixmapLoader("filter_hp"));
    m_filter1TypeModel.addItem(tr("Tripole"), new PixmapLoader("filter_lp"));
    m_filter1TypeModel.addItem(tr("Brown"), new PixmapLoader("filter_lp"));
    m_filter1TypeModel.addItem(tr("Pink"), new PixmapLoader("filter_lp"));
    m_filter1TypeModel.addItem(tr("Peak"), new PixmapLoader("filter_bp"));

    m_filter2TypeModel.addItem(tr("LowPass"), new PixmapLoader("filter_lp"));
    m_filter2TypeModel.addItem(tr("HiPass"), new PixmapLoader("filter_hp"));
    m_filter2TypeModel.addItem(tr("BandPass csg"),
                               new PixmapLoader("filter_bp"));
    m_filter2TypeModel.addItem(tr("BandPass czpg"),
                               new PixmapLoader("filter_bp"));
    m_filter2TypeModel.addItem(tr("Notch"), new PixmapLoader("filter_notch"));
    m_filter2TypeModel.addItem(tr("Allpass"), new PixmapLoader("filter_ap"));
    m_filter2TypeModel.addItem(tr("Moog"), new PixmapLoader("filter_lp"));
    m_filter2TypeModel.addItem(tr("2x LowPass"),
                               new PixmapLoader("filter_2lp"));
    m_filter2TypeModel.addItem(tr("RC12 LowPass"),
                               new PixmapLoader("filter_lp"));
    m_filter2TypeModel.addItem(tr("RC12 BandPass"),
                               new PixmapLoader("filter_bp"));
    m_filter2TypeModel.addItem(tr("RC12 HighPass"),
                               new PixmapLoader("filter_hp"));
    m_filter2TypeModel.addItem(tr("RC24 LowPass"),
                               new PixmapLoader("filter_lp"));
    m_filter2TypeModel.addItem(tr("RC24 BandPass"),
                               new PixmapLoader("filter_bp"));
    m_filter2TypeModel.addItem(tr("RC24 HighPass"),
                               new PixmapLoader("filter_hp"));
    m_filter2TypeModel.addItem(tr("Vocal Formant"),
                               new PixmapLoader("filter_hp"));
    m_filter2TypeModel.addItem(tr("2x Moog"), new PixmapLoader("filter_2lp"));
    m_filter2TypeModel.addItem(tr("SV LowPass"),
                               new PixmapLoader("filter_lp"));
    m_filter2TypeModel.addItem(tr("SV BandPass"),
                               new PixmapLoader("filter_bp"));
    m_filter2TypeModel.addItem(tr("SV HighPass"),
                               new PixmapLoader("filter_hp"));
    m_filter2TypeModel.addItem(tr("SV Notch"),
                               new PixmapLoader("filter_notch"));
    m_filter2TypeModel.addItem(tr("Fast Formant"),
                               new PixmapLoader("filter_hp"));
    m_filter2TypeModel.addItem(tr("Tripole"), new PixmapLoader("filter_lp"));
    m_filter2TypeModel.addItem(tr("Brown"), new PixmapLoader("filter_lp"));
    m_filter2TypeModel.addItem(tr("Pink"), new PixmapLoader("filter_lp"));
    m_filter2TypeModel.addItem(tr("Peak"), new PixmapLoader("filter_bp"));
}

InstrumentSoundShaping::~InstrumentSoundShaping()
{
    // qInfo("InstrumentSoundShaping::~InstrumentSoundShaping START");
    for(int i = 0; i < NumTargets; ++i)
        DELETE_HELPER(m_envLfoParameters[i])
    m_instrumentTrack = nullptr;
    // qInfo("InstrumentSoundShaping::~InstrumentSoundShaping END");
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
    m_envLfoParameters[Volume]->fillLevel(&level, frame, envReleaseBegin, 1,
                                          n->legato(), n->marcato(),
                                          n->staccato());

    return level;
}

void InstrumentSoundShaping::processAudioBuffer(sampleFrame*    buffer,
                                                const fpp_t     frames,
                                                NotePlayHandle* nph)
{
    f_cnt_t envTotalFrames = nph->totalFramesPlayed();

    f_cnt_t envReleaseBegin = envTotalFrames - nph->releaseFramesDone()
                              + nph->framesBeforeRelease();
    if(!nph->isReleased()
       || (nph->instrumentTrack()->isSustainPedalPressed()
           && !nph->isReleaseStarted()))
    {
        envReleaseBegin += frames;
    }

    if(nph->m_filter1 == nullptr && m_filter1EnabledModel.value())
    {
        nph->m_filter1
                = new BasicFilters<>(Engine::mixer()->processingSampleRate());
    }

    if(nph->m_filter2 == nullptr && m_filter1EnabledModel.value())
    {
        nph->m_filter2
                = new BasicFilters<>(Engine::mixer()->processingSampleRate());
    }

    processAudioBuffer(buffer, frames, nph->m_filter1, nph->m_filter2,
                       envTotalFrames, envReleaseBegin, nph->legato(),
                       nph->marcato(), nph->staccato());
}

void InstrumentSoundShaping::processAudioBuffer(sampleFrame*    buffer,
                                                const fpp_t     frames,
                                                BasicFilters<>* filter1,
                                                BasicFilters<>* filter2,
                                                f_cnt_t envTotalFrames,
                                                f_cnt_t envReleaseBegin,
                                                bool    _legato,
                                                bool    _marcato,
                                                bool    _staccato)
{
    // because of optimizations, there's special code for several cases:
    // 	- cut- and res-lfo/envelope active
    // 	- cut-lfo/envelope active
    // 	- res-lfo/envelope active
    //	- no lfo/envelope active but filter is used

    // only use filter, if it is really needed

    if(filter1 != nullptr && m_filter1EnabledModel.value())
    {
        real_t* cutBuffer = MM_ALLOC(real_t, frames);
        real_t* resBuffer = MM_ALLOC(real_t, frames);

        int old_filter_cut = 0;
        int old_filter_res = 0;

        filter1->setTypeAndPasses(m_filter1TypeModel.value(),
                                  m_filter1PassesModel.value() + 1);
        filter1->setFeedbackAmount(m_filter1FeedbackAmountModel.value());
        filter1->setFeedbackDelay(m_filter1FeedbackDelayModel.value());

        if(m_envLfoParameters[Cut]->isUsed())
        {
            m_envLfoParameters[Cut]->fillLevel(cutBuffer, envTotalFrames,
                                               envReleaseBegin, frames,
                                               _legato, _marcato, _staccato);
        }

        if(m_envLfoParameters[Resonance]->isUsed())
        {
            m_envLfoParameters[Resonance]->fillLevel(
                    resBuffer, envTotalFrames, envReleaseBegin, frames,
                    _legato, _marcato, _staccato);
        }

        const real_t fcv = m_filter1CutModel.value();
        const real_t frv = m_filter1ResModel.value();
        const real_t fgv = m_filter1GainModel.value();

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
                    filter1->calcFilterCoeffs(new_cut_val, new_res_val, fgv);
                    old_filter_cut = static_cast<int>(new_cut_val);
                    old_filter_res
                            = static_cast<int>(new_res_val * RES_PRECISION);
                }
                /*
                buffer[frame][0]
                        = bound(-1., filter1->update(buffer[frame][0],
                0), 1.); buffer[frame][1] = bound(-1.,
                filter1->update(buffer[frame][1], 1), 1.);
                */
                filter1->update(buffer[frame]);
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
                    filter1->calcFilterCoeffs(new_cut_val, frv, fgv);
                    old_filter_cut = static_cast<int>(new_cut_val);
                }
                /*
                buffer[frame][0]
                        = bound(-1., filter1->update(buffer[frame][0],
                0), 1.); buffer[frame][1] = bound(-1.,
                filter1->update(buffer[frame][1], 1), 1.);
                */
                filter1->update(buffer[frame]);
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
                    filter1->calcFilterCoeffs(fcv, new_res_val, fgv);
                    old_filter_res
                            = static_cast<int>(new_res_val * RES_PRECISION);
                }
                /*
                buffer[frame][0]
                        = bound(-1., filter1->update(buffer[frame][0],
                0), 1.); buffer[frame][1] = bound(-1.,
                filter1->update(buffer[frame][1], 1), 1.);
                */
                filter1->update(buffer[frame]);
            }
        }
        else
        {
            filter1->calcFilterCoeffs(fcv, frv, fgv);

            for(fpp_t frame = 0; frame < frames; ++frame)
            {
                /*
                buffer[frame][0]
                        = bound(-1., filter1->update(buffer[frame][0],
                0), 1.); buffer[frame][1] = bound(-1.,
                filter1->update(buffer[frame][1], 1), 1.);
                */
                filter1->update(buffer[frame]);
            }
        }

        MM_FREE(resBuffer);
        MM_FREE(cutBuffer);

        real_t resp = m_filter1ResponseModel.value();
        if(resp != 0.)
        {
            if(resp > 0.)
                resp = 1. / (1. + 3. * resp);
            else
                resp = 1. - 3. * resp;
            for(fpp_t frame = 0; frame < frames; ++frame)
            {
                buffer[frame][0] = sign(buffer[frame][0])
                                   * fastpow(abs(buffer[frame][0]), resp);
                buffer[frame][1] = sign(buffer[frame][1])
                                   * fastpow(abs(buffer[frame][1]), resp);
            }
        }

        if(fgv != 0.)
        {
            const real_t amp = pow(10, fgv * 0.45);
            for(fpp_t frame = 0; frame < frames; ++frame)
            {
                buffer[frame][0] = bound(-1., amp * buffer[frame][0], 1.);
                buffer[frame][1] = bound(-1., amp * buffer[frame][1], 1.);
            }
        }
    }

    if(filter2 != nullptr && m_filter2EnabledModel.value())
    {
        filter2->setTypeAndPasses(m_filter2TypeModel.value(),
                                  m_filter2PassesModel.value() + 1);
        filter2->setFeedbackAmount(m_filter2FeedbackAmountModel.value());
        filter2->setFeedbackDelay(m_filter2FeedbackDelayModel.value());

        const real_t fcv2 = m_filter2CutModel.value();
        const real_t frv2 = m_filter2ResModel.value();
        const real_t fgv2 = m_filter2GainModel.value();

        filter2->calcFilterCoeffs(fcv2, frv2, fgv2);

        for(fpp_t f = 0; f < frames; ++f)
            filter2->update(buffer[f]);

        real_t resp2 = m_filter2ResponseModel.value();
        if(resp2 != 0.)
        {
            if(resp2 > 0.)
                resp2 = 1. / (1. + 3. * resp2);
            else
                resp2 = 1. - 3. * resp2;
            for(fpp_t f = 0; f < frames; ++f)
            {
                buffer[f][0] = sign(buffer[f][0])
                               * fastpow(abs(buffer[f][0]), resp2);
                buffer[f][1] = sign(buffer[f][1])
                               * fastpow(abs(buffer[f][1]), resp2);
            }
        }

        if(fgv2 != 0.)
        {
            const real_t amp = pow(10, fgv2 * 0.45);
            for(fpp_t f = 0; f < frames; ++f)
            {
                buffer[f][0] = bound(-1., amp * buffer[f][0], 1.);
                buffer[f][1] = bound(-1., amp * buffer[f][1], 1.);
            }
        }
    }

    if(m_envLfoParameters[Volume]->isUsed())
    {
        real_t volBuffer[frames];
        m_envLfoParameters[Volume]->fillLevel(volBuffer, envTotalFrames,
                                              envReleaseBegin, frames,
                                              _legato, _marcato, _staccato);

        for(fpp_t frame = 0; frame < frames; ++frame)
        {
            real_t vol_level = volBuffer[frame];
            vol_level        = vol_level * vol_level;  // ????
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
    QDomElement ef1 = _doc.createElement("filter1");
    m_filter1TypeModel.saveSettings(_doc, ef1, "type");
    m_filter1CutModel.saveSettings(_doc, ef1, "cutoff");
    m_filter1ResModel.saveSettings(_doc, ef1, "resonance");
    m_filter1EnabledModel.saveSettings(_doc, ef1, "enabled");
    m_filter1PassesModel.saveSettings(_doc, ef1, "passes");
    m_filter1GainModel.saveSettings(_doc, ef1, "gain");
    m_filter1ResponseModel.saveSettings(_doc, ef1, "response");
    m_filter1FeedbackAmountModel.saveSettings(_doc, ef1, "feedback_amount");
    m_filter1FeedbackDelayModel.saveSettings(_doc, ef1, "feedback_delay");
    _this.appendChild(ef1);

    QDomElement ef2 = _doc.createElement("filter2");
    m_filter2TypeModel.saveSettings(_doc, ef2, "type");
    m_filter2CutModel.saveSettings(_doc, ef2, "cutoff");
    m_filter2ResModel.saveSettings(_doc, ef2, "resonance");
    m_filter2EnabledModel.saveSettings(_doc, ef2, "enabled");
    m_filter2PassesModel.saveSettings(_doc, ef2, "passes");
    m_filter2GainModel.saveSettings(_doc, ef2, "gain");
    m_filter2ResponseModel.saveSettings(_doc, ef2, "response");
    m_filter2FeedbackAmountModel.saveSettings(_doc, ef2, "feedback_amount");
    m_filter2FeedbackDelayModel.saveSettings(_doc, ef2, "feedback_delay");
    _this.appendChild(ef2);

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
    // qInfo("InstrumentSoundShaping::loadSettings START");
    // QDomNodeList 	elementsByTagName(const QString & tagname) const

    QDomNode nf1 = _this.namedItem("filter1");
    if(!nf1.isNull() && nf1.isElement())
    {
        QDomElement ef1 = nf1.toElement();
        if(!_this.hasAttribute("ftype")
           && _this.elementsByTagName("ftype").isEmpty())
            m_filter1TypeModel.loadSettings(ef1, "type");
        if(!_this.hasAttribute("fcut")
           && _this.elementsByTagName("fcut").isEmpty())
            m_filter1CutModel.loadSettings(ef1, "cutoff");
        if(!_this.hasAttribute("fres")
           && _this.elementsByTagName("fres").isEmpty())
            m_filter1ResModel.loadSettings(ef1, "resonance");
        if(!_this.hasAttribute("fwet")
           && _this.elementsByTagName("fwet").isEmpty())
            m_filter1EnabledModel.loadSettings(ef1, "enabled");
        m_filter1PassesModel.loadSettings(ef1, "passes");
        m_filter1GainModel.loadSettings(ef1, "gain");
        m_filter1ResponseModel.loadSettings(ef1, "response");
        m_filter1FeedbackAmountModel.loadSettings(ef1, "feedback_amount");
        m_filter1FeedbackDelayModel.loadSettings(ef1, "feedback_delay");
    }

    // for compatibility
    if(_this.hasAttribute("ftype")
       || !_this.elementsByTagName("ftype").isEmpty())
        m_filter1TypeModel.loadSettings(_this, "ftype");
    if(_this.hasAttribute("fcut")
       || !_this.elementsByTagName("fcut").isEmpty())
    {
        m_filter1CutModel.loadSettings(_this, "fcut");
        qInfo("ISS: filter1 fcut.uuid=%s",
              qPrintable(m_filter1CutModel.uuid()));
        qInfo("     Model::find %p %p", Model::find(m_filter1CutModel.uuid()),
              Model::find("972d721b-8d74-491e-b275-c1445b3e82fd"));
        qInfo("     id=%d", m_filter1CutModel.id());
        qInfo("     jo=%p", Engine::projectJournal()->journallingObject(
                                    m_filter1CutModel.id()));
    }
    if(_this.hasAttribute("fres")
       || !_this.elementsByTagName("fres").isEmpty())
        m_filter1ResModel.loadSettings(_this, "fres");
    if(_this.hasAttribute("fwet")
       || !_this.elementsByTagName("fwet").isEmpty())
        m_filter1EnabledModel.loadSettings(_this, "fwet");  // on/off

    QDomNode nf2 = _this.namedItem("filter2");
    if(!nf2.isNull() && nf2.isElement())
    {
        QDomElement ef2 = nf2.toElement();
        m_filter2TypeModel.loadSettings(ef2, "type");
        m_filter2CutModel.loadSettings(ef2, "cutoff");
        m_filter2ResModel.loadSettings(ef2, "resonance");
        m_filter2EnabledModel.loadSettings(ef2, "enabled");
        m_filter2PassesModel.loadSettings(ef2, "passes");
        m_filter2GainModel.loadSettings(ef2, "gain");
        m_filter2ResponseModel.loadSettings(ef2, "response");
        m_filter2FeedbackAmountModel.loadSettings(ef2, "feedback_amount");
        m_filter2FeedbackDelayModel.loadSettings(ef2, "feedback_delay");
    }

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
                    // qInfo("InstrumentSoundShaping: node %s START",
                    //      qPrintable(node.nodeName()));
                    m_envLfoParameters[i]->restoreState(node.toElement());
                    // qInfo("InstrumentSoundShaping: node %s END",
                    //      qPrintable(node.nodeName()));
                }
            }
        }
        node = node.nextSibling();
    }

    // qInfo("InstrumentSoundShaping::loadSettings END");
}
