/*
 * SampleTrack.h - Track which provides arrangement of samples
 *
 * Copyright (c) 2017-2019 gi0e5b06 (on github.com)
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

#ifndef SAMPLE_TRACK_H_
#define SAMPLE_TRACK_H_

#include "AudioPort.h"
#include "Track.h"

//#include <QDialog>

class SampleTrack;
class LedCheckBox;
class SampleBuffer;
class EffectRackView;
class FadeButton;
class Knob;
class LcdSpinBox;
class FxLineLcdSpinBox;
class TabWidget;
class TrackLabelButton;
class SampleTrackWindow;

class QLineEdit;
class QLabel;

class SampleTCO : public Tile
{
    Q_OBJECT
    mapPropertyFromModel(bool, isRecord, setRecord, m_recordModel);

  public:
    SampleTCO(SampleTrack* _track);
    SampleTCO(const SampleTCO& _other);
    virtual ~SampleTCO();

    virtual bool    isEmpty() const;
    virtual QString defaultName() const;
    virtual tick_t  unitLength() const;

    // virtual void changeLength( const MidiTime & _length );

    virtual void resizeLeft(const MidiTime& pos, const MidiTime& len);
    // virtual void resizeRight(const MidiTime& pos, const MidiTime& len);

    // settings-management
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "sampletco";
    }

    const QString& sampleFile() const;

    SampleBuffer* sampleBuffer()
    {
        return m_sampleBuffer;
    }

    MidiTime sampleLength() const;

    tick_t            initialPlayTick();
    void              setInitialPlayTick(tick_t _t);

    inline SampleTrack* sampleTrack() const
    {
        return m_sampleTrack;
    }

    virtual TileView* createView(TrackView* _tv);

    bool isPlaying() const;
    void setIsPlaying(bool isPlaying);

  public slots:
    void setSampleBuffer(SampleBuffer* sb);
    void setSampleFile(const QString& _sf);
    void updateLength();
    void toggleRecord();
    void playbackPositionChanged();
    void updateTrackTcos();

  protected:
    virtual void split(tick_t _ticks);
    virtual void doConnections();

  private:
    SampleTrack* m_sampleTrack;

    // number of ticts skipped (after the start frame)
    // most of the time, 0
    tick_t m_initialPlayTick;

    SampleBuffer* m_sampleBuffer;
    BoolModel     m_recordModel;
    bool          m_isPlaying;

    friend class SampleTCOView;

  signals:
    void sampleChanged();
};

class SampleTCOView : public TileView
{
    Q_OBJECT

  public:
    SampleTCOView(SampleTCO* _tco, TrackView* _tv);
    virtual ~SampleTCOView();

  public slots:
    void loadSample();
    void reloadSample();
    void updateSample();
    void openInAudacity();
    void createRmsAutomation();

  protected:
    virtual QMenu* buildContextMenu();

    // virtual void contextMenuEvent( QContextMenuEvent * _cme );
    virtual void mousePressEvent(QMouseEvent* _me);
    virtual void mouseReleaseEvent(QMouseEvent* _me);
    virtual void dragEnterEvent(QDragEnterEvent* _dee);
    virtual void dropEvent(QDropEvent* _de);
    virtual void mouseDoubleClickEvent(QMouseEvent*);
    virtual void paintEvent(QPaintEvent*);

  private:
    SampleTCO* m_tco;
    QPixmap    m_paintPixmap;
};

class SampleTrack : public Track
{
    Q_OBJECT

  public:
    SampleTrack(TrackContainer* tc);
    virtual ~SampleTrack();

    virtual QString defaultName() const;

    virtual bool play(const MidiTime& _start,
                      const fpp_t     _frames,
                      const f_cnt_t   _frame_base,
                      int             _tco_num = -1);

    virtual TrackView* createView(TrackContainerView* tcv);
    virtual Tile*      createTCO(const MidiTime& _pos);

    virtual void saveTrackSpecificSettings(QDomDocument& _doc,
                                           QDomElement&  _parent);
    virtual void loadTrackSpecificSettings(const QDomElement& _this);

    AudioPortPointer& audioPort()
    {
        return m_audioPort;
    }

    const AudioPortPointer& audioPort() const
    {
        return m_audioPort;
    }

    FloatModel* volumeModel()
    {
        return &m_volumeModel;
    }

    FloatModel* panningModel()
    {
        return &m_panningModel;
    }

    IntModel* effectChannelModel()
    {
        return &m_effectChannelModel;
    }

  public slots:
    void updateTcos();
    void setPlayingTcos(bool isPlaying);

  protected:
    virtual QString nodeName() const
    {
        return "sampletrack";
    }

  protected slots:
    void updateEffectChannel();
    void updatePanning();
    void updateVolume();

  private:
    FloatModel       m_volumeModel;
    FloatModel       m_panningModel;
    BoolModel        m_useMasterPitchModel;  // TODO?
    IntModel         m_effectChannelModel;
    AudioPortPointer m_audioPort;
    // FloatModel m_pitchModel;         //TODO?
    // IntModel m_pitchRangeModel;      //TODO?

    friend class SampleTrackView;
    friend class SampleTrackWindow;
};

class SampleTrackView : public TrackView
{
    Q_OBJECT

  public:
    SampleTrackView(SampleTrack* _st, TrackContainerView* _tcv);
    virtual ~SampleTrackView();

    SampleTrackWindow* sampleTrackWindow();

    SampleTrack* model()
    {
        return castModel<SampleTrack>();
    }

    const SampleTrack* model() const
    {
        return castModel<SampleTrack>();
    }

    void freeSampleTrackWindow();

    // Create a menu for assigning/creating channels for this track
    QMenu* createFxMenu(QString title, QString newFxLabel);

    QMenu* createAudioInputMenu();
    QMenu* createAudioOutputMenu();
    // QMenu* createMidiInputMenu();
    // QMenu* createMidiOutputMenu();

  public slots:
    // void textChanged( const QString & _new_name );
    // void toggleVisibility(bool _on);
    void updateName();
    // void updateSampleView();

    virtual void addSpecificMenu(QMenu* _cm, bool _enabled);

  protected:
    virtual void resizeEvent(QResizeEvent* _re);
    virtual void dragEnterEvent(QDragEnterEvent* _dee);
    virtual void dropEvent(QDropEvent* _de);

    virtual QString nodeName() const
    {
        qWarning("SampleTrackView::nodeName() useless?");
        return "SampleTrackView";
    }

  private slots:
    void toggleSampleWindow(bool _on);
    void activityIndicatorPressed();
    void activityIndicatorReleased();

    // void midiInSelected();
    // void midiOutSelected();
    // void midiConfigChanged();
    void modelChanged();
    void muteChanged();

    void assignFxLine(int _channelIndex);
    void createFxLine();

  private:
    SampleTrackWindow* m_window;
    // EffectRackView* m_effectRack;
    // QWidget * m_effWindow;

    // widgets in track-settings-widget
    TrackLabelButton* m_tlb;
    Knob*             m_volumeKnob;
    Knob*             m_panningKnob;
    FadeButton*       m_activityIndicator;

    QPoint m_lastPos;

    friend class SampleTrackWindow;
};

class SampleTrackWindow :
      public QWidget,
      public ModelView,
      public SerializingObjectHook
{
    Q_OBJECT

  public:
    SampleTrackWindow(SampleTrackView* _stv);
    virtual ~SampleTrackWindow();

    // parent for all internal tab-widgets
    TabWidget* tabWidgetParent()
    {
        return m_tabWidget;
    }

    SampleTrack* model()
    {
        return castModel<SampleTrack>();
    }

    const SampleTrack* model() const
    {
        return castModel<SampleTrack>();
    }

    void setSampleTrackView(SampleTrackView* _stv);

    SampleTrackView* sampleTrackView()
    {
        return m_stv;
    }

    // PianoView * pianoView()
    //{
    //	return m_pianoView;
    //}

    static void dragEnterEventGeneric(QDragEnterEvent* _dee);

    virtual void dragEnterEvent(QDragEnterEvent* _dee);
    virtual void dropEvent(QDropEvent* _de);

  public slots:
    void textChanged(const QString& _new_name);
    void toggleVisibility(bool _on);
    void updateName();
    void updateSampleView();

  protected:
    // capture close-events for toggling instrument-track-button
    virtual void closeEvent(QCloseEvent* _ce);
    virtual void focusInEvent(QFocusEvent* _fe);

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _this);
    virtual void loadSettings(const QDomElement& _this);

  protected slots:
    void saveSettingsBtnClicked();
    // void viewNextSample();
    // void viewPrevSample();

  private:
    virtual void modelChanged();
    // void viewSampleInDirection(int d);

    SampleTrack*     m_track;
    SampleTrackView* m_stv;

    // widgets on the top of an instrument-track-window
    QLineEdit* m_nameLineEdit;
    // LeftRightNav * m_leftRightNav;
    Knob* m_volumeKnob;
    Knob* m_panningKnob;
    Knob* m_bendingKnob;
    // QLabel*       m_bendingLabel;
    LcdSpinBox* m_bendingRangeSpinBox;
    // QLabel*       m_bendingRangeLabel;
    FxLineLcdSpinBox* m_effectChannelNumber;

    // tab-widget with all children
    TabWidget* m_tabWidget;
    QWidget*   m_sampleView;
    // PluginView * m_instrumentView;
    // SampleSoundShapingView * m_ssView;
    // SampleFunctionNoteHumanizingView* m_noteHumanizingView;
    // SampleFunctionNoteStackingView* m_noteStackingView;
    // SampleFunctionArpeggioView* m_arpeggioView;
    // SampleFunctionNoteDuplicatesRemovingView*
    // m_noteDuplicatesRemovingView;
    // SampleMidiIOView * m_midiView;
    EffectRackView* m_effectView;
    // SampleMiscView *m_miscView;

    // test-piano at the bottom of every instrument-settings-window
    // PianoView * m_pianoView;

    friend class SampleView;
};

#endif
