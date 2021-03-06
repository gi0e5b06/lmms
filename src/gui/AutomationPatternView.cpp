/*
 * AutomationPatternView.cpp - implementation of view for AutomationPattern
 *
 * Copyright (c) 2008-2010 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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
#include "AutomationPatternView.h"

#include "AutomationEditor.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "ProjectJournal.h"
#include "RenameDialog.h"
#include "StringPairDrag.h"
#include "TextFloat.h"
#include "ToolTip.h"
#include "embed.h"
//#include "gui_templates.h"

#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>

// QPixmap* AutomationPatternView::s_pat_rec = nullptr;

AutomationPatternView::AutomationPatternView(AutomationPattern* _pattern,
                                             TrackView*         _parent) :
      TileView(_pattern, _parent),
      m_pat(_pattern), m_paintPixmap(), m_pat_rec(embed::getPixmap("pat_rec"))
{
    // if(s_pat_rec == nullptr)
    //    s_pat_rec = embed::getPixmap("pat_rec");

    connect(m_pat, SIGNAL(dataChanged()), this, SLOT(update()));
    connect(gui->automationWindow(), SIGNAL(currentPatternChanged()), this,
            SLOT(update()));

    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setStyle(QApplication::style());
    update();
}

AutomationPatternView::~AutomationPatternView()
{
}

void AutomationPatternView::update()
{
    ToolTip::add(this, m_pat->name());
    TileView::update();
}

void AutomationPatternView::openInAutomationEditor()
{
    gui->automationWindow()->open(m_pat);
}

/*
void AutomationPatternView::resetName()
{
        m_pat->setName( QString::null );
}

void AutomationPatternView::changeName()
{
        QString s = m_pat->name();
        RenameDialog rename_dlg( s );
        rename_dlg.exec();
        m_pat->setName( s );
        update();
}
*/

void AutomationPatternView::disconnectObject(QAction* _a)
{
    JournallingObject* j
            = Engine::projectJournal()->journallingObject(_a->data().toInt());
    AutomatableModel* m = dynamic_cast<AutomatableModel*>(j);

    if(m != nullptr)
    {
        real_t oldMin = m_pat->getMin();
        real_t oldMax = m_pat->getMax();

        /*
        m_pat->m_objects.erase(qFind(m_pat->m_objects.begin(),
                                     m_pat->m_objects.end(),
                                     m));
        */
        m_pat->m_objects.removeAll(m);
        // if(m_pat->m_objects.isEmpty())
        //  m_pat->addObject(&AutomationPattern::s_dummyFirstObject);
        update();

        // If automation editor is opened, update its display after
        // disconnection
        if(gui->automationWindow())
            gui->automationWindow()->m_editor->updateAfterPatternChange();

        // if there is no more connection connected to the AutomationPattern
        // scale the points to fit the new min. and max. value
        if(m_pat->m_objects.isEmpty())
            scaleTimemapToFit(oldMin, oldMax);
    }
}

void AutomationPatternView::toggleRecording()
{
    m_pat->setRecording(!m_pat->isRecording());
    update();
}

void AutomationPatternView::flipY()
{
    m_pat->flipY(m_pat->getMin(), m_pat->getMax());
    update();
}

void AutomationPatternView::flipX()
{
    m_pat->flipX(m_pat->length());
    update();
}

QMenu* AutomationPatternView::buildContextMenu()
{
    QMenu*   cm = new QMenu(this);
    QAction* a;

    a = cm->addAction(embed::getIcon("automation"),
                      tr("Open in Automation editor"), this,
                      SLOT(openInAutomationEditor()));

    addRemoveMuteClearMenu(cm, true, true, m_pat->hasAutomation());
    cm->addSeparator();
    addCutCopyPasteMenu(cm, true, true, true);
    cm->addSeparator();
    addFlipMenu(cm, !m_pat->isEmpty(), !m_pat->isEmpty());
    addRotateMenu(cm, !m_pat->isEmpty(), !m_pat->isEmpty(), isFixed());
    addSplitMenu(cm, m_pat->length() > MidiTime::ticksPerTact(),
                 m_pat->length() > 4 * MidiTime::ticksPerTact());

    if(isFixed())
    {
        cm->addSeparator();
        addStepMenu(cm, m_pat->length() >= MidiTime::ticksPerTact(), true,
                    true);
    }

    cm->addSeparator();
    cm->addAction(embed::getIcon("record"), tr("Set/clear record"), this,
                  SLOT(toggleRecording()));
    /*
    cm->addAction(embed::getIcon("flip_y"),
                  tr("Flip Vertically (Visible)"), this, SLOT(flipY()));
    cm->addAction(embed::getIcon("flip_x"),
                  tr("Flip Horizontally (Visible)"), this, SLOT(flipX()));
    */
    // if( !m_pat->m_objects.isEmpty() )
    {
        bool smce = (!m_pat->m_objects.isEmpty());

        // cm->addSeparator();
        int     n   = m_pat->m_objects.size();
        QString t   = (n <= 1 ? tr("%1 connection") : tr("%1 connections"));
        QMenu*  smc = new QMenu(t.arg(n), cm);
        /*
        for(AutomationPattern::Objects::iterator it
            = m_pat->m_objects.begin();
            it != m_pat->m_objects.end(); ++it)
        {
            if(*it)
            {
                a = new QAction(
                        tr("Disconnect \"%1\"").arg((*it)->fullDisplayName()),
                        smc);
                a->setData((*it)->id());
                a->setEnabled(smce);
                smc->addAction(a);
            }
        }
        */
        m_pat->m_objects.map([&smc, &a, smce](auto m) {
            a = new QAction(tr("Disconnect \"%1\"").arg(m->fullDisplayName()),
                            smc);
            a->setData(m->id());
            a->setEnabled(smce);
            smc->addAction(a);
        });

        connect(smc, SIGNAL(triggered(QAction*)), this,
                SLOT(disconnectObject(QAction*)));
        cm->addMenu(smc);
        smc->setEnabled(smce);
    }

    cm->addSeparator();
    addPropertiesMenu(cm, !isFixed(), !isFixed());
    cm->addSeparator();
    addNameMenu(cm, true);
    cm->addSeparator();
    addColorMenu(cm, true);

    // cm->addSeparator();
    return cm;
}

