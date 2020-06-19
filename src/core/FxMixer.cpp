/*
 * FxMixer.cpp - effect mixer for LMMS
 *
 * Copyright (c) 2008-2011 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "FxMixer.h"

#include "BufferManager.h"
#include "MixHelpers.h"
#include "Mixer.h"
#include "MixerWorkerThread.h"
//#include "PluginFactory.h"
#include "BBTrackContainer.h"
#include "EffectControlDialog.h"
#include "EffectControls.h"
#include "InstrumentTrack.h"
#include "SampleTrack.h"
#include "Song.h"
//#include "LadspaControlView.h"
#include "Backtrace.h"
#include "Knob.h"
#include "SampleBuffer.h"
//#include "debug.h"

#include <QDir>
#include <QDomElement>
#include <QFileInfo>
//#include <QLayout>
//#include <QUuid>

FxRoute::FxRoute(FxChannel* from, FxChannel* to, real_t amount) :
      m_from(from), m_to(to),
      m_amount(amount,
               0,
               1,
               0.001,
               nullptr,
               tr("Amount to send from channel %1 to channel %2")
                       .arg(m_from->channelIndex())
                       .arg(m_to->channelIndex()))
{
    // qDebug( "created: %d to %d", m_from->m_channelIndex,
    // m_to->m_channelIndex );
    // create send amount model
}

FxRoute::~FxRoute()
{
}

void FxRoute::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    _this.setAttribute("channel", receiverIndex());
    m_amount.saveSettings(_doc, _this, "amount");
}

void FxRoute::loadSettings(const QDomElement& _this)
{
    // m_receiverIndex = _this.attribute("channel").toInt();
    m_amount.loadSettings(_this, "amount");
}

void FxRoute::updateName()
{
    m_amount.setDisplayName(tr("Amount to send from channel %1 to channel %2")
                                    .arg(m_from->channelIndex())
                                    .arg(m_to->channelIndex()));
}

FxChannel::FxChannel(int idx, Model* _parent) :
      Model(_parent, QString("FxChannel #%1").arg(idx)), m_fxChain(this),
      m_hasInput(false), m_stillRunning(false), m_frozenBuf(nullptr),
      m_frozenModel(false, _parent, tr("Frozen")),
      m_clippingModel(false, _parent, tr("Clipping")), m_eqDJ(nullptr),
      m_eqDJEnableModel(false, _parent, tr("DJ Enabled")),
      // m_eqDJHighModel  ( 0., -70., 0., 1., _parent ),
      // m_eqDJMediumModel( 0., -70., 0., 1., _parent ),
      // m_eqDJLowModel   ( 0., -70., 0., 1., _parent ),
      m_peakLeft(0.), m_peakRight(0.), m_buffer(BufferManager::acquire()),
      m_mutedModel(false, this, tr("Mute")),
      m_soloModel(false, this, tr("Solo")),
      m_volumeModel(1.0, 0.0, 1.0, 0.001, _parent, tr("Volume")),  // max=2.
      m_name(), m_channelIndex(idx), m_lock(), m_queued(false),
      m_dependenciesMet(0)
{
    if(idx > 0)
    {
        Effect::Descriptor::SubPluginFeatures::Key* key
                = new Effect::Descriptor::SubPluginFeatures::Key();
        key->name = "ladspaeffect";
        key->attributes.insert("file", "dj_eq_1901");
        key->attributes.insert("plugin", "dj_eq");
        m_eqDJ = Effect::instantiate(key->name, nullptr, key);
        // qInfo("FxChannel::FxChannel eqDJ=%p",m_eqDJ);

        /*
        EffectControls* c=m_eqDJ->controls();
        if(c)
        {
                EffectControlDialog* v=c->createView();
                if(v)
                {
                        qInfo("FxChannel::FxChannel v object tree
        '%s'",qPrintable(v->objectName())); v->dumpObjectTree(); qInfo("v %d
        children",v->children().size()); QList<Knob*>
        allKnobs=v->findChildren<Knob*>(); for(Knob* k : allKnobs)
                                qInfo("FxChannel::FxChannel knob k='%s'",
                                         qPrintable(k->objectName()));
                }
                else qInfo("FxChannel::FxChannel EffectControlDialog*
        v=nullptr");
        }
        else qInfo("FxChannel::FxChannel EffectControls* c=nullptr");
        */
    }

    BufferManager::clear(m_buffer);

    connect(&m_frozenModel, SIGNAL(dataChanged()), this,
            SLOT(toggleFrozen()));
}

FxChannel::~FxChannel()
{
    // delete[] m_buffer;
    BufferManager::release(m_buffer);
    // qInfo("FxChannel::~FxChannel idx=%d",m_channelIndex);

    if(m_frozenBuf)
        delete m_frozenBuf;
}

