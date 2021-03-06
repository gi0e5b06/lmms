/*
 * TrackContainerView.cpp - view-component for TrackContainer
 *
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "TrackContainerView.h"

#include "BBTrack.h"
#include "Clipboard.h"
#include "FileBrowser.h"
#include "GuiApplication.h"
#include "ImportFilter.h"
#include "Instrument.h"
#include "MainWindow.h"
#include "Mixer.h"
#include "PluginFactory.h"
#include "Song.h"
#include "StringPairDrag.h"
#include "TrackContainer.h"
#include "denormals.h"

//#include <QApplication>
#include <QLayout>
#include <QVBoxLayout>
//#include <QMdiArea>
#include <QWheelEvent>

#include <cmath>

using namespace std;

TrackContainerView::TrackContainerView(TrackContainer* _tc) :
      QWidget(), ModelView(_tc, this), JournallingObject(),
      SerializingObjectHook(), m_currentPosition(0, 0), m_ppt(-1), m_tc(_tc),
      m_trackViews(), m_scrollArea(new scrollArea(this)),
      m_rubberBand(new RubberBand(m_scrollArea)), m_origin()
{
    m_tc->setHook(this);
    // keeps the direction of the widget, undepended on the locale
    setLayoutDirection(Qt::LeftToRight);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(m_scrollArea);

    QWidget* scrollContent = new QWidget;
    m_scrollLayout         = new QVBoxLayout(scrollContent);
    m_scrollLayout->setContentsMargins(0, 0, 0, 12);
    m_scrollLayout->setSpacing(0);
    m_scrollLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    m_scrollArea->setWidget(scrollContent);

    m_scrollArea->show();
    m_rubberBand->hide();
    m_rubberBand->setEnabled(false);

    setAcceptDrops(true);

    connect(Engine::song(), SIGNAL(timeSignatureChanged(int, int)), this,
            SLOT(realignTracks()));
    connect(Engine::song(),
            SIGNAL(metaDataChanged(const QString&, const QString&)), this,
            SLOT(updateBackgrounds()));
    connect(m_tc, SIGNAL(trackAdded(Track*)), this,
            SLOT(createTrackView(Track*)), Qt::QueuedConnection);

    computeHyperBarViews();
}

TrackContainerView::~TrackContainerView()
{
    // while(!m_trackViews.isEmpty())
    //    delete m_trackViews.takeLast();
    // deleteTrackView(m_trackViews.takeLast());
}

void TrackContainerView::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    MainWindow::saveWidgetState(this, _this);
}

void TrackContainerView::loadSettings(const QDomElement& _this)
{
    MainWindow::restoreWidgetState(this, _this);
}

const TrackViews& TrackContainerView::trackViews() const
{
    TrackViews r;
    for(QObject* o: children())
    {
        TrackView* tv = dynamic_cast<TrackView*>(o);
        if(tv != nullptr)
            r.append(tv);
    }
    if(r != m_trackViews)
        qWarning("TrackContainerView::trackViews");

    return m_trackViews;
}

void TrackContainerView::addTrackView(TrackView* _tv)
{
    m_trackViews.append(_tv);
    m_scrollLayout->addWidget(_tv);
    connect(this, SIGNAL(positionChanged(const MidiTime&)),
            _tv->getTrackContentWidget(),
            SLOT(changePosition(const MidiTime&)));
    realignTracks();
    requireActionUpdate();
}

void TrackContainerView::removeTrackView(TrackView* _tv)
{
    if(m_trackViews.removeOne(_tv))
    {
        // disconnect(_tv);
        m_scrollLayout->removeWidget(_tv);
        realignTracks();
        if(Engine::song() != nullptr)
        {
            Engine::song()->setModified();
            requireActionUpdate();
        }
    }
}

void TrackContainerView::moveTrackView(TrackView* trackView, int indexTo)
{
    // Can't move out of bounds
    if(indexTo >= m_trackViews.size() || indexTo < 0)
        return;

    // Does not need to move to itself
    int indexFrom = m_trackViews.indexOf(trackView);
    if(indexFrom == indexTo)
        return;

    // BBTrack::swapBBTracks(trackView->track(),
    //       m_trackViews.at(indexTo)->track());

    m_scrollLayout->removeWidget(trackView);
    m_scrollLayout->insertWidget(indexTo, trackView);

    Track* track = m_tc->m_tracks[indexFrom];

    m_tc->m_tracks.remove(indexFrom);
    m_tc->m_tracks.insert(indexTo, track);
    m_trackViews.move(indexFrom, indexTo);

    realignTracks();
}

void TrackContainerView::moveTrackViewUp(TrackView* trackView)
{
    int index = m_trackViews.indexOf(trackView);
    moveTrackView(trackView, index - 1);
}

void TrackContainerView::moveTrackViewDown(TrackView* trackView)
{
    int index = m_trackViews.indexOf(trackView);
    moveTrackView(trackView, index + 1);
}

void TrackContainerView::scrollToTrackView(TrackView* _tv)
{
    if(!m_trackViews.contains(_tv))
    {
        qWarning(
                "TrackContainerView::scrollToTrackView: TrackView is not "
                "owned by this");
    }
    else
    {
        int currentScrollTop = m_scrollArea->verticalScrollBar()->value();
        int scrollAreaHeight = m_scrollArea->size().height();
        int trackViewTop     = _tv->pos().y();
        int trackViewBottom  = trackViewTop + _tv->size().height();

        // displayed_location = widget_location - currentScrollTop
        // want to make sure that the widget top has displayed location > 0,
        // and widget bottom < scrollAreaHeight
        // trackViewTop - scrollY > 0 && trackViewBottom - scrollY <
        // scrollAreaHeight therefore scrollY < trackViewTop && scrollY >
        // trackViewBottom - scrollAreaHeight
        int newScroll = std::max(trackViewBottom - scrollAreaHeight,
                                 std::min(currentScrollTop, trackViewTop));
        m_scrollArea->verticalScrollBar()->setValue(newScroll);
    }
}

void TrackContainerView::realignTracks()
{
    QWidget* content = m_scrollArea->widget();
    content->setFixedWidth(width()
                           - m_scrollArea->verticalScrollBar()->width());
    content->setFixedHeight(content->minimumSizeHint().height());

    for(TrackView* tv: m_trackViews)
    {
        tv->show();
        tv->getTrackContentWidget()->updateBackground();
        tv->update();
    }
}

TrackView* TrackContainerView::createTrackView(Track* _t)
{
    // m_tc->addJournalCheckPoint();

    // Avoid duplicating track views
    for(TrackView* tv: m_trackViews)
        if(tv->track() == _t)
            return tv;

    TrackView* r = _t->createView(this);
    requireActionUpdate();
    return r;
}

void TrackContainerView::deleteTrackView(TrackView* _tv)
{
    qInfo("TrackContainerView::deleteTrackView");
    // m_tc->addJournalCheckPoint();

    Track* t = _tv->track();
    removeTrackView(_tv);
    // delete _tv;

    // Engine::mixer()->requestChangeInModel();
    delete t;
    // Engine::mixer()->doneChangeInModel();
    requireActionUpdate();
}

// y in TrackContentWidget
const TrackView* TrackContainerView::trackViewAt(const int _y) const
{
    const int abs_y = _y + m_scrollArea->verticalScrollBar()->value();
    int       y_cnt = 0;

    //	debug code
    //	qDebug( "abs_y %d", abs_y );

    for(TrackView* tv: m_trackViews)
    {
        const int y_cnt1 = y_cnt;
        y_cnt += tv->height();
        if(abs_y >= y_cnt1 && abs_y < y_cnt)
            return tv;
    }
    return nullptr;
}

bool TrackContainerView::allowRubberband() const
{
    return false;
}

real_t TrackContainerView::pixelsPerTact() const
{
    return m_ppt > 0. ? m_ppt : 16.;
}

void TrackContainerView::setPixelsPerTact(real_t _ppt)
{
    m_ppt = _ppt;
    updateBackgrounds();
}

void TrackContainerView::updateBackgrounds()
{
    computeHyperBarViews();

    // tell all TrackContentWidgets to update their background tile pixmap
    for(TrackView* tv: m_trackViews)
        tv->getTrackContentWidget()->updateBackground();

    update();
}

void TrackContainerView::computeHyperBarViews()
{
    m_hyperBarViews.clear();
    if(m_tc == Engine::song())
    {
        QMap<QChar, QPair<int, QColor>> map;
        map.insert('I', QPair<int, QColor>(4, QColor(0, 255, 0, 64)));
        map.insert('A', QPair<int, QColor>(16, QColor(255, 0, 0, 64)));
        map.insert('B', QPair<int, QColor>(8, QColor(0, 0, 255, 64)));
        map.insert('C', QPair<int, QColor>(4, QColor(0, 255, 255, 64)));
        map.insert('D', QPair<int, QColor>(12, QColor(255, 0, 255, 64)));
        map.insert('E', QPair<int, QColor>(2, QColor(128, 255, 0, 64)));
        map.insert('F', QPair<int, QColor>(3, QColor(0, 128, 255, 64)));
        map.insert('G', QPair<int, QColor>(4, QColor(255, 0, 128, 64)));
        map.insert('H', QPair<int, QColor>(5, QColor(255, 128, 0, 64)));
        map.insert('J', QPair<int, QColor>(6, QColor(0, 255, 128, 64)));
        map.insert('K', QPair<int, QColor>(7, QColor(128, 0, 255, 64)));
        map.insert('M', QPair<int, QColor>(8, QColor(255, 192, 128, 64)));
        map.insert('N', QPair<int, QColor>(9, QColor(128, 255, 192, 64)));
        map.insert('P', QPair<int, QColor>(10, QColor(192, 128, 255, 64)));
        map.insert('Q', QPair<int, QColor>(11, QColor(255, 128, 192, 64)));
        map.insert('R', QPair<int, QColor>(13, QColor(192, 255, 128, 64)));
        map.insert('S', QPair<int, QColor>(14, QColor(128, 192, 255, 64)));
        map.insert('T', QPair<int, QColor>(15, QColor(160, 160, 160, 64)));
        map.insert('U', QPair<int, QColor>(18, QColor(160, 160, 160, 64)));
        map.insert('V', QPair<int, QColor>(20, QColor(160, 160, 160, 64)));
        map.insert('W', QPair<int, QColor>(24, QColor(160, 160, 160, 64)));
        map.insert('X', QPair<int, QColor>(28, QColor(160, 160, 160, 64)));
        map.insert('Y', QPair<int, QColor>(32, QColor(160, 160, 160, 64)));
        map.insert('Z', QPair<int, QColor>(1, QColor(255, 255, 0, 64)));
        map.insert('O', QPair<int, QColor>(16, QColor(0, 255, 0, 64)));

        /*
        hyperBarViews().append(new HyperBarView( 5,QColor(0,255,0,64),"I"));
        hyperBarViews().append(new HyperBarView(16,QColor(255,0,0,64),"A"));
        hyperBarViews().append(new HyperBarView( 7,QColor(0,0,255,64),"B"));
        hyperBarViews().append(new HyperBarView(16,QColor(255,0,0,64),"A"));
        hyperBarViews().append(new HyperBarView( 7,QColor(0,0,255,64),"B"));
        hyperBarViews().append(new HyperBarView( 1,QColor(0,255,0,64),"B"));
        hyperBarViews().append(new HyperBarView(16,QColor(255,0,0,64),"A"));
        hyperBarViews().append(new HyperBarView( 7,QColor(0,0,255,64),"B"));
        hyperBarViews().append(new HyperBarView( 7,QColor(0,0,255,64),"B"));
        hyperBarViews().append(new HyperBarView( 2,QColor(255,255,0,64),"O"));
        */

        // QString s="I16A16B8AB";
        // QString s="IABABCBO";
        QString s = Engine::song()->songMetaData("Structure");
        s.replace(QRegExp("[^'A-Z0-9].*$"), "");

        for(int i = 0; i < s.length(); i++)
        {
            int     o   = i;
            QChar   c   = s.at(i);
            QString hbs = QString(c);
            int     hbl = 0;
            QColor  hbc = QColor(255, 255, 255, 64);
            if(map.contains(c))
            {
                QPair<int, QColor> p = map.value(c);
                hbl                  = p.first;
                hbc                  = p.second;
            }
            int   p = 0;
            QChar d;
            while((i + 1 < s.length()) && (d = s.at(i + 1)) == '\'')
            {
                p++;
                i++;
            }
            int n = 4;
            while((i + 1 < s.length()) && (d = s.at(i + 1)).isDigit())
            {
                n = 10 * n + d.digitValue();
                i++;
            }
            if(n > 0)
                hbl = n;
            if(hbl == 0)
                continue;
            hyperBarViews().append(
                    new HyperBarView(hbl, hbc, s.mid(o, 1 + p)));
        }

        computeBarViews();
    }
}

