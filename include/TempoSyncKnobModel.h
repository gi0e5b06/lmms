/*
 * TempoSyncKnobModel.h - adds bpm to ms conversion for knob class
 *
 * Copyright (c) 2005-2008 Danny McRae <khjklujn/at/yahoo.com>
 * Copyright (c) 2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef TEMPO_SYNC_KNOB_MODEL_H
#define TEMPO_SYNC_KNOB_MODEL_H

#include "MeterModel.h"

class QAction;

class EXPORT TempoSyncKnobModel : public FloatModel
{
    Q_OBJECT
  public:
    enum TempoSyncMode
    {
        SyncNone,
        SyncDoubleWholeNote,
        SyncWholeNote,
        SyncHalfNote,
        SyncQuarterNote,
        SyncEighthNote,
        SyncSixteenthNote,
        SyncThirtySecondNote,
        SyncSixtyFourthNote,
        SyncOneHundredTwentyEighthNote,
        SyncCustom
    };

    TempoSyncKnobModel(const real_t   _val,
                       const real_t   _min,
                       const real_t   _max,
                       const real_t   _step,
                       const real_t   _scale,
                       Model*         _parent,
                       const QString& _displayName = "[tempo sync model]",
                       const QString& _objectName  = QString::null,
                       bool           _defaultConstructed = false);
    virtual ~TempoSyncKnobModel();

    void saveSettings(QDomDocument&  _doc,
                      QDomElement&   _this,
                      const QString& name);
    void loadSettings(const QDomElement& _this, const QString& name);

    TempoSyncMode syncMode() const
    {
        return m_tempoSyncMode;
    }

    void setSyncMode(TempoSyncMode _new_mode);

    real_t scale() const
    {
        return m_scale;
    }

    void setScale(real_t _new_scale);

  signals:
    void syncModeChanged(TempoSyncMode _new_mode);
    // void scaleChanged( real_t _new_scale );

  public slots:
    INLINE void disableSync()
    {
        setTempoSync(SyncNone);
    }
    void setTempoSync(int _note_type);
    void setTempoSync(QAction* _item);

  protected slots:
    void calculateTempoSyncTime(bpm_t _bpm);
    void updateCustom();

  private:
    TempoSyncMode m_tempoSyncMode;
    TempoSyncMode m_tempoLastSyncMode;
    real_t        m_scale;

    MeterModel m_custom;

    friend class TempoSyncKnob;
};

#endif