void FxChannel::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    m_fxChain.saveState(_doc, _this);
    m_volumeModel.saveSettings(_doc, _this, "volume");
    m_mutedModel.saveSettings(_doc, _this, "muted");
    m_soloModel.saveSettings(_doc, _this, "soloed");
    _this.setAttribute("num", m_channelIndex);
    _this.setAttribute("name", m_name);
}

void FxChannel::loadSettings(const QDomElement& _this)
{
    if(_this.tagName() != "fxchannel")
        qCritical("FxChannel::loadSettings fxchannel != %s",
                  qPrintable(_this.tagName()));

    m_volumeModel.loadSettings(_this, "volume");
    m_mutedModel.loadSettings(_this, "muted");
    m_soloModel.loadSettings(_this, "soloed");
    m_name = _this.attribute("name");

    QDomElement e = _this.firstChildElement("fxchain");
    if(!e.isNull())
        m_fxChain.restoreState(e);
}

bool FxChannel::hasCableFrom(Model* _m) const
{
    // qInfo("FxChannel::hasCableFrom");

    InstrumentTrack* it = dynamic_cast<InstrumentTrack*>(_m);
    if(it != nullptr)
        return m_channelIndex == it->effectChannelModel()->value();

    SampleTrack* st = dynamic_cast<SampleTrack*>(_m);
    if(st != nullptr)
        return m_channelIndex == st->effectChannelModel()->value();

    return false;
}

void FxChannel::toggleFrozen()
{
    // qInfo("FxChannel::toggleFrozen");
    updateFrozenBuffer();
}

void FxChannel::updateFrozenBuffer()
{
    // qInfo("FxChannel::updateFrozenBuffer");
    // if(m_frozenModel)
    {
        const Song*   song = Engine::getSong();
        const real_t  fpt  = Engine::framesPerTick();
        const f_cnt_t len  = song->ticksPerTact() * song->length() * fpt;

        if((m_frozenBuf == nullptr) || (len != m_frozenBuf->frames()))
        {
            if(m_frozenBuf)
                delete m_frozenBuf;
            m_frozenBuf = new SampleBuffer(len);
            // qInfo("FxChannel::updateFrozenBuffer len=%d",len);
        }
    }
}

void FxChannel::cleanFrozenBuffer()
{
    // qInfo("FxChannel::cleanFrozenBuffer");
    // if(m_frozenModel)
    {
        const Song*   song = Engine::getSong();
        const real_t  fpt  = Engine::framesPerTick();
        const f_cnt_t len  = song->ticksPerTact() * song->length() * fpt;

        if((m_frozenBuf == nullptr) || (len != m_frozenBuf->frames())
           || m_frozenBuf->m_mmapped)
        {
            if(m_frozenBuf)
                delete m_frozenBuf;
            m_frozenBuf = new SampleBuffer(len);
            // qInfo("FxChannel::cleanFrozenBuffer len=%d",len);
        }
    }
}

void FxChannel::readFrozenBuffer()
{
    // qInfo("InstrumentTrack::readFrozenBuffer");
    if(  // m_frozenModel&&
         // m_frozenModel->value()&&
            m_frozenBuf)
    {
        delete m_frozenBuf;
        m_frozenBuf = nullptr;
        QString d   = Engine::getSong()->projectDir() + QDir::separator()
                    + "channels" + QDir::separator() + "frozen";
        if(QFileInfo(d).exists())
        {
            QString f = d + QDir::separator() + uuid() + "."
                        + SampleBuffer::rawStereoSuffix();
            // qInfo("FxChannel::readFrozenBuffer f=%s",
            //      qPrintable(f));
            QFile fi(f);
            if(fi.exists())
            {
                if(fi.size() == 0)
                    fi.remove();
                else
                    m_frozenBuf = new SampleBuffer(f);
            }
        }
    }
}

void FxChannel::writeFrozenBuffer()
{
    // qInfo("InstrumentTrack::writeFrozenBuffer");
    if(  // m_frozenModel&&
         // m_frozenModel->value()&&
            m_frozenBuf)
    {
        QString d = Engine::getSong()->projectDir() + QDir::separator()
                    + "channels" + QDir::separator() + "frozen";
        if(QFileInfo(d).exists())
        {
            QString f = d + QDir::separator() + uuid() + "."
                        + SampleBuffer::rawStereoSuffix();
            // qInfo("AudioPort::writeFrozenBuffer f=%s",
            //      qPrintable(f));
            m_frozenBuf->writeCacheData(f);
        }
    }
}

void FxChannel::processed()
{
    for(const FxRoute* receiverRoute: m_sends)
    {
        // tmp if( receiverRoute->receiver()->m_muted == false )
        {
            receiverRoute->receiver()->incrementDeps();
        }
    }
}

void FxChannel::resetDeps()
{
    m_dependenciesMet = 0;
}