void AutomationPatternView::mouseDoubleClickEvent(QMouseEvent* me)
{
    if(me->button() != Qt::LeftButton)
    {
        me->ignore();
        return;
    }
    openInAutomationEditor();
}

void AutomationPatternView::paintEvent(QPaintEvent*)
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

    // QLinearGradient lingrad( 0, 0, 0, height() );
    bool muted   = m_pat->track()->isMuted() || m_pat->isMuted();
    bool current = gui->automationWindow()->currentPattern() == m_pat;

    // state: selected, muted, default, user
    QColor bgcolor
            = isSelected()
                      ? selectedColor()
                      : (muted ? mutedBackgroundColor()
                               : (useStyleColor() ? (
                                          m_pat->track()->useStyleColor()
                                                  ? painter.background()
                                                            .color()
                                                  : m_pat->track()->color())
                                                  : color()));
    /*
    lingrad.setColorAt( 1, c.darker( 300 ) );
    lingrad.setColorAt( 0, c );

    // paint a black rectangle under the pattern to prevent glitches with
    transparent backgrounds p.fillRect( rect(), QColor( 0, 0, 0 ) );

    if( gradient() )
    {
            p.fillRect( rect(), lingrad );
    }
    else
    */
    {
        p.fillRect(rect(), bgcolor);
    }

    /*
    const real_t ppt = fixedTCOs() ?
                    ( parentWidget()->width() - 2 * TCO_BORDER_WIDTH )
                            / (real_t) m_pat->timeMapLength().getTact() :
                                                            pixelsPerTact();
    */
    const real_t ppt = pixelsPerTact();

    const int x_base = TCO_BORDER_WIDTH;

    const real_t min = m_pat->firstObject()->minValue<real_t>();
    const real_t max = m_pat->firstObject()->maxValue<real_t>();

    const real_t y_scale = max - min;
    const real_t h       = (height() - 2 * TCO_BORDER_WIDTH) / y_scale;
    const real_t ppTick  = ppt / MidiTime::ticksPerTact();

    p.translate(0.0f, max * height() / y_scale - TCO_BORDER_WIDTH);
    p.scale(1.0f, -h);

    // QLinearGradient lin2grad( 0, min, 0, max );
    QColor fgcolor
            = muted ? mutedColor()
                    : (useStyleColor()
                               ? (m_pat->track()->useStyleColor()
                                          ? painter.pen().brush().color()
                                          : m_pat->track()->color().lighter())
                               : color().lighter());
    if(bgcolor.value() >= 192)
        fgcolor = bgcolor.darker();

    /*
    lin2grad.setColorAt( 1, col.lighter( 150 ) );
    lin2grad.setColorAt( 0.5, col );
    lin2grad.setColorAt( 0, col.darker( 150 ) );
    */

    p.setRenderHints(QPainter::Antialiasing, true);

    real_t x0 = x_base;
    while(x0 < (width() - TCO_BORDER_WIDTH))
    {
        for(AutomationPattern::timeMap::const_iterator it
            = m_pat->getTimeMap().begin();
            it != m_pat->getTimeMap().end(); ++it)
        {
            if(it + 1 == m_pat->getTimeMap().end())
            {
                if(!m_pat->autoRepeat())
                {
                    const real_t x1 = x0 + it.key() * ppTick;
                    const real_t x2 = (real_t)(width() - TCO_BORDER_WIDTH);
                    if(x1 > (width() - TCO_BORDER_WIDTH))
                        break;
                    /*
                    if( gradient() )
                    {
                            p.fillRect( QRectF( x1, 0.0f, x2 - x1, it.value()
                    ), lin2grad );
                    }
                    else
                    */
                    {
                        p.fillRect(QRectF(x1, 0.f, x2 - x1, it.value()),
                                   fgcolor);
                    }
                }
                break;
            }

            real_t* values = m_pat->valuesAfter(it.key());

            real_t nextValue;
            if(m_pat->progressionType()
               == AutomationPattern::DiscreteProgression)
            {
                nextValue = it.value();
            }
            else
            {
                nextValue = (it + 1).value();
            }

            QPainterPath path;
            QPointF      origin = QPointF(x0 + it.key() * ppTick, 0.f);
            path.moveTo(origin);
            path.moveTo(QPointF(x0 + it.key() * ppTick, values[0]));
            real_t x;
            for(int i = it.key() + 1; i < (it + 1).key(); i++)
            {
                x = x0 + i * ppTick;
                if(x > (width() - TCO_BORDER_WIDTH))
                    break;
                real_t value = values[i - it.key()];
                path.lineTo(QPointF(x, value));
            }
            path.lineTo(x0 + ((it + 1).key()) * ppTick, nextValue);
            path.lineTo(x0 + ((it + 1).key()) * ppTick, 0.0f);
            path.lineTo(origin);

            /*
            if( gradient() )
            {
                    p.fillPath( path, lin2grad );
            }
            else
            */
            {
                p.fillPath(path, fgcolor);
            }
            delete[] values;
        }

        if(!m_pat->autoRepeat())
            break;

        x0 += m_pat->unitLength() * ppTick;
        // qInfo("X0=%f w=%d", x0, width());
    }

    p.setRenderHints(QPainter::Antialiasing, false);
    p.resetMatrix();

    /*
    // bar lines
    const int lineSize = 3;
    p.setPen( c.darker( 300 ) );

    for( tact_t t = 1; t < width() - TCO_BORDER_WIDTH; ++t )
    {
            const int tx = x_base + static_cast<int>( ppt * t ) - 2;
            p.drawLine( tx, TCO_BORDER_WIDTH, tx, TCO_BORDER_WIDTH + lineSize
    ); p.drawLine( tx,	rect().bottom() - ( lineSize + TCO_BORDER_WIDTH ), tx,
    rect().bottom() - TCO_BORDER_WIDTH );
    }
    */

    // recording icon for when recording automation
    if(m_pat->isRecording())
    {
        p.drawPixmap(1, rect().bottom() - m_pat_rec.height(), m_pat_rec);
    }

    bool frozen = m_pat->track()->isFrozen();
    paintFrozenIcon(frozen, p);

    paintTileTacts(false, m_pat->length().nextFullTact(), 1, bgcolor, p);

    // pattern name
    paintTextLabel(m_pat->name(), bgcolor, p);

    paintTileBorder(current, false, bgcolor, p);
    paintTileLoop(p);
    paintMutedIcon(m_pat->isMuted(), p);

    p.end();

    painter.drawPixmap(0, 0, m_paintPixmap);
}

