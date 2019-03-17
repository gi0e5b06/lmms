/*
 * VstEffectControlDialog.h - dialog for displaying GUI of VST-effect-plugin
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2006-2010 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef VST_EFFECT_CONTROL_DIALOG_H
#define VST_EFFECT_CONTROL_DIALOG_H

#include "EffectControlDialog.h"
#include "VstPlugin.h"

#include <QLabel>
#include <QObject>
#include <QPainter>

class VstEffectControls;
class MidiProcessorView;
class PixmapButton;
class QPixmap;
class QPushButton;
class PixmapButton;

class VstEffectControlDialog : public EffectControlDialog
{
    Q_OBJECT

  public:
    VstEffectControlDialog(VstEffectControls* _controls);
    virtual ~VstEffectControlDialog();

  protected:
    // virtual void paintEvent(QPaintEvent* _pe);

  protected slots:
    virtual void toggleVstWidget();

  private:
    // QWidget * m_pluginWidget;

    PixmapButton* m_openPresetButton;
    PixmapButton* m_rolLPresetButton;
    PixmapButton* m_rolRPresetButton;
    PixmapButton* m_managePluginButton;
    PixmapButton* m_savePresetButton;

    MidiProcessorView* m_midiInProc;
    // MidiProcessorView* m_midiOutProc;

    VstPlugin* m_plugin;

    // QLabel * tbLabel;
};

#endif
