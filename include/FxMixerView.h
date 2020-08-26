/*
 * FxMixerView.h - effect-mixer-view for LMMS
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

#ifndef FX_MIXER_VIEW_H
#define FX_MIXER_VIEW_H

//#include "ModelView.h"
//#include "Engine.h"
//#include "Effect.h"
#include "Fader.h"
#include "FxLine.h"
#include "FxMixer.h"
//#include "Knob.h"
#include "PixmapButton.h"
//#include "ToolTip.h"
//#include "embed.h"
#include "EffectChainView.h"
#include "debug.h"

//#include <QWidget>
#include <QHBoxLayout>
#include <QLine>
//#include <QPointer>
#include <QScrollArea>
#include <QStackedLayout>

class QButtonGroup;
// class FxLine;
// class FxChannel;
class FxChannelView;

typedef DebugQVector<QPointer<FxChannelView>> FxChannelViews;

class EXPORT FxMixerView :
      public QWidget,
      public ModelView,
      public SerializingObjectHook
{
    Q_OBJECT

  public:
    FxMixerView();
    virtual ~FxMixerView();

    void keyPressEvent(QKeyEvent* _ke) override;

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _this);
    virtual void loadSettings(const QDomElement& _this);

    INLINE FxChannelView* channelView(int index)
    {
        return m_fxChannelViews[index];
    }

    FxLine* currentFxLine();
    int     currentLine();
    void    setCurrentLine(int _line);

    void clear();

    // display the send button and knob correctly
    void updateFxLine(int index);

    // notify the view that an fx channel was deleted
    void deleteChannel(int index);

    // delete all unused channels
    void deleteUnusedChannels();

    // move the channel to the left or right
    void moveChannelLeft(int index);
    void moveChannelLeft(int index, int focusIndex);
    void moveChannelRight(int index);

    // make sure the display syncs up with the fx mixer.
    // useful for loading projects
    void refreshDisplay();

    virtual QList<ModelView*> childModelViews() const;

  public slots:
    int  addNewChannel();
    void setCurrentLine();

  protected:
    void closeEvent(QCloseEvent* _ce) override;

    void addChannelView(fx_ch_t _chIndex);
    void removeChannelView(fx_ch_t _chIndex);
    void updateIndexes();

  private slots:
    void updateFaders();
    void toggledSolo();

  private:
    IntModel          m_currentLine;
    FxChannelViews    m_fxChannelViews;
    QScrollArea*      channelArea;
    QHBoxLayout*      chLayout;
    QWidget*          m_channelAreaWidget;
    QStackedLayout*   m_racksLayout;
    QPointer<QWidget> m_racksWidget;

    void updateMaxChannelSelector();

    friend class FxChannelView;
};

class FxChannelView : public QObject, public ModelView
{
    Q_OBJECT

  public:
    /*
    INLINE FxChannel* model()  // non virtual
    {
        return dynamic_cast<FxChannel*>(ModelView::model());
    }

    INLINE const FxChannel* model() const // non virtual
    {
        return dynamic_cast<const FxChannel*>(ModelView::model());
    }

    INLINE FxLine* widget()  // non virtual
    {
        return dynamic_cast<FxLine*>(ModelView::widget());
    }

    INLINE const FxLine* widget() const // non virtual
    {
        return dynamic_cast<const FxLine*>(ModelView::widget());
    }
    */

    void setChannelIndex(fx_ch_t _index);

    QLine  cableFrom() const override;
    QLine  cableTo() const override;
    QColor cableColor() const override;

    static bool lessThan(FxChannelView* _a, FxChannelView* _b)
    {
        if(_a == nullptr)
            return false;
        if(_b == nullptr)
            return true;
        return _a->m_fxLine->channelIndex() < _b->m_fxLine->channelIndex();
    }

  protected:
    FxChannelView(QWidget* _parent, FxMixerView* _mv, fx_ch_t _chIndex);
    virtual ~FxChannelView();

  private:
    PixmapButton*             m_frozenBtn;
    PixmapButton*             m_clippingBtn;
    PixmapButton*             m_eqEnableBtn;
    Knob*                     m_eqHighKnob;
    Knob*                     m_eqMediumKnob;
    Knob*                     m_eqLowKnob;
    QPointer<FxLine>          m_fxLine;
    PixmapButton*             m_muteBtn;
    PixmapButton*             m_soloBtn;
    Fader*                    m_fader;
    QPointer<EffectChainView> m_rackView;
    QPointer<FxMixerView>     m_fxmv;

    friend class FxMixerView;
};

#endif
