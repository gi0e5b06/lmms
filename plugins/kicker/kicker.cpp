/*
 * kicker.cpp - drum synthesizer
 *
 * Copyright (c) 2018      gi0e5b06 (on github.com)
 * Copyright (c) 2014      grejppi <grejppi/at/gmail.com>
 * Copyright (c) 2006-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "kicker.h"

#include "Engine.h"
#include "InstrumentTrack.h"
#include "KickerOsc.h"
#include "Knob.h"
#include "Mixer.h"
#include "NotePlayHandle.h"
#include "WaveFormStandard.h"
#include "embed.h"

#include <QDomDocument>
#include <QPainter>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT kicker_plugin_descriptor = {
            STRINGIFY(PLUGIN_NAME),
            "Kicker",
            QT_TRANSLATE_NOOP("pluginBrowser", "Versatile drum synthesizer"),
            "Tobias Doerffel <tobydox/at/users.sf.net>",
            0x0100,
            Plugin::Instrument,
            new PluginPixmapLoader("logo"),
            NULL,
            NULL};
}

kickerInstrument::kickerInstrument(InstrumentTrack* _instrument_track) :
      Instrument(_instrument_track, &kicker_plugin_descriptor),
      m_startFreqModel(150., 5., 1000., 1., this, tr("Start frequency")),
      m_endFreqModel(40., 5., 1000., 1., this, tr("End frequency")),
      m_decayModel(440., 5., 5000., 1., 5000., this, tr("Length")),
      m_distModel(0.8, 0., 100., 0.1, this, tr("Distortion Start")),
      m_distEndModel(0.8, 0., 100., 0.1, this, tr("Distortion End")),
      m_gainModel(1., 0.1, 5., 0.05, this, tr("Gain")),
      m_envModel(0.163, 0.01, 1., 0.001, this, tr("Envelope Slope")),
      m_tailModel(1., 0., 8., 0.01, this, tr("Tail")),
      m_noiseModel(0., 0., 1., 0.01, this, tr("Noise")),
      m_clickModel(0.4, 0., 1., 0.05, this, tr("Click")),
      m_slopeModel(0.06, 0.001, 1., 0.001, this, tr("Frequency Slope")),
      m_phaseFactorModel(1., 0.125, 8., 0.001, this, tr("Phase Factor")),
      m_startNoteModel(true, this, tr("Start from note")),
      m_endNoteModel(false, this, tr("End to note")),
      m_versionModel(KICKER_PRESET_VERSION,
                     0,
                     KICKER_PRESET_VERSION,
                     this,
                     "[kicker version]"),
      m_sinBankModel(this, "Bank 1"), m_sinIndexModel(this, "Index 1"),
      m_whnBankModel(this, "Bank 2"), m_whnIndexModel(this, "Index 2")
{
    WaveFormStandard::fillBankModel(m_sinBankModel);
    WaveFormStandard::fillBankModel(m_whnBankModel);
    WaveFormStandard::fillIndexModel(m_sinIndexModel, 0);
    WaveFormStandard::fillIndexModel(m_whnIndexModel, 0);

    connect(&m_sinBankModel, SIGNAL(dataChanged()), this,
            SLOT(updateSinIndexModel()));
    connect(&m_whnBankModel, SIGNAL(dataChanged()), this,
            SLOT(updateWhnIndexModel()));
}

kickerInstrument::~kickerInstrument()
{
}

void kickerInstrument::updateSinIndexModel()
{
    int bank = m_sinBankModel.value();
    int old  = m_sinIndexModel.value();
    WaveFormStandard::fillIndexModel(m_sinIndexModel, bank);
    m_sinIndexModel.setValue(old);
}

void kickerInstrument::updateWhnIndexModel()
{
    int bank = m_whnBankModel.value();
    int old  = m_whnIndexModel.value();
    WaveFormStandard::fillIndexModel(m_whnIndexModel, bank);
    m_whnIndexModel.setValue(old);
}

void kickerInstrument::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    m_startFreqModel.saveSettings(_doc, _this, "startfreq");
    m_endFreqModel.saveSettings(_doc, _this, "endfreq");
    m_decayModel.saveSettings(_doc, _this, "decay");
    m_distModel.saveSettings(_doc, _this, "dist");
    m_distEndModel.saveSettings(_doc, _this, "distend");
    m_gainModel.saveSettings(_doc, _this, "gain");
    m_envModel.saveSettings(_doc, _this, "env");
    m_tailModel.saveSettings(_doc, _this, "tail");
    m_noiseModel.saveSettings(_doc, _this, "noise");
    m_clickModel.saveSettings(_doc, _this, "click");
    m_slopeModel.saveSettings(_doc, _this, "slope");
    m_startNoteModel.saveSettings(_doc, _this, "startnote");
    m_endNoteModel.saveSettings(_doc, _this, "endnote");
    m_versionModel.saveSettings(_doc, _this, "version");

    m_sinBankModel.saveSettings(_doc, _this, "sin_bank");
    m_sinIndexModel.saveSettings(_doc, _this, "sin_index");
    m_whnBankModel.saveSettings(_doc, _this, "whn_bank");
    m_whnIndexModel.saveSettings(_doc, _this, "whn_index");
}

void kickerInstrument::loadSettings(const QDomElement& _this)
{
    m_versionModel.loadSettings(_this, "version");

    m_startFreqModel.loadSettings(_this, "startfreq");
    m_endFreqModel.loadSettings(_this, "endfreq");
    m_decayModel.loadSettings(_this, "decay");
    m_distModel.loadSettings(_this, "dist");

    if(_this.hasAttribute("distend"))
        m_distEndModel.loadSettings(_this, "distend");
    else
        m_distEndModel.setValue(m_distModel.value());

    m_gainModel.loadSettings(_this, "gain");
    m_envModel.loadSettings(_this, "env");

    if(_this.hasAttribute("tail"))
        m_tailModel.loadSettings(_this, "tail");
    else
        m_distEndModel.setValue(2.);

    m_noiseModel.loadSettings(_this, "noise");
    m_clickModel.loadSettings(_this, "click");
    m_slopeModel.loadSettings(_this, "slope");
    m_startNoteModel.loadSettings(_this, "startnote");
    if(m_versionModel.value() < 1)
    {
        m_startNoteModel.setValue(false);
    }
    m_endNoteModel.loadSettings(_this, "endnote");

    // Try to maintain backwards compatibility
    if(!_this.hasAttribute("version"))
    {
        m_startNoteModel.setValue(false);
        m_decayModel.setValue(m_decayModel.value() * 1.33);
        m_envModel.setValue(1.);
        m_slopeModel.setValue(1.);
        m_clickModel.setValue(0.);
    }

    m_sinBankModel.setInitValue(WaveFormStandard::SINE_BANK);
    m_sinIndexModel.setInitValue(WaveFormStandard::SINE_INDEX);
    m_whnBankModel.setInitValue(WaveFormStandard::WHITENOISE_BANK);
    m_whnIndexModel.setInitValue(WaveFormStandard::WHITENOISE_INDEX);
    m_sinBankModel.loadSettings(_this, "sin_bank");
    m_sinIndexModel.loadSettings(_this, "sin_index");
    m_whnBankModel.loadSettings(_this, "whn_bank");
    m_whnIndexModel.loadSettings(_this, "whn_index");

    m_versionModel.setValue(KICKER_PRESET_VERSION);
}

/*
QString kickerInstrument::nodeName() const
{
    return kicker_plugin_descriptor.name;
}
*/

