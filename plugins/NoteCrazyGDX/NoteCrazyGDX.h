/*
 * NoteCrazyGDX.h -
 *
 * Copyright (c) 2019 gi0e5b06 (on github.com)
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

#ifndef NOTECRAZYGDX_H
#define NOTECRAZYGDX_H

#include "InstrumentFunction.h"
#include "Plugin.h"

class PLUGIN_EXPORT NoteCrazyGDX : public Plugin, public InstrumentFunction
{
  public:
    NoteCrazyGDX(Model*                                    parent,
                 const Descriptor::SubPluginFeatures::Key* key);
    virtual ~NoteCrazyGDX();

    virtual bool processNote(NotePlayHandle* n);
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    virtual QString nodeName() const;

    virtual InstrumentFunctionView* createView();

  protected:
    virtual PluginView* instantiateView(QWidget*);
    virtual bool        shouldProcessNote(NotePlayHandle* n);

  private:
    // friend class NoteCrazyGDXView;
};

#endif
