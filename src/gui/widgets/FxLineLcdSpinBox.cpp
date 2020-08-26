/*
 * FxLineLcdSpinBox.cpp -
 *
 * Copyright (c) 2019      gi0e5b06 (on github.com)
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

#include "FxLineLcdSpinBox.h"

#include "CaptionMenu.h"
#include "FxMixerView.h"
#include "GuiApplication.h"
#include "InstrumentTrack.h"
#include "SampleTrack.h"

FxLineLcdSpinBox::FxLineLcdSpinBox(int            _numDigits,
                                   QWidget*       _parent,
                                   const QString& _name) :
      LcdSpinBox(_numDigits, _parent, _name)
{
}

FxLineLcdSpinBox::~FxLineLcdSpinBox()
{
}

void FxLineLcdSpinBox::mouseDoubleClickEvent(QMouseEvent* _me)
{
    gui->fxMixerView()->setCurrentLine(model()->value());
    gui->fxMixerView()->parentWidget()->show();
    gui->fxMixerView()->show();      // show fxMixer window
    gui->fxMixerView()->setFocus();  // set focus to fxMixer window
                                     // engine::getFxMixerView()->raise();
}

void FxLineLcdSpinBox::contextMenuEvent(QContextMenuEvent* _cme)
{
    // for the case, the user clicked right while pressing left mouse-
    // button, the context-menu appears while mouse-cursor is still hidden
    // and it isn't shown again until user does something which causes
    // an QApplication::restoreOverride Cursor()-call...
    mouseReleaseEvent(nullptr);

    QPointer<CaptionMenu> contextMenu
            = new CaptionMenu(model()->displayName(), this);

    if(InstrumentTrackWindow* window = dynamic_cast<InstrumentTrackWindow*>(
               (QWidget*)this->parent()->parent()))
    {
        QMenu* fxMenu = window->instrumentTrackView()->createFxMenu(
                tr("Assign to:"), tr("New FX Channel"));
        contextMenu->addMenu(fxMenu);

        contextMenu->addSeparator();
    }
    else if(SampleTrackWindow* window = dynamic_cast<SampleTrackWindow*>(
                    (QWidget*)this->parent()->parent()))
    {
        QMenu* fxMenu = window->sampleTrackView()->createFxMenu(
                tr("Assign to:"), tr("New FX Channel"));
        contextMenu->addMenu(fxMenu);

        contextMenu->addSeparator();
    }

    addDefaultActions(contextMenu);
    contextMenu->exec(QCursor::pos());
}
