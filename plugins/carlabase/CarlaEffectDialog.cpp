/*
 * Carla for LSMM
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (C) 2014      Filipe Coelho <falktx@falktx.com>
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

#include "CarlaEffectDialog.h"

#include "CarlaEffect.h"
#include "CarlaEffectControls.h"
#include "MidiProcessorView.h"
#include "Song.h"
#include "embed.h"
#include "gui_templates.h"

#include <QCompleter>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

CarlaEffectDialog::CarlaEffectDialog(CarlaEffectControls* controls) :
      EffectControlDialog(controls), fHandle(controls->m_effect->fHandle),
      fDescriptor(controls->m_effect->fDescriptor),
      fTimerId(fHandle != NULL && fDescriptor->ui_idle != NULL
                       ? startTimer(30)
                       : 0),
      m_carlaEffect(controls->m_effect), m_paramsView(nullptr)
{
    /*
    if (QWidget* const window = parent->window())
            controls->m_effect->fHost.uiParentId = window->winId();
    else
    */
    controls->m_effect->fHost.uiParentId = 0;
    std::free((char*)controls->m_effect->fHost.uiName);
    // TODO - get plugin instance name
    // fHost.uiName = strdup(parent->windowTitle().toUtf8().constData());
    controls->m_effect->fHost.uiName = strdup(
            controls->m_effect->kIsPatchbay ? "CarlaEffectBay-LMMS"
                                            : "CarlaEffectRack-LMMS");

    // return new CarlaEffectDialog(this, parent);

    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(),
                 controls->m_effect->kIsPatchbay
                         ? PLUGIN_NAME::getPixmap("artwork-patchbay")
                         : PLUGIN_NAME::getPixmap("artwork-rack"));
    setPalette(pal);
    setMinimumSize(250, 310);

    // QLabel* lb=new QLabel("Controls",this);
    // lb->setGeometry(10,10,130,140);

    /*
      QVBoxLayout* l = new QVBoxLayout(this);
      l->setContentsMargins(6, 66, 6, 6);
      l->setSpacing(6);
    */
    QGridLayout* mainLOT = new QGridLayout(this);
    mainLOT->setContentsMargins(6, 66, 6, 6);
    mainLOT->setHorizontalSpacing(0);
    mainLOT->setVerticalSpacing(6);
    setLayout(mainLOT);

    m_toggleUIButton = new QPushButton(tr("Show GUI"), this);
    // m_toggleUIButton->setGeometry(10,10,130,140);
    m_toggleUIButton->setCheckable(true);
    m_toggleUIButton->setChecked(false);
    m_toggleUIButton->setIcon(embed::getIconPixmap("zoom"));
    m_toggleUIButton->setFont(pointSize<8>(m_toggleUIButton->font()));
    connect(m_toggleUIButton, SIGNAL(clicked(bool)), this,
            SLOT(toggleUI(bool)));
    m_toggleUIButton->setWhatsThis(
            tr("Click here to show or hide the graphical user interface "
               "(GUI) of Carla."));

    // Open params sub window button
    m_toggleParamsWindowButton = new QPushButton(tr("Parameters"), this);
    m_toggleParamsWindowButton->setCheckable(true);
    m_toggleParamsWindowButton->setChecked(false);
    m_toggleParamsWindowButton->setIcon(embed::getIcon("controller"));
    m_toggleParamsWindowButton->setFont(
            pointSize<8>(m_toggleParamsWindowButton->font()));
    connect(m_toggleParamsWindowButton, SIGNAL(clicked(bool)), this,
            SLOT(toggleParamsWindow(bool)));
    m_toggleParamsWindowButton->setWhatsThis(
            tr("Click here to show or hide the parameters "
               "of your Carla effect."));
    // m_toggleParamsWindowButton->setFixedSize(250, 22);

    /*
    QWidget* kg = new QWidget(this);
    kg->setFixedSize(228, 117 + 21 + 34);
    for(int i = 0; i < NB_KNOBS; i++)
    {
        m_knobs[i] = new Knob(knobBright_26, kg);
        m_knobs[i]->setText(QString("CC %1").arg(NB_KNOB_START + i));
        m_knobs[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
                                        .arg(MIDI_CH)
                                        .arg(NB_KNOB_START + i),
                                "");
        m_knobs[i]->setWhatsThis("");
        m_knobs[i]->setGeometry(39 * (i % 6), 39 * (i / 6), 39, 39);
        m_knobs[i]->setModel(controls->m_knobs[i]);
        connect(controls->m_knobs[i], SIGNAL(dataChanged()), this,
                SLOT(onDataChanged()));
    }
    for(int i = 0; i < NB_LEDS; i++)
    {
        m_leds[i] = new LedCheckBox(kg);
        m_leds[i]->setText(QString("CC %1").arg(NB_LED_START + i));
        m_leds[i]->setTextAnchorPoint(Qt::AnchorBottom);
        // m_leds[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
        //                        .arg(MIDI_CH).arg(NB_LED_START+i),"");
        // m_leds[i]->setWhatsThis("");
        m_leds[i]->setGeometry(29 * (i % 8), 117 + 21 * (i / 8), 29, 21);
        m_leds[i]->setModel(controls->m_leds[i]);
        connect(controls->m_leds[i], SIGNAL(dataChanged()), this,
                SLOT(onDataChanged()));
    }
    for(int i = 0; i < NB_LCDS; i++)
    {
        m_lcds[i] = new LcdSpinBox(3, kg);
        m_lcds[i]->setText(QString("CC %1").arg(NB_LCD_START + i));
        // m_lcds[i]->setHintText(QString("MIDI Channel %1, CC %2, V ")
        //                        .arg(MIDI_CH).arg(NB_LCD_START+i),"");
        // m_lcds[i]->setWhatsThis("");
        m_lcds[i]->setGeometry(58 * (i % 4), 117 + 21 + 34 * (i / 4), 58, 34);
        m_lcds[i]->setModel(controls->m_lcds[i]);
        connect(controls->m_lcds[i], SIGNAL(dataChanged()), this,
                SLOT(onDataChanged()));
    }
    mainLOT->addWidget(kg);
    */

    QWidget*     midiProcPNL = new QWidget(this);
    QHBoxLayout* midiProcLOT = new QHBoxLayout(midiProcPNL);
    midiProcLOT->setContentsMargins(6, 6, 6, 6);
    midiProcLOT->setSpacing(6);
    m_midiInProc = new MidiProcessorView(true, m_carlaEffect, midiProcPNL);
    // m_midiOutProc = new MidiProcessorView(false, m_carlaEffect,
    // midiProcPNL);
    midiProcLOT->addWidget(m_midiInProc);
    // midiProcLOT->addWidget(m_midiOutProc);

    mainLOT->addWidget(m_toggleUIButton, 0, 0);  //,Qt::AlignHCenter);
    mainLOT->addWidget(m_toggleParamsWindowButton, 1,
                       0);  //,Qt::AlignHCenter);
    mainLOT->addWidget(midiProcPNL, 2, 0, Qt::AlignHCenter);

    // mainLOT->addStretch(1);
    mainLOT->setColumnStretch(0, 1);
    mainLOT->setRowStretch(3, 1);

    connect(controls->m_effect, SIGNAL(uiClosed()), this, SLOT(uiClosed()));

    setFixedWidth(250);
    setMinimumHeight(310);
}

