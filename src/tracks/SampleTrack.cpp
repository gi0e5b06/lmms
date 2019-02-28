/*
 * SampleTrack.cpp - implementation of class SampleTrack, a track which
 *                   provides arrangement of samples
 *
 * Copyright (c) 2017-2018 gi0e5b06 (on github.com)
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "SampleTrack.h"

//#include "FileDialog.h"
#include "AutomationPattern.h"
#include "AutomationTrack.h"
#include "BBTrack.h"
//#include "CaptionMenu.h"
#include "ConfigManager.h"
//#include "ControllerConnection.h"
#include "EffectChain.h"
#include "EffectRackView.h"
#include "embed.h"
//#include "FileBrowser.h"
#include "FxMixer.h"
#include "FxMixerView.h"
#include "GuiApplication.h"
//#include "InstrumentSoundShapingView.h"
#include "FadeButton.h"
#include "gui_templates.h"
//#include "Instrument.h"
//#include "InstrumentFunctionViews.h"
//#include "InstrumentMidiIOView.h"
#include "Knob.h"
#include "LcdSpinBox.h"
#include "LedCheckBox.h"
#include "LeftRightNav.h"
#include "MainWindow.h"
//#include "MidiClient.h"
//#include "MidiPortMenu.h"
#include "MixHelpers.h"
#include "Mixer.h"
//#include "Pattern.h"
//#include "PluginFactory.h"
//#include "PluginView.h"
#include "SamplePlayHandle.h"
#include "SampleRecordHandle.h"
#include "Song.h"
#include "StringPairDrag.h"
#include "TimeLineWidget.h"
#include "ToolTip.h"
//#include "TrackContainerView.h"
#include "TrackLabelButton.h"
#include "WaveForm.h"

#include <QDropEvent>
#include <QFileInfo>
#include <QLayout>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QProcess>
#include <QPushButton>
#include <QSignalMapper>

const char* STVOLHELP = QT_TRANSLATE_NOOP("SampleTrack",
                                          "With this knob you can set "
                                          "the volume of the opened "
                                          "channel.");

SampleTCO::SampleTCO(Track* _track) :
      TrackContentObject(_track, "Sample tile"), m_initialPlayTick(0),
      m_sampleBuffer(new SampleBuffer()), m_isPlaying(false)
{
    saveJournallingState(false);
    setSampleFile("");
    restoreJournallingState();

    changeLength(MidiTime(1, 0));
    setAutoResize(false);
    setAutoRepeat(false);

    doConnections();
    updateTrackTcos();
}

SampleTCO::SampleTCO(const SampleTCO& _other) :
      TrackContentObject(_other.getTrack(), _other.displayName()),
      m_initialPlayTick(_other.m_initialPlayTick),
      m_sampleBuffer(new SampleBuffer(*_other.m_sampleBuffer)),
      m_isPlaying(false)
{
    doConnections();
    updateTrackTcos();
}

SampleTCO::~SampleTCO()
{
    SampleTrack* sampletrack = dynamic_cast<SampleTrack*>(getTrack());
    if(sampletrack)
    {
        sampletrack->updateTcos();
    }
    sharedObject::unref(m_sampleBuffer);
}

bool SampleTCO::isEmpty() const
{
    return m_sampleBuffer->audioFile().isEmpty()
           || (m_sampleBuffer->frames() <= 1);
}

tick_t SampleTCO::unitLength() const
{
    tick_t n = MidiTime::ticksPerTact() / 8;
    return qMax(n, sampleLength().getTicks());
    // tick_t(ceilf(float(sampleLength().getTicks()) / n) * n));
}

QString SampleTCO::defaultName() const
{
    QString r = m_sampleBuffer->audioFile();
    return r.isEmpty() ? getTrack()->name() : r;
}

void SampleTCO::resizeLeft(const MidiTime& pos, const MidiTime& len)
{
    const MidiTime& sp = startPosition();

    MidiTime np = pos;
    tick_t   dp = sp - pos;
    tick_t   it = initialPlayTick();

    if(it >= dp)
    {
        it -= dp;
    }
    else
    {
        np = sp - it;
        it = 0;
    }

    setInitialPlayTick(it);
    TrackContentObject::resizeLeft(np, len);
}

void SampleTCO::split(tick_t _ticks)
{
    qInfo("SampleTCO::split %d ticks", _ticks);
    Track* t = track();
    if(t == nullptr)
    {
        qWarning("SampleTCO::split track is null");
        return;
    }

    bool     first = true;
    MidiTime sp    = startPosition();
    MidiTime ep    = endPosition();
    tick_t   it    = initialPlayTick();
    int      n     = 1;
    while(sp < ep)
    {
        if(first)
        {
            t->addJournalCheckPoint();
            first = false;
        }

        SampleTCO* newp = new SampleTCO(*this);
        newp->setJournalling(false);
        newp->setAutoResize(false);
        newp->setInitialPlayTick(it);
        newp->movePosition(sp);
        newp->changeLength(_ticks);
        newp->setName(QString::number(n));
        newp->setJournalling(true);
        newp->emit dataChanged();

        sp += _ticks;
        it += _ticks;
        n++;
    }
    deleteLater();  // t->removeTCO(this);
}

void SampleTCO::doConnections()
{
    Song* song = Engine::getSong();

    // we need to receive bpm-change-events, because then we have to
    // change length of this TCO
    connect(song, SIGNAL(tempoChanged(bpm_t)), this, SLOT(updateLength()));
    connect(song, SIGNAL(timeSignatureChanged(int, int)), this,
            SLOT(updateLength()));

    // care about positionmarker
    TimeLineWidget* timeLine
            = song->getPlayPos(song->Mode_PlaySong).m_timeLine;

    if(timeLine)
    {
        connect(timeLine, SIGNAL(positionMarkerMoved()), this,
                SLOT(playbackPositionChanged()));
    }
    // playbutton clicked or space key / on Export Song set isPlaying to false
    connect(song, SIGNAL(playbackStateChanged()), this,
            SLOT(playbackPositionChanged()));
    // care about loops
    connect(song, SIGNAL(updateSampleTracks()), this,
            SLOT(playbackPositionChanged()));
    // care about mute TCOs
    connect(this, SIGNAL(dataChanged()), this,
            SLOT(playbackPositionChanged()));
    // care about mute track
    connect(getTrack()->mutedModel(), SIGNAL(dataChanged()), this,
            SLOT(playbackPositionChanged()));
    // care about TCO position
    connect(this, SIGNAL(positionChanged()), this, SLOT(updateTrackTcos()));
}

/*
void SampleTCO::changeLength( const MidiTime & _length )
{
        float nom = Engine::getSong()->getTimeSigModel().getNumerator();
        float den = Engine::getSong()->getTimeSigModel().getDenominator();
        int ticksPerTact = DefaultTicksPerTact * ( nom / den );
        TrackContentObject::changeLength( qMax( static_cast<int>( _length ),
ticksPerTact/32 ) );
        //TrackContentObject::changeLength( qMax( static_cast<int>( _length ),
1 ) );
}
*/

