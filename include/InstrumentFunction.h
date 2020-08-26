/*
 * InstrumentFunction.h - models for instrument-functions-tab
 *
 * Copyright (c) 2017-2020 gi0e5b06 (on github.com)
 * Copyright (c) 2004-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef INSTRUMENT_FUNCTION_H
#define INSTRUMENT_FUNCTION_H

#include <QMultiHash>
#include <QMutex>

//#include "AutomatableModel.h"
#include "Chord.h"
#include "ComboBoxModel.h"
#include "Note.h"
#include "NotePlayHandle.h"
#include "PlayHandle.h"
//#include "JournallingObject.h"
#include "TempoSyncKnobModel.h"

//#include "lmms_basics.h"

class InstrumentTrack;
// class NotePlayHandle;
class InstrumentFunctionView;

class InstrumentFunction : public Model, public JournallingObject
{
    Q_OBJECT

  public:
    virtual ~InstrumentFunction()
    {
    }

    virtual bool processNote(NotePlayHandle* n)                         = 0;
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent) = 0;
    virtual void loadSettings(const QDomElement& _this)                 = 0;
    virtual QString nodeName() const                                    = 0;

    virtual InstrumentFunctionView* createView() = 0;

    IntModel* minNoteGenerationModel()
    {
        return &m_minNoteGenerationModel;
    }
    IntModel* maxNoteGenerationModel()
    {
        return &m_maxNoteGenerationModel;
    }
    // INLINE int minNoteGeneration() const { return
    // m_minNoteGenerationModel.value(); } INLINE void
    // setMinNoteGeneration(int _min) {
    // m_minNoteGenerationModel.setValue(_min); } INLINE int
    // maxNoteGeneration() const { return m_maxNoteGenerationModel.value(); }
    // INLINE void setMaxNoteGeneration(int _max) {
    // m_maxNoteGenerationModel.setValue(_max); }

  protected:
    InstrumentFunction(Model* _parent, QString _name);
    virtual bool shouldProcessNote(NotePlayHandle* n);

    BoolModel m_enabledModel;
    IntModel  m_minNoteGenerationModel;
    IntModel  m_maxNoteGenerationModel;
};

class InstrumentFunctionNoteStacking : public InstrumentFunction
{
    Q_OBJECT

    /*
    public:
      static const int MAX_CHORD_POLYPHONY = 13;

    private:
      typedef int8_t ChordSemitones[MAX_CHORD_POLYPHONY];
    */
  public:
    InstrumentFunctionNoteStacking(Model* _parent);
    virtual ~InstrumentFunctionNoteStacking();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "chordcreator";
    }

    virtual InstrumentFunctionView* createView();

    /*
    struct Chord
    {
      private:
        QString        m_name;
        ChordSemitones m_semitones;
        int            m_size;

      public:
        Chord() : m_size(0)
        {
        }

        Chord(const char* _name, const ChordSemitones& _semitones);

        int size() const
        {
            return m_size;
        }

        bool isScale() const
        {
            return size() > 6;
        }

        bool isEmpty() const
        {
            return size() == 0;
        }

        bool hasSemitone(int8_t _semitone) const;

        int8_t last() const
        {
            return m_semitones[size() - 1];
        }

        const QString& getName() const
        {
            return m_name;
        }

        int8_t operator[](int n) const
        {
            return m_semitones[n];
        }
    };

    struct ChordTable : public QVector<Chord>
    {
      private:
        ChordTable();

        struct Init
        {
            const char*    m_name;
            ChordSemitones m_semitones;
        };

        static Init s_initTable[];

      public:
        static const ChordTable& getInstance()
        {
            static ChordTable inst;
            return inst;
        }

        const Chord& getByName(const QString& name,
                               bool           is_scale = false) const;

        const Chord& getScaleByName(const QString& name) const
        {
            return getByName(name, true);
        }

        const Chord& getChordByName(const QString& name) const
        {
            return getByName(name, false);
        }
    };
    */

  private:
    // BoolModel m_chordsEnabledModel;
    ComboBoxModel m_chordsModel;
    RealModel     m_chordRangeModel;

    friend class InstrumentFunctionNoteStackingView;
};

class InstrumentFunctionArpeggio : public InstrumentFunction
{
    Q_OBJECT

  public:
    enum ArpDirections
    {
        ArpDirUp,
        ArpDirDown,
        ArpDirUpAndDown,
        ArpDirDownAndUp,
        ArpDirRandom,
        NumArpDirections
    };

    InstrumentFunctionArpeggio(Model* _parent);
    virtual ~InstrumentFunctionArpeggio();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "arpeggiator";
    }

    virtual InstrumentFunctionView* createView();

  private:
    enum ArpModes
    {
        FreeMode,
        SortMode,
        SyncMode
    };

    // BoolModel m_arpEnabledModel;
    ComboBoxModel      m_arpModel;
    RealModel          m_arpRangeModel;
    RealModel          m_arpCycleModel;
    RealModel          m_arpSkipModel;
    RealModel          m_arpMissModel;
    TempoSyncKnobModel m_arpTimeModel;
    RealModel          m_arpGateModel;
    ComboBoxModel      m_arpDirectionModel;
    ComboBoxModel      m_arpModeModel;
    RealModel          m_arpBaseModel;
    RealModel          m_arpRepeatModel;
    RealModel          m_arpLimitModel;

    friend class InstrumentTrack;
    friend class InstrumentFunctionArpeggioView;
};

