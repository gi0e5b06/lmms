/*
 * AutomationEditor.h - a window where you can edit dynamic values in an easy
 *                      way
 *
 * Copyright (c) 2018      gi0e5b06 (on github.com)
 * Copyright (c) 2006-2008 Javier Serrano Polo
 * <jasp00/at/users.sourceforge.net>
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

#ifndef AUTOMATION_EDITOR_H
#define AUTOMATION_EDITOR_H

#include "AutomationPattern.h"
#include "ComboBoxModel.h"
#include "Editor.h"
#include "JournallingObject.h"
#include "Knob.h"
#include "MidiTime.h"
#include "lmms_basics.h"

#include <QMutex>
#include <QVector>
#include <QWidget>

class QPainter;
class QPixmap;
class QScrollBar;

class ComboBox;
class NotePlayHandle;
class TimeLineWidget;

class AutomationEditor
      : public QWidget
      , public JournallingObject
      , public virtual ActionUpdatable
{
    Q_OBJECT
    Q_PROPERTY(QColor barLineColor READ barLineColor WRITE setBarLineColor)
    Q_PROPERTY(QColor beatLineColor READ beatLineColor WRITE setBeatLineColor)
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor)
    Q_PROPERTY(QColor vertexColor READ vertexColor WRITE setVertexColor)
    Q_PROPERTY(QBrush scaleColor READ scaleColor WRITE setScaleColor)
    Q_PROPERTY(QBrush graphColor READ graphColor WRITE setGraphColor)
    Q_PROPERTY(QColor crossColor READ crossColor WRITE setCrossColor)
    Q_PROPERTY(QColor backgroundShade READ backgroundShade WRITE
                                                           setBackgroundShade)
  public:
    void setCurrentPattern(AutomationPattern* new_pattern);

    inline const AutomationPattern* currentPattern() const
    {
        return m_pattern;
    }

    inline bool validPattern() const
    {
        return m_pattern != nullptr;
    }

    virtual void saveSettings(QDomDocument& doc, QDomElement& parent);
    virtual void loadSettings(const QDomElement& parent);

    QString nodeName() const
    {
        return "automationeditor";
    }

    virtual void updateActions(const bool            _active,
                               QHash<QString, bool>& _table) const;  // = 0;
    virtual void actionTriggered(QString _name);

    // qproperty access methods
    QColor barLineColor() const;
    void   setBarLineColor(const QColor& c);
    QColor beatLineColor() const;
    void   setBeatLineColor(const QColor& c);
    QColor lineColor() const;
    void   setLineColor(const QColor& c);
    QBrush graphColor() const;
    void   setGraphColor(const QBrush& c);
    QColor vertexColor() const;
    void   setVertexColor(const QColor& c);
    QBrush scaleColor() const;
    void   setScaleColor(const QBrush& c);
    QColor crossColor() const;
    void   setCrossColor(const QColor& c);
    QColor backgroundShade() const;
    void   setBackgroundShade(const QColor& c);

    enum EditModes
    {
        DRAW,
        ERASE,
        SELECT,
        MOVE
    };

  public slots:
    void update();
    void updateAfterPatternChange();

    void deleteSelection();
    void cutSelection();
    void copySelection();
    void pasteSelection();

  protected:
    typedef AutomationPattern::timeMap timeMap;

    virtual void keyPressEvent(QKeyEvent* ke);
    virtual void leaveEvent(QEvent* e);
    virtual void mousePressEvent(QMouseEvent* mouseEvent);
    virtual void mouseReleaseEvent(QMouseEvent* mouseEvent);
    virtual void mouseMoveEvent(QMouseEvent* mouseEvent);
    virtual void paintEvent(QPaintEvent* pe);
    virtual void resizeEvent(QResizeEvent* re);
    virtual void wheelEvent(QWheelEvent* we);

    real_t      getLevel(int y);
    int         xCoordOfTick(int tick);
    real_t      yCoordOfLevel(real_t level);
    inline void drawLevelTick(
            QPainter& p,
            int       tick,
            real_t    value);  // bool is_selected ); //NEEDS Change in CSS
    void removeSelection();
    void selectAll();
    void getSelectedValues(timeMap& selected_values);

    void drawLine(int x0, real_t y0, int x1, real_t y1);

  protected slots:
    void play();
    void stop();

    void horScrolled(int new_pos);
    void verScrolled(int new_pos);

    void setEditMode(AutomationEditor::EditModes mode);
    void setEditMode(int mode);

    void setProgressionType(AutomationPattern::ProgressionTypes type);
    void setProgressionType(int type);
    void setTension();
    void setWaveBank();
    void setWaveIndex();
    void setWaveRatio();
    void setWaveSkew();
    void setWaveAmplitude();
    void setWaveRepeat();

    void copySelectedValues();
    void cutSelectedValues();
    void pasteValues();
    void deleteSelectedValues();

    void updatePosition(const MidiTime& t);

    void zoomingXChanged();
    void zoomingYChanged();

    /// Updates the pattern's quantization using the current user selected
    /// value.
    void setQuantization();

  private:
    enum Actions
    {
        NONE,
        MOVE_VALUE,
        SELECT_VALUES,
        MOVE_SELECTION
    };

    // some constants...
    static const int SCROLLBAR_SIZE = 12;
    static const int TOP_MARGIN     = 16;

    static const int DEFAULT_Y_DELTA        = 6;
    static const int DEFAULT_STEPS_PER_TACT = 16;
    static const int DEFAULT_PPT            = 12 * DEFAULT_STEPS_PER_TACT;

    static const int VALUES_WIDTH = 64;

    AutomationEditor();
    AutomationEditor(const AutomationEditor&);
    virtual ~AutomationEditor();

    static QPixmap* s_toolDraw;
    static QPixmap* s_toolErase;
    static QPixmap* s_toolSelect;
    static QPixmap* s_toolMove;
    static QPixmap* s_toolYFlip;
    static QPixmap* s_toolXFlip;

    ComboBoxModel m_zoomingXModel;
    ComboBoxModel m_zoomingYModel;
    ComboBoxModel m_quantizeModel;

    // static const QVector<double> m_zoomXLevels;

    FloatModel*    m_tensionModel;
    ComboBoxModel* m_waveBankModel;
    ComboBoxModel* m_waveIndexModel;
    FloatModel*    m_waveRatioModel;
    FloatModel*    m_waveSkewModel;
    FloatModel*    m_waveAmplitudeModel;
    FloatModel*    m_waveRepeatModel;

    QMutex             m_patternMutex;
    AutomationPattern* m_pattern;
    real_t             m_minLevel;
    real_t             m_maxLevel;
    real_t             m_step;
    real_t             m_scrollLevel;
    real_t             m_bottomLevel;
    real_t             m_topLevel;

    void updateTopBottomLevels();

    QScrollBar* m_leftRightScroll;
    QScrollBar* m_topBottomScroll;

    MidiTime m_currentPosition;

    Actions m_action;

    tick_t m_selectStartTick;
    tick_t m_selectedTick;
    real_t m_selectStartLevel;
    real_t m_selectedLevels;

    real_t m_moveStartLevel;
    tick_t m_moveStartTick;
    int    m_moveXOffset;

    real_t m_drawLastLevel;
    tick_t m_drawLastTick;

    int  m_ppt;
    int  m_y_delta;
    bool m_y_auto;

    timeMap m_valuesToCopy;
    timeMap m_selValuesForMove;

    EditModes m_editMode;

    bool m_mouseDownRight;  // true if right click is being held down

    TimeLineWidget* m_timeLine;
    bool            m_scrollBack;

    void drawCross(QPainter& p);
    void drawAutomationPoint(QPainter& p, timeMap::iterator it);
    bool inBBEditor();

    QColor m_barLineColor;
    QColor m_beatLineColor;
    QColor m_lineColor;
    QBrush m_graphColor;
    QColor m_vertexColor;
    QBrush m_scaleColor;
    QColor m_crossColor;
    QColor m_backgroundShade;

    friend class AutomationEditorWindow;

  signals:
    void currentPatternChanged();
    void positionChanged(const MidiTime&);
};

class AutomationEditorWindow : public Editor
{
    Q_OBJECT

    static const int INITIAL_WIDTH  = 860;
    static const int INITIAL_HEIGHT = 480;

  public:
    AutomationEditorWindow();
    virtual ~AutomationEditorWindow();

    void reset();

    void                     setCurrentPattern(AutomationPattern* pattern);
    const AutomationPattern* currentPattern();

    virtual void dropEvent(QDropEvent* _de);
    virtual void dragEnterEvent(QDragEnterEvent* _dee);

    void open(AutomationPattern* pattern);

    AutomationEditor* m_editor;

    virtual void updateActions(const bool            _active,
                               QHash<QString, bool>& _table) const;  // = 0;
    virtual void actionTriggered(QString _name);

    QSize sizeHint() const;

  public slots:
    void clearCurrentPattern();

  signals:
    void currentPatternChanged();

  protected slots:
    void play();
    void stop();

  private slots:
    void updateWindowTitle();

  private:
    QAction* m_discreteAction;
    QAction* m_linearAction;
    QAction* m_cubicHermiteAction;
    QAction* m_parabolicAction;

    QAction* m_flipYAction;
    QAction* m_flipXAction;

    Knob* m_tensionKnob;

    ComboBox* m_waveBankComboBox;
    ComboBox* m_waveIndexComboBox;
    Knob*     m_waveRatioKnob;
    Knob*     m_waveSkewKnob;
    Knob*     m_waveAmplitudeKnob;
    Knob*     m_waveRepeatKnob;

    ComboBox* m_zoomingXComboBox;
    ComboBox* m_zoomingYComboBox;
    ComboBox* m_quantizeComboBox;
};

#endif
