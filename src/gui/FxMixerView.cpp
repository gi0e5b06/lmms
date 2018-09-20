/*
 * FxMixerView.cpp - effect-mixer-view for LMMS
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

#include "FxMixerView.h"

//#include <QButtonGroup>
//#include <QDebug>
//#include <QInputDialog>
#include <QKeyEvent>
#include <QLayout>
//#include <QMdiArea>
//#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>

#include "BBTrackContainer.h"
#include "EffectControlDialog.h"
#include "EffectControls.h"
#include "FxLine.h"
#include "FxMixer.h"
#include "GuiApplication.h"
#include "InstrumentTrack.h"
#include "Knob.h"
#include "MainWindow.h"
#include "Mixer.h"
#include "Song.h"
#include "SubWindow.h"
#include "ToolTip.h"

#include "embed.h"
//#include "gui_templates.h"
//#include "lmms_math.h"

FxMixerView::FxMixerView() :
      QWidget(), ModelView(NULL, this), SerializingObjectHook()
{
    FxMixer* m = Engine::fxMixer();
    m->setHook(this);

    // QPalette pal = palette();
    // pal.setColor( QPalette::Background, QColor( 72, 76, 88 ) );
    // setPalette( pal );

    setAutoFillBackground(true);
    setWindowTitle(tr("FX-Mixer"));
    setWindowIcon(embed::getIconPixmap("fx_mixer"));

    // main-layout
    QHBoxLayout* ml = new QHBoxLayout;

    // Set margins
    ml->setContentsMargins(4, 6, 4, 0);
    // style()->pixelMetric( QStyle::PM_ScrollBarExtent )+2 /*16*/, 4, 0 );

    // Channel area
    m_channelAreaWidget = new QWidget;
    chLayout            = new QHBoxLayout(m_channelAreaWidget);
    chLayout->setSizeConstraint(QLayout::SetMinimumSize);
    chLayout->setSpacing(4);
    chLayout->setMargin(0);
    m_channelAreaWidget->setLayout(chLayout);

    // create rack layout before creating the first channel
    m_racksWidget = new QWidget;
    m_racksLayout = new QStackedLayout(m_racksWidget);
    m_racksLayout->setContentsMargins(0, 0, 0, 0);
    m_racksWidget->setLayout(m_racksLayout);

    // add master channel
    m_fxChannelViews.resize(m->numChannels());
    m_fxChannelViews[0] = new FxChannelView(this, this, 0);

    m_racksLayout->addWidget(m_fxChannelViews[0]->m_rackView);

    FxChannelView* masterView = m_fxChannelViews[0];
    // TMP ml->addWidget( masterView->m_fxLine, 0, Qt::AlignTop );

    QSize fxLineSize = masterView->m_fxLine->size();

    // add mixer channels
    for(int i = 1; i < m_fxChannelViews.size(); ++i)
    {
        m_fxChannelViews[i] = new FxChannelView(m_channelAreaWidget, this, i);
        chLayout->addWidget(m_fxChannelViews[i]->m_fxLine);
    }

    // add the scrolling section to the main layout
    // class solely for scroll area to pass key presses down
    class ChannelArea : public QScrollArea
    {
      public:
        ChannelArea(QWidget* parent, FxMixerView* mv) :
              QScrollArea(parent), m_mv(mv)
        {
        }
        ~ChannelArea()
        {
        }
        virtual void keyPressEvent(QKeyEvent* e)
        {
            m_mv->keyPressEvent(e);
        }

      private:
        FxMixerView* m_mv;
    };
    channelArea = new ChannelArea(this, this);
    channelArea->setWidget(m_channelAreaWidget);
    channelArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    channelArea->setFrameStyle(QFrame::NoFrame);
    channelArea->setMinimumWidth(fxLineSize.width() * 2
                                 + 4);  // typical w*8+7*4
    channelArea->setFixedHeight(
            fxLineSize.height()
            + style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    ml->addWidget(channelArea, 1, Qt::AlignTop);

    // show the add new effect channel button
    QPushButton* newChannelBtn = new QPushButton(
            embed::getIconPixmap("new_channel"), QString::null, this);
    newChannelBtn->setObjectName("newChannelBtn");
    newChannelBtn->setFixedSize(
            QSize(fxLineSize.width(), fxLineSize.width()));
    connect(newChannelBtn, SIGNAL(clicked()), this, SLOT(addNewChannel()));
    ml->addWidget(newChannelBtn, 0, Qt::AlignTop);

    ml->addWidget(masterView->m_fxLine, 0, Qt::AlignTop);

    // add the stacked layout for the effect racks of fx channels
    ml->addWidget(m_racksWidget, 0, Qt::AlignTop | Qt::AlignRight);

    setCurrentFxLine(m_fxChannelViews[0]->m_fxLine);

    setLayout(ml);
    updateGeometry();

    // timer for updating faders
    connect(gui->mainWindow(), SIGNAL(periodicUpdate()), this,
            SLOT(updateFaders()));

    // add ourself to workspace
    /*
    QMdiSubWindow * subWin = gui->mainWindow()->addWindowedWidget( this );
    Qt::WindowFlags flags = subWin->windowFlags();
    flags &= ~Qt::WindowMaximizeButtonHint;
    subWin->setWindowFlags( flags );
    layout()->setSizeConstraint( QLayout::SetMinimumSize );
    subWin->layout()->setSizeConstraint( QLayout::SetMinAndMaxSize );
    parentWidget()->setAttribute( Qt::WA_DeleteOnClose, false );
    parentWidget()->move( 5, 310 );
    */
    /*SubWindow* win=*/SubWindow::putWidgetOnWorkspace(this, false, false,
                                                       true);
    setFixedSize(size());
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    ml->setSizeConstraint(QLayout::SetMinimumSize);
    ml->setSizeConstraint(QLayout::SetMinAndMaxSize);
    // win->move(5,310);

    // we want to receive dataChanged-signals in order to update
    setModel(m);
}

