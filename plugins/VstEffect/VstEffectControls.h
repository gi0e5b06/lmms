/*
 * VstEffectControls.h - controls for VST effect plugins
 *
 * Copyright (c) 2008-2011 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef _VST_EFFECT_CONTROLS_H
#define _VST_EFFECT_CONTROLS_H

#include "EffectControls.h"
#include "VstEffectControlDialog.h"

#include <QMenu>
//#include <QPushButton>
#include "Knob.h"
#include "PixmapButton.h"
#include "embed.h"

#include <QLayout>
#include <QMdiSubWindow>
#include <QObject>
#include <QPainter>
#include <QScrollArea>

class VstEffect;

class VstEffectControls : public EffectControls
{
    Q_OBJECT

  public:
    VstEffectControls(VstEffect* _eff);
    virtual ~VstEffectControls();

    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "vsteffectcontrols";
    }

    virtual int controlCount();

    virtual EffectControlDialog* createView()
    {
        return new VstEffectControlDialog(this);
    }

  protected slots:
    void updateMenu(void);
    void managePlugin(void);
    void openPreset(void);
    void savePreset(void);
    void rollPreset(void);
    void rolrPreset(void);
    void selPreset(void);
    void setParameter(void);

  protected:
    // virtual void paintEvent( QPaintEvent * _pe );

  private:
    VstEffect* m_effect;

    PixmapButton* m_selPresetButton;  // QPushButton
    QMenu*        menu;

    QMdiSubWindow* m_subWindow;
    // QScrollArea * m_scrollArea;

    FloatModel** knobFModel;
    Knob**       vstKnobs;
    int          paramCount;

    QObject* ctrHandle;

    int lastPosInMenu;
    //	QLabel * m_presetLabel;

    friend class VstEffectControlDialog;
    friend class manageVSTEffectView;
};

class manageVSTEffectView : public QObject
{
    Q_OBJECT
  public:
    manageVSTEffectView(VstEffect* _effect, VstEffectControls* _controls);
    virtual ~manageVSTEffectView();

  protected slots:
    void syncPlugin();
    void displayAutomatedOnly();
    void setParameter();
    void closeWindow();

  private:
    //	static QPixmap * s_artwork;

    VstEffect*         m_effect;
    VstEffectControls* m_controls;

    QPushButton* m_syncButton;
    QPushButton* m_displayAutomatedOnly;
    // QPushButton * m_closeButton;
};

#endif
