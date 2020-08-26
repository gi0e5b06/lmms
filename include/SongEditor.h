/*
 * SongEditor.h - declaration of class SongEditor, a window where you can
 *                 setup your songs
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
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

#ifndef SONG_EDITOR_H
#define SONG_EDITOR_H

//#include "ActionUpdatable.h"
#include "ActionGroup.h"
#include "Editor.h"
#include "TrackContainerView.h"

class QLabel;
class QScrollBar;

class AutomatableSlider;
class ComboBox;
class ComboBoxModel;
class Knob;
class VolumeKnob;
class LcdSpinBox;
class MeterDialog;
class Song;
class TextFloat;
class TimeLineWidget;

/*
class positionLine : public QWidget
{
public:
        positionLine( QWidget * parent );

private:
        virtual void paintEvent( QPaintEvent * pe );

};
*/

class SongEditor :
      public TrackContainerView,
      public virtual Editor,
      public virtual ActionUpdatable
{
    Q_OBJECT
  public:
    SongEditor(Song* song);
    virtual ~SongEditor();

    tick_t quantization() const;

    void joinTiles();
    void splitTiles();

    void saveSettings(QDomDocument& doc, QDomElement& element);
    void loadSettings(const QDomElement& element);

    virtual void updateActions(const bool            _active,
                               QHash<QString, bool>& _table) const;  // = 0;
    virtual void actionTriggered(QString _name);

    ComboBoxModel* zoomingXModel() const;
    ComboBoxModel* zoomingYModel() const;
    ComboBoxModel* quantizeModel() const;

    const TimeLineWidget* timeLineWidget()
    {
        return m_timeLine;
    }

    bool allowRubberband() const override;

  public slots:
    void scrolled(int _newPos);

    void updatePosition(const MidiTime& _time);
    void updatePositionLine();

    void deleteSelection();
    void cutSelection();
    void copySelection();
    void pasteSelection();

    void selectAll();
    void unselectAll();

  protected:
    void closeEvent(QCloseEvent* ce) override;
    void focusOutEvent(QFocusEvent* fe) override;
    void keyPressEvent(QKeyEvent* ke) override;
    // void keyReleaseEvent(QKeyEvent* ke) override;
    // void leaveEvent(QEvent* le) override;
    // void mouseMoveEvent(QMouseEvent* me) override;
    // void paintEvent(QPaintEvent* pe) override;
    void resizeEvent(QResizeEvent* re) override;
    void wheelEvent(QWheelEvent* we) override;

  private slots:
    void setHighQuality(bool _on);

    void setMasterVolume(real_t _newVal);
    // void showMasterVolumeFloat();
    // void updateMasterVolumeFloat( int _newVal );
    // void hideMasterVolumeFloat();

    void setMasterPitch(real_t _newVal);
    // void showMasterPitchFloat();
    // void updateMasterPitchFloat( int _newVal );
    // void hideMasterPitchFloat();

    void setMasterPanning(real_t _newVal);
    // void showMasterPanFloat();
    // void updateMasterPanFloat( int _newVal );
    // void hideMasterPanFloat();

    void updateScrollBar(int len);

    void zoomingXChanged();
    void zoomingYChanged();

  private:
    void splitTiles(tick_t _splitPos, TileViews _tileViews);
    void selectAllTcos(bool select);

    static int headerWidth();

    Song* m_song;

    QScrollBar* m_leftRightScroll;

    LcdSpinBox* m_tempoSpinBox;

    TimeLineWidget* m_timeLine;

    MeterDialog* m_timeSigDisplay;

    VolumeKnob* m_masterVolumeKNB;
    Knob*       m_masterPitchKNB;
    Knob*       m_masterPanningKNB;

    TextFloat* m_masterVolumeTFT;
    TextFloat* m_masterPitchTFT;
    TextFloat* m_masterPanningTFT;

    // positionLine * m_positionLine;

    ComboBoxModel* m_zoomingXModel;
    ComboBoxModel* m_zoomingYModel;
    ComboBoxModel* m_quantizeModel;

    bool m_scrollBack;
    bool m_smoothScroll;

    // EditMode m_editMode;

    friend class SongWindow;
};

class SongWindow : public EditorWindow, public virtual ActionUpdatable
{
    Q_OBJECT

  public:
    SongWindow(Song* song);

    virtual void updateActions(const bool            _active,
                               QHash<QString, bool>& _table) const;  // = 0;
    virtual void actionTriggered(QString _name);

    QSize sizeHint() const;

    SongEditor* m_editor;

  public slots:
    void play();
    void record();
    void recordAccompany();
    void stop();
    // void stopRecording();

    void adjustUiAfterProjectLoad();  // ???
    void onLostFocus();

    virtual void setEditMode(int _mode) final
    {
        m_editor->setEditMode((Editor::EditMode)_mode);
    }

  protected:
    void focusInEvent(QFocusEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* ke) override;
    void keyReleaseEvent(QKeyEvent* ke) override;

  signals:
    void playTriggered();
    void resized();

  private:
    QAction* m_addBBTrackAction;
    QAction* m_addSampleTrackAction;
    QAction* m_addAutomationTrackAction;

    /*
    ActionGroup* m_editModeGroup;
    QAction*     m_drawModeAction;
    QAction*     m_eraseModeAction;
    QAction*     m_selectModeAction;
    // QAction*     m_glueModeAction; // Unite
    // QAction*     m_knifeModeAction; // Divide
    QAction* m_previousEditAction;
    */

    ComboBox* m_zoomingXComboBox;
    ComboBox* m_zoomingYComboBox;
    ComboBox* m_quantizeComboBox;
};

#endif
