/*
 * AutomationEditor.cpp - used for actual setting of dynamic values
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2008-2013 Paul Giblock <pgib/at/users.sourceforge.net>
 * Copyright (c) 2006-2008 Javier Serrano Polo
 * <jasp00/at/users.sourceforge.net>
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

#include "AutomationEditor.h"

#include "ActionGroup.h"
#include "AutomationTrack.h"
#include "BBTrackContainer.h"
#include "ComboBox.h"
#include "Engine.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "PianoRoll.h"
#include "ProjectJournal.h"
#include "SongEditor.h"
#include "StringPairDrag.h"
#include "TextFloat.h"
#include "TimeLineWidget.h"
#include "ToolTip.h"
#include "WaveFormStandard.h"

//#include "debug.h"
#include "embed.h"
#include "gui_templates.h"
#include "lmms_qt_core.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>  // REQUIRED
//#include <QMdiArea>
#include <QPainter>
#include <QScrollBar>
#include <QShortcut>
#include <QStyleOption>
#include <QToolTip>

#include <cmath>

QPixmap* AutomationEditor::s_toolDraw   = nullptr;
QPixmap* AutomationEditor::s_toolErase  = nullptr;
QPixmap* AutomationEditor::s_toolSelect = nullptr;
QPixmap* AutomationEditor::s_toolMove   = nullptr;
// QPixmap* AutomationEditor::s_toolYFlip  = nullptr;
// QPixmap* AutomationEditor::s_toolXFlip  = nullptr;

// const QVector<double> AutomationEditor::m_zoomXLevels =
//      { 0.10f, 0.20f, 0.50f, 1.0f, 2.0f, 5.0f, 10.0f, 20.0f };
//{ 0.125f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f };

AutomationEditor::AutomationEditor() :
      Editor(nullptr, tr("Automation editor"), "automationEditor"),
      // QWidget(),
      m_zoomingXModel(editorModel(), tr("Zoom x"), "zoomX"),
      m_zoomingYModel(editorModel(), tr("Zoom y"), "zoomY"),
      m_quantizeXModel(editorModel(), tr("Quantize X"), "quantizeX"),
      m_quantizeYModel(editorModel(), tr("Quantize Y"), "quantizeY"),
      m_patternMutex(QMutex::Recursive), m_pattern(nullptr), m_minLevel(0),
      m_maxLevel(0), m_step(1), m_scrollLevel(0), m_bottomLevel(0),
      m_topLevel(0), m_currentPosition(), m_action(NONE), m_moveStartLevel(0),
      m_moveStartTick(0), m_drawLastLevel(0.0f), m_drawLastTick(0),
      m_ppt(DEFAULT_PPT), m_y_delta(DEFAULT_Y_DELTA), m_y_auto(true),
      // m_editMode(DRAW),
      m_mouseDownRight(false), m_scrollBack(false), m_barLineColor(0, 0, 0),
      m_beatLineColor(0, 0, 0), m_lineColor(0, 0, 0),
      m_graphColor(Qt::SolidPattern), m_vertexColor(0, 0, 0),
      m_scaleColor(Qt::SolidPattern), m_crossColor(0, 0, 0),
      m_backgroundShade(0, 0, 0)
{
    connect(this, SIGNAL(currentPatternChanged()), this,
            SLOT(updateAfterPatternChange()), Qt::QueuedConnection);
    connect(Engine::song(), SIGNAL(timeSignatureChanged(int, int)), this,
            SLOT(update()));

    setAttribute(Qt::WA_OpaquePaintEvent, true);

    // keeps the direction of the widget, undepended on the locale
    setLayoutDirection(Qt::LeftToRight);

    m_tensionModel
            = new FloatModel(0.f, -10.f, 10.f, 0.01f, nullptr, tr("Tension"));
    m_waveBankModel  = new ComboBoxModel(nullptr, tr("Wave bank"));
    m_waveIndexModel = new ComboBoxModel(nullptr, tr("Wave index"));
    m_waveRatioModel
            = new FloatModel(1.f, 0.f, 1.f, 0.01f, nullptr, tr("Wave ratio"));
    m_waveSkewModel
            = new FloatModel(1.f, 0.f, 1.f, 0.01f, nullptr, tr("Wave skew"));
    m_waveAmplitudeModel = new FloatModel(0.2f, -1.f, 1.f, 0.01f, nullptr,
                                          tr("Wave amplitude"));
    m_waveRepeatModel    = new FloatModel(0.f, -10.f, 20.f, 0.01f, nullptr,
                                       tr("Wave repeat"));

    WaveFormStandard::fillBankModel(*m_waveBankModel);
    WaveFormStandard::fillIndexModel(*m_waveIndexModel,
                                     m_waveBankModel->value());

    connect(m_tensionModel, SIGNAL(dataChanged()), this, SLOT(setTension()));
    connect(m_waveBankModel, SIGNAL(dataChanged()), this,
            SLOT(setWaveBank()));
    connect(m_waveIndexModel, SIGNAL(dataChanged()), this,
            SLOT(setWaveIndex()));
    connect(m_waveRatioModel, SIGNAL(dataChanged()), this,
            SLOT(setWaveRatio()));
    connect(m_waveSkewModel, SIGNAL(dataChanged()), this,
            SLOT(setWaveSkew()));
    connect(m_waveAmplitudeModel, SIGNAL(dataChanged()), this,
            SLOT(setWaveAmplitude()));
    connect(m_waveRepeatModel, SIGNAL(dataChanged()), this,
            SLOT(setWaveRepeat()));

    for(int i = 0; i < 7; ++i)
        m_quantizeXModel.addItem("1/" + QString::number(1 << i));
    for(int i = 0; i < 5; ++i)
        m_quantizeXModel.addItem("1/" + QString::number((1 << i) * 3));
    m_quantizeXModel.addItem("1/192");

    connect(&m_quantizeXModel, SIGNAL(dataChanged()), this,
            SLOT(setQuantizationX()));
    m_quantizeXModel.setValue(m_quantizeXModel.findText("1/8", true, 0));

    for(int i = 0; i < 7; ++i)
        m_quantizeYModel.addItem("1/" + QString::number(1 << i));
    for(int i = 0; i < 5; ++i)
        m_quantizeYModel.addItem("1/" + QString::number(pow(10, i + 1)));
    m_quantizeYModel.addItem("1/192");

    connect(&m_quantizeYModel, SIGNAL(dataChanged()), this,
            SLOT(setQuantizationY()));
    m_quantizeYModel.setValue(m_quantizeYModel.findText("1/64", true, 0));

    /*
    if(s_toolYFlip == nullptr)
        s_toolYFlip = new QPixmap(embed::getIconPixmap("flip_y"));
    if(s_toolXFlip == nullptr)
        s_toolXFlip = new QPixmap(embed::getIconPixmap("flip_x"));
    */

    // add time-line
    m_timeLine = new TimeLineWidget(
            VALUES_WIDTH, 0, m_ppt,
            Engine::song()->getPlayPos(Song::Mode_PlayAutomation),
            m_currentPosition, this);
    connect(this, SIGNAL(positionChanged(const MidiTime&)), m_timeLine,
            SLOT(updatePosition(const MidiTime&)));
    connect(m_timeLine, SIGNAL(positionChanged(const MidiTime&)), this,
            SLOT(updatePosition(const MidiTime&)));

    removeSelection();

    // init scrollbars
    m_leftRightScroll = new QScrollBar(Qt::Horizontal, this);
    m_leftRightScroll->setSingleStep(1);
    connect(m_leftRightScroll, SIGNAL(valueChanged(int)), this,
            SLOT(horScrolled(int)));

    m_topBottomScroll = new QScrollBar(Qt::Vertical, this);
    m_topBottomScroll->setSingleStep(1);
    m_topBottomScroll->setPageStep(20);
    connect(m_topBottomScroll, SIGNAL(valueChanged(int)), this,
            SLOT(verScrolled(int)));

    // init pixmaps
    if(s_toolDraw == nullptr)
        s_toolDraw = new QPixmap(embed::getPixmap("edit_draw"));
    if(s_toolErase == nullptr)
        s_toolErase = new QPixmap(embed::getPixmap("edit_erase"));
    if(s_toolSelect == nullptr)
        s_toolSelect = new QPixmap(embed::getPixmap("edit_select"));
    if(s_toolMove == nullptr)
        s_toolMove = new QPixmap(embed::getPixmap("edit_move"));

    setCurrentPattern(nullptr);

    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setMouseTracking(true);

    connect(Engine::song(), SIGNAL(timeSignatureChanged(int, int)), this,
            SLOT(update()));
}

AutomationEditor::~AutomationEditor()
{
    m_zoomingXModel.disconnect();
    m_zoomingYModel.disconnect();
    m_quantizeXModel.disconnect();
    m_tensionModel->disconnect();
    m_waveBankModel->disconnect();
    m_waveIndexModel->disconnect();
    m_waveRatioModel->disconnect();
    m_waveSkewModel->disconnect();
    m_waveAmplitudeModel->disconnect();
    m_waveRepeatModel->disconnect();

    delete m_tensionModel;
    delete m_waveBankModel;
    delete m_waveIndexModel;
    delete m_waveRatioModel;
    delete m_waveSkewModel;
    delete m_waveAmplitudeModel;
    delete m_waveRepeatModel;
}

void AutomationEditor::setCurrentPattern(AutomationPattern* _newPattern)
{
    const AutomationPattern* old = currentPattern();
    if(old == _newPattern)
        return;

    if(old != nullptr)
    {
        old->disconnect(this);
        old->automationTrack()->disconnect(this);
    }

    // stop playing if it played before
    if(Engine::song()->isPlaying()
       && Engine::song()->playMode() == Song::Mode_PlayAutomation)
        Engine::song()->playAutomation(nullptr);

    // set new data
    m_patternMutex.lock();
    m_pattern         = _newPattern;
    m_currentPosition = 0;
    m_patternMutex.unlock();

    if(m_pattern == nullptr)
    {
        // resizeEvent( nullptr );
        update();
        emit currentPatternChanged();
        return;
    }

    m_leftRightScroll->setValue(0);

    resizeEvent(nullptr);

    connect(m_pattern, SIGNAL(dataChanged()), this, SLOT(update()));

    // make sure to always get informed about the pattern being destroyed
    // connect(m_pattern, SIGNAL(destroyedPattern(Pattern*)), this,
    //        SLOT(hidePattern(Pattern*)));

    update();
    emit currentPatternChanged();
}

void AutomationEditor::saveSettings(QDomDocument& doc,
                                    QDomElement&  dom_parent)
{
    MainWindow::saveWidgetState(parentWidget(), dom_parent);
}

void AutomationEditor::loadSettings(const QDomElement& dom_parent)
{
    MainWindow::restoreWidgetState(parentWidget(), dom_parent);
}

// qproperty access methods

QColor AutomationEditor::barLineColor() const
{
    return m_barLineColor;
}

void AutomationEditor::setBarLineColor(const QColor& c)
{
    m_barLineColor = c;
}

QColor AutomationEditor::beatLineColor() const
{
    return m_beatLineColor;
}

void AutomationEditor::setBeatLineColor(const QColor& c)
{
    m_beatLineColor = c;
}

QColor AutomationEditor::lineColor() const
{
    return m_lineColor;
}

void AutomationEditor::setLineColor(const QColor& c)
{
    m_lineColor = c;
}

QBrush AutomationEditor::graphColor() const
{
    return m_graphColor;
}

void AutomationEditor::setGraphColor(const QBrush& c)
{
    m_graphColor = c;
}

QColor AutomationEditor::vertexColor() const
{
    return m_vertexColor;
}

void AutomationEditor::setVertexColor(const QColor& c)
{
    m_vertexColor = c;
}

QBrush AutomationEditor::scaleColor() const
{
    return m_scaleColor;
}

void AutomationEditor::setScaleColor(const QBrush& c)
{
    m_scaleColor = c;
}

QColor AutomationEditor::crossColor() const
{
    return m_crossColor;
}