const QString& SampleTCO::sampleFile() const
{
    return m_sampleBuffer->audioFile();
}

void SampleTCO::setSampleBuffer(SampleBuffer* sb)
{
    sharedObject::unref(m_sampleBuffer);
    m_sampleBuffer = sb;
    updateLength();

    emit sampleChanged();
}

void SampleTCO::setSampleFile(const QString& _sf)
{
    m_sampleBuffer->setAudioFile(_sf);
    changeLength(int(m_sampleBuffer->frames() / Engine::framesPerTick()));

    emit sampleChanged();
    emit playbackPositionChanged();
}

void SampleTCO::toggleRecord()
{
    m_recordModel.setValue(!m_recordModel.value());
    emit dataChanged();
}

void SampleTCO::playbackPositionChanged()
{
    Engine::mixer()->emit playHandlesOfTypesToRemove(
            // removePlayHandlesOfTypes(
            getTrack(), PlayHandle::TypeSamplePlayHandle);
    SampleTrack* st = dynamic_cast<SampleTrack*>(getTrack());
    st->setPlayingTcos(false);
}

void SampleTCO::updateTrackTcos()
{
    SampleTrack* sampletrack = dynamic_cast<SampleTrack*>(getTrack());
    if(sampletrack)
    {
        sampletrack->updateTcos();
    }
}

bool SampleTCO::isPlaying() const
{
    return m_isPlaying;
}

void SampleTCO::setIsPlaying(bool isPlaying)
{
    m_isPlaying = isPlaying;
}

void SampleTCO::updateLength()
{
    // emit sampleChanged();
    tick_t len;
    if(autoResize() && !isEmpty())
        len = sampleLength();
    else
        len = length();

    TrackContentObject::updateLength(len);
}

MidiTime SampleTCO::sampleLength() const
{
    return int(m_sampleBuffer->frames() / Engine::framesPerTick());
}

tick_t SampleTCO::initialPlayTick()
{
    return m_initialPlayTick;
}

void SampleTCO::setInitialPlayTick(tick_t _t)
{
    m_initialPlayTick = _t;
}

void SampleTCO::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    /*
      //if( _this.parentNode().nodeName() == "clipboarddata" )
      //{
      //	_this.setAttribute( "pos", -1 );
      //}
      //else
    {
            _this.setAttribute( "pos", startPosition() );
    }
    _this.setAttribute( "len", length() );
    _this.setAttribute( "muted", isMuted() );
    */
    TrackContentObject::saveSettings(_doc, _this);
    _this.setAttribute("initial", initialPlayTick());
    _this.setAttribute("src", sampleFile());
    if(sampleFile() == "")
    {
        QString s;
        _this.setAttribute("data", m_sampleBuffer->toBase64(s));
    }
    // TODO: start- and end-frame
}

void SampleTCO::loadSettings(const QDomElement& _this)
{
    setSampleFile(_this.attribute("src"));
    if(sampleFile().isEmpty() && _this.hasAttribute("data"))
        m_sampleBuffer->loadFromBase64(_this.attribute("data"));
    setInitialPlayTick(_this.attribute("initial").toInt());

    TrackContentObject::loadSettings(_this);
    /*
    if( _this.attribute( "pos" ).toInt() >= 0 )
    {
            movePosition( _this.attribute( "pos" ).toInt() );
    }
    changeLength( _this.attribute( "len" ).toInt() );
    setMuted( _this.attribute( "muted" ).toInt() );
    */
}

TrackContentObjectView* SampleTCO::createView(TrackView* _tv)
{
    return new SampleTCOView(this, _tv);
}

SampleTCOView::SampleTCOView(SampleTCO* _tco, TrackView* _tv) :
      TrackContentObjectView(_tco, _tv), m_tco(_tco), m_paintPixmap()
{
    // update UI and tooltip
    updateSample();

    // track future changes of SampleTCO
    connect(m_tco, SIGNAL(sampleChanged()), this, SLOT(updateSample()));

    setStyle(QApplication::style());
}

