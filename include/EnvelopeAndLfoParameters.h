/*
 * EnvelopeAndLfoParameters.h - class EnvelopeAndLfoParameters
 *
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

#ifndef ENVELOPE_AND_LFO_PARAMETERS_H
#define ENVELOPE_AND_LFO_PARAMETERS_H

//#include <QVector>

//#include "JournallingObject.h"
#include "AutomatableModel.h"
#include "ComboBoxModel.h"
#include "SampleBuffer.h"
#include "TempoSyncKnobModel.h"
#include "lmms_basics.h"

class EXPORT EnvelopeAndLfoParameters : public Model, public JournallingObject
{
    Q_OBJECT

  public:
    // how long should be each envelope-segment maximal (e.g. attack)?
    static constexpr real_t SECS_PER_ENV_SEGMENT = 5.;
    // how long should be one LFO-oscillation maximal?
    static constexpr real_t SECS_PER_LFO_OSCILLATION = 20.;

    class LfoInstances
    {
      public:
        LfoInstances()
        {
        }

        virtual ~LfoInstances()
        {
        }

        inline bool isEmpty() const
        {
            return m_lfos.isEmpty();
        }

        void trigger();
        void reset();

        void add(EnvelopeAndLfoParameters* lfo);
        void remove(EnvelopeAndLfoParameters* lfo);

      private:
        QMutex                                   m_lfoListMutex;
        typedef QList<EnvelopeAndLfoParameters*> LfoList;
        LfoList                                  m_lfos;
    };

    EnvelopeAndLfoParameters(real_t _value_for_zero_amount, Model* _parent);
    virtual ~EnvelopeAndLfoParameters();

    static inline real_t expKnobVal(real_t _val)
    {
        // return abs(_val);
        return ((_val < 0) ? -_val : _val) * _val;
    }

    static LfoInstances* instances()
    {
        return s_lfoInstances;
    }

    void fillLevel(real_t*       _buf,
                   f_cnt_t       _frame,
                   const f_cnt_t _release_begin,
                   const fpp_t   _frames,
                   const bool    _legato,
                   const bool    _marcato,
                   const bool    _staccato);

    inline bool isUsed() const
    {
        return m_used;
    }

    virtual void    saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void    loadSettings(const QDomElement& _this);
    virtual QString nodeName() const
    {
        return "el";
    }

    inline f_cnt_t PAHD_Frames() const
    {
        return m_pahdFrames;
    }

    inline f_cnt_t releaseFrames() const
    {
        return m_rFrames;
    }

  signals:
    void sendOut(const ValueBuffer* _v);

  public slots:
    void updateSampleVars();

  protected:
    void fillLfoLevel(real_t* _buf, f_cnt_t _frame, const fpp_t _frames);

  private:
    static LfoInstances* s_lfoInstances;
    bool                 m_used;

    QMutex m_paramMutex;

    TempoSyncKnobModel m_predelayModel;
    TempoSyncKnobModel m_attackModel;
    TempoSyncKnobModel m_holdModel;
    TempoSyncKnobModel m_decayModel;
    FloatModel         m_sustainModel;
    TempoSyncKnobModel m_releaseModel;
    FloatModel         m_amountModel;

    real_t    m_sustainLevel;
    real_t    m_amount;
    real_t    m_valueForZeroAmount;
    real_t    m_amountAdd;
    f_cnt_t   m_pahdFrames;
    f_cnt_t   m_rFrames;
    sample_t* m_pahdEnv;
    sample_t* m_rEnv;
    f_cnt_t   m_pahdBufSize;
    f_cnt_t   m_rBufSize;

    FloatModel         m_lfoPredelayModel;
    FloatModel         m_lfoAttackModel;
    TempoSyncKnobModel m_lfoSpeedModel;
    FloatModel         m_lfoAmountModel;
    // IntModel m_lfoWaveModel;
    ComboBoxModel m_lfoWaveBankModel;
    ComboBoxModel m_lfoWaveIndexModel;

    BoolModel   m_x100Model;
    BoolModel   m_controlEnvAmountModel;
    FloatModel  m_outModel;
    ValueBuffer m_outBuffer;

    f_cnt_t      m_lfoPredelayFrames;
    f_cnt_t      m_lfoAttackFrames;
    f_cnt_t      m_lfoOscillationFrames;
    f_cnt_t      m_lfoFrame;
    real_t       m_lfoAmount;
    bool         m_lfoAmountIsZero;
    sample_t*    m_lfoShapeData;
    sample_t     m_random;
    bool         m_bad_lfoShapeData;
    SampleBuffer m_userWave;

    enum LfoShapes
    {
        SineWave,
        TriangleWave,
        SawWave,
        SquareWave,
        UserDefinedWave,
        RandomWave,
        NumLfoShapes
    };

    sample_t lfoShapeSample(fpp_t _frame_offset);
    void     updateLfoShapeData();

    friend class EnvelopeAndLfoView;
};

#endif
