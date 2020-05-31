/*
 * Effect.cpp - base-class for effects
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2006-2007 Danny McRae <khjklujn/at/users.sourceforge.net>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "Effect.h"

#include "Clipboard.h"
#include "Configuration.h"
#include "EffectChain.h"
#include "EffectControls.h"
#include "EffectView.h"

#include <QDomElement>

#include <cmath>
//#include "ConfigManager.h"
#include "MixHelpers.h"
#include "SampleRate.h"
#include "WaveForm.h"
//#include "lmms_math.h"

Effect::Effect(const Plugin::Descriptor*                 _desc,
               Model*                                    _parent,
               const Descriptor::SubPluginFeatures::Key* _key) :
      Plugin(_desc, _parent),
      m_gateClosed(true), m_parent(NULL),
      m_key(_key ? *_key : Descriptor::SubPluginFeatures::Key()),
      m_processors(1), m_okay(true), m_noRun(false),
      m_runningModel(false, this, tr("Effect running")), m_bufferCount(0),
      m_enabledModel(true, this, tr("Effect enabled")),
      m_clippingModel(false, this, tr("Clipping alert")),
      m_wetDryModel(1., 0., 1., 0.01, this, tr("Wet/Dry mix")),  // min=-1
      m_autoQuitModel(1000., 1., 8000., 1., 8000., this, tr("Decay")),
      m_gateModel(0.000001, 0.000001, 1., 0.000001, this, tr("Gate")),
      m_balanceModel(0., -1., 1., 0.01, this, tr("Balance")),
      m_color(59, 66, 74),  //#3B424A
      m_useStyleColor(true)
// m_autoQuitDisabled( false )
{
    // m_gateModel.setScaleLogarithmic(true);

    m_srcState[0] = m_srcState[1] = NULL;
    reinitSRC();

    // if( ConfigManager::inst()->value( "ui", "disableautoquit").toInt() )
    //{
    //	m_autoQuitDisabled = true;
    //}
    // qInfo("Effect::Effect end constructor");
}

Effect::~Effect()
{
    for(int i = 0; i < 2; ++i)
    {
        if(m_srcState[i] != NULL)
        {
            src_delete(m_srcState[i]);
        }
    }
}

void Effect::startRunning()
{
    if(!isRunning())
    {
        // qInfo("%s: start running", qPrintable(nodeName()));
        // resetBufferCount();//m_bufferCount = 0;
        m_runningModel.setAutomatedValue(true);
    }
}

void Effect::stopRunning()
{
    if(isRunning())
    {
        // qInfo("%s: stop running", qPrintable(nodeName()));
        m_runningModel.setAutomatedValue(false);
    }

    if(!m_gateClosed)
        m_gateClosed = true;
}

QDomElement Effect::saveState(QDomDocument& _doc, QDomElement& _parent)
{
    // qInfo("Effect::saveState");
    QDomElement r = Plugin::saveState(_doc, _parent);
    r.setAttribute("name", /* QString::fromUtf8*/ descriptor()->name());
    r.appendChild(key().saveXML(_doc));
    return r;
}

void Effect::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    m_enabledModel.saveSettings(_doc, _this, "on");
    m_wetDryModel.saveSettings(_doc, _this, "wet");
    m_autoQuitModel.saveSettings(_doc, _this, "autoquit");
    m_gateModel.saveSettings(_doc, _this, "gate");
    m_balanceModel.saveSettings(_doc, _this, "balance");
    controls()->saveState(_doc, _this);
}

void Effect::loadSettings(const QDomElement& _this)
{
    m_enabledModel.loadSettings(_this, "on");
    m_wetDryModel.loadSettings(_this, "wet");
    if(_this.hasAttribute("autoquit"))
        m_autoQuitModel.loadSettings(_this, "autoquit");
    else
        m_autoQuitModel.setValue(1000);
    m_gateModel.loadSettings(_this, "gate");
    m_balanceModel.loadSettings(_this, "balance");

    QDomNode node = _this.firstChild();
    while(!node.isNull())
    {
        if(node.isElement())
        {
            if(controls()->nodeName() == node.nodeName())
            {
                controls()->restoreState(node.toElement());
            }
        }
        node = node.nextSibling();
    }

    /*
    if(m_wetDryModel.minValue()==-1.)
    {
            m_wetDryModel.setMinValue(0.);
    }
    */
    if(m_wetDryModel.value() < 0.)
    {
        m_wetDryModel.setValue(-m_wetDryModel.value());
    }

    if(!isBalanceable())
        m_balanceModel.setValue(0.);

    m_gateModel.setScaleLogarithmic(true);
}

