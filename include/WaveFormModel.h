/*
 * AutomatableModel.h - declaration of class AutomatableModel
 *
 * Copyright (c) 2018 gi0e5b06 (on github.com)
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

#ifndef WAVEFORM_MODEL_H
#define WAVEFORM_MODEL_H

#include "JournallingObject.h"
#include "Model.h"
#include "WaveForm.h"

#include <QDomElement>

class EXPORT WaveFormModel
      : public Model
      , public JournallingObject
{
    Q_OBJECT

  public:
    WaveFormModel(Model * _parent, QString _displayName = QString::null,
               bool _defaultConstructed = false);
    virtual ~WaveFormModel();

    virtual void saveSettings(QDomDocument&  doc,
                              QDomElement&   element,
                              const QString& name,
                              const bool     unique = true);
    virtual void loadSettings(const QDomElement& element,
                              const QString&     name,
                              const bool         required = true);

    const WaveForm* value() const;

  public slots:
    void setValue(const WaveForm* _wf);

  private:
    QString formatNumber(real_t v);

    const WaveForm* m_wf;
};

#endif
