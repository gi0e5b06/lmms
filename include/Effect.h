/*
 * Effect.h - base class for effects
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2006-2007 Danny McRae <khjklujn/at/users.sourceforge.net>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef EFFECT_H
#define EFFECT_H

#include "AutomatableModel.h"
#include "Engine.h"
#include "MidiEventProcessor.h"
#include "Mixer.h"
#include "Plugin.h"
#include "TempoSyncKnobModel.h"
//#include "MemoryManager.h"
//#include "Configuration.h"

#include <QColor>

class EffectChain;
class EffectControls;

class EXPORT Effect : public Plugin, public MidiEventProcessor
{
    MM_OPERATORS
    Q_OBJECT

  public:
    Effect(const Plugin::Descriptor*                 _desc,
           Model*                                    _parent,
           const Descriptor::SubPluginFeatures::Key* _key);
    virtual ~Effect();

    virtual QDomElement saveState(QDomDocument& _doc, QDomElement& _parent);

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    static INLINE const QString classNodeName()
    {
        return "effect";
    }

    virtual QString nodeName() const final
    {
        return classNodeName();
    }

    virtual bool processAudioBuffer(sampleFrame* _buf, const fpp_t _frames)
            = 0;

    virtual bool hasMidiIn()
    {
        return false;
    }

    virtual bool hasMidiOut()
    {
        return false;
    }

    virtual void processInEvent(const MidiEvent& event,
                                const MidiTime&  time   = MidiTime(),
                                f_cnt_t          offset = 0)
    {
        qWarning(
                "Effect::processInEvent should be implemented in inheriting "
                "classes");
    }

    virtual void processOutEvent(const MidiEvent& event,
                                 const MidiTime&  time   = MidiTime(),
                                 f_cnt_t          offset = 0)
    {
        qWarning(
                "Effect::processOutEvent should be implemented in inheriting "
                "classes");
    }

    INLINE ch_cnt_t processorCount() const
    {
        return m_processors;
    }

    INLINE void setProcessorCount(ch_cnt_t _processors)
    {
        m_processors = _processors;
    }

    INLINE bool isOkay() const
    {
        return m_okay;
    }

    INLINE void setOkay(bool _state)
    {
        m_okay = _state;
    }

    INLINE bool isRunning() const
    {
        return m_runningModel.value();
    }

    void startRunning();
    void stopRunning();

    INLINE bool isEnabled() const
    {
        return m_enabledModel.value();
    }

    virtual bool isSwitchable() const
    {
        return true;
    }

    virtual bool isWetDryable() const
    {
        return true;
    }

    virtual bool isGateable() const
    {
        return true;
    }

    virtual bool isBalanceable() const
    {
        return true;
    }

    /*
    INLINE real_t wetLevel() const
    {
            return m_wetDryModel.value();
    }

    INLINE real_t dryLevel() const
    {
            return 1. - m_wetDryModel.value();
    }
    */

    INLINE real_t gate() const
    {
        return m_gateModel.value();
        // const real_t level = m_gateModel.value();
        // return level*level * m_processors;
    }

    // should be replaced by Runnable
    INLINE bool dontRun() const
    {
        return m_noRun;
    }

    INLINE void setDontRun(bool _state)
    {
        m_noRun = _state;
    }

    INLINE const Descriptor::SubPluginFeatures::Key& key() const
    {
        return m_key;
    }

    EffectChain* effectChain() const
    {
        return m_parent;
    }

    virtual EffectControls* controls() = 0;

    static Effect* instantiate(const QString& _plugin_name,
                               Model*         _parent,
                               Descriptor::SubPluginFeatures::Key* _key);

    bool   useStyleColor() const;
    void   setUseStyleColor(bool _b);
    QColor color() const;
    void   setColor(const QColor& _c);

  public slots:
    // virtual void clear();
    virtual void copy() final;
    // virtual void paste();
    virtual void toggleMute() final;

  protected:
    INLINE bool isAutoQuitEnabled() const
    {
        return !CONFIG_GET_BOOL("ui.disableautoquit");
        //! m_autoQuitDisabled;
    }

    int timeout() const;

    virtual PluginView* instantiateView(QWidget*);

    // some effects might not be capable of higher sample-rates so they can
    // sample it down before processing and back after processing
    INLINE void sampleDown(const sampleFrame* _src_buf,
                           sampleFrame*       _dst_buf,
                           sample_rate_t      _dst_sr)
    {
        resample(0, _src_buf, Engine::mixer()->processingSampleRate(),
                 _dst_buf, _dst_sr, Engine::mixer()->framesPerPeriod());
    }

    INLINE void sampleBack(const sampleFrame* _src_buf,
                           sampleFrame*       _dst_buf,
                           sample_rate_t      _src_sr)
    {
        resample(1, _src_buf, _src_sr, _dst_buf,
                 Engine::mixer()->processingSampleRate(),
                 Engine::mixer()->framesPerPeriod() * _src_sr
                         / Engine::mixer()->processingSampleRate());
    }
    void reinitSRC();

    INLINE bool isGateClosed() const
    {
        return m_gateClosed;
    }

    bool shouldProcessAudioBuffer(sampleFrame* _buf,
                                  const fpp_t  _frames,
                                  bool&        _smoothBegin,
                                  bool&        _smoothEnd);

    bool shouldKeepRunning(sampleFrame* _buf,
                           const fpp_t  _frames,
                           bool         _unclip = false);