void TrackContainerView::computeBarViews()
{
    // qWarning("TrackContainerView::computeBarViews");
    m_barViews.clear();
    if(m_tc == Engine::song())
    {
        bool sign = true;
        for(int i = 0; i < m_hyperBarViews.size(); i++)
        {
            const QPointer<HyperBarView>& hbv = m_hyperBarViews.at(i);
            for(int j = 0; j < hbv->length(); j++)
            {
                BarView::Types type
                        = (j == 0 ? BarView::Types::START
                                  : (j == hbv->length() - 1
                                             ? BarView::Types::END
                                             : BarView::Types::MIDDLE));
                if(j == 0)
                    sign = !sign;
                else
                    switch(hbv->length())
                    {
                        case 6:
                            if(j == 3)
                                sign = !sign;
                            break;
                        case 7:
                            if(j == 4)
                                sign = !sign;
                            break;
                        case 9:
                            if((j == 3) || (j == 7))
                                sign = !sign;
                            break;
                        case 10:
                            if(j == 5)
                                sign = !sign;
                            break;
                        case 14:
                            if((j == 4) || (j == 8))
                                sign = !sign;
                            break;
                        case 15:
                            if((j == 5) || (j == 10))
                                sign = !sign;
                            break;
                        case 17:
                            if((j == 8) || (j == 16))
                                sign = !sign;
                            break;
                        case 18:
                            if((j == 6) || (j == 12))
                                sign = !sign;
                            break;
                        default:
                            if(j % 4 == 0)
                                sign = !sign;
                            break;
                    }
                m_barViews.append(new BarView(hbv, type, sign));
            }
        }
    }
}