SampleTCOView::~SampleTCOView()
{
}

void SampleTCOView::loadSample()
{
    QString af = m_tco->m_sampleBuffer->openAudioFile();
    if(af != "" && af != m_tco->m_sampleBuffer->audioFile())
    {
        m_tco->setSampleFile(af);
        Engine::getSong()->setModified();
    }
}

void SampleTCOView::reloadSample()
{
    m_tco->setSampleFile(m_tco->m_sampleBuffer->audioFile());
    // Engine::getSong()->setModified();
}

void SampleTCOView::updateSample()
{
    update();
    // set tooltip to filename so that user can see what sample this
    // sample-tco contains
    ToolTip::add(this, (m_tco->m_sampleBuffer->audioFile() != "")
                               ? m_tco->m_sampleBuffer->audioFile()
                               : tr("double-click to select sample"));
}

void SampleTCOView::openInAudacity()
{
    // qWarning("SampleTCOView::openInAudacity not implemented");
    QFile p("/usr/bin/audacity");
    if(!p.exists())
    {
        if(gui)
        {
            QMessageBox::information(gui->mainWindow(),
                                     tr("Audacity missing"),
                                     tr("The audacity software is required "
                                        "for this operation. "
                                        " Please install it and retry. "
                                        "{ /usr/bin/audacity not found }"));
        }
        else
        {
            qInfo("Error: audacity not found");
        }
        return;
    }

    QProcess::execute(p.fileName(),
                      QStringList() << SampleBuffer::tryToMakeAbsolute(
                              m_tco->sampleFile()));
}

void SampleTCOView::createRmsAutomation()
{
    const MidiTime&  start  = m_tco->startPosition();
    const MidiTime&  length = m_tco->length();
    AutomationTrack* at     = nullptr;
    for(Track* t: m_tco->track()->trackContainer()->tracks())
        if(t->type() == Track::AutomationTrack)
        {
            at = dynamic_cast<AutomationTrack*>(t);
            break;
        }
    if(at == nullptr)
        return;

    AutomationPattern* ap
            = dynamic_cast<AutomationPattern*>(at->createTCO(start));
    if(ap == nullptr)
        return;

    ap->setAutoResize(false);
    ap->changeLength(length);

    const SampleBuffer* sb     = m_tco->sampleBuffer();
    const sampleFrame*  buf    = sb->data();
    const f_cnt_t       frames = sb->frames();

    sample_t vmax = 0.;
    for(f_cnt_t f = 0; f < frames; f++)
    {
        sample_t v = abs(buf[f][0]);
        if(vmax < v)
            vmax = v;
        v = abs(buf[f][1]);
        if(vmax < v)
            vmax = v;
    }
    if(vmax > SILENCE)
    {
        real_t  rms = 0.;
        f_cnt_t step
                = (Engine::framesPerTick() * MidiTime::ticksPerTact()) / 16;
        for(f_cnt_t f = 0; f < frames; f++)
        {
            rms += buf[f][0] * buf[f][0] + buf[f][1] * buf[f][1];
            if(f % step < step - 1)
                continue;

            ap->putValue(
                    (f - step + 1) / Engine::framesPerTick(),
                    bound(0.,
                          100. - 1000. * WaveForm::sqrt(rms / frames) / vmax,
                          100.),
                    false, true);
            rms = 0.;
        }
    }
    else
    {
        ap->putValue(0, 0., false, true);
    }

    ap->putValue(frames / Engine::framesPerTick(), 0., false, true);
}

QMenu* SampleTCOView::buildContextMenu()
{
    QMenu*   cm = new QMenu(this);
    QAction* a;

    bool normal  = !isFixed();  //(fixedTCOs() == false);
    bool hasFile = (m_tco->sampleFile() != "");

    a = cm->addAction(embed::getIconPixmap("sample_file"),
                      tr("Open in audacity"), this, SLOT(openInAudacity()));
    a->setEnabled(hasFile);
    addRemoveMuteClearMenu(cm, normal, normal, true);
    cm->addSeparator();
    addCutCopyPasteMenu(cm, normal, true, normal);

    if(isFixed())
    {
        cm->addSeparator();
        addStepMenu(cm, m_tco->length() >= MidiTime::ticksPerTact(), true,
                    true);
    }
    else
    {
        cm->addSeparator();
        addSplitMenu(cm, m_tco->length() > MidiTime::ticksPerTact(),
                     m_tco->length() > 4 * MidiTime::ticksPerTact());
    }

    cm->addSeparator();
    a = cm->addAction(embed::getIconPixmap("sample_file"), tr("Load sample"),
                      this, SLOT(loadSample()));
    a = cm->addAction(embed::getIconPixmap("reload"), tr("Reload"), this,
                      SLOT(reloadSample()));
    a->setEnabled(hasFile);

    cm->addSeparator();
    addPropertiesMenu(cm, !isFixed(), !isFixed());
    a = cm->addAction(embed::getIconPixmap("automation"),
                      tr("Create RMS automation"), this,
                      SLOT(createRmsAutomation()));

    cm->addSeparator();
    addNameMenu(cm, true);
    cm->addSeparator();
    addColorMenu(cm, true);

    return cm;
}

