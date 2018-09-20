/*
 * FrequencyGDXDialog.cpp -
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

#include "FrequencyGDXDialog.h"

#include "ComboBox.h"
#include "FrequencyGDXControls.h"
#include "Knob.h"
#include "embed.h"

#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>

FrequencyGDXDialog::FrequencyGDXDialog(FrequencyGDXControls* controls) :
      EffectControlDialog(controls)
{
    setWindowIcon(PLUGIN_NAME::getIcon("logo"));

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), embed::getIconPixmap("plugin_bg"));
    // pal.setBrush(backgroundRole(),PLUGIN_NAME::getIconPixmap("artwork"));
    setPalette(pal);

    QGridLayout* mainLayout = new QGridLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);
    mainLayout->setSpacing(6);

    m_topFrequencyLBL = new QLabel("------", this);
    m_topKeyLBL       = new QLabel("---", this);
    m_topNoteLBL      = new QLabel("-", this);

    m_avgFrequencyLBL = new QLabel("------", this);
    m_avgKeyLBL       = new QLabel("---", this);
    m_avgNoteLBL      = new QLabel("-", this);

    m_mainFrequencyLBL = new QLabel("------", this);
    m_mainKeyLBL       = new QLabel("---", this);
    m_mainNoteLBL      = new QLabel("-", this);

    m_topFrequencyLBL->setAlignment(Qt::AlignRight);
    m_avgFrequencyLBL->setAlignment(Qt::AlignRight);
    m_mainFrequencyLBL->setAlignment(Qt::AlignRight);

    m_topKeyLBL->setAlignment(Qt::AlignHCenter);
    m_avgKeyLBL->setAlignment(Qt::AlignHCenter);
    m_mainKeyLBL->setAlignment(Qt::AlignHCenter);

    m_topNoteLBL->setAlignment(Qt::AlignHCenter);
    m_avgNoteLBL->setAlignment(Qt::AlignHCenter);
    m_mainNoteLBL->setAlignment(Qt::AlignHCenter);

    m_topFrequencyLBL->setFixedWidth(51);
    m_avgFrequencyLBL->setFixedWidth(51);
    m_mainFrequencyLBL->setFixedWidth(51);

    m_topKeyLBL->setFixedWidth(51);
    m_avgKeyLBL->setFixedWidth(51);
    m_mainKeyLBL->setFixedWidth(51);

    m_topNoteLBL->setFixedWidth(51);
    m_avgNoteLBL->setFixedWidth(51);
    m_mainNoteLBL->setFixedWidth(51);

    Knob* m_topFrequencyKNB = new Knob(this);
    m_topFrequencyKNB->setModel(&controls->m_topFrequencyModel);
    m_topFrequencyKNB->setText("INST");

    Knob* m_topKeyKNB = new Knob(this);
    m_topKeyKNB->setModel(&controls->m_topKeyModel);
    m_topKeyKNB->setText("KEY");
    m_topKeyKNB->setPointColor(Qt::cyan);

    Knob* m_topNoteKNB = new Knob(this);
    m_topNoteKNB->setModel(&controls->m_topNoteModel);
    m_topNoteKNB->setText("NOTE");
    m_topNoteKNB->setPointColor(Qt::darkCyan);

    Knob* m_avgFrequencyKNB = new Knob(this);
    m_avgFrequencyKNB->setModel(&controls->m_avgFrequencyModel);
    m_avgFrequencyKNB->setText("AVG");

    Knob* m_avgKeyKNB = new Knob(this);
    m_avgKeyKNB->setModel(&controls->m_avgKeyModel);
    m_avgKeyKNB->setText("KEY");
    m_avgKeyKNB->setPointColor(Qt::cyan);

    Knob* m_avgNoteKNB = new Knob(this);
    m_avgNoteKNB->setModel(&controls->m_avgNoteModel);
    m_avgNoteKNB->setText("NOTE");
    m_avgNoteKNB->setPointColor(Qt::darkCyan);

    Knob* m_mainFrequencyKNB = new Knob(this);
    m_mainFrequencyKNB->setModel(&controls->m_mainFrequencyModel);
    m_mainFrequencyKNB->setText("MAIN");

    Knob* m_mainKeyKNB = new Knob(this);
    m_mainKeyKNB->setModel(&controls->m_mainKeyModel);
    m_mainKeyKNB->setText("KEY");
    m_mainKeyKNB->setPointColor(Qt::cyan);

    Knob* m_mainNoteKNB = new Knob(this);
    m_mainNoteKNB->setModel(&controls->m_mainNoteModel);
    m_mainNoteKNB->setText("NOTE");
    m_mainNoteKNB->setPointColor(Qt::darkCyan);

    mainLayout->addWidget(m_topFrequencyLBL, 0, 0);
    mainLayout->addWidget(m_avgFrequencyLBL, 0, 1);
    mainLayout->addWidget(m_mainFrequencyLBL, 0, 2);

    mainLayout->addWidget(m_topFrequencyKNB, 1, 0,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(m_avgFrequencyKNB, 1, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(m_mainFrequencyKNB, 1, 2,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLayout->addWidget(m_topKeyLBL, 2, 0);
    mainLayout->addWidget(m_avgKeyLBL, 2, 1);
    mainLayout->addWidget(m_mainKeyLBL, 2, 2);

    mainLayout->addWidget(m_topKeyKNB, 3, 0,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(m_avgKeyKNB, 3, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(m_mainKeyKNB, 3, 2,
                          Qt::AlignBottom | Qt::AlignHCenter);

    mainLayout->addWidget(m_topNoteLBL, 4, 0);
    mainLayout->addWidget(m_avgNoteLBL, 4, 1);
    mainLayout->addWidget(m_mainNoteLBL, 4, 2);

    mainLayout->addWidget(m_topNoteKNB, 5, 0,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(m_avgNoteKNB, 5, 1,
                          Qt::AlignBottom | Qt::AlignHCenter);
    mainLayout->addWidget(m_mainNoteKNB, 5, 2,
                          Qt::AlignBottom | Qt::AlignHCenter);

    setFixedWidth(250);
    setMinimumHeight(((sizeHint().height() - 1) / 50 + 1) * 50);

    connect(&controls->m_topFrequencyModel, SIGNAL(dataChanged()), this,
            SLOT(updateTopFrequency()));
    connect(&controls->m_avgFrequencyModel, SIGNAL(dataChanged()), this,
            SLOT(updateAvgFrequency()));
    connect(&controls->m_mainFrequencyModel, SIGNAL(dataChanged()), this,
            SLOT(updateMainFrequency()));

    connect(&controls->m_topKeyModel, SIGNAL(dataChanged()), this,
            SLOT(updateTopKey()));
    connect(&controls->m_avgKeyModel, SIGNAL(dataChanged()), this,
            SLOT(updateAvgKey()));
    connect(&controls->m_mainKeyModel, SIGNAL(dataChanged()), this,
            SLOT(updateMainKey()));

    connect(&controls->m_topNoteModel, SIGNAL(dataChanged()), this,
            SLOT(updateTopNote()));
    connect(&controls->m_avgNoteModel, SIGNAL(dataChanged()), this,
            SLOT(updateAvgNote()));
    connect(&controls->m_mainNoteModel, SIGNAL(dataChanged()), this,
            SLOT(updateMainNote()));
}

FrequencyGDXDialog::~FrequencyGDXDialog()
{
}

void FrequencyGDXDialog::updateTopFrequency()
{
    // qInfo("FrequencyGDXDialog: updateTop");
    int v = dynamic_cast<FrequencyGDXControls*>(controls())
                    ->m_topFrequencyModel.value();
    m_topFrequencyLBL->setText(QString::number(v).append(" Hz"));
}

void FrequencyGDXDialog::updateAvgFrequency()
{
    int v = dynamic_cast<FrequencyGDXControls*>(controls())
                    ->m_avgFrequencyModel.value();
    m_avgFrequencyLBL->setText(QString::number(v).append(" Hz"));
}

void FrequencyGDXDialog::updateMainFrequency()
{
    int v = dynamic_cast<FrequencyGDXControls*>(controls())
                    ->m_mainFrequencyModel.value();
    m_mainFrequencyLBL->setText(QString::number(v).append(" Hz"));
}

void FrequencyGDXDialog::updateTopKey()
{
    int v = dynamic_cast<FrequencyGDXControls*>(controls())
                    ->m_topKeyModel.value();
    QString s = "---";
    if(v >= 0 && v <= 127)
        s = Note::findKeyName(v);
    m_topKeyLBL->setText(s);
}

void FrequencyGDXDialog::updateAvgKey()
{
    int v = dynamic_cast<FrequencyGDXControls*>(controls())
                    ->m_avgKeyModel.value();
    QString s = "---";
    if(v >= 0 && v <= 127)
        s = Note::findKeyName(v);
    m_avgKeyLBL->setText(s);
}

void FrequencyGDXDialog::updateMainKey()
{
    int v = dynamic_cast<FrequencyGDXControls*>(controls())
                    ->m_mainKeyModel.value();
    QString s = "---";
    if(v >= 0 && v <= 127)
        s = Note::findKeyName(v);
    else
        m_mainFrequencyLBL->setText(s);
    m_mainKeyLBL->setText(s);
}

void FrequencyGDXDialog::updateTopNote()
{
    int v = dynamic_cast<FrequencyGDXControls*>(controls())
                    ->m_topNoteModel.value();
    QString s = "-";
    if(v >= 0 && v < 12)
        s = Note::findNoteName(v);
    m_topNoteLBL->setText(s);
}

void FrequencyGDXDialog::updateAvgNote()
{
    int v = dynamic_cast<FrequencyGDXControls*>(controls())
                    ->m_avgNoteModel.value();
    QString s = "-";
    if(v >= 0 && v < 12)
        s = Note::findNoteName(v);
    m_avgNoteLBL->setText(s);
}

void FrequencyGDXDialog::updateMainNote()
{
    int v = dynamic_cast<FrequencyGDXControls*>(controls())
                    ->m_mainNoteModel.value();
    QString s = "-";
    if(v >= 0 && v < 12)
        s = Note::findNoteName(v);
    m_mainNoteLBL->setText(s);
}
