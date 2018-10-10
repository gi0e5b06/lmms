/*
 * VectorGDXControls.h - controls for wall effect
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

#ifndef VECTORGDX_CONTROLS_H
#define VECTORGDX_CONTROLS_H

#include "EffectControls.h"
#include "TempoSyncKnobModel.h"
//#include "Knob.h"
#include "VectorGDXDialog.h"

class VectorGDXEffect;

class VectorGDXControls : public EffectControls
{
    Q_OBJECT

  public:
    VectorGDXControls(VectorGDXEffect* effect);
    virtual ~VectorGDXControls()
    {
    }
    virtual void saveSettings(QDomDocument& _doc, QDomElement& _parent);
    virtual void loadSettings(const QDomElement& _this);
    inline virtual QString nodeName() const
    {
        return "VectorGDXControls";
    }

    virtual int controlCount()
    {
        return 1;
    }

    virtual EffectControlDialog* createView()
    {
        return new VectorGDXDialog(this);
    }

    int size()
    {
        return m_sizeModel.value();
    }

    int* pos()
    {
        return m_pos;
    }

    real_t* vector()
    {
        return m_vector;
    }

    real_t level()
    {
        return m_levelModel.value() / 100.;
    }

  public slots:
    void identifyVector();
    void headVector();
    void tailVector();
    void randomizeVector();
    void softenVector();
    void normalizeVector();

  signals:
    void dataChanged();

  private slots:
    void changeControl();

  private:
    VectorGDXEffect* m_effect;

    FloatModel         m_sizeModel;
    FloatModel         m_levelModel;
    FloatModel         m_gainModel;
    FloatModel         m_feedbackModel;
    TempoSyncKnobModel m_latencyModel;
    FloatModel         m_frequencyModel;

    static const int N = 169;
    int              m_pos[N];
    real_t           m_vector[N];

    friend class VectorGDXDialog;
    friend class VectorGDXEffect;
};

#endif