void FxChannel::incrementDeps()
{
    int i = m_dependenciesMet.fetchAndAddOrdered(1) + 1;
    if(i >= m_receives.size() && !m_queued)
    {
        m_queued = true;
        MixerWorkerThread::addJob(this);
    }
}

void FxChannel::muteForSolo()
{
    // TODO: Recursively activate every channel, this channel sends to
    m_mutedBeforeSolo = m_mutedModel.value();
    m_mutedModel.setValue(true);
}

void FxChannel::unmuteForSolo()
{
    // TODO: Recursively activate every channel, this channel sends to
    m_mutedModel.setValue(m_mutedBeforeSolo);
}

void FxChannel::resetSolo()
{
    m_soloModel.setValue(false);
}

void FxChannel::doProcessing()
{
    const Song*  song  = Engine::getSong();
    const Mixer* mixer = Engine::mixer();
    // const real_t   fpt       = Engine::framesPerTick();
    const fpp_t   fpp       = mixer->framesPerPeriod();
    const bool    exporting = song->isExporting();
    const f_cnt_t af        = song->getPlayPos().absoluteFrame();

    // qInfo("InstrumentTrack::play
    // exporting=%d",Engine::getSong()->isExporting());
    if(isFrozen() && m_frozenBuf && !exporting
       && (song->playMode() == Song::Mode_PlaySong) && song->isPlaying())
    {
        // qInfo("FxChannel::doProcessing use frozen buffer"
        //      " fx=%d fb=%p af=%d s=%p ap=%p",
        //      m_channelIndex,m_frozenBuf,af,m_buffer,this);
        for(f_cnt_t f = 0; f < fpp; ++f)
        {
            sample_t vch0, vch1;
            m_frozenBuf->getDataFrame(af + f, vch0, vch1);
            m_buffer[f][0] = vch0;
            m_buffer[f][1] = vch1;

            /*
            if(af+f>=1000 && af+f<1005)
                    qInfo("FxChannel::doProcessing use frozen buffer"
                          " fb=%p af=%d s=%p ap=%p vch0=%f vch1=%f",
                          m_frozenBuf,af+f,m_buffer,this,vch0,vch1);
            */
        }

        // We apply dj stuff
        if(m_eqDJ
           && /*m_stillRunning && m_hasInput &&*/ m_eqDJEnableModel.value())
        {
            // m_eqDJ->startRunning();
            m_stillRunning = m_eqDJ->processAudioBuffer(m_buffer, fpp);
        }
        // else if(m_channelIndex)
        //	qInfo("NOT processing... %p %d %d %d",m_eqDJ,m_stillRunning,
        //	      m_hasInput,m_eqDJEnableModel.value());

        const real_t v = m_volumeModel.value();
        if(v > 0.)
        {
            real_t peakLeft  = 0.;
            real_t peakRight = 0.;
            mixer->getPeakValues(m_buffer, fpp, peakLeft, peakRight);
            m_peakLeft  = qMax(m_peakLeft, peakLeft * v);
            m_peakRight = qMax(m_peakRight, peakRight * v);

            // if(m_channelIndex==1)
            //      qInfo("ch1 peaks=%f,%f",m_peakLeft,m_peakRight);

            if(m_peakLeft > 1. || m_peakRight > 1.)
            {
                // qInfo("ch #%d clipping",m_channelIndex);
                m_clippingModel.setValue(true);
            }
        }

        processed();
        return;
    }

    if(true)  // tmp !m_muted)
    {
        for(FxRoute* senderRoute: m_receives)
        {
            FxChannel*  sender    = senderRoute->sender();
            FloatModel* sendModel = senderRoute->amount();
            if(!sendModel)
                qFatal("Error: no send model found from %d to %d",
                       senderRoute->senderIndex(), m_channelIndex);

            if(sender->m_hasInput || sender->m_stillRunning)
            {
                m_hasInput = true;

                // figure out if we're getting sample-exact input
                const ValueBuffer* sendBuf = sendModel->valueBuffer();
                const ValueBuffer* volBuf
                        = sender->m_volumeModel.valueBuffer();

                // mix it's output with this one's output
                sampleFrame* ch_buf = sender->m_buffer;

                // use sample-exact mixing if sample-exact values are
                // available
                if(!volBuf && !sendBuf)  // neither volume nor send has
                                         // sample-exact data...
                {
                    const real_t v = sender->m_volumeModel.value()
                                     * sendModel->value();
                    if(v > 0.)
                    {
                        if(exporting)
                        {
                            MixHelpers::addSanitizedMultiplied(
                                    m_buffer, ch_buf, v, fpp);
                        }
                        else
                        {
                            MixHelpers::addMultiplied(m_buffer, ch_buf, v,
                                                      fpp);
                        }
                    }
                    else
                        continue;
                }
                else if(volBuf && sendBuf)  // both volume and send have
                                            // sample-exact data
                {
                    if(exporting)
                    {
                        MixHelpers::addSanitizedMultipliedByBuffers(
                                m_buffer, ch_buf, volBuf, sendBuf, fpp);
                    }
                    else
                    {
                        MixHelpers::addMultipliedByBuffers(
                                m_buffer, ch_buf, volBuf, sendBuf, fpp);
                    }
                }
                else if(volBuf)  // volume has sample-exact data but send does
                                 // not
                {
                    const real_t v = sendModel->value();
                    if(v > 0.)
                    {
                        if(exporting)
                        {
                            MixHelpers::addSanitizedMultipliedByBuffer(
                                    m_buffer, ch_buf, v, volBuf, fpp);
                        }
                        else
                        {
                            MixHelpers::addMultipliedByBuffer(
                                    m_buffer, ch_buf, v, volBuf, fpp);
                        }
                    }
                    else
                        continue;
                }
                else  // vice versa
                {
                    const real_t v = sender->m_volumeModel.value();
                    if(v > 0.)
                    {
                        if(exporting)
                        {
                            MixHelpers::addSanitizedMultipliedByBuffer(
                                    m_buffer, ch_buf, v, sendBuf, fpp);
                        }
                        else
                        {
                            MixHelpers::addMultipliedByBuffer(
                                    m_buffer, ch_buf, v, sendBuf, fpp);
                        }
                    }
                    else
                        continue;
                }
            }
        }

        if(m_hasInput)
        {
            // only start fxchain when we have input...
            // m_fxChain.startRunning();
        }

        m_stillRunning
                = m_fxChain.processAudioBuffer(m_buffer, fpp, m_hasInput);

        // qWarning("FxMixer: hasInput=%d
        // stillrun=%d",m_hasInput,m_stillRunning);

        if(!isFrozen() && m_frozenBuf
           && (((song->playMode() == Song::Mode_PlaySong)
                && song->isPlaying())
               || (exporting
                   && mixer->processingSampleRate()
                              == mixer->baseSampleRate())))
        {
            for(f_cnt_t f = 0; f < fpp; ++f)
            {
                m_frozenBuf->setDataFrame(af + f, m_buffer[f][0],
                                          m_buffer[f][1]);
                /*
                if(af+f>=1000 && af+f<1004)
                        qInfo("FxChannel::doProcessing freeze to buffer"
                              " fxch=%d fb=%p af=%d s=%p ap=%p vch0=%f
                vch1=%f", m_channelIndex,m_frozenBuf,af+f,m_buffer,this,
                              m_buffer[f][0],m_buffer[f][1]);
                */
            }
        }

        if(m_eqDJ
           && /*m_stillRunning && m_hasInput &&*/ m_eqDJEnableModel.value())
        {
            // m_eqDJ->startRunning();
            m_stillRunning = m_eqDJ->processAudioBuffer(m_buffer, fpp);
        }
        // else if(m_channelIndex)
        //	qInfo("NOT processing... %p %d %d %d",m_eqDJ,m_stillRunning,
        //	      m_hasInput,m_eqDJEnableModel.value());

        const real_t v = m_volumeModel.value();
        if(v > 0.)
        {
            real_t peakLeft  = 0.;
            real_t peakRight = 0.;
            mixer->getPeakValues(m_buffer, fpp, peakLeft, peakRight);
            m_peakLeft  = qMax(m_peakLeft, peakLeft * v);
            m_peakRight = qMax(m_peakRight, peakRight * v);

            // if(m_channelIndex==1)
            //      qInfo("ch1 peaks=%f,%f",m_peakLeft,m_peakRight);

            if(m_peakLeft > 1. || m_peakRight > 1.)
            {
                // qInfo("ch #%d clipping",m_channelIndex);
                m_clippingModel.setValue(true);
            }
        }
    }
    // else
    //{
    //        m_peakRight=m_peakLeft=0.;
    //}

    // increment dependency counter of all receivers
    processed();
}