void AutomationEditor::setCrossColor(const QColor& c)
{
    m_crossColor = c;
}

QColor AutomationEditor::backgroundShade() const
{
    return m_backgroundShade;
}

void AutomationEditor::setBackgroundShade(const QColor& c)
{
    m_backgroundShade = c;
}

void AutomationEditor::updateAfterPatternChange()
{
    QMutexLocker m(&m_patternMutex);

    m_currentPosition = 0;

    if(m_pattern == nullptr)
    {
        m_minLevel = m_maxLevel = m_scrollLevel = 0;
        m_step                                  = 1;
        resizeEvent(nullptr);
        return;
    }

    m_minLevel    = m_pattern->firstObject()->minValue<real_t>();
    m_maxLevel    = m_pattern->firstObject()->maxValue<real_t>();
    m_step        = m_pattern->firstObject()->step<real_t>();
    m_scrollLevel = (m_minLevel + m_maxLevel) / 2;

    m_tensionModel->setValue(m_pattern->tension());
    m_waveBankModel->setValue(m_pattern->waveBank());
    m_waveIndexModel->setValue(m_pattern->waveIndex());
    m_waveRatioModel->setValue(m_pattern->waveRatio());
    m_waveSkewModel->setValue(m_pattern->waveSkew());
    m_waveAmplitudeModel->setValue(m_pattern->waveAmplitude());
    m_waveRepeatModel->setValue(m_pattern->waveRepeat());

    // resizeEvent() does the rest for us (scrolling, range-checking
    // of levels and so on...)
    resizeEvent(nullptr);

    update();
}

void AutomationEditor::update()
{
    QWidget::update();

    QMutexLocker m(&m_patternMutex);
    // Note detuning?
    if(m_pattern && !m_pattern->track())
    {
        gui->pianoRollWindow()->update();
    }
}

void AutomationEditor::removeSelection()
{
    m_selectStartTick  = 0;
    m_selectedTick     = 0;
    m_selectStartLevel = 0;
    m_selectedLevels   = 0;
}

void AutomationEditor::keyPressEvent(QKeyEvent* ke)
{
    switch(ke->key())
    {
        case Qt::Key_Up:
        case Qt::Key_Down:
        {
            int direction = (ke->key() == Qt::Key_Up ? +1 : -1);
            m_topBottomScroll->setValue(m_topBottomScroll->value()
                                        - direction);
            ke->accept();
        }
        break;
        case Qt::Key_Right:
        case Qt::Key_Left:
        {
            int direction = (ke->key() == Qt::Key_Right ? +1 : -1);
            if(ke->modifiers() & Qt::AltModifier)
            {
                // switch to editing a tile adjacent to this one in the
                // song editor
                if(m_pattern != nullptr)
                {
                    AutomationPattern* p = dynamic_cast<AutomationPattern*>(
                            direction > 0 ? m_pattern->nextTile()
                                          : m_pattern->previousTile());
                    if(p != nullptr)
                        setCurrentPattern(p);
                }
            }
            else
            {
                m_timeLine->pos().setTicks(
                        qMax(0, m_timeLine->pos().ticks() + 16 * direction));
                m_timeLine->updatePosition();
                ke->accept();
            }
        }
        break;
            // TODO: m_selectButton and m_moveButton are broken.
            /*
              case Qt::Key_A:
                        if( ke->modifiers() & Qt::ControlModifier )
                        {
                                m_selectButton->setChecked( true );
                                selectAll();
                                update();
                                ke->accept();
                        }
                        break;

                case Qt::Key_Delete:
                        deleteSelectedValues();
                        ke->accept();
                        break;
            */

        case Qt::Key_Home:
            m_timeLine->pos().setTicks(0);
            m_timeLine->updatePosition();
            ke->accept();
            break;

        default:
            break;
    }
}

void AutomationEditor::focusOutEvent(QFocusEvent*)
{
    // m_editMode = m_ctrlMode;
    update();
}

void AutomationEditor::leaveEvent(QEvent* e)
{
    QPoint    mouse_pos = mapFromGlobal(QCursor::pos());
    const int mx        = mouse_pos.x();
    const int my        = mouse_pos.y();

    if(my < TOP_MARGIN || my >= height() - SCROLLBAR_SIZE || mx < VALUES_WIDTH
       || mx >= width() - SCROLLBAR_SIZE)
    {
        QToolTip::hideText();
        update();
    }

    Editor::resetOverrideCursor();
    QWidget::leaveEvent(e);
}

void AutomationEditor::drawLine(int x0In, real_t y0, int x1In, real_t y1)
{
    int x0     = Note::quantized(x0In, AutomationPattern::quantizationX());
    int x1     = Note::quantized(x1In, AutomationPattern::quantizationX());
    int deltax = qAbs(x1 - x0);
    // int deltax = qRound( qAbs<real_t>( x1 - x0 ) );
    real_t deltay = qAbs<real_t>(y1 - y0);
    int    x      = x0;
    real_t y      = y0;
    int    xstep;
    int    ystep;

    if(deltax < AutomationPattern::quantizationX())
    {
        return;
    }

    deltax /= AutomationPattern::quantizationX();

    real_t yscale = deltay / (deltax);

    if(x0 < x1)
    {
        xstep = AutomationPattern::quantizationX();
    }
    else
    {
        xstep = -(AutomationPattern::quantizationX());
    }

    real_t lineAdjust;
    if(y0 < y1)
    {
        ystep      = 1;
        lineAdjust = yscale;
    }
    else
    {
        ystep      = -1;
        lineAdjust = -(yscale);
    }

    int i = 0;
    while(i < deltax)
    {
        y = y0 + (ystep * yscale * i) + lineAdjust;

        x += xstep;
        i += 1;
        m_pattern->removeValue(MidiTime(x));
        m_pattern->putValue(MidiTime(x), y);
    }
}

void AutomationEditor::mousePressEvent(QMouseEvent* mouseEvent)
{
    QMutexLocker m(&m_patternMutex);

    if(m_pattern == nullptr)
    {
        QToolTip::hideText();
        update();
        return;
    }

    bool inside = false;

    const int my = mouseEvent->y();
    if(my >= TOP_MARGIN && my < height() - SCROLLBAR_SIZE)
    {
        real_t level = getLevel(my);

        const int mx = mouseEvent->x();
        if(mx >= VALUES_WIDTH && mx < width() - SCROLLBAR_SIZE)
        {
            inside = true;

            // set or move value

            int x = mx - VALUES_WIDTH;

            // get tick in which the user clicked
            int pos_ticks = x * MidiTime::ticksPerTact() / m_ppt
                            + m_currentPosition;

            // get time map of current pattern
            timeMap& time_map = m_pattern->getTimeMap();

            // will be our iterator in the following loop
            timeMap::iterator it = time_map.begin();

            // loop through whole time-map...
            while(it != time_map.end())
            {
                MidiTime len = 4;

                // and check whether the user clicked on an
                // existing value
                if(pos_ticks >= it.key() && len > 0
                   && (it + 1 == time_map.end()
                       || pos_ticks <= (it + 1).key())
                   && (pos_ticks
                       <= it.key() + MidiTime::ticksPerTact() * 4 / m_ppt)
                   && (level == it.value()
                       || mouseEvent->button() == Qt::RightButton))
                {
                    break;
                }
                ++it;
            }

            if(mouseEvent->button() == Qt::RightButton)
            {
                m_mouseDownRight = true;
            }

            // left button??
            if(mouseEvent->button() == Qt::LeftButton
               && isEditMode(EditMode::ModeDraw))
            {
                m_pattern->addJournalCheckPoint();
                // Connect the dots
                /*
                if(mouseEvent->modifiers() & Qt::ShiftModifier)
                {
                    drawLine(m_drawLastTick, m_drawLastLevel, pos_ticks,
                             level);
                }
                */
                m_drawLastTick  = pos_ticks;
                m_drawLastLevel = level;

                // did it reach end of map because
                // there's no value??
                if(it == time_map.end())
                {
                    // then set new value
                    MidiTime value_pos(pos_ticks);

                    MidiTime new_time = m_pattern->setDragValue(
                            value_pos, level,
                            !(mouseEvent->modifiers() & Qt::ControlModifier),
                            mouseEvent->modifiers() & Qt::ShiftModifier);

                    // reset it so that it can be used for
                    // ops (move, resize) after this
                    // code-block
                    it = time_map.find(new_time);
                }

                // move it
                m_action      = MOVE_VALUE;
                int aligned_x = (int)((real_t)((it.key() - m_currentPosition)
                                               * m_ppt)
                                      / MidiTime::ticksPerTact());
                m_moveXOffset = x - aligned_x - 1;

                Editor::applyOverrideCursor(Qt::SizeAllCursor);
                Engine::song()->setModified();
            }
            else if((mouseEvent->button() == Qt::MiddleButton)
                    || (mouseEvent->button() == Qt::RightButton
                        && mouseEvent->modifiers() & Qt::ShiftModifier
                        && isEditMode(EditMode::ModeDraw))
                    || isEditMode(EditMode::ModeErase))
            {
                m_pattern->addJournalCheckPoint();
                // erase single value
                if(it != time_map.end())
                {
                    m_pattern->removeValue(it.key());
                    Engine::song()->setModified();
                }
                m_action = NONE;
            }
            else if(mouseEvent->button() == Qt::LeftButton
                    && isEditMode(EditMode::ModeSelect))
            {
                // select an area of values

                m_selectStartTick  = pos_ticks;
                m_selectedTick     = 0;
                m_selectStartLevel = level;
                m_selectedLevels   = 1;
                m_action           = SELECT_VALUES;
            }
            else if(mouseEvent->button() == Qt::RightButton
                    && isEditMode(EditMode::ModeSelect))
            {
                // when clicking right in select-move, we
                // switch to move-mode
                // m_moveButton->setChecked( true );
            }
            else if(mouseEvent->button() == Qt::LeftButton
                    && isEditMode(EditMode::ModeMove))
            {
                m_pattern->addJournalCheckPoint();
                // move selection (including selected values)

                // save position where move-process began
                m_moveStartTick  = pos_ticks;
                m_moveStartLevel = level;

                m_action = MOVE_SELECTION;

                Engine::song()->setModified();
            }
            else if(mouseEvent->button() == Qt::RightButton
                    && isEditMode(EditMode::ModeMove))
            {
                // when clicking right in select-move, we
                // switch to draw-mode
                // m_drawButton->setChecked( true );
            }

            // update();
        }
    }

    if(!inside)
        QToolTip::hideText();

    update();
}

void AutomationEditor::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    bool mustRepaint = false;

    if(mouseEvent->button() == Qt::RightButton)
    {
        m_mouseDownRight = false;
        mustRepaint      = true;
    }

    if(mouseEvent->button() == Qt::LeftButton)
    {
        mustRepaint = true;
    }

    if(isEditMode(EditMode::ModeDraw))
    {
        if(m_action == MOVE_VALUE)
            m_pattern->applyDragValue();

        Editor::resetOverrideCursor();
    }

    m_action = NONE;

    if(mustRepaint)
    {
        repaint();
    }
}