FxMixerView::~FxMixerView()
{
    for(int i = 0; i < m_fxChannelViews.size(); i++)
    {
        delete m_fxChannelViews.at(i);
    }
}

int FxMixerView::addNewChannel()
{
    // add new fx mixer channel and redraw the form.
    FxMixer* mix = Engine::fxMixer();

    int newChannelIndex = mix->createChannel();
    m_fxChannelViews.push_back(
            new FxChannelView(m_channelAreaWidget, this, newChannelIndex));
    chLayout->addWidget(m_fxChannelViews[newChannelIndex]->m_fxLine);
    m_racksLayout->addWidget(m_fxChannelViews[newChannelIndex]->m_rackView);

    updateFxLine(newChannelIndex);

    updateMaxChannelSelector();

    return newChannelIndex;
}

void FxMixerView::refreshDisplay()
{
    // delete all views and re-add them
    for(int i = 1; i < m_fxChannelViews.size(); ++i)
    {
        chLayout->removeWidget(m_fxChannelViews[i]->m_fxLine);
        m_racksLayout->removeWidget(m_fxChannelViews[i]->m_rackView);
        delete m_fxChannelViews[i]->m_fader;
        delete m_fxChannelViews[i]->m_muteBtn;
        delete m_fxChannelViews[i]->m_soloBtn;
        delete m_fxChannelViews[i]->m_fxLine;
        delete m_fxChannelViews[i]->m_rackView;
        delete m_fxChannelViews[i];
    }
    m_channelAreaWidget->adjustSize();

    // re-add the views
    m_fxChannelViews.resize(Engine::fxMixer()->numChannels());
    for(int i = 1; i < m_fxChannelViews.size(); ++i)
    {
        m_fxChannelViews[i] = new FxChannelView(m_channelAreaWidget, this, i);
        chLayout->addWidget(m_fxChannelViews[i]->m_fxLine);
        m_racksLayout->addWidget(m_fxChannelViews[i]->m_rackView);
    }

    // set selected fx line to 0
    setCurrentFxLine(0);

    // update all fx lines
    for(int i = 0; i < m_fxChannelViews.size(); ++i)
    {
        updateFxLine(i);
    }

    updateMaxChannelSelector();
}