CarlaEffectDialog::~CarlaEffectDialog()
{
    if(m_toggleUIButton->isChecked())
        toggleUI(false);
}

void CarlaEffectDialog::toggleUI(bool visible)
{
    if(fHandle != NULL && fDescriptor->ui_show != NULL)
        fDescriptor->ui_show(fHandle, visible);
}

void CarlaEffectDialog::uiClosed()
{
    m_toggleUIButton->setChecked(false);
}

void CarlaEffectDialog::toggleParamsWindow(bool visible)
{
    qInfo("CarlaEffectDialog::toggleParamsWindow 0.1");
    if(m_paramsView == nullptr)
    {
        qInfo("CarlaEffectDialog::toggleParamsWindow 0.2");
        m_paramsView = new CarlaEffectParamsView(m_carlaEffect,
                                                 this);  // p_parent);
        connect(m_paramsView->subWindow(), SIGNAL(closed()), this,
                SLOT(paramsWindowClosed()));
    }
    qInfo("CarlaEffectDialog::toggleParamsWindow 0.3");
    if(visible != m_paramsView->subWindow()->isVisible())
    {
        if(visible)
        {
            qInfo("CarlaEffectDialog::toggleParamsWindow 1");
            m_paramsView->subWindow()->show();
            qInfo("CarlaEffectDialog::toggleParamsWindow 2");
            m_paramsView->show();
            qInfo("CarlaEffectDialog::toggleParamsWindow 3");
        }
        else
        {
            m_paramsView->subWindow()->hide();
            m_paramsView->hide();
        }
    }
}