void AutomationEditor::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    QMutexLocker m(&m_patternMutex);

    if(m_pattern == nullptr)
    {
        QToolTip::hideText();
        update();
        return;
    }

    bool inside = false;

    const int my = mouseEvent->y();
    if(my >= TOP_MARGIN && my < height() - SCROLLBAR_SIZE)
    {
        real_t level = getLevel(my);

        const int mx = mouseEvent->x();

        if(mx >= VALUES_WIDTH && mx < width() - SCROLLBAR_SIZE)
        {
            inside = true;

            int x = mx - VALUES_WIDTH;
            if(m_action == MOVE_VALUE)
                x -= m_moveXOffset;

            int pos_ticks = x * MidiTime::ticksPerTact() / m_ppt
                            + m_currentPosition;
            if(mouseEvent->buttons() & Qt::LeftButton
               && isEditMode(EditMode::ModeDraw))
            {
                if(m_action == MOVE_VALUE)
                {
                    // moving value
                    if(pos_ticks < 0)
                    {
                        pos_ticks = 0;
                    }

                    // drawLine(m_drawLastTick, m_drawLastLevel,
                    // pos_ticks, level);
                    m_drawLastTick  = pos_ticks;
                    m_drawLastLevel = level;

                    // we moved the value so the value has to be
                    // moved properly according to new starting-
                    // time in the time map of pattern
                    m_pattern->setDragValue(
                            MidiTime(pos_ticks), level,
                            !(mouseEvent->modifiers() & Qt::ControlModifier),
                            mouseEvent->modifiers() & Qt::ShiftModifier);
                }

                Engine::song()->setModified();
            }
            else if(((mouseEvent->buttons() & Qt::RightButton)
                     && isEditMode(EditMode::ModeDraw))
                    || ((mouseEvent->buttons() & Qt::LeftButton)
                        && isEditMode(EditMode::ModeErase)))
            {
                // int resolution needed to improve the sensitivity of
                // the erase manoeuvre with zoom levels < 100%
                /*
                  int zoom       = m_zoomingXModel.value();
                  int resolution = 1 + zoom * zoom;
                  for(int i = -resolution; i < resolution; ++i)
                  {
                  m_pattern->removeValue(MidiTime(pos_ticks + i));
                  }
                */
                int resolution = MidiTime::ticksPerTact() * 5 / m_ppt;
                for(int i = -resolution; i <= resolution; ++i)
                    m_pattern->removeValue(MidiTime(pos_ticks + i));
            }
            else if((mouseEvent->buttons() & Qt::NoButton)
                    && isEditMode(EditMode::ModeDraw))
            {
                // set move- or resize-cursor

                // get time map of current pattern
                timeMap& time_map = m_pattern->getTimeMap();

                // will be our iterator in the following loop
                timeMap::iterator it = time_map.begin();
                // loop through whole time map...
                for(; it != time_map.end(); ++it)
                {
                    // and check whether the cursor is over an
                    // existing value
                    if(pos_ticks >= it.key()
                       && (it + 1 == time_map.end()
                           || pos_ticks <= (it + 1).key())
                       && level <= it.value())
                    {
                        break;
                    }
                }

                // did it reach end of map because there's
                // no value??
                if(it != time_map.end())
                {
                    Editor::applyOverrideCursor(Qt::SizeAllCursor);
                }
                else
                {
                    // the cursor is over no value, so restore
                    // cursor
                    Editor::resetOverrideCursor();
                }
            }
            else if(mouseEvent->buttons() & Qt::LeftButton
                    && isEditMode(EditMode::ModeSelect)
                    && m_action == SELECT_VALUES)
            {
                // change size of selection

                if(x < 0 && m_currentPosition > 0)
                {
                    x = 0;
                    QCursor::setPos(mapToGlobal(QPoint(VALUES_WIDTH, my)));
                    if(m_currentPosition >= 4)
                    {
                        m_leftRightScroll->setValue(m_currentPosition - 4);
                    }
                    else
                    {
                        m_leftRightScroll->setValue(0);
                    }
                }
                else if(mx > width() - VALUES_WIDTH)
                {
                    x = width() - VALUES_WIDTH;
                    QCursor::setPos(
                            mapToGlobal(QPoint(width(), mouseEvent->y())));
                    m_leftRightScroll->setValue(m_currentPosition + 4);
                }

                // get tick in which the cursor is posated
                int pos_ticks = x * MidiTime::ticksPerTact() / m_ppt
                                + m_currentPosition;

                m_selectedTick = pos_ticks - m_selectStartTick;
                if((int)m_selectStartTick + m_selectedTick < 0)
                {
                    m_selectedTick = -m_selectStartTick;
                }
                m_selectedLevels = level - m_selectStartLevel;
                if(level <= m_selectStartLevel)
                {
                    --m_selectedLevels;
                }
            }
            else if(mouseEvent->buttons() & Qt::LeftButton
                    && isEditMode(EditMode::ModeMove)
                    && m_action == MOVE_SELECTION)
            {
                // move selection + selected values

                // do horizontal move-stuff
                int pos_ticks = x * MidiTime::ticksPerTact() / m_ppt
                                + m_currentPosition;
                int ticks_diff = pos_ticks - m_moveStartTick;
                if(m_selectedTick > 0)
                {
                    if((int)m_selectStartTick + ticks_diff < 0)
                    {
                        ticks_diff = -m_selectStartTick;
                    }
                }
                else
                {
                    if((int)m_selectStartTick + m_selectedTick + ticks_diff
                       < 0)
                    {
                        ticks_diff = -(m_selectStartTick + m_selectedTick);
                    }
                }
                m_selectStartTick += ticks_diff;

                int tact_diff = ticks_diff / MidiTime::ticksPerTact();
                ticks_diff    = ticks_diff % MidiTime::ticksPerTact();

                // do vertical move-stuff
                real_t level_diff = level - m_moveStartLevel;

                if(m_selectedLevels > 0)
                {
                    if(m_selectStartLevel + level_diff < m_minLevel)
                    {
                        level_diff = m_minLevel - m_selectStartLevel;
                    }
                    else if(m_selectStartLevel + m_selectedLevels + level_diff
                            > m_maxLevel)
                    {
                        level_diff = m_maxLevel - m_selectStartLevel
                                     - m_selectedLevels;
                    }
                }
                else
                {
                    if(m_selectStartLevel + m_selectedLevels + level_diff
                       < m_minLevel)
                    {
                        level_diff = m_minLevel - m_selectStartLevel
                                     - m_selectedLevels;
                    }
                    else if(m_selectStartLevel + level_diff > m_maxLevel)
                    {
                        level_diff = m_maxLevel - m_selectStartLevel;
                    }
                }
                m_selectStartLevel += level_diff;

                timeMap new_selValuesForMove;
                for(timeMap::iterator it = m_selValuesForMove.begin();
                    it != m_selValuesForMove.end(); ++it)
                {
                    MidiTime new_value_pos;
                    if(it.key())
                    {
                        int value_tact = (it.key() / MidiTime::ticksPerTact())
                                         + tact_diff;
                        int value_ticks
                                = (it.key() % MidiTime::ticksPerTact())
                                  + ticks_diff;
                        // ensure value_ticks range
                        if(value_ticks / MidiTime::ticksPerTact())
                        {
                            value_tact
                                    += value_ticks / MidiTime::ticksPerTact();
                            value_ticks %= MidiTime::ticksPerTact();
                        }
                        m_pattern->removeValue(it.key());
                        new_value_pos = MidiTime(value_tact, value_ticks);
                    }
                    new_selValuesForMove[m_pattern->putValue(
                            new_value_pos, it.value() + level_diff, false)]
                            = it.value() + level_diff;
                }
                m_selValuesForMove = new_selValuesForMove;

                m_moveStartTick  = pos_ticks;
                m_moveStartLevel = level;
            }
        }
        else
        {
            if(mouseEvent->buttons() & Qt::LeftButton
               && isEditMode(EditMode::ModeSelect)
               && m_action == SELECT_VALUES)
            {

                int x = mx - VALUES_WIDTH;
                if(mx < 0 && m_currentPosition > 0)
                {
                    x = 0;
                    QCursor::setPos(mapToGlobal(QPoint(VALUES_WIDTH, my)));
                    if(m_currentPosition >= 4)
                    {
                        m_leftRightScroll->setValue(m_currentPosition - 4);
                    }
                    else
                    {
                        m_leftRightScroll->setValue(0);
                    }
                }
                else if(x > width() - VALUES_WIDTH)
                {
                    x = width() - VALUES_WIDTH;
                    QCursor::setPos(
                            mapToGlobal(QPoint(width(), mouseEvent->y())));
                    m_leftRightScroll->setValue(m_currentPosition + 4);
                }

                // get tick in which the cursor is posated
                int pos_ticks = x * MidiTime::ticksPerTact() / m_ppt
                                + m_currentPosition;

                m_selectedTick = pos_ticks - m_selectStartTick;
                if((int)m_selectStartTick + m_selectedTick < 0)
                {
                    m_selectedTick = -m_selectStartTick;
                }

                // real_t level = getLevel(my);

                if(level <= m_bottomLevel)
                {
                    QCursor::setPos(mapToGlobal(
                            QPoint(mx, height() - SCROLLBAR_SIZE)));
                    m_topBottomScroll->setValue(m_topBottomScroll->value()
                                                + 1);
                    level = m_bottomLevel;
                }
                else if(level >= m_topLevel)
                {
                    QCursor::setPos(mapToGlobal(QPoint(mx, TOP_MARGIN)));
                    m_topBottomScroll->setValue(m_topBottomScroll->value()
                                                - 1);
                    level = m_topLevel;
                }
                m_selectedLevels = level - m_selectStartLevel;
                if(level <= m_selectStartLevel)
                {
                    --m_selectedLevels;
                }
            }

            Editor::resetOverrideCursor();
        }

        // update();
    }

    if(!inside)
        QToolTip::hideText();

    update();
}

inline void AutomationEditor::drawCross(QPainter& p)
{
    QPoint    mouse_pos = mapFromGlobal(QCursor::pos());
    const int mx        = mouse_pos.x();
    const int my        = mouse_pos.y();

    /*
    p.setPen(Qt::red);
    p.drawRect(VALUES_WIDTH, TOP_MARGIN,
               width() - SCROLLBAR_SIZE - VALUES_WIDTH - 1,
               height() - SCROLLBAR_SIZE - TOP_MARGIN - 1);
    */

    if(mx < VALUES_WIDTH || mx >= width() - SCROLLBAR_SIZE - 1
       || my < TOP_MARGIN || my >= height() - SCROLLBAR_SIZE - 1)
    {
        QToolTip::hideText();
        return;
    }

    const real_t level       = getLevel(my);
    const int    grid_bottom = height() - SCROLLBAR_SIZE - 1;
    const real_t cross_y
            = m_y_auto ? grid_bottom
                                 - ((grid_bottom - TOP_MARGIN)
                                    * (level - m_minLevel)
                                    / (real_t)(m_maxLevel - m_minLevel))
                       : grid_bottom - (level - m_bottomLevel) * m_y_delta;

    p.setPen(crossColor());
    p.drawLine(VALUES_WIDTH, (int)cross_y, width() - SCROLLBAR_SIZE,
               (int)cross_y);
    p.drawLine(mx, TOP_MARGIN, mx, height() - SCROLLBAR_SIZE);
    QPoint tt_pos = QCursor::pos();
    tt_pos.ry() -= 64;
    tt_pos.rx() += 32;

    real_t scaledLevel = m_pattern->firstObject()->scaledValue(level);

    int tick = round((mx - VALUES_WIDTH) * MidiTime::ticksPerTact() / m_ppt);

    real_t x = real_t(mx - VALUES_WIDTH)
               //*Engine::song()->getTimeSigModel().getNumerator()
               / m_ppt;

    real_t current = m_pattern->valueAt(tick);

    QToolTip::showText(tt_pos,
                       QString("Tick: %1\nValue: %2\nCurrent: %3\n"
                               "X: %4\nY: %5")
                               .arg(tick)
                               .arg(scaledLevel, 5, 'f', 3)
                               .arg(current, 5, 'f', 3)
                               .arg(x, 5, 'f', 3)
                               .arg(level, 5, 'f', 3),
                       this);
    /*
    p.setPen(Qt::blue);
    p.drawRect(VALUES_WIDTH + 3, TOP_MARGIN + 3,
               width() - SCROLLBAR_SIZE - VALUES_WIDTH - 1 - 6,
               height() - SCROLLBAR_SIZE - TOP_MARGIN - 1 - 6);
    */
}

inline void AutomationEditor::drawAutomationPoint(QPainter&         p,
                                                  timeMap::iterator it)
{
    int          x = xCoordOfTick(it.key());
    int          y = yCoordOfLevel(it.value());
    const real_t outerRadius
            = bound(2., (m_ppt * AutomationPattern::quantizationX()) / 576.,
                    5.);  // man, getting this calculation right took forever
    p.setPen(QPen(vertexColor().lighter(200)));
    p.setBrush(QBrush(vertexColor()));
    p.drawEllipse(x - outerRadius, y - outerRadius, outerRadius * 2 + 1,
                  outerRadius * 2 + 1);
}