typedef DspEffectLibrary::Distortion                             DistFX;
typedef KickerOsc<DspEffectLibrary::MonoToStereoAdaptor<DistFX>> SweepOsc;

void kickerInstrument::playNote(NotePlayHandle* _n,
                                sampleFrame*    _working_buffer)
{
    const fpp_t   frames = _n->framesLeftForCurrentPeriod();
    const f_cnt_t offset = _n->noteOffset();
    const real_t  decfr  = m_decayModel.value()
                         * Engine::mixer()->processingSampleRate() / 1000.;
    const f_cnt_t tfp = _n->totalFramesPlayed();

    if((tfp > decfr) && !_n->isReleased())
    {
        _n->noteOff();
    }
    else if(tfp == 0)  //||!_n->m_pluginData)
    {
        _n->m_pluginData = new SweepOsc(
                DistFX(m_distModel.value(), m_gainModel.value()),
                m_startNoteModel.value() ? _n->frequency()
                                         : m_startFreqModel.value(),
                m_endNoteModel.value() ? _n->frequency()
                                       : m_endFreqModel.value(),
                m_tailModel.value() * m_tailModel.value(),
                m_noiseModel.value() * m_noiseModel.value(),
                m_clickModel.value() * 0.25, m_slopeModel.value(),
                m_phaseFactorModel.value(), m_envModel.value(),
                m_distModel.value(), m_distEndModel.value(), decfr,
                WaveFormStandard::get(m_sinBankModel.value(),
                                      m_sinIndexModel.value()),
                WaveFormStandard::get(m_whnBankModel.value(),
                                      m_whnIndexModel.value()));
    }

    // if(!_n->isReleased())
    {
        SweepOsc* so = static_cast<SweepOsc*>(_n->m_pluginData);
        if(!so)
            qWarning("kickerInstrument::playNote null pluginData");
        else
            so->update(_working_buffer + offset, frames,
                       Engine::mixer()->processingSampleRate());
    }

    if(_n->isReleased())
    {
        const real_t done    = _n->releaseFramesDone();
        const real_t desired = desiredReleaseFrames();
        for(fpp_t f = 0; f < frames; ++f)
        {
            const real_t fac = (done + real_t(f) < desired)
                                       ? (1. - ((done + real_t(f)) / desired))
                                       : 0.;
            _working_buffer[f + offset][0] *= fac;
            _working_buffer[f + offset][1] *= fac;
        }
    }

    instrumentTrack()->processAudioBuffer(_working_buffer, frames + offset,
                                          _n);
}

