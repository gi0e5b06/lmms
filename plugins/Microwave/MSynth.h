/*
 * MSynth.h -
 *
 * Copyright (c) 2019 Robert Black AKA DouglasDGI AKA Lost Robot
 * <r94231/at/gmail/dot/com>
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

#ifndef MSYNTH_H
#define MSYNTH_H

#include "Microwave.h"

class MSynth
{
    MM_OPERATORS

  public:
    MSynth(NotePlayHandle*     _nph,
           const sample_rate_t _sample_rate,

           real_t*  morphVal,
           real_t*  rangeVal,
           real_t*  modifyVal,
           int32_t* modifyModeVal,
           int32_t* sampLen,
           bool*    enabled,
           bool*    muted,
           int32_t* detuneVal,
           int32_t* unisonVoices,
           int32_t* unisonDetune,
           real_t*  unisonMorph,
           real_t*  unisonModify,
           real_t*  morphMaxVal,
           real_t*  phase,
           real_t*  phaseRand,
           real_t*  vol,
           real_t*  pan,
           // MWave (&waveforms)[NB_MAINOSC],
           // PtrArray<NumArray<sample_t, NB_MAINOSC>, MAINWAV_SIZE>&
           // waveforms,

           bool*    subEnabled,
           real_t*  subVol,
           real_t*  subPhase,
           real_t*  subPhaseRand,
           int32_t* subDetune,
           bool*    subMuted,
           bool*    subKeytrack,
           int32_t* subSampLen,
           bool*    subNoise,
           real_t*  subPanning,
           int32_t* subTempo,
           // sample_t (&subs)[SUBWAV_SIZE],
           // NumArray<sample_t, SUBWAV_SIZE>& subs,

           int32_t* modIn,
           int32_t* modInNum,
           int32_t* modInOtherNum,
           real_t*  modInAmnt,
           real_t*  modInCurve,
           int32_t* modIn2,
           int32_t* modInNum2,
           int32_t* modInOtherNum2,
           real_t*  modInAmnt2,
           real_t*  modInCurve2,
           int32_t* modOutSec,
           int32_t* modOutSig,
           int32_t* modOutSecNum,
           int32_t* modCombineType,
           bool*    modType,
           bool*    modEnabled,

           real_t*  filtInVol,
           int32_t* filtType,
           int32_t* filtSlope,
           real_t*  filtCutoff,
           real_t*  filtReso,
           real_t*  filtGain,
           real_t*  filtSatu,
           real_t*  filtWetDry,
           real_t*  filtBal,
           real_t*  filtOutVol,
           bool*    filtEnabled,
           real_t*  filtFeedback,
           int32_t* filtDetune,
           bool*    filtKeytracking,
           bool*    filtMuted,

           // FLOAT*  sampGraphs,

           bool*    sampleEnabled,
           bool*    sampleGraphEnabled,
           bool*    sampleMuted,
           bool*    sampleKeytracking,
           bool*    sampleLoop,
           real_t*  sampleVolume,
           real_t*  samplePanning,
           int32_t* sampleDetune,
           real_t*  samplePhase,
           real_t*  samplePhaseRand,
           real_t*  sampleStart,
           real_t*  sampleEnd,
           StereoClip (&samples)[NB_SMPLR],

           real_t* macro);
    virtual ~MSynth();

    StereoSample nextStringSample(
            // MWave (&waveforms)[NB_MAINOSC],
            ObjArray<NumArray<sample_t, MAINWAV_SIZE>, NB_MAINOSC>& waveforms,
            // sample_t* subs,
            NumArray<sample_t, SUBWAV_SIZE>& subs,
            FLOAT*                           sampGraphs,
            StereoClip (&samples)[NB_SMPLR],
            int32_t       maxFiltEnabled,
            int32_t       maxModEnabled,
            int32_t       maxSubEnabled,
            int32_t       maxSampleEnabled,
            int32_t       maxMainEnabled,
            sample_rate_t sample_rate);

    inline real_t detuneWithCents(real_t pitchValue, int32_t detuneValue);

    void refreshValue(int32_t which, int32_t num);

  private:
    NotePlayHandle*     nph;
    const sample_rate_t sample_rate;
    QVector<int32_t>    modValType;
    QVector<int32_t>    modValNum;
    int32_t             noteDuration;
    real_t              noteFreq;

    real_t sample_realindex[NB_MAINOSC][NB_WAVES] = {{0}};
    real_t sample_subindex[NB_SUBOSC]             = {0};
    real_t sample_sampleindex[NB_SMPLR]           = {0};
    real_t lastMainOscVal[NB_MAINOSC][2]          = {{0}};
    real_t lastSubVal[NB_SUBOSC][2]               = {{0}};
    real_t lastSampleVal[NB_SMPLR][2]             = {{0}};
    real_t sample_step_sub                        = 0;
    real_t noiseSampRand                          = 0;
    real_t sample_step_sample                     = 0;

    real_t lastMainOscEnvVal[NB_MAINOSC][2] = {{0}};
    real_t lastSubEnvVal[NB_SUBOSC][2]      = {{0}};
    real_t lastSampleEnvVal[NB_SMPLR][2]    = {{0}};

    bool lastMainOscEnvDone[NB_MAINOSC] = {false};
    bool lastSubEnvDone[NB_SUBOSC]      = {false};
    bool lastSampleEnvDone[NB_SMPLR]    = {false};

    int32_t loopStart             = 0;
    int32_t loopEnd               = 0;
    real_t  currentRangeValInvert = 0;
    int32_t currentSampLen        = 0;
    int32_t currentIndex          = 0;

    StereoSample filtInputs[NB_FILTR];   // [filter number][channel]
    StereoSample filtOutputs[NB_FILTR];  // [filter number][channel]
    StereoSample filtPrevSampIn[NB_FILTR][8][5];
    // [filter number][slope][samples back in time][channel]
    StereoSample filtPrevSampOut[NB_FILTR][8][5];
    // [filter number][slope][samples back in time][channel]
    StereoReal        filtModOutputs[NB_FILTR];   // [filter number][channel]
    QVector<sample_t> filtDelayBuf[NB_FILTR][2];  // [filter number][channel]

    real_t filty1[2]    = {0};
    real_t filty2[2]    = {0};
    real_t filty3[2]    = {0};
    real_t filty4[2]    = {0};
    real_t filtoldx[2]  = {0};
    real_t filtoldy1[2] = {0};
    real_t filtoldy2[2] = {0};
    real_t filtoldy3[2] = {0};
    real_t filtx[2]     = {0};

    int32_t modifyModeVal[NB_MAINOSC];
    real_t  modifyVal[NB_MAINOSC];
    real_t  morphVal[NB_MAINOSC];
    real_t  rangeVal[NB_MAINOSC];
    real_t  unisonVoices[NB_MAINOSC];
    int32_t unisonDetune[NB_MAINOSC];
    real_t  unisonMorph[NB_MAINOSC];
    real_t  unisonModify[NB_MAINOSC];
    real_t  morphMaxVal[NB_MAINOSC];
    int32_t detuneVal[NB_MAINOSC];
    int32_t sampLen[NB_MAINOSC];
    real_t  phase[NB_MAINOSC];
    real_t  phaseRand[NB_MAINOSC];
    real_t  vol[NB_MAINOSC];
    bool    enabled[NB_MAINOSC];
    bool    muted[NB_MAINOSC];
    real_t  pan[NB_MAINOSC];

    int32_t modIn[NB_MODLT];
    int32_t modInNum[NB_MODLT];
    int32_t modInOtherNum[NB_MODLT];
    real_t  modInAmnt[NB_MODLT];
    real_t  modInCurve[NB_MODLT];
    int32_t modIn2[NB_MODLT];
    int32_t modInNum2[NB_MODLT];
    int32_t modInOtherNum2[NB_MODLT];
    real_t  modInAmnt2[NB_MODLT];
    real_t  modInCurve2[NB_MODLT];
    int32_t modOutSec[NB_MODLT];
    int32_t modOutSig[NB_MODLT];
    int32_t modOutSecNum[NB_MODLT];
    bool    modEnabled[NB_MODLT];
    int32_t modCombineType[NB_MODLT];
    bool    modType[NB_MODLT];

    bool    subEnabled[NB_SUBOSC];
    real_t  subVol[NB_SUBOSC];
    real_t  subPhase[NB_SUBOSC];
    real_t  subPhaseRand[NB_SUBOSC];
    int32_t subDetune[NB_SUBOSC];
    bool    subMuted[NB_SUBOSC];
    bool    subKeytrack[NB_SUBOSC];
    int32_t subSampLen[NB_SUBOSC];
    bool    subNoise[NB_SUBOSC];
    real_t  subPanning[NB_SUBOSC];
    int32_t subTempo[NB_SUBOSC];

    real_t  filtInVol[NB_FILTR];
    int32_t filtType[NB_FILTR];
    int32_t filtSlope[NB_FILTR];
    real_t  filtCutoff[NB_FILTR];
    real_t  filtReso[NB_FILTR];
    real_t  filtGain[NB_FILTR];
    real_t  filtSatu[NB_FILTR];
    real_t  filtWetDry[NB_FILTR];
    real_t  filtBal[NB_FILTR];
    real_t  filtOutVol[NB_FILTR];
    bool    filtEnabled[NB_FILTR];
    real_t  filtFeedback[NB_FILTR];
    int32_t filtDetune[NB_FILTR];
    bool    filtKeytracking[NB_FILTR];
    bool    filtMuted[NB_FILTR];

    bool    sampleEnabled[NB_SMPLR];
    bool    sampleGraphEnabled[NB_SMPLR];
    bool    sampleMuted[NB_SMPLR];
    bool    sampleKeytracking[NB_SMPLR];
    bool    sampleLoop[NB_SMPLR];
    real_t  sampleVolume[NB_SMPLR];
    real_t  samplePanning[NB_SMPLR];
    int32_t sampleDetune[NB_SMPLR];
    real_t  samplePhase[NB_SMPLR];
    real_t  samplePhaseRand[NB_SMPLR];
    real_t  sampleStart[NB_SMPLR];
    real_t  sampleEnd[NB_SMPLR];

    real_t  cutoff;
    int32_t mode;
    real_t  reso;
    real_t  dbgain;
    real_t  Fs;
    real_t  a0;
    real_t  a1;
    real_t  a2;
    real_t  b0;
    real_t  b1;
    real_t  b2;
    real_t  alpha;
    real_t  w0;
    real_t  A;
    real_t  f;
    real_t  k;
    real_t  p;
    real_t  scale;
    real_t  r;

    real_t humanizer[NB_SMPLR] = {0};

    real_t unisonDetuneAmounts[NB_MAINOSC][NB_WAVES] = {{0}};

    // real_t temp1;
    // real_t temp2;
    // real_t temp3;

    StereoReal curModVal;
    StereoReal curModVal2;
    StereoReal curModValCurve;
    StereoReal curModVal2Curve;
    StereoReal comboModVal;
    real_t     comboModValMono = 0;

    real_t sample_morerealindex[NB_MAINOSC][NB_WAVES] = {{0}};
    real_t sample_step[NB_MAINOSC][NB_WAVES]          = {{0}};
    real_t sample_length_modify[NB_MAINOSC][NB_WAVES] = {{0}};

    // real_t unisonVoicesMinusOne = 0;

    bool updateFrequency = false;

    real_t macro[NB_MACRO];

    friend class Microwave;
};

#endif