void AutomationEditor::paintEvent(QPaintEvent* pe)
{
    QMutexLocker m(&m_patternMutex);

    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    // get foreground color
    QColor fgColor = p.pen().brush().color();
    // get background color and fill background
    QBrush bgColor = p.background();
    p.fillRect(0, 0, width(), height(), bgColor);

    // set font-size to 8
    p.setFont(pointSize<8>(p.font()));

    const int grid_width  = width() - VALUES_WIDTH - SCROLLBAR_SIZE;
    const int grid_height = height() - TOP_MARGIN - SCROLLBAR_SIZE;

    // start drawing at the bottom
    int grid_bottom = height() - SCROLLBAR_SIZE - 1;

    p.fillRect(0, TOP_MARGIN, VALUES_WIDTH, height() - TOP_MARGIN,
               scaleColor());

    // print value numbers
    int           font_height = p.fontMetrics().height();
    Qt::Alignment text_flags
            = (Qt::Alignment)(Qt::AlignRight | Qt::AlignVCenter);

    if(m_pattern != nullptr)
    {
        if(m_y_auto)
        {
            int    y[]     = {grid_bottom, TOP_MARGIN + font_height / 2};
            real_t level[] = {m_minLevel, m_maxLevel};
            for(int i = 0; i < 2; ++i)
            {
                const QString& label
                        = m_pattern->firstObject()->displayValue(level[i]);
                p.setPen(QApplication::palette().color(QPalette::Active,
                                                       QPalette::Shadow));
                p.drawText(1, y[i] - font_height + 1, VALUES_WIDTH - 10,
                           2 * font_height, text_flags, label);
                p.setPen(fgColor);
                p.drawText(0, y[i] - font_height, VALUES_WIDTH - 10,
                           2 * font_height, text_flags, label);
            }
        }
        /*
        else
        {
            int y;
            int level     = (int)m_bottomLevel;
            int printable = qMax(1, 5 * DEFAULT_Y_DELTA / m_y_delta);
            int module    = level % printable;
            if(module)
            {
                int inv_module = (printable - module) % printable;
                level += inv_module;
            }
            for(; level <= m_topLevel; level += printable)
            {
                const QString& label
                        = m_pattern->firstObject()->displayValue(level);
                y = yCoordOfLevel(level);
                p.setPen(QApplication::palette().color(QPalette::Active,
                                                       QPalette::Shadow));
                p.drawText(1, y - font_height + 1, VALUES_WIDTH - 10,
                           2 * font_height, text_flags, label);
                p.setPen(fgColor);
                p.drawText(0, y - font_height, VALUES_WIDTH - 10,
                           2 * font_height, text_flags, label);
            }
        }
        */
    }

    // set clipping area, because we are not allowed to paint over
    // keyboard...
    p.setClipRect(VALUES_WIDTH, TOP_MARGIN,
                  grid_width,  // width() - VALUES_WIDTH,
                  grid_height);

    // draw vertical raster

    if(m_pattern)
    {
        int tick, x, q;
        int x_line_end
                = (int)(m_y_auto || m_topLevel < m_maxLevel
                                ? TOP_MARGIN
                                : grid_bottom
                                          - (m_topLevel - m_bottomLevel)
                                                    * m_y_delta);

        if(m_zoomingXModel.value() > 3)
        {
            // If we're over 100% zoom, we allow all quantization level
            // grids
            q = AutomationPattern::quantizationX();
        }
        else if(AutomationPattern::quantizationX() % 3 != 0)
        {
            // If we're under 100% zoom, we allow quantization grid up to
            // 1/24 for triplets to ensure a dense doesn't fill out the
            // background
            q = AutomationPattern::quantizationX() < 8
                        ? 8
                        : AutomationPattern::quantizationX();
        }
        else
        {
            // If we're under 100% zoom, we allow quantization grid up to
            // 1/32 for normal notes
            q = AutomationPattern::quantizationX() < 6
                        ? 6
                        : AutomationPattern::quantizationX();
        }

        // 3 independent loops, because quantization might not divide
        // evenly into exotic denominators (e.g. 7/11 time), which are
        // allowed ATM. First quantization grid...
        for(tick = m_currentPosition - m_currentPosition % q,
        x        = xCoordOfTick(tick);
            x <= width(); tick += q, x = xCoordOfTick(tick))
        {
            p.setPen(lineColor());
            p.drawLine(x, grid_bottom, x, x_line_end);
        }

        /// \todo move this horizontal line drawing code into the same
        /// loop as the value ticks?
        if(m_y_auto)
        {
            // QPen pen( beatLineColor() );
            // pen.setStyle( Qt::DotLine );
            // p.setPen( pen );
            p.setPen(beatLineColor());
            real_t y_delta = (grid_bottom - TOP_MARGIN) / 8.0f;
            for(int i = 1; i < 8; ++i)
            {
                int y = (int)(grid_bottom - i * y_delta);
                p.drawLine(VALUES_WIDTH, y, width(), y);
            }
            // emboss
            p.setPen(QColor(16, 18, 20));
            for(int i = 1; i < 8; ++i)
            {
                int y = (int)(grid_bottom - i * y_delta);
                p.drawLine(VALUES_WIDTH, y - 1, width(), y - 1);
            }
        }
        else
        {
            real_t y;
            for(int level = (int)m_bottomLevel; level <= m_topLevel; level++)
            {
                y = yCoordOfLevel((real_t)level);
                if(level % 10 == 0)
                {
                    p.setPen(beatLineColor());
                }
                else
                {
                    p.setPen(lineColor());
                }
                // draw level line
                p.drawLine(VALUES_WIDTH, (int)y, width(), (int)y);
                // emboss
                p.setPen(QColor(16, 18, 20));
                p.drawLine(VALUES_WIDTH, (int)y - 1, width(), (int)y - 1);
            }
        }

        // alternating shades for better contrast
        const real_t timeSignature
                = static_cast<real_t>(
                          Engine::song()->getTimeSigModel().getNumerator())
                  / static_cast<real_t>(
                          Engine::song()->getTimeSigModel().getDenominator());
        const real_t zoomXFactor
                = EditorWindow::ZOOM_LEVELS[m_zoomingXModel.value()];
        // the bars which disappears at the left side by scrolling
        const int leftBars
                = m_currentPosition * zoomXFactor / MidiTime::ticksPerTact();

        // iterates the visible bars and draw the shading on uneven bars
        for(int x = VALUES_WIDTH, barCount = leftBars;
            x < width() + m_currentPosition * zoomXFactor / timeSignature;
            x += m_ppt, ++barCount)
        {
            if((barCount + leftBars) % 2 != 0)
            {
                p.fillRect(
                        x - m_currentPosition * zoomXFactor / timeSignature,
                        TOP_MARGIN, m_ppt,
                        height() - (SCROLLBAR_SIZE + TOP_MARGIN),
                        backgroundShade());
            }
        }

        // Draw the beat grid
        const int ticksPerBeat
                = DefaultTicksPerTact
                  / Engine::song()->getTimeSigModel().getDenominator();

        for(tick = m_currentPosition - m_currentPosition % ticksPerBeat,
        x        = xCoordOfTick(tick);
            x <= width(); tick += ticksPerBeat, x = xCoordOfTick(tick))
        {
            p.setPen(beatLineColor());
            p.drawLine(x, grid_bottom, x, x_line_end);
            // emboss
            p.setPen(QColor(16, 18, 20));
            p.drawLine(x - 1, grid_bottom, x - 1, x_line_end);
        }

        // and finally bars
        for(tick = m_currentPosition
                   - m_currentPosition % MidiTime::ticksPerTact(),
        x = xCoordOfTick(tick);
            x <= width();
            tick += MidiTime::ticksPerTact(), x = xCoordOfTick(tick))
        {
            p.setPen(barLineColor());
            p.drawLine(x, grid_bottom, x, x_line_end);
            // emboss
            p.setPen(QColor(16, 18, 20));
            p.drawLine(x - 1, grid_bottom, x - 1, x_line_end);
        }
    }

    // following code draws all visible values

    // setup selection-vars
    int sel_pos_start = m_selectStartTick;
    int sel_pos_end   = m_selectStartTick + m_selectedTick;
    if(sel_pos_start > sel_pos_end)
    {
        qSwap<int>(sel_pos_start, sel_pos_end);
    }

    real_t selLevel_start = m_selectStartLevel;
    real_t selLevel_end   = selLevel_start + m_selectedLevels;
    if(selLevel_start > selLevel_end)
    {
        qSwap<real_t>(selLevel_start, selLevel_end);
    }

    if(m_pattern != nullptr)
    {
        // NEEDS Change in CSS
        // int len_ticks = 4;
        timeMap& time_map = m_pattern->getTimeMap();

        // Don't bother doing/rendering anything if there is no automation
        // points
        if(time_map.size() > 0)
        {
            p.setRenderHints(QPainter::Antialiasing, true);
            timeMap::iterator it = time_map.begin();
            while(it + 1 != time_map.end())
            {
                // skip this section if it occurs completely before the
                // visible area
                int next_x = xCoordOfTick((it + 1).key());
                if(next_x < 0)
                {
                    ++it;
                    continue;
                }

                int x = xCoordOfTick(it.key());
                if(x > width())
                {
                    break;
                }

                // NEEDS Change in CSS
                /*bool is_selected = false;
                // if we're in move-mode, we may only draw
                // values in selected area, that have originally
                // been selected and not values that are now in
                // selection because the user moved it...
                if( isEditMode(EditMode::Mode MOVE )
                {
                        if( m_selValuesForMove.contains( it.key() ) )
                        {
                                is_selected = true;
                        }
                }
                else if( it.value() >= selLevel_start &&
                        it.value() <= selLevel_end &&
                        it.key() >= sel_pos_start &&
                        it.key() + len_ticks <= sel_pos_end )
                {
                        is_selected = true;
                }*/

                real_t* values = m_pattern->valuesAfter(it.key());

                real_t nextValue;
                if(m_pattern->progressionType()
                   == AutomationPattern::DiscreteProgression)
                {
                    nextValue = it.value();
                }
                else
                {
                    nextValue = (it + 1).value();
                }

                QPainterPath path;
                path.moveTo(
                        QPointF(xCoordOfTick(it.key()), yCoordOfLevel(0)));
                for(int i = 0; i < (it + 1).key() - it.key(); i++)
                {
                    path.lineTo(QPointF(xCoordOfTick(it.key() + i),
                                        yCoordOfLevel(values[i])));
                    // NEEDS Change in CSS
                    // drawLevelTick( p, it.key() + i, values[i],
                    // is_selected
                    // );
                }
                path.lineTo(QPointF(xCoordOfTick((it + 1).key()),
                                    yCoordOfLevel(nextValue)));
                path.lineTo(QPointF(xCoordOfTick((it + 1).key()),
                                    yCoordOfLevel(0)));
                path.lineTo(
                        QPointF(xCoordOfTick(it.key()), yCoordOfLevel(0)));
                p.fillPath(path, graphColor());
                delete[] values;

                // Draw circle
                drawAutomationPoint(p, it);

                ++it;
            }

            for(int i = it.key(), x = xCoordOfTick(i); x <= width();
                i++, x              = xCoordOfTick(i))
            {
                // TODO: Find out if the section after the last control
                // point is able to be selected and if so set this
                // boolean correctly
                drawLevelTick(p, i,
                              it.value());  ////NEEDS Change in CSS:, false );
            }
            // Draw circle(the last one)
            drawAutomationPoint(p, it);
            p.setRenderHints(QPainter::Antialiasing, false);
        }
    }
    else
    {
        QFont f = p.font();
        f.setBold(true);
        p.setFont(pointSize<14>(f));
        p.setPen(QApplication::palette().color(QPalette::Active,
                                               QPalette::BrightText));
        p.drawText(VALUES_WIDTH + 20, TOP_MARGIN + 40,
                   width() - VALUES_WIDTH - 20 - SCROLLBAR_SIZE,
                   grid_height - 40, Qt::TextWordWrap,
                   tr("Open an automation tile by double-clicking on it."));
    }

    // now draw selection-frame
    int x = (sel_pos_start - m_currentPosition) * m_ppt
            / MidiTime::ticksPerTact();
    int w = (sel_pos_end - sel_pos_start) * m_ppt / MidiTime::ticksPerTact();
    int y, h;
    if(m_y_auto)
    {
        y = (int)(grid_bottom
                  - ((grid_bottom - TOP_MARGIN)
                     * (selLevel_start - m_minLevel)
                     / (real_t)(m_maxLevel - m_minLevel)));
        h = (int)(grid_bottom
                  - ((grid_bottom - TOP_MARGIN) * (selLevel_end - m_minLevel)
                     / (real_t)(m_maxLevel - m_minLevel))
                  - y);
    }
    else
    {
        y = (int)(grid_bottom - (selLevel_start - m_bottomLevel) * m_y_delta);
        h = (int)((selLevel_start - selLevel_end) * m_y_delta);
    }
    p.setPen(QColor(0, 64, 192));
    p.drawRect(x + VALUES_WIDTH, y, w, h);

    // TODO: Get this out of paint event
    int l = m_pattern != nullptr ? (int)m_pattern->length() : 0;

    // reset scroll-range
    if(m_leftRightScroll->maximum() != l)
    {
        m_leftRightScroll->setRange(0, l);
        m_leftRightScroll->setPageStep(l);
    }

    if(m_pattern != nullptr && gui->automationWindow()->hasFocus())
    {
        drawCross(p);
    }

    /*
    const QPixmap* cursor = nullptr;
    // draw current edit-mode-icon below the cursor
    switch(editMode())
    {
        case DRAW:
            if(m_mouseDownRight)
            {
                cursor = s_toolErase;
            }
            else if(m_action == MOVE_VALUE)
            {
                cursor = s_toolMove;
            }
            else
            {
                cursor = s_toolDraw;
            }

            break;
        case ERASE:
            cursor = s_toolErase;
            break;
        case SELECT:
            cursor = s_toolSelect;
            break;
        case MOVE:
            cursor = s_toolMove;
            break;
    }

    p.drawPixmap(mapFromGlobal(QCursor::pos()) + QPoint(8, 8), *cursor);
    */
}