void kickerInstrument::deleteNotePluginData(NotePlayHandle* _n)
{
    SweepOsc* so = static_cast<SweepOsc*>(_n->m_pluginData);
    if(so)
    {
        delete so;
        _n->m_pluginData = NULL;
    }
}

PluginView* kickerInstrument::instantiateView(QWidget* _parent)
{
    return new kickerInstrumentView(this, _parent);
}

class kickerKnob : public Knob
{
  public:
    kickerKnob(QWidget* _parent) : Knob(knobStyled, _parent)
    {
        setFixedSize(29, 29);
        setObjectName("smallKnob");
    }
};

class kickerEnvKnob : public TempoSyncKnob
{
  public:
    kickerEnvKnob(QWidget* _parent) : TempoSyncKnob(knobStyled, _parent)
    {
        setFixedSize(29, 29);
        setObjectName("smallKnob");
    }
};

class kickerLargeKnob : public Knob
{
  public:
    kickerLargeKnob(QWidget* _parent) : Knob(knobStyled, _parent)
    {
        setFixedSize(34, 34);
        setObjectName("largeKnob");
    }
};

kickerInstrumentView::kickerInstrumentView(Instrument* _instrument,
                                           QWidget*    _parent) :
      InstrumentView(_instrument, _parent)
{
    const int ROW1    = 14;
    const int ROW2    = ROW1 + 56;
    const int ROW3    = ROW2 + 56;
    const int LED_ROW = 60;
    const int COL1    = 14;
    const int COL2    = COL1 + 56;
    const int COL3    = COL2 + 59;
    const int COL4    = COL3 + 41;
    const int COL5    = COL4 + 41;
    const int END_COL = COL1 + 48;

    m_startFreqKnob = new kickerLargeKnob(this);
    m_startFreqKnob->setHintText(tr("Start frequency:"), "Hz");
    m_startFreqKnob->move(COL1, ROW1);

    m_endFreqKnob = new kickerLargeKnob(this);
    m_endFreqKnob->setHintText(tr("End frequency:"), "Hz");
    m_endFreqKnob->move(END_COL, ROW1);

    m_slopeKnob = new kickerKnob(this);
    m_slopeKnob->setHintText(tr("Frequency Slope:"), "");
    m_slopeKnob->move(COL3, ROW1 + 1);

    m_phaseFactorKnob = new kickerKnob(this);
    m_phaseFactorKnob->setHintText(tr("Phase Factor:"), "");
    m_phaseFactorKnob->move(COL4, ROW1 + 1);

    m_clickKnob = new kickerKnob(this);
    m_clickKnob->setHintText(tr("Click:"), "");
    m_clickKnob->move(COL5, ROW1 + 1);

    m_gainKnob = new kickerKnob(this);
    m_gainKnob->setHintText(tr("Gain:"), "");
    m_gainKnob->move(COL1 - 4, ROW3);

    m_decayKnob = new kickerEnvKnob(this);
    m_decayKnob->setHintText(tr("Envelope Length:"), "ms");
    m_decayKnob->move(COL1 + 36, ROW3);

    m_envKnob = new kickerKnob(this);
    m_envKnob->setHintText(tr("Envelope Slope:"), "");
    m_envKnob->move(COL1 + 76, ROW3);

    m_tailKnob = new kickerKnob(this);
    m_tailKnob->setHintText(tr("Tail:"), "");
    m_tailKnob->move(COL4, ROW3);

    m_noiseKnob = new kickerKnob(this);
    m_noiseKnob->setHintText(tr("Mix:"), "");
    m_noiseKnob->move(COL5, ROW3);

    m_distKnob = new kickerKnob(this);
    m_distKnob->setHintText(tr("Distortion Start:"), "");
    m_distKnob->move(COL4, ROW2);

    m_distEndKnob = new kickerKnob(this);
    m_distEndKnob->setHintText(tr("Distortion End:"), "");
    m_distEndKnob->move(COL5, ROW2);

    m_startNoteToggle = new LedCheckBox("", this, "", LedCheckBox::Green);
    m_startNoteToggle->move(COL1 + 8, LED_ROW);

    m_endNoteToggle = new LedCheckBox("", this, "", LedCheckBox::Green);
    m_endNoteToggle->move(END_COL + 8, LED_ROW);

    m_sinBankCMB  = new ComboBox(this);
    m_sinIndexCMB = new ComboBox(this);
    m_whnBankCMB  = new ComboBox(this);
    m_whnIndexCMB = new ComboBox(this);

    m_sinBankCMB->setGeometry(10, 191, 100, 18);
    m_sinIndexCMB->setGeometry(115, 191, 120, 18);
    m_whnBankCMB->setGeometry(10, 218, 100, 18);
    m_whnIndexCMB->setGeometry(115, 218, 120, 18);

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), PLUGIN_NAME::getIconPixmap("kicker_bg"));
    setPalette(pal);
}