FxMixer::FxMixer() :
      Model(nullptr, "FxMixer"), JournallingObject(), m_fxRoutes(),
      m_fxChannels()
{
    // create master channel
    createChannel();
    m_lastSoloed = -1;
}

FxMixer::~FxMixer()
{
    qInfo("FxMixer::~FxMixer START");
    while(!m_fxRoutes.isEmpty())
    {
        deleteChannelSend(m_fxRoutes.first());
    }
    while(m_fxChannels.size())
    {
        FxChannel* f = m_fxChannels[m_fxChannels.size() - 1];
        m_fxChannels.pop_back();
        delete f;
    }
    qInfo("FxMixer::~FxMixer END");
}

int FxMixer::createChannel()
{
    const int index = m_fxChannels.size();
    // create new channel
    m_fxChannels.push_back(new FxChannel(index, this));

    // reset channel state
    clearChannel(index);

    return index;
}

void FxMixer::activateSolo()
{
    for(int i = 1; i < m_fxChannels.size(); ++i)
    {
        // m_fxChannels[i]->m_muteBeforeSolo
        //  = m_fxChannels[i]->mutedModel()->value();
        m_fxChannels[i]->muteForSolo();
    }
}

void FxMixer::deactivateSolo()
{
    for(int i = 1; i < m_fxChannels.size(); ++i)
    {
        // m_fxChannels[i]->mutedModel()->setValue(
        //      m_fxChannels[i]->m_muteBeforeSolo);
        m_fxChannels[i]->unmuteForSolo();
    }
}

