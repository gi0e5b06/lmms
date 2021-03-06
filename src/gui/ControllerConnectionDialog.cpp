/*
 * ControllerConnectionDialog.cpp - dialog allowing the user to create and
 *	modify links between controllers and models
 *
 * Copyright (c) 2008 Paul Giblock <drfaygo/at/gmail.com>
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

#include "ControllerConnectionDialog.h"

#include "ControllerConnection.h"
//#include "HwScreenWidget.h"
#include "ComboBox.h"
#include "GroupBox.h"
#include "LcdSpinBox.h"
#include "LedCheckBox.h"
#include "MidiClient.h"
#include "MidiController.h"
#include "MidiPortMenu.h"
#include "Mixer.h"
#include "Song.h"
#include "TabWidget.h"
#include "ToolButton.h"

//#include "gui_templates.h"
//#include "debug.h"
#include "embed.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
//#include <QScrollArea>
#include <QMessageBox>

class AutoDetectMidiController : public MidiController
{
  public:
    AutoDetectMidiController(Model* parent) :
          MidiController(parent), m_detectedMidiChannel(0),
          m_detectedMidiController(0), m_detectedMidiPort("")
    {
        updateName();
    }

    virtual ~AutoDetectMidiController()
    {
    }

    virtual void processInEvent(const MidiEvent& event,
                                const MidiTime&  time,
                                f_cnt_t          offset = 0)
    {
        if(/*event.type() == MidiControlChange &&*/
           (m_midiPort.inputChannel() == 0
            || m_midiPort.inputChannel() == event.channel() + 1))
        {
            m_detectedMidiChannel = event.channel() + 1;
            m_detectedMidiPort
                    = Engine::mixer()->midiClient()->sourcePortName(event);
            if(event.type() == MidiPitchBend)
            {
                m_detectedMidiController = 0;
                m_midiPort.setWidgetType(6);
                qInfo("AutoDetectMidiController: detected '%s'",
                      qPrintable(m_detectedMidiPort));
                qInfo("                        : ch=%d pitchbend",
                      m_detectedMidiChannel);
            }
            else
            {
                m_detectedMidiController = event.controllerNumber() + 1;
                qInfo("AutoDetectMidiController: detected '%s'",
                      qPrintable(m_detectedMidiPort));
                qInfo("                        : ch=%d cc=%d",
                      m_detectedMidiChannel, m_detectedMidiController);
            }
            emit controlledValueChanged(currentValue(0));
        }
    }

    // Would be a nice copy ctor, but too hard to add copy ctor because
    // model has none.
    MidiController* copyToMidiController(Model* parent)
    {
        qInfo("AutoDetectMidiController: copy");

        MidiController* c = new MidiController(parent);
        c->m_midiPort.setInputChannel(m_midiPort.inputChannel());
        c->m_midiPort.setInputController(m_midiPort.inputController());

        c->m_midiPort.setWidgetType(m_midiPort.widgetType());
        c->m_midiPort.setMinInputValue(m_midiPort.minInputValue());
        c->m_midiPort.setMaxInputValue(m_midiPort.maxInputValue());
        c->m_midiPort.setStepInputValue(m_midiPort.stepInputValue());
        c->m_midiPort.setBaseInputValue(m_midiPort.baseInputValue());
        c->m_midiPort.setSlopeInputValue(m_midiPort.slopeInputValue());
        c->m_midiPort.setDeltaInputValue(m_midiPort.deltaInputValue());

        // c->subscribeReadablePorts(m_midiPort.readablePorts());
        c->m_midiPort.subscribeReadablePorts(m_midiPort.readablePorts());
        c->updateName();

        return c;
    }

    void useDetected()
    {
        m_midiPort.setInputChannel(m_detectedMidiChannel);
        m_midiPort.setInputController(m_detectedMidiController);

        const MidiPort::Map& map = m_midiPort.readablePorts();
        for(MidiPort::Map::ConstIterator it = map.begin(); it != map.end();
            ++it)
        {
            qInfo("AutoDetectMidiController: map '%s'", qPrintable(it.key()));
            m_midiPort.subscribeReadablePort(
                    it.key(), m_detectedMidiPort.isEmpty()
                                      || (it.key() == m_detectedMidiPort));
        }
    }

  public slots:
    void reset()
    {
        m_midiPort.reset();
    }

  private:
    int     m_detectedMidiChannel;
    int     m_detectedMidiController;
    QString m_detectedMidiPort;
};

