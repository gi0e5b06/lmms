/*
 * EqEffect.cpp -
 *
 * Copyright (c) 2014 David French <dave/dot/french3/at/googlemail/dot/com>
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

#include "EqEffect.h"

#include "Engine.h"
#include "EqFader.h"
#include "embed.h"
#include "interpolation.h"
#include "lmms_math.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT eq_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "Equalizer",
               QT_TRANSLATE_NOOP("pluginBrowser", "A native eq plugin"),
               "Dave French "
               "<contact/dot/dave/dot/french3/at/googlemail/dot/com>",
               0x0100,
               Plugin::Effect,
               new PluginPixmapLoader("logo"),
               NULL,
               NULL};
}

EqEffect::EqEffect(Model*                                            parent,
                   const Plugin::Descriptor::SubPluginFeatures::Key* key) :
      Effect(&eq_plugin_descriptor, parent, key),
      m_eqControls(this), m_inGain(1.0), m_outGain(1.0)
{
}

EqEffect::~EqEffect()
{
}

bool EqEffect::processAudioBuffer(sampleFrame* buf, const fpp_t frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(buf, frames, smoothBegin, smoothEnd))
        return false;

    // int passes = m_eqControls.m_passesModel.value() + 1;

    // setup sample exact controls
    real_t hpRes        = m_eqControls.m_hpResModel.value();
    real_t lowShelfRes  = m_eqControls.m_lowShelfResModel.value();
    real_t para1Bw      = m_eqControls.m_para1BwModel.value();
    real_t para2Bw      = m_eqControls.m_para2BwModel.value();
    real_t para3Bw      = m_eqControls.m_para3BwModel.value();
    real_t para4Bw      = m_eqControls.m_para4BwModel.value();
    real_t highShelfRes = m_eqControls.m_highShelfResModel.value();
    real_t lpRes        = m_eqControls.m_lpResModel.value();

    real_t hpFreq        = m_eqControls.m_hpFeqModel.value();
    real_t lowShelfFreq  = m_eqControls.m_lowShelfFreqModel.value();
    real_t para1Freq     = m_eqControls.m_para1FreqModel.value();
    real_t para2Freq     = m_eqControls.m_para2FreqModel.value();
    real_t para3Freq     = m_eqControls.m_para3FreqModel.value();
    real_t para4Freq     = m_eqControls.m_para4FreqModel.value();
    real_t highShelfFreq = m_eqControls.m_highShelfFreqModel.value();
    real_t lpFreq        = m_eqControls.m_lpFreqModel.value();

    ValueBuffer* hpResBuffer = m_eqControls.m_hpResModel.valueBuffer();
    ValueBuffer* lowShelfResBuffer
            = m_eqControls.m_lowShelfResModel.valueBuffer();
    ValueBuffer* para1BwBuffer = m_eqControls.m_para1BwModel.valueBuffer();
    ValueBuffer* para2BwBuffer = m_eqControls.m_para2BwModel.valueBuffer();
    ValueBuffer* para3BwBuffer = m_eqControls.m_para3BwModel.valueBuffer();
    ValueBuffer* para4BwBuffer = m_eqControls.m_para4BwModel.valueBuffer();
    ValueBuffer* highShelfResBuffer
            = m_eqControls.m_highShelfResModel.valueBuffer();
    ValueBuffer* lpResBuffer = m_eqControls.m_lpResModel.valueBuffer();

    ValueBuffer* hpFreqBuffer = m_eqControls.m_hpFeqModel.valueBuffer();
    ValueBuffer* lowShelfFreqBuffer
            = m_eqControls.m_lowShelfFreqModel.valueBuffer();
    ValueBuffer* para1FreqBuffer
            = m_eqControls.m_para1FreqModel.valueBuffer();
    ValueBuffer* para2FreqBuffer
            = m_eqControls.m_para2FreqModel.valueBuffer();
    ValueBuffer* para3FreqBuffer
            = m_eqControls.m_para3FreqModel.valueBuffer();
    ValueBuffer* para4FreqBuffer
            = m_eqControls.m_para4FreqModel.valueBuffer();
    ValueBuffer* highShelfFreqBuffer
            = m_eqControls.m_highShelfFreqModel.valueBuffer();
    ValueBuffer* lpFreqBuffer = m_eqControls.m_lpFreqModel.valueBuffer();

    const int hpResInc        = hpResBuffer ? 1 : 0;
    const int lowShelfResInc  = lowShelfResBuffer ? 1 : 0;
    const int para1BwInc      = para1BwBuffer ? 1 : 0;
    const int para2BwInc      = para2BwBuffer ? 1 : 0;
    const int para3BwInc      = para3BwBuffer ? 1 : 0;
    const int para4BwInc      = para4BwBuffer ? 1 : 0;
    const int highShelfResInc = highShelfResBuffer ? 1 : 0;
    const int lpResInc        = lpResBuffer ? 1 : 0;

    const int hpFreqInc        = hpFreqBuffer ? 1 : 0;
    const int lowShelfFreqInc  = lowShelfFreqBuffer ? 1 : 0;
    const int para1FreqInc     = para1FreqBuffer ? 1 : 0;
    const int para2FreqInc     = para2FreqBuffer ? 1 : 0;
    const int para3FreqInc     = para3FreqBuffer ? 1 : 0;
    const int para4FreqInc     = para4FreqBuffer ? 1 : 0;
    const int highShelfFreqInc = highShelfFreqBuffer ? 1 : 0;
    const int lpFreqInc        = lpFreqBuffer ? 1 : 0;

    real_t* hpResPtr = hpResBuffer ? hpResBuffer->values() : &hpRes;
    real_t* lowShelfResPtr
            = lowShelfResBuffer ? lowShelfResBuffer->values() : &lowShelfRes;
    real_t* para1BwPtr = para1BwBuffer ? para1BwBuffer->values() : &para1Bw;
    real_t* para2BwPtr = para2BwBuffer ? para2BwBuffer->values() : &para2Bw;
    real_t* para3BwPtr = para3BwBuffer ? para3BwBuffer->values() : &para3Bw;
    real_t* para4BwPtr = para4BwBuffer ? para4BwBuffer->values() : &para4Bw;
    real_t* highShelfResPtr = highShelfResBuffer
                                      ? highShelfResBuffer->values()
                                      : &highShelfRes;
    real_t* lpResPtr = lpResBuffer ? lpResBuffer->values() : &lpRes;

    real_t* hpFreqPtr       = hpFreqBuffer ? hpFreqBuffer->values() : &hpFreq;
    real_t* lowShelfFreqPtr = lowShelfFreqBuffer
                                      ? lowShelfFreqBuffer->values()
                                      : &lowShelfFreq;
    real_t* para1FreqPtr
            = para1FreqBuffer ? para1FreqBuffer->values() : &para1Freq;
    real_t* para2FreqPtr
            = para2FreqBuffer ? para2FreqBuffer->values() : &para2Freq;
    real_t* para3FreqPtr
            = para3FreqBuffer ? para3FreqBuffer->values() : &para3Freq;
    real_t* para4FreqPtr
            = para4FreqBuffer ? para4FreqBuffer->values() : &para4Freq;
    real_t* hightShelfFreqPtr = highShelfFreqBuffer
                                        ? highShelfFreqBuffer->values()
                                        : &highShelfFreq;
    real_t* lpFreqPtr = lpFreqBuffer ? lpFreqBuffer->values() : &lpFreq;

    bool hpActive        = m_eqControls.m_hpActiveModel.value();
    bool hp24Active      = m_eqControls.m_hp24Model.value();
    bool hp48Active      = m_eqControls.m_hp48Model.value();
    bool lowShelfActive  = m_eqControls.m_lowShelfActiveModel.value();
    bool para1Active     = m_eqControls.m_para1ActiveModel.value();
    bool para2Active     = m_eqControls.m_para2ActiveModel.value();
    bool para3Active     = m_eqControls.m_para3ActiveModel.value();
    bool para4Active     = m_eqControls.m_para4ActiveModel.value();
    bool highShelfActive = m_eqControls.m_highShelfActiveModel.value();
    bool lpActive        = m_eqControls.m_lpActiveModel.value();
    bool lp24Active      = m_eqControls.m_lp24Model.value();
    bool lp48Active      = m_eqControls.m_lp48Model.value();

    real_t lowShelfGain  = m_eqControls.m_lowShelfGainModel.value();
    real_t para1Gain     = m_eqControls.m_para1GainModel.value();
    real_t para2Gain     = m_eqControls.m_para2GainModel.value();
    real_t para3Gain     = m_eqControls.m_para3GainModel.value();
    real_t para4Gain     = m_eqControls.m_para4GainModel.value();
    real_t highShelfGain = m_eqControls.m_highShelfGainModel.value();

    if(m_eqControls.m_outGainModel.isValueChanged())
    {
        m_outGain = dbfsToAmp(m_eqControls.m_outGainModel.value());
    }

    if(m_eqControls.m_inGainModel.isValueChanged())
    {
        m_inGain = dbfsToAmp(m_eqControls.m_inGainModel.value());
    }

    // if(hpActive) m_hp12.setPasses(passes);
    /*
    EqHp12Filter m_hp24;
    EqHp12Filter m_hp480;
    EqHp12Filter m_hp481;

    EqLowShelfFilter m_lowShelf;

    EqPeakFilter m_para1;
    EqPeakFilter m_para2;
    EqPeakFilter m_para3;
    EqPeakFilter m_para4;

    EqHighShelfFilter m_highShelf;

    EqLp12Filter m_lp12;
    EqLp12Filter m_lp24;
    EqLp12Filter m_lp480;
    EqLp12Filter m_lp481;
    */

    m_eqControls.m_inProgress = true;

    const real_t outGain    = m_outGain;
    const int    sampleRate = Engine::mixer()->processingSampleRate();
    sampleFrame  m_inPeak   = {0, 0};

    if(!smoothEnd && m_eqControls.m_analyseInModel.value(true)
       && m_eqControls.m_inFftBands.getActive())  // outSum > 0 )
    {
        m_eqControls.m_inFftBands.analyze(buf, frames);
    }
    else
    {
        // m_eqControls.m_inFftBands.clear();
    }

    gain(buf, frames, m_inGain, &m_inPeak);
    m_eqControls.m_inPeakL = m_eqControls.m_inPeakL < m_inPeak[0]
                                     ? m_inPeak[0]
                                     : m_eqControls.m_inPeakL;
    m_eqControls.m_inPeakR = m_eqControls.m_inPeakR < m_inPeak[1]
                                     ? m_inPeak[1]
                                     : m_eqControls.m_inPeakR;

    for(fpp_t f = 0; f < frames; f++)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        sample_t dryS[2] = {buf[f][0], buf[f][1]};
        // dryS[0] = buf[f][0];
        // dryS[1] = buf[f][1];

        if(hpActive)
        {
            m_hp12.setParameters(sampleRate, *hpFreqPtr, *hpResPtr, 1);
            // for(int p = 0; p < passes; p++)
            {
                // buf[f][0] = bound(-1., m_hp12.update(buf[f][0], 0), 1.);
                // buf[f][1] = bound(-1., m_hp12.update(buf[f][1], 1), 1.);
                m_hp12.update(buf[f]);
            }

            if(hp24Active || hp48Active)
            {
                m_hp24.setParameters(sampleRate, *hpFreqPtr, *hpResPtr, 1);
                // for(int p = 0; p < passes; p++)
                {
                    // buf[f][0] = bound(-1., m_hp24.update(buf[f][0],
                    // 0), 1.); buf[f][1] = bound(-1.,
                    // m_hp24.update(buf[f][1], 1), 1.);
                    m_hp24.update(buf[f]);
                }
            }

            if(hp48Active)
            {
                m_hp480.setParameters(sampleRate, *hpFreqPtr, *hpResPtr, 1);
                // for(int p = 0; p < passes; p++)
                {
                    /*
                      buf[f][0] = bound(-1., m_hp480.update(buf[f][0],
                      0), 1.);
                      buf[f][1] = bound(-1.,
                      m_hp480.update(buf[f][1], 1), 1.);
                    */
                    m_hp480.update(buf[f]);
                }
                m_hp481.setParameters(sampleRate, *hpFreqPtr, *hpResPtr, 1);
                // for(int p = 0; p < passes; p++)
                {
                    /*
                      buf[f][0] = bound(-1., m_hp481.update(buf[f][0],
                      0), 1.); buf[f][1] = bound(-1.,
                      m_hp481.update(buf[f][1], 1), 1.);
                    */
                    m_hp481.update(buf[f]);
                }
            }
        }

        if(lowShelfActive)
        {
            m_lowShelf.setParameters(sampleRate, *lowShelfFreqPtr,
                                     *lowShelfResPtr, lowShelfGain);
            // for(int p = 0; p < passes; p++)
            {
                /*
                  buf[f][0] = bound(-1., m_lowShelf.update(buf[f][0], 0), 1.);
                  buf[f][1] = bound(-1., m_lowShelf.update(buf[f][1], 1), 1.);
                */
                m_lowShelf.update(buf[f]);
            }
        }

        if(para1Active)
        {
            m_para1.setParameters(sampleRate, *para1FreqPtr, *para1BwPtr,
                                  para1Gain);
            // for(int p = 0; p < passes; p++)
            {
                // buf[f][0] = bound(-1., m_para1.update(buf[f][0], 0), 1.);
                // buf[f][1] = bound(-1., m_para1.update(buf[f][1], 1), 1.);
                m_para1.update(buf[f]);
            }
        }

        if(para2Active)
        {
            m_para2.setParameters(sampleRate, *para2FreqPtr, *para2BwPtr,
                                  para2Gain);
            // for(int p = 0; p < passes; p++)
            {
                // buf[f][0] = bound(-1., m_para2.update(buf[f][0], 0), 1.);
                // buf[f][1] = bound(-1., m_para2.update(buf[f][1], 1), 1.);
                m_para2.update(buf[f]);
            }
        }

        if(para3Active)
        {
            m_para3.setParameters(sampleRate, *para3FreqPtr, *para3BwPtr,
                                  para3Gain);
            // for(int p = 0; p < passes; p++)
            {
                // buf[f][0] = bound(-1., m_para3.update(buf[f][0], 0), 1.);
                // buf[f][1] = bound(-1., m_para3.update(buf[f][1], 1), 1.);
                m_para3.update(buf[f]);
            }
        }

        if(para4Active)
        {
            m_para4.setParameters(sampleRate, *para4FreqPtr, *para4BwPtr,
                                  para4Gain);
            // for(int p = 0; p < passes; p++)
            {
                // buf[f][0] = bound(-1., m_para4.update(buf[f][0], 0), 1.);
                // buf[f][1] = bound(-1., m_para4.update(buf[f][1], 1), 1.);
                m_para4.update(buf[f]);
            }
        }

        if(highShelfActive)
        {
            m_highShelf.setParameters(sampleRate, *hightShelfFreqPtr,
                                      *highShelfResPtr, highShelfGain);
            // for(int p = 0; p < passes; p++)
            {
                // buf[f][0] = bound(-1., m_highShelf.update(buf[f][0],
                // 0), 1.);
                // buf[f][1] = bound(-1., m_highShelf.update(buf[f][1],
                // 1), 1.);
                m_highShelf.update(buf[f]);
            }
        }

        if(lpActive)
        {
            m_lp12.setParameters(sampleRate, *lpFreqPtr, *lpResPtr, 1);
            // for(int p = 0; p < passes; p++)
            {
                // buf[f][0] = bound(-1., m_lp12.update(buf[f][0], 0), 1.);
                // buf[f][1] = bound(-1., m_lp12.update(buf[f][1], 1), 1.);
                m_lp12.update(buf[f]);
            }

            if(lp24Active || lp48Active)
            {
                m_lp24.setParameters(sampleRate, *lpFreqPtr, *lpResPtr, 1);
                // for(int p = 0; p < passes; p++)
                {
                    // buf[f][0] = bound(-1., m_lp24.update(buf[f][0],
                    // 0), 1.);
                    // buf[f][1] = bound(-1., m_lp24.update(buf[f][1],
                    // 1), 1.);
                    m_lp24.update(buf[f]);
                }
            }

            if(lp48Active)
            {
                m_lp480.setParameters(sampleRate, *lpFreqPtr, *lpResPtr, 1);
                // for(int p = 0; p < passes; p++)
                {
                    // buf[f][0] = bound(-1., m_lp480.update(buf[f][0],
                    // 0), 1.);
                    // buf[f][1] = bound(-1., m_lp480.update(buf[f][1],
                    // 1), 1.);
                    m_lp480.update(buf[f]);
                }
                m_lp481.setParameters(sampleRate, *lpFreqPtr, *lpResPtr, 1);
                // for(int p = 0; p < passes; p++)
                {
                    // buf[f][0] = bound(-1., m_lp481.update(buf[f][0],
                    // 0), 1.);
                    // buf[f][1] = bound(-1., m_lp481.update(buf[f][1],
                    // 1), 1.);
                    m_lp481.update(buf[f]);
                }
            }
        }

        // apply wet / dry levels
        buf[f][0] = (d0 * dryS[0]) + (w0 * bound(-1., buf[f][0], 1.));
        buf[f][1] = (d1 * dryS[1]) + (w1 * bound(-1., buf[f][1], 1.));

        // increment pointers if needed
        hpResPtr += hpResInc;
        lowShelfResPtr += lowShelfResInc;
        para1BwPtr += para1BwInc;
        para2BwPtr += para2BwInc;
        para3BwPtr += para3BwInc;
        para4BwPtr += para4BwInc;
        highShelfResPtr += highShelfResInc;
        lpResPtr += lpResInc;

        hpFreqPtr += hpFreqInc;
        lowShelfFreqPtr += lowShelfFreqInc;
        para1FreqPtr += para1FreqInc;
        para2FreqPtr += para2FreqInc;
        para3FreqPtr += para3FreqInc;
        para4FreqPtr += para4FreqInc;
        hightShelfFreqPtr += highShelfFreqInc;
        lpFreqPtr += lpFreqInc;
    }

    sampleFrame outPeak = {0, 0};
    gain(buf, frames, outGain, &outPeak);
    m_eqControls.m_outPeakL = m_eqControls.m_outPeakL < outPeak[0]
                                      ? outPeak[0]
                                      : m_eqControls.m_outPeakL;
    m_eqControls.m_outPeakR = m_eqControls.m_outPeakR < outPeak[1]
                                      ? outPeak[1]
                                      : m_eqControls.m_outPeakR;

    if(!smoothEnd && m_eqControls.m_analyseOutModel.value(true)
       && m_eqControls.m_outFftBands.getActive())  // outSum > 0 )
    {
        m_eqControls.m_outFftBands.analyze(buf, frames);
        setBandPeaks(&m_eqControls.m_outFftBands, int(sampleRate));
    }
    else
    {
        // m_eqControls.m_outFftBands.clear();
    }

    m_eqControls.m_inProgress = false;

    return true;
}

