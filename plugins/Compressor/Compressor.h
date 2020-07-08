/*
 * Compressor.h
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

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "CompressorControls.h"
#include "Effect.h"
#include "RmsHelper.h"
#include "ValueBuffer.h"

constexpr real_t COMP_LOG = -2.2;

class CompressorEffect : public Effect
{
    Q_OBJECT

  public:
    CompressorEffect(Model*                                    parent,
                     const Descriptor::SubPluginFeatures::Key* key);
    ~CompressorEffect() override;
    bool processAudioBuffer(sampleFrame* buf, const fpp_t frames) override;

    EffectControls* controls() override
    {
        return &m_compressorControls;
    }

  private slots:
    void calcAutoMakeup();
    void calcAttack();
    void calcRelease();
    void calcAutoAttack();
    void calcAutoRelease();
    void calcHold();
    void calcOutGain();
    void calcRatio();
    void calcRange();
    void resizeRMS();
    void calcLookaheadLength();
    void calcThreshold();
    void calcKnee();
    void calcInGain();
    void calcTiltCoeffs();
    void calcMix();
    void changeSampleRate();
    void redrawKnee();

  private:
    CompressorControls m_compressorControls;

    real_t msToCoeff(real_t time);

    inline void   calcTiltFilter(sample_t  inputSample,
                                 sample_t& outputSample,
                                 int       filtNum);
    inline int    realmod(int k, int n);
    inline real_t realfmod(real_t k, real_t n);

    std::vector<sample_t> m_preLookaheadBuf[2];
    int                   m_preLookaheadBufLoc[2] = {0};

    std::vector<sample_t> m_lookaheadBuf[2];
    int                   m_lookaheadBufLoc[2] = {0};

    std::vector<sample_t> m_inputBuf[2];
    int                   m_inputBufLoc = 0;

    real_t m_attCoeff;
    real_t m_relCoeff;
    real_t m_autoAttVal;
    real_t m_autoRelVal;

    int m_holdLength   = 0;
    int m_holdTimer[2] = {0, 0};

    int    m_lookaheadLength;
    int    m_lookaheadDelayLength;
    int    m_preLookaheadLength;
    real_t m_thresholdAmpVal;
    real_t m_autoMakeupVal;
    real_t m_outGainVal;
    real_t m_inGainVal;
    real_t m_rangeVal;
    real_t m_tiltVal;
    real_t m_mixVal;

    real_t m_coeffPrecalc;

    sampleFrame m_maxLookaheadVal      = {0};
    int         m_maxLookaheadTimer[2] = {1, 1};

    RmsHelper* m_rms[2];

    real_t m_crestPeakVal[2]   = {0, 0};
    real_t m_crestRmsVal[2]    = {0, 0};
    real_t m_crestFactorVal[2] = {0, 0};
    real_t m_crestTimeConst;

    real_t m_tiltOut[2] = {0};

    bool m_cleanedBuffers = false;

    sample_rate_t m_sampleRate;

    real_t m_lgain;
    real_t m_hgain;
    real_t m_a0;
    real_t m_b1;

    real_t m_prevOut[2] = {0};

    real_t m_yL[2];
    real_t m_displayPeak[2];
    real_t m_displayGain[2];

    real_t m_kneeVal;
    real_t m_thresholdVal;
    real_t m_ratioVal;

    bool m_redrawKnee      = true;
    bool m_redrawThreshold = true;

    friend class CompressorControls;
    friend class CompressorControlDialog;
};

#endif