int AutomationEditor::xCoordOfTick(int tick)
{
    return VALUES_WIDTH
           + ((tick - m_currentPosition) * m_ppt / MidiTime::ticksPerTact());
}

real_t AutomationEditor::yCoordOfLevel(real_t level)
{
    int grid_bottom = height() - SCROLLBAR_SIZE - 1;
    if(m_y_auto)
    {
        return (grid_bottom
                - (grid_bottom - TOP_MARGIN) * (level - m_minLevel)
                          / (m_maxLevel - m_minLevel));
    }
    else
    {
        return (grid_bottom - (level - m_bottomLevel) * m_y_delta);
    }
}

// NEEDS Change in CSS
void AutomationEditor::drawLevelTick(QPainter& p, int tick, real_t value)
//			bool is_selected )
{
    int       grid_bottom = height() - SCROLLBAR_SIZE - 1;
    const int x           = xCoordOfTick(tick);
    int       rect_width  = xCoordOfTick(tick + 1) - x;

    // is the level in visible area?
    if((value >= m_bottomLevel && value <= m_topLevel)
       || (value > m_topLevel && m_topLevel >= 0)
       || (value < m_bottomLevel && m_bottomLevel <= 0))
    {
        int y_start = yCoordOfLevel(value);
        int rect_height;

        if(m_y_auto)
        {
            int y_end = (int)(grid_bottom
                              + (grid_bottom - TOP_MARGIN) * m_minLevel
                                        / (m_maxLevel - m_minLevel));

            rect_height = y_end - y_start;
        }
        else
        {
            rect_height = (int)(value * m_y_delta);
        }

        // NEEDS Change in CSS
        /*QBrush currentColor = is_selected
                ? QBrush( QColor( 0x00, 0x40, 0xC0 ) )
                : graphColor();

        */

        QBrush currentColor = graphColor();

        p.fillRect(x, y_start, rect_width, rect_height, currentColor);
    }

    else
    {
        printf("not in range\n");
    }
}

// responsible for moving/resizing scrollbars after window-resizing
void AutomationEditor::resizeEvent(QResizeEvent* re)
{
    m_leftRightScroll->setGeometry(VALUES_WIDTH, height() - SCROLLBAR_SIZE,
                                   width() - VALUES_WIDTH, SCROLLBAR_SIZE);

    int grid_height = height() - TOP_MARGIN - SCROLLBAR_SIZE;
    m_topBottomScroll->setGeometry(width() - SCROLLBAR_SIZE, TOP_MARGIN,
                                   SCROLLBAR_SIZE, grid_height);

    int half_grid    = grid_height / 2;
    int total_pixels = (int)((m_maxLevel - m_minLevel) * m_y_delta + 1);
    if(!m_y_auto && grid_height < total_pixels)
    {
        int min_scroll
                = (int)(m_minLevel + floorf(half_grid / (real_t)m_y_delta));
        int max_scroll = (int)(m_maxLevel
                               - (int)floorf((grid_height - half_grid)
                                             / (real_t)m_y_delta));
        m_topBottomScroll->setRange(min_scroll, max_scroll);
    }
    else
    {
        m_topBottomScroll->setRange((int)m_scrollLevel, (int)m_scrollLevel);
    }

    m_topBottomScroll->setValue((int)m_scrollLevel);

    if(Engine::song())
    {
        Engine::song()
                ->getPlayPos(Song::Mode_PlayAutomation)
                .m_timeLine->setFixedWidth(width());
    }

    updateTopBottomLevels();
    update();
}

void AutomationEditor::wheelEvent(QWheelEvent* we)
{
    we->accept();
    if(we->modifiers() & Qt::ControlModifier
       && we->modifiers() & Qt::ShiftModifier)
    {
        int y = m_zoomingYModel.value();
        if(we->delta() > 0)
        {
            y++;
        }
        else if(we->delta() < 0)
        {
            y--;
        }
        y = qBound(m_zoomingYModel.min(), y, m_zoomingYModel.max());
        m_zoomingYModel.setValue(y);
    }
    else if(we->modifiers() & Qt::ControlModifier
            && we->modifiers() & Qt::AltModifier)
    {
        int q = m_quantizeXModel.value();
        if(we->delta() > 0)
        {
            q--;
        }
        else if(we->delta() < 0)
        {
            q++;
        }
        q = qBound(m_quantizeXModel.min(), q, m_quantizeXModel.max());
        m_quantizeXModel.setValue(q);
        update();
    }
    else if(we->modifiers() & Qt::ControlModifier)
    {
        int x = m_zoomingXModel.value();
        if(we->delta() > 0)
        {
            x++;
        }
        else if(we->delta() < 0)
        {
            x--;
        }
        x = qBound(m_quantizeXModel.min(), x, m_zoomingXModel.max());
        m_zoomingXModel.setValue(x);
    }
    else if(we->modifiers() & Qt::ShiftModifier
            || we->orientation() == Qt::Horizontal)
    {
        m_leftRightScroll->setValue(m_leftRightScroll->value()
                                    - we->delta() * 2 / 15);
    }
    else
    {
        m_topBottomScroll->setValue(m_topBottomScroll->value()
                                    - we->delta() / 30);
    }
}

real_t AutomationEditor::getLevel(int y)
{
    int level_line_y = height() - SCROLLBAR_SIZE - 1;
    // pressed level
    real_t level
            = roundf((m_bottomLevel
                      + (m_y_auto ? (m_maxLevel - m_minLevel)
                                            * (level_line_y - y)
                                            / (real_t)(level_line_y
                                                       - (TOP_MARGIN + 2))
                                  : (level_line_y - y) / (real_t)m_y_delta))
                     / m_step)
              * m_step;
    // some range-checking-stuff
    level = qBound(m_bottomLevel, level, m_topLevel);

    return level;
}

inline bool AutomationEditor::inBBEditor()
{
    QMutexLocker m(&m_patternMutex);
    return (m_pattern != nullptr
            && m_pattern->track()->trackContainer()
                       == Engine::getBBTrackContainer());
}

void AutomationEditor::play()
{
    QMutexLocker m(&m_patternMutex);
    if(m_pattern == nullptr)
        return;

    if(Engine::song()->playMode() != Song::Mode_PlayAutomation)
        Engine::song()->playAutomation(m_pattern);
    else
        Engine::song()->togglePause();

    /*
    if(!m_pattern->track())
    {
        if(Engine::song()->playMode() != Song::Mode_PlayPattern)
        {
            Engine::song()->stop();
            Engine::song()->playPattern(
                    gui->pianoRollWindow()->currentPattern());
        }
        else if(Engine::song()->isStopped() == false)
        {
            Engine::song()->togglePause();
        }
        else
        {
            Engine::song()->playPattern(
                    gui->pianoRollWindow()->currentPattern());
        }
    }
    else if(inBBEditor())
    {
        Engine::getBBTrackContainer()->play();
    }
    else
    {
        if(Engine::song()->isStopped() == true)
        {
            Engine::song()->playSong();
        }
        else
        {
            Engine::song()->togglePause();
        }
    }
    */
}

void AutomationEditor::stop()
{
    QMutexLocker m(&m_patternMutex);
    if(m_pattern == nullptr)
        return;

    if(m_pattern->track() && inBBEditor())
        Engine::getBBTrackContainer()->stop();
    else
        Engine::song()->stop();

    m_scrollBack = true;
}

void AutomationEditor::horScrolled(int new_pos)
{
    m_currentPosition = new_pos;
    emit positionChanged(m_currentPosition);
    update();
}

void AutomationEditor::verScrolled(int new_pos)
{
    m_scrollLevel = new_pos;
    updateTopBottomLevels();
    update();
}

void AutomationEditor::setEditMode(Editor::EditMode _mode)
{
    if(editMode() == _mode)
        return;

    Editor::setEditMode(_mode);
    switch(_mode)
    {
        case EditMode::ModeDraw:
        case EditMode::ModeErase:
        case EditMode::ModeSelect:
            removeSelection();
            break;
        case EditMode::ModeMove:
            m_selValuesForMove.clear();
            getSelectedValues(m_selValuesForMove);
            break;
        case EditMode::ModeSplit:
        case EditMode::ModeJoin:
        case EditMode::ModeDetune:
            break;
    }
    update();
}

void AutomationEditor::setProgressionType(
        AutomationPattern::ProgressionTypes type)
{
    if(m_pattern != nullptr)
    {
        m_pattern->addJournalCheckPoint();
        QMutexLocker m(&m_patternMutex);
        m_pattern->setProgressionType(type);
        Engine::song()->setModified();
        update();
    }
}

void AutomationEditor::setProgressionType(int type)
{
    setProgressionType((AutomationPattern::ProgressionTypes)type);
}

void AutomationEditor::setTension()
{
    if(m_pattern)
    {
        m_pattern->setTension(m_tensionModel->value());
        update();
    }
}

void AutomationEditor::setWaveBank()
{
    if(m_pattern)
    {
        int old = m_waveIndexModel->value();
        WaveFormStandard::fillIndexModel(*m_waveIndexModel,
                                         m_waveBankModel->value());
        m_pattern->setWaveBank(m_waveBankModel->value());
        m_waveIndexModel->setValue(old);
        update();
    }
}

void AutomationEditor::setWaveIndex()
{
    if(m_pattern)
    {
        m_pattern->setWaveIndex(m_waveIndexModel->value());
        update();
    }
}

void AutomationEditor::setWaveRatio()
{
    if(m_pattern)
    {
        m_pattern->setWaveRatio(m_waveRatioModel->value());
        update();
    }
}