ControllerConnectionDialog::ControllerConnectionDialog(
        QWidget* _parent, const AutomatableModel* _target_model) :
      QDialog(_parent),
      m_readablePorts(nullptr), m_midiAutoDetect(false),
      m_controller(nullptr), m_targetModel(_target_model),
      m_midiController(nullptr)
{
    setWindowIcon(embed::getIconPixmap("setup_audio"));
    setWindowTitle(tr("Connection Settings"));
    setModal(true);

    // Midi stuff
    int xg = 8, yg = 8;
    m_midiGroupBox = new GroupBox(tr("MIDI CONTROLLER"), this);
    m_midiGroupBox->setGeometry(xg, yg, 240, 108);
    connect(m_midiGroupBox, SIGNAL(enabledChanged()), this,
            SLOT(midiToggled()));

    int xw = 8, yw = 24;
    m_midiChannelSpinBox
            = new LcdSpinBox(2, m_midiGroupBox, tr("Input channel"));
    m_midiChannelSpinBox->addTextForValue(0, "--");
    m_midiChannelSpinBox->setLabel(tr("CHANNEL"));
    m_midiChannelSpinBox->move(xw, yw);
    xw += 68;

    /*
    HwScreenWidget* p=new HwScreenWidget(m_midiGroupBox,12,2,"label");
    p->setText("0AZ1BY23456789\nabcdefghij");//"Hello\nWorld!");
    p->move(108,8);
    */

    m_midiControllerSpinBox
            = new LcdSpinBox(3, m_midiGroupBox, tr("Input controller"));
    m_midiControllerSpinBox->addTextForValue(0, "---");
    m_midiControllerSpinBox->setLabel(tr("CONTROLLER"));
    m_midiControllerSpinBox->move(xw, yw);
    xw += 68 + 32;

    // when using with non-raw-clients we can provide buttons showing
    // our port-menus when being clicked
    if(!Engine::mixer()->midiClient()->isRaw())
    {
        m_readablePorts = new MidiPortMenu(MidiPort::Input);
        connect(m_readablePorts, SIGNAL(triggered(QAction*)), this,
                SLOT(enableAutoDetect(QAction*)));
        ToolButton* rp_btn = new ToolButton(m_midiGroupBox);
        rp_btn->setText(
                tr("MIDI-devices to receive "
                   "MIDI-events from"));
        rp_btn->setIcon(embed::getIconPixmap("piano"));
        rp_btn->setGeometry(xw, yw, 32, 32);
        rp_btn->setMenu(m_readablePorts);
        rp_btn->setPopupMode(QToolButton::InstantPopup);
    }

    xw = 8;
    yw += 40;
    m_midiAutoDetectCheckBox = new LedCheckBox(
            tr("Auto Detect"), m_midiGroupBox, tr("Auto Detect"));
    m_midiAutoDetectCheckBox->setModel(&m_midiAutoDetect);
    m_midiAutoDetectCheckBox->move(xw, yw);
    connect(&m_midiAutoDetect, SIGNAL(dataChanged()), this,
            SLOT(autoDetectToggled()));

    xw = 8;
    yw += 16;
    m_midiWidgetTypeComboBox
            = new ComboBox(m_midiGroupBox, tr("Widget Type"));
    m_midiWidgetTypeComboBox->setGeometry(xw, yw, 224, 22);

    xg += 248;
    m_controllerBehavior = new TabWidget(tr("BEHAVIOR"), this);
    m_controllerBehavior->setGeometry(xg, yg, 240, 108);
    xg = 8;
    yg += 116;

    xw = 8;
    yw = 24;
    m_midiMinValueSpinBox
            = new LcdSpinBox(3, m_controllerBehavior, tr("Minimal value"));
    // m_midiMinValueSpinBox->addTextForValue( 0, "---" );
    m_midiMinValueSpinBox->setLabel(tr("MIN"));
    m_midiMinValueSpinBox->move(xw, yw);
    xw += 68;

    m_midiMaxValueSpinBox
            = new LcdSpinBox(3, m_controllerBehavior, tr("Maximal value"));
    // m_midiMaxValueSpinBox->addTextForValue( 0, "---" );
    m_midiMaxValueSpinBox->setLabel(tr("MAX"));
    m_midiMaxValueSpinBox->move(xw, yw);
    xw += 68;

    m_midiStepValueSpinBox
            = new LcdSpinBox(3, m_controllerBehavior, tr("Step value"));
    m_midiStepValueSpinBox->addTextForValue(1, "---");
    m_midiStepValueSpinBox->setLabel(tr("STEP"));
    m_midiStepValueSpinBox->move(xw, yw);
    xw = 8;
    yw += 40;

    m_midiBaseValueSpinBox
            = new LcdSpinBox(3, m_controllerBehavior, tr("Base value"));
    m_midiBaseValueSpinBox->addTextForValue(0, "---");
    m_midiBaseValueSpinBox->setLabel(tr("BASE"));
    m_midiBaseValueSpinBox->move(xw, yw);
    xw += 68;

    m_midiSlopeValueSpinBox
            = new LcdSpinBox(3, m_controllerBehavior, tr("Slope value"));
    m_midiSlopeValueSpinBox->addTextForValue(1, "---");
    m_midiSlopeValueSpinBox->setLabel(tr("SLOPE"));
    m_midiSlopeValueSpinBox->move(xw, yw);
    xw += 68;

    m_midiDeltaValueSpinBox
            = new LcdSpinBox(3, m_controllerBehavior, tr("Delta value"));
    m_midiDeltaValueSpinBox->addTextForValue(0, "---");
    m_midiDeltaValueSpinBox->setLabel(tr("DELTA"));
    m_midiDeltaValueSpinBox->move(xw, yw);
    xw = 8;
    yw += 40;

    // User stuff
    m_userGroupBox = new GroupBox(tr("USER CONTROLLER"), this);
    m_userGroupBox->setGeometry(xg, yg, 240, 60);
    xg += 248;
    connect(m_userGroupBox, SIGNAL(enabledChanged()), this,
            SLOT(userToggled()));

    m_userController = new ComboBox(m_userGroupBox, "Controller");
    m_userController->setGeometry(10, 24, 200, 22);

    for(Controller* c: Engine::song()->controllers())
    {
        m_userController->model()->addItem(c->name());
    }

    // Mapping functions
    m_mappingBox = new TabWidget(tr("MAPPING FUNCTION"), this);
    m_mappingBox->setGeometry(xg, yg, 240, 64);
    xg = 8;
    yg += 68;
    m_mappingFunction = new QLineEdit(m_mappingBox);
    m_mappingFunction->setGeometry(10, 20, 170, 16);
    m_mappingFunction->setText("input");
    m_mappingFunction->setReadOnly(true);

    // Buttons
    QWidget* buttons = new QWidget(this);
    buttons->setGeometry(xg, yg, 488, 32);
    yg += 40;

    QHBoxLayout* btn_layout = new QHBoxLayout(buttons);
    btn_layout->setSpacing(0);
    btn_layout->setMargin(0);

    QPushButton* select_btn
            = new QPushButton(embed::getIconPixmap("add"), tr("OK"), buttons);
    connect(select_btn, SIGNAL(clicked()), this, SLOT(selectController()));

    QPushButton* cancel_btn = new QPushButton(embed::getIconPixmap("cancel"),
                                              tr("Cancel"), buttons);
    connect(cancel_btn, SIGNAL(clicked()), this, SLOT(reject()));

    btn_layout->addStretch();
    btn_layout->addWidget(select_btn);
    btn_layout->addSpacing(8);
    btn_layout->addWidget(cancel_btn);

    setFixedSize(504, yg);

    // Crazy MIDI View stuff

    // TODO, handle by making this a model for the Dialog "view"
    ControllerConnection* cc = nullptr;
    if(m_targetModel)
    {
        cc = m_targetModel->controllerConnection();

        if(cc != nullptr
           && cc->getController()->type() != Controller::DummyController
           && Engine::song() != nullptr)
        {
            if(cc->getController()->type() == Controller::MidiController)
            {
                m_midiGroupBox->setEnabled(true);
                // ensure controller is created
                midiToggled();

                MidiController* cont = (MidiController*)(cc->getController());
                m_midiChannelSpinBox->model()->setValue(
                        cont->m_midiPort.inputChannel());
                m_midiControllerSpinBox->model()->setValue(
                        cont->m_midiPort.inputController());

                m_midiWidgetTypeComboBox->model()->setValue(
                        cont->m_midiPort.widgetType());
                m_midiMinValueSpinBox->model()->setValue(
                        cont->m_midiPort.minInputValue());
                m_midiMaxValueSpinBox->model()->setValue(
                        cont->m_midiPort.maxInputValue());
                m_midiStepValueSpinBox->model()->setValue(
                        cont->m_midiPort.stepInputValue());
                m_midiBaseValueSpinBox->model()->setValue(
                        cont->m_midiPort.baseInputValue());
                m_midiSlopeValueSpinBox->model()->setValue(
                        cont->m_midiPort.slopeInputValue());
                m_midiDeltaValueSpinBox->model()->setValue(
                        cont->m_midiPort.deltaInputValue());

                // copy ports
                m_midiController->m_midiPort.subscribeReadablePorts(
                        static_cast<MidiController*>(cc->getController())
                                ->m_midiPort.readablePorts());
            }
            else
            {
                int idx = Engine::song()->controllers().indexOf(
                        cc->getController());

                if(idx >= 0)
                {
                    m_userGroupBox->setEnabled(true);
                    m_userController->model()->setValue(idx);
                }
            }
        }
    }

    if(!cc)
    {
        m_midiGroupBox->setEnabled(true);
    }

    show();
}