// update the max. channel number for every instrument
void FxMixerView::updateMaxChannelSelector()
{
    Tracks songTracks = Engine::getSong()->tracks();
    Tracks bbTracks   = Engine::getBBTrackContainer()->tracks();

    Tracks trackLists[] = {songTracks, bbTracks};
    for(int tl = 0; tl < 2; ++tl)
    {
        Tracks tracks = trackLists[tl];
        for(int i = 0; i < tracks.size(); ++i)
        {
            if(tracks[i]->type() == Track::InstrumentTrack)
            {
                InstrumentTrack* inst = (InstrumentTrack*)tracks[i];
                inst->effectChannelModel()->setRange(
                        0, m_fxChannelViews.size() - 1, 1);
            }
        }
    }
}

void FxMixerView::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    MainWindow::saveWidgetState(this, _this);
}

void FxMixerView::loadSettings(const QDomElement& _this)
{
    MainWindow::restoreWidgetState(this, _this);
}

FxMixerView::FxChannelView::FxChannelView(QWidget*     _parent,
                                          FxMixerView* _mv,
                                          int          channelIndex)
{
    m_fxLine = new FxLine(_parent, _mv, channelIndex);

    FxChannel* fxChannel = Engine::fxMixer()->effectChannel(channelIndex);

    m_fader = new Fader(&fxChannel->m_volumeModel,
                        tr("FX Fader %1").arg(channelIndex), m_fxLine);
    m_fader->setLevelsDisplayedInDBFS(true);
    m_fader->setMinPeak(dbfsToAmp(-42));
    m_fader->setMaxPeak(dbfsToAmp(9));

    m_fader->move(16 - m_fader->width() / 2,
                  m_fxLine->height() - m_fader->height() - 5);

    int XF=9;
    int YF=m_fader->y();

    YF-=16;
    m_soloBtn = new PixmapButton(m_fxLine, tr("Solo"));
    m_soloBtn->setModel(&fxChannel->m_soloModel);
    m_soloBtn->setActiveGraphic(embed::getIconPixmap("led_magenta"));
    m_soloBtn->setInactiveGraphic(embed::getIconPixmap("led_off"));
    m_soloBtn->setCheckable(true);
    m_soloBtn->move(XF, YF);
    connect(&fxChannel->m_soloModel, SIGNAL(dataChanged()), _mv,
            SLOT(toggledSolo()));
    ToolTip::add(m_soloBtn, tr("Solo"));

    YF-=14;
    m_muteBtn = new PixmapButton(m_fxLine, tr("Mute"));
    m_muteBtn->setModel(&fxChannel->m_mutedModel);
    m_muteBtn->setActiveGraphic(embed::getIconPixmap("led_off"));
    m_muteBtn->setInactiveGraphic(embed::getIconPixmap("led_green"));
    m_muteBtn->setCheckable(true);
    m_muteBtn->move(XF, YF);
    ToolTip::add(m_muteBtn, tr("Mute"));

    YF-=116;
    if(channelIndex == 0)
    {
        m_eqEnableBtn  = NULL;
        m_eqHighKnob   = NULL;
        m_eqMediumKnob = NULL;
        m_eqLowKnob    = NULL;
    }
    else
    {
        m_eqEnableBtn = new PixmapButton(m_fxLine, tr("Eq"));
        m_eqEnableBtn->setActiveGraphic(embed::getIconPixmap("led_yellow"));
        m_eqEnableBtn->setInactiveGraphic(embed::getIconPixmap("led_off"));
        m_eqEnableBtn->setCheckable(true);
        m_eqEnableBtn->setModel(&fxChannel->m_eqDJEnableModel);
        m_eqEnableBtn->setBlinking(true);
        m_eqEnableBtn->move(XF, YF);

        Effect*         eq = fxChannel->m_eqDJ;
        EffectControls* cq = eq->controls();
        if(cq)
        {
            EffectControlDialog* vq = cq->createView();
            if(vq)
            {
                QList<Knob*> all = vq->findChildren<Knob*>();
                if(all.size() == 3)
                {
                    m_eqHighKnob   = all[2];
                    m_eqMediumKnob = all[1];
                    m_eqLowKnob    = all[0];

                    m_eqHighKnob->setLabel("");
                    m_eqMediumKnob->setLabel("");
                    m_eqLowKnob->setLabel("");

                    // m_fxLine->layout()->addWidget(m_eqHighKnob);
                    // m_fxLine->layout()->addWidget(m_eqMediumKnob);
                    // m_fxLine->layout()->addWidget(m_eqLowKnob);
                    m_eqHighKnob->setParent(m_fxLine);
                    m_eqMediumKnob->setParent(m_fxLine);
                    m_eqLowKnob->setParent(m_fxLine);

                    m_eqHighKnob->setPointColor(Qt::yellow);
                    m_eqMediumKnob->setPointColor(Qt::yellow);
                    m_eqLowKnob->setPointColor(Qt::yellow);

                    m_eqHighKnob->move(XF-6, m_eqEnableBtn->y() + 17);
                    m_eqMediumKnob->move(XF-6, m_eqEnableBtn->y() + 47);
                    m_eqLowKnob->move(XF-6, m_eqEnableBtn->y() + 77);

                    m_eqHighKnob->model()->setRange(-100.f, 0.f, 0.5f);
                    m_eqMediumKnob->model()->setRange(-100.f, 0.f, 0.5f);
                    m_eqLowKnob->model()->setRange(-100.f, 0.f, 0.5f);
                }
                else
                    qWarning("FxChannel: not 3???");
            }
            else
                qWarning("FxChannel: EffectControlDialog* v=NULL");
        }
        else
            qWarning("FxChannel: EffectControls* c=NULL");

        /*
        m_eqHighKnob  =new Knob(knobBright_26, m_fxLine );
        m_eqMediumKnob=new Knob(knobBright_26, m_fxLine );
        m_eqLowKnob   =new Knob(knobBright_26, m_fxLine );
        m_eqHighKnob  ->setModel( &fxChannel->m_eqDJHighModel );
        m_eqMediumKnob->setModel( &fxChannel->m_eqDJMediumModel );
        m_eqLowKnob   ->setModel( &fxChannel->m_eqDJLowModel );
        //m_eqHighKnob  ->setVolumeKnob(true);
        //m_eqMediumKnob->setVolumeKnob(true);
        //m_eqLowKnob   ->setVolumeKnob(true);
        m_eqHighKnob  ->setUnit(" dB");
        m_eqMediumKnob->setUnit(" dB");
        m_eqLowKnob   ->setUnit(" dB");
        m_eqHighKnob  ->move(3, m_eqEnableBtn->y()+13);
        m_eqMediumKnob->move(3, m_eqEnableBtn->y()+43);
        m_eqLowKnob   ->move(3, m_eqEnableBtn->y()+73);
        */
    }

    YF-=14;
    m_frozenBtn = new PixmapButton(m_fxLine, tr("Frozen"));
    m_frozenBtn->setModel(&fxChannel->m_frozenModel);
    m_frozenBtn->setActiveGraphic(embed::getIconPixmap("led_blue"));
    m_frozenBtn->setInactiveGraphic(embed::getIconPixmap("led_off"));
    m_frozenBtn->setCheckable(true);
    m_frozenBtn->move(XF, YF);
    ToolTip::add(m_frozenBtn, tr("Frozen"));

    YF-=14;
    m_clippingBtn = new PixmapButton(m_fxLine, tr("Clipping"));
    m_clippingBtn->setModel(&fxChannel->m_clippingModel);
    m_clippingBtn->setActiveGraphic(embed::getIconPixmap("led_red"));
    m_clippingBtn->setInactiveGraphic(embed::getIconPixmap("led_off"));
    m_clippingBtn->setCheckable(true);
    m_clippingBtn->setBlinking(true);
    m_clippingBtn->move(XF,YF);
    ToolTip::add(m_clippingBtn, tr("Clipping alert"));

    // Create EffectRack for the channel
    m_rackView
            = new EffectRackView(&fxChannel->m_fxChain, _mv->m_racksWidget);
    //m_rackView->setFixedSize(250, FxLine::FxLineHeight);
    m_rackView->setFixedHeight(FxLine::FxLineHeight);
}