void SampleTCOView::dragEnterEvent(QDragEnterEvent* _dee)
{
    if(StringPairDrag::processDragEnterEvent(_dee, "samplefile,sampledata")
       == false)
    {
        TrackContentObjectView::dragEnterEvent(_dee);
    }
}

void SampleTCOView::dropEvent(QDropEvent* _de)
{
    if(StringPairDrag::decodeKey(_de) == "samplefile")
    {
        m_tco->setSampleFile(StringPairDrag::decodeValue(_de));
        _de->accept();
    }
    else if(StringPairDrag::decodeKey(_de) == "sampledata")
    {
        m_tco->m_sampleBuffer->loadFromBase64(
                StringPairDrag::decodeValue(_de));
        m_tco->updateLength();
        update();
        _de->accept();
        Engine::getSong()->setModified();
    }
    else
    {
        TrackContentObjectView::dropEvent(_de);
    }
}

void SampleTCOView::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::LeftButton
       && _me->modifiers() & Qt::ControlModifier
       && _me->modifiers() & Qt::ShiftModifier)
    {
        m_tco->toggleRecord();
    }
    else
    {
        if(_me->button() == Qt::MiddleButton
           && _me->modifiers() == Qt::ControlModifier)
        {
            SampleTCO* sTco
                    = dynamic_cast<SampleTCO*>(getTrackContentObject());
            if(sTco)
            {
                sTco->updateTrackTcos();
            }
        }
        TrackContentObjectView::mousePressEvent(_me);
    }
}

void SampleTCOView::mouseReleaseEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::MiddleButton && !_me->modifiers())
    {
        SampleTCO* sTco = dynamic_cast<SampleTCO*>(getTrackContentObject());
        if(sTco)
        {
            sTco->playbackPositionChanged();
        }
    }
    TrackContentObjectView::mouseReleaseEvent(_me);
}

void SampleTCOView::mouseDoubleClickEvent(QMouseEvent*)
{
    loadSample();
    /*
    QString af = m_tco->m_sampleBuffer->openAudioFile();
    if( af != "" && af != m_tco->m_sampleBuffer->audioFile() )
    {
            m_tco->setSampleFile( af );
            Engine::getSong()->setModified();
    }
    */
}

void SampleTCOView::paintEvent(QPaintEvent* pe)
{
    QPainter painter(this);

    if(!needsUpdate())
    {
        painter.drawPixmap(0, 0, m_paintPixmap);
        return;
    }

    setNeedsUpdate(false);

    if(m_paintPixmap.isNull() || m_paintPixmap.size() != size())
    {
        m_paintPixmap = QPixmap(size());
    }

    QPainter p(&m_paintPixmap);

    bool muted = m_tco->getTrack()->isMuted() || m_tco->isMuted();

    // state: selected, muted, default, user
    QColor bgcolor
            = isSelected()
                      ? selectedColor()
                      : (muted ? mutedBackgroundColor()
                               : (useStyleColor()
                                          ? (m_tco->getTrack()
                                                             ->useStyleColor()
                                                     ? painter.background()
                                                               .color()
                                                     : m_tco->getTrack()
                                                               ->color())
                                          : color()));

    /*
    // paint a black rectangle under the pattern to prevent glitches with
    transparent backgrounds p.fillRect( rect(), QColor( 0, 0, 0 ) );

    if( gradient() )
    {
            QLinearGradient lingrad( 0, 0, 0, height() );
            lingrad.setColorAt( 1, c.darker( 300 ) );
            lingrad.setColorAt( 0, c );
            p.fillRect( rect(), lingrad );
    }
    else
    */
    {
        p.fillRect(rect(), bgcolor);
    }

    p.setPen(!muted ? painter.pen().brush().color() : mutedColor());

    const int spacing = TCO_BORDER_WIDTH + 1;
    /*
    const float ppt = fixedTCOs() ?
                    ( parentWidget()->width() - 2 * TCO_BORDER_WIDTH )
                                    / (float) m_tco->length().getTact() :
                                                            pixelsPerTact();
    */
    const float ppt = pixelsPerTact();
    const float nom = Engine::getSong()->getTimeSigModel().getNumerator();
    const float den = Engine::getSong()->getTimeSigModel().getDenominator();
    const float ticksPerTact = DefaultTicksPerTact * nom / den;
    const float ppTick       = ppt / ticksPerTact;

    float x0 = TCO_BORDER_WIDTH;
    while(x0 < width())
    {
        QRect r = QRect(
                x0, spacing,
                qMax(static_cast<int>(m_tco->sampleLength() * ppTick), 1),
                rect().bottom() - 2 * spacing);

        f_cnt_t fstart = m_tco->initialPlayTick() * Engine::framesPerTick();
        f_cnt_t fend   = m_tco->m_sampleBuffer->frames() - fstart;
        // m_tco->m_sampleBuffer->visualize(p, r);  //, pe->rect() );
        m_tco->m_sampleBuffer->visualize(p, r, r, fstart, fend);

        if(!m_tco->autoRepeat())
            break;

        x0 += m_tco->unitLength() * ppTick;
    }

    bool frozen = m_tco->getTrack()->isFrozen();
    paintFrozenIcon(frozen, p);

    paintTileTacts(false, m_tco->length().nextFullTact(), 1, bgcolor, p);

    QString name = m_tco->name();
    if(name.isEmpty() && !m_tco->m_sampleBuffer->audioFile().isEmpty())
    {
        QFileInfo fileInfo(m_tco->m_sampleBuffer->audioFile());
        name = fileInfo.fileName();
    }
    paintTextLabel(name, bgcolor, p);

    // disable antialiasing for borders, since its not needed
    // p.setRenderHint( QPainter::Antialiasing, false );

    /*
    // inner border
    p.setPen( c.lighter( 160 ) );
    p.drawRect( 1, 1, rect().right() - TCO_BORDER_WIDTH,
            rect().bottom() - TCO_BORDER_WIDTH );

    // outer border
    p.setPen( c.darker( 300 ) );
    p.drawRect( 0, 0, rect().right(), rect().bottom() );
    */
    paintTileBorder(false, false, bgcolor, p);

    /*
    // draw the 'muted' pixmap only if the pattern was manualy muted
    if( m_tco->isMuted() )
    {
            const int spacing = TCO_BORDER_WIDTH;
            const int size = 14;
            p.drawPixmap( spacing, height() - ( size + spacing ),
                    embed::getIconPixmap( "muted", size, size ) );
    }
    */
    paintMutedIcon(m_tco->isMuted(), p);

    // recording sample tracks is not possible at the moment

    /* if( m_tco->isRecord() )
    {
            p.setFont( pointSize<7>( p.font() ) );

            p.setPen( textShadowColor() );
            p.drawText( 10, p.fontMetrics().height()+1, "Rec" );
            p.setPen( textColor() );
            p.drawText( 9, p.fontMetrics().height(), "Rec" );

            p.setBrush( QBrush( textColor() ) );
            p.drawEllipse( 4, 5, 4, 4 );
    }*/

    p.end();

    painter.drawPixmap(0, 0, m_paintPixmap);
}

