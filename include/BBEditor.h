/*
 * BBEditor.h - view-component of BB-Editor
 *
 * Copyright (c) 2018-2020 gi0e5b06 (on github.com)
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


#ifndef BB_EDITOR_H
#define BB_EDITOR_H

#include "Editor.h"
#include "TrackContainerView.h"

class BBTrackContainer;
class ComboBox;

class BBEditor;

class BBWindow : public EditorWindow
{
    Q_OBJECT

  public:
    BBWindow(BBTrackContainer* _tc);
    virtual ~BBWindow();

    QSize sizeHint() const;

    const BBEditor* trackContainerView() const
    {
        return m_trackContainerView;
    }

    BBEditor* trackContainerView()
    {
        return m_trackContainerView;
    }

    void removeBBView(int bb);

  public slots:
    void play();
    void stop();

  private:
    BBEditor* m_trackContainerView;
    ComboBox* m_bbComboBox;
};

class BBEditor :
      public TrackContainerView,
      public virtual Editor,
      public virtual ActionUpdatable
{
    Q_OBJECT

  public:
    BBEditor(BBTrackContainer* tc);

    virtual bool isFixed() const  // fixedTCOs()
    {
        return true;
    }

    float pixelsPerTact() const;

    void removeBBView(int bb);

    void saveSettings(QDomDocument& doc, QDomElement& element);
    void loadSettings(const QDomElement& element);

  public slots:
    void cloneBeat(); // same as spawnTrack
    void addInstrumentTrack();
    void addSampleTrack();
    void addAutomationTrack();
    void removeSteps();
    void addSteps();
    void cloneSteps();
    void rotateOneStepLeft();
    void rotateOneStepRight();

  protected slots:
    void dropEvent(QDropEvent* de);
    void updatePosition();

  protected:
    int highestStepResolution();

  private:
    BBTrackContainer* m_bbtc;
    void              makeSteps(bool clone);
};

#endif