Effect* Effect::instantiate(const QString&                      pluginName,
                            Model*                              _parent,
                            Descriptor::SubPluginFeatures::Key* _key)
{
    Plugin* p = Plugin::instantiate(pluginName, _parent, _key);
    // check whether instantiated plugin is an effect
    if(dynamic_cast<Effect*>(p) != NULL)
    {
        // qInfo("Effect::instantiate SUCCESS %s",qPrintable(pluginName));
        // everything ok, so return pointer
        Effect* effect   = dynamic_cast<Effect*>(p);
        effect->m_parent = dynamic_cast<EffectChain*>(_parent);
        return effect;
    }

    qWarning("Effect::instantiate FAILED %s", qPrintable(pluginName));
    // not quite... so delete plugin and leave it up to the caller to
    // instantiate a DummyEffect
    delete p;

    return NULL;
}

int Effect::timeout() const
{
    real_t decay = m_autoQuitModel.value();
    if(decay >= 8000.)
        decay = 3600000.;  // one hour

    const real_t samples = Engine::mixer()->processingSampleRate()
                           * m_autoQuitModel.value() / 1000.;
    return /*1+*/ static_cast<int>(samples
                                   / Engine::mixer()->framesPerPeriod());
}

bool Effect::gateHasClosed(real_t&      _rms,
                           sampleFrame* _buf,
                           const fpp_t  _frames)
{
    if(!isAutoQuitEnabled())
        return false;
    if(!isRunning())
    {
        m_gateClosed = true;
        return false;
    }

    real_t g = gate();
    // Check whether we need to continue processing input.  Restart the
    // counter if the threshold has been exceeded.
    if(g > 0.)
    {
        if(g < 1. && _rms < 0.)
        {
            _rms = computeRMS(_buf, _frames);
            // qInfo("GHC g=%f rms=%f", g, _rms);
        }

        if(Engine::mixer()->warningXRuns())
            g += 0.01;
        if(Engine::mixer()->criticalXRuns())
            g += 0.05;

        if(_rms < g)
        {
            m_bufferCount++;  // incrementBufferCount();
            // qInfo("GHC buffercount %d/%d",m_bufferCount,timeout());
            if(!m_gateClosed && (m_bufferCount > timeout()))
            {
                // qInfo("GHC STOP");
                m_gateClosed = true;
                stopRunning();
                // resetBufferCount();
                return true;
            }
            else
                return false;
        }
    }

    m_bufferCount = 0;
    // qInfo("GHC RESET"); // resetBufferCount();
    return false;
}

bool Effect::gateHasOpen(real_t& _rms, sampleFrame* _buf, const fpp_t _frames)
{
    if(!isAutoQuitEnabled())
        return false;
    // if(isRunning()) return false;

    const real_t g = gate();
    // Check whether we need to continue processing input.  Restart the
    // counter if the threshold has been exceeded.
    if(m_gateClosed && g < 1.)
    {
        if(g > 0. && _rms < 0.)
            _rms = computeRMS(_buf, _frames);
        if(g == 0. || _rms >= g)
        {
            m_gateClosed = false;
            if(!isRunning())
                startRunning();
            m_bufferCount = 0;
            // qInfo("GHO RESET");  // resetBufferCount();
            return true;
        }
    }

    return false;
}

real_t Effect::computeRMS(sampleFrame* _buf, const fpp_t _frames)
{
    // if( !isEnabled() ) return 0.;
    // if(!isAutoQuitEnabled()) return 1.;

    real_t rms  = 0.;
    fpp_t  step = qMax(1, _frames >> 5);
    for(fpp_t f = 0; f < _frames; f += step)
        rms += _buf[f][0] * _buf[f][0] + _buf[f][1] * _buf[f][1];
    // rms/=_frames;
    rms /= (_frames / step);

    if(isnan(rms))
    {
        qInfo("Effect::computeRMS rms=%f step=%d _frames=%d", rms, step,
              _frames);
        rms = 0.;
    }

    // return fastsqrtf01(qBound(0.,rms,1.));
    real_t r = WaveForm::sqrt(bound(0., rms, 1.));
    // qInfo("Effect::computeRMS rms=%f r=%f", rms, r);
    return r;
}