void FxMixer::toggledSolo()
{
    int  soloedChan = -1;
    bool resetSolo
            = (m_lastSoloed >= 0) && (m_lastSoloed < m_fxChannels.size());

    // untoggle if lastsoloed is entered
    if(resetSolo)
        m_fxChannels[m_lastSoloed]->resetSolo();
    // soloModel()->setValue(false);

    // determine the soloed channel
    for(int i = 0; i < m_fxChannels.size(); ++i)
    {
        if(m_fxChannels[i]->soloModel().value() == true)
            soloedChan = i;
    }

    // if no channel is soloed, unmute everything, else mute everything
    if(soloedChan != -1)
    {
        if(resetSolo)
        {
            deactivateSolo();
            activateSolo();
        }
        else
        {
            activateSolo();
        }
        // unmute the soloed chan and every channel it sends to
        m_fxChannels[soloedChan]->unmuteForSolo();
    }
    else
    {
        deactivateSolo();
    }

    m_lastSoloed = soloedChan;
}

void FxMixer::deleteChannel(int index)
{
    qInfo("FxMixer::deleteChannel index=%d", index);

    // channel deletion is performed between mixer rounds
    Engine::mixer()->requestChangeInModel();

    // go through every instrument and adjust for the channel index change
    Tracks tracks;
    tracks += Engine::getSong()->tracks();
    tracks += Engine::getBBTrackContainer()->tracks();

    for(Track* t: tracks)
    {
        if(t->type() == Track::InstrumentTrack)
        {
            InstrumentTrack* inst = dynamic_cast<InstrumentTrack*>(t);
            int              val  = inst->effectChannelModel()->value(0);
            if(val == index)
            {
                // we are deleting this track's fx send
                // send to master
                inst->effectChannelModel()->setValue(0);
            }
            else if(val > index)
            {
                // subtract 1 to make up for the missing channel
                inst->effectChannelModel()->setValue(val - 1);
            }
        }
    }

    FxChannel* ch = m_fxChannels[index];

    // delete all of this channel's sends and receives
    while(!ch->sends().isEmpty())
    {
        deleteChannelSend(ch->sends().first());
    }
    while(!ch->receives().isEmpty())
    {
        deleteChannelSend(ch->receives().first());
    }

    // actually delete the channel
    m_fxChannels.remove(index);
    delete ch;

    // qInfo("FxMixer: delete #1 last soloed=%d",m_lastSoloed);
    if(m_lastSoloed == index)
        m_lastSoloed = -1;
    // qInfo("FxMixer: delete #2 last soloed=%d",m_lastSoloed);

    for(int i = index; i < m_fxChannels.size(); ++i)
    {
        validateChannelName(i, i + 1);

        // set correct channel index
        m_fxChannels[i]->setChannelIndex(i);

        // qInfo("FxMixer: delete #3 last soloed=%d",m_lastSoloed);
        if(m_lastSoloed == i + 1)
            m_lastSoloed = i;
        // qInfo("FxMixer: delete #4 last soloed=%d",m_lastSoloed);

        // now check all routes and update names of the send models
        for(FxRoute* r: m_fxChannels[i]->sends())
        {
            r->updateName();
        }
        for(FxRoute* r: m_fxChannels[i]->receives())
        {
            r->updateName();
        }
    }

    Engine::mixer()->doneChangeInModel();
}

void FxMixer::moveChannelLeft(int index)
{
    // can't move master or first channel
    if(index <= 1 || index >= m_fxChannels.size())
        return;

    // channels to swap
    int a = index - 1, b = index;

    // go through every instrument and adjust for the channel index change
    Tracks songTracks = Engine::getSong()->tracks();
    Tracks bbTracks   = Engine::getBBTrackContainer()->tracks();

    Tracks trackLists[] = {songTracks, bbTracks};
    for(int tl = 0; tl < 2; ++tl)
    {
        Tracks trackList = trackLists[tl];
        for(int i = 0; i < trackList.size(); ++i)
        {
            if(trackList[i]->type() == Track::InstrumentTrack)
            {
                InstrumentTrack* inst = (InstrumentTrack*)trackList[i];
                int              val  = inst->effectChannelModel()->value(0);
                if(val == a)
                {
                    inst->effectChannelModel()->setValue(b);
                }
                else if(val == b)
                {
                    inst->effectChannelModel()->setValue(a);
                }
            }
        }
    }

    // qInfo("FxMixer: move last soloed=%d",m_lastSoloed);
    if(m_lastSoloed == a)
        m_lastSoloed = b;
    else if(m_lastSoloed == b)
        m_lastSoloed = a;
    // qInfo("FxMixer: move #2 last soloed=%d",m_lastSoloed);

    // Swap positions in array
    qSwap(m_fxChannels[a], m_fxChannels[b]);

    // Update m_channelIndex of both channels
    m_fxChannels[b]->setChannelIndex(b);
    m_fxChannels[a]->setChannelIndex(a);
}