void AutomationPatternView::dragEnterEvent(QDragEnterEvent* _dee)
{
    StringPairDrag::processDragEnterEvent(_dee, "automatable_model");
    if(!_dee->isAccepted())
    {
        TileView::dragEnterEvent(_dee);
    }
}

void AutomationPatternView::dropEvent(QDropEvent* _de)
{
    QString type = StringPairDrag::decodeKey(_de);
    QString val  = StringPairDrag::decodeValue(_de);
    if(type == "automatable_model")
    {
        AutomatableModel* mod = dynamic_cast<AutomatableModel*>(
                Engine::projectJournal()->journallingObject(val.toInt()));
        if(mod != nullptr)
        {
            bool added = m_pat->addObject(mod);
            if(!added)
            {
                TextFloat::displayMessage(mod->displayName(),
                                          tr("Model is already connected "
                                             "to this pattern."),
                                          embed::getPixmap("automation"),
                                          2000);
            }
        }
        update();

        if(gui->automationWindow()
           && gui->automationWindow()->currentPattern() == m_pat)
        {
            gui->automationWindow()->setCurrentPattern(m_pat);
        }
    }
    else
    {
        TileView::dropEvent(_de);
    }
}

/**
 * @brief Preserves the auto points over different scale
 */
void AutomationPatternView::scaleTimemapToFit(real_t oldMin, real_t oldMax)
{
    real_t newMin = m_pat->getMin();
    real_t newMax = m_pat->getMax();

    if(oldMin == newMin && oldMax == newMax)
    {
        return;
    }

    for(AutomationPattern::timeMap::iterator it = m_pat->m_timeMap.begin();
        it != m_pat->m_timeMap.end(); ++it)
    {
        if(*it < oldMin)
        {
            *it = oldMin;
        }
        else if(*it > oldMax)
        {
            *it = oldMax;
        }
        *it = (*it - oldMin) * (newMax - newMin) / (oldMax - oldMin) + newMin;
    }

    m_pat->generateTangents();
}
