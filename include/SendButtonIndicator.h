#ifndef SENDBUTTONINDICATOR_H
#define SENDBUTTONINDICATOR_H

#include "AutomatableModel.h"

//#include <QDebug>
#include <QLabel>
#include <QPointer>
//#include <QPixmap>

//#include "FxLine.h"
//#include "FxMixerView.h"

// class FxLine;
// class FxMixerView;

class SendButtonIndicator : public QLabel
{
    Q_OBJECT

  public:
    SendButtonIndicator(QWidget*  _parent,
                        IntModel* _currentLine,
                        IntModel* _channelIndex);

    virtual void updateLightStatus() final;

  signals:
    void sendModelChanged(int);

  protected:
    void mousePressEvent(QMouseEvent* e) override;

  private:
    RealModel* sendModel();

    QPointer<IntModel> m_currentLine;
    QPointer<IntModel> m_channelIndex;
};

#endif  // SENDBUTTONINDICATOR_H