void FxMixer::moveChannelRight(int index)
{
    moveChannelLeft(index + 1);
}

FxRoute* FxMixer::createChannelSend(fx_ch_t fromChannel,
                                    fx_ch_t toChannel,
                                    real_t  amount)
{
    //	qDebug( "requested: %d to %d", fromChannel, toChannel );
    // find the existing connection
    FxChannel* from = m_fxChannels[fromChannel];
    FxChannel* to   = m_fxChannels[toChannel];

    for(int i = 0; i < from->sends().size(); ++i)
    {
        if(from->sends()[i]->receiver() == to)
        {
            // simply adjust the amount
            from->sends()[i]->amount()->setValue(amount);
            return from->sends()[i];
        }
    }

    // connection does not exist. create a new one
    return createRoute(from, to, amount);
}

FxRoute* FxMixer::createRoute(FxChannel* from, FxChannel* to, real_t amount)
{
    if(from == to)
        return nullptr;

    Engine::mixer()->requestChangeInModel();
    FxRoute* route = new FxRoute(from, to, amount);

    // add us to from's sends
    // from->sends().append(route);
    from->addSendRoute(route);

    // add us to to's receives
    // to->receives().append(route);
    to->addReceiveRoute(route);

    // add us to fxmixer's list
    Engine::fxMixer()->m_fxRoutes.append(route);
    Engine::mixer()->doneChangeInModel();

    return route;
}

// delete the connection made by createChannelSend
void FxMixer::deleteChannelSend(fx_ch_t fromChannel, fx_ch_t toChannel)
{
    // delete the send
    FxChannel* from = m_fxChannels[fromChannel];
    FxChannel* to   = m_fxChannels[toChannel];

    // find and delete the send entry
    for(int i = 0; i < from->sends().size(); ++i)
    {
        if(from->sends()[i]->receiver() == to)
        {
            deleteChannelSend(from->sends()[i]);
            break;
        }
    }
}

void FxMixer::deleteChannelSend(FxRoute* route)
{
    Engine::mixer()->requestChangeInModel();
    // remove us from from's sends
    // route->sender()->sends().remove(route->sender()->sends().indexOf(route));
    route->sender()->removeSendRoute(route);
    // remove us from to's receives
    // route->receiver()->receives().remove(route->receiver()->receives().indexOf(route));
    route->receiver()->removeReceiveRoute(route);

    // remove us from fxmixer's list
    Engine::fxMixer()->m_fxRoutes.remove(
            Engine::fxMixer()->m_fxRoutes.indexOf(route));
    delete route;
    Engine::mixer()->doneChangeInModel();
}

bool FxMixer::isInfiniteLoop(fx_ch_t sendFrom, fx_ch_t sendTo)
{
    if(sendFrom == sendTo)
        return true;
    FxChannel* from = m_fxChannels[sendFrom];
    FxChannel* to   = m_fxChannels[sendTo];
    bool       b    = checkInfiniteLoop(from, to);
    return b;
}

bool FxMixer::checkInfiniteLoop(FxChannel* from, FxChannel* to)
{
    // can't send master to anything
    if(from == m_fxChannels[0])
    {
        return true;
    }

    // can't send channel to itself
    if(from == to)
    {
        return true;
    }

    // follow sendTo's outputs recursively looking for something that sends
    // to sendFrom
    for(int i = 0; i < to->sends().size(); ++i)
    {
        if(checkInfiniteLoop(from, to->sends()[i]->receiver()))
        {
            return true;
        }
    }

    return false;
}

// how much does fromChannel send its output to the input of toChannel?
FloatModel* FxMixer::channelSendModel(fx_ch_t fromChannel, fx_ch_t toChannel)
{
    if(fromChannel == toChannel)
    {
        return nullptr;
    }
    const FxChannel* from = m_fxChannels[fromChannel];
    const FxChannel* to   = m_fxChannels[toChannel];

    for(FxRoute* route: from->sends())
    {
        if(route->receiver() == to)
        {
            return route->amount();
        }
    }

    return nullptr;
}