void TrackContainerView::clearAllTracks()
{
    while(!m_trackViews.isEmpty())
    {
        TrackView* tv = m_trackViews.takeLast();
        Track*     t  = tv->track();
        qInfo("TrackContainerView::clearAllTrack views [%s]",
              qPrintable(t->name()));
        // delete tv;
        delete t;
    }

    requireActionUpdate();
}

void TrackContainerView::dragEnterEvent(QDragEnterEvent* _dee)
{
    StringPairDrag::processDragEnterEvent(
            _dee,
            QString("presetfile,pluginpresetfile,samplefile,instrument,"
                    "importedproject,soundfontfile,vstpluginfile,projectfile,"
                    "track_%1,track_%2,track_%3")
                    .arg(Track::InstrumentTrack)
                    .arg(Track::SampleTrack)
                    .arg(Track::AutomationTrack));
}

void TrackContainerView::selectRegionFromPixels(int xStart, int xEnd)
{
    m_rubberBand->setEnabled(true);
    m_rubberBand->show();
    m_rubberBand->setGeometry(min(xStart, xEnd), 0,
                              max(xStart, xEnd) - min(xStart, xEnd),
                              std::numeric_limits<int>::max());
}

void TrackContainerView::stopRubberBand()
{
    m_rubberBand->hide();
    m_rubberBand->setEnabled(false);
    qInfo("TrackContainerView::stopRubberBand");
    requireActionUpdate();
}