SampleTrack::SampleTrack(TrackContainer* tc) :
      Track(Track::SampleTrack, tc),
      m_volumeModel(
              DefaultVolume, MinVolume, MaxVolume, 0.1f, this, tr("Volume")),
      m_panningModel(DefaultPanning,
                     PanningLeft,
                     PanningRight,
                     0.1f,
                     this,
                     tr("Panning")),
      m_audioPort(tr("Sample track"),
                  true,
                  nullptr,
                  &m_volumeModel,
                  nullptr,
                  &m_panningModel,
                  nullptr,
                  nullptr,
                  &m_mutedModel,
                  nullptr,
                  nullptr),
      m_effectChannelModel(0, 0, 0, this, tr("FX channel"))
{
    setColor(QColor("#D98F26"));

    m_panningModel.setCenterValue(DefaultPanning);

    m_effectChannelModel.setRange(0, Engine::fxMixer()->numChannels() - 1);

    setName(tr("Sample track"));

    connect(&m_effectChannelModel, SIGNAL(dataChanged()), this,
            SLOT(updateEffectChannel()));
}

SampleTrack::~SampleTrack()
{
    Engine::mixer()->emit playHandlesOfTypesToRemove(
            // removePlayHandlesOfTypes(
            this, PlayHandle::TypeSamplePlayHandle);
}

QString SampleTrack::defaultName() const
{
    QString r = "";
    if(numOfTCOs() > 0)
        r = getTCO(0)->name();
    if(r.isEmpty())
        r = tr("Sample track");
    return r;
}

/*
void InstrumentTrack::setName( const QString & _new_name )
{
        // when changing name of track, also change name of those patterns,
        // which have the same name as the instrument-track
        for( int i = 0; i < numOfTCOs(); ++i )
        {
                Pattern* p = dynamic_cast<Pattern*>( getTCO( i ) );
                if( ( p != NULL && p->name() == name() ) || p->name() == "" )
                {
                        p->setName( _new_name );
                }
        }

        Track::setName( _new_name );
        m_midiPort.setName( name() );
        m_audioPort.setName( name() );

        emit nameChanged();
}
*/

void SampleTrack::updateEffectChannel()
{
    m_audioPort.setNextFxChannel(m_effectChannelModel.value());
}

