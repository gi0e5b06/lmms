/*
 * MSynth.cpp -
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

#include "MSynth.h"

#include "Engine.h"
//#include "Graph.h"
#include "InstrumentTrack.h"
#include "Mixer.h"
#include "NotePlayHandle.h"
#include "Oscillator.h"
#include "SampleBuffer.h"
#include "base64.h"
#include "interpolation.h"
#include "lmms_math.h"
//#include "plugin_export.h"
#include "templates.h"

#include <QDomElement>

#include <iostream>
using namespace std;

// Initializes MSynth (when a new note is played).  Clone all of the arrays
// storing the knob values so they can be changed by modulation.
MSynth::MSynth(
        NotePlayHandle*     _nph,
        const sample_rate_t _sample_rate,

        real_t*  _morphArr,
        real_t*  _rangeArr,
        real_t*  _modifyArr,
        int32_t* _modifyModeArr,
        int32_t* _sampLenArr,
        bool*    _enabledArr,
        bool*    _mutedArr,
        int32_t* _detuneArr,
        int32_t* _unisonVoicesArr,
        int32_t* _unisonDetuneArr,
        real_t*  _unisonMorphArr,
        real_t*  _unisonModifyArr,
        real_t*  _morphMaxArr,
        real_t*  _phaseArr,
        real_t*  _phaseRandArr,
        real_t*  _volArr,
        real_t*  _panArr,
        // MWave (&waveforms)[NB_MAINOSC],
        // PtrArray<NumArray<sample_t, NB_MAINOSC>, MAINWAV_SIZE>& waveforms,

        bool*    _subEnabledArr,
        real_t*  _subVolArr,
        real_t*  _subPhaseArr,
        real_t*  _subPhaseRandArr,
        int32_t* _subDetuneArr,
        bool*    _subMutedArr,
        bool*    _subKeytrackArr,
        int32_t* _subSampLenArr,
        bool*    _subNoiseArr,
        real_t*  _subPanningArr,
        int32_t* _subTempoArr,
        // sample_t (&_subsArr)[SUBWAV_SIZE],
        // NumArray<sample_t, SUBWAV_SIZE>& _subsArr,

        int32_t* _modInArr,
        int32_t* _modInNumArr,
        int32_t* _modInOtherNumArr,
        real_t*  _modInAmntArr,
        real_t*  _modInCurveArr,
        int32_t* _modIn2Arr,
        int32_t* _modInNum2Arr,
        int32_t* _modInOtherNum2Arr,
        real_t*  _modInAmnt2Arr,
        real_t*  _modInCurve2Arr,
        int32_t* _modOutSecArr,
        int32_t* _modOutSigArr,
        int32_t* _modOutSecNumArr,
        int32_t* _modCombineTypeArr,
        bool*    _modTypeArr,
        bool*    _modEnabledArr,

        real_t*  _filtInVolArr,
        int32_t* _filtTypeArr,
        int32_t* _filtSlopeArr,
        real_t*  _filtCutoffArr,
        real_t*  _filtResoArr,
        real_t*  _filtGainArr,
        real_t*  _filtSatuArr,
        real_t*  _filtWetDryArr,
        real_t*  _filtBalArr,
        real_t*  _filtOutVolArr,
        bool*    _filtEnabledArr,
        real_t*  _filtFeedbackArr,
        int32_t* _filtDetuneArr,
        bool*    _filtKeytrackingArr,
        bool*    _filtMutedArr,

        // FLOAT*  sampGraphs,

        bool*    _sampleEnabledArr,
        bool*    _sampleGraphEnabledArr,
        bool*    _sampleMutedArr,
        bool*    _sampleKeytrackingArr,
        bool*    _sampleLoopArr,
        real_t*  _sampleVolumeArr,
        real_t*  _samplePanningArr,
        int32_t* _sampleDetuneArr,
        real_t*  _samplePhaseArr,
        real_t*  _samplePhaseRandArr,
        real_t*  _sampleStartArr,
        real_t*  _sampleEndArr,
        StereoClip (&samples)[NB_SMPLR],

        real_t* _macroArr) :
      nph(_nph),
      sample_rate(_sample_rate), modValType(81), modValNum(81),
      noteDuration(-1)
{
    memcpy(phaseRand, _phaseRandArr, sizeof(int32_t) * NB_MAINOSC);
    memcpy(modifyModeVal, _modifyModeArr, sizeof(int32_t) * NB_MAINOSC);
    memcpy(modifyVal, _modifyArr, sizeof(real_t) * NB_MAINOSC);
    memcpy(morphVal, _morphArr, sizeof(real_t) * NB_MAINOSC);
    memcpy(rangeVal, _rangeArr, sizeof(real_t) * NB_MAINOSC);
    memcpy(sampLen, _sampLenArr, sizeof(int32_t) * NB_MAINOSC);
    memcpy(unisonVoices, _unisonVoicesArr, sizeof(int32_t) * NB_MAINOSC);
    memcpy(unisonDetune, _unisonDetuneArr, sizeof(int32_t) * NB_MAINOSC);
    memcpy(unisonMorph, _unisonMorphArr, sizeof(real_t) * NB_MAINOSC);
    memcpy(unisonModify, _unisonModifyArr, sizeof(real_t) * NB_MAINOSC);
    memcpy(morphMaxVal, _morphMaxArr, sizeof(real_t) * NB_MAINOSC);
    memcpy(detuneVal, _detuneArr, sizeof(int32_t) * NB_MAINOSC);

    memcpy(phase, _phaseArr, sizeof(real_t) * NB_MAINOSC);
    memcpy(vol, _volArr, sizeof(int32_t) * NB_MAINOSC);
    memcpy(enabled, _enabledArr, sizeof(bool) * NB_MAINOSC);
    memcpy(muted, _mutedArr, sizeof(bool) * NB_MAINOSC);
    memcpy(pan, _panArr, sizeof(real_t) * NB_MAINOSC);

    memcpy(subEnabled, _subEnabledArr, sizeof(bool) * NB_SUBOSC);
    memcpy(subVol, _subVolArr, sizeof(real_t) * NB_SUBOSC);
    memcpy(subPhase, _subPhaseArr, sizeof(real_t) * NB_SUBOSC);
    memcpy(subPhaseRand, _subPhaseRandArr, sizeof(real_t) * NB_SUBOSC);
    memcpy(subDetune, _subDetuneArr, sizeof(int32_t) * NB_SUBOSC);
    memcpy(subMuted, _subMutedArr, sizeof(bool) * NB_SUBOSC);
    memcpy(subKeytrack, _subKeytrackArr, sizeof(bool) * NB_SUBOSC);
    memcpy(subSampLen, _subSampLenArr, sizeof(int32_t) * NB_SUBOSC);
    memcpy(subNoise, _subNoiseArr, sizeof(bool) * NB_SUBOSC);
    memcpy(subPanning, _subPanningArr, sizeof(real_t) * NB_SUBOSC);
    memcpy(subTempo, _subTempoArr, sizeof(int32_t) * NB_SUBOSC);

    memcpy(modIn, _modInArr, sizeof(int32_t) * 64);
    memcpy(modInNum, _modInNumArr, sizeof(int32_t) * 64);
    memcpy(modInOtherNum, _modInOtherNumArr, sizeof(int32_t) * 64);
    memcpy(modInAmnt, _modInAmntArr, sizeof(real_t) * 64);
    memcpy(modInCurve, _modInCurveArr, sizeof(real_t) * 64);
    memcpy(modIn2, _modIn2Arr, sizeof(int32_t) * 64);
    memcpy(modInNum2, _modInNum2Arr, sizeof(int32_t) * 64);
    memcpy(modInOtherNum2, _modInOtherNum2Arr, sizeof(int32_t) * 64);
    memcpy(modInAmnt2, _modInAmnt2Arr, sizeof(real_t) * 64);
    memcpy(modInCurve2, _modInCurve2Arr, sizeof(real_t) * 64);
    memcpy(modOutSec, _modOutSecArr, sizeof(int32_t) * 64);
    memcpy(modOutSig, _modOutSigArr, sizeof(int32_t) * 64);
    memcpy(modOutSecNum, _modOutSecNumArr, sizeof(int32_t) * 64);
    memcpy(modCombineType, _modCombineTypeArr, sizeof(int32_t) * 64);
    memcpy(modType, _modTypeArr, sizeof(bool) * 64);
    memcpy(modEnabled, _modEnabledArr, sizeof(bool) * 64);

    memcpy(filtInVol, _filtInVolArr, sizeof(real_t) * NB_FILTR);
    memcpy(filtType, _filtTypeArr, sizeof(int32_t) * NB_FILTR);
    memcpy(filtSlope, _filtSlopeArr, sizeof(int32_t) * NB_FILTR);
    memcpy(filtCutoff, _filtCutoffArr, sizeof(real_t) * NB_FILTR);
    memcpy(filtReso, _filtResoArr, sizeof(real_t) * NB_FILTR);
    memcpy(filtGain, _filtGainArr, sizeof(real_t) * NB_FILTR);
    memcpy(filtSatu, _filtSatuArr, sizeof(real_t) * NB_FILTR);
    memcpy(filtWetDry, _filtWetDryArr, sizeof(real_t) * NB_FILTR);
    memcpy(filtBal, _filtBalArr, sizeof(real_t) * NB_FILTR);
    memcpy(filtOutVol, _filtOutVolArr, sizeof(real_t) * NB_FILTR);
    memcpy(filtEnabled, _filtEnabledArr, sizeof(bool) * NB_FILTR);
    memcpy(filtFeedback, _filtFeedbackArr, sizeof(real_t) * NB_FILTR);
    memcpy(filtDetune, _filtDetuneArr, sizeof(int32_t) * NB_FILTR);
    memcpy(filtKeytracking, _filtKeytrackingArr, sizeof(bool) * NB_FILTR);
    memcpy(filtMuted, _filtMutedArr, sizeof(bool) * NB_FILTR);

    // ??? sampGraphs
    memcpy(sampleEnabled, _sampleEnabledArr, sizeof(bool) * NB_SMPLR);
    memcpy(sampleGraphEnabled, _sampleGraphEnabledArr,
           sizeof(bool) * NB_SMPLR);
    memcpy(sampleMuted, _sampleMutedArr, sizeof(bool) * NB_SMPLR);
    memcpy(sampleKeytracking, _sampleKeytrackingArr, sizeof(bool) * NB_SMPLR);
    memcpy(sampleLoop, _sampleLoopArr, sizeof(bool) * NB_SMPLR);
    memcpy(sampleVolume, _sampleVolumeArr, sizeof(real_t) * NB_SMPLR);
    memcpy(samplePanning, _samplePanningArr, sizeof(real_t) * NB_SMPLR);
    memcpy(sampleDetune, _sampleDetuneArr, sizeof(int32_t) * NB_SMPLR);
    memcpy(samplePhase, _samplePhaseArr, sizeof(real_t) * NB_SMPLR);
    memcpy(samplePhaseRand, _samplePhaseRandArr, sizeof(real_t) * NB_SMPLR);
    // ??? samples
    memcpy(sampleStart, _sampleStartArr, sizeof(real_t) * NB_SMPLR);
    memcpy(sampleEnd, _sampleEndArr, sizeof(real_t) * NB_SMPLR);

    memcpy(macro, _macroArr, sizeof(real_t) * NB_MACRO);

    // Main oscs
    for(int i = 0; i < NB_MAINOSC; ++i)
    {
        int32_t sampLeni = sampLen[i];
        if(sampLeni < 1)
        {
            qWarning("MSynth::MSynth sampLen < 1");
            sampLeni = sampLen[i] = WAVE_SIZE;
        }
        for(int j = 0; j < NB_WAVES; ++j)
        {
            // Randomize the phases of all of the waveforms
            sample_realindex[i][j]
                    = int(((fastrand(sampLen[i]) - (sampLeni * 0.5))
                           * (phaseRand[i] * 0.01))
                          + (sampLeni * 0.5))
                      % sampLeni;
        }
        // }
        // for(int i = 0; i < NB_MAINOSC; ++i)
        // {
        for(int j = 0; j < unisonVoices[i]; ++j)
        {
            unisonDetuneAmounts[i][j]
                    = ((rand() / real_t(RAND_MAX)) * 2.f) - 1;
        }
    }

    // Sub oscs
    for(int i = 0; i < NB_SUBOSC; ++i)
    {
        sample_subindex[i] = 0;
    }

    // Samplers
    for(int i = 0; i < NB_SMPLR; ++i)
    {
        sample_sampleindex[i]
                = fmod(fastrand(samples[i][0].size())
                               * (samplePhaseRand[i] * 0.01),
                       (samples[i][0].size() * sampleEnd[i])
                               - (samples[i][0].size() * sampleStart[i]))
                  + (samples[i][0].size() * sampleStart[i]);
        humanizer[i] = (rand() / real_t(RAND_MAX)) * 2
                       - 1;  // Generate humanizer values at the beginning of
                             // every note
    }
}

MSynth::~MSynth()
{
}

// The heart of Microwave.  As you probably learned in anatomy class, hearts
// actually aren't too pretty.  This is no exception. This is the part that
// puts everything together and calculates an audio output.
StereoSample MSynth::nextStringSample(
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
        sample_rate_t sample_rate)
{
    ++noteDuration;

    sample_t sample[NB_MAINOSC][NB_WAVES] = {{0}};

    // maxModEnabled keeps this from looping 64 times every sample,
    // saving a lot of CPU
    for(int l = 0; l < maxModEnabled; ++l)
    {
        if(modEnabled[l])
        {
            switch(modIn[l])
            {
                case 0:
                {
                    curModVal[0] = 0;
                    curModVal[1] = 0;
                    break;
                }
                case 1:
                {
                    if(modType[l])  // If envelope
                    {
                        curModVal[0] = lastMainOscEnvVal[modInNum[l] - 1][0];
                        curModVal[1] = lastMainOscEnvVal[modInNum[l] - 1][1];
                    }
                    else
                    {
                        curModVal[0] = lastMainOscVal[modInNum[l] - 1][0];
                        curModVal[1] = lastMainOscVal[modInNum[l] - 1][1];
                    }
                    break;
                }
                case 2:
                {
                    if(modType[l])  // If envelope
                    {
                        curModVal[0] = lastSubEnvVal[modInNum[l] - 1][0];
                        curModVal[1] = lastSubEnvVal[modInNum[l] - 1][1];
                    }
                    else
                    {
                        curModVal[0] = lastSubVal[modInNum[l] - 1][0];
                        curModVal[1] = lastSubVal[modInNum[l] - 1][1];
                    }
                    break;
                }
                case 3:
                {
                    curModVal[0] = lastSampleVal[modInNum[l] - 1][0];
                    curModVal[1] = lastSampleVal[modInNum[l] - 1][1];
                    break;
                }
                case 4:
                {
                    curModVal[0] = filtModOutputs[modInNum[l] - 1][0];
                    curModVal[1] = filtModOutputs[modInNum[l] - 1][1];
                    break;
                }
                case 5:  // Velocity
                {
                    curModVal[0] = (nph->getVolume() * 0.01) - 1;
                    curModVal[1] = curModVal[0];
                    break;
                }
                case 6:  // Panning
                {
                    curModVal[0] = (nph->getPanning() * 0.01);
                    curModVal[1] = curModVal[0];
                    break;
                }
                case 7:  // Humanizer
                {
                    curModVal[0] = humanizer[modInNum[l] - 1];
                    curModVal[1] = humanizer[modInNum[l] - 1];
                    break;
                }
                case 8:  // Macro
                {
                    curModVal[0] = macro[modInNum[l] - 1] * 0.01;
                    curModVal[1] = macro[modInNum[l] - 1] * 0.01;
                    break;
                }
                default:
                {
                    switch(modCombineType[l])
                    {
                        case 0:  // Add Bidirectional
                        {
                            curModVal[0] = 0;
                            curModVal[1] = 0;
                            break;
                        }
                        case 1:  // Multiply Bidirectional
                        {
                            curModVal[0] = 0;
                            curModVal[1] = 0;
                            break;
                        }
                        case 2:  // Add Unidirectional
                        {
                            curModVal[0] = -1;
                            curModVal[1] = -1;
                            break;
                        }
                        case 3:  // Multiply Unidirectional
                        {
                            curModVal[0] = -1;
                            curModVal[1] = -1;
                            break;
                        }
                    }
                }
            }
            switch(modIn2[l])
            {
                case 0:
                {
                    curModVal2[0] = 0;
                    curModVal2[1] = 0;
                    break;
                }
                case 1:
                {
                    if(modType[l])  // If envelope
                    {
                        curModVal2[0]
                                = lastMainOscEnvVal[modInNum2[l] - 1][0];
                        curModVal2[1]
                                = lastMainOscEnvVal[modInNum2[l] - 1][1];
                    }
                    else
                    {
                        curModVal2[0] = lastMainOscVal[modInNum2[l] - 1][0];
                        curModVal2[1] = lastMainOscVal[modInNum2[l] - 1][1];
                    }
                    break;
                }
                case 2:
                {
                    if(modType[l])  // If envelope
                    {
                        curModVal2[0] = lastSubEnvVal[modInNum2[l] - 1][0];
                        curModVal2[1] = lastSubEnvVal[modInNum2[l] - 1][1];
                    }
                    else
                    {
                        curModVal2[0] = lastSubVal[modInNum2[l] - 1][0];
                        curModVal2[1] = lastSubVal[modInNum2[l] - 1][1];
                    }
                    break;
                }
                case 3:
                {
                    curModVal2[0] = lastSampleVal[modInNum2[l] - 1][0];
                    curModVal2[1] = lastSampleVal[modInNum2[l] - 1][1];
                    break;
                }
                case 4:
                {
                    curModVal2[0] = filtModOutputs[modInNum2[l] - 1][0];
                    curModVal2[1] = filtModOutputs[modInNum2[l] - 1][1];
                }
                case 5:  // Velocity
                {
                    curModVal2[0] = (nph->getVolume() * 0.01) - 1;
                    curModVal2[1] = curModVal2[0];
                    break;
                }
                case 6:  // Panning
                {
                    curModVal2[0] = (nph->getPanning() * 0.01);
                    curModVal2[1] = curModVal2[0];
                    break;
                }
                case 7:  // Humanizer
                {
                    curModVal2[0] = humanizer[modInNum2[l] - 1];
                    curModVal2[1] = humanizer[modInNum2[l] - 1];
                    break;
                }
                case 8:  // Macro
                {
                    curModVal2[0] = macro[modInNum2[l] - 1] * 0.01;
                    curModVal2[1] = macro[modInNum2[l] - 1] * 0.01;
                    break;
                }
                default:
                {
                    switch(modCombineType[l])
                    {
                        case 0:  // Add Bidirectional
                        {
                            curModVal[0] = 0;
                            curModVal[1] = 0;
                            break;
                        }
                        case 1:  // Multiply Bidirectional
                        {
                            curModVal[0] = 0;
                            curModVal[1] = 0;
                            break;
                        }
                        case 2:  // Add Unidirectional
                        {
                            curModVal[0] = -1;
                            curModVal[1] = -1;
                            break;
                        }
                        case 3:  // Multiply Unidirectional
                        {
                            curModVal[0] = -1;
                            curModVal[1] = -1;
                            break;
                        }
                    }
                }
            }

            // "if" statements are there to prevent unnecessary division if
            // the modulation isn't used.
            if(modInAmnt[l] != 100.)
            {
                if(curModVal[0] != 0.)
                    curModVal[0] *= modInAmnt[l] * 0.01;
                if(curModVal[1] != 0.)
                    curModVal[1] *= modInAmnt[l] * 0.01;
            }
            if(modInAmnt2[l] != 100.)
            {
                if(curModVal2[0] != 0.)
                    curModVal2[0] *= modInAmnt2[l] * 0.01;
                if(curModVal2[1] != 0.)
                    curModVal2[1] *= modInAmnt2[l] * 0.01;
            }

            // Calculate curve
            if(modCombineType[l] <= 1)  // Bidirectional
            {
                if(modInCurve[l] != 100.)
                // The "if" statement is there so unnecessary
                // CPU isn't spent (pow is very expensive) if
                // the curve knob isn't being used.
                {
                    // Move to a scale of 0 to 1 (from -1 to 1) and then apply
                    // the curve.
                    curModValCurve[0]
                            = (curModVal[0] <= -1)
                                      ? (curModVal[0] + 1) * 0.5
                                      : pow((curModVal[0] + 1) * 0.5,
                                            1. / (modInCurve[l] * 0.01));
                    curModValCurve[1]
                            = (curModVal[1] <= -1)
                                      ? (curModVal[1] + 1) * 0.5
                                      : pow((curModVal[1] + 1) * 0.5,
                                            1. / (modInCurve[l] * 0.01));
                }
                else
                {
                    curModValCurve[0] = (curModVal[0] + 1) * 0.5;
                    curModValCurve[1] = (curModVal[1] + 1) * 0.5;
                }

                if(modInCurve2[l] != 100.)
                {
                    curModVal2Curve[0]
                            = (curModVal2[0] <= -1)
                                      ? (curModVal2[0] + 1) * 0.5
                                      : pow((curModVal2[0] + 1) * 0.5,
                                            1. / (modInCurve2[l] * 0.01));
                    curModVal2Curve[1]
                            = (curModVal2[1] <= -1)
                                      ? (curModVal2[1] + 1) * 0.5
                                      : pow((curModVal2[1] + 1) * 0.5,
                                            1. / (modInCurve2[l] * 0.01));
                }
                else
                {
                    curModVal2Curve[0] = (curModVal2[0] + 1) * 0.5;
                    curModVal2Curve[1] = (curModVal2[1] + 1) * 0.5;
                }
            }
            else  // Unidirectional
            {
                if(modInCurve[l] != 100.)
                {
                    curModValCurve[0]
                            = ((curModVal[0] <= -1)
                                       ? curModVal[0]
                                       : pow(abs(curModVal[0]),
                                             1. / (modInCurve[l] * 0.01))
                                                 * (curModVal[0] < 0 ? -1
                                                                     : 1))
                              + (modInAmnt[l] / 100.);
                    curModValCurve[1]
                            = ((curModVal[1] <= -1)
                                       ? curModVal[1]
                                       : pow(abs(curModVal[1]),
                                             1. / (modInCurve[l] * 0.01))
                                                 * (curModVal[1] < 0 ? -1
                                                                     : 1))
                              + (modInAmnt[l] / 100.);
                }
                else
                {
                    curModValCurve[0] = curModVal[0] + (modInAmnt[l] / 100.);
                    curModValCurve[1] = curModVal[1] + (modInAmnt[l] / 100.);
                }

                if(modInCurve2[l] != 100.)
                {
                    curModVal2Curve[0]
                            = ((curModVal2[0] <= -1)
                                       ? curModVal2[0]
                                       : pow(abs(curModVal2[0]),
                                             1. / (modInCurve2[l] * 0.01))
                                                 * (curModVal2[0] < 0 ? -1
                                                                      : 1))
                              + (modInAmnt2[l] / 100.);
                    curModVal2Curve[1]
                            = ((curModVal2[1] <= -1)
                                       ? curModVal2[1]
                                       : pow(abs(curModVal2[0]),
                                             1. / (modInCurve2[l] * 0.01))
                                                 * (curModVal2[0] < 0 ? -1
                                                                      : 1))
                              + (modInAmnt2[l] / 100.);
                }
                else
                {
                    curModVal2Curve[0]
                            = curModVal2[0] + (modInAmnt2[l] / 100.);
                    curModVal2Curve[1]
                            = curModVal2[1] + (modInAmnt2[l] / 100.);
                }
            }

            switch(modCombineType[l])
            {
                case 0:  // Add Bidirectional
                {
                    comboModVal[0]
                            = curModValCurve[0] + curModVal2Curve[0] - 1;
                    comboModVal[1]
                            = curModValCurve[1] + curModVal2Curve[1] - 1;
                    break;
                }
                case 1:  // Multiply Bidirectional
                {
                    comboModVal[0]
                            = curModValCurve[0] * curModVal2Curve[0] - 1;
                    comboModVal[1]
                            = curModValCurve[1] * curModVal2Curve[1] - 1;
                    break;
                }
                case 2:  // Add Unidirectional
                {
                    comboModVal[0] = curModValCurve[0] + curModVal2Curve[0];
                    comboModVal[1] = curModValCurve[1] + curModVal2Curve[1];
                    break;
                }
                case 3:  // Multiply Unidirectional
                {
                    comboModVal[0] = curModValCurve[0] * curModVal2Curve[0];
                    comboModVal[1] = curModValCurve[1] * curModVal2Curve[1];
                    break;
                }
                default:
                {
                    comboModVal[0] = 0;
                    comboModVal[1] = 0;
                }
            }
            comboModValMono = (comboModVal[0] + comboModVal[1]) * 0.5;
            switch(modOutSec[l])
            {
                case 0:
                {
                    break;
                }
                case 1:  // Main Oscillator
                {
                    switch(modOutSig[l])
                    {
                        case 0:
                        {
                            break;
                        }
                        case 1:  // Send input to Morph
                        {
                            morphVal[modOutSecNum[l] - 1] = bound(
                                    0.,
                                    morphVal[modOutSecNum[l] - 1]
                                            + (round((
                                                    comboModValMono)*morphMaxVal
                                                             [modOutSecNum[l]
                                                              - 1])),
                                    morphMaxVal[modOutSecNum[l] - 1]);
                            modValType.push_back(1);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 2:  // Send input to Range
                        {
                            rangeVal[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., rangeVal[modOutSecNum[l] - 1]
                                                + comboModValMono * 16.);
                            modValType.push_back(2);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 3:  // Send input to Modify
                        {
                            modifyVal[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., modifyVal[modOutSecNum[l] - 1]
                                                + comboModValMono * 2048.);
                            modValType.push_back(3);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 4:  // Send input to Pitch/Detune
                        {
                            detuneVal[modOutSecNum[l] - 1]
                                    = detuneVal[modOutSecNum[l] - 1]
                                      + comboModValMono * 4800.;
                            modValType.push_back(11);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 5:  // Send input to Phase
                        {
                            phase[modOutSecNum[l] - 1]
                                    = fmod(phase[modOutSecNum[l] - 1]
                                                   + comboModValMono,
                                           100);
                            modValType.push_back(13);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 6:  // Send input to Volume
                        {
                            vol[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., vol[modOutSecNum[l] - 1]
                                                + comboModValMono * 100.);
                            modValType.push_back(14);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 7:  // Send input to Panning
                        {
                            pan[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., pan[modOutSecNum[l] - 1]
                                                + comboModValMono * 200.);
                            modValType.push_back(75);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 8:  // Send input to Unison Voice Number
                        {
                            unisonVoices[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., unisonVoices[modOutSecNum[l] - 1]
                                                + comboModValMono * 32.);
                            modValType.push_back(6);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 9:  // Send input to Unison Detune
                        {
                            unisonDetune[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., unisonDetune[modOutSecNum[l] - 1]
                                                + comboModValMono * 2000.);
                            modValType.push_back(7);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 10:  // Send input to Unison Morph
                        {
                            unisonDetune[modOutSecNum[l] - 1] = bound(
                                    0.,
                                    unisonDetune[modOutSecNum[l] - 1]
                                            + (round((
                                                    comboModValMono)*morphMaxVal
                                                             [modOutSecNum[l]
                                                              - 1])),
                                    morphMaxVal[modOutSecNum[l] - 1]);
                            modValType.push_back(8);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 11:  // Send input to Unison Modify
                        {
                            unisonModify[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., unisonModify[modOutSecNum[l] - 1]
                                                + comboModValMono * 2048.);
                            modValType.push_back(9);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        default:
                        {
                        }
                    }
                    break;
                }
                case 2:  // Sub Oscillator
                {
                    switch(modOutSig[l])
                    {
                        case 0:
                        {
                            break;
                        }
                        case 1:  // Send input to Pitch/Detune
                        {
                            subDetune[modOutSecNum[l] - 1]
                                    = subDetune[modOutSecNum[l] - 1]
                                      + comboModValMono * 4800.;
                            modValType.push_back(30);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 2:  // Send input to Phase
                        {
                            subPhase[modOutSecNum[l] - 1]
                                    = fmod(subPhase[modOutSecNum[l] - 1]
                                                   + comboModValMono,
                                           100);
                            modValType.push_back(28);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 3:  // Send input to Volume
                        {
                            subVol[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., subVol[modOutSecNum[l] - 1]
                                                + comboModValMono * 100.);
                            modValType.push_back(27);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 4:  // Send input to Panning
                        {
                            subPanning[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., subPanning[modOutSecNum[l] - 1]
                                                + comboModValMono * 200.);
                            modValType.push_back(72);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 5:  // Send input to Sample Length
                        {
                            subSampLen[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., int(subSampLen[modOutSecNum[l] - 1]
                                            + comboModValMono * 2048.));
                            modValType.push_back(33);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        default:
                        {
                        }
                    }
                    break;
                }
                case 3:  // Sample Oscillator
                {
                    switch(modOutSig[l])
                    {
                        case 0:
                        {
                            break;
                        }
                        case 1:  // Send input to Pitch/Detune
                        {
                            sampleDetune[modOutSecNum[l] - 1]
                                    = sampleDetune[modOutSecNum[l] - 1]
                                      + comboModValMono * 4800.;
                            modValType.push_back(66);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 2:  // Send input to Phase
                        {
                            samplePhase[modOutSecNum[l] - 1]
                                    = fmod(samplePhase[modOutSecNum[l] - 1]
                                                   + comboModValMono,
                                           100);
                            modValType.push_back(67);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 3:  // Send input to Volume
                        {
                            sampleVolume[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., sampleVolume[modOutSecNum[l] - 1]
                                                + comboModValMono * 100.);
                            modValType.push_back(64);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        default:
                        {
                        }
                    }
                    break;
                }
                case 4:  // Matrix
                {
                    switch(modOutSig[l])
                    {
                        case 0:
                        {
                            break;
                        }
                        case 1:  // Mod In Amount
                        {
                            modInAmnt[modOutSecNum[l] - 1]
                                    = modInAmnt[modOutSecNum[l] - 1]
                                      + comboModValMono * 100.;
                            modValType.push_back(38);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 2:  // Mod In Curve
                        {
                            modInCurve[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., modInCurve[modOutSecNum[l] - 1]
                                                + comboModValMono * 100.);
                            modValType.push_back(39);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 3:  // Secondary Mod In Amount
                        {
                            modInAmnt2[modOutSecNum[l] - 1]
                                    = modInAmnt2[modOutSecNum[l] - 1]
                                      + comboModValMono * 100.;
                            modValType.push_back(43);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 4:  // Secondary Mod In Curve
                        {
                            modInCurve2[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0., modInCurve2[modOutSecNum[l] - 1]
                                                + comboModValMono * 100.);
                            modValType.push_back(44);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                    }
                    break;
                }
                case 5:  // Filter Input
                {
                    filtInputs[modOutSig[l]][0] += curModVal[0];
                    filtInputs[modOutSig[l]][1] += curModVal[1];
                    break;
                }
                case 6:  // Filter Parameters
                {
                    switch(modOutSig[l])
                    {
                        case 0:  // None
                        {
                            break;
                        }
                        case 1:  // Cutoff Frequency
                        {
                            filtCutoff[modOutSecNum[l] - 1] = bound(
                                    20.,
                                    filtCutoff[modOutSecNum[l] - 1]
                                            + comboModValMono * 19980.,
                                    21999.);
                            modValType.push_back(18);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 2:  // Resonance
                        {
                            filtReso[modOutSecNum[l] - 1] = qMax<real_t>(
                                    0.0001, filtReso[modOutSecNum[l] - 1]
                                                    + comboModValMono * 16.);
                            modValType.push_back(19);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 3:  // dbGain
                        {
                            filtGain[modOutSecNum[l] - 1]
                                    = filtGain[modOutSecNum[l] - 1]
                                      + comboModValMono * 64.;
                            modValType.push_back(20);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 4:  // Filter Type
                        {
                            filtType[modOutSecNum[l] - 1] = qMax<int>(
                                    0, int(filtType[modOutSecNum[l] - 1]
                                           + comboModValMono * 7));
                            modValType.push_back(16);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 5:  // Filter Slope
                        {
                            filtSlope[modOutSecNum[l] - 1] = qMax<int>(
                                    0, int(filtSlope[modOutSecNum[l] - 1]
                                           + comboModValMono * 8));
                            modValType.push_back(17);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 6:  // Input Volume
                        {
                            filtInVol[modOutSecNum[l] - 1] = qMax(
                                    0., filtInVol[modOutSecNum[l] - 1]
                                                + comboModValMono * 100.);
                            modValType.push_back(15);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 7:  // Output Volume
                        {
                            filtOutVol[modOutSecNum[l] - 1] = qMax(
                                    0., filtOutVol[modOutSecNum[l] - 1]
                                                + comboModValMono * 100.);
                            modValType.push_back(24);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 8:  // Wet/Dry
                        {
                            filtWetDry[modOutSecNum[l] - 1]
                                    = bound(0.,
                                            filtWetDry[modOutSecNum[l] - 1]
                                                    + comboModValMono * 100.,
                                            100.);
                            modValType.push_back(22);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 9:  // Balance/Panning
                        {
                            filtWetDry[modOutSecNum[l] - 1]
                                    = bound(-100.,
                                            filtWetDry[modOutSecNum[l] - 1]
                                                    + comboModValMono * 200.,
                                            100.);
                            modValType.push_back(23);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 10:  // Saturation
                        {
                            filtSatu[modOutSecNum[l] - 1] = qMax(
                                    0., filtSatu[modOutSecNum[l] - 1]
                                                + comboModValMono * 100.);
                            modValType.push_back(21);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 11:  // Feedback
                        {
                            filtFeedback[modOutSecNum[l] - 1] = qMax(
                                    0., filtFeedback[modOutSecNum[l] - 1]
                                                + comboModValMono * 100.);
                            modValType.push_back(69);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                        case 12:  // Detune
                        {
                            filtDetune[modOutSecNum[l] - 1] = qMax(
                                    0., filtDetune[modOutSecNum[l] - 1]
                                                + comboModValMono * 4800.);
                            modValType.push_back(70);
                            modValNum.push_back(modOutSecNum[l] - 1);
                            break;
                        }
                    }
                    break;
                }
                case 7:  // Macro
                {
                    macro[modOutSig[l]]
                            = macro[modOutSig[l]] + comboModValMono * 200.;
                    modValType.push_back(78);
                    modValNum.push_back(modOutSig[l]);
                    break;
                }
                default:
                {
                }
            }
        }
    }

    //============//
    //== FILTER ==//
    //============//

    // As much as it may seem like it, this section contains no intentional
    // attempts at obfuscation.

    for(int l = 0; l < maxFiltEnabled; ++l)
    {
        if(filtEnabled[l])
        {
            {
                const real_t temp1 = filtInVol[l] * 0.01;
                filtInputs[l][0] *= temp1;
                filtInputs[l][1] *= temp1;
            }

            if(filtKeytracking[l])
            {
                const real_t temp1 = round(
                        sample_rate
                        / detuneWithCents(nph->frequency(), filtDetune[l]));
                filtDelayBuf[l][0].resize(temp1);
                filtDelayBuf[l][1].resize(temp1);
            }
            else
            {
                const real_t temp1 = round(
                        sample_rate / detuneWithCents(440., filtDetune[l]));
                filtDelayBuf[l][0].resize(temp1);
                filtDelayBuf[l][1].resize(temp1);
            }

            {
                const real_t temp1 = filtDelayBuf[l][0].size() - 1;
                filtInputs[l][0] += filtDelayBuf[l][0].at(temp1);
                filtInputs[l][1] += filtDelayBuf[l][1].at(temp1);
            }

            cutoff = filtCutoff[l];
            mode   = filtType[l];
            reso   = filtReso[l];
            dbgain = filtGain[l];
            Fs     = sample_rate;
            w0     = R_2PI * cutoff / Fs;

            if(mode == 8)
            {
                f     = 2. * cutoff / Fs;
                k     = 3.6 * f - 1.6 * f * f - 1;
                p     = (k + 1) * 0.5;
                scale = pow(2.7182818284590452353602874713527,
                            (1 - p) * 1.386249);
                r     = reso * scale;
            }

            for(int m = 0; m < filtSlope[l] + 1; ++m)
            // m is the slope number.  So if m = 2, then the sound
            // is going from a 24 db to a 36 db slope, for example.
            {
                if(m)
                {
                    filtInputs[l][0] = filtOutputs[l][0];
                    filtInputs[l][1] = filtOutputs[l][1];
                }

                int32_t formulaType = 1;
                if(mode <= 7)
                {
                    switch(mode)
                    {
                        case 0:  // LP
                        {
                            const real_t temp1 = cos(w0);

                            alpha = sin(w0) / (2 * reso);
                            b0    = (1 - temp1) * 0.5;
                            b1    = 1 - temp1;
                            b2    = b0;
                            a0    = 1 + alpha;
                            a1    = -2 * temp1;
                            a2    = 1 - alpha;
                            break;
                        }
                        case 1:  // HP
                        {
                            const real_t temp1 = cos(w0);

                            alpha = sin(w0) / (2 * reso);
                            b0    = (1 + temp1) * 0.5;
                            b1    = -(1 + temp1);
                            b2    = b0;
                            a0    = 1 + alpha;
                            a1    = -2 * temp1;
                            a2    = 1 - alpha;
                            break;
                        }
                        case 2:  // BP
                        {
                            const real_t temp2 = sin(w0);

                            // log(2)/log(2.7182818284590452353602874713527)
                            alpha = temp2
                                    * sinh(R_2_LOG * 0.5 * reso * w0 / temp2);
                            b0          = alpha;
                            b1          = 0;
                            b2          = -alpha;
                            a0          = 1 + alpha;
                            a1          = -2 * cos(w0);
                            a2          = 1 - alpha;
                            formulaType = 2;
                            break;
                        }
                        case 3:  // Low Shelf
                        {
                            alpha = sin(w0) / (2 * reso);
                            A     = exp10(dbgain / 40);

                            const real_t temp1 = cos(w0);
                            const real_t temp2 = 2 * sqrt(A) * alpha;

                            b0 = A * ((A + 1) - (A - 1) * temp1 + temp2);
                            b1 = 2 * A * ((A - 1) - (A + 1) * temp1);
                            b2 = A * ((A + 1) - (A - 1) * temp1 - temp2);
                            a0 = (A + 1) + (A - 1) * temp1 + temp2;
                            a1 = -2 * ((A - 1) + (A + 1) * temp1);
                            a2 = (A + 1) + (A - 1) * temp1 - temp2;
                            break;
                        }
                        case 4:  // High Shelf
                        {
                            alpha = sin(w0) / (2 * reso);
                            A     = exp10(dbgain / 40);

                            const real_t temp1 = cos(w0);
                            const real_t temp2 = 2 * sqrt(A) * alpha;

                            b0 = A * ((A + 1) + (A - 1) * temp1 + temp2);
                            b1 = -2 * A * ((A - 1) + (A + 1) * temp1);
                            b2 = A * ((A + 1) + (A - 1) * temp1 - temp2);
                            a0 = (A + 1) - (A - 1) * temp1 + temp2;
                            a1 = 2 * ((A - 1) - (A + 1) * temp1);
                            a2 = (A + 1) - (A - 1) * temp1 - temp2;
                            break;
                        }
                        case 5:  // Peak
                        {
                            const real_t temp1 = -2 * cos(w0);
                            const real_t temp2 = sin(w0);

                            alpha = temp2
                                    * sinh((log(2)
                                            / log(2.7182818284590452353602874713527))
                                           / 2 * reso * w0 / temp2);
                            A  = pow(10, dbgain / 40);
                            b0 = 1 + alpha * A;
                            b1 = temp1;
                            b2 = 1 - alpha * A;
                            a0 = 1 + alpha / A;
                            a1 = -temp1;
                            a2 = 1 - alpha / A;
                            break;
                        }
                        case 6:  // Notch
                        {
                            const real_t temp1 = -2 * cos(w0);
                            const real_t temp2 = sin(w0);

                            // log(2) / log(2.7182818284590452353602874713527)
                            alpha = temp2
                                    * sinh(R_2_LOG / 2 * reso * w0 / temp2);
                            b0 = 1;
                            b1 = temp1;
                            b2 = 1;
                            a0 = 1 + alpha;
                            a1 = temp1;
                            a2 = 1 - alpha;
                            break;
                        }
                        case 7:  // Allpass
                        {
                            const real_t temp1 = -2 * cos(w0);

                            alpha = sin(w0) / (2 * reso);
                            b0    = 1 - alpha;
                            b1    = temp1;
                            b2    = 1 + alpha;
                            a0    = b2;
                            a1    = temp1;
                            a2    = b0;
                            break;
                        }
                    }

                    // Translation of this monstrosity (case 1): y[n] =
                    // (b0/a0)*x[n] + (b1/a0)*x[n-1] + (b2/a0)*x[n-2] -
                    // (a1/a0)*y[n-1] - (a2/a0)*y[n-2] See
                    // www.musicdsp.org/files/Audio-EQ-Cookbook.txt
                    switch(formulaType)
                    {
                        case 1:  // Normal
                        {
                            filtPrevSampOut[l][m][0][0]
                                    = (b0 / a0) * filtInputs[l][0]
                                      + (b1 / a0) * filtPrevSampIn[l][m][1][0]
                                      + (b2 / a0) * filtPrevSampIn[l][m][2][0]
                                      - (a1 / a0)
                                                * filtPrevSampOut[l][m][1][0]
                                      - (a2 / a0)
                                                * filtPrevSampOut[l][m][2]
                                                                 [0];  // Left
                                                                       // ear
                            filtPrevSampOut[l][m][0][1]
                                    = (b0 / a0) * filtInputs[l][1]
                                      + (b1 / a0) * filtPrevSampIn[l][m][1][1]
                                      + (b2 / a0) * filtPrevSampIn[l][m][2][1]
                                      - (a1 / a0)
                                                * filtPrevSampOut[l][m][1][1]
                                      - (a2 / a0)
                                                * filtPrevSampOut
                                                        [l][m][2]
                                                        [1];  // Right
                                                              // ear
                            break;
                        }
                        case 2:  // Bandpass
                        {
                            filtPrevSampOut[l][m][0][0]
                                    = (b0 / a0) * filtInputs[l][0]
                                      + (b2 / a0) * filtPrevSampIn[l][m][2][0]
                                      - (a1 / a0)
                                                * filtPrevSampOut[l][m][1][0]
                                      - (a2 / a0)
                                                * filtPrevSampOut[l][m][2]
                                                                 [0];  // Left
                                                                       // ear
                            filtPrevSampOut[l][m][0][1]
                                    = (b0 / a0) * filtInputs[l][1]
                                      + (b2 / a0) * filtPrevSampIn[l][m][2][1]
                                      - (a1 / a0)
                                                * filtPrevSampOut[l][m][1][1]
                                      - (a2 / a0)
                                                * filtPrevSampOut
                                                        [l][m][2]
                                                        [1];  // Right
                                                              // ear
                            break;
                        }
                    }

                    // Output results
                    {
                        const real_t temp1 = filtOutVol[l] * 0.01;
                        filtOutputs[l][0]
                                = filtPrevSampOut[l][m][0][0] * temp1;
                        filtOutputs[l][1]
                                = filtPrevSampOut[l][m][0][1] * temp1;
                    }
                }
                else if(mode == 8)
                {
                    // Moog 24db Lowpass
                    filtx[0] = filtInputs[l][0] - r * filty4[0];
                    filtx[1] = filtInputs[l][1] - r * filty4[1];
                    for(int i = 0; i < 2; ++i)
                    {
                        filty1[i] = filtx[i] * p + filtoldx[i] * p
                                    - k * filty1[i];
                        filty2[i] = filty1[i] * p + filtoldy1[i] * p
                                    - k * filty2[i];
                        filty3[i] = filty2[i] * p + filtoldy2[i] * p
                                    - k * filty3[i];
                        filty4[i] = filty3[i] * p + filtoldy3[i] * p
                                    - k * filty4[i];
                        filty4[i]    = filty4[i] - pow(filty4[i], 3) / 6.f;
                        filtoldx[i]  = filtx[i];
                        filtoldy1[i] = filty1[i];
                        filtoldy2[i] = filty2[i];
                        filtoldy3[i] = filty3[i];
                    }
                    {
                        const real_t temp1 = filtOutVol[l] * 0.01;
                        filtOutputs[l][0]  = filty4[0] * temp1;
                        filtOutputs[l][1]  = filty4[1] * temp1;
                    }
                }

                // Calculates Saturation.  It looks ugly, but the algorithm is
                // just y = x ^ ( 1 - saturation );
                {
                    const real_t temp1 = 1 - (filtSatu[l] * 0.01);
                    filtOutputs[l][0]  = pow(abs(filtOutputs[l][0]), temp1)
                                        * (filtOutputs[l][0] < 0 ? -1 : 1);
                    filtOutputs[l][1] = pow(abs(filtOutputs[l][1]), temp1)
                                        * (filtOutputs[l][1] < 0 ? -1 : 1);
                }

                // Balance knob wet
                filtOutputs[l][0]
                        *= filtBal[l] > 0 ? (100. - filtBal[l]) * 0.01 : 1.;
                filtOutputs[l][1]
                        *= filtBal[l] < 0 ? (100. + filtBal[l]) * 0.01 : 1.;

                // Wet
                {
                    const real_t temp1 = filtWetDry[l] * 0.01;
                    filtOutputs[l][0] *= temp1;
                    filtOutputs[l][1] *= temp1;
                }

                // Balance knob dry
                filtOutputs[l][0]
                        += (1.
                            - (filtBal[l] > 0 ? (100. - filtBal[l]) * 0.01
                                              : 1.))
                           * filtInputs[l][0];
                filtOutputs[l][1]
                        += (1.
                            - (filtBal[l] < 0 ? (100. + filtBal[l]) * 0.01
                                              : 1.))
                           * filtInputs[l][1];

                // Dry
                {
                    const real_t temp1 = 1. - (filtWetDry[l] * 0.01);
                    filtOutputs[l][0] += temp1 * filtInputs[l][0];
                    filtOutputs[l][1] += temp1 * filtInputs[l][1];
                }

                // Send back past samples
                // I'm sorry you had to read this
                filtPrevSampIn[l][m][4][0] = filtPrevSampIn[l][m][3][0];
                filtPrevSampIn[l][m][4][1] = filtPrevSampIn[l][m][3][1];

                filtPrevSampIn[l][m][3][0] = filtPrevSampIn[l][m][2][0];
                filtPrevSampIn[l][m][3][1] = filtPrevSampIn[l][m][2][1];

                filtPrevSampIn[l][m][2][0] = filtPrevSampIn[l][m][1][0];
                filtPrevSampIn[l][m][2][1] = filtPrevSampIn[l][m][1][1];

                filtPrevSampIn[l][m][1][0] = filtInputs[l][0];
                filtPrevSampIn[l][m][1][1] = filtInputs[l][1];

                filtPrevSampOut[l][m][4][0] = filtPrevSampOut[l][m][3][0];
                filtPrevSampOut[l][m][4][1] = filtPrevSampOut[l][m][3][1];

                filtPrevSampOut[l][m][3][0] = filtPrevSampOut[l][m][2][0];
                filtPrevSampOut[l][m][3][1] = filtPrevSampOut[l][m][2][1];

                filtPrevSampOut[l][m][2][0] = filtPrevSampOut[l][m][1][0];
                filtPrevSampOut[l][m][2][1] = filtPrevSampOut[l][m][1][1];

                filtPrevSampOut[l][m][1][0] = filtPrevSampOut[l][m][0][0];
                filtPrevSampOut[l][m][1][1] = filtPrevSampOut[l][m][0][1];
            }

            {
                const real_t temp1 = filtFeedback[l] * 0.01;
                filtDelayBuf[l][0].insert(filtDelayBuf[l][0].begin(),
                                          filtOutputs[l][0] * temp1);
                filtDelayBuf[l][1].insert(filtDelayBuf[l][1].begin(),
                                          filtOutputs[l][1] * temp1);
            }

            filtInputs[l][0] = 0;
            filtInputs[l][1] = 0;

            filtModOutputs[l][0] = filtOutputs[l][0];
            filtModOutputs[l][1] = filtOutputs[l][1];

            if(filtMuted[l])
            {
                filtOutputs[l][0] = 0;
                filtOutputs[l][1] = 0;
            }
        }
    }

    //=====================//
    //== MAIN OSCILLATOR ==//
    //=====================//

    for(int i = 0; i < maxMainEnabled; ++i)
    // maxMainEnabled keeps this from looping 8 times every sample,
    // saving some CPU
    {
        if(!enabled[i])
            continue;  // Skip to next loop if oscillator is not enabled

        for(int l = 0; l < unisonVoices[i]; ++l)
        {
            sample_morerealindex[i][l]
                    = fmod((sample_realindex[i][l]
                            + (fmod(phase[i], 100.) * sampLen[i] * 0.01)),
                           sampLen[i]);  // Calculates phase

            // unisonVoices[i] - 1 is needed many times, which
            // is why unisonVoicesMinusOne exists
            const int32_t unisonVoicesMinusOne = unisonVoices[i] - 1;

            // Calculates frequency depending on detune and unison detune
            noteFreq
                    = unisonVoicesMinusOne ? detuneWithCents(
                              nph->frequency(),
                              unisonDetuneAmounts[i][l] * unisonDetune[i]
                                      + detuneVal[i])
                                           : detuneWithCents(nph->frequency(),
                                                             detuneVal[i]);

            sample_step[i][l] = sampLen[i] * (noteFreq / sample_rate);

            // Figures out Morph and Modify values for individual unison
            // voices
            if(unisonVoicesMinusOne > 0)
            {
                if(unisonMorph[i] != 0.)
                {
                    const real_t temp1 = unisonMorph[i]
                                         * real_t(unisonVoicesMinusOne - l)
                                         / real_t(unisonVoicesMinusOne);

                    morphVal[i] = bound(
                            0., temp1 - (unisonMorph[i] * 0.5) + morphVal[i],
                            morphMaxVal[i]);
                }

                if(unisonModify[i] != 0.)
                {
                    const real_t temp1 = unisonModify[i]
                                         * real_t(unisonVoicesMinusOne - l)
                                         / real_t(unisonVoicesMinusOne);

                    modifyVal[i] = bound(
                            0.,
                            temp1 - (unisonModify[i] * 0.5) + modifyVal[i],
                            sampLen[i] - 1.);  // SampleLength - 1 = ModifyMax
                }
            }

            sample_length_modify[i][l] = 0;
            // Horrifying formulas for the various Modify Modes
            switch(modifyModeVal[i])
            {
                case 0:  // None
                {
                    break;
                }
                case 1:  // Pulse Width
                {
                    sample_morerealindex[i][l]
                            /= (-modifyVal[i] + sampLen[i]) / sampLen[i];

                    // Keeps sample index within bounds
                    sample_morerealindex[i][l] = bound(
                            0., sample_morerealindex[i][l],
                            sampLen[i] + sample_length_modify[i][l] - 1);
                    break;
                }
                case 2:  // Weird 1
                {
                    sample_morerealindex[i][l]
                            = (((sin((((sample_morerealindex[i][l])
                                       / (sampLen[i]))
                                      * (modifyVal[i] / 50.))
                                     / 2))
                                * (sampLen[i]))
                                       * (modifyVal[i] / sampLen[i])
                               + (sample_morerealindex[i][l]
                                  + ((-modifyVal[i] + sampLen[i])
                                     / sampLen[i])))
                              / 2.;
                    break;
                }
                case 3:  // Weird 2
                {
                    if(sample_morerealindex[i][l] > sampLen[i] / 2.f)
                    {
                        sample_morerealindex[i][l]
                                = pow((((sample_morerealindex[i][l])
                                        / sampLen[i])),
                                      (modifyVal[i] * 10 / sampLen[i]) + 1)
                                  * (sampLen[i]);
                    }
                    else
                    {
                        sample_morerealindex[i][l]
                                = -sample_morerealindex[i][l] + sampLen[i];
                        sample_morerealindex[i][l]
                                = pow((((sample_morerealindex[i][l])
                                        / sampLen[i])),
                                      (modifyVal[i] * 10 / sampLen[i]) + 1)
                                  * (sampLen[i]);
                        sample_morerealindex[i][l]
                                = sample_morerealindex[i][l]
                                  - sampLen[i] / 2.f;
                    }
                    break;
                }
                case 4:  // Asym To Right
                {
                    sample_morerealindex[i][l]
                            = pow((((sample_morerealindex[i][l])
                                    / sampLen[i])),
                                  (modifyVal[i] * 10 / sampLen[i]) + 1)
                              * (sampLen[i]);
                    break;
                }
                case 5:  // Asym To Left
                {
                    sample_morerealindex[i][l]
                            = -sample_morerealindex[i][l] + sampLen[i];
                    sample_morerealindex[i][l]
                            = pow((((sample_morerealindex[i][l])
                                    / sampLen[i])),
                                  (modifyVal[i] * 10 / sampLen[i]) + 1)
                              * (sampLen[i]);
                    sample_morerealindex[i][l]
                            = -sample_morerealindex[i][l] + sampLen[i];
                    break;
                }
                case 6:  // Stretch From Center
                {
                    sample_morerealindex[i][l] -= sampLen[i] / 2.f;
                    sample_morerealindex[i][l] /= sampLen[i] / 2.f;
                    sample_morerealindex[i][l]
                            = (sample_morerealindex[i][l] >= 0)
                                      ? pow(sample_morerealindex[i][l],
                                            1
                                                    / ((modifyVal[i] * 4)
                                                               / sampLen[i]
                                                       + 1))
                                      : -pow(-sample_morerealindex[i][l],
                                             1
                                                     / ((modifyVal[i] * 4)
                                                                / sampLen[i]
                                                        + 1));
                    sample_morerealindex[i][l] *= (sampLen[i] / 2.f);
                    sample_morerealindex[i][l] += sampLen[i] / 2.f;
                    break;
                }
                case 7:  // Squish To Center
                {
                    sample_morerealindex[i][l] -= sampLen[i] / 2.f;
                    sample_morerealindex[i][l] /= (sampLen[i] / 2.f);
                    sample_morerealindex[i][l]
                            = (sample_morerealindex[i][l] >= 0)
                                      ? pow(sample_morerealindex[i][l],
                                            1
                                                    / (-modifyVal[i]
                                                               / sampLen[i]
                                                       + 1))
                                      : -pow(-sample_morerealindex[i][l],
                                             1
                                                     / (-modifyVal[i]
                                                                / sampLen[i]
                                                        + 1));
                    sample_morerealindex[i][l] *= (sampLen[i] / 2.f);
                    sample_morerealindex[i][l] += sampLen[i] / 2.f;
                    break;
                }
                case 8:  // Stretch And Squish
                {
                    sample_morerealindex[i][l] -= sampLen[i] / 2.f;
                    sample_morerealindex[i][l] /= (sampLen[i] / 2.f);
                    sample_morerealindex[i][l]
                            = (sample_morerealindex[i][l] >= 0)
                                      ? pow(sample_morerealindex[i][l],
                                            1
                                                    / ((modifyVal[i] * 4)
                                                       / sampLen[i]))
                                      : -pow(-sample_morerealindex[i][l],
                                             1
                                                     / ((modifyVal[i] * 4)
                                                        / sampLen[i]));
                    sample_morerealindex[i][l] *= (sampLen[i] / 2.f);
                    sample_morerealindex[i][l] += sampLen[i] / 2.f;
                    break;
                }
                case 9:  // Cut Off Right
                {
                    sample_morerealindex[i][l]
                            *= (-modifyVal[i] + sampLen[i]) / sampLen[i];
                    break;
                }
                case 10:  // Cut Off Left
                {
                    sample_morerealindex[i][l]
                            = -sample_morerealindex[i][l] + sampLen[i];
                    sample_morerealindex[i][l]
                            *= (-modifyVal[i] + sampLen[i]) / sampLen[i];
                    sample_morerealindex[i][l]
                            = -sample_morerealindex[i][l] + sampLen[i];
                    break;
                }
                default:
                {
                }
            }
            // sample_morerealindex[i][l] = qBound( 0.,
            // sample_morerealindex[i][l], sampLen[i] +
            // sample_length_modify[i][l] - 1);// Keeps sample index within
            // bounds

            sample[i][l] = 0;

            loopStart = qMax(0., morphVal[i] - rangeVal[i]) + 1.;
            loopEnd   = qMin(morphVal[i] + rangeVal[i],
                           real_t(MAINWAV_SIZE) / sampLen[i])
                      + 1.;
            currentRangeValInvert
                    = 1. / rangeVal[i];  // Prevents repetitive division in
                                         // the loop below.
            currentSampLen = sampLen[i];
            currentIndex   = sample_morerealindex[i][l];
            if(modifyModeVal[i] != 15
               && modifyModeVal[i]
                          != 16)  // If NOT Squarify or Pulsify Modify Mode
            {
                // Only grab samples from the waveforms when they will be
                // used, depending on the Range knob
                for(int j = loopStart; j < loopEnd; ++j)
                {
                    // Get waveform samples, set their volumes depending on
                    // Range knob
                    sample[i][l]
                            += waveforms[i][currentIndex + j * currentSampLen]
                               * (1
                                  - (abs(morphVal[i] - j)
                                     * currentRangeValInvert));
                }
            }
            else if(modifyModeVal[i] == 15)  // If Squarify Modify Mode
            {
                // Self-made formula, may be buggy.  Crossfade one half of
                // waveform with the inverse of the other. Some major CPU
                // improvements can be made here.
                for(int j = loopStart; j < loopEnd; ++j)
                {
                    sample[i][l]
                            += (waveforms[i]
                                         [currentIndex + j * currentSampLen]
                                * (1
                                   - (abs(morphVal[i] - j)
                                      * currentRangeValInvert)))
                               +  // Normal
                               ((-waveforms[i][((currentIndex
                                                 + (currentSampLen / 2))
                                                % currentSampLen)
                                               + j * currentSampLen]
                                 * (1
                                    - (abs(morphVal[i] - j)
                                       * currentRangeValInvert)))
                                * ((modifyVal[i] * 2.f) / currentSampLen))
                                       /  // Inverted other half of waveform
                                       ((modifyVal[i] / currentSampLen)
                                        + 1);  // Volume compensation
                }
            }
            else if(modifyModeVal[i] == 16)  // Pulsify Mode
            {
                // Self-made formula, may be buggy.  Crossfade one side of
                // waveform with the inverse of the other. Some major CPU
                // improvements can be made here.
                for(int j = loopStart; j < loopEnd; ++j)
                {
                    sample[i][l]
                            += (waveforms[i]
                                         [currentIndex + j * currentSampLen]
                                * (1
                                   - (abs(morphVal[i] - j)
                                      * currentRangeValInvert)))
                               +  // Normal
                               ((-waveforms[i][((currentIndex
                                                 + int(currentSampLen
                                                       * (modifyVal[i]
                                                          / currentSampLen)))
                                                % currentSampLen)
                                               + j * currentSampLen]
                                 * (1
                                    - (abs(morphVal[i] - j)
                                       * currentRangeValInvert)))
                                * 2.) /     // Inverted other side of waveform
                                       2.;  // Volume compensation
                }
            }

            if(rangeVal[i] - 1)
            {
                sample[i][l]
                        /= rangeVal[i];  // Decreases volume so Range value
                                         // doesn't make things too loud
            }

            switch(modifyModeVal[i])  // More horrifying formulas for the
                                      // various Modify Modes
            {
                case 1:  // Pulse Width
                {
                    if(sample_realindex[i][l]
                               / ((-modifyVal[i] + sampLen[i]) / sampLen[i])
                       > sampLen[i])
                    {
                        sample[i][l] = 0;
                    }
                    break;
                }
                case 13:  // Flip
                {
                    if(sample_realindex[i][l] > modifyVal[i])
                    {
                        sample[i][l] *= -1;
                    }
                    break;
                }
                case 14:  // Clip
                {
                    const real_t temp1 = (modifyVal[i] / (sampLen[i] - 1));
                    sample[i][l]       = bound(-temp1, sample[i][l], temp1);
                    break;
                }
                case 15:  // Inverse Clip
                {
                    const real_t temp1 = (modifyVal[i] / (sampLen[i] - 1));
                    if(sample[i][l] > 0 && sample[i][l] < temp1)
                    {
                        sample[i][l] = temp1;
                    }
                    else if(sample[i][l] < 0 && sample[i][l] > -temp1)
                    {
                        sample[i][l] = -temp1;
                    }
                    break;
                }
                case 16:  // Sine
                {
                    // const real_t temp1 = (modifyVal[i] / (sampLen[i] - 1));
                    sample[i][l] = sin(sample[i][l] * F_PI);
                    break;
                }
                case 17:  // Atan
                {
                    // const real_t temp1 = (modifyVal[i] / (sampLen[i] - 1));
                    sample[i][l] = atan(sample[i][l] * F_PI);
                    break;
                }
                default:
                {
                }
            }

            sample_realindex[i][l] += sample_step[i][l];

            // check overflow
            {
                const real_t temp2 = sampLen[i] + sample_length_modify[i][l];
                while(sample_realindex[i][l] >= temp2)  // was temp1!!!
                {
                    sample_realindex[i][l] -= temp2;
                    lastMainOscEnvDone[l] = true;
                }
            }

            sample[i][l] = sample[i][l] * vol[i] * 0.01;
        }
    }

    //====================//
    //== SUB OSCILLATOR ==//
    //====================//

    StereoSample subsample[NB_SUBOSC];  // = {{0}};
    // maxSubEnabled keeps this from looping 64 times every sample,
    // saving a lot of CPU
    for(int l = 0; l < maxSubEnabled; ++l)
    {
        if(subEnabled[l])
        {
            if(!subNoise[l])
            {
                if(subTempo[l])
                {
                    noteFreq = subKeytrack[l]
                                       ? detuneWithCents(nph->frequency(),
                                                         subDetune[l])
                                                 / 26400. * subTempo[l]
                                       : detuneWithCents(440., subDetune[l])
                                                 / 26400. * subTempo[l];
                }
                else
                {
                    noteFreq = subKeytrack[l]
                                       ? detuneWithCents(nph->frequency(),
                                                         subDetune[l])
                                       : detuneWithCents(440., subDetune[l]);
                }
                sample_step_sub = subSampLen[l] / (sample_rate / noteFreq);

                Q_ASSERT(subSampLen[l] > 0);
                subsample[l][0] = (subVol[l] * 0.01)
                                  * subs[(int(sample_subindex[l]
                                              + (subPhase[l] * subSampLen[l]))
                                          % subSampLen[l])
                                         + (2048 * l)];
                subsample[l][1] = subsample[l][0];

                if(subPanning[l] < 0)
                {
                    subsample[l][1] *= (100. + subPanning[l]) * 0.01;
                }
                else if(subPanning[l] > 0)
                {
                    subsample[l][0] *= (100. - subPanning[l]) * 0.01;
                }

                /* That is all that is needed for the sub oscillator
                calculations.

                (To the tune of Hallelujah)

                        There was a Happy CPU
                No processing power to chew through
                In this wonderful synthesis brew
                        Hallelujah

                        But with some wavetable synthesis
                Your CPU just blows a kiss
                And leaves you there despite your miss
                        Hallelujah

                Hallelujah, Hallelujah, Hallelujah, Halleluuu-uuuuuuuu-uuuujah

                        Your music ambition lays there, dead
                Can't get your ideas out of your head
                Because your computer's slower than lead
                        Hallelujah

                        Sometimes you may try and try
                To keep your ping from saying goodbye
                Leaving you to die and cry
                        Hallelujah

                Hallelujah, Hallelujah, Hallelujah, Halleluuu-uuuuuuuu-uuuujah

                        But what is this, an alternative?
                Sub oscillators supported native
                To prevent CPU obliteratives
                        Hallelujah

                        Your music has come back to life
                CPU problems cut off like a knife
                Sub oscillators removing your strife
                        Hallelujah

                Hallelujah, Hallelujah, Hallelujah, Halleluuu-uuuuuuuu-uuuujah

                *cool outro*

                */

                lastSubVal[l][0] = subsample[l][0];  // Store value for matrix
                lastSubVal[l][1] = subsample[l][1];

                if(!lastSubEnvDone[l])
                {
                    lastSubEnvVal[l][0]
                            = subsample[l][0];  // Store envelope value for
                                                // matrix
                    lastSubEnvVal[l][1] = subsample[l][1];
                }

                // Mutes sub after saving value for modulation if the muted
                // option is on
                if(subMuted[l])
                {
                    subsample[l][0] = 0;
                    subsample[l][1] = 0;
                }

                sample_subindex[l] += sample_step_sub;

                // move waveform position back if it passed the end of the
                // waveform
                while(sample_subindex[l] >= subSampLen[l])
                {
                    sample_subindex[l] -= subSampLen[l];
                    lastSubEnvDone[l] = true;
                }
            }
            else  // sub oscillator is noise
            {
                noiseSampRand = fastrand(subSampLen[l]) - 1;

                subsample[l][0]
                        = bound(-1.,
                                (lastSubVal[l][0]
                                 + (subs[int(noiseSampRand)] / 8.))
                                        / 1.2f,
                                1.)
                          * (subVol[l]
                             * 0.01);  // Division by 1.2f to tame DC offset
                subsample[l][1] = subsample[l][0];

                lastSubVal[l][0] = subsample[l][0];
                lastSubVal[l][1] = subsample[l][1];

                lastSubEnvVal[l][0]
                        = subsample[l][0];  // Store envelope value for matrix
                lastSubEnvVal[l][1] = subsample[l][1];

                // Mutes sub after saving value for modulation if the muted
                // option is on
                if(subMuted[l])
                {
                    subsample[l][0] = 0;
                    subsample[l][1] = 0;
                }
            }
        }
        else
        {
            lastSubVal[l][0]    = 0;
            lastSubVal[l][1]    = 0;
            lastSubEnvVal[l][0] = 0;
            lastSubEnvVal[l][1] = 0;
        }
    }

    //=======================//
    //== SAMPLE OSCILLATOR ==//
    //=======================//

    sample_step_sample = 0;
    StereoSample samplesample[NB_SMPLR];  // = {{0}};

    // maxSampleEnabled keeps this from looping 8 times every
    // sample, saving some CPU
    for(int l = 0; l < maxSampleEnabled; ++l)
    {
        int32_t sampleSize = samples[l][0].size() * sampleEnd[l];
        if(sampleEnabled[l]
           && (sample_sampleindex[l] < sampleSize || sampleLoop[l]))
        {
            if(sampleLoop[l])
            {
                if(sample_sampleindex[l] > sampleSize)
                {
                    sample_sampleindex[l]
                            = sample_sampleindex[l] - sampleSize
                              + (samples[l][0].size() * sampleStart[l]);
                    lastSampleEnvDone[l] = true;
                }
            }

            if(sampleKeytracking[l])
            {
                sample_step_sample
                        = (detuneWithCents(nph->frequency(), sampleDetune[l])
                           / 440.)
                          * 44100. / sample_rate;
            }
            else
            {
                sample_step_sample = 44100. / sample_rate;
            }

            if(sampleGraphEnabled[l])
            {
                const real_t progress
                        = fmod((sample_sampleindex[l]
                                + (samplePhase[l] * sampleSize * 0.01)),
                               sampleSize)
                          / sampleSize * 128.;

                const int32_t intprogress = int(progress);

                const real_t temp1 = fmod(progress, 1);
                const real_t progress2
                        = sampGraphs[intprogress] * (1 - temp1);

                real_t progress3;
                if(intprogress + 1 < 128)
                {
                    progress3 = sampGraphs[intprogress + 1] * temp1;
                }
                else
                {
                    progress3 = sampGraphs[intprogress] * temp1;
                }

                const int32_t temp2 = int(((progress2 + progress3 + 1) * 0.5)
                                          * sampleSize);
                samplesample[l][0]  = samples[l][0][temp2];
                samplesample[l][1]  = samples[l][1][temp2];
            }
            else
            {
                const real_t temp1
                        = fmod((sample_sampleindex[l]
                                + (samplePhase[l] * sampleSize * 0.01)),
                               sampleSize);
                samplesample[l][0] = samples[l][0][temp1];
                samplesample[l][1] = samples[l][1][temp1];
            }

            {
                const real_t temp3 = sampleVolume[l] * 0.01;
                samplesample[l][0] *= temp3;
                samplesample[l][1] *= temp3;
            }

            if(samplePanning[l] < 0)
            {
                samplesample[l][1] *= (100. + samplePanning[l]) * 0.01;
            }
            else if(samplePanning[l] > 0)
            {
                samplesample[l][0] *= (100. - samplePanning[l]) * 0.01;
            }

            lastSampleVal[l][0]
                    = samplesample[l][0];  // Store value for modulation
            lastSampleVal[l][1]
                    = samplesample[l][1];  // Store value for modulation

            if(!lastSampleEnvDone[l])
            {
                lastSampleEnvVal[l][0] = lastSampleVal[l][0];
                lastSampleEnvVal[l][1] = lastSampleVal[l][1];
            }

            if(sampleMuted[l])
            {
                samplesample[l][0] = 0;
                samplesample[l][1] = 0;
            }

            sample_sampleindex[l] += sample_step_sample;
        }
        else
        {
            lastSampleVal[l][0] = 0;
            lastSampleVal[l][1] = 0;
        }
    }

    // QVector<sample_t> sampleAvg(2);
    // QVector<sample_t> sampleMainOsc(2);
    StereoSample sampleAvg;
    StereoSample sampleMainOsc;
    // Main Oscillator outputs
    for(int i = 0; i < maxMainEnabled; ++i)
    {
        if(enabled[i])
        {

            if(!muted[i])
            {
                if(unisonVoices[i] > 1)
                {
                    const int32_t unisonVoicesMinusOne = unisonVoices[i] - 1;

                    sampleMainOsc[0] = 0;
                    sampleMainOsc[1] = 0;
                    for(int j = 0; j < unisonVoices[i]; ++j)
                    {
                        // Pan unison voices
                        sampleMainOsc[0] += sample[i][j]
                                            * real_t(unisonVoicesMinusOne - j)
                                            / real_t(unisonVoicesMinusOne);
                        sampleMainOsc[1] += sample[i][j] * real_t(j)
                                            / real_t(unisonVoicesMinusOne);
                    }
                    // Decrease volume so more unison voices won't increase
                    // volume too much
                    {
                        const real_t temp1 = 0.5 * unisonVoices[i];
                        sampleMainOsc[0] /= temp1;
                        sampleMainOsc[1] /= temp1;
                    }

                    sampleAvg[0] += sampleMainOsc[0];
                    sampleAvg[1] += sampleMainOsc[1];

                    lastMainOscVal[i][0]
                            += sampleMainOsc[0];  // Store results for
                                                  // modulations
                    lastMainOscVal[i][1] += sampleMainOsc[1];

                    if(!lastMainOscEnvDone[i])
                    {
                        lastMainOscEnvVal[i][0] = lastMainOscVal[i][0];
                        lastMainOscEnvVal[i][1] = lastMainOscVal[i][1];
                    }
                }
                else
                {
                    sampleAvg[0] += sample[i][0];
                    sampleAvg[1] += sample[i][0];
                }
            }
        }
    }

    // Sub Oscillator outputs
    // maxSubEnabled keeps this from looping NB_SUBOSC times every sample,
    // saving a lot of CPU
    for(int l = 0; l < maxSubEnabled; ++l)
    {
        if(subEnabled[l])
        {
            sampleAvg[0] += subsample[l][0];
            sampleAvg[1] += subsample[l][1];
        }
    }

    // Sample Oscillator outputs
    // maxSampleEnabled keeps this from looping 8 times every
    // sample, saving some CPU
    for(int l = 0; l < maxSampleEnabled; ++l)
    {
        if(sampleEnabled[l])
        {
            sampleAvg[0] += samplesample[l][0];
            sampleAvg[1] += samplesample[l][1];
        }
    }

    // Filter outputs
    // maxFiltEnabled keeps this from looping 8 times every sample,
    // saving some CPU
    for(int l = 0; l < maxFiltEnabled; ++l)
    {
        if(filtEnabled[l])
        {
            sampleAvg[0] += filtOutputs[l][0];
            sampleAvg[1] += filtOutputs[l][1];
        }
    }

    // Refresh all modulated values back to the value of the knob.
    /*
    for(int i = 0; i < modValType.size(); ++i)
    {
        refreshValue(modValType[0], modValNum[0]);  // Possible CPU problem
        modValType.erase(modValType.begin());
        modValNum.erase(modValNum.begin());
    }
    */
    for(int i = modValType.size() - 1; i >= 0; i--)
        refreshValue(modValType[i], modValNum[i]);
    modValType.clear();
    modValNum.clear();

    return sampleAvg;
}

