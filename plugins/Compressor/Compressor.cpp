/*
 * Compressor.cpp
 *
 * Copyright (c) 2020 Lost Robot <r94231@gmail.com>
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

#include "Compressor.h"

#include "embed.h"
#include "interpolation.h"
#include "lmms_math.h"
//#include "plugin_export.h"

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT compressor_plugin_descriptor = {
            STRINGIFY(PLUGIN_NAME),
            "Compressor",
            QT_TRANSLATE_NOOP("pluginBrowser", "A dynamic range compressor."),
            "Lost Robot <r94231@gmail.com>",
            0x0100,
            Plugin::Effect,
            new PluginPixmapLoader("logo"),
            NULL,
            NULL};
}

CompressorEffect::CompressorEffect(
        Model* parent, const Descriptor::SubPluginFeatures::Key* key) :
      Effect(&compressor_plugin_descriptor, parent, key),
      m_compressorControls(this)
{
    m_sampleRate = Engine::mixer()->processingSampleRate();

    m_yL[0] = m_yL[1] = COMP_NOISE_FLOOR;

    // These will be resized later
    m_rms[0] = new RmsHelper(1);
    m_rms[1] = new RmsHelper(1);

    // 200 ms
    m_crestTimeConst = exp(-1. / (0.2 * m_sampleRate));

    connect(&m_compressorControls.m_attackModel, SIGNAL(dataChanged()), this,
            SLOT(calcAttack()));
    connect(&m_compressorControls.m_releaseModel, SIGNAL(dataChanged()), this,
            SLOT(calcRelease()));
    connect(&m_compressorControls.m_holdModel, SIGNAL(dataChanged()), this,
            SLOT(calcHold()));
    connect(&m_compressorControls.m_ratioModel, SIGNAL(dataChanged()), this,
            SLOT(calcRatio()));
    connect(&m_compressorControls.m_rangeModel, SIGNAL(dataChanged()), this,
            SLOT(calcRange()));
    connect(&m_compressorControls.m_rmsModel, SIGNAL(dataChanged()), this,
            SLOT(resizeRMS()));
    connect(&m_compressorControls.m_lookaheadLengthModel,
            SIGNAL(dataChanged()), this, SLOT(calcLookaheadLength()));
    connect(&m_compressorControls.m_thresholdModel, SIGNAL(dataChanged()),
            this, SLOT(calcThreshold()));
    connect(&m_compressorControls.m_kneeModel, SIGNAL(dataChanged()), this,
            SLOT(calcKnee()));
    connect(&m_compressorControls.m_outGainModel, SIGNAL(dataChanged()), this,
            SLOT(calcOutGain()));
    connect(&m_compressorControls.m_inGainModel, SIGNAL(dataChanged()), this,
            SLOT(calcInGain()));
    connect(&m_compressorControls.m_tiltModel, SIGNAL(dataChanged()), this,
            SLOT(calcTiltCoeffs()));
    connect(&m_compressorControls.m_tiltFreqModel, SIGNAL(dataChanged()),
            this, SLOT(calcTiltCoeffs()));
    connect(&m_compressorControls.m_limiterModel, SIGNAL(dataChanged()), this,
            SLOT(redrawKnee()));
    connect(&m_compressorControls.m_mixModel, SIGNAL(dataChanged()), this,
            SLOT(calcMix()));

    connect(&m_compressorControls.m_autoAttackModel, SIGNAL(dataChanged()),
            this, SLOT(calcAutoAttack()));
    connect(&m_compressorControls.m_autoReleaseModel, SIGNAL(dataChanged()),
            this, SLOT(calcAutoRelease()));

    connect(&m_compressorControls.m_thresholdModel, SIGNAL(dataChanged()),
            this, SLOT(calcAutoMakeup()));
    connect(&m_compressorControls.m_ratioModel, SIGNAL(dataChanged()), this,
            SLOT(calcAutoMakeup()));
    connect(&m_compressorControls.m_kneeModel, SIGNAL(dataChanged()), this,
            SLOT(calcAutoMakeup()));

    connect(Engine::mixer(), SIGNAL(sampleRateChanged()), this,
            SLOT(changeSampleRate()));
    emit changeSampleRate();
}

CompressorEffect::~CompressorEffect()
{
    delete m_rms[0];
    delete m_rms[1];
}

real_t CompressorEffect::msToCoeff(real_t ms)
{
    // Convert time in milliseconds to applicable lowpass coefficient
    return exp(m_coeffPrecalc / ms);
}

void CompressorEffect::calcAutoMakeup()
{
    // Formulas using the compressor's Threshold, Ratio, and Knee values to
    // estimate a good makeup gain value

    real_t tempGainResult;
    if(0 - m_thresholdVal < -m_kneeVal)
    {
        tempGainResult = 0;
    }
    else if(abs(1 - m_thresholdVal) < m_kneeVal)
    // If the input is within the knee's range
    {
        const real_t temp = m_thresholdVal + m_kneeVal;
        tempGainResult
                = ((m_compressorControls.m_limiterModel.value() ? 0
                                                                : m_ratioVal)
                   - 1)
                  * temp * temp / (4 * m_kneeVal);
    }
    else
    {
        tempGainResult
                = m_compressorControls.m_limiterModel.value()
                          ? m_thresholdVal
                          : (m_thresholdVal + m_thresholdVal * m_ratioVal);
    }

    m_autoMakeupVal = 1. / dbfsToAmp(tempGainResult);
}

void CompressorEffect::calcAttack()
{
    m_attCoeff = msToCoeff(m_compressorControls.m_attackModel.value());
}

void CompressorEffect::calcRelease()
{
    m_relCoeff = msToCoeff(m_compressorControls.m_releaseModel.value());
}

void CompressorEffect::calcAutoAttack()
{
    m_autoAttVal = m_compressorControls.m_autoAttackModel.value() * 0.01;
}

void CompressorEffect::calcAutoRelease()
{
    m_autoRelVal = m_compressorControls.m_autoReleaseModel.value() * 0.01;
}

void CompressorEffect::calcHold()
{
    m_holdLength
            = m_compressorControls.m_holdModel.value() * 0.001 * m_sampleRate;
    m_holdTimer[0] = 0;
    m_holdTimer[1] = 0;
}

void CompressorEffect::calcOutGain()
{
    // 0.999 is needed to keep the values from crossing the threshold all the
    // time (most commonly for limiters specifically), and is kept across all
    // modes for consistency.
    m_outGainVal
            = dbfsToAmp(m_compressorControls.m_outGainModel.value()) * 0.999;
}

void CompressorEffect::calcRatio()
{
    m_ratioVal   = 1. / m_compressorControls.m_ratioModel.value();
    m_redrawKnee = true;
}

void CompressorEffect::calcRange()
{
    // Range is inactive when turned all the way down
    m_rangeVal
            = (m_compressorControls.m_rangeModel.value()
               > m_compressorControls.m_rangeModel.minValue())
                      ? dbfsToAmp(m_compressorControls.m_rangeModel.value())
                      : 0.;
}

void CompressorEffect::resizeRMS()
{
    m_rms[0]->setSize(m_compressorControls.m_rmsModel.value() * m_sampleRate
                      / 44100.);
    m_rms[1]->setSize(m_compressorControls.m_rmsModel.value() * m_sampleRate
                      / 44100.);
}

void CompressorEffect::calcLookaheadLength()
{
    m_lookaheadLength
            = qMax<real_t>(m_compressorControls.m_lookaheadLengthModel.value()
                                   * 0.001 * m_sampleRate,
                           1.);

    m_lookaheadBuf[0].resize(m_lookaheadLength);
    m_lookaheadBuf[1].resize(m_lookaheadLength);

    m_preLookaheadLength = ceil(m_lookaheadDelayLength - m_lookaheadLength);
    m_preLookaheadBuf[0].resize(m_preLookaheadLength);
    m_preLookaheadBuf[1].resize(m_preLookaheadLength);
}

void CompressorEffect::calcThreshold()
{
    m_thresholdVal    = m_compressorControls.m_thresholdModel.value();
    m_thresholdAmpVal = dbfsToAmp(m_thresholdVal);
    m_redrawKnee      = true;
    m_redrawThreshold = true;
}

void CompressorEffect::calcKnee()
{
    m_kneeVal    = m_compressorControls.m_kneeModel.value() * 0.5;
    m_redrawKnee = true;
}

void CompressorEffect::calcInGain()
{
    m_inGainVal = dbfsToAmp(m_compressorControls.m_inGainModel.value());
}

void CompressorEffect::redrawKnee()
{
    m_redrawKnee = true;
}

void CompressorEffect::calcTiltCoeffs()
{
    m_tiltVal = m_compressorControls.m_tiltModel.value();

    const real_t amp = 6. / log(2.);

    const real_t gfactor = 5.;
    const real_t g1 = (m_tiltVal > 0) ? -gfactor * m_tiltVal : -m_tiltVal;
    const real_t g2 = (m_tiltVal > 0) ? m_tiltVal : gfactor * m_tiltVal;

    m_lgain = exp(g1 / amp) - 1.;
    m_hgain = exp(g2 / amp) - 1.;

    const real_t omega = R_2PI * m_compressorControls.m_tiltFreqModel.value();
    const real_t n     = 1. / (m_sampleRate * 3. + omega);
    m_a0               = 2. * omega * n;
    m_b1               = (m_sampleRate * 3 - omega) * n;
}

void CompressorEffect::calcMix()
{
    m_mixVal = m_compressorControls.m_mixModel.value() * 0.01;
}

bool CompressorEffect::processAudioBuffer(sampleFrame* buf,
                                          const fpp_t  frames)
{
    bool smoothBegin, smoothEnd;
    if(!shouldProcessAudioBuffer(buf, frames, smoothBegin, smoothEnd))
    // if(!isEnabled() || !isRunning())
    {
        // Clear lookahead buffers and other values when needed
        if(!m_cleanedBuffers)
        {
            m_yL[0] = m_yL[1] = COMP_NOISE_FLOOR;
            std::fill(std::begin(m_lookaheadBuf[0]),
                      std::end(m_lookaheadBuf[0]), 0);
            std::fill(std::begin(m_lookaheadBuf[1]),
                      std::end(m_lookaheadBuf[1]), 0);
            m_lookaheadBufLoc[0] = 0;
            m_lookaheadBufLoc[1] = 0;
            std::fill(std::begin(m_preLookaheadBuf[0]),
                      std::end(m_preLookaheadBuf[0]), 0);
            std::fill(std::begin(m_preLookaheadBuf[1]),
                      std::end(m_preLookaheadBuf[1]), 0);
            m_preLookaheadBufLoc[0] = 0;
            m_preLookaheadBufLoc[1] = 0;
            m_cleanedBuffers        = true;
        }
        return false;
    }
    else
    {
        m_cleanedBuffers = false;
    }

    // real_t       outSum = 0.0;
    // const real_t d      = 0.;  // TMP dryLevel();
    // const real_t w      = 1.;  // TMP wetLevel();

    real_t lOutPeak = 0.0;
    real_t rOutPeak = 0.0;
    real_t lInPeak  = 0.0;
    real_t rInPeak  = 0.0;

    const bool   midside    = m_compressorControls.m_midsideModel.value();
    const bool   peakmode   = m_compressorControls.m_peakmodeModel.value();
    const real_t inBalance  = m_compressorControls.m_inBalanceModel.value();
    const real_t outBalance = m_compressorControls.m_outBalanceModel.value();
    const bool   limiter    = m_compressorControls.m_limiterModel.value();
    const real_t blend      = m_compressorControls.m_blendModel.value();
    const real_t stereoBalance
            = m_compressorControls.m_stereoBalanceModel.value();
    const bool autoMakeup = m_compressorControls.m_autoMakeupModel.value();
    const int  stereoLink = m_compressorControls.m_stereoLinkModel.value();
    const bool audition   = m_compressorControls.m_auditionModel.value();
    const bool feedback   = m_compressorControls.m_feedbackModel.value();
    const bool lookahead  = m_compressorControls.m_lookaheadModel.value();

    for(fpp_t f = 0; f < frames; ++f)
    {
        real_t w0, d0, w1, d1;
        computeWetDryLevels(f, frames, smoothBegin, smoothEnd, w0, d0, w1,
                            d1);

        sample_t drySignal[2]
                = {buf[f][0] * m_inGainVal, buf[f][1] * m_inGainVal};
        sample_t s[2] = {drySignal[0], drySignal[1]};

        // Calculate tilt filters, to bias the sidechain to the low or high
        // frequencies
        if(m_tiltVal)
        {
            calcTiltFilter(s[0], s[0], 0);
            calcTiltFilter(s[1], s[1], 1);
        }

        s[0] *= inBalance > 0 ? 1 - inBalance : 1;
        s[1] *= inBalance < 0 ? 1 + inBalance : 1;

        real_t gainResult[2] = {0, 0};

        for(int i = 0; i < 2; i++)
        {
            real_t inputValue = feedback ? m_prevOut[i] : s[i];

            // Calculate the crest factor of the audio by diving the peak by
            // the RMS
            m_crestPeakVal[i]
                    = qMax(inputValue * inputValue,
                           m_crestTimeConst * m_crestPeakVal[i]
                                   + (1 - m_crestTimeConst)
                                             * (inputValue * inputValue));
            m_crestRmsVal[i]
                    = m_crestTimeConst * m_crestRmsVal[i]
                      + ((1 - m_crestTimeConst) * (inputValue * inputValue));
            m_crestFactorVal[i] = m_crestPeakVal[i] / m_crestRmsVal[i];

            // Grab the peak or RMS value
            inputValue = qMax(COMP_NOISE_FLOOR,
                              peakmode ? abs(inputValue)
                                       : m_rms[i]->update(inputValue));

            // The following code uses math magic to semi-efficiently
            // find the largest value in the lookahead buffer.
            // This can probably be improved.
            if(lookahead)
            {
                // Pre-lookahead delay, so the total delay always matches 20
                // ms
                ++m_preLookaheadBufLoc[i];
                if(m_preLookaheadBufLoc[i] >= m_preLookaheadLength)
                {
                    m_preLookaheadBufLoc[i] = 0;
                }
                const real_t tempInputValue = inputValue;
                inputValue = m_preLookaheadBuf[i][m_preLookaheadBufLoc[i]];
                m_preLookaheadBuf[i][m_preLookaheadBufLoc[i]]
                        = tempInputValue;

                // Increment ring buffer location
                ++m_lookaheadBufLoc[i];
                if(m_lookaheadBufLoc[i] >= m_lookaheadLength)
                {
                    m_lookaheadBufLoc[i] = 0;
                }

                m_lookaheadBuf[i][m_lookaheadBufLoc[i]] = inputValue;

                // If the new input value is larger than the stored maximum,
                // store that as the maximum
                if(inputValue >= m_maxLookaheadVal[i])
                {
                    m_maxLookaheadVal[i]   = inputValue;
                    m_maxLookaheadTimer[i] = m_lookaheadLength;
                }

                // Decrement timer.  When the timer reaches 0, that means the
                // stored maximum value has left the buffer and a new
                // maximum value must be found.
                if(--m_maxLookaheadTimer[i] <= 0)
                {
                    m_maxLookaheadTimer[i] = std::distance(
                            std::begin(m_lookaheadBuf[i]),
                            std::max_element(std::begin(m_lookaheadBuf[i]),
                                             std::begin(m_lookaheadBuf[i])
                                                     + m_lookaheadLength));
                    m_maxLookaheadVal[i]
                            = m_lookaheadBuf[i][m_maxLookaheadTimer[i]];
                    m_maxLookaheadTimer[i] = realmod(
                            m_maxLookaheadTimer[i] - m_lookaheadBufLoc[i],
                            m_lookaheadLength);
                }

                inputValue = m_maxLookaheadVal[i];
            }

            real_t t = inputValue;

            if(t > m_yL[i])  // Attack phase
            {
                // Calculate attack value depending on crest factor
                const real_t att
                        = m_autoAttVal ? msToCoeff(
                                  2.
                                  * m_compressorControls.m_attackModel.value()
                                  / ((m_crestFactorVal[i] - 1) * m_autoAttVal
                                     + 1))
                                       : m_attCoeff;

                m_yL[i]        = m_yL[i] * att + (1 - att) * t;
                m_holdTimer[i] = m_holdLength;  // Reset hold timer
            }
            else  // Release phase
            {
                // Calculate release value depending on crest factor
                const real_t rel
                        = m_autoRelVal ? msToCoeff(
                                  2.
                                  * m_compressorControls.m_releaseModel
                                            .value()
                                  / ((m_crestFactorVal[i] - 1) * m_autoRelVal
                                     + 1))
                                       : m_relCoeff;

                if(m_holdTimer[i])  // Don't change peak if hold is being
                                    // applied
                {
                    --m_holdTimer[i];
                }
                else
                {
                    m_yL[i] = m_yL[i] * rel + (1 - rel) * t;
                }
            }

            // Keep it above the noise floor
            m_yL[i] = qMax(COMP_NOISE_FLOOR, m_yL[i]);

            // For the visualizer
            m_displayPeak[i] = m_yL[i];

            const real_t currentPeakDbfs = ampToDbfs(m_yL[i]);

            // Now find the gain change that should be applied,
            // depending on the measured input value.
            if(currentPeakDbfs - m_thresholdVal < -m_kneeVal)  // Below knee
            {
                gainResult[i] = currentPeakDbfs;
            }
            else if(abs(currentPeakDbfs - m_thresholdVal)
                    < m_kneeVal)  // Within knee
            {
                const real_t temp
                        = currentPeakDbfs - m_thresholdVal + m_kneeVal;
                gainResult[i] = currentPeakDbfs
                                + ((limiter ? 0 : m_ratioVal) - 1) * temp
                                          * temp / (4 * m_kneeVal);
            }
            else  // Above knee
            {
                gainResult[i]
                        = limiter ? m_thresholdVal
                                  : (m_thresholdVal
                                     + (currentPeakDbfs - m_thresholdVal)
                                               * m_ratioVal);
            }

            gainResult[i] = dbfsToAmp(gainResult[i]) / m_yL[i];
            gainResult[i] = qMax(m_rangeVal, gainResult[i]);
        }

        switch(stereoLink)
        {
            case 1:  // Maximum
            {
                gainResult[0] = gainResult[1]
                        = qMin(gainResult[0], gainResult[1]);
                break;
            }
            case 2:  // Average
            {
                gainResult[0] = gainResult[1]
                        = (gainResult[0] + gainResult[1]) * 0.5;
                break;
            }
            case 3:  // Minimum
            {
                gainResult[0] = gainResult[1]
                        = qMax(gainResult[0], gainResult[1]);
                break;
            }
            case 4:  // Blend
            {
                if(blend > 0)  // 0 is unlinked
                {
                    if(blend <= 1)  // Blend to minimum volume
                    {
                        const real_t temp1
                                = qMin(gainResult[0], gainResult[1]);
                        gainResult[0] = linearInterpolate(gainResult[0],
                                                          temp1, blend);
                        gainResult[1] = linearInterpolate(gainResult[1],
                                                          temp1, blend);
                    }
                    else if(blend <= 2)  // Blend to average volume
                    {
                        const real_t temp1
                                = qMin(gainResult[0], gainResult[1]);
                        const real_t temp2
                                = (gainResult[0] + gainResult[1]) * 0.5;
                        gainResult[0]
                                = linearInterpolate(temp1, temp2, blend - 1);
                        gainResult[1] = gainResult[0];
                    }
                    else  // Blend to maximum volume
                    {
                        const real_t temp1
                                = (gainResult[0] + gainResult[1]) * 0.5;
                        const real_t temp2
                                = qMax(gainResult[0], gainResult[1]);
                        gainResult[0]
                                = linearInterpolate(temp1, temp2, blend - 2);
                        gainResult[1] = gainResult[0];
                    }
                }
                break;
            }
        }

        // Bias compression to the left or right (or mid or side)
        if(stereoBalance != 0)
        {
            gainResult[0] = 1
                            - ((1 - gainResult[0])
                               * (stereoBalance > 0 ? 1 - stereoBalance : 1));
            gainResult[1] = 1
                            - ((1 - gainResult[1])
                               * (stereoBalance < 0 ? 1 + stereoBalance : 1));
        }

        // For visualizer
        m_displayGain[0] = gainResult[0];
        m_displayGain[1] = gainResult[1];

        // Delay the signal by 20 ms via ring buffer if lookahead is enabled
        if(lookahead)
        {
            ++m_inputBufLoc;
            if(m_inputBufLoc >= m_lookaheadDelayLength)
            {
                m_inputBufLoc = 0;
            }

            const real_t temp[2] = {drySignal[0], drySignal[1]};

            s[0] = m_inputBuf[0][m_inputBufLoc];
            s[1] = m_inputBuf[1][m_inputBufLoc];

            m_inputBuf[0][m_inputBufLoc] = temp[0];
            m_inputBuf[1][m_inputBufLoc] = temp[1];
        }
        else
        {
            s[0] = drySignal[0];
            s[1] = drySignal[1];
        }

        real_t trueDrySignal[2] = {s[0], s[1]};

        if(midside)  // Convert left/right to mid/side
        {
            const real_t temp = s[0];
            s[0]              = (temp + s[1]) * 0.5;
            s[1]              = temp - s[1];
        }

        s[0] *= gainResult[0] * m_outGainVal
                * (outBalance > 0 ? 1 - outBalance : 1);
        s[1] *= gainResult[1] * m_outGainVal
                * (outBalance < 0 ? 1 + outBalance : 1);

        if(midside)  // Convert mid/side back to left/right
        {
            const real_t temp1 = s[0];
            const real_t temp2 = s[1] * 0.5;
            s[0]               = temp1 + temp2;
            s[1]               = temp1 - temp2;
        }

        m_prevOut[0] = s[0];
        m_prevOut[1] = s[1];

        // Negate wet signal from dry signal
        if(audition)
        {
            s[0] = -s[0] + trueDrySignal[0];
            s[1] = -s[1] + trueDrySignal[1];
        }
        else if(autoMakeup)
        {
            s[0] *= m_autoMakeupVal;
            s[1] *= m_autoMakeupVal;
        }

        // Calculate wet/dry value results
        const real_t temp1 = trueDrySignal[0] / m_inGainVal;
        const real_t temp2 = trueDrySignal[1] / m_inGainVal;
        // buf[f][0]          = d * temp1 + w * s[0];
        // buf[f][1]          = d * temp2 + w * s[1];
        buf[f][0] = d0 * temp1 + w0 * s[0];
        buf[f][1] = d1 * temp2 + w1 * s[1];
        buf[f][0] = (1 - m_mixVal) * temp1 + m_mixVal * buf[f][0];
        buf[f][1] = (1 - m_mixVal) * temp2 + m_mixVal * buf[f][1];

        // outSum += s[0] + s[1];

        lInPeak  = buf[f][0] > lInPeak ? buf[f][0] : lInPeak;
        rInPeak  = buf[f][1] > rInPeak ? buf[f][1] : rInPeak;
        lOutPeak = s[0] > lOutPeak ? s[0] : lOutPeak;
        rOutPeak = s[1] > rOutPeak ? s[1] : rOutPeak;
    }

    // checkGate(outSum / frames);
    m_compressorControls.m_outPeakL = lOutPeak;
    m_compressorControls.m_outPeakR = rOutPeak;
    m_compressorControls.m_inPeakL  = lInPeak;
    m_compressorControls.m_inPeakR  = rInPeak;

    // return isRunning();
    return shouldKeepRunning(buf, frames);  //, true);
}

// Regular modulo doesn't handle negative numbers correctly.  This does.
inline int CompressorEffect::realmod(int k, int n)
{
    return ((k %= n) < 0) ? k + n : k;
}

// Regular fmod doesn't handle negative numbers correctly.  This does.
inline real_t CompressorEffect::realfmod(real_t k, real_t n)
{
    return ((k = fmod(k, n)) < 0) ? k + n : k;
}

inline void CompressorEffect::calcTiltFilter(sample_t  inputSample,
                                             sample_t& outputSample,
                                             int       filtNum)
{
    m_tiltOut[filtNum] = m_a0 * inputSample + m_b1 * m_tiltOut[filtNum];
    outputSample       = inputSample + m_lgain * m_tiltOut[filtNum]
                   + m_hgain * (inputSample - m_tiltOut[filtNum]);
}

void CompressorEffect::changeSampleRate()
{
    m_sampleRate   = Engine::mixer()->processingSampleRate();
    m_coeffPrecalc = COMP_LOG / (m_sampleRate * 0.001);

    // 200 ms
    m_crestTimeConst = exp(-1. / (0.2 * m_sampleRate));

    // 20 ms
    m_lookaheadDelayLength = 0.020 * m_sampleRate;
    m_inputBuf[0].resize(m_lookaheadDelayLength);
    m_inputBuf[1].resize(m_lookaheadDelayLength);

    emit calcAutoMakeup();
    emit calcAttack();
    emit calcRelease();
    emit calcRatio();
    emit calcRange();
    emit calcLookaheadLength();
    emit calcHold();
    emit resizeRMS();
    emit calcThreshold();
    emit calcKnee();
    emit calcOutGain();
    emit calcInGain();
    emit calcTiltCoeffs();
    emit calcMix();
}

extern "C"
{

    // necessary for getting instance out of shared lib
    PLUGIN_EXPORT Plugin* lmms_plugin_main(Model* parent, void* data)
    {
        return new CompressorEffect(
                parent,
                static_cast<
                        const Plugin::Descriptor::SubPluginFeatures::Key*>(
                        data));
    }
}