bool SampleTrack::play(const MidiTime& _start,
                       const fpp_t     _frames,
                       const f_cnt_t   _offset,
                       int             _tco_num)
{
    // m_audioPort.effects()->startRunning();
    bool played_a_note = false;  // will be return variable

    const float fpt = Engine::framesPerTick();

    tcoVector  tcos;
    ::BBTrack* bb_track = NULL;
    if(_tco_num >= 0)
    {
        if(_start != 0)
        {
            return false;
        }
        tcos.push_back(getTCO(_tco_num));
        if(trackContainer() == (TrackContainer*)Engine::getBBTrackContainer())
        {
            bb_track = BBTrack::findBBTrack(_tco_num);
        }
    }
    else
    {
        for(int i = 0; i < numOfTCOs(); ++i)
        {
            TrackContentObject* tco  = getTCO(i);
            SampleTCO*          sTco = dynamic_cast<SampleTCO*>(tco);
            if(_start >= sTco->startPosition()
               && _start < sTco->endPosition())
            {
                // if(sTco->isPlaying() == false)
                {
                    /*
                tick_t start = (_start - sTco->startPosition());
                if(sTco->autoRepeat())
                {
                    tick_t ul = sTco->unitLength();
                    start %= ul;
                }

                f_cnt_t sampleStart = fpt * start;
                f_cnt_t tcoFrameLength
                        = fpt
                          * (sTco->endPosition() - sTco->startPosition());
                f_cnt_t sampleBufferLength
                        = sTco->sampleBuffer()->frames();
                // if the Tco smaller than the sample length we play only
                // until Tco end else we play the sample to the end but
                // nothing more
                f_cnt_t samplePlayLength
                        = tcoFrameLength > sampleBufferLength
                                  ? sampleBufferLength
                                  : tcoFrameLength;
                // we only play within the sampleBuffer limits
                if(sampleStart < sampleBufferLength)
                    */
                    {
                        /*
qInfo("sampleStart=%d samplePlayLength=%d",
                          sampleStart, samplePlayLength);
                    sTco->sampleBuffer()->setStartFrame(sampleStart);
                    sTco->sampleBuffer()->setEndFrame(samplePlayLength
                                                      - sampleStart);
                        */
                        if(sTco->isPlaying() == false)
                        {
                            tcos.push_back(sTco);
                            sTco->setIsPlaying(true);
                        }
                    }
                }
            }
            else
            {
                sTco->setIsPlaying(false);
            }
        }
    }

    if(!isMuted())  // test with isMuted GDX
        for(tcoVector::Iterator it = tcos.begin(); it != tcos.end(); ++it)
        {
            SampleTCO* st = dynamic_cast<SampleTCO*>(*it);
            if(st == nullptr || st->isMuted())
                continue;

            PlayHandle* handle;
            if(st->isRecord())
            {
                if(!Engine::getSong()->isRecording())
                {
                    return played_a_note;
                }

                SampleRecordHandle* smpHandle = new SampleRecordHandle(st);

                handle = smpHandle;
            }
            else
            {
                SamplePlayHandle* smpHandle = new SamplePlayHandle(st);
                smpHandle->setVolumeModel(&m_volumeModel);
                smpHandle->setBBTrack(bb_track);

                if(st->autoRepeat())
                {
                    tick_t ul = st->unitLength();
                    // qInfo("smpHandle->setAutoRepeat 1");
                    smpHandle->setAutoRepeat(f_cnt_t(roundf(ul * fpt)));
                }
                else
                {
                    // qInfo("smpHandle->setAutoRepeat 0");
                    smpHandle->setAutoRepeat(0);
                }

                tick_t t = _start - st->startPosition();
                if(t > 0)
                {
                    f_cnt_t f = smpHandle->currentFrame();
                    f += fpt * t;
                    smpHandle->setCurrentFrame(f);
                }
                handle = smpHandle;
            }

            handle->setOffset(_offset);
            // send it to the mixer
            // handle->setAffinity(Engine::mixer()->thread());
            // Engine::mixer()->addPlayHandle(handle);
            Engine::mixer()->emit playHandleToAdd(handle);
            played_a_note = true;
        }

    return played_a_note;
}

TrackView* SampleTrack::createView(TrackContainerView* tcv)
{
    return new SampleTrackView(this, tcv);
}

TrackContentObject* SampleTrack::createTCO(const MidiTime&)
{
    return new SampleTCO(this);
}

void SampleTrack::saveTrackSpecificSettings(QDomDocument& _doc,
                                            QDomElement&  thisElement)
{
    m_volumeModel.saveSettings(_doc, thisElement, "vol");
    m_panningModel.saveSettings(_doc, thisElement, "pan");

    m_effectChannelModel.saveSettings(_doc, thisElement, "fxch");

    m_audioPort.effects()->saveState(_doc, thisElement);

#if 0
	thisElement.setAttribute( "icon", tlb->pixmapFile() );
#endif
}

void SampleTrack::loadTrackSpecificSettings(const QDomElement& thisElement)
{
    m_volumeModel.loadSettings(thisElement, "vol");
    m_panningModel.loadSettings(thisElement, "pan");

    m_effectChannelModel.setRange(0, Engine::fxMixer()->numChannels() - 1);
    // if ( !m_previewMode )
    {
        m_effectChannelModel.loadSettings(thisElement, "fxch");
    }

    m_audioPort.effects()->clear();

    QDomNode node = thisElement.firstChild();
    while(!node.isNull())
    {
        if(node.isElement())
        {
            if(m_audioPort.effects()->nodeName() == node.nodeName())
            {
                m_audioPort.effects()->restoreState(node.toElement());
            }
        }
        node = node.nextSibling();
    }
}

void SampleTrack::updateTcos()
{
    Engine::mixer()->emit playHandlesOfTypesToRemove(
            // removePlayHandlesOfTypes(
            this, PlayHandle::TypeSamplePlayHandle);
    setPlayingTcos(false);
}

void SampleTrack::setPlayingTcos(bool isPlaying)
{
    for(int i = 0; i < numOfTCOs(); ++i)
    {
        TrackContentObject* tco  = getTCO(i);
        SampleTCO*          sTco = dynamic_cast<SampleTCO*>(tco);
        sTco->setIsPlaying(isPlaying);
    }
}