bool Effect::shouldProcessAudioBuffer(sampleFrame* _buf,
                                      const fpp_t  _frames,
                                      bool&        _smoothBegin,
                                      bool&        _smoothEnd)
{
    if(!isOkay() || dontRun() || !isEnabled())
    {
        if(isRunning())
            stopRunning();

        // qInfo("%s: shouldPAB return false okey=%d enabled=%d",
        //      qPrintable(nodeName()), isOkay(), isEnabled());
        return false;
    }

    _smoothBegin = false;
    _smoothEnd   = false;

    if(isAutoQuitEnabled())
    {
        real_t rms = -1.;
        if(gateHasOpen(rms, _buf, _frames))
        {
            // qInfo("%s: gate open", qPrintable(nodeName()));
            _smoothBegin = true;
        }
        else if(gateHasClosed(rms, _buf, _frames))
        {
            // qInfo("%s: gate closed", qPrintable(nodeName()));
            _smoothEnd = true;
        }
        // qInfo("%s: shouldPAB autoquit rms=%f", qPrintable(nodeName()),
        // rms);
    }

    if(!isRunning() && !_smoothEnd)
    {
        /*qInfo("%s: shouldPAB return false running=%d",
          qPrintable(nodeName()), isRunning());*/
        ValueBuffer* wetDryBuf = m_wetDryModel.valueBuffer();

        for(fpp_t f = 0; f < _frames; ++f)
        {
            real_t w = (wetDryBuf ? wetDryBuf->value(f)
                                  : m_wetDryModel.value());
            real_t d = 1. - w;
            _buf[f][0] *= d;
            _buf[f][1] *= d;
        }

        return false;
    }

    return true;
}

bool Effect::shouldKeepRunning(sampleFrame* _buf,
                               const fpp_t  _frames,
                               bool         _unclip)
{
    if(!isOkay() || dontRun() || !isEnabled())
    {
        // qInfo("KR STOP 1");
        return false;
    }
    if(!isRunning())
    {
        // qInfo("KR STOP 2");
        return false;
    }

    if(_unclip)
    {
        if(MixHelpers::unclip(_buf, _frames))
            setClipping(true);
    }
    else
    {
        if(MixHelpers::isClipping(_buf, _frames))
            setClipping(true);
    }

    if(!isAutoQuitEnabled())
        return true;

    real_t rms = computeRMS(_buf, _frames);
    if(rms > 0.00001)
        return true;

    // qInfo("KR buffercount %d/%d",m_bufferCount,timeout());
    if(m_bufferCount > timeout())  // qMax<int>(176,timeout()))
    {
        // qInfo("KR STOP");
        stopRunning();
        // resetBufferCount();
        return false;
    }
    else
        m_bufferCount++;  // incrementBufferCount();

    return true;
}

#ifdef REAL_IS_DOUBLE
// TMP, obsolete
void Effect::computeWetDryLevels(fpp_t  _f,
                                 fpp_t  _frames,
                                 bool   _smoothBegin,
                                 bool   _smoothEnd,
                                 FLOAT& _w0,
                                 FLOAT& _d0,
                                 FLOAT& _w1,
                                 FLOAT& _d1)
{
    real_t w0, d0, w1, d1;
    computeWetDryLevels(_f, _frames, _smoothBegin, _smoothEnd, w0, d0, w1,
                        d1);
    _w0 = w0;
    _d0 = d0;
    _w1 = w1;
    _d1 = d1;
}
#endif

