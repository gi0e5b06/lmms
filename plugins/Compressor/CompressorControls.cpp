/*
 * CompressorControls.cpp
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

#include "CompressorControls.h"

#include "Compressor.h"
#include "Engine.h"
#include "Song.h"

#include <QDomElement>

CompressorControls::CompressorControls(CompressorEffect* effect) :
      EffectControls(effect), m_effect(effect),
      m_thresholdModel(-8., -60., 0., 0.001, this, tr("Threshold")),
      m_ratioModel(1.8, 1., 20., 0.001, this, tr("Ratio")),
      m_attackModel(0.25, 0.005, 250., 0.001, this, tr("Attack")),
      m_releaseModel(100., 1., 2500., 0.001, this, tr("Release")),
      m_kneeModel(12., 0., 96., 0.01, this, tr("Knee")),
      m_holdModel(0., 0., 500., 0.01, this, tr("Hold")),
      m_rangeModel(-240., -240., 0., 0.01, this, tr("Range")),
      m_rmsModel(64., 1., 2048., 1., this, tr("RMS Size")),
      m_midsideModel(0., 0., 1., this, tr("Mid/Side")),
      m_peakmodeModel(0., 0., 1., this, tr("Peak Mode")),
      m_lookaheadLengthModel(
              0., 0., 20., 0.0001, this, tr("Lookahead Length")),
      m_inBalanceModel(0., -1., 1., 0.0001, this, tr("Input Balance")),
      m_outBalanceModel(
              0., -1., 1., 0.0001, this, tr("Output Balance")),
      m_limiterModel(0., 0., 1., this, tr("Limiter")),
      m_outGainModel(0., -60., 30., 0.01, this, tr("Output Gain")),
      m_inGainModel(0., -60., 30., 0.01, this, tr("Input Gain")),
      m_blendModel(1., 0., 3., 0.0001, this, tr("Blend")),
      m_stereoBalanceModel(
              0., -1., 1., 0.0001, this, tr("Stereo Balance")),
      m_autoMakeupModel(false, this, tr("Auto Makeup Gain")),
      m_auditionModel(false, this, tr("Audition")),
      m_feedbackModel(false, this, tr("Feedback")),
      m_autoAttackModel(0., 0., 100., 0.01, this, tr("Auto Attack")),
      m_autoReleaseModel(0., 0., 100., 0.01, this, tr("Auto Release")),
      m_lookaheadModel(false, this, tr("Lookahead")),
      m_tiltModel(0., -6., 6., 0.0001, this, tr("Tilt")),
      m_tiltFreqModel(
              150., 20., 20000., 0.1, this, tr("Tilt Frequency")),
      m_stereoLinkModel(1., 0., 4., this, tr("Stereo Link")),
      m_mixModel(100., 0., 100., 0.01, this, tr("Mix"))
{
    m_ratioModel.setScaleLogarithmic(true);
    m_holdModel.setScaleLogarithmic(true);
    m_attackModel.setScaleLogarithmic(true);
    m_releaseModel.setScaleLogarithmic(true);
    m_thresholdModel.setScaleLogarithmic(true);
    m_rangeModel.setScaleLogarithmic(true);
    m_lookaheadLengthModel.setScaleLogarithmic(true);
    m_rmsModel.setScaleLogarithmic(true);
    m_kneeModel.setScaleLogarithmic(true);
    m_tiltFreqModel.setScaleLogarithmic(true);
    m_rangeModel.setScaleLogarithmic(true);
}

void CompressorControls::saveSettings(QDomDocument& doc, QDomElement& _this)
{
    m_thresholdModel.saveSettings(doc, _this, "threshold");
    m_ratioModel.saveSettings(doc, _this, "ratio");
    m_attackModel.saveSettings(doc, _this, "attack");
    m_releaseModel.saveSettings(doc, _this, "release");
    m_kneeModel.saveSettings(doc, _this, "knee");
    m_holdModel.saveSettings(doc, _this, "hold");
    m_rangeModel.saveSettings(doc, _this, "range");
    m_rmsModel.saveSettings(doc, _this, "rms");
    m_midsideModel.saveSettings(doc, _this, "midside");
    m_peakmodeModel.saveSettings(doc, _this, "peakmode");
    m_lookaheadLengthModel.saveSettings(doc, _this, "lookaheadLength");
    m_inBalanceModel.saveSettings(doc, _this, "inBalance");
    m_outBalanceModel.saveSettings(doc, _this, "outBalance");
    m_limiterModel.saveSettings(doc, _this, "limiter");
    m_outGainModel.saveSettings(doc, _this, "outGain");
    m_inGainModel.saveSettings(doc, _this, "inGain");
    m_blendModel.saveSettings(doc, _this, "blend");
    m_stereoBalanceModel.saveSettings(doc, _this, "stereoBalance");
    m_autoMakeupModel.saveSettings(doc, _this, "autoMakeup");
    m_auditionModel.saveSettings(doc, _this, "audition");
    m_feedbackModel.saveSettings(doc, _this, "feedback");
    m_autoAttackModel.saveSettings(doc, _this, "autoAttack");
    m_autoReleaseModel.saveSettings(doc, _this, "autoRelease");
    m_lookaheadModel.saveSettings(doc, _this, "lookahead");
    m_tiltModel.saveSettings(doc, _this, "tilt");
    m_tiltFreqModel.saveSettings(doc, _this, "tiltFreq");
    m_stereoLinkModel.saveSettings(doc, _this, "stereoLink");
    m_mixModel.saveSettings(doc, _this, "mix");
}

void CompressorControls::loadSettings(const QDomElement& _this)
{
    m_thresholdModel.loadSettings(_this, "threshold");
    m_ratioModel.loadSettings(_this, "ratio");
    m_attackModel.loadSettings(_this, "attack");
    m_releaseModel.loadSettings(_this, "release");
    m_kneeModel.loadSettings(_this, "knee");
    m_holdModel.loadSettings(_this, "hold");
    m_rangeModel.loadSettings(_this, "range");
    m_rmsModel.loadSettings(_this, "rms");
    m_midsideModel.loadSettings(_this, "midside");
    m_peakmodeModel.loadSettings(_this, "peakmode");
    m_lookaheadLengthModel.loadSettings(_this, "lookaheadLength");
    m_inBalanceModel.loadSettings(_this, "inBalance");
    m_outBalanceModel.loadSettings(_this, "outBalance");
    m_limiterModel.loadSettings(_this, "limiter");
    m_outGainModel.loadSettings(_this, "outGain");
    m_inGainModel.loadSettings(_this, "inGain");
    m_blendModel.loadSettings(_this, "blend");
    m_stereoBalanceModel.loadSettings(_this, "stereoBalance");
    m_autoMakeupModel.loadSettings(_this, "autoMakeup");
    m_auditionModel.loadSettings(_this, "audition");
    m_feedbackModel.loadSettings(_this, "feedback");
    m_autoAttackModel.loadSettings(_this, "autoAttack");
    m_autoReleaseModel.loadSettings(_this, "autoRelease");
    m_lookaheadModel.loadSettings(_this, "lookahead");
    m_tiltModel.loadSettings(_this, "tilt");
    m_tiltFreqModel.loadSettings(_this, "tiltFreq");
    m_stereoLinkModel.loadSettings(_this, "stereoLink");
    m_mixModel.loadSettings(_this, "mix");
}