void CarlaEffectDialog::paramsWindowClosed()
{
    m_toggleParamsWindowButton->setChecked(false);
}

/*
void CarlaEffectDialog::onDataChanged()
{
    QObject* o = sender();
    if(!o)
        qInfo("no sender");
    AutomatableModel* m = dynamic_cast<AutomatableModel*>(sender());
    if(m)
    {
        int cc = -1;
        int v  = -1;
        if(cc == -1)
            for(int i = 0; i < NB_KNOBS; i++)
                if(m_knobs[i]->model() == m)
                {
                    cc = NB_KNOB_START + i;
                    v  = m_knobs[i]->model()->value();
                }
        if(cc == -1)
            for(int i = 0; i < NB_LEDS; i++)
                if(m_leds[i]->model() == m)
                {
                    cc = NB_LED_START + i;
                    v  = m_leds[i]->model()->value();
                }

        if(cc == -1)
            for(int i = 0; i < NB_LCDS; i++)
                if(m_lcds[i]->model() == m)
                {
                    cc = NB_LCD_START + i;
                    v  = m_lcds[i]->model()->value();
                }
        if(cc != -1)
        {
            qInfo("cc %d: data changed: %d", cc, v);
            CarlaEffect* fx
                    = dynamic_cast<CarlaEffect*>(controls()->effect());
            if(fx)
            {
                MidiEvent ev(MidiEventTypes::MidiControlChange, MIDI_CH - 1,
                             cc, v);
                PlayPos   pos = Engine::getSong()->getPlayPos();
                // qInfo("sending midi event");
                fx->handleMidiEvent(ev, pos, pos.currentFrame());
            }
            else
                qInfo("fx is null");
        }
        else
            qInfo("cc model not found");
    }
    else
        qInfo("sender but no model");
}
*/

void CarlaEffectDialog::modelChanged()
{
}

void CarlaEffectDialog::timerEvent(QTimerEvent* event)
{
    if(event->timerId() == fTimerId)
        fDescriptor->ui_idle(fHandle);

    EffectControlDialog::timerEvent(event);
}