void Effect::computeWetDryLevels(fpp_t   _f,
                                 fpp_t   _frames,
                                 bool    _smoothBegin,
                                 bool    _smoothEnd,
                                 real_t& _w0,
                                 real_t& _d0,
                                 real_t& _w1,
                                 real_t& _d1)
{
    ValueBuffer* wetDryBuf = m_wetDryModel.valueBuffer();

    real_t w = (wetDryBuf ? wetDryBuf->value(_f) : m_wetDryModel.value());
    _d0      = 1.0 - w;
    _d1      = 1.0 - w;

    int nsb = _frames;
    if(nsb > 128)
        nsb = 128;
    if(_smoothBegin && _f < nsb)
        w *= real_t(_f) / real_t(nsb);
    int nse = _frames;
    if(nse > 128)
        nse = 128;
    if(_smoothEnd && _f >= _frames - nse)
        w *= real_t(_frames - 1 - _f) / real_t(nse);

    if(isGateClosed() && !_smoothEnd)
    {
        // w=0.;
        _w0 = 0.;
        _w1 = 0.;
    }
    else if(isBalanceable())
    {
        ValueBuffer* balanceBuf = m_balanceModel.valueBuffer();
        const real_t bal        = (balanceBuf ? balanceBuf->value(_f)
                                       : m_balanceModel.value());
        const real_t bal0       = bal < 0. ? 1. : 1. - bal;
        const real_t bal1       = bal < 0. ? 1. + bal : 1.;
        _w0                     = bal0 * w;
        _w1                     = bal1 * w;
        const real_t df         = (2. + _d0 + _d1) / 4.;
        _d0 += (1. - bal0) * w * df;
        _d1 += (1. - bal1) * w * df;
    }
    else
    {
        _w0 = w;
        _w1 = w;
    }

    _d0 *= (1. - _w0);
    _d1 *= (1. - _w1);
}

PluginView* Effect::instantiateView(QWidget* _parent)
{
    return new EffectView(this, _parent);
}

void Effect::reinitSRC()
{
    for(int i = 0; i < 2; ++i)
    {
        if(m_srcState[i] != NULL)
        {
            src_delete(m_srcState[i]);
        }
        int error;
        if((m_srcState[i] = src_new(Engine::mixer()
                                            ->currentQualitySettings()
                                            .libsrcInterpolation(),
                                    DEFAULT_CHANNELS, &error))
           == NULL)
        {
            qFatal("Error: src_new() failed in effect.cpp!\n");
        }
    }
}

void Effect::resample(int                _i,
                      const sampleFrame* _src_buf,
                      sample_rate_t      _src_sr,
                      sampleFrame*       _dst_buf,
                      sample_rate_t      _dst_sr,
                      f_cnt_t            _frames)
{
    const double frqRatio                = double(_dst_sr) / double(_src_sr);
    const fpp_t  fpp                     = Engine::mixer()->framesPerPeriod();
    f_cnt_t      input_frames_used       = 0;
    f_cnt_t      output_frames_generated = 0;

    SampleRate::resample(_src_buf, _dst_buf, _frames, fpp, frqRatio, 10,
                         input_frames_used, output_frames_generated,
                         m_srcState[_i]);
    /*
#ifdef REAL_IS_FLOAT
    {
            if( m_srcState[_i] == NULL )
                    return;
            m_srcData[_i].input_frames = _frames;
            m_srcData[_i].output_frames = Engine::mixer()->framesPerPeriod();
            m_srcData[_i].data_in = _src_buf[0];
            m_srcData[_i].data_out = _dst_buf[0];
            m_srcData[_i].src_ratio = double(_dst_sr) / double(_src_sr);
            m_srcData[_i].end_of_input = 0;
            int error;
            if( ( error = src_process( m_srcState[_i], &m_srcData[_i] ) ) )
            {
                    qFatal( "Effect::resample(): error while resampling: %s",
                            src_strerror( error ) );
            }
    }
#endif
#ifdef REAL_IS_DOUBLE
    {
            //resample64
            qFatal("Effect::resample");
    }
#endif
    */
}

QColor Effect::color() const
{
    return m_color;
}

void Effect::setColor(const QColor& _c)
{
    m_color = _c;
}

bool Effect::useStyleColor() const
{
    return m_useStyleColor;
}

void Effect::setUseStyleColor(bool _b)
{
    m_useStyleColor = _b;
}

void Effect::copy()
{
    Clipboard::copy(this);
}

void Effect::toggleMute()
{
    m_enabledModel.setValue(!m_enabledModel.value());
}