// Takes input of original Hz and the number of cents to detune it by, and
// returns the detuned result in Hz.
inline real_t MSynth::detuneWithCents(real_t pitchValue, int32_t detuneValue)
{
    // Avoids expensive exponentiation if no detuning is necessary
    if(detuneValue != 0)
        return pitchValue * std::exp2(real_t(detuneValue) / 1200.);
    else
        return pitchValue;
}

// At the end of MSynth::nextStringSample, this will refresh all modulated
// values back to the value of the knob.
void MSynth::refreshValue(int32_t which, int32_t num)
{
    Microwave* mwc
            = dynamic_cast<Microwave*>(nph->instrumentTrack()->instrument());

    if(mwc == nullptr)
    {
        qWarning("MSynth::refreshValue mwc is null");
        return;
    }

    switch(which)
    {
        case 1:
            morphVal[num] = mwc->morph[num]->value();
            break;
        case 2:
            rangeVal[num] = mwc->range[num]->value();
            break;
        case 3:
            modifyVal[num] = mwc->modify[num]->value();
            break;
        case 4:
            modifyModeVal[num] = mwc->modifyMode[num]->value();
            break;
        case 6:
            unisonVoices[num] = mwc->unisonVoices[num]->value();
            break;
        case 7:
            unisonDetune[num] = mwc->unisonDetune[num]->value();
            break;
        case 8:
            unisonMorph[num] = mwc->unisonMorph[num]->value();
            break;
        case 9:
            unisonModify[num] = mwc->unisonModify[num]->value();
            break;
        case 10:
            morphMaxVal[num] = mwc->morphMax[num]->value();
            break;
        case 11:
            detuneVal[num] = mwc->detune[num]->value();
            break;
        case 12:
            sampLen[num] = mwc->sampLen[num]->value();
            break;
        case 13:
            phase[num] = mwc->phase[num]->value();
            break;
        case 14:
            vol[num] = mwc->vol[num]->value();
            break;
        case 15:
            filtInVol[num] = mwc->filtInVol[num]->value();
            break;
        case 16:
            filtType[num] = mwc->filtType[num]->value();
            break;
        case 17:
            filtSlope[num] = mwc->filtSlope[num]->value();
            break;
        case 18:
            filtCutoff[num] = mwc->filtCutoff[num]->value();
            break;
        case 19:
            filtReso[num] = mwc->filtReso[num]->value();
            break;
        case 20:
            filtGain[num] = mwc->filtGain[num]->value();
            break;
        case 21:
            filtSatu[num] = mwc->filtSatu[num]->value();
            break;
        case 22:
            filtWetDry[num] = mwc->filtWetDry[num]->value();
            break;
        case 23:
            filtBal[num] = mwc->filtBal[num]->value();
            break;
        case 24:
            filtOutVol[num] = mwc->filtOutVol[num]->value();
            break;
        case 25:
            filtEnabled[num] = mwc->filtEnabled[num]->value();
            break;
        case 26:
            subEnabled[num] = mwc->subEnabled[num]->value();
            break;
        case 27:
            subVol[num] = mwc->subVol[num]->value();
            break;
        case 28:
            subPhase[num] = mwc->subPhase[num]->value();
            break;
        case 29:
            subPhaseRand[num] = mwc->subPhaseRand[num]->value();
            break;
        case 30:
            subDetune[num] = mwc->subDetune[num]->value();
            break;
        case 31:
            subMuted[num] = mwc->subMuted[num]->value();
            break;
        case 32:
            subKeytrack[num] = mwc->subKeytrack[num]->value();
            break;
        case 33:
            subSampLen[num] = mwc->subSampLen[num]->value();
            break;
        case 34:
            subNoise[num] = mwc->subNoise[num]->value();
            break;
        case 35:
            modIn[num] = mwc->modIn[num]->value();
            break;
        case 36:
            modInNum[num] = mwc->modInNum[num]->value();
            break;
        case 37:
            modInOtherNum[num] = mwc->modInOtherNum[num]->value();
            break;
        case 38:
            modInAmnt[num] = mwc->modInAmnt[num]->value();
            break;
        case 39:
            modInCurve[num] = mwc->modInCurve[num]->value();
            break;
        case 40:
            modIn2[num] = mwc->modIn2[num]->value();
            break;
        case 41:
            modInNum2[num] = mwc->modInNum2[num]->value();
            break;
        case 42:
            modInOtherNum2[num] = mwc->modInOtherNum2[num]->value();
            break;
        case 43:
            modInAmnt2[num] = mwc->modInAmnt2[num]->value();
            break;
        case 44:
            modInCurve2[num] = mwc->modInCurve2[num]->value();
            break;
        case 45:
            modOutSec[num] = mwc->modOutSec[num]->value();
            break;
        case 46:
            modOutSig[num] = mwc->modOutSig[num]->value();
            break;
        case 47:
            modOutSecNum[num] = mwc->modOutSecNum[num]->value();
            break;
        case 48:
            enabled[num] = mwc->enabled[num]->value();
            break;
        case 49:
            modEnabled[num] = mwc->modEnabled[num]->value();
            break;
        case 50:
            modCombineType[num] = mwc->modCombineType[num]->value();
            break;
        case 57:
            muted[num] = mwc->muted[num]->value();
            break;
        case 59:
            sampleEnabled[num] = mwc->sampleEnabled[num]->value();
            break;
        case 60:
            sampleGraphEnabled[num] = mwc->sampleGraphEnabled[num]->value();
            break;
        case 61:
            sampleMuted[num] = mwc->sampleMuted[num]->value();
            break;
        case 62:
            sampleKeytracking[num] = mwc->sampleKeytracking[num]->value();
            break;
        case 63:
            sampleLoop[num] = mwc->sampleLoop[num]->value();
            break;
        case 64:
            sampleVolume[num] = mwc->sampleVolume[num]->value();
            break;
        case 65:
            samplePanning[num] = mwc->samplePanning[num]->value();
            break;
        case 66:
            sampleDetune[num] = mwc->sampleDetune[num]->value();
            break;
        case 67:
            samplePhase[num] = mwc->samplePhase[num]->value();
            break;
        case 68:
            samplePhaseRand[num] = mwc->samplePhaseRand[num]->value();
            break;
        case 69:
            filtFeedback[num] = mwc->filtFeedback[num]->value();
            break;
        case 70:
            filtDetune[num] = mwc->filtDetune[num]->value();
            break;
        case 71:
            filtKeytracking[num] = mwc->filtKeytracking[num]->value();
            break;
        case 72:
            subPanning[num] = mwc->subPanning[num]->value();
            break;
        case 73:
            sampleStart[num] = mwc->sampleStart[num]->value();
            break;
        case 74:
            sampleEnd[num] = mwc->sampleEnd[num]->value();
            break;
        case 75:
            pan[num] = mwc->pan[num]->value();
            break;
        case 76:
            subTempo[num] = mwc->subTempo[num]->value();
            break;
        case 77:
            modType[num] = mwc->modType[num]->value();
            break;
        case 78:
            macro[num] = mwc->macro[num]->value();
            break;
        case 79:
            filtMuted[num] = mwc->filtMuted[num]->value();
            break;
        case 80:
            phaseRand[num] = mwc->phaseRand[num]->value();
            break;
    }
}