kickerInstrumentView::~kickerInstrumentView()
{
}

void kickerInstrumentView::modelChanged()
{
    kickerInstrument* k = castModel<kickerInstrument>();
    m_startFreqKnob->setModel(&k->m_startFreqModel);
    m_endFreqKnob->setModel(&k->m_endFreqModel);
    m_decayKnob->setModel(&k->m_decayModel);
    m_distKnob->setModel(&k->m_distModel);
    m_distEndKnob->setModel(&k->m_distEndModel);
    m_gainKnob->setModel(&k->m_gainModel);
    m_envKnob->setModel(&k->m_envModel);
    m_tailKnob->setModel(&k->m_tailModel);
    m_noiseKnob->setModel(&k->m_noiseModel);
    m_clickKnob->setModel(&k->m_clickModel);
    m_slopeKnob->setModel(&k->m_slopeModel);
    m_startNoteToggle->setModel(&k->m_startNoteModel);
    m_endNoteToggle->setModel(&k->m_endNoteModel);

    m_sinBankCMB->setModel(&k->m_sinBankModel);
    m_sinIndexCMB->setModel(&k->m_sinIndexModel);
    m_whnBankCMB->setModel(&k->m_whnBankModel);
    m_whnIndexCMB->setModel(&k->m_whnIndexModel);
    m_phaseFactorKnob->setModel(&k->m_phaseFactorModel);
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model*, void* _data)
    {
        return new kickerInstrument(static_cast<InstrumentTrack*>(_data));
    }
}
