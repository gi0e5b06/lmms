/*
 * InstrumentTrack.h - Track which provides arrangement of notes
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

#ifndef INSTRUMENT_TRACK_H
#define INSTRUMENT_TRACK_H

#include "AudioPort.h"
#include "BasicFilters.h"
//#include "GroupBox.h"
#include "InstrumentFunction.h"
#include "InstrumentSoundShaping.h"
#include "MidiEventProcessor.h"
#include "MidiPort.h"
#include "Mutex.h"
#include "NotePlayHandle.h"
#include "PeripheralView.h"
#include "Piano.h"
#include "Track.h"

template <class T>
class QQueue;

class InstrumentFunctionNoteFilteringView;
class InstrumentFunctionNoteHumanizingView;
class InstrumentFunctionNoteStackingView;
class InstrumentFunctionArpeggioView;
class InstrumentFunctionNoteDuplicatesRemovingView;
class EffectRackView;
class InstrumentSoundShapingView;
class FadeButton;
class Instrument;
class InstrumentTrackWindow;
class InstrumentMidiIOView;
class InstrumentMiscView;
class Knob;
class FxLineLcdSpinBox;
class LcdSpinBox;
class LeftRightNav;
class midiPortMenu;
class DataFile;
class PluginView;
class TabWidget;
class TrackLabelButton;
class LedCheckBox;

class QLabel;
class QLineEdit;

class EXPORT InstrumentTrack : public Track, public MidiEventProcessor
{
    Q_OBJECT
    MM_OPERATORS

    mapPropertyFromModel(int, getVolume, setVolume, m_volumeModel);

  public:
    InstrumentTrack(TrackContainer* tc);
    virtual ~InstrumentTrack();

    // used by instrument
    void processAudioBuffer(sampleFrame*    _buf,
                            const fpp_t     _frames,
                            NotePlayHandle* _nph);

    MidiEvent applyMasterKey(const MidiEvent& event);

    virtual void removeMidiNote(const int _key, const f_cnt_t _offset) final;
    virtual void addMidiNote(const int      _key,
                             const f_cnt_t  _offset,
                             const volume_t _volume,
                             const int      _channel) final;
    virtual void processInEvent(const MidiEvent& event,
                                const MidiTime&  time   = MidiTime(),
                                f_cnt_t          offset = 0);
    virtual void processOutEvent(const MidiEvent& event,
                                 const MidiTime&  time   = MidiTime(),
                                 f_cnt_t          offset = 0);

    virtual bool isSustainPedalPressed() const
    {
        return m_sustainPedalPressed;
    }

    virtual QString defaultName() const;

    f_cnt_t beatLen(NotePlayHandle* _n) const;

    // for capturing note-play-events -> need that for arpeggio,
    // filter and so on
    void playNote(NotePlayHandle* _n, sampleFrame* _working_buffer);

    QString instrumentName() const;

    const Instrument* instrument() const
    {
        return m_instrument;
    }

    Instrument* instrument()
    {
        return m_instrument;
    }

    void deleteNotePluginData(NotePlayHandle* _n);

    // name-stuff
    virtual void setName(const QString& _new_name);

    // translate given key of a note-event to absolute key (i.e.
    // add global master-pitch and base-note of this instrument track)
    int masterKey(int _midi_key) const;

    // translate volume to midi-volume [0,127]@CC7
    int midiVolume() const;

    // translate panning to midi-panning [0,127]@CC10
    int midiPanning() const;

    // translate pitch to midi-pitch [0,16383]
    int midiBending() const;

    /*! \brief Returns current range for pitch bend in semitones */
    int midiBendingRange() const
    {
        return m_bendingRangeModel.value();
    }

    // play everything in given frame-range - creates note-play-handles
    virtual bool play(const MidiTime& _start,
                      const fpp_t     _frames,
                      const f_cnt_t   _frame_base,
                      int             _tco_num = -1);

    // create new view for me
    virtual TrackView* createView(TrackContainerView* tcv);

    // create new track-content-object = pattern
    virtual Tile* createTCO(const MidiTime& _pos);

    // called by track
    virtual void saveTrackSpecificSettings(QDomDocument& _doc,
                                           QDomElement&  _parent);
    virtual void loadTrackSpecificSettings(const QDomElement& _this);

    using Track::setJournalling;

    // load instrument whose name matches given one
    Instrument* loadInstrument(const QString& _instrument_name);

    const Scale* scale() const;
    void         setScale(const Scale* _scale);

    const AudioPortPointer audioPort() const
    {
        return m_audioPort;
    }

    AudioPortPointer audioPort()
    {
        return m_audioPort;
    }

    const MidiPort* midiPort() const
    {
        return &m_midiPort;
    }

    MidiPort* midiPort()
    {
        return &m_midiPort;
    }

    const IntModel* baseNoteModel() const
    {
        return &m_baseNoteModel;
    }

    IntModel* baseNoteModel()
    {
        return &m_baseNoteModel;
    }

    int baseNote() const;

    Piano* pianoModel()
    {
        return &m_piano;
    }

    /*
    bool isArpeggioEnabled() const
    {
            return m_arpeggio.m_enabledModel.value();
    }
    */

    // simple helper for removing midiport-XML-node when loading presets
    static void removeMidiPortNode(DataFile& dataFile);

    BoolModel* volumeEnabledModel()
    {
        return &m_volumeEnabledModel;
    }

    FloatModel* volumeModel()
    {
        return &m_volumeModel;
    }

    FloatModel* noteVolumeModel()
    {
        return &m_noteVolumeModel;
    }

    BoolModel* panningEnabledModel()
    {
        return &m_panningEnabledModel;
    }

    FloatModel* panningModel()
    {
        return &m_panningModel;
    }

    FloatModel* notePanningModel()
    {
        return &m_notePanningModel;
    }

    BoolModel* bendingEnabledModel()
    {
        return &m_bendingEnabledModel;
    }

    FloatModel* bendingModel()
    {
        return &m_bendingModel;
    }

    FloatModel* noteBendingModel()
    {
        return &m_noteBendingModel;
    }

    IntModel* bendingRangeModel()
    {
        return &m_bendingRangeModel;
    }

    IntModel* effectChannelModel()
    {
        return &m_effectChannelModel;
    }

    FloatModel* midiCCModel(const uint8_t _cc)
    {
        return m_midiCCModel[_cc];
    }

    // void setPreviewMode(const bool);

    virtual void cleanFrozenBuffer();
    virtual void readFrozenBuffer();
    virtual void writeFrozenBuffer();

    void setEnvOffset(f_cnt_t _o)
    {
        m_envOffset = _o;
    }

    void setEnvTotalFramesPlayed(f_cnt_t _tfp)
    {
        m_envTotalFramesPlayed = _tfp;
    }

    void setEnvReleaseBegin(f_cnt_t _rb)
    {
        m_envReleaseBegin = _rb;
    }

    void setEnvLegato(bool _legato)
    {
        m_envLegato = _legato;
    }

    void setEnvMarcato(bool _marcato)
    {
        m_envMarcato = _marcato;
    }

    void setEnvStaccato(bool _staccato)
    {
        m_envStaccato = _staccato;
    }

    void setEnvVolume(volume_t _v)
    {
        m_envVolume = _v;
    }

    void setEnvPanning(panning_t _p)
    {
        m_envPanning = _p;
    }

    bool isKeyPressed(int _key)
    {
        return (_key >= 0 && _key < NumMidiKeys
                && (m_notes[_key] != nullptr
                    || m_runningMidiNotes[_key] > 0));
    }

  signals:
    void instrumentChanged();
    void instrumentFunctionAdded(InstrumentFunction* _f);
    void midiNoteOn(const Note&);
    void midiNoteOff(const Note&);
    // void nameChanged();
    void newNote();

  public slots:
    virtual void toggleFrozen();
    // silence all running notes played by this track
    void silenceAllNotes(/*bool removeIPH = false*/);

  protected:
    virtual QString nodeName() const
    {
        return "instrumenttrack";
    }

  protected slots:
    void updateBaseNote();
    void updateVolume();
    void updatePanning();
    void updateMidiCC();
    void updateMidiCC(uint8_t _cc);
    void updateBending();
    void updateBendingRange();
    void updateEffectChannel();

  private:
    MidiPort m_midiPort;

    NotePlayHandle* m_notes[NumMidiKeys];
    NotePlayHandles m_sustainedNotes;

    int   m_runningMidiNotes[NumMidiKeys];
    Mutex m_midiNotesMutex;

    bool m_sustainPedalPressed;

    bool m_silentBuffersProcessed;

    // bool m_previewMode;

    const Scale* m_scale;
    IntModel     m_baseNoteModel;

    // NotePlayHandleList m_processHandles;
    NotePlayHandles m_processHandles;

    BoolModel  m_volumeEnabledModel;
    FloatModel m_volumeModel;
    FloatModel m_noteVolumeModel;  // for midi instr.

    BoolModel  m_panningEnabledModel;
    FloatModel m_panningModel;
    FloatModel m_notePanningModel;  // for midi instr.

    BoolModel  m_bendingEnabledModel;
    FloatModel m_bendingModel;
    FloatModel m_noteBendingModel;  // for midi instr.
    IntModel   m_bendingRangeModel;

    FloatModel* m_midiCCModel[MidiControllerCount];

    BoolModel              m_useMasterPitchModel;
    IntModel               m_effectChannelModel;
    AudioPortPointer       m_audioPort;
    Instrument*            m_instrument;
    InstrumentSoundShaping m_soundShaping;

    QVector<InstrumentFunction*> m_noteFunctions;
    /*
    InstrumentFunctionNoteFiltering m_noteFiltering;
    InstrumentFunctionNoteHumanizing m_noteHumanizing;
    InstrumentFunctionNoteStacking m_noteStacking;
    InstrumentFunctionArpeggio m_arpeggio;
    InstrumentFunctionNoteDuplicatesRemoving m_noteDuplicatesRemoving;
    */

    Piano m_piano;

    BasicFilters<>* m_envFilter1;
    BasicFilters<>* m_envFilter2;
    f_cnt_t         m_envOffset;
    f_cnt_t         m_envTotalFramesPlayed;
    f_cnt_t         m_envReleaseBegin;
    bool            m_envLegato;
    bool            m_envMarcato;
    bool            m_envStaccato;
    volume_t        m_envVolume;
    panning_t       m_envPanning;
    // friend class InstrumentSoundShaping;

    friend class InstrumentTrackView;
    friend class InstrumentTrackWindow;
    friend class NotePlayHandle;
    friend class InstrumentMiscView;
};