class InstrumentFunctionNoteHumanizing : public InstrumentFunction
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteHumanizing(Model* _parent);
    virtual ~InstrumentFunctionNoteHumanizing();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "notehumanizing";
    }

    virtual InstrumentFunctionView* createView();

  private:
    RealModel m_volumeRangeModel;
    RealModel m_panRangeModel;
    RealModel m_tuneRangeModel;
    RealModel m_offsetRangeModel;
    RealModel m_shortenRangeModel;
    RealModel m_lengthenRangeModel;

    friend class InstrumentFunctionNoteHumanizingView;
};

class InstrumentFunctionNoteDuplicatesRemoving : public InstrumentFunction
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteDuplicatesRemoving(Model* _parent);
    virtual ~InstrumentFunctionNoteDuplicatesRemoving();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "noteduplicatesremoving";
    }

    virtual InstrumentFunctionView* createView();

  private:
    QMultiHash<int64_t, int> m_cache;
    QMutex                   m_mutex;

    friend class InstrumentFunctionNoteDuplicatesRemovingView;
};

class InstrumentFunctionNoteFiltering : public InstrumentFunction
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteFiltering(Model* _parent);
    virtual ~InstrumentFunctionNoteFiltering();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "notefiltering";
    }

    virtual InstrumentFunctionView* createView();

  public slots:
    void onRootOrModeChanged();

  private:
    ComboBoxModel  m_configModel;
    ComboBoxModel* m_actionModel[4];
    BoolModel*     m_noteSelectionModel[4][KeysPerOctave];
    RealModel*     m_intervalModel[4];
    ComboBoxModel* m_rootModel[4];
    ComboBoxModel* m_modeModel[4];

    friend class InstrumentFunctionNoteFilteringView;
};

class InstrumentFunctionNoteKeying : public InstrumentFunction
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteKeying(Model* _parent);
    virtual ~InstrumentFunctionNoteKeying();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "notekeying";
    }

    virtual InstrumentFunctionView* createView();

  private:
    RealModel m_volumeRangeModel;
    RealModel m_volumeBaseModel;
    RealModel m_volumeMinModel;
    RealModel m_volumeMaxModel;

    RealModel m_panRangeModel;
    RealModel m_panBaseModel;
    RealModel m_panMinModel;
    RealModel m_panMaxModel;

    friend class InstrumentFunctionNoteKeyingView;
};

class InstrumentFunctionNoteOutting : public InstrumentFunction
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteOutting(Model* _parent);
    virtual ~InstrumentFunctionNoteOutting();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "noteoutting";
    }

    virtual InstrumentFunctionView* createView();

  private:
    RealModel m_volumeModel;
    RealModel m_panModel;
    RealModel m_keyModel;
    RealModel m_noteModel;
    RealModel m_modValueModel;
    RealModel m_modRefKeyModel;
    RealModel m_modAmountModel;
    RealModel m_modBaseModel;

    friend class InstrumentFunctionNoteOuttingView;
};

class InstrumentFunctionGlissando : public InstrumentFunction
{
    Q_OBJECT

  public:
    InstrumentFunctionGlissando(Model* _parent);
    virtual ~InstrumentFunctionGlissando();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "glissando";
    }

    virtual InstrumentFunctionView* createView();

  public slots:
    void reset();

  private:
    TempoSyncKnobModel m_gliTimeModel;
    RealModel          m_gliGateModel;
    RealModel          m_gliAttenuationModel;
    ComboBoxModel      m_gliUpModeModel;
    ComboBoxModel      m_gliDownModeModel;

    int     m_lastKey;
    int64_t m_lastTime;
    QMutex  m_mutex;

    friend class InstrumentTrack;
    friend class InstrumentFunctionGlissandoView;
};

class InstrumentFunctionNoteSustaining : public InstrumentFunction
{
    Q_OBJECT

  public:
    InstrumentFunctionNoteSustaining(Model* _parent);
    virtual ~InstrumentFunctionNoteSustaining();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "notesustaining";
    }

    virtual InstrumentFunctionView* createView();

  public slots:
    void reset();

  private:
    int     m_lastKey;
    int64_t m_lastTime;
    QMutex  m_mutex;

    // volatile NotePlayHandle* m_lastNPH;
    /*
    RealModel m_volumeRangeModel;
    RealModel m_panRangeModel;
    RealModel m_tuneRangeModel;
    RealModel m_offsetRangeModel;
    RealModel m_shortenRangeModel;

    RealModel m_volumeStepModel;
    RealModel m_panStepModel;
    RealModel m_tuneStepModel;
    RealModel m_offsetStepModel;
    RealModel m_shortenStepModel;
    */

    friend class InstrumentFunctionNoteSustainingView;
};

class InstrumentFunctionNotePlaying : public InstrumentFunction
{
    Q_OBJECT

  public:
    InstrumentFunctionNotePlaying(Model* _parent);
    virtual ~InstrumentFunctionNotePlaying();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);

    INLINE virtual QString nodeName() const
    {
        return "noteplaying";
    }

    virtual InstrumentFunctionView* createView();

  public slots:
    void reset();
    void onGateChanged();

  private:
    RealModel m_gateModel;
    RealModel m_keyModel;
    RealModel m_volModel;
    RealModel m_panModel;

    int               m_currentGeneration;
    PlayHandlePointer m_currentPH;
    NotePlayHandle*   m_currentNPH;
    // QMutex  m_mutex;

    friend class InstrumentFunctionNotePlayingView;
};

#endif