void AutomationEditor::setWaveSkew()
{
    if(m_pattern)
    {
        m_pattern->setWaveSkew(m_waveSkewModel->value());
        update();
    }
}

void AutomationEditor::setWaveAmplitude()
{
    if(m_pattern)
    {
        m_pattern->setWaveAmplitude(m_waveAmplitudeModel->value());
        update();
    }
}

void AutomationEditor::setWaveRepeat()
{
    if(m_pattern)
    {
        m_pattern->setWaveRepeat(m_waveRepeatModel->value());
        update();
    }
}

void AutomationEditor::selectAll()
{
    QMutexLocker m(&m_patternMutex);
    if(m_pattern == nullptr)
        return;

    timeMap& time_map = m_pattern->getTimeMap();

    timeMap::iterator it = time_map.begin();
    m_selectStartTick    = 0;
    m_selectedTick       = m_pattern->length();
    m_selectStartLevel   = it.value();
    m_selectedLevels     = 1;

    while(++it != time_map.end())
    {
        const real_t level = it.value();
        if(level < m_selectStartLevel)
        {
            // if we move start-level down, we have to add
            // the difference between old and new start-level
            // to m_selectedLevels, otherwise the selection
            // is just moved down...
            m_selectedLevels += m_selectStartLevel - level;
            m_selectStartLevel = level;
        }
        else if(level >= m_selectStartLevel + m_selectedLevels)
        {
            m_selectedLevels = level - m_selectStartLevel + 1;
        }
    }
}

// returns vector with pointers to all selected values
void AutomationEditor::getSelectedValues(timeMap& selected_values)
{
    QMutexLocker m(&m_patternMutex);
    if(m_pattern == nullptr)
        return;

    int sel_pos_start = m_selectStartTick;
    int sel_pos_end   = sel_pos_start + m_selectedTick;
    if(sel_pos_start > sel_pos_end)
    {
        qSwap<int>(sel_pos_start, sel_pos_end);
    }

    real_t selLevel_start = m_selectStartLevel;
    real_t selLevel_end   = selLevel_start + m_selectedLevels;
    if(selLevel_start > selLevel_end)
    {
        qSwap<real_t>(selLevel_start, selLevel_end);
    }

    timeMap& time_map = m_pattern->getTimeMap();

    for(timeMap::iterator it = time_map.begin(); it != time_map.end(); ++it)
    {
        // TODO: Add constant
        tick_t len_ticks = MidiTime::ticksPerTact() / 16;

        real_t level     = it.value();
        tick_t pos_ticks = it.key();

        if(level >= selLevel_start && level <= selLevel_end
           && pos_ticks >= sel_pos_start
           && pos_ticks + len_ticks <= sel_pos_end)
        {
            selected_values[it.key()] = level;
        }
    }
}

void AutomationEditor::copySelectedValues()
{
    m_valuesToCopy.clear();

    timeMap selected_values;
    getSelectedValues(selected_values);

    if(!selected_values.isEmpty())
    {
        for(timeMap::iterator it = selected_values.begin();
            it != selected_values.end(); ++it)
        {
            m_valuesToCopy[it.key()] = it.value();
        }
        TextFloat::displayMessage(tr("Values copied"),
                                  tr("All selected values were copied to the "
                                     "clipboard."),
                                  embed::getPixmap("edit_copy"), 2000);
    }
}

void AutomationEditor::cutSelectedValues()
{
    QMutexLocker m(&m_patternMutex);
    if(m_pattern == nullptr)
        return;

    m_pattern->addJournalCheckPoint();
    m_valuesToCopy.clear();

    timeMap selected_values;
    getSelectedValues(selected_values);

    if(!selected_values.isEmpty())
    {
        Engine::song()->setModified();

        for(timeMap::iterator it = selected_values.begin();
            it != selected_values.end(); ++it)
        {
            m_valuesToCopy[it.key()] = it.value();
            m_pattern->removeValue(it.key());
        }
    }

    update();
    gui->songWindow()->update();
}

void AutomationEditor::pasteValues()
{
    QMutexLocker m(&m_patternMutex);
    if(m_pattern != nullptr && !m_valuesToCopy.isEmpty())
    {
        m_pattern->addJournalCheckPoint();
        for(timeMap::iterator it = m_valuesToCopy.begin();
            it != m_valuesToCopy.end(); ++it)
        {
            m_pattern->putValue(it.key() + m_currentPosition, it.value());
        }

        // we only have to do the following lines if we pasted at
        // least one value...
        Engine::song()->setModified();
        update();
        gui->songWindow()->update();
    }
}

void AutomationEditor::deleteSelectedValues()
{
    QMutexLocker m(&m_patternMutex);
    if(m_pattern == nullptr)
        return;

    m_pattern->addJournalCheckPoint();
    timeMap selected_values;
    getSelectedValues(selected_values);

    const bool update_after_delete = !selected_values.empty();

    for(timeMap::iterator it = selected_values.begin();
        it != selected_values.end(); ++it)
    {
        m_pattern->removeValue(it.key());
    }

    if(update_after_delete == true)
    {
        Engine::song()->setModified();
        update();
        gui->songWindow()->update();
    }
}

void AutomationEditor::updatePosition(const MidiTime& t)
{
    if((Engine::song()->isPlaying()
        && Engine::song()->playMode() == Song::Mode_PlayAutomation)
       || m_scrollBack == true)
    {
        const real_t w = width() - VALUES_WIDTH;
        if(t
           > m_currentPosition + tick_t(w * MidiTime::ticksPerTact() / m_ppt))
        {
            m_leftRightScroll->setValue(t.getTact()
                                        * MidiTime::ticksPerTact());
        }
        else if(t < m_currentPosition)
        {
            MidiTime u = qMax<MidiTime>(
                    t - tick_t(w * MidiTime::ticksPerTact() / m_ppt),
                    0);  //* MidiTime::ticksPerTact()
            m_leftRightScroll->setValue(u.getTact()
                                        * MidiTime::ticksPerTact());
        }
        m_scrollBack = false;
    }
}

void AutomationEditor::zoomingXChanged()
{
    m_ppt = EditorWindow::ZOOM_LEVELS[m_zoomingXModel.value()] * DEFAULT_PPT;

#ifdef LMMS_DEBUG
    Q_ASSERT(m_ppt > 0.f);
#endif

    m_timeLine->setPixelsPerTact(m_ppt);
    update();
}

void AutomationEditor::zoomingYChanged()
{
    /*
    const QString& zfac = m_zoomingYModel.currentText();
    m_y_auto            = zfac == "Auto";
    if(!m_y_auto)
        m_y_delta = zfac.left(zfac.length() - 1).toInt() * DEFAULT_Y_DELTA
                    / 100;
    */

    int z = m_zoomingYModel.value();
    if(z < 0)
    {
        m_y_auto  = true;
        m_y_delta = DEFAULT_Y_DELTA;
    }
    else
    {
        m_y_auto  = false;
        m_y_delta = EditorWindow::ZOOM_LEVELS[z] * DEFAULT_Y_DELTA;
    }

#ifdef LMMS_DEBUG
    Q_ASSERT(m_y_delta > 0.f);
#endif
    resizeEvent(nullptr);
}

void AutomationEditor::setQuantizationX()
{
    int quantization = m_quantizeXModel.value();
    if(quantization < 0)
    {
        quantization = DefaultTicksPerTact;
    }
    else if(quantization < 7)
    {
        quantization = 1 << quantization;
    }
    else if(quantization < 12)
    {
        quantization = 1 << (quantization - 7);
        quantization *= 3;
    }
    else
    {
        quantization = DefaultTicksPerTact;
    }
    quantization = DefaultTicksPerTact / quantization;
    AutomationPattern::setQuantizationX(quantization);

    update();
}

void AutomationEditor::setQuantizationY()
{
    int quantization = m_quantizeYModel.value();
    if(quantization < 0)
    {
        quantization = 192;
    }
    else if(quantization < 7)
    {
        quantization = 1 << quantization;
    }
    else if(quantization < 12)
    {
        quantization = pow(10, quantization - 6);
    }
    else
    {
        quantization = 192;
    }

    AutomationPattern::setQuantizationY(1. / quantization);

    update();
}

void AutomationEditor::updateTopBottomLevels()
{
    if(m_y_auto)
    {
        m_bottomLevel = m_minLevel;
        m_topLevel    = m_maxLevel;
        return;
    }

    int total_pixels = (int)((m_maxLevel - m_minLevel) * m_y_delta + 1);
    int grid_height  = height() - TOP_MARGIN - SCROLLBAR_SIZE;
    int half_grid    = grid_height / 2;

    if(total_pixels > grid_height)
    {
        int centralLevel = (int)(m_minLevel + m_maxLevel - m_scrollLevel);

        m_bottomLevel = centralLevel - (half_grid / (real_t)m_y_delta);
        if(m_bottomLevel < m_minLevel)
        {
            m_bottomLevel = m_minLevel;
            m_topLevel    = m_minLevel
                         + (int)floorf(grid_height / (real_t)m_y_delta);
        }
        else
        {
            m_topLevel = m_bottomLevel
                         + (int)floorf(grid_height / (real_t)m_y_delta);
            if(m_topLevel > m_maxLevel)
            {
                m_topLevel = m_maxLevel;
                m_bottomLevel
                        = m_maxLevel
                          - (int)floorf(grid_height / (real_t)m_y_delta);
            }
        }
    }
    else
    {
        m_bottomLevel = m_minLevel;
        m_topLevel    = m_maxLevel;
    }
}