class InstrumentTrackView : public TrackView
{
    Q_OBJECT

  public:
    InstrumentTrackView(InstrumentTrack* _it, TrackContainerView* _tcv);
    virtual ~InstrumentTrackView();

    InstrumentTrackWindow* instrumentTrackWindow();

    InstrumentTrack* model()
    {
        return castModel<InstrumentTrack>();
    }

    const InstrumentTrack* model() const
    {
        return castModel<InstrumentTrack>();
    }

    static InstrumentTrackWindow* topLevelInstrumentTrackWindow();
    // static void cleanupWindowCache();

    QMenu* midiMenu()
    {
        return m_midiMenu;
    }

    // Create a menu for assigning/creating channels for this track
    QMenu* createFxMenu(QString title, QString newFxLabel);

    QMenu* createAudioInputMenu();
    QMenu* createAudioOutputMenu();
    QMenu* createMidiInputMenu();
    QMenu* createMidiOutputMenu();

    virtual void addSpecificMenu(QMenu* _cm, bool _enabled);

  protected:
    virtual void paintEvent(QPaintEvent* pe);
    virtual void resizeEvent(QResizeEvent* re);
    virtual void dragEnterEvent(QDragEnterEvent* _dee);
    virtual void dropEvent(QDropEvent* _de);

