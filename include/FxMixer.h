/*
 * FxMixer.h - effect-mixer for LMMS
 *
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef FX_MIXER_H
#define FX_MIXER_H

#include "Effect.h"
#include "EffectChain.h"
#include "JournallingObject.h"
#include "Model.h"
#include "ThreadableJob.h"
#include "debug.h"

class FxChannel;
class FxRoute;
class SampleBuffer;

typedef DebugQVector<QPointer<FxRoute>>   FxRoutes;
typedef DebugQVector<QPointer<FxChannel>> FxChannels;

class FxChannel :
      public Model,
      public virtual ThreadableJob,
      public SerializingObject
{
    Q_OBJECT

    mapPropertyFromModel(bool, isMuted, setMuted, m_mutedModel);
    mapPropertyFromModel(bool, isSolo, setSolo, m_soloModel);
    mapPropertyFromModel(bool, isFrozen, setFrozen, m_frozenModel);
    mapPropertyFromModel(bool, isClipping, setClipping, m_clippingModel);

  public:
    FxChannel(int idx, Model* _parent);
    virtual ~FxChannel();

    virtual QString nodeName() const
    {
        return "fxchannel";
    }

    virtual QString name() const
    {
        return m_name;
    }

    virtual void setName(const QString _name)
    {
        m_name = _name;
    }

    virtual fx_ch_t channelIndex() const
    {
        return m_channelIndex;
    }

    virtual void setChannelIndex(fx_ch_t _ci)
    {
        m_channelIndex = _ci;
    }

    virtual const EffectChain& fxChain() const
    {
        return m_fxChain;
    }

    virtual EffectChain& fxChain()
    {
        return m_fxChain;
    }

    virtual bool hasInput() const
    {
        return m_hasInput;
    }

    virtual void setHasInput(bool _b)
    {
        m_hasInput = _b;
    }

    virtual bool isQueued() const
    {
        return m_queued;
    }

    virtual void setQueued(bool _b)
    {
        m_queued = _b;
    }

    virtual sampleFrame* buffer() const
    {
        return m_buffer;
    }

    virtual real_t peakLeft() const
    {
        return m_peakLeft;
    }

    virtual void resetPeakLeft()
    {
        m_peakLeft = 0.;
    }

    virtual real_t peakRight() const
    {
        return m_peakRight;
    }

    virtual void resetPeakRight()
    {
        m_peakRight = 0.;
    }

    virtual bool hasCableFrom(Model* _m) const;

    INLINE const BoolModel& clippingModel() const
    {
        return m_clippingModel;
    }

    INLINE BoolModel& clippingModel()
    {
        return m_clippingModel;
    }

    INLINE const BoolModel& frozenModel() const
    {
        return m_frozenModel;
    }

    INLINE BoolModel& frozenModel()
    {
        return m_frozenModel;
    }

    INLINE const BoolModel& mutedModel() const
    {
        return m_mutedModel;
    }

    INLINE BoolModel& mutedModel()
    {
        return m_mutedModel;
    }

    INLINE const BoolModel& soloModel() const
    {
        return m_soloModel;
    }

    INLINE BoolModel& soloModel()
    {
        return m_soloModel;
    }

    INLINE const RealModel& volumeModel() const
    {
        return m_volumeModel;
    }

    INLINE RealModel& volumeModel()
    {
        return m_volumeModel;
    }

    INLINE const FxRoutes& sends() const
    {
        return m_sends;
    }

    INLINE const FxRoutes& receives() const
    {
        return m_receives;
    }

    void addSendRoute(FxRoute* _route)
    {
        m_sends.append(_route);
    }

    void addReceiveRoute(FxRoute* _route)
    {
        m_receives.append(_route);
    }

    void removeSendRoute(FxRoute* _route)
    {
        m_sends.remove(m_sends.indexOf(_route));
    }

    void removeReceiveRoute(FxRoute* _route)
    {
        m_receives.remove(m_receives.indexOf(_route));
    }

    virtual void updateFrozenBuffer();
    virtual void cleanFrozenBuffer();
    virtual void readFrozenBuffer();
    virtual void writeFrozenBuffer();

    virtual bool requiresProcessing() const final
    {
        return true;
    }

    void muteForSolo();
    void unmuteForSolo();
    void resetSolo();

    INLINE void processed();

    void lock()
    {
        m_lock.lock();
    }

    void unlock()
    {
        m_lock.unlock();
    }

    virtual void resetDeps();

  public slots:
    void toggleFrozen();

  protected:
    virtual void doProcessing() final;
    virtual void incrementDeps();

    virtual void saveSettings(QDomDocument& doc, QDomElement& element);
    virtual void loadSettings(const QDomElement& element);

  private:
    EffectChain m_fxChain;
    // set to true when input fed from mixToChannel or child channel
    bool m_hasInput;
    // set to true if any effect in the channel is enabled and running
    bool m_stillRunning;

    SampleBuffer* m_frozenBuf;
    BoolModel     m_frozenModel;
    BoolModel     m_clippingModel;

  public:  // TMP
    QPointer<Effect> m_eqDJ;
    BoolModel        m_eqDJEnableModel;
    // RealModel m_eqDJHighModel;
    // RealModel m_eqDJMediumModel;
    // RealModel m_eqDJLowModel;
  private:
    real_t       m_peakLeft;
    real_t       m_peakRight;
    sampleFrame* m_buffer;
    bool         m_mutedBeforeSolo;
    BoolModel    m_mutedModel;
    BoolModel    m_soloModel;
    RealModel    m_volumeModel;
    QString      m_name;
    int          m_channelIndex;  // what channel index are we

    QMutex m_lock;
    bool   m_queued;  // are we queued up for rendering yet?
    // bool m_muted; // are we muted? updated per period so we don't have to
    // call m_muteModel.value() twice

    // pointers to other channels that this one sends to
    FxRoutes m_sends;

    // pointers to other channels that send to this one
    FxRoutes m_receives;

    QAtomicInt m_dependenciesMet;
};

class FxRoute : public QObject, public SerializingObject
{
    Q_OBJECT

  public:
    FxRoute(FxChannel* from, FxChannel* to, real_t amount);
    virtual ~FxRoute();

    virtual QString nodeName() const
    {
        return "send";
    }

    fx_ch_t senderIndex() const
    {
        return m_from->channelIndex();
    }

    fx_ch_t receiverIndex() const
    {
        return m_to->channelIndex();
    }

    RealModel* amount()
    {
        return &m_amount;
    }

    FxChannel* sender() const
    {
        return m_from;
    }

    FxChannel* receiver() const
    {
        return m_to;
    }

    void updateName();

  protected:
    virtual void saveSettings(QDomDocument& doc, QDomElement& element);
    virtual void loadSettings(const QDomElement& element);

  private:
    QPointer<FxChannel> m_from;
    QPointer<FxChannel> m_to;
    RealModel           m_amount;
};

class EXPORT FxMixer : public Model, public JournallingObject
{
    Q_OBJECT

  public:
    FxMixer();
    virtual ~FxMixer();

    void mixToChannel(const sampleFrame* _buf, fx_ch_t _ch);

    void prepareMasterMix();
    void masterMix(sampleFrame* _buf);

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    virtual QString nodeName() const
    {
        return "fxmixer";
    }

    const FxChannels& channels() const
    {
        return m_fxChannels;
    }

    FxChannel* effectChannel(int _ch)
    {
        return m_fxChannels[_ch];
    }

    // make the output of channel fromChannel go to the input of channel
    // toChannel it is safe to call even if the send already exists
    FxRoute* createChannelSend(fx_ch_t fromChannel,
                               fx_ch_t toChannel,
                               real_t  amount = 1.);
    FxRoute* createRoute(FxChannel* from, FxChannel* to, real_t amount);

    // delete the connection made by createChannelSend
    void deleteChannelSend(fx_ch_t fromChannel, fx_ch_t toChannel);
    void deleteChannelSend(FxRoute* route);

    // determine if adding a send from sendFrom to
    // sendTo would result in an infinite mixer loop.
    bool isInfiniteLoop(fx_ch_t fromChannel, fx_ch_t toChannel);
    bool checkInfiniteLoop(FxChannel* from, FxChannel* to);

    // return the RealModel of fromChannel sending its output to the input of
    // toChannel. nullptr if there is no send.
    RealModel* channelSendModel(fx_ch_t fromChannel, fx_ch_t toChannel);

    // add a new channel to the Fx Mixer.
    // returns the index of the channel that was just added
    int createChannel();

    // delete a channel from the FX mixer.
    void deleteChannel(int index);

    // delete all the mixer channels except master and remove all effects
    void clear();

    // re-arrange channels
    void moveChannelLeft(int index);
    void moveChannelRight(int index);

    // reset a channel's name, fx, sends, etc
    void clearChannel(fx_ch_t channelIndex);

    // rename channels when moving etc. if they still have their original name
    void validateChannelName(int index, int oldIndex);

    void toggledSolo();
    void activateSolo();
    void deactivateSolo();

    INLINE fx_ch_t numChannels() const
    {
        return m_fxChannels.size();
    }

  private:
    // make sure we have at least num channels
    void allocateChannelsTo(int num);

    FxRoutes m_fxRoutes;

    // the fx channels in the mixer. index 0 is always master.
    FxChannels m_fxChannels;

    int m_lastSoloed;
};

#endif
