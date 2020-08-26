/*
 * FxMixerView.cpp - effect-mixer-view for LMMS
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "FxMixerView.h"

//#include <QButtonGroup>
//#include <QDebug>
//#include <QInputDialog>
#include <QKeyEvent>
#include <QLayout>
//#include <QMdiArea>
//#include <QPainter>
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

#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
//#include "gui_templates.h"
//#include "lmms_math.h"

FxMixerView::FxMixerView() :
      QWidget(), ModelView(Engine::fxMixer() /*nullptr*/, this),
      SerializingObjectHook(), m_currentLine(0,
                                             0,
                                             0,
                                             nullptr,
                                             tr("Current channel"),
                                             "currentChannel",
                                             false)
{
    // FxMixer* m = Engine::fxMixer();
    FxMixer* m = dynamic_cast<FxMixer*>(model());
    m->setHook(this);

    // QPalette pal = palette();
    // pal.setColor( QPalette::Background, QColor( 72, 76, 88 ) );
    // setPalette( pal );

    setAutoFillBackground(true);
    setWindowTitle(tr("Mixer"));
    setWindowIcon(embed::getIcon("fx_mixer"));

    // main-layout
    QHBoxLayout* ml = new QHBoxLayout;

    // Set margins
    ml->setContentsMargins(4, 6, 4, 0);
    // style()->pixelMetric( QStyle::PM_ScrollBarExtent )+2 /*16*/, 4, 0 );

    // Channel area
    m_channelAreaWidget = new QWidget();
    chLayout            = new QHBoxLayout(m_channelAreaWidget);
    chLayout->setSizeConstraint(QLayout::SetMinimumSize);
    chLayout->setSpacing(4);
    chLayout->setMargin(0);
    m_channelAreaWidget->setLayout(chLayout);

    // create rack layout before creating the first channel
    m_racksWidget = new QWidget();
    m_racksLayout = new QStackedLayout(m_racksWidget);
    m_racksLayout->setContentsMargins(0, 0, 0, 0);
    m_racksWidget->setLayout(m_racksLayout);

    // add master channel
    m_fxChannelViews.resize(m->numChannels());

    /*
    m_fxChannelViews[0] = new FxChannelView(this, this, 0);
    m_racksLayout->addWidget(m_fxChannelViews[0]->m_rackView);
    */
    addChannelView(0);
    FxChannelView* masterView = m_fxChannelViews[0];
    // TMP ml->addWidget( masterView->m_fxLine, 0, Qt::AlignTop );
    QSize fxLineSize = masterView->m_fxLine->size();

    // add mixer channels
    for(int i = 1; i < m_fxChannelViews.size(); ++i)
    {
        addChannelView(i);
        // m_fxChannelViews[i] = new FxChannelView(m_channelAreaWidget, this,
        // i); chLayout->addWidget(m_fxChannelViews[i]->m_fxLine);
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
    channelArea->setMinimumWidth(fxLineSize.width() * 8 + 7 * 4);
    channelArea->setFixedHeight(
            fxLineSize.height()
            + style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    /*
    channelArea->resize(
            fxLineSize.width() * 8 + 7 * 4,
            fxLineSize.height()
                    + style()->pixelMetric(QStyle::PM_ScrollBarExtent));
    */
    ml->addWidget(channelArea, 1, Qt::AlignTop);

    // show the add new effect channel button
    QPushButton* newChannelBtn = new QPushButton(
            embed::getIconPixmap("new_channel"), QString::null, this);
    // newChannelBtn->setObjectName("newChannelButton");
    const int wpb = fxLineSize.width();
    newChannelBtn->setFixedSize(wpb, wpb);
    connect(newChannelBtn, SIGNAL(clicked()), this, SLOT(addNewChannel()));
    ml->addWidget(newChannelBtn, 0, Qt::AlignTop);

    ml->addWidget(masterView->m_fxLine, 0, Qt::AlignTop);

    // add the stacked layout for the effect racks of fx channels
    ml->addWidget(m_racksWidget, 0, Qt::AlignTop | Qt::AlignRight);

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
    // setModel(m);

    connect(&m_currentLine, SIGNAL(dataChanged()), this,
            SLOT(setCurrentLine()));
    setCurrentLine(0);  // m_fxChannelViews[0]->m_fxLine);
}

FxMixerView::~FxMixerView()
{
    /*
    for(int i = 0; i < m_fxChannelViews.size(); i++)
    {
        delete m_fxChannelViews[i];
        m_fxChannelViews[i] = nullptr;
    }
    */
    m_fxChannelViews.clear();
}

int FxMixerView::addNewChannel()
{
    qInfo("FxMixerView::addNewChannel 1");
    // add new fx mixer channel and redraw the form.
    FxMixer* mix = Engine::fxMixer();

    int newChannelIndex = mix->createChannel();
    qInfo("FxMixerView::addNewChannel 2");
    addChannelView(newChannelIndex);
    /*
    m_fxChannelViews.push_back(
            new FxChannelView(m_channelAreaWidget, this, newChannelIndex));
    chLayout->addWidget(m_fxChannelViews[newChannelIndex]->m_fxLine);
    m_racksLayout->addWidget(m_fxChannelViews[newChannelIndex]->m_rackView);
    */

    qInfo("FxMixerView::addNewChannel 3");
    updateMaxChannelSelector();

    qInfo("FxMixerView::addNewChannel 4");
    updateFxLine(newChannelIndex);

    qInfo("FxMixerView::addNewChannel 5");
    return newChannelIndex;
}

void FxMixerView::refreshDisplay()
{
    qInfo("FxMixerView::refreshDisplay");

    // delete all views and re-add them
    for(int i = 1; i < m_fxChannelViews.size(); ++i)
    {
        removeChannelView(i);
        // chLayout->removeWidget(m_fxChannelViews[i]->m_fxLine);
        // m_racksLayout->removeWidget(m_fxChannelViews[i]->m_rackView);
        /*
        delete m_fxChannelViews[i]->m_fader;
        delete m_fxChannelViews[i]->m_muteBtn;
        delete m_fxChannelViews[i]->m_soloBtn;
        */
        // delete m_fxChannelViews[i]->m_fxLine;
        // delete m_fxChannelViews[i]->m_rackView;
        // delete m_fxChannelViews[i];
    }
    m_channelAreaWidget->adjustSize();

    // re-add the views
    m_fxChannelViews.resize(Engine::fxMixer()->numChannels());
    for(int i = 1; i < m_fxChannelViews.size(); ++i)
    {
        addChannelView(i);
        /*
        m_fxChannelViews[i] = new FxChannelView(m_channelAreaWidget, this, i);
        chLayout->addWidget(m_fxChannelViews[i]->m_fxLine);
        m_racksLayout->addWidget(m_fxChannelViews[i]->m_rackView);
        */
    }

    // set selected fx line to 0
    setCurrentLine(0);
    updateIndexes();

    // update all fx lines
    for(int i = 0; i < m_fxChannelViews.size(); ++i)
        updateFxLine(i);
}

// update the max. channel number for every instrument
void FxMixerView::updateMaxChannelSelector()
{
    m_currentLine.setRange(0, m_fxChannelViews.size() - 1, 1);

    Tracks songTracks = Engine::getSong()->tracks();
    Tracks bbTracks   = Engine::getBBTrackContainer()->tracks();

    Tracks trackLists[] = {songTracks, bbTracks};
    for(int tl = 0; tl < 2; ++tl)
    {
        Tracks& tracks = trackLists[tl];
        for(int i = 0; i < tracks.size(); ++i)
        {
            if(tracks[i]->type() == Track::InstrumentTrack)
            {
                InstrumentTrack* inst
                        = qobject_cast<InstrumentTrack*>(tracks[i]);
                Q_ASSERT(inst != nullptr);
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

QList<ModelView*> FxMixerView::childModelViews() const
{
    QList<ModelView*> r;
    for(FxChannelView* chv: m_fxChannelViews)
        r.append(chv);
    return r;
}

void FxMixerView::addChannelView(fx_ch_t i)
{
    const int s = m_fxChannelViews.size();
    if(i >= s)
    {
        if(s != i)
            qWarning("FxMixerView::addChannelView i=%d s=%d", i, s);
        m_fxChannelViews.resize(i + 1);
        for(int j = s; j < i + 1; j++)
            m_fxChannelViews[j] = nullptr;
    }

    if(m_fxChannelViews[i] == nullptr)
    {
        /*m_fxChannelViews[i] =*/new FxChannelView(m_channelAreaWidget, this,
                                                   i);
        // chLayout->addWidget(m_fxChannelViews[i]->m_fxLine);
        // m_racksLayout->addWidget(m_fxChannelViews[i]->m_rackView);
    }
    else
    {
        BACKTRACE
        qWarning("FxMixerView::addChannelView ch %d not null", i);
    }
}

void FxMixerView::removeChannelView(fx_ch_t i)
{
    if(i < 0 || i >= m_fxChannelViews.size())
    {
        BACKTRACE
        qWarning("FxMixerView::removeChannelView bad index i=%d [0-%d]", i,
                 m_fxChannelViews.size() - 1);
        return;
    }

    if(m_fxChannelViews[i] != nullptr)
    {
        // chLayout->removeWidget(m_fxChannelViews[i]->m_fxLine);
        // m_racksLayout->removeWidget(m_fxChannelViews[i]->m_rackView);
        delete m_fxChannelViews[i];
        /*m_fxChannelViews[i] = nullptr;*/
    }
    else
    {
        BACKTRACE
        qWarning("FxMixerView::removeChannelView ch %d already null", i);
    }
}

void FxMixerView::updateIndexes()
{
    qInfo("FxMixerView::updateIndexes START");
    int i = 0, j = 0, s = -1;
    for(FxChannelView* cv: m_fxChannelViews)
    {
        if(cv == nullptr)
        {
            j++;
            continue;
        }
        if(i != j)
        {
            m_fxChannelViews[i] = cv;
            m_fxChannelViews[j] = nullptr;
            if(currentLine() == j)
                s = i;
        }
        cv->m_fxLine->setChannelIndex(i);
        i++;
        j++;
    }
    if(i != m_fxChannelViews.size())
    {
        m_fxChannelViews.resize(i);
        updateMaxChannelSelector();
    }
    if(s >= 0)
        setCurrentLine(s);
    qInfo("FxMixerView::updateIndexes i=%d j=%d END", i, j);
}

QLine FxChannelView::cableFrom() const
{
    const QWidget* w = widget();
    if(w == nullptr)
        return QLine();

    QPoint p(w->width() / 2, 0);
    return QLine(p, p + QPoint(0, -50));
}

QLine FxChannelView::cableTo() const
{
    const QWidget* w = widget();
    if(w == nullptr)
        return QLine();

    QPoint p(w->width() / 2, w->height());
    return QLine(p, p + QPoint(0, 50));
}

QColor FxChannelView::cableColor() const
{
    return Qt::white;
}

FxChannelView::FxChannelView(QWidget*     _parent,
                             FxMixerView* _fxmv,
                             fx_ch_t      _channelIndex) :
      ModelView(Engine::fxMixer()->effectChannel(_channelIndex),
                new FxLine(_parent, &_fxmv->m_currentLine, _channelIndex)),
      m_fxmv(_fxmv)
{
    m_fxLine = castWidget<FxLine>();
    // setWidget(m_fxLine);  // GDX
    FxChannel* fxChannel = castModel<FxChannel>();
    // setModel(fxChannel);  // GDX

    m_fader = new Fader(&fxChannel->volumeModel(),
                        QObject::tr("FX Fader %1").arg(_channelIndex),
                        m_fxLine);
    m_fader->setLevelsDisplayedInDBFS(true);
    m_fader->setMinPeak(dbfsToAmp(-42));
    m_fader->setMaxPeak(dbfsToAmp(9));

    m_fader->move(16 - m_fader->width() / 2,
                  m_fxLine->height() - m_fader->height() - 5);

    int XF = 9;
    int YF = m_fader->y();

    YF -= 16;
    m_soloBtn = new PixmapButton(m_fxLine, QObject::tr("Solo"));
    m_soloBtn->setModel(&fxChannel->soloModel());
    m_soloBtn->setActiveGraphic(embed::getPixmap("led_magenta"));
    m_soloBtn->setInactiveGraphic(embed::getPixmap("led_off"));
    m_soloBtn->setCheckable(true);
    m_soloBtn->move(XF, YF);
    QObject::connect(&fxChannel->soloModel(), SIGNAL(dataChanged()), _fxmv,
                     SLOT(toggledSolo()));
    ToolTip::add(m_soloBtn, QObject::tr("Solo"));

    YF -= 14;
    m_muteBtn = new PixmapButton(m_fxLine, QObject::tr("Mute"));
    m_muteBtn->setModel(&fxChannel->mutedModel());
    m_muteBtn->setActiveGraphic(embed::getPixmap("led_off"));
    m_muteBtn->setInactiveGraphic(embed::getPixmap("led_green"));
    m_muteBtn->setCheckable(true);
    m_muteBtn->move(XF, YF);
    ToolTip::add(m_muteBtn, QObject::tr("Mute"));

    YF -= 116;
    if(_channelIndex == 0)
    {
        m_eqEnableBtn  = nullptr;
        m_eqHighKnob   = nullptr;
        m_eqMediumKnob = nullptr;
        m_eqLowKnob    = nullptr;
    }
    else
    {
        m_eqEnableBtn = new PixmapButton(m_fxLine, QObject::tr("Eq"));
        m_eqEnableBtn->setActiveGraphic(embed::getPixmap("led_yellow"));
        m_eqEnableBtn->setInactiveGraphic(embed::getPixmap("led_off"));
        m_eqEnableBtn->setCheckable(true);
        m_eqEnableBtn->setModel(&fxChannel->m_eqDJEnableModel);
        m_eqEnableBtn->setBlinking(true);
        m_eqEnableBtn->move(XF, YF);

        Effect*         eq = fxChannel->m_eqDJ;
        EffectControls* cq = eq->controls();
        if(cq)
        {
            EffectControlDialog* vq = cq->createView();
            if(vq != nullptr)
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

                    m_eqHighKnob->move(XF - 6, m_eqEnableBtn->y() + 17);
                    m_eqMediumKnob->move(XF - 6, m_eqEnableBtn->y() + 47);
                    m_eqLowKnob->move(XF - 6, m_eqEnableBtn->y() + 77);

                    m_eqHighKnob->model()->setRange(-100.f, 0.f, 0.5f);
                    m_eqMediumKnob->model()->setRange(-100.f, 0.f, 0.5f);
                    m_eqLowKnob->model()->setRange(-100.f, 0.f, 0.5f);
                }
                else
                    qWarning("FxChannel: not 3???");
            }
            else
                qWarning("FxChannel: EffectControlDialog* v=nullptr");
        }
        else
            qWarning("FxChannel: EffectControls* c=nullptr");

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

    YF -= 14;
    m_frozenBtn = new PixmapButton(m_fxLine, QObject::tr("Frozen"));
    m_frozenBtn->setModel(&fxChannel->frozenModel());
    m_frozenBtn->setActiveGraphic(embed::getIconPixmap("led_blue"));
    m_frozenBtn->setInactiveGraphic(embed::getIconPixmap("led_off"));
    m_frozenBtn->setCheckable(true);
    m_frozenBtn->move(XF, YF);
    ToolTip::add(m_frozenBtn, QObject::tr("Frozen"));

    YF -= 14;
    m_clippingBtn = new PixmapButton(m_fxLine, QObject::tr("Clipping"));
    m_clippingBtn->setModel(&fxChannel->clippingModel());
    m_clippingBtn->setActiveGraphic(embed::getIconPixmap("led_red"));
    m_clippingBtn->setInactiveGraphic(embed::getIconPixmap("led_off"));
    m_clippingBtn->setCheckable(true);
    m_clippingBtn->setBlinking(true);
    m_clippingBtn->move(XF, YF);
    ToolTip::add(m_clippingBtn, QObject::tr("Clipping alert"));

    m_rackView = new EffectChainView(&fxChannel->fxChain(),
                                     _fxmv->m_racksWidget);
    // m_rackView->setFixedSize(250, FxLine::FxLineHeight);
    m_rackView->setFixedHeight(FxLine::FxLineHeight);

    m_fxmv->m_fxChannelViews[_channelIndex] = this;
    m_fxmv->chLayout->addWidget(m_fxLine);
    m_fxmv->m_racksLayout->addWidget(m_rackView);
    m_fxmv->updateIndexes();
}

FxChannelView::~FxChannelView()
{
    qInfo("FxChannelView::~FxChannelView 1");
    int channelIndex                       = m_fxLine->channelIndex();
    m_fxmv->m_fxChannelViews[channelIndex] = nullptr;
    m_fxmv->updateIndexes();
    qInfo("FxChannelView::~FxChannelView 2");
    if(m_fxLine != nullptr)
    {
        // m_fxLine->setChannelIndex(-1);
        m_fxLine->hide();
        // m_fxmv->chLayout->removeWidget(m_fxLine);
        DELETE_HELPER(m_fxLine);
    }
    qInfo("FxChannelView::~FxChannelView 3");
    if(m_rackView != nullptr)
    {
        m_rackView->hide();
        // m_fxmv->m_racksLayout->removeWidget(m_rackView);
        DELETE_HELPER(m_rackView);
    }
    // qInfo("FxChannelView::~FxChannelView 6");
    // m_fxmv->m_fxChannelViews.removeOne(this);  //(channelIndex);
    // BACKTRACE
    m_fxmv->m_channelAreaWidget->adjustSize();
    qInfo("FxChannelView::~FxChannelView 4");
}

void FxChannelView::setChannelIndex(fx_ch_t index)
{
    FxChannel* fxChannel = Engine::fxMixer()->effectChannel(index);

    m_frozenBtn->setModel(&fxChannel->frozenModel());
    m_clippingBtn->setModel(&fxChannel->clippingModel());

    m_fader->setModel(&fxChannel->volumeModel());
    m_muteBtn->setModel(&fxChannel->mutedModel());
    m_soloBtn->setModel(&fxChannel->soloModel());
    m_rackView->setModel(&fxChannel->fxChain());
}

void FxMixerView::toggledSolo()
{
    Engine::fxMixer()->toggledSolo();
}

FxLine* FxMixerView::currentFxLine()
{
    return m_fxChannelViews[currentLine()]->m_fxLine;
}

int FxMixerView::currentLine()
{
    int index = m_currentLine.value();
    if(index < 0 || index >= m_fxChannelViews.size())
    {
        BACKTRACE
        qCritical("FxMixerView::currentFxLine invalid index %d [0-%d]", index,
                  m_fxChannelViews.size() - 1);
        index = qBound(0, index, m_fxChannelViews.size() - 1);
    }
    return index;
}

void FxMixerView::setCurrentLine()
{
    setCurrentLine(currentLine());
}

void FxMixerView::setCurrentLine(int index)
{
    if(index < 0 || index >= m_fxChannelViews.size())
    {
        BACKTRACE
        qWarning("FxMixerView::setCurrentLine invalid index %d", index);
        index = qBound(0, index, m_fxChannelViews.size() - 1);
    }

    // select
    // BACKTRACE
    qWarning("index=%d line=%p rack=%p cv=%p", index,
             m_fxChannelViews[index]->m_fxLine.data(),
             m_fxChannelViews[index]->m_rackView.data(),
             m_fxChannelViews[index].data());
    m_currentLine.setValue(index);
    m_racksLayout->setCurrentWidget(m_fxChannelViews[index]->m_rackView);

    // set up send knob
    for(int i = 0; i < m_fxChannelViews.size(); ++i)
        updateFxLine(i);
}

void FxMixerView::updateFxLine(int index)
{
    qInfo("FxMixerView::updateFxLine index=%d START", index);

    if(index < 0 || index >= m_fxChannelViews.size())
    {
        BACKTRACE
        qWarning("FxMixerView::updateFxLine invalid index=%d", index);
        return;
    }

    m_fxChannelViews[index]->m_fxLine->refresh();
}

/*
int selIndex = currentLine();
if(selIndex < 0 || selIndex >= m_fxChannelViews.size())
{
    BACKTRACE
    qWarning("FxMixerView::updateFxLine invalid m_currentLine=%d",
             selIndex);
    selIndex = 0;
    m_currentLine.setValue(0);
    return;
}

// does current channel send to this channel?
FxLine* thisLine = m_fxChannelViews[index]->m_fxLine;

if(thisLine == nullptr)
    qWarning("updateFxLine: thisLine is null");
if(thisLine->m_sendKnob == nullptr)
    qWarning("updateFxLine: thisLine->m_sendKnob is null");
if(thisLine->m_sendBtn == nullptr)
    qWarning("updateFxLine: thisLine->m_sendBtn is null");

FxMixer*   mix       = Engine::fxMixer();
RealModel* sendModel = mix->channelSendModel(selIndex, index);
if(sendModel == nullptr)
{
    // does not send, hide send knob
    thisLine->m_sendKnob->setVisible(false);
    thisLine->m_sendKnob->setModel(RealModel::createDefaultConstructed());
}
else
{
    // it does send, show knob and connect
    thisLine->m_sendKnob->setVisible(true);
    if(thisLine->m_sendKnob->model() != sendModel)
        thisLine->m_sendKnob->setModel(sendModel);
}

// disable the send button if it would cause an infinite loop
thisLine->m_sendBtn->setVisible(!mix->isInfiniteLoop(selIndex, index));
thisLine->m_sendBtn->updateLightStatus();
thisLine->update();

qInfo("FxMixerView::updateFxLine index=%d END", index);
*/

void FxMixerView::deleteChannel(int index)
{
    // can't delete master
    if(index == 0)
        return;

    qInfo("FxMixerView::deleteChannel index=%d START", index);
    // remember selected line

    if(currentLine() == index)
        setCurrentLine(index - 1);

    // delete the real channel
    Engine::fxMixer()->deleteChannel(index);

    // delete the view
    /*
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
    */
    // removeChannelView(index);
    // updateIndexes();
    qInfo("FxMixerView::deleteChannel index=%d END", index);
}

void FxMixerView::deleteUnusedChannels()
{
    Tracks tracks;
    tracks += Engine::song()->tracks();
    tracks += Engine::getBBTrackContainer()->tracks();

    // go through all FX Channels
    for(int i = m_fxChannelViews.size() - 1; i > 0; --i)
    {
        // check if an instrument references to the current channel
        bool empty = true;
        for(Track* t: tracks)
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
        if(!empty)
            continue;

        FxChannel* ch = Engine::fxMixer()->effectChannel(i);
        // delete channel if no references found
        if(ch->receives().isEmpty())
            deleteChannel(i);
    }

    updateIndexes();
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
    setCurrentLine(focusIndex);
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
            deleteChannel(currentLine());  //->channelIndex());
            break;
        case Qt::Key_Left:
            if(e->modifiers() & Qt::AltModifier)
            {
                moveChannelLeft(currentLine());  //->channelIndex());
            }
            else
            {
                // select channel to the left
                setCurrentLine(currentLine() - 1);  //->channelIndex() - 1);
            }
            break;
        case Qt::Key_Right:
            if(e->modifiers() & Qt::AltModifier)
            {
                moveChannelRight(currentLine());  //->channelIndex());
            }
            else
            {
                // select channel to the right
                setCurrentLine(currentLine() + 1);  //->channelIndex() + 1);
            }
            break;
        case Qt::Key_Insert:
            if(e->modifiers() & Qt::ShiftModifier)
            {
                addNewChannel();
            }
            break;
        case Qt::Key_0:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_6:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
            setCurrentLine(e->key() - Qt::Key_0);
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

void FxMixerView::clear()
{
    Engine::fxMixer()->clear();

    refreshDisplay();
}

void FxMixerView::updateFaders()
{
    FxMixer* m = Engine::fxMixer();

    // apply master gain
    // m->effectChannel(0)->m_peakLeft *= Engine::mixer()->masterGain();
    // m->effectChannel(0)->m_peakRight *= Engine::mixer()->masterGain();

    for(int i = 0; i < m_fxChannelViews.size(); ++i)
    {
        const real_t opl      = m_fxChannelViews[i]->m_fader->getPeak_L();
        const real_t opr      = m_fxChannelViews[i]->m_fader->getPeak_R();
        const real_t fall_off = 1.2;

        if(m->effectChannel(i)->peakLeft() > opl)
        {
            m_fxChannelViews[i]->m_fader->setPeak_L(
                    m->effectChannel(i)->peakLeft());
            m->effectChannel(i)->resetPeakLeft();
        }
        else
        {
            m_fxChannelViews[i]->m_fader->setPeak_L(opl / fall_off);
        }

        if(m->effectChannel(i)->peakRight() > opr)
        {
            m_fxChannelViews[i]->m_fader->setPeak_R(
                    m->effectChannel(i)->peakRight());
            m->effectChannel(i)->resetPeakRight();
        }
        else
        {
            m_fxChannelViews[i]->m_fader->setPeak_R(opr / fall_off);
        }

        // if(m_fxChannelViews[i]->m_fader->needsUpdate())
        //    m_fxChannelViews[i]->m_fader->update();
    }
}
