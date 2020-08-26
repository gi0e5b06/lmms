/*
 * Filter.h -
 *
 * Copyright (c) 2020-2020 gi0e5b06 (on github.com)
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

#ifndef FILTER_H
#define FILTER_H

#include "ComboBoxModel.h"
#include "TempoSyncKnobModel.h"

class InstrumentSoundShaping;

class Filter : public Model, public JournallingObject
{
    Q_OBJECT

  public:
    Filter(InstrumentSoundShaping* _parent,
           const QString&          _displayName = QString::null,
           const QString&          _objectName  = QString::null);
    virtual ~Filter();

  private:
    BoolModel          m_enabledModel;
    ComboBoxModel      m_typeModel;
    FloatModel         m_cutModel;
    FloatModel         m_resModel;  // Q, BW
    ComboBoxModel      m_passesModel;
    FloatModel         m_gainModel;
    FloatModel         m_responseModel;
    FloatModel         m_feedbackAmountModel;
    TempoSyncKnobModel m_feedbackDelayModel;
};

#endif
