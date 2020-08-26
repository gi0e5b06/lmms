/*
 * Pattern.h - declaration of class Pattern, which contains all information
 *             about a pattern
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

#ifndef PATTERN_H_
#define PATTERN_H_

#include "Note.h"
#include "Track.h"

//#include <QVector>
//#include <QWidget>
//#include <QDialog>
//#include <QThread>
#include <QPixmap>
#include <QStaticText>

class QAction;
class QProgressBar;
class QPushButton;

class InstrumentTrack;
class SampleBuffer;

class EXPORT Pattern : public Tile
{
    Q_OBJECT

  public:
    enum PatternTypes
    {
        BeatPattern,
        MelodyPattern
    };

    Pattern(InstrumentTrack* instrumentTrack);
    Pattern(const Pattern& other);
    virtual ~Pattern();

    bool empty()
    {
        return isEmpty();
    }  // obsolete

    bool isEmpty() const override;
    // QString defaultName() const override;
    tick_t unitLength() const override;
    void   rotate(tick_t _ticks) override;
    void   splitEvery(tick_t _ticks) override;
    void   splitAt(tick_t _tick) override;

    void init();

    virtual void updateLength();
    // MidiTime beatPatternLength() const;

    // note management
    Note* addNote(const Note& _new_note, const bool _quant_pos = true);
    void  removeNote(Note* _note_to_del);

    void rearrangeAllNotes();
    void clearNotes();

    // all notes of the pattern, once
    const Notes& notes() const
    {
        return m_notes;
    }

    const Chords chords() const;

    Note* noteAtStep(int _step);
    Note* addStepNote(int step);
    void  removeStepNote(int step);
    void  toggleStepNote(int step);
    void  setStepNote(int step, bool enabled);

    // pattern-type stuff
    INLINE PatternTypes type() const
    {
        return m_patternType;
    }

    // next/previous track based on position in the containing track
    // Pattern* previousPattern() const;
    // Pattern* nextPattern() const;

    // settings-management
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "pattern";
    }

    INLINE InstrumentTrack* instrumentTrack() const
    {
        return m_instrumentTrack;
    }

    virtual TileView* createView(TrackView* _tv);

    // signals:
    // using Model::dataChanged(); <-- not working
    // void dataChanged(); <-- not needed

  public slots:
    void clear() override;
    void cloneSteps() override;
    void flipHorizontally() override;
    void flipVertically() override;

  protected:
    // void updateBBTrack();

  protected slots:
    void changeTimeSignature();
    /*
    void addBarSteps();
    void addBeatSteps();
    void addOneStep();
    void removeBarSteps();
    void removeBeatSteps();
    void removeOneStep();
    void setStepResolution(int _res);
    int      stepsPerTact() const;
    MidiTime stepPosition(int _step) const;
    */

  private:
    void setType(PatternTypes _new_pattern_type);
    void checkType();
    void resizeToFirstTrack();
    // Pattern* adjacentPatternByOffset(int offset) const;

    InstrumentTrack* m_instrumentTrack;
    PatternTypes     m_patternType;
    Notes            m_notes;

    friend class PatternView;
    friend class BBEditor;

  signals:
    void destroyedPattern(Pattern*);
};

class PatternView : public TileView
{
    Q_OBJECT

  public:
    Q_PROPERTY(
            QColor noteFillColor READ getNoteFillColor WRITE setNoteFillColor)
    Q_PROPERTY(QColor noteBorderColor READ getNoteBorderColor WRITE
                       setNoteBorderColor)
    Q_PROPERTY(QColor mutedNoteFillColor READ getMutedNoteFillColor WRITE
                       setMutedNoteFillColor)
    Q_PROPERTY(QColor mutedNoteBorderColor READ getMutedNoteBorderColor WRITE
                       setMutedNoteBorderColor)

    QColor const& getNoteFillColor() const
    {
        return m_noteFillColor;
    }
    void setNoteFillColor(QColor const& color)
    {
        m_noteFillColor = color;
    }

    QColor const& getNoteBorderColor() const
    {
        return m_noteBorderColor;
    }
    void setNoteBorderColor(QColor const& color)
    {
        m_noteBorderColor = color;
    }

    QColor const& getMutedNoteFillColor() const
    {
        return m_mutedNoteFillColor;
    }
    void setMutedNoteFillColor(QColor const& color)
    {
        m_mutedNoteFillColor = color;
    }

    QColor const& getMutedNoteBorderColor() const
    {
        return m_mutedNoteBorderColor;
    }
    void setMutedNoteBorderColor(QColor const& color)
    {
        m_mutedNoteBorderColor = color;
    }

  public slots:
    virtual void update();

    // virtual void remove();
    // virtual void cut();
    // virtual void copy();
    // virtual void paste();
    // virtual void changeName();
    // virtual void resetName();
    // virtual void changeColor();
    // virtual void resetColor();

    void openInPianoRoll();
    void setGhostInPianoRoll();
    void setNameFromFirstNote();

    void changeStepResolution(QAction* _a);

    static PatternView* create(Pattern* _p, TrackView* _tv)
    {
        return new PatternView(_p, _tv);
    }

  protected:
    PatternView(Pattern* pattern, TrackView* parent);
    virtual ~PatternView();

    virtual QMenu* buildContextMenu();
    // virtual void addStepMenu(QMenu* _cm, bool _enabled) final;

    virtual void mousePressEvent(QMouseEvent* _me);
    virtual void mouseDoubleClickEvent(QMouseEvent* _me);
    virtual void paintEvent(QPaintEvent* pe);
    virtual void wheelEvent(QWheelEvent* _we);
    virtual int  mouseToStep(int _x, int _y);

  private:
    static QPixmap* s_stepBtnOn0;
    static QPixmap* s_stepBtnOn200;
    static QPixmap* s_stepBtnOff;
    static QPixmap* s_stepBtnOffLight;

    Pattern* m_pat;
    QPixmap  m_paintPixmap;

    QColor m_noteFillColor;
    QColor m_noteBorderColor;
    QColor m_mutedNoteFillColor;
    QColor m_mutedNoteBorderColor;

    QStaticText m_staticTextName;
};

#endif