SampleTrackView::SampleTrackView(SampleTrack* _st, TrackContainerView* _tcv) :
      TrackView(_st, _tcv), m_window(NULL), m_lastPos(-1, -1)
{
    setAcceptDrops(true);
    setFixedHeight(32);

    m_tlb = new TrackLabelButton(this, getTrackSettingsWidget());
    m_tlb->setCheckable(true);
    m_tlb->setIcon(embed::getIconPixmap("sample_track"));
    m_tlb->move(3, 1);
    m_tlb->show();

    // connect( m_tlb, SIGNAL( clicked( bool ) ),
    //         this, SLOT( showEffects() ) );
    connect(m_tlb, SIGNAL(toggled(bool)), this, SLOT(toggleVisibility(bool)));

    connect(_st, SIGNAL(dataChanged()), m_tlb, SLOT(update()));
    connect(_st, SIGNAL(dataChanged()), this, SLOT(updateName()));

    // creation of widgets for track-settings-widget
    int widgetWidth;
    if(ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt())
    {
        widgetWidth = DEFAULT_SETTINGS_WIDGET_WIDTH_COMPACT;
    }
    else
    {
        widgetWidth = DEFAULT_SETTINGS_WIDGET_WIDTH;
    }

    m_volumeKnob = new Knob(knobBright_26,  // knobSmall_17,
                            getTrackSettingsWidget(), tr("Volume"));
    m_volumeKnob->setVolumeKnob(true);
    m_volumeKnob->setModel(&_st->m_volumeModel);
    m_volumeKnob->setHintText(tr("Volume:"), "%");
    m_volumeKnob->move(widgetWidth - 2 * 29, 3);  // 24,2
    // m_volumeKnob->setLabel( tr( "VOL" ) );
    m_volumeKnob->show();
    m_volumeKnob->setWhatsThis(tr(STVOLHELP));

    m_panningKnob = new Knob(knobBright_26,  // knobSmall_17,
                             getTrackSettingsWidget(), tr("Panning"));
    m_panningKnob->setModel(&_st->m_panningModel);
    m_panningKnob->setHintText(tr("Panning:"), "%");
    m_panningKnob->move(widgetWidth - 29, 3);  // 24,2
    // m_panningKnob->setLabel( tr( "PAN" ) );
    m_panningKnob->setPointColor(Qt::magenta);
    m_panningKnob->show();

    m_effectRack = new EffectRackView(_st->audioPort()->effects());
    m_effectRack->setFixedSize(250 + 3, 200);  //( 240, 242 );

    m_effectRack->setWindowTitle(
            QString(tr("Effects on %1")).arg(_st->name()));
    m_effectRack->setWindowIcon(embed::getIconPixmap("sample_track"));

    m_window = SubWindow::putWidgetOnWorkspace(m_effectRack, false, false,
                                               false);
    m_window->hide();

    m_activityIndicator
            = new FadeButton(QApplication::palette().color(
                                     QPalette::Active, QPalette::Background),
                             QApplication::palette().color(
                                     QPalette::Active, QPalette::BrightText),
                             getTrackSettingsWidget());
    m_activityIndicator->setGeometry(widgetWidth - 2 * 29 - 11, 2, 8,
                                     28);  // 24
    m_activityIndicator->show();
    connect(m_activityIndicator, SIGNAL(pressed()), this,
            SLOT(activityIndicatorPressed()));
    connect(m_activityIndicator, SIGNAL(released()), this,
            SLOT(activityIndicatorReleased()));
    // connect( _it, SIGNAL( newNote() ),
    //		 m_activityIndicator, SLOT( activate() ) );

    connect(&_st->m_mutedModel, SIGNAL(dataChanged()), this,
            SLOT(muteChanged()));
    // connect( _st, SIGNAL( dataChanged() ),
    // this, SLOT( modelChanged() ) );

    setModel(_st);
}

SampleTrackView::~SampleTrackView()
{
    freeSampleTrackWindow();
    // m_window->deleteLater();
}

void SampleTrackView::updateName()
{
    // qInfo("SampleTrackView::updateName");
    SampleTrack* st = castModel<SampleTrack>();
    m_effectRack->setWindowTitle(
            QString(tr("Effects on %1")).arg(st->name()));
}

void SampleTrackView::modelChanged()
{
    SampleTrack* st = castModel<SampleTrack>();

    st->disconnect(SIGNAL(nameChanged()), this);
    connect(st, SIGNAL(nameChanged()), this, SLOT(updateName()));

    m_volumeKnob->setModel(&st->m_volumeModel);
    m_panningKnob->setModel(&st->m_panningModel);
    // m_effectChannelNumber->setModel( &st->m_effectChannelModel );

    TrackView::modelChanged();
}

void SampleTrackView::muteChanged()
{
    if(model()->m_mutedModel.value())
    {
        m_activityIndicator->setActiveColor(QApplication::palette().color(
                QPalette::Active, QPalette::Highlight));
    }
    else
    {
        m_activityIndicator->setActiveColor(QApplication::palette().color(
                QPalette::Active, QPalette::BrightText));
    }
}

QMenu* SampleTrackView::createAudioInputMenu()
{
    QString title = tr("Audio Input (N/A)");
    //.arg( channelIndex ).arg( fxChannel->m_name );
    QMenu* fxMenu = new QMenu(title);
    return fxMenu;
}

QMenu* SampleTrackView::createAudioOutputMenu()
{
    int        channelIndex = model()->effectChannelModel()->value();
    FxChannel* fxChannel    = Engine::fxMixer()->effectChannel(channelIndex);

    QString title = tr("Audio Output (%1:%2)")
                            .arg(channelIndex)
                            .arg(fxChannel->m_name);
    QMenu* fxMenu = new QMenu(title);

    QSignalMapper* fxMenuSignalMapper = new QSignalMapper(fxMenu);

    QString newFxLabel = tr("Assign to new channel");
    fxMenu->addAction(newFxLabel, this, SLOT(createFxLine()));
    // fxMenu->addSeparator();
    for(int i = 0; i < Engine::fxMixer()->numChannels(); ++i)
    {
        FxChannel* ch = Engine::fxMixer()->effectChannel(i);

        QString label
                = tr("Mixer %1:%2").arg(ch->m_channelIndex).arg(ch->m_name);
        QAction* action
                = fxMenu->addAction(label, fxMenuSignalMapper, SLOT(map()));
        action->setEnabled(ch != fxChannel);
        fxMenuSignalMapper->setMapping(action, ch->m_channelIndex);
        if(ch->m_channelIndex != i)
            qWarning(
                    "InstrumentTrackView::createAudioOutputMenu suspicious "
                    "ch: %d %d",
                    ch->m_channelIndex, i);
    }

    connect(fxMenuSignalMapper, SIGNAL(mapped(int)), this,
            SLOT(assignFxLine(int)));

    return fxMenu;
}