ControllerConnectionDialog::~ControllerConnectionDialog()
{
    delete m_readablePorts;

    delete m_midiController;
}

void ControllerConnectionDialog::selectController()
{
    // Midi
    if(m_midiGroupBox->isEnabled())
    {
        if((m_midiControllerSpinBox->model()->value() > 0)
           || (m_midiController->m_midiPort.widgetType() == 6))
        {
            // qInfo("ControllerConnectionDialog::selectController
            // m_controller=%p",m_controller);

            MidiController* mc;
            mc = m_midiController->copyToMidiController(Engine::song());

            /*
            if( m_targetModel->track() &&
                            !m_targetModel->track()->displayName().isEmpty() )
            {
                    mc->m_midiPort.setName( QString( "%1 (%2)" ).
                                    arg( m_targetModel->track()->displayName()
            ). arg( m_targetModel->displayName() ) );
            }
            else
            {
                    mc->m_midiPort.setName( m_targetModel->displayName() );
            }
            */
            mc->m_midiPort.setName(m_targetModel->fullDisplayName());
            m_controller = mc;
        }
    }
    // User
    else
    {
        if(m_userGroupBox->isEnabled() > 0
           && Engine::song()->controllers().size())
        {
            m_controller = Engine::song()->controllers().at(
                    m_userController->model()->value());
        }

        if(m_controller && m_controller->hasModel(m_targetModel))
        {
            QMessageBox::warning(this, tr("LMMS"), tr("Cycle Detected."));
            return;
        }
    }

    accept();
}