void FxMixer::mixToChannel(const sampleFrame* _buf, fx_ch_t _ch)
{
    FxChannel* ch = m_fxChannels[_ch];
    if(!ch->isMuted())
    {
        ch->lock();
        MixHelpers::add(ch->buffer(), _buf,
                        Engine::mixer()->framesPerPeriod());
        ch->setHasInput(true);
        ch->unlock();
    }
}

void FxMixer::prepareMasterMix()
{
    // m_fxChannels[0]->m_lock.lock();
    BufferManager::clear(m_fxChannels[0]->buffer());
    // m_fxChannels[0]->m_lock.unlock();
}

void FxMixer::masterMix(sampleFrame* _buf)
{
    const int fpp = Engine::mixer()->framesPerPeriod();

    // add the channels that have no dependencies (no incoming senders, ie.
    // no receives) to the jobqueue. The channels that have receives get
    // added when their senders get processed, which is detected by
    // dependency counting.
    // also instantly add all muted channels as they don't need to care
    // about their senders, and can just increment the deps of their
    // recipients right away.
    MixerWorkerThread::resetJobQueue(MixerWorkerThread::JobQueue::Dynamic);
    for(FxChannel* ch: m_fxChannels)
    {
        /*
        bool old=ch->m_muted;
        ch->m_muted = ch->m_mutedModel.value();
        if(old && (old!=ch->m_muted))
                BufferManager::clear(ch->m_buffer);
        */

        /*
        if( ch->m_muted ) // instantly "process" muted channels
        {
                ch->processed();
                ch->done();
        }
        else*/
        if(ch->receives().size() == 0)
        {
            ch->setQueued(true);
            MixerWorkerThread::addJob(ch);
        }
    }
    while(m_fxChannels[0]->state() != ThreadableJob::Done)
    {
        bool found = false;
        for(FxChannel* ch: m_fxChannels)
        {
            int s = ch->state();
            if(s == ThreadableJob::Queued || s == ThreadableJob::InProgress)
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            break;
        }
        MixerWorkerThread::startAndWaitForJobs();
    }

    /*
    // handle sample-exact data in master volume fader
    ValueBuffer * volBuf = m_fxChannels[0]->m_volumeModel.valueBuffer();

    if( volBuf )
    {
            for( int f = 0; f < fpp; f++ )
            {
                    m_fxChannels[0]->m_buffer[f][0] *= volBuf->values()[f];
                    m_fxChannels[0]->m_buffer[f][1] *= volBuf->values()[f];
            }
    }

    const real_t v = volBuf
            ? 1.
            : m_fxChannels[0]->m_volumeModel.value();
    MixHelpers::addSanitizedMultiplied( _buf, m_fxChannels[0]->m_buffer, v,
    fpp );
    */

    // if(MixHelpers::sanitize(m_fxChannels[0]->m_buffer,fpp))
    //        qInfo("FxMixer: sanitize #1: inf/nan found");

    const ValueBuffer* volBuf = m_fxChannels[0]->volumeModel().valueBuffer();
    if(volBuf)
    {
        MixHelpers::addMultiplied(_buf, m_fxChannels[0]->buffer(), volBuf,
                                  fpp);
    }
    else
    {
        const real_t volVal = m_fxChannels[0]->volumeModel().value();
        if(volVal == 0.)
            ;
        else if(volVal == 1.)
            MixHelpers::add(_buf, m_fxChannels[0]->buffer(), fpp);
        else
            MixHelpers::addMultiplied(_buf, m_fxChannels[0]->buffer(), volVal,
                                      fpp);
    }

    // if(MixHelpers::sanitize(_buf,fpp))
    //        qInfo("FxMixer: sanitize #2: inf/nan found");

    if(MixHelpers::isClipping(_buf, fpp))
        m_fxChannels[0]->clippingModel().setValue(true);

    // clear all channel buffers and
    // reset channel process state
    for(int i = 0; i < numChannels(); ++i)
    {
        BufferManager::clear(m_fxChannels[i]->buffer());
        m_fxChannels[i]->reset();
        m_fxChannels[i]->setQueued(false);
        m_fxChannels[i]->setHasInput(false);
        m_fxChannels[i]->resetDeps();
    }
}

void FxMixer::clear()
{
    while(m_fxChannels.size() > 1)
    {
        deleteChannel(1);
    }

    clearChannel(0);
}

void FxMixer::clearChannel(fx_ch_t index)
{
    FxChannel* ch = m_fxChannels[index];
    ch->fxChain().clear();
    ch->volumeModel().setValue(1.);
    ch->mutedModel().setValue(false);
    ch->soloModel().setValue(false);
    ch->setName((index == 0) ? tr("Master") : tr("FX %1").arg(index));

    /*
      ch->volumeModel().setDisplayName(ch->name() + ">" + tr("Volume"));
      ch->mutedModel().setDisplayName(ch->name() + ">" + tr("Mute"));
      ch->soloModel().setDisplayName(ch->name() + ">" + tr("Solo"));
    */

    // send only to master
    if(index > 0)
    {
        // delete existing sends
        while(!ch->sends().isEmpty())
        {
            deleteChannelSend(ch->sends().first());
        }

        // add send to master
        createChannelSend(index, 0);
    }

    // delete receives
    while(!ch->receives().isEmpty())
    {
        deleteChannelSend(ch->receives().first());
    }
}