AutomationWindow::AutomationWindow() :
      EditorWindow(), m_editor(new AutomationEditor())
{
    setCentralWidget(m_editor);

    // Play/stop buttons
    m_playAction->setToolTip(tr("Play/pause current pattern (Space)"));
    m_playAction->setWhatsThis(
            tr("Click here if you want to play the current pattern. "
               "This is useful while editing it.  The pattern is "
               "automatically looped when the end is reached."));

    m_stopAction->setToolTip(tr("Stop playing of current pattern (Space)"));
    m_stopAction->setWhatsThis(
            tr("Click here if you want to stop playing of the "
               "current pattern."));

    // Edit mode buttons
    DropToolBar* editActionsToolBar = addDropToolBarToTop(tr("Edit actions"));

    ActionGroup* editModeGroup = new ActionGroup(this);
    QAction*     drawAction    = editModeGroup->addAction(
            embed::getIcon("edit_draw"), TR("Draw mode (Shift+D)"));
    drawAction->setShortcut(Qt::SHIFT | Qt::Key_D);
    drawAction->setChecked(true);

    QAction* eraseAction = editModeGroup->addAction(
            embed::getIcon("edit_erase"), TR("Erase mode (Shift+E)"));
    eraseAction->setShortcut(Qt::SHIFT | Qt::Key_E);

    //	TODO: m_selectButton and m_moveButton are broken.
    //	m_selectButton = new QAction(embed::getIcon("edit_select"),
    // TR("Select mode (Shift+S)", editModeGroup); 	m_moveButton = new
    // QAction(embed::getIcon("edit_move"), TR("Move selection mode
    //(Shift+M)", editModeGroup);

    drawAction->setWhatsThis(
            tr("Click here and draw-mode will be activated. In this "
               "mode you can add and move single values.  This "
               "is the default mode which is used most of the time.  "
               "You can also press 'Shift+D' on your keyboard to "
               "activate this mode."));
    eraseAction->setWhatsThis(
            tr("Click here and erase-mode will be activated. In this "
               "mode you can erase single values. You can also press "
               "'Shift+E' on your keyboard to activate this mode."));
    /*m_selectButton->setWhatsThis(
            tr( "Click here and select-mode will be activated. In this "
                    "mode you can select values. This is necessary "
                    "if you want to cut, copy, paste, delete, or move "
                    "values. You can also press 'Shift+S' on your keyboard
    " "to activate this mode." ) ); m_moveButton->setWhatsThis( tr( "If
    you click here, move-mode will be activated. In this " "mode you can
    move the values you selected in select-" "mode. You can also press
    'Shift+M' on your keyboard " "to activate this mode." ) );*/

    connect(editModeGroup, SIGNAL(triggered(int)), m_editor,
            SLOT(setEditMode(int)));

    editActionsToolBar->addAction(drawAction);
    editActionsToolBar->addAction(eraseAction);
    //	editActionsToolBar->addAction(m_selectButton);
    //	editActionsToolBar->addAction(m_moveButton);

    // Interpolation actions
    DropToolBar* interpolationActionsToolBar
            = addDropToolBarToTop(tr("Interpolation controls"));

    ActionGroup* progression_type_group = new ActionGroup(this);

    m_discreteAction = progression_type_group->addAction(
            embed::getIcon("progression_discrete"),
            tr("Discrete progression"));
    m_discreteAction->setChecked(true);

    m_linearAction = progression_type_group->addAction(
            embed::getIcon("progression_linear"), tr("Linear progression"));
    m_cubicHermiteAction = progression_type_group->addAction(
            embed::getIcon("progression_cubic_hermite"),
            tr("Cubic Hermite progression"));
    m_parabolicAction = progression_type_group->addAction(
            embed::getIcon("progression_parabolic"),
            tr("Parabolic progression"));

    connect(progression_type_group, SIGNAL(triggered(int)), m_editor,
            SLOT(setProgressionType(int)));

    // setup tension-stuff
    // m_tensionKnob = new Knob( knobSmall_17, this, "Tension" );
    m_tensionKnob = new Knob(this, "Tension");
    m_tensionKnob->setText(tr("TENSN"));
    m_tensionKnob->setFixedSize(32, 32);
    m_tensionKnob->setPointColor(QColor(146, 74, 255));
    m_tensionKnob->setModel(m_editor->m_tensionModel);
    ToolTip::add(m_tensionKnob, tr("Tension value for spline"));
    m_tensionKnob->setHintText(tr("Tension:"), "");
    m_tensionKnob->setWhatsThis(
            tr("A higher tension value may make a smoother curve "
               "but overshoot some values. A low tension "
               "value will cause the slope of the curve to "
               "level off at each control point."));

    connect(m_cubicHermiteAction, SIGNAL(toggled(bool)), m_tensionKnob,
            SLOT(setEnabled(bool)));

    m_waveBankComboBox = new ComboBox(this, "Wave Type");
    m_waveBankComboBox->setFixedSize(32 * 4, 32);
    m_waveBankComboBox->setModel(m_editor->m_waveBankModel);

    m_waveIndexComboBox = new ComboBox(this, "Wave Type");
    m_waveIndexComboBox->setFixedSize(32 * 5, 32);
    m_waveIndexComboBox->setModel(m_editor->m_waveIndexModel);

    m_waveRatioKnob = new Knob(this, "Wave Ratio");
    m_waveRatioKnob->setText(tr("RATIO"));
    m_waveRatioKnob->setFixedSize(32, 32);
    m_waveRatioKnob->setPointColor(Qt::white);
    m_waveRatioKnob->setModel(m_editor->m_waveRatioModel);
    m_waveRatioKnob->setHintText(tr("Wave ratio:"), "");

    m_waveSkewKnob = new Knob(this, "Wave Skew");
    m_waveSkewKnob->setText(tr("SKEW"));
    m_waveSkewKnob->setFixedSize(32, 32);
    m_waveSkewKnob->setPointColor(Qt::white);
    m_waveSkewKnob->setModel(m_editor->m_waveSkewModel);
    m_waveSkewKnob->setHintText(tr("Wave skew:"), "");

    m_waveAmplitudeKnob = new Knob(this, "Wave Amplitude");
    m_waveAmplitudeKnob->setText(tr("AMP"));
    m_waveAmplitudeKnob->setFixedSize(32, 32);
    m_waveAmplitudeKnob->setPointColor(Qt::white);
    m_waveAmplitudeKnob->setModel(m_editor->m_waveAmplitudeModel);
    m_waveAmplitudeKnob->setHintText(tr("Wave amplitude:"), "");

    m_waveRepeatKnob = new Knob(this, "Wave Repeat");
    m_waveRepeatKnob->setText(tr("RPT"));
    m_waveRepeatKnob->setFixedSize(32, 32);
    m_waveRepeatKnob->setPointColor(Qt::white);
    m_waveRepeatKnob->setModel(m_editor->m_waveRepeatModel);
    m_waveRepeatKnob->setHintText(tr("Wave repeat:"), "");

    m_discreteAction->setWhatsThis(
            tr("Click here to choose discrete progressions for this "
               "automation pattern.  The value of the connected "
               "object will remain constant between control points "
               "and be set immediately to the new value when each "
               "control point is reached."));
    m_linearAction->setWhatsThis(
            tr("Click here to choose linear progressions for this "
               "automation pattern.  The value of the connected "
               "object will change at a steady rate over time "
               "between control points to reach the correct value at "
               "each control point without a sudden change."));
    m_cubicHermiteAction->setWhatsThis(
            tr("Click here to choose cubic hermite progressions for this "
               "automation pattern.  The value of the connected "
               "object will change in a smooth curve and ease in to "
               "the peaks and valleys."));

    interpolationActionsToolBar->addSeparator();
    interpolationActionsToolBar->addAction(m_discreteAction);
    interpolationActionsToolBar->addAction(m_linearAction);
    interpolationActionsToolBar->addAction(m_cubicHermiteAction);
    interpolationActionsToolBar->addAction(m_parabolicAction);
    interpolationActionsToolBar->addSeparator();
    // interpolationActionsToolBar->addWidget( new QLabel( tr("Tension:
    // "),
    //                                      interpolationActionsToolBar
    //                                      ));
    interpolationActionsToolBar->addWidget(m_tensionKnob);
    interpolationActionsToolBar->addSeparator();
    interpolationActionsToolBar->addWidget(m_waveBankComboBox);
    interpolationActionsToolBar->addSeparator();
    interpolationActionsToolBar->addWidget(m_waveIndexComboBox);
    interpolationActionsToolBar->addSeparator();
    interpolationActionsToolBar->addWidget(m_waveRatioKnob);
    interpolationActionsToolBar->addWidget(m_waveSkewKnob);
    interpolationActionsToolBar->addWidget(m_waveAmplitudeKnob);
    interpolationActionsToolBar->addWidget(m_waveRepeatKnob);

    // Copy paste buttons
    /*DropToolBar *copyPasteActionsToolBar = addDropToolBarToTop(tr("Copy
     * paste actions"));*/

    QAction* cutAction
            = new QAction(embed::getIcon("edit_cut"),
                          tr("Cut selected values (%1+X)").arg(UI_CTRL_KEY));
    QAction* copyAction
            = new QAction(embed::getIcon("edit_copy"),
                          tr("Copy selected values (%1+C)").arg(UI_CTRL_KEY));
    QAction* pasteAction = new QAction(
            embed::getIcon("edit_paste"),
            tr("Paste values from clipboard (%1+V)").arg(UI_CTRL_KEY));

    cutAction->setWhatsThis(
            tr("Click here and selected values will be cut into the "
               "clipboard.  You can paste them anywhere in any pattern "
               "by clicking on the paste button."));
    copyAction->setWhatsThis(
            tr("Click here and selected values will be copied into "
               "the clipboard.  You can paste them anywhere in any "
               "pattern by clicking on the paste button."));
    pasteAction->setWhatsThis(
            tr("Click here and the values from the clipboard will be "
               "pasted at the first visible measure."));

    cutAction->setShortcut(Qt::CTRL | Qt::Key_X);
    copyAction->setShortcut(Qt::CTRL | Qt::Key_C);
    pasteAction->setShortcut(Qt::CTRL | Qt::Key_V);

    connect(cutAction, SIGNAL(triggered()), m_editor,
            SLOT(cutSelectedValues()));
    connect(copyAction, SIGNAL(triggered()), m_editor,
            SLOT(copySelectedValues()));
    connect(pasteAction, SIGNAL(triggered()), m_editor, SLOT(pasteValues()));

    //	Select is broken
    //	copyPasteActionsToolBar->addAction( cutAction );
    //	copyPasteActionsToolBar->addAction( copyAction );
    //	copyPasteActionsToolBar->addAction( pasteAction );

    // Not implemented.
    // DropToolBar *timeLineToolBar = addDropToolBarToTop(tr("Timeline
    // controls")); m_editor->m_timeLine->addToolButtons(timeLineToolBar);

    addToolBarBreak();

    // Zoom controls
    DropToolBar* zoomToolBar = addDropToolBarToTop(tr("Zoom controls"));

    QLabel* zoomXLBL = new QLabel(zoomToolBar);
    zoomXLBL->setPixmap(embed::getPixmap("zoom_x"));
    zoomXLBL->setFixedSize(32, 32);
    zoomXLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    ComboBox* zoomXCBX = new ComboBox(zoomToolBar);
    zoomXCBX->setFixedSize(70, 32);
    EditorWindow::fillZoomLevels(m_editor->m_zoomingXModel, false);
    /*
    for(const real_t& zoomLevel: EditorWindow::ZOOM_LEVELS)
        m_editor->m_zoomingXModel.addItem(
                QString("%1\%").arg(zoomLevel * 100));
    */
    m_editor->m_zoomingXModel.setInitValue(
            m_editor->m_zoomingXModel.findText("100%", true, 0));
    zoomXCBX->setModel(&m_editor->m_zoomingXModel);

    new QShortcut(Qt::Key_Minus, zoomXCBX, SLOT(selectPrevious()));
    new QShortcut(Qt::Key_Plus, zoomXCBX, SLOT(selectNext()));

    connect(&m_editor->m_zoomingXModel, SIGNAL(dataChanged()), m_editor,
            SLOT(zoomingXChanged()));

    QLabel* zoomYLBL = new QLabel(zoomToolBar);
    zoomYLBL->setPixmap(embed::getPixmap("zoom_y"));
    zoomYLBL->setFixedSize(32, 32);
    zoomYLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    ComboBox* zoomYCBX = new ComboBox(zoomToolBar);
    zoomYCBX->setFixedSize(70, 32);
    EditorWindow::fillZoomLevels(m_editor->m_zoomingYModel, true);
    /*
    m_editor->m_zoomingYModel.addItem("Auto");
    for(int i = 0; i < 7; ++i)
        m_editor->m_zoomingYModel.addItem(QString::number(25 << i) + "%");
    */
    m_editor->m_zoomingYModel.setInitValue(
            m_editor->m_zoomingYModel.findText("Auto", true, 0));
    zoomYCBX->setModel(&m_editor->m_zoomingYModel);

    new QShortcut(Qt::SHIFT | Qt::Key_Minus, zoomYCBX,
                  SLOT(selectPrevious()));
    new QShortcut(Qt::SHIFT | Qt::Key_Plus, zoomYCBX, SLOT(selectNext()));

    connect(&m_editor->m_zoomingYModel, SIGNAL(dataChanged()), m_editor,
            SLOT(zoomingYChanged()));

    zoomToolBar->addWidget(zoomXLBL);
    zoomToolBar->addWidget(zoomXCBX);
    zoomToolBar->addSeparator();
    zoomToolBar->addWidget(zoomYLBL);
    zoomToolBar->addWidget(zoomYCBX);

    // Quantization controls
    DropToolBar* quantizationActionsToolBar
            = addDropToolBarToTop(tr("Quantization controls"));

    QLabel* quantizeXLBL = new QLabel(m_toolBar);
    quantizeXLBL->setPixmap(embed::getPixmap("quantize_x"));
    quantizeXLBL->setFixedSize(32, 32);
    quantizeXLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    ComboBox* quantizeXCBX = new ComboBox(m_toolBar);
    quantizeXCBX->setFixedSize(70, 32);
    quantizeXCBX->setModel(&m_editor->m_quantizeXModel);
    quantizeXCBX->setToolTip(tr("Quantization"));
    quantizeXCBX->setWhatsThis(
            tr("Quantization. Sets the smallest "
               "step size for the Automation Point. By default "
               "this also sets the length, clearing out other "
               "points in the range. Press <Ctrl> to override "
               "this behaviour."));

    QLabel* quantizeYLBL = new QLabel(m_toolBar);
    quantizeYLBL->setPixmap(embed::getPixmap("quantize_y"));
    quantizeYLBL->setFixedSize(32, 32);
    quantizeYLBL->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    ComboBox* quantizeYCBX = new ComboBox(m_toolBar);
    quantizeYCBX->setFixedSize(70, 32);
    quantizeYCBX->setModel(&m_editor->m_quantizeYModel);
    quantizeYCBX->setToolTip(tr("Quantization"));
    quantizeYCBX->setWhatsThis(
            tr("Value quantization. Sets the smallest "
               "step size for the Automation Point."));

    quantizationActionsToolBar->addWidget(quantizeXLBL);
    quantizationActionsToolBar->addWidget(quantizeXCBX);
    quantizationActionsToolBar->addWidget(quantizeYLBL);
    quantizationActionsToolBar->addWidget(quantizeYCBX);

    // Ops
    DropToolBar* opsToolBar = addDropToolBarToTop(tr("Ops"));
    opsToolBar->addSeparator();

    m_moveAbsDownAction = new QAction(embed::getIcon("arrow_bottom"),
                                      tr("Down the selection"), opsToolBar);
    m_moveAbsUpAction   = new QAction(embed::getIcon("arrow_top"),
                                    tr("Up the selection"), opsToolBar);
    m_moveRelDownAction = new QAction(embed::getIcon("arrow_down"),
                                      tr("Down the selection"), opsToolBar);
    m_moveRelUpAction   = new QAction(embed::getIcon("arrow_up"),
                                    tr("Up the selection"), opsToolBar);

    /*
    QToolButton* downChord12BTN = new QToolButton(opsToolBar);
    downChord12BTN->setIcon(embed::getIcon("arrow_bottom"));
    downChord12BTN->setToolTip(tr("Down the selection"));
    connect(downChord12BTN, SIGNAL(clicked()), m_editor,
    SLOT(downChord12()));

    QToolButton* upChord12BTN = new QToolButton(opsToolBar);
    upChord12BTN->setIcon(embed::getIcon("arrow_top"));
    upChord12BTN->setToolTip(tr("Up the selection"));
    connect(upChord12BTN, SIGNAL(clicked()), m_editor, SLOT(upChord12()));

    QToolButton* downChord1BTN = new QToolButton(opsToolBar);
    downChord1BTN->setIcon(embed::getIcon("arrow_down"));
    downChord1BTN->setToolTip(tr("Down the selection"));
    connect(downChord1BTN, SIGNAL(clicked()), m_editor,
    SLOT(downChord1()));

    QToolButton* upChord1BTN = new QToolButton(opsToolBar);
    upChord1BTN->setIcon(embed::getIcon("arrow_up"));
    upChord1BTN->setToolTip(tr("Up the selection"));
    connect(upChord1BTN, SIGNAL(clicked()), m_editor, SLOT(upChord1()));

    QToolButton* downScaleBTN = new QToolButton(opsToolBar);
    downScaleBTN->setIcon(embed::getIcon("remove"));
    downScaleBTN->setToolTip(tr("Down the selection to the scale"));
    connect(downScaleBTN, SIGNAL(clicked()), m_editor, SLOT(downScale()));

    QToolButton* upScaleBTN = new QToolButton(opsToolBar);
    upScaleBTN->setIcon(embed::getIcon("add"));
    upScaleBTN->setToolTip(tr("Up the selection to the scale"));
    connect(upScaleBTN, SIGNAL(clicked()), m_editor, SLOT(upScale()));
    */

    m_flipYAction = new QAction(embed::getIcon("flip_y"),
                                tr("Flip vertically"), opsToolBar);
    m_flipXAction = new QAction(embed::getIcon("flip_x"),
                                tr("Flip horizontally"), opsToolBar);

    m_flipYAction->setWhatsThis(
            tr("Click here and the pattern will be inverted."
               "The points are flipped in the y direction. "));
    m_flipXAction->setWhatsThis(
            tr("Click here and the pattern will be reversed. "
               "The points are flipped in the x direction."));

    opsToolBar->addAction(m_moveAbsDownAction);
    opsToolBar->addAction(m_moveAbsUpAction);
    opsToolBar->addAction(m_moveRelDownAction);
    opsToolBar->addAction(m_moveRelUpAction);
    opsToolBar->addAction(m_flipXAction);
    opsToolBar->addAction(m_flipYAction);

    // Setup our actual window
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setWindowIcon(embed::getIcon("automation"));

    setAcceptDrops(true);
    m_toolBar->setAcceptDrops(true);

    // Connections
    connect(m_editor, SIGNAL(currentPatternChanged()), this,
            SIGNAL(currentPatternChanged()));
    connect(m_editor, SIGNAL(currentPatternChanged()), this,
            SLOT(updateWindowTitle()));
}

