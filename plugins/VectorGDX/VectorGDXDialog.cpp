/*
 * VectorGDXDialog.cpp - control dialog for wall effect
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

#include "VectorGDXDialog.h"

#include "Knob.h"
#include "TempoSyncKnob.h"
#include "VectorGDXControls.h"

#include "embed.h"

#include <QGridLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QPushButton>
#include <QWidget>

class VectorGDXDisplay : public QWidget
{
  public:
    VectorGDXDisplay(VectorGDXControls* _controls, QWidget* _parent) :
          QWidget(_parent), m_controls(_controls), m_prev(-1)
    {
        update();
    }

    virtual void mousePressEvent(QMouseEvent* _me)
    {
        _me->accept();
        int i = _me->x() / m_cell;
        if(i < 0 || i >= m_cols)
            return;
        int j = _me->y() / m_cell;
        if(j < 0 || j >= m_rows)
            return;

        const int size = m_controls->size();
        int       k    = i + j * m_cols;
        if(k < 0 || k >= size || k == m_prev)
            return;

        real_t* vector = m_controls->vector();
        real_t  vmax   = 0.;
        for(int i = 0; i < size; i++)
        {
            const real_t v = abs(vector[i]);
            if(vmax < v)
                vmax = v;
        }

        const real_t level = m_controls->level();
        if(vmax < level / size)
            vmax = level / size;

        real_t v = vmax;
        if(_me->modifiers() & Qt::ShiftModifier)
            v = -vmax;
        if(_me->modifiers() & Qt::ControlModifier)
            v = 0.;

        m_prev    = k;
        vector[k] = (1. - level) * vector[k] + v;

        update();  // m_controls->normalizeVector();
    }

    virtual void mouseMoveEvent(QMouseEvent* _me)
    {
        mousePressEvent(_me);
    }

    virtual void mouseReleaseEvent(QMouseEvent* _me)
    {
        m_prev = -1;
    }

    virtual void paintEvent(QPaintEvent* _pe)
    {
        QPainter p(this);

        p.setRenderHint(QPainter::Antialiasing, false);
        // p.fillRect(0, 0, width(), height(), QColor(128, 128, 128));

        const int     size   = m_controls->size();
        const real_t* vector = m_controls->vector();

        p.setRenderHint(QPainter::Antialiasing, true);
        real_t vmax = 0.;
        for(int i = 0; i < size; i++)
        {
            const real_t v = abs(vector[i]);
            if(vmax < v)
                vmax = v;
        }
        for(int i = 0; i < size; i++)
        {
            real_t v     = vector[i];
            int    blue  = qBound(0, 128 + int(127. * v / vmax), 255);
            int    green = (255 - blue) * 3 / 4;
            int    red   = 0;

            QColor c(red, green, blue);
            p.fillRect(m_cell * (i % m_cols), m_cell * (i / m_cols), m_cell,
                       m_cell, c);
        }

        p.setRenderHint(QPainter::Antialiasing, false);
    }

  public slots:
    virtual void update()
    {
        int size = m_controls->size();
        m_cols   = 13;
        m_rows   = size / m_cols;
        if(m_cols * m_rows < size)
            m_rows++;
        m_cell = 18;
        setFixedSize(234,234);

        QWidget::update();
    }

  private:
    VectorGDXControls* m_controls;
    int                m_cols;
    int                m_rows;
    int                m_cell;
    int                m_prev;
};

VectorGDXDialog::VectorGDXDialog(VectorGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    setPalette(pal);

    QGridLayout* mainLOT = new QGridLayout(this);
    mainLOT->setContentsMargins(6, 6, 6, 6);
    mainLOT->setSpacing(6);

    Knob* sizeKNB = new Knob(this);
    sizeKNB->setModel(&controls->m_sizeModel);
    sizeKNB->setText(tr("SIZE"));
    sizeKNB->setHintText(tr("Size:"), "");

    Knob* levelKNB = new Knob(this);
    levelKNB->setModel(&controls->m_levelModel);
    levelKNB->setText(tr("LEVEL"));
    levelKNB->setHintText(tr("Level:"), "%");

    Knob* gainKNB = new Knob(this);
    gainKNB->setModel(&controls->m_gainModel);
    gainKNB->setText(tr("GAIN"));
    gainKNB->setHintText(tr("Gain:"), "");

    Knob* feedbackKNB = new Knob(this);
    feedbackKNB->setModel(&controls->m_feedbackModel);
    feedbackKNB->setText(tr("FEED"));
    feedbackKNB->setHintText(tr("Feedback:"), "");

    Knob* latencyKNB = new Knob(this);
    latencyKNB->setModel(&controls->m_latencyModel);
    latencyKNB->setText(tr("LTNCY"));
    latencyKNB->setHintText(tr("Latency:"), "");

    QPushButton* identifyBTN = new QPushButton("Identify", this);
    connect(identifyBTN, SIGNAL(clicked()), controls, SLOT(identifyVector()));

    QPushButton* randomizeBTN = new QPushButton("Randomize", this);
    connect(randomizeBTN, SIGNAL(clicked()), controls,
            SLOT(randomizeVector()));

    QPushButton* softenBTN = new QPushButton("Soften", this);
    connect(softenBTN, SIGNAL(clicked()), controls, SLOT(softenVector()));

    QPushButton* normalizeBTN = new QPushButton("Normalize", this);
    connect(normalizeBTN, SIGNAL(clicked()), controls,
            SLOT(normalizeVector()));

    QPushButton* headBTN = new QPushButton("Head", this);
    connect(headBTN, SIGNAL(clicked()), controls, SLOT(headVector()));

    QPushButton* tailBTN = new QPushButton("Tail", this);
    connect(tailBTN, SIGNAL(clicked()), controls, SLOT(tailVector()));

    VectorGDXDisplay* vdd = new VectorGDXDisplay(controls, this);
    connect(controls, SIGNAL(dataChanged()), vdd, SLOT(update()));

    int col = 0, row = 0;  // first row
    mainLOT->addWidget(vdd, row, col, 1, 4);

    ++row;
    mainLOT->addWidget(sizeKNB, row, col++, 1, 1,
                       Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(levelKNB, row, col++, 1, 1,
                       Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(gainKNB, row, col++, 1, 1,
                       Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(feedbackKNB, row, col++, 1, 1,
                       Qt::AlignBottom | Qt::AlignHCenter);
    mainLOT->addWidget(latencyKNB, row, col++, 1, 1,
                       Qt::AlignBottom | Qt::AlignHCenter);

    ++row;
    mainLOT->addWidget(normalizeBTN, row, col = 0, 1, 3, Qt::AlignBottom);
    mainLOT->addWidget(identifyBTN, row, col = 3, 1, 3, Qt::AlignBottom);

    col = 0;
    ++row;
    mainLOT->addWidget(softenBTN, row, col = 0, 1, 3, Qt::AlignBottom);
    mainLOT->addWidget(randomizeBTN, row, col = 3, 1, 3, Qt::AlignBottom);

    col = 0;
    ++row;
    mainLOT->addWidget(headBTN, row, col = 0, 1, 3, Qt::AlignBottom);
    mainLOT->addWidget(tailBTN, row, col = 3, 1, 3, Qt::AlignBottom);

    mainLOT->setColumnStretch(7, 1);
    mainLOT->setRowStretch(4, 1);

    setFixedWidth(250);
    // setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);
}
