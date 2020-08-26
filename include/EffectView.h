/*
 * EffectView.h - view-component for an effect
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
 * Copyright (c) 2006-2007 Danny McRae <khjklujn/at/users.sourceforge.net>
 * Copyright (c) 2007-2010 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef EFFECT_VIEW_H
#define EFFECT_VIEW_H

//#include "AutomatableModel.h"
#include "Effect.h"
#include "PluginView.h"

class QMenu;
class QGroupBox;
class QLabel;
class QPushButton;
class QMdiSubWindow;

class EffectControlDialog;
class Knob;
class LedCheckBox;
class TempoSyncKnob;

class EffectView : public PluginView
{
    Q_OBJECT

  public:
    EffectView(Effect* _model, QWidget* _parent);
    virtual ~EffectView();

    INLINE Effect* model()
    {
        return castModel<Effect>();
    }

    INLINE const Effect* model() const
    {
        return castModel<Effect>();
    }

  public slots:
    // virtual bool close();
    // virtual void update();
    // virtual void remove() final;
    // virtual void mute() final;
    // virtual void clear() final;
    virtual void cut() final;
    virtual void copy() final;
    virtual void paste() final;
    virtual void changeName() final;
    virtual void resetName();
    virtual void changeColor() final;
    virtual void resetColor() final;

    void displayHelp();
    void showContextMenu();

  signals:
    void moveUp(EffectView* _plugin);
    void moveDown(EffectView* _plugin);
    void moveTop(EffectView* _plugin);
    void moveBottom(EffectView* _plugin);
    void removeEffect(EffectView* _plugin);

  protected:
    virtual QMenu* buildContextMenu();
    virtual void   addRemoveMuteClearMenu(QMenu* _cm,
                                          bool   _remove,
                                          bool   _mute,
                                          bool   _clear) final;
    virtual void   addCutCopyPasteMenu(QMenu* _cm,
                                       bool   _cut,
                                       bool   _copy,
                                       bool   _paste) final;
    virtual void   addNameMenu(QMenu* _cm, bool _enabled) final;
    virtual void   addColorMenu(QMenu* _cm, bool _enabled) final;
    virtual void   contextMenuEvent(QContextMenuEvent* _me);

    virtual void mousePressEvent(QMouseEvent* _me);
    virtual void mouseReleaseEvent(QMouseEvent* _me);
    virtual void paintEvent(QPaintEvent* _pe);
    virtual void modelChanged();

  private:
    QPixmap m_bg;

    LedCheckBox*   m_runningLCB;
    LedCheckBox*   m_enabledLCB;
    LedCheckBox*   m_clippingLCB;
    Knob*          m_wetDryKNB;
    TempoSyncKnob* m_autoQuitKNB;
    Knob*          m_gateInKNB;
    Knob*          m_balanceKNB;

    QMdiSubWindow*       m_subWindow;
    EffectControlDialog* m_controlView;

  private slots:
    void openControls();
    void closeControls();
    void cloneEffect();
    void toggleEffect();
    void clearEffect();
    void removeEffect();
    void moveUp();
    void moveDown();
    void moveTop();
    void moveBottom();
};

#endif
