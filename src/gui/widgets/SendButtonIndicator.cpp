#include "SendButtonIndicator.h"

#include "FxMixer.h"
#include "FxMixerView.h"
#include "embed.h"

static PixmapLoader s_qpmOff("mixer_send_off");
static PixmapLoader s_qpmOn("mixer_send_on");

SendButtonIndicator::SendButtonIndicator(QWidget*  _parent,
                                         IntModel* _currentLine,
                                         IntModel* _channelIndex) :
      QLabel(_parent),
      m_currentLine(_currentLine), m_channelIndex(_channelIndex)
{
    /*
    if(s_qpmOff == nullptr)
        s_qpmOff
                = new QPixmap(embed::getPixmap("mixer_send_off", 29, 20));

    if(s_qpmOn == nullptr)
        s_qpmOn = new QPixmap(embed::getPixmap("mixer_send_on", 29, 20));
    */

    // don't do any initializing yet, because the FxMixerView and FxLine
    // that were passed to this constructor are not done with their
    // constructors yet.
    setPixmap(s_qpmOff);
}

void SendButtonIndicator::mousePressEvent(QMouseEvent* e)
{
    const int  from      = m_currentLine->value();
    const int  to        = m_channelIndex->value();
    FxMixer*   mix       = Engine::fxMixer();
    RealModel* sendModel = mix->channelSendModel(from, to);
    if(sendModel == nullptr)
    {
        // not sending. create a mixer send.
        mix->createChannelSend(from, to);
    }
    else
    {
        // sending. delete the mixer send.
        mix->deleteChannelSend(from, to);
    }

    emit sendModelChanged(m_channelIndex->value());
    // m_mv->updateFxLine(m_owner->channelIndex());
    updateLightStatus();
}

RealModel* SendButtonIndicator::sendModel()
{
    FxMixer* mix = Engine::fxMixer();
    return mix->channelSendModel(m_currentLine->value(),
                                 m_channelIndex->value());
}

void SendButtonIndicator::updateLightStatus()
{
    setPixmap(sendModel() == nullptr ? s_qpmOff : s_qpmOn);
}