real_t EqEffect::peakBand(frequency_t   minF,
                          frequency_t   maxF,
                          EqAnalyser*   fft,
                          sample_rate_t sr)
{
    real_t peak = -60;
    FLOAT* b    = fft->m_bands;
    real_t h    = 0;
    for(int x = 0; x < MAX_BANDS; x++, b++)
    {
        if(bandToFreq(x, sr) >= minF && bandToFreq(x, sr) <= maxF)
        {
            h    = 20 * (log10(*b / fft->getEnergy()));
            peak = h > peak ? h : peak;
        }
    }

    return (peak + 60) / 100;
}

void EqEffect::setBandPeaks(EqAnalyser* fft, sample_rate_t samplerate)
{
    m_eqControls.m_lowShelfPeakR = m_eqControls.m_lowShelfPeakL = peakBand(
            m_eqControls.m_lowShelfFreqModel.value()
                    * (1 - m_eqControls.m_lowShelfResModel.value() * 0.5),
            m_eqControls.m_lowShelfFreqModel.value(), fft, samplerate);

    m_eqControls.m_para1PeakL = m_eqControls.m_para1PeakR = peakBand(
            m_eqControls.m_para1FreqModel.value()
                    * (1 - m_eqControls.m_para1BwModel.value() * 0.5),
            m_eqControls.m_para1FreqModel.value()
                    * (1 + m_eqControls.m_para1BwModel.value() * 0.5),
            fft, samplerate);

    m_eqControls.m_para2PeakL = m_eqControls.m_para2PeakR = peakBand(
            m_eqControls.m_para2FreqModel.value()
                    * (1 - m_eqControls.m_para2BwModel.value() * 0.5),
            m_eqControls.m_para2FreqModel.value()
                    * (1 + m_eqControls.m_para2BwModel.value() * 0.5),
            fft, samplerate);

    m_eqControls.m_para3PeakL = m_eqControls.m_para3PeakR = peakBand(
            m_eqControls.m_para3FreqModel.value()
                    * (1 - m_eqControls.m_para3BwModel.value() * 0.5),
            m_eqControls.m_para3FreqModel.value()
                    * (1 + m_eqControls.m_para3BwModel.value() * 0.5),
            fft, samplerate);

    m_eqControls.m_para4PeakL = m_eqControls.m_para4PeakR = peakBand(
            m_eqControls.m_para4FreqModel.value()
                    * (1 - m_eqControls.m_para4BwModel.value() * 0.5),
            m_eqControls.m_para4FreqModel.value()
                    * (1 + m_eqControls.m_para4BwModel.value() * 0.5),
            fft, samplerate);

    m_eqControls.m_highShelfPeakL = m_eqControls.m_highShelfPeakR = peakBand(
            m_eqControls.m_highShelfFreqModel.value(),
            m_eqControls.m_highShelfFreqModel.value()
                    * (1 + m_eqControls.m_highShelfResModel.value() * 0.5),
            fft, samplerate);
}

extern "C"
{

    // needed for getting plugin out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model* parent, void* data)
    {
        return new EqEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