  private slots:
    void toggleInstrumentWindow(bool _on);
    void activityIndicatorPressed();
    void activityIndicatorReleased();

    void midiInSelected();
    void midiOutSelected();
    void midiConfigChanged();
    void muteChanged();

    void assignFxLine(int channelIndex);
    void createFxLine();

  private:
    void freeInstrumentTrackWindow();

    InstrumentTrackWindow* m_window;

    // static QQueue<InstrumentTrackWindow*> s_windowCache;

    // widgets in track-settings-widget
    TrackLabelButton* m_tlb;
    Knob*             m_volumeKnob;
    Knob*             m_panningKnob;
    FadeButton*       m_activityIndicator;

    QMenu*   m_midiMenu;
    QAction* m_midiInputAction;
    QAction* m_midiOutputAction;

    QPoint m_lastPos;

    friend class InstrumentTrackWindow;
};

class InstrumentTrackWindow :
      public QWidget,
      public ModelView,
      public SerializingObjectHook
{
    Q_OBJECT

  public:
    InstrumentTrackWindow(InstrumentTrackView* _tv);
    virtual ~InstrumentTrackWindow();

    // parent for all internal tab-widgets
    TabWidget* tabWidgetParent()
    {
        return m_tabWidget;
    }

    InstrumentTrack* model()
    {
        return castModel<InstrumentTrack>();
    }

    const InstrumentTrack* model() const
    {
        return castModel<InstrumentTrack>();
    }

    void setInstrumentTrackView(InstrumentTrackView* _tv);

    InstrumentTrackView* instrumentTrackView()
    {
        return m_itv;
    }

    PeripheralView* peripheralView()
    {
        return m_peripheralView;
    }

    static void dragEnterEventGeneric(QDragEnterEvent* _dee);

    virtual void dragEnterEvent(QDragEnterEvent* _dee);
    virtual void dropEvent(QDropEvent* _de);

  public slots:
    void textChanged(const QString& _new_name);
    void toggleVisibility(bool _on);
    void updateName();
    void updateInstrumentView();
    void switchToLaunchpad();
    void switchToPads();
    void switchToPiano();

  protected:
    // capture close-events for toggling instrument-track-button
    virtual void closeEvent(QCloseEvent* _ce);
    virtual void focusInEvent(QFocusEvent* _fe);

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _this);
    virtual void loadSettings(const QDomElement& _this);

  protected slots:
    void saveSettingsBtnClicked();
    /*
    void viewNextInstrument();
    void viewPrevInstrument();
    */

  private:
    virtual void modelChanged();
    // void         viewInstrumentInDirection(int d);

    InstrumentTrack*     m_track;
    InstrumentTrackView* m_itv;

    // widgets on the top of an instrument-track-window
    QLineEdit* m_nameLineEdit;
    // LeftRightNav* m_leftRightNav;
    Knob* m_volumeKnob;
    Knob* m_panningKnob;
    Knob* m_bendingKnob;
    // QLabel*       m_bendingLabel;
    LcdSpinBox* m_bendingRangeSpinBox;
    // QLabel*       m_bendingRangeLabel;
    FxLineLcdSpinBox* m_effectChannelNumber;

    // tab-widget with all children
    TabWidget*                  m_tabWidget;
    PluginView*                 m_instrumentView;
    InstrumentSoundShapingView* m_ssView;

    QVector<InstrumentFunctionView*> m_noteFunctionViews;
    /*
    InstrumentFunctionNoteFilteringView* m_noteFilteringView;
    InstrumentFunctionNoteHumanizingView* m_noteHumanizingView;
    InstrumentFunctionNoteStackingView* m_noteStackingView;
    InstrumentFunctionArpeggioView* m_arpeggioView;
    InstrumentFunctionNoteDuplicatesRemovingView*
    m_noteDuplicatesRemovingView;
    */

    InstrumentMidiIOView* m_midiView;
    EffectRackView*       m_effectView;
    InstrumentMiscView*   m_miscView;

    // test-piano at the bottom of every instrument-settings-window
    PeripheralView* m_peripheralView;

    friend class InstrumentView;
};

#endif