void ControllerConnectionDialog::midiToggled()
{
    // qInfo("ControllerConnectionDialog::midiToggled
    // %d",m_midiGroupBox->isEnabled());
    bool enabled = m_midiGroupBox->isEnabled();
    if(enabled)
    {
        if(m_userGroupBox->isEnabled())
            m_userGroupBox->setEnabled(false);

        if(m_midiController == nullptr)
        {
            m_midiController = new AutoDetectMidiController(Engine::song());

            MidiPort::Map map = m_midiController->m_midiPort.readablePorts();
            for(MidiPort::Map::Iterator it = map.begin(); it != map.end();
                ++it)
                it.value() = true;
            m_midiController->m_midiPort.subscribeReadablePorts(map);

            m_midiChannelSpinBox->setModel(
                    &m_midiController->m_midiPort.m_inputChannelModel);
            m_midiControllerSpinBox->setModel(
                    &m_midiController->m_midiPort.m_inputControllerModel);

            m_midiWidgetTypeComboBox->setModel(
                    &m_midiController->m_midiPort.m_widgetTypeModel);
            m_midiMinValueSpinBox->setModel(
                    &m_midiController->m_midiPort.m_minInputValueModel);
            m_midiMaxValueSpinBox->setModel(
                    &m_midiController->m_midiPort.m_maxInputValueModel);
            m_midiStepValueSpinBox->setModel(
                    &m_midiController->m_midiPort.m_stepInputValueModel);
            m_midiBaseValueSpinBox->setModel(
                    &m_midiController->m_midiPort.m_baseInputValueModel);
            m_midiSlopeValueSpinBox->setModel(
                    &m_midiController->m_midiPort.m_slopeInputValueModel);
            m_midiDeltaValueSpinBox->setModel(
                    &m_midiController->m_midiPort.m_deltaInputValueModel);

            if(m_readablePorts)
                m_readablePorts->setModel(&m_midiController->m_midiPort);

            connect(m_midiController, SIGNAL(controlledValueChanged(real_t)),
                    this, SLOT(midiValueChanged()));
            // connect( m_midiController, SIGNAL( dataChanged() ), this, SLOT(
            // midiValueChanged() ) );
        }
    }
    m_midiAutoDetect.setValue(enabled);

    m_midiChannelSpinBox->setEnabled(enabled);
    m_midiControllerSpinBox->setEnabled(enabled);
    m_midiAutoDetectCheckBox->setEnabled(enabled);

    m_midiWidgetTypeComboBox->setEnabled(enabled);
    m_midiMinValueSpinBox->setEnabled(enabled);
    m_midiMaxValueSpinBox->setEnabled(enabled);
    m_midiStepValueSpinBox->setEnabled(enabled);
    m_midiBaseValueSpinBox->setEnabled(enabled);
    m_midiSlopeValueSpinBox->setEnabled(enabled);
    m_midiDeltaValueSpinBox->setEnabled(enabled);
}

void ControllerConnectionDialog::userToggled()
{
    // qInfo("ControllerConnectionDialog::userToggled
    // %d",m_userGroupBox->isEnabled());
    bool enabled = m_userGroupBox->isEnabled();
    if(enabled && m_midiGroupBox->isEnabled())
        m_midiGroupBox->setEnabled(false);
    // m_userController->setEnabled( enabled );
}

void ControllerConnectionDialog::autoDetectToggled()
{
    if(m_midiAutoDetect.value())
    {
        m_midiController->reset();
    }
}

void ControllerConnectionDialog::midiValueChanged()
{
    if(m_midiAutoDetect.value())
    {
        m_midiController->useDetected();
        if(m_readablePorts)
        {
            m_readablePorts->updateMenu();
        }
    }
}

void ControllerConnectionDialog::enableAutoDetect(QAction* _a)
{
    if(_a->isChecked())
    {
        m_midiAutoDetectCheckBox->model()->setValue(true);
    }
}