void TrackContainerView::dropEvent(QDropEvent* _de)
{
    QString type  = StringPairDrag::decodeKey(_de);
    QString value = StringPairDrag::decodeValue(_de);

    qInfo("TrackContainerView::dropEvent type=%s", qPrintable(type));

    if(type == "instrument")
    {
        InstrumentTrack* it = dynamic_cast<InstrumentTrack*>(
                Track::create(Track::InstrumentTrack, m_tc));
        InstrumentLoaderThread* ilt
                = new InstrumentLoaderThread(this, it, value);
        ilt->start();
        // it->toggledInstrumentTrackButton( true );
        _de->accept();
    }
    else if(type == "samplefile" || type == "pluginpresetfile"
            || type == "soundfontfile" || type == "vstpluginfile")
    {
        InstrumentTrack* it = dynamic_cast<InstrumentTrack*>(
                Track::create(Track::InstrumentTrack, m_tc));
        Instrument* i = it->loadInstrument(
                pluginFactory
                        ->pluginSupportingExtension(
                                FileItem::extension(value))
                        .name());
        i->loadFile(value);
        // it->toggledInstrumentTrackButton( true );
        _de->accept();
    }
    else if(type == "presetfile")
    {
        DataFile         dataFile(value);
        InstrumentTrack* it = dynamic_cast<InstrumentTrack*>(
                Track::create(Track::InstrumentTrack, m_tc));
        it->setSimpleSerializing();
        it->loadSettings(dataFile.content().toElement());
        // it->toggledInstrumentTrackButton( true );
        _de->accept();
    }
    else if(type == "importedproject")
    {
        ImportFilter::import(value, m_tc);
        _de->accept();
    }

    else if(type == "projectfile")
    {
        if(gui->mainWindow()->mayChangeProject(true))
        {
            Engine::song()->loadProject(value);
        }
        _de->accept();
    }

    else if(type.left(6) == "track_")
    {
        DataFile dataFile(value.toUtf8());
        Track::create(dataFile.content().firstChild().toElement(), m_tc);
        _de->accept();
    }

    else
    {
        qInfo("TrackContainerView::dropEvent unknown type=%s",
              qPrintable(type));
        qInfo("%s", qPrintable(value));
    }

    requireActionUpdate();
}