CarlaEffectParamsView::CarlaEffectParamsView(CarlaEffect* effect,
                                             QWidget*     parent) :
      QWidget(parent),
      m_carlaEffect(effect)
{
    lMaxColumns = 3;
    lCurColumn  = 0;
    lCurRow     = 0;
    // Create central widget
    /*  ___ centralWidget _______________	QWidget
     * |  __ verticalLayout _____________	QVBoxLayout
     * | |  __ m_toolBarLayout __________	QHBoxLayout
     * | | |
     * | | | option_0 | option_1 ..
     * | | |_____________________________
     * | |
     * | |  __ m_scrollArea _____________	QScrollArea
     * | | |  __ m_scrollAreaWidgetContent	QWidget
     * | | | |  __ m_scrollAreaLayout ___	QGridLayout
     * | | | | |
     * | | | | | knob | knob | knob
     * | | | | | knob | knob | knob
     * | | | | | knob | knob | knob
     * | | | | |_________________________
     * | | | |___________________________
     * | | |_____________________________
     * */
    // QWidget*     centralWidget  = new QWidget(this);
    QVBoxLayout* verticalLayout = new QVBoxLayout(this);  // centralWidget);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->setSpacing(6);

    // Toolbar
    m_toolBarLayout = new QHBoxLayout();
    m_toolBarLayout->setContentsMargins(3, 3, 3, 3);
    m_toolBarLayout->setSpacing(3);

    // Refresh params button
    m_refreshParamsButton = new QPushButton(tr(""), this);
    m_refreshParamsButton->setIcon(embed::getIcon("reload"));
    m_refreshParamsButton->setToolTip(tr("Reload Carla parameters."));
    m_refreshParamsButton->setFixedSize(24, 24);

    // Refresh param values button
    m_refreshParamValuesButton = new QPushButton(tr(""), this);
    m_refreshParamValuesButton->setIcon(embed::getIcon("synchronize"));
    m_refreshParamValuesButton->setToolTip(tr("Synchronize LSMM with Carla"));
    m_refreshParamValuesButton->setFixedSize(24, 24);

    // Params filter line edit
    m_paramsFilterLineEdit = new QLineEdit(this);
    m_paramsFilterLineEdit->setPlaceholderText(tr("Search.."));
    m_paramCompleter = new QCompleter(m_carlaEffect->m_completerList);
    m_paramsFilterLineEdit->setCompleter(m_paramCompleter);
    m_paramsFilterLineEdit->setFixedHeight(24);

    // Clear filter line edit button
    m_clearFilterButton = new QPushButton(tr(""), this);
    m_clearFilterButton->setIcon(embed::getIcon("edit_erase"));
    m_clearFilterButton->setToolTip(tr("Clear filter text"));
    m_clearFilterButton->setFixedSize(24, 24);

    // Show automated only button
    m_automatedOnlyButton = new QPushButton(tr(""), this);
    m_automatedOnlyButton->setIcon(embed::getIcon("automation"));
    m_automatedOnlyButton->setToolTip(
            tr("Only show knobs with a connection."));
    m_automatedOnlyButton->setCheckable(true);
    m_automatedOnlyButton->setFixedSize(24, 24);

    // Add stuff to toolbar
    m_toolBarLayout->addWidget(m_refreshParamsButton);
    m_toolBarLayout->addWidget(m_refreshParamValuesButton);
    m_toolBarLayout->addWidget(m_paramsFilterLineEdit);
    m_toolBarLayout->addWidget(m_clearFilterButton);
    m_toolBarLayout->addWidget(m_automatedOnlyButton);

    // Create scroll area for the knobs
    m_scrollArea              = new QScrollArea(this);
    m_scrollAreaWidgetContent = new QWidget();
    m_scrollAreaLayout        = new QGridLayout(m_scrollAreaWidgetContent);
    m_scrollAreaWidgetContent->setLayout(m_scrollAreaLayout);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scrollArea->setWidget(m_scrollAreaWidgetContent);
    m_scrollArea->setWidgetResizable(true);
    m_scrollAreaLayout->setContentsMargins(3, 3, 3, 3);
    m_scrollAreaLayout->setVerticalSpacing(3);
    m_scrollAreaLayout->setHorizontalSpacing(6);
    m_scrollAreaLayout->setColumnStretch(lMaxColumns, 1);

    // Add m_toolBarLayout and m_scrollArea to the verticalLayout.
    verticalLayout->addLayout(m_toolBarLayout);
    verticalLayout->addWidget(m_scrollArea);

    /*
    CarlaParamsSubWindow* win = new CarlaParamsSubWindow(
            gui->mainWindow()->workspace()->viewport(),
            Qt::SubWindow | Qt::CustomizeWindowHint | Qt::WindowTitleHint
                    | Qt::WindowSystemMenuHint);
    m_subWindow
            = gui->mainWindow()->workspace()->addSubWindow(win);
    */

    /*m_subWindow->*/
    setWindowTitle(m_carlaEffect->displayName() + " - " + tr("Parameters"));

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    setFixedWidth(250);
    setMinimumHeight(500);

    qInfo("CarlaEffectParamsView::CarlaEffectParamsView 1");
    m_subWindow = SubWindow::putWidgetOnWorkspace(this, false, false, false);
    qInfo("CarlaEffectParamsView::CarlaEffectParamsView 2");
    // m_subWindow->setWidget(centralWidget);
    m_subWindow->hide();

    // Connect signals
    connect(m_refreshParamsButton, SIGNAL(clicked(bool)), this,
            SLOT(onRefreshButton()));
    connect(m_refreshParamValuesButton, SIGNAL(clicked(bool)), this,
            SLOT(onRefreshValuesButton()));
    connect(m_paramsFilterLineEdit, SIGNAL(textChanged(const QString)), this,
            SLOT(filterKnobs()));
    connect(m_clearFilterButton, SIGNAL(clicked(bool)), this,
            SLOT(clearFilterText()));
    connect(m_automatedOnlyButton, SIGNAL(toggled(bool)), this,
            SLOT(filterKnobs()));

    qInfo("CarlaEffectParamsView::CarlaEffectParamsView 3");
    refreshKnobs();  // Add buttons if there are any already.
    qInfo("CarlaEffectParamsView::CarlaEffectParamsView 4");
    m_subWindow->show();  // Show the subwindow
    qInfo("CarlaEffectParamsView::CarlaEffectParamsView 5");
}

CarlaEffectParamsView::~CarlaEffectParamsView()
{
    qInfo("CarlaEffectParamsView::~CarlaEffectParamsView START");

    // Close and delete m_subWindow
    if(m_subWindow != nullptr)
    {
        m_subWindow->setAttribute(Qt::WA_DeleteOnClose);
        m_subWindow->close();
        // if(m_subWindow != nullptr)
        // delete m_subWindow;
        m_subWindow = nullptr;
    }
    // p_subWindow = nullptr;
    // Clear models
    if(m_carlaEffect->paramModels.isEmpty() == false)
    {
        m_carlaEffect->clearParamModels();
    }

    qInfo("CarlaEffectParamsView::~CarlaEffectParamsView END");
}