AutomationWindow::~AutomationWindow()
{
}

void AutomationWindow::reset()
{
    setCurrentPattern(nullptr);
}

const AutomationPattern* AutomationWindow::currentPattern()
{
    return m_editor->currentPattern();
}

void AutomationWindow::setCurrentPattern(AutomationPattern* pattern)
{
    const AutomationPattern* old = m_editor->currentPattern();
    if(old == pattern)
        return;

    // Disconnect our old pattern
    if(old != nullptr)
    {
        old->disconnect(this);
        m_moveAbsUpAction->disconnect();
        m_moveAbsDownAction->disconnect();
        m_moveRelUpAction->disconnect();
        m_moveRelDownAction->disconnect();
        m_flipXAction->disconnect();
        m_flipYAction->disconnect();
    }

    m_editor->setCurrentPattern(pattern);

    updateWindowTitle();

    if(pattern == nullptr)
        return;

    switch(pattern->progressionType())
    {
        case AutomationPattern::DiscreteProgression:
            m_discreteAction->setChecked(true);
            m_tensionKnob->setEnabled(false);
            break;
        case AutomationPattern::LinearProgression:
            m_linearAction->setChecked(true);
            m_tensionKnob->setEnabled(false);
            break;
        case AutomationPattern::CubicHermiteProgression:
            m_cubicHermiteAction->setChecked(true);
            m_tensionKnob->setEnabled(true);
            break;
        case AutomationPattern::ParabolicProgression:
            m_parabolicAction->setChecked(true);
            m_tensionKnob->setEnabled(false);
            break;
    }

    // Connect new pattern
    connect(pattern->automationTrack(), SIGNAL(nameChanged()), this,
            SLOT(updateWindowTitle()));
    connect(pattern, SIGNAL(dataChanged()), this, SLOT(update()));
    connect(pattern, SIGNAL(dataChanged()), this, SLOT(updateWindowTitle()));
    connect(pattern, SIGNAL(destroyed()), this, SLOT(clearCurrentPattern()));

    connect(m_moveAbsUpAction, SIGNAL(triggered()), pattern,
            SLOT(moveAbsUp()));
    connect(m_moveAbsDownAction, SIGNAL(triggered()), pattern,
            SLOT(moveAbsDown()));
    connect(m_moveRelUpAction, SIGNAL(triggered()), pattern,
            SLOT(moveRelUp()));
    connect(m_moveRelDownAction, SIGNAL(triggered()), pattern,
            SLOT(moveRelDown()));
    connect(m_flipXAction, SIGNAL(triggered()), pattern, SLOT(flipX()));
    connect(m_flipYAction, SIGNAL(triggered()), pattern, SLOT(flipY()));

    emit currentPatternChanged();
}

void AutomationWindow::clearCurrentPattern()
{
    // m_editor->m_pattern = nullptr;
    setCurrentPattern(nullptr);
}

void AutomationWindow::dropEvent(QDropEvent* _de)
{
    QString type = StringPairDrag::decodeKey(_de);
    QString val  = StringPairDrag::decodeValue(_de);
    if(type == "automatable_model")
    {
        AutomatableModel* mod
                = dynamic_cast<AutomatableModel*>(Model::find(val));
        if(mod == nullptr)
            mod = dynamic_cast<AutomatableModel*>(
                    Engine::projectJournal()->journallingObject(val.toInt()));

        if(mod != nullptr)
        {
            bool added = m_editor->m_pattern->addObject(mod);
            if(!added)
            {
                TextFloat::displayMessage(mod->displayName(),
                                          tr("Model is already connected "
                                             "to this pattern."),
                                          embed::getPixmap("automation"),
                                          2000);
            }
            setCurrentPattern(m_editor->m_pattern);
        }
    }

    update();
}

void AutomationWindow::dragEnterEvent(QDragEnterEvent* _dee)
{
    if(m_editor->m_pattern != nullptr)
        StringPairDrag::processDragEnterEvent(_dee, "automatable_model");
}

void AutomationWindow::open(AutomationPattern* pattern)
{
    setCurrentPattern(pattern);

    parentWidget()->show();
    show();
    setFocus();
}

QSize AutomationWindow::sizeHint() const
{
    // static const int INITIAL_WIDTH  = 860;
    // static const int INITIAL_HEIGHT = 480;
    // return {INITIAL_WIDTH, INITIAL_HEIGHT};
    return {860, 480};
}

void AutomationWindow::updateWindowTitle()
{
    const AutomationPattern* p = currentPattern();
    setWindowTitle(p != nullptr ? tr("Automation - %1").arg(p->name())
                                : tr("Automation - no tile"));
}

void AutomationWindow::focusInEvent(QFocusEvent* event)
{
    // when the window is given focus, also give focus to the actual piano
    // roll
    m_editor->setFocus(event->reason());
}

void AutomationWindow::play()
{
    m_editor->play();
    setPauseIcon(Engine::song()->isPlaying());
}

void AutomationWindow::stop()
{
    m_editor->stop();
}

// ActionUpdatable //

void AutomationWindow::updateActions(const bool            _active,
                                     QHash<QString, bool>& _table) const
{
    // qInfo("AutomationWindow::updateActions() active=%d",_active);
    m_editor->updateActions(_active, _table);
}

void AutomationWindow::actionTriggered(QString _name)
{
    m_editor->actionTriggered(_name);
}

void AutomationEditor::updateActions(const bool            _active,
                                     QHash<QString, bool>& _table) const
{
    // qInfo("AutomationEditor::updateActions() active=%d",_active);
    bool hasPoints = _active && m_pattern && (m_pattern->timeMapLength() > 0);
    // qInfo("AutomationWindow::updateActions() active=%d selection=%d",
    //      _active,hasSelection);
    _table.insert("edit_cut", false);
    _table.insert("edit_copy", hasPoints);
    _table.insert("edit_paste", false);
}

void AutomationEditor::actionTriggered(QString _name)
{
    qInfo("AutomationEditor::actionTriggered() name=%s", qPrintable(_name));
    if(_name == "edit_cut")
        cutSelection();
    else if(_name == "edit_copy")
        copySelection();
    else if(_name == "edit_paste")
        pasteSelection();
}

// Selection //

void AutomationEditor::deleteSelection()
{
}

void AutomationEditor::cutSelection()
{
    // copySelection();
    // deleteSelection();
}

void AutomationEditor::copySelection()
{
    if(m_pattern == nullptr)
        return;

    QString r   = "";
    auto    map = m_pattern->getTimeMap();
    for(auto t: map.keys())
    {
        real_t x = real_t(t) / MidiTime::ticksPerTact();
        real_t y = map.value(t);

        r.append(QString::number(t));
        r.append(QChar(','));
        r.append(QString::number(x, 'f'));
        r.append(QChar(','));
        r.append(QString::number(y, 'f'));

        // for(auto m: m_pattern->objects())
        m_pattern->objects().map([&r, y](auto m) {
            const real_t v = m->scaledValue(y);
            r.append(QChar(','));
            r.append(QString::number(v, 'f'));
        });

        r.append(QChar('\n'));
    }

    qInfo("Automation\n%s", qPrintable(r));
}

void AutomationEditor::pasteSelection()
{
    // Clipboard::paste(vso)
}