void TrackContainerView::mousePressEvent(QMouseEvent* _me)
{
    if(allowRubberband() == true)
    {
        m_origin = m_scrollArea->mapFromParent(_me->pos());
        m_rubberBand->setEnabled(true);
        m_rubberBand->setGeometry(QRect(m_origin, QSize()));
        m_rubberBand->show();
    }
    QWidget::mousePressEvent(_me);
}

void TrackContainerView::mouseMoveEvent(QMouseEvent* _me)
{
    if(rubberBandActive() == true)
    {
        m_rubberBand->setGeometry(
                QRect(m_origin, m_scrollArea->mapFromParent(_me->pos()))
                        .normalized());
    }
    QWidget::mouseMoveEvent(_me);
}

void TrackContainerView::mouseReleaseEvent(QMouseEvent* _me)
{
    Tiles so = selectedTCOs();
    if(so.length() > 0)
        Selection::select(so.at(0));

    m_rubberBand->hide();
    m_rubberBand->setEnabled(false);

    QWidget::mouseReleaseEvent(_me);
    requireActionUpdate();
}

void TrackContainerView::resizeEvent(QResizeEvent* _re)
{
    realignTracks();
    QWidget::resizeEvent(_re);
}

RubberBand* TrackContainerView::rubberBand() const
{
    return m_rubberBand;
}

Tiles TrackContainerView::selectedTCOs()
{
    Tiles r;
    for(TileView* tv: selectedTCOViews())
        r.append(tv->tile());
    return r;
}

TileViews TrackContainerView::selectedTCOViews()
{
    TileViews r;
    for(SelectableObject* o: selectedObjects())
    {
        TileView* tv = dynamic_cast<TileView*>(o);
        if(tv == nullptr)
            continue;

        r.append(tv);
    }
    return r;
}

TileViews TrackContainerView::selectedTileViewsAt(const MidiTime& _pos,
                                                  bool _startExcluded,
                                                  bool _endExcluded)
{
    return filterTileViewsAt(selectedTCOViews(), _pos, _startExcluded,
                             _endExcluded);
}

Tiles TrackContainerView::allTiles()
{
    Tiles r;
    for(TileView* tv: allTileViews())
        r.append(tv->tile());
    return r;
}

TileViews TrackContainerView::allTileViews()
{
    TileViews r;
    for(auto trackView: trackViews())
        for(auto tileView: trackView->getTrackContentWidget()->tileViews())
            r.append(tileView);
    return r;
}

TileViews TrackContainerView::allTileViewsAt(const MidiTime& _pos,
                                             bool            _startExcluded,
                                             bool            _endExcluded)
{
    return filterTileViewsAt(allTileViews(), _pos, _startExcluded,
                             _endExcluded);
}

TileViews TrackContainerView::filterTileViewsAt(const TileViews& _tileViews,
                                                const MidiTime&  _pos,
                                                bool _startExcluded,
                                                bool _endExcluded)
{
    TileViews r;
    for(TileView* tv: _tileViews)
    {
        Tile* tile = tv->tile();
        if((!_startExcluded && tile->startPosition() > _pos)
           || (_startExcluded && tile->startPosition() >= _pos))
            continue;
        if((!_endExcluded && tile->endPosition() < _pos)
           || (_endExcluded && tile->endPosition() <= _pos))
            continue;
        r.append(tv);
    }
    return r;
}

TrackContainerView::scrollArea::scrollArea(TrackContainerView* _parent) :
      QScrollArea(_parent), m_trackContainerView(_parent)
{
    setFrameStyle(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

TrackContainerView::scrollArea::~scrollArea()
{
}

void TrackContainerView::scrollArea::wheelEvent(QWheelEvent* _we)
{
    // always pass wheel-event to parent-widget (song-editor
    // bb-editor etc.) because they might want to use it for zooming
    // or scrolling left/right if a modifier-key is pressed, otherwise
    // they do not accept it and we pass it up to QScrollArea
    m_trackContainerView->wheelEvent(_we);
    if(!_we->isAccepted())
    {
        QScrollArea::wheelEvent(_we);
    }
}

InstrumentLoaderThread::InstrumentLoaderThread(QObject*         parent,
                                               InstrumentTrack* it,
                                               QString          name) :
      QThread(parent),
      m_it(it), m_name(name)
{
    setObjectName("instrumentLoader_" + name.trimmed().replace(' ', '_'));
    m_containerThread = thread();
}

void InstrumentLoaderThread::run()
{
    disable_denormals();

    Instrument* i      = m_it->loadInstrument(m_name);
    QObject*    parent = i->parent();
    i->setParent(0);
    i->moveToThread(m_containerThread);
    i->setParent(parent);
}