void FxMixerView::FxChannelView::setChannelIndex(int index)
{
    FxChannel* fxChannel = Engine::fxMixer()->effectChannel(index);

    m_frozenBtn->setModel(&fxChannel->m_frozenModel);
    m_clippingBtn->setModel(&fxChannel->m_clippingModel);

    m_fader->setModel(&fxChannel->m_volumeModel);
    m_muteBtn->setModel(&fxChannel->m_mutedModel);
    m_soloBtn->setModel(&fxChannel->m_soloModel);
    m_rackView->setModel(&fxChannel->m_fxChain);
}

void FxMixerView::toggledSolo()
{
    Engine::fxMixer()->toggledSolo();
}

void FxMixerView::setCurrentFxLine(FxLine* _line)
{
    // select
    m_currentFxLine = _line;
    m_racksLayout->setCurrentWidget(
            m_fxChannelViews[_line->channelIndex()]->m_rackView);

    // set up send knob
    for(int i = 0; i < m_fxChannelViews.size(); ++i)
    {
        updateFxLine(i);
    }
}

void FxMixerView::updateFxLine(int index)
{
    FxMixer* mix = Engine::fxMixer();

    // does current channel send to this channel?
    int         selIndex  = m_currentFxLine->channelIndex();
    FxLine*     thisLine  = m_fxChannelViews[index]->m_fxLine;
    FloatModel* sendModel = mix->channelSendModel(selIndex, index);
    if(sendModel == NULL)
    {
        // does not send, hide send knob
        thisLine->m_sendKnob->setVisible(false);
    }
    else
    {
        // it does send, show knob and connect
        thisLine->m_sendKnob->setVisible(true);
        thisLine->m_sendKnob->setModel(sendModel);
    }

    // disable the send button if it would cause an infinite loop
    thisLine->m_sendBtn->setVisible(!mix->isInfiniteLoop(selIndex, index));
    thisLine->m_sendBtn->updateLightStatus();
    thisLine->update();
}