// make sure we have at least num channels
void FxMixer::allocateChannelsTo(int num)
{
    while(num > m_fxChannels.size() - 1)
    {
        createChannel();

        // delete the default send to master
        deleteChannelSend(m_fxChannels.size() - 1, 0);
    }
}

void FxMixer::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    // save channels
    for(int i = 0; i < m_fxChannels.size(); ++i)
    {
        FxChannel* ch = m_fxChannels[i];

        /*
        QDomElement fxch = _doc.createElement(QString("fxchannel"));
        _this.appendChild(fxch);
        ch->fxChain().saveState(_doc, fxch);
        ch->volumeModel().saveSettings(_doc, fxch, "volume");
        ch->mutedModel().saveSettings(_doc, fxch, "muted");
        ch->soloModel().saveSettings(_doc, fxch, "soloed");
        fxch.setAttribute("num", i);
        fxch.setAttribute("name", ch->name());
        */

        QDomElement fxch = ch->saveState(_doc, _this);

        // add the channel sends
        for(int si = 0; si < ch->sends().size(); ++si)
        {
            ch->sends()[si]->saveState(_doc, fxch);
            /*
            QDomElement sendsDom = _doc.createElement(QString("send"));
            fxch.appendChild(sendsDom);

            sendsDom.setAttribute("channel",
                                  ch->sends()[si]->receiverIndex());
            ch->sends()[si]->saveSettings(_doc, sendsDom);
            // ch->sends()[si]->amount()->saveSettings(_doc, sendsDom,
            // "amount");
            */
        }
    }
}

void FxMixer::loadSettings(const QDomElement& _this)
{
    // qInfo("FxMixer::loadSettings start");
    clear();

    if(_this.tagName() != "fxmixer")
        qCritical("FxMixer::loadSettings fxmixer != %s",
                  qPrintable(_this.tagName()));
    /*
    QDomNode node = _this.firstChild();
    while(!node.isNull())
        QDomElement fxch = node.toElement();
    */

    QDomElement fxch = _this.firstChildElement("fxchannel");
    while(!fxch.isNull())
    {
        if(fxch.tagName() != "fxchannel")
            qCritical("FxMixer::loadSettings fxchannel != %s",
                      qPrintable(fxch.tagName()));

        // index of the channel we are about to load
        int num = fxch.attribute("num").toInt();
        if(num < 0 || num >= 1024)
        {
            BACKTRACE
            qCritical("FxMixer::loadSettings strange num=%d", num);
            fxch = fxch.nextSiblingElement("fxchannel");
            continue;
        }

        // allocate enough channels
        allocateChannelsTo(num);

        m_fxChannels[num]->restoreState(fxch);
        /*
        m_fxChannels[num]->volumeModel().loadSettings(fxch, "volume");
        m_fxChannels[num]->mutedModel().loadSettings(fxch, "muted");
        m_fxChannels[num]->soloModel().loadSettings(fxch, "soloed");
        m_fxChannels[num]->setName(fxch.attribute("name"));

        QDomElement e = fxch.firstChildElement("fxchain");
        if(!e.isNull())
            m_fxChannels[num]->fxChain().restoreState(e);
        // m_fxChannels[num]->m_fxChain.restoreState(fxch.firstChildElement(
        //  m_fxChannels[num]->m_fxChain.nodeName()));
        */

        // mixer sends
        QDomNodeList chData = fxch.childNodes();
        for(unsigned int i = 0; i < chData.length(); ++i)
        {
            QDomElement chDataItem = chData.at(i).toElement();
            if(!chDataItem.isNull() && chDataItem.nodeName() == "send")
            {
                int sendTo = chDataItem.attribute("channel").toInt();
                allocateChannelsTo(sendTo);
                FxRoute* fxr = createChannelSend(num, sendTo, 1.);
                if(fxr)
                    fxr->restoreState(chDataItem);
                // fxr->amount()->loadSettings(chDataItem, "amount");
            }
        }

        // node = node.nextSibling();
        fxch = fxch.nextSiblingElement("fxchannel");  //.toElement();
    }

    emit dataChanged();
    // qInfo("FxMixer::loadSettings end");
}

void FxMixer::validateChannelName(int index, int oldIndex)
{
    if(m_fxChannels[index]->name() == tr("FX %1").arg(oldIndex))
        m_fxChannels[index]->setName(tr("FX %1").arg(index));
}