void CarlaEffectParamsView::clearFilterText()
{
    m_paramsFilterLineEdit->setText("");
}

void CarlaEffectParamsView::filterKnobs()
{
    QString text = m_paramsFilterLineEdit->text();
    clearKnobs();  // Remove all knobs from the layout.
    for(uint32_t i = 0; i < m_knobs.count(); ++i)
    {
        // Filter on automation only
        if(m_automatedOnlyButton->isChecked())
        {
            if(!m_carlaEffect->paramModels[i]->isAutomatedOrControlled())
            {
                continue;
            }
        }

        // Filter on text
        if(text != "")
        {
            if(m_knobs[i]->objectName().contains(text, Qt::CaseInsensitive))
            {
                addKnob(i);
            }
        }
        else
        {
            addKnob(i);
        }
    }
}

void CarlaEffectParamsView::onRefreshButton()
{
    if(m_carlaEffect->paramModels.isEmpty() == false)
    {
        if(QMessageBox::warning(
                   nullptr, tr("Reload knobs"),
                   tr("There are already knobs loaded, if any of them "
                      "are connected to a controller or automation track "
                      "their connection will be lost. Do you want to "
                      "continue?"),
                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
           != QMessageBox::Yes)
        {
            return;
        }
    }
    m_carlaEffect->refreshParams(false);
    refreshKnobs();
}

void CarlaEffectParamsView::onRefreshValuesButton()
{
    m_carlaEffect->refreshParams(true);
}

void CarlaEffectParamsView::refreshKnobs()
{
    if(m_carlaEffect->paramModels.count() == 0)
    {
        return;
    }
    // Make sure all the knobs are deleted.
    for(uint32_t i = 0; i < m_knobs.count(); ++i)
    {
        delete m_knobs[i];  // Delete knob widgets itself.
    }
    m_knobs.clear();  // Clear the pointer list.
                      // Clear the layout (posible spacer).
    QLayoutItem* item;
    while((item = m_scrollAreaLayout->takeAt(0)))
    {
        if(item->widget())
        {
            delete item->widget();
        }
        delete item;
    }

    // Reset position data.
    lCurColumn = 0;
    lCurRow    = 0;
    // Make room in QList m_knobs
    m_knobs.reserve(m_carlaEffect->paramModels.count());
    for(uint32_t i = 0; i < m_carlaEffect->paramModels.count(); ++i)
    {
        m_knobs.push_back(new Knob(m_scrollAreaWidgetContent));
        QString name = (*m_carlaEffect->paramModels[i]).displayName();
        m_knobs[i]->setHintText(name, "");
        m_knobs[i]->setText(name);
        m_knobs[i]->setObjectName(
                name);  // this is being used for filtering the knobs.
                        // Set the newly created model to the knob.
        m_knobs[i]->setModel(m_carlaEffect->paramModels[i]);
        m_knobs[i]->setFixedSize(228 / lMaxColumns - 6, 36);
        // Add knob to layout
        addKnob(i);
    }
    // Add spacer so all knobs go to top
    /*
    QSpacerItem* verticalSpacer = new QSpacerItem(
            20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_scrollAreaLayout->addItem(verticalSpacer, lCurRow + 1, 0, 1, 1);
    */
}

void CarlaEffectParamsView::addKnob(uint32_t index)
{
    // Add the new knob to layout
    m_scrollAreaLayout->addWidget(m_knobs[index], lCurRow, lCurColumn,
                                  Qt::AlignHCenter | Qt::AlignTop);
    // Chances that we did close() on the widget is big, so show it.
    m_knobs[index]->show();
    // Keep track of current column and row index.
    if(lCurColumn < lMaxColumns - 1)
    {
        lCurColumn++;
    }
    else
    {
        lCurColumn = 0;
        lCurRow++;
    }
}

void CarlaEffectParamsView::clearKnobs()
{
    // Remove knobs from layout.
    for(uint32_t i = 0; i < m_knobs.count(); ++i)
    {
        m_knobs[i]->close();
    }
    // Reset position data.
    lCurColumn = 0;
    lCurRow    = 0;
}

void CarlaEffectParamsView::modelChanged()
{
    refreshKnobs();
}