void FxMixerView::deleteChannel(int index)
{
    // can't delete master
    if(index == 0)
        return;

    // remember selected line
    int selLine = m_currentFxLine->channelIndex();

    // delete the real channel
    Engine::fxMixer()->deleteChannel(index);

    // delete the view
    chLayout->removeWidget(m_fxChannelViews[index]->m_fxLine);
    m_racksLayout->removeWidget(m_fxChannelViews[index]->m_rackView);
    delete m_fxChannelViews[index]->m_fader;
    delete m_fxChannelViews[index]->m_muteBtn;
    delete m_fxChannelViews[index]->m_soloBtn;
    // delete fxLine later to prevent a crash when deleting from context menu
    m_fxChannelViews[index]->m_fxLine->hide();
    m_fxChannelViews[index]->m_fxLine->deleteLater();
    delete m_fxChannelViews[index]->m_rackView;
    delete m_fxChannelViews[index];
    m_channelAreaWidget->adjustSize();

    // make sure every channel knows what index it is
    for(int i = 0; i < m_fxChannelViews.size(); ++i)
    {
        if(i > index)
        {
            m_fxChannelViews[i]->m_fxLine->setChannelIndex(i - 1);
        }
    }
    m_fxChannelViews.remove(index);

    // select the next channel
    if(selLine >= m_fxChannelViews.size())
    {
        selLine = m_fxChannelViews.size() - 1;
    }
    setCurrentFxLine(selLine);

    updateMaxChannelSelector();
}

void FxMixerView::deleteUnusedChannels()
{
    Tracks tracks;
    tracks += Engine::getSong()->tracks();
    tracks += Engine::getBBTrackContainer()->tracks();

    // go through all FX Channels
    for(int i = m_fxChannelViews.size() - 1; i > 0; --i)
    {
        // check if an instrument references to the current channel
        bool empty = true;
        for(Track* t : tracks)
        {
            if(t->type() == Track::InstrumentTrack)
            {
                InstrumentTrack* inst = dynamic_cast<InstrumentTrack*>(t);
                if(i == inst->effectChannelModel()->value(0))
                {
                    empty = false;
                    break;
                }
            }
        }
        FxChannel* ch = Engine::fxMixer()->effectChannel(i);
        // delete channel if no references found
        if(empty && ch->m_receives.isEmpty())
        {
            deleteChannel(i);
        }
    }
}