#ifdef REAL_IS_DOUBLE
    // TMP, obsolete
    void computeWetDryLevels(fpp_t  _f,
                             fpp_t  _frames,
                             bool   _smoothBegin,
                             bool   _smoothEnd,
                             float& _w0,
                             float& _d0,
                             float& _w1,
                             float& _d1);
#endif

    void computeWetDryLevels(fpp_t   _f,
                             fpp_t   _frames,
                             bool    _smoothBegin,
                             bool    _smoothEnd,
                             real_t& _w0,
                             real_t& _d0,
                             real_t& _w1,
                             real_t& _d1);

    real_t computeRMS(sampleFrame* _buf, const fpp_t _frames);

    INLINE bool isClipping()
    {
        return m_clippingModel.value();
    }

    INLINE void setClipping(bool _b)
    {
        m_clippingModel.setAutomatedValue(_b);
    }

  private:
    bool gateHasClosed(real_t& _rms, sampleFrame* _buf, const fpp_t _frames);
    bool gateHasOpen(real_t& _rms, sampleFrame* _buf, const fpp_t _frames);

    bool         m_gateClosed;
    EffectChain* m_parent;
    void         resample(int                _i,
                          const sampleFrame* _src_buf,
                          sample_rate_t      _src_sr,
                          sampleFrame*       _dst_buf,
                          sample_rate_t      _dst_sr,
                          const f_cnt_t      _frames);

    Descriptor::SubPluginFeatures::Key m_key;

    ch_cnt_t m_processors;

    bool      m_okay;
    bool      m_noRun;
    BoolModel m_runningModel;
    f_cnt_t   m_bufferCount;

    BoolModel          m_enabledModel;
    BoolModel          m_clippingModel;
    FloatModel         m_wetDryModel;
    TempoSyncKnobModel m_autoQuitModel;
    FloatModel         m_gateModel;
    FloatModel         m_balanceModel;

    QColor m_color;
    bool   m_useStyleColor;

    // bool m_autoQuitDisabled;

    SRC_DATA   m_srcData[2];
    SRC_STATE* m_srcState[2];

    friend class EffectView;
    friend class EffectChain;
};

typedef Effect::Descriptor::SubPluginFeatures::Key     EffectKey;
typedef Effect::Descriptor::SubPluginFeatures::KeyList EffectKeyList;

#endif