// obsolete
QMenu* SampleTrackView::createFxMenu(QString title, QString newFxLabel)
{
    int channelIndex = model()->effectChannelModel()->value();

    FxChannel* fxChannel = Engine::fxMixer()->effectChannel(channelIndex);

    // If title allows interpolation, pass channel index and name
    if(title.contains("%2"))
    {
        title = title.arg(channelIndex).arg(fxChannel->m_name);
    }

    QMenu* fxMenu = new QMenu(title);

    QSignalMapper* fxMenuSignalMapper = new QSignalMapper(fxMenu);

    fxMenu->addAction(newFxLabel, this, SLOT(createFxLine()));
    fxMenu->addSeparator();

    for(int i = 0; i < Engine::fxMixer()->numChannels(); ++i)
    {
        FxChannel* currentChannel = Engine::fxMixer()->effectChannel(i);

        if(currentChannel != fxChannel)
        {
            QString label = tr("FX %1: %2")
                                    .arg(currentChannel->m_channelIndex)
                                    .arg(currentChannel->m_name);
            QAction* action = fxMenu->addAction(label, fxMenuSignalMapper,
                                                SLOT(map()));
            fxMenuSignalMapper->setMapping(action,
                                           currentChannel->m_channelIndex);
        }
    }

    connect(fxMenuSignalMapper, SIGNAL(mapped(int)), this,
            SLOT(assignFxLine(int)));

    return fxMenu;
}

/*! \brief Create and assign a new FX Channel for this track */
void SampleTrackView::createFxLine()
{
    int channelIndex = gui->fxMixerView()->addNewChannel();
    Engine::fxMixer()->effectChannel(channelIndex)->m_name
            = getTrack()->name();
    assignFxLine(channelIndex);
}

/*! \brief Assign a specific FX Channel for this track */
void SampleTrackView::assignFxLine(int channelIndex)
{
    qInfo("SampleTrackView::assignFxLine %d", channelIndex);
    model()->effectChannelModel()->setRange(
            0, Engine::fxMixer()->numChannels() - 1);
    model()->effectChannelModel()->setValue(channelIndex);
    gui->fxMixerView()->setCurrentFxLine(channelIndex);
}

// TODO: Add windows to free list on freeInstrumentTrackWindow.
// But, don't NULL m_window or disconnect signals.  This will allow windows
// that are being show/hidden frequently to stay connected.
void SampleTrackView::freeSampleTrackWindow()
{
    if(m_window != NULL)
    {
        /*
        m_lastPos = m_window->parentWidget()->pos();

        if( ConfigManager::inst()->value( "ui",
                                          "oneinstrumenttrackwindow" ).toInt()
        || s_windowCache.count() < INSTRUMENT_WINDOW_CACHE_SIZE )
        {
                model()->setHook( NULL );
                m_window->setInstrumentTrackView( NULL );
                m_window->parentWidget()->hide();
                //m_window->setModel(
                //	engine::dummyTrackContainer()->
                //			dummyInstrumentTrack() );
                m_window->updateInstrumentView();
                s_windowCache << m_window;
        }
        else
        {
        delete m_window;
        }
        */

        m_window->deleteLater();
        m_window = NULL;
    }
}

void SampleTrackView::resizeEvent(QResizeEvent* re)
{
    TrackView::resizeEvent(re);
    /*
      I don't how to vertical align the content to the top
      m_tlb->setFixedHeight(height()-2);
    */
}

void SampleTrackView::dragEnterEvent(QDragEnterEvent* _dee)
{
    // SampleTrackWindow::dragEnterEventGeneric( _dee );
    if(!_dee->isAccepted())
    {
        TrackView::dragEnterEvent(_dee);
    }
}

void SampleTrackView::dropEvent(QDropEvent* _de)
{
    // getInstrumentTrackWindow()->dropEvent( _de );
    TrackView::dropEvent(_de);
}

/*
void SampleTrackView::showEffects()
{
        if( m_window->isHidden() )
        {
                m_effectRack->show();
                m_window->show();
                m_window->raise();
        }
        else
        {
                m_window->hide();
        }
}
*/

void SampleTrackView::toggleVisibility(bool _on)
{
    // sampleTrackWindow()->toggleVisibility( _on );

    if(_on)  // m_window->isHidden() )
    {
        m_effectRack->show();
        m_window->show();
        m_window->raise();
    }
    else
    {
        m_window->hide();
    }

    /*
    if( !_on )
    {
            freeSampleTrackWindow();
    }
    */
}

void SampleTrackView::activityIndicatorPressed()
{
    // model()->processInEvent( MidiEvent( MidiNoteOn, 0, DefaultKey,
    // MidiDefaultVelocity ) );
}

void SampleTrackView::activityIndicatorReleased()
{
    // model()->processInEvent( MidiEvent( MidiNoteOff, 0, DefaultKey, 0 ) );
}