void FxMixerView::moveChannelLeft(int index, int focusIndex)
{
    // can't move master or first channel left or last channel right
    if(index <= 1 || index >= m_fxChannelViews.size())
        return;

    FxMixer* m = Engine::fxMixer();

    // Move instruments channels
    m->moveChannelLeft(index);

    // Update widgets models
    m_fxChannelViews[index]->setChannelIndex(index);
    m_fxChannelViews[index - 1]->setChannelIndex(index - 1);

    // Focus on new position
    setCurrentFxLine(focusIndex);
}

void FxMixerView::moveChannelLeft(int index)
{
    moveChannelLeft(index, index - 1);
}

void FxMixerView::moveChannelRight(int index)
{
    moveChannelLeft(index + 1, index + 1);
}

void FxMixerView::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
        case Qt::Key_Delete:
            deleteChannel(m_currentFxLine->channelIndex());
            break;
        case Qt::Key_Left:
            if(e->modifiers() & Qt::AltModifier)
            {
                moveChannelLeft(m_currentFxLine->channelIndex());
            }
            else
            {
                // select channel to the left
                setCurrentFxLine(m_currentFxLine->channelIndex() - 1);
            }
            break;
        case Qt::Key_Right:
            if(e->modifiers() & Qt::AltModifier)
            {
                moveChannelRight(m_currentFxLine->channelIndex());
            }
            else
            {
                // select channel to the right
                setCurrentFxLine(m_currentFxLine->channelIndex() + 1);
            }
            break;
        case Qt::Key_Insert:
            if(e->modifiers() & Qt::ShiftModifier)
            {
                addNewChannel();
            }
            break;
    }
}

void FxMixerView::closeEvent(QCloseEvent* _ce)
{
    if(parentWidget())
    {
        parentWidget()->hide();
    }
    else
    {
        hide();
    }
    _ce->ignore();
}

void FxMixerView::setCurrentFxLine(int _line)
{
    if(_line >= 0 && _line < m_fxChannelViews.size())
    {
        setCurrentFxLine(m_fxChannelViews[_line]->m_fxLine);
    }
}

void FxMixerView::clear()
{
    Engine::fxMixer()->clear();

    refreshDisplay();
}

void FxMixerView::updateFaders()
{
    FxMixer* m = Engine::fxMixer();

    // apply master gain
    m->effectChannel(0)->m_peakLeft *= Engine::mixer()->masterGain();
    m->effectChannel(0)->m_peakRight *= Engine::mixer()->masterGain();

    for(int i = 0; i < m_fxChannelViews.size(); ++i)
    {
        const float opl      = m_fxChannelViews[i]->m_fader->getPeak_L();
        const float opr      = m_fxChannelViews[i]->m_fader->getPeak_R();
        const float fall_off = 1.2;

        if(m->effectChannel(i)->m_peakLeft > opl)
        {
            m_fxChannelViews[i]->m_fader->setPeak_L(
                    m->effectChannel(i)->m_peakLeft);
            m->effectChannel(i)->m_peakLeft = 0;
        }
        else
        {
            m_fxChannelViews[i]->m_fader->setPeak_L(opl / fall_off);
        }

        if(m->effectChannel(i)->m_peakRight > opr)
        {
            m_fxChannelViews[i]->m_fader->setPeak_R(
                    m->effectChannel(i)->m_peakRight);
            m->effectChannel(i)->m_peakRight = 0;
        }
        else
        {
            m_fxChannelViews[i]->m_fader->setPeak_R(opr / fall_off);
        }

        //if(m_fxChannelViews[i]->m_fader->needsUpdate())
        //    m_fxChannelViews[i]->m_fader->update();
    }
}
