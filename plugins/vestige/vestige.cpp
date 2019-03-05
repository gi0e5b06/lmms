/*
 * vestige.cpp - instrument-plugin for hosting VST-instruments
 *
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "vestige.h"

#include "BufferManager.h"
#include "ConfigManager.h"
#include "Engine.h"
#include "FileDialog.h"
#include "GuiApplication.h"
#include "InstrumentPlayHandle.h"
#include "InstrumentTrack.h"
#include "MainWindow.h"
#include "Mixer.h"
#include "PixmapButton.h"
#include "SampleBuffer.h"
#include "Song.h"
#include "StringPairDrag.h"
#include "TextFloat.h"
#include "ToolTip.h"
#include "VstPlugin.h"
#include "embed.h"
#include "gui_templates.h"

#include <QDomElement>
#include <QDropEvent>
#include <QMdiArea>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>

extern "C"
{

    Plugin::Descriptor PLUGIN_EXPORT vestige_plugin_descriptor
            = {STRINGIFY(PLUGIN_NAME),
               "VeSTige",
               QT_TRANSLATE_NOOP(
                       "pluginBrowser",
                       "VST-host for using VST(i)-plugins within LMMS"),
               "Tobias Doerffel <tobydox/at/users.sf.net>",
               0x0100,
               Plugin::Instrument,
               new PluginPixmapLoader("logo"),
               "dll",
               nullptr};
}

QPixmap* VestigeInstrumentView::s_artwork       = nullptr;
QPixmap* manageVestigeInstrumentView::s_artwork = nullptr;

vestigeInstrument::vestigeInstrument(InstrumentTrack* _instrumentTrack) :
      Instrument(_instrumentTrack, &vestige_plugin_descriptor),
      m_plugin(nullptr), m_pluginMutex(), m_controls(nullptr),
      vstKnobs(nullptr), knobFModel(nullptr)  //,
                                              // p_subWindow( nullptr )
{
    // now we need a play-handle which cares for calling play()
    InstrumentPlayHandle* iph
            = new InstrumentPlayHandle(this, _instrumentTrack);
    Engine::mixer()->emit playHandleToAdd(iph);
}

vestigeInstrument::~vestigeInstrument()
{
    /*
    if (p_subWindow != nullptr) {
            delete p_subWindow;
            p_subWindow = nullptr;
    }
    */

    if(knobFModel != nullptr)
    {
        delete[] knobFModel;
        knobFModel = nullptr;
    }

    closePlugin();
}

void vestigeInstrument::loadSettings(const QDomElement& _this)
{
    loadFile(_this.attribute("plugin"));
    // m_pluginMutex.lock();
    if(!m_pluginMutex.tryLock())
        return;

    if(m_plugin != nullptr)
    {
        m_plugin->loadSettings(_this);

        const QMap<QString, QString>& dump = m_plugin->parameterDump();
        paramCount                         = dump.size();
        char paramStr[35];
        vstKnobs   = new Knob*[paramCount];
        knobFModel = new FloatModel*[paramCount];
        QStringList s_dumpValues;
        QWidget*    widget = new QWidget();
        for(int i = 0; i < paramCount; i++)
        {
            sprintf(paramStr, "param%d", i);
            s_dumpValues = dump[paramStr].split(":");

            vstKnobs[i] = new Knob(knobBright_26, widget, s_dumpValues.at(1));
            vstKnobs[i]->setHintText(s_dumpValues.at(1) + ":", "");
            vstKnobs[i]->setLabel(s_dumpValues.at(1).left(15));

            knobFModel[i] = new FloatModel(0.0f, 0.0f, 1.0f, 0.01f, this,
                                           QString::number(i));
            knobFModel[i]->loadSettings(_this, paramStr);

            if(!(knobFModel[i]->isAutomated()
                 || knobFModel[i]->controllerConnection()))
            {
                knobFModel[i]->setValue((s_dumpValues.at(2)).toFloat());
                knobFModel[i]->setInitValue((s_dumpValues.at(2)).toFloat());
            }

            connect(knobFModel[i], SIGNAL(dataChanged()), this,
                    SLOT(setParameter()));

            vstKnobs[i]->setModel(knobFModel[i]);
        }
    }
    m_pluginMutex.unlock();
}

void vestigeInstrument::setParameter(void)
{

    Model* action   = qobject_cast<Model*>(sender());
    int    knobUNID = action->displayName().toInt();

    if(m_plugin != nullptr)
    {
        m_plugin->setParam(knobUNID, knobFModel[knobUNID]->value());
    }
}

void vestigeInstrument::saveSettings(QDomDocument& _doc, QDomElement& _this)
{
    _this.setAttribute("plugin", m_pluginDLL);
    m_pluginMutex.lock();
    if(m_plugin != nullptr)
    {
        m_plugin->saveSettings(_doc, _this);
        if(knobFModel != nullptr)
        {
            const QMap<QString, QString>& dump = m_plugin->parameterDump();
            paramCount                         = dump.size();
            char paramStr[35];
            for(int i = 0; i < paramCount; i++)
            {
                if(knobFModel[i]->isAutomated()
                   || knobFModel[i]->controllerConnection())
                {
                    sprintf(paramStr, "param%d", i);
                    knobFModel[i]->saveSettings(_doc, _this, paramStr);
                }

                /*				QDomElement me =
                   _doc.createElement( paramStr ); me.setAttribute( "id",
                   knobFModel[i]->id() ); me.setAttribute( "value",
                   knobFModel[i]->value() ); _this.appendChild( me );

                                                ControllerConnection *
                   m_controllerConnection =
                   knobFModel[i]->controllerConnection(); if
                   (m_controllerConnection) { QDomElement controller_element;
                                                        QDomNode node =
                   _this.namedItem( "connection" ); if( node.isElement() )
                                                        {
                                                                controller_element
                   = node.toElement();
                                                        }
                                                        else
                                                        {
                                                                controller_element
                   = _doc.createElement( "connection" ); _this.appendChild(
                   controller_element );
                                                        }
                                                        QDomElement element =
                   _doc.createElement( paramStr );
                                                        m_controllerConnection->saveSettings(
                   _doc, element ); controller_element.appendChild( element );
                                                }*/
            }
        }
    }
    m_pluginMutex.unlock();
}

/*
QString vestigeInstrument::nodeName(void) const
{
    return vestige_plugin_descriptor.name;
}
*/

void vestigeInstrument::loadFile(const QString& _file)
{
    m_pluginMutex.lock();
    const bool set_ch_name
            = (m_plugin != nullptr
               && instrumentTrack()->name() == m_plugin->name())
              || instrumentTrack()->name()
                         == InstrumentTrack::tr("Default preset")
              || instrumentTrack()->name() == displayName();

    m_pluginMutex.unlock();

    if(m_plugin != nullptr)
    {
        closePlugin();
    }
    m_pluginDLL   = SampleBuffer::tryToMakeRelative(_file);
    TextFloat* tf = TextFloat::displayMessage(
            tr("Loading plugin"),
            tr("Please wait while loading VST-plugin..."),
            PLUGIN_NAME::getIconPixmap("logo", 24, 24), 0);

    m_pluginMutex.lock();
    m_plugin = new VstPlugin(m_pluginDLL);
    if(m_plugin->failed())
    {
        m_pluginMutex.unlock();
        closePlugin();
        delete tf;
        collectErrorForUI(
                VstPlugin::tr("The VST plugin %1 could not be loaded.")
                        .arg(m_pluginDLL));
        m_pluginDLL = "";
        return;
    }

    m_plugin->showEditor(false);  // nullptr, false );

    if(set_ch_name)
    {
        instrumentTrack()->setName(m_plugin->name());
    }

    m_pluginMutex.unlock();

    emit dataChanged();

    delete tf;
}

void vestigeInstrument::play(sampleFrame* _buf)
{
    // m_pluginMutex.lock();
    if(!m_pluginMutex.tryLock(Engine::getSong()->isExporting() ? -1 : 0))
        return;

    if(m_plugin == nullptr)
    {
        m_pluginMutex.unlock();
        return;
    }

    m_plugin->process(nullptr, _buf);

    const fpp_t frames = Engine::mixer()->framesPerPeriod();

    instrumentTrack()->processAudioBuffer(_buf, frames, nullptr);

    m_pluginMutex.unlock();
}

bool vestigeInstrument::handleMidiEvent(const MidiEvent& event,
                                        const MidiTime&  time,
                                        f_cnt_t          offset)
{
    if(!m_pluginMutex.tryLock(Engine::getSong()->isExporting() ? -1 : 0))
        return true;

    if(m_plugin != nullptr)
        m_plugin->processMidiEvent(event, offset);

    m_pluginMutex.unlock();

    return true;
}

void vestigeInstrument::closePlugin(void)
{
    // disconnect all signals
    if(knobFModel != nullptr)
    {
        for(int i = 0; i < paramCount; i++)
        {
            delete knobFModel[i];
            delete vstKnobs[i];
        }
    }

    if(vstKnobs != nullptr)
    {
        delete[] vstKnobs;
        vstKnobs = nullptr;
    }

    if(knobFModel != nullptr)
    {
        delete[] knobFModel;
        knobFModel = nullptr;
    }

    /*
    if( m_scrollArea != nullptr )
    {
    //delete m_scrollArea;
            m_scrollArea = nullptr;
    }
    */

    if(m_controls != nullptr)
    {
        m_controls->setAttribute(Qt::WA_DeleteOnClose);
        m_controls->close();

        if(m_controls != nullptr)
        {
            delete m_controls;
        }
        m_controls = nullptr;
    }

    /*
    if( p_subWindow != nullptr )
    {
            p_subWindow = nullptr;
    }
    */

    m_pluginMutex.lock();
    if(m_plugin)
    {
        delete m_plugin->pluginWidget();
    }
    delete m_plugin;
    m_plugin = nullptr;
    m_pluginMutex.unlock();
}

PluginView* vestigeInstrument::instantiateView(QWidget* _parent)
{
    return new VestigeInstrumentView(this, _parent);
}

VestigeInstrumentView::VestigeInstrumentView(Instrument* _instrument,
                                             QWidget*    _parent) :
      InstrumentView(_instrument, _parent),
      lastPosInMenu(0)
{
    if(s_artwork == nullptr)
    {
        s_artwork = new QPixmap(PLUGIN_NAME::getIconPixmap("artwork"));
    }

    m_rolLPresetButton = new PixmapButton(this, "[previous preset]");
    m_rolLPresetButton->setCheckable(false);
    m_rolLPresetButton->setCursor(Qt::PointingHandCursor);
    m_rolLPresetButton->move(155, 201);
    m_rolLPresetButton->setActiveGraphic(
            embed::getIconPixmap("stepper-left-press", 16, 16));
    m_rolLPresetButton->setInactiveGraphic(
            embed::getIconPixmap("stepper-left", 16, 16));
    connect(m_rolLPresetButton, SIGNAL(clicked()), this,
            SLOT(previousProgram()));
    ToolTip::add(m_rolLPresetButton, tr("Previous (-)"));

    m_rolLPresetButton->setShortcut(Qt::Key_Minus);

    m_rolLPresetButton->setWhatsThis(
            tr("Click here, if you want to switch to another VST-plugin "
               "preset program."));

    m_rolRPresetButton = new PixmapButton(this, "[next preset]");
    m_rolRPresetButton->setCheckable(false);
    m_rolRPresetButton->setCursor(Qt::PointingHandCursor);
    m_rolRPresetButton->move(173, 201);
    m_rolRPresetButton->setActiveGraphic(
            embed::getIconPixmap("stepper-right-press", 16, 16));
    m_rolRPresetButton->setInactiveGraphic(
            embed::getIconPixmap("stepper-right", 16, 16));
    connect(m_rolRPresetButton, SIGNAL(clicked()), this, SLOT(nextProgram()));
    ToolTip::add(m_rolRPresetButton, tr("Next (+)"));
    m_rolRPresetButton->setShortcut(Qt::Key_Plus);
    m_rolRPresetButton->setWhatsThis(
            tr("Click here, if you want to switch to another VST-plugin "
               "preset program."));

    m_selPresetButton = new QPushButton(tr(""), this);
    m_selPresetButton->setGeometry(191, 201, 16, 16);
    m_selPresetButton->setIcon(
            embed::getIconPixmap("stepper-down"));  // need PixmapButton
    m_selPresetButton->setWhatsThis(
            tr("Click here to select presets that are currently loaded in "
               "VST."));
    QMenu* menu = new QMenu();
    connect(menu, SIGNAL(aboutToShow()), this, SLOT(updateMenu()));
    m_selPresetButton->setMenu(menu);

    m_openPresetButton = new PixmapButton(this, "[open preset]");
    m_openPresetButton->setCheckable(false);
    m_openPresetButton->setCursor(Qt::PointingHandCursor);
    m_openPresetButton->move(209, 201);
    m_openPresetButton->setActiveGraphic(
            embed::getIconPixmap("project_open", 16, 16));  // 20
    m_openPresetButton->setInactiveGraphic(
            embed::getIconPixmap("project_open", 16, 16));
    connect(m_openPresetButton, SIGNAL(clicked()), this, SLOT(openPreset()));
    ToolTip::add(m_openPresetButton, tr("Open VST-plugin preset"));
    m_openPresetButton->setWhatsThis(
            tr("Click here, if you want to open another *.fxp, *.fxb "
               "VST-plugin preset."));

    m_savePresetButton = new PixmapButton(this, "[save preset]");
    m_savePresetButton->setCheckable(false);
    m_savePresetButton->setCursor(Qt::PointingHandCursor);
    m_savePresetButton->move(227, 201);
    m_savePresetButton->setActiveGraphic(
            embed::getIconPixmap("project_save", 16, 16));  // 20
    m_savePresetButton->setInactiveGraphic(
            embed::getIconPixmap("project_save", 16, 16));
    connect(m_savePresetButton, SIGNAL(clicked()), this, SLOT(savePreset()));
    ToolTip::add(m_savePresetButton, tr("Save preset"));
    m_savePresetButton->setWhatsThis(
            tr("Click here, if you want to save current VST-plugin preset "
               "program."));

    const QString CSS_BUTTON = ("text-align: left; padding-left: 3px;");

    QPushButton* m_toggleGUIButton
            = new QPushButton(tr("Show/hide GUI"), this);
    m_toggleGUIButton->setGeometry(6, 130, 118, 28);
    m_toggleGUIButton->setIcon(embed::getIconPixmap("zoom"));
    m_toggleGUIButton->setFont(pointSize<8>(m_toggleGUIButton->font()));
    m_toggleGUIButton->setStyleSheet(CSS_BUTTON);
    connect(m_toggleGUIButton, SIGNAL(clicked()), this, SLOT(toggleGUI()));
    m_toggleGUIButton->setWhatsThis(
            tr("Click here to show or hide the graphical user interface "
               "(GUI) of your VST-plugin."));

    QPushButton* note_off_all_btn
            = new QPushButton(tr("Turn off all notes"), this);
    note_off_all_btn->setGeometry(6, 162, 118, 28);
    note_off_all_btn->setIcon(embed::getIconPixmap("stop"));
    note_off_all_btn->setFont(pointSize<8>(note_off_all_btn->font()));
    note_off_all_btn->setStyleSheet(CSS_BUTTON);
    connect(note_off_all_btn, SIGNAL(clicked()), this, SLOT(noteOffAll()));

    QPushButton* m_openPluginButton
            = new QPushButton(tr("Open other VST"), this);  // VST-plugin
    m_openPluginButton->setGeometry(128, 130, 118, 28);
    m_openPluginButton->setIcon(PLUGIN_NAME::getIconPixmap("select_file"));
    m_openPluginButton->setFont(pointSize<8>(m_openPluginButton->font()));
    m_openPluginButton->setStyleSheet(CSS_BUTTON);
    connect(m_openPluginButton, SIGNAL(clicked()), this, SLOT(openPlugin()));

    /*
    m_openPluginButton = new PixmapButton( this, "" );
    m_openPluginButton->setCheckable( false );
    m_openPluginButton->setCursor( Qt::PointingHandCursor );
    m_openPluginButton->move( 216, 83 );
    m_openPluginButton->setActiveGraphic
    (PLUGIN_NAME::getIconPixmap("select_file_active",16,16));
    m_openPluginButton->setInactiveGraphic(PLUGIN_NAME::getIconPixmap("select_file",16,16));
    connect( m_openPluginButton, SIGNAL( clicked() ),
             this, SLOT( openPlugin() ) );
    ToolTip::add( m_openPluginButton, tr( "Open other VST-plugin" ) );
    m_openPluginButton->setWhatsThis
            ( tr( "Click here, if you want to open another VST-plugin. After "
                  "clicking on this button, a file-open-dialog appears "
                  "and you can select your file." ) );
    */

    QPushButton* m_managePluginButton
            = new QPushButton(tr("Host controls"), this);
    m_managePluginButton->setGeometry(128, 162, 118, 28);
    m_managePluginButton->setIcon(PLUGIN_NAME::getIconPixmap("controls"));
    m_managePluginButton->setFont(pointSize<8>(m_managePluginButton->font()));
    m_managePluginButton->setStyleSheet(CSS_BUTTON);
    connect(m_managePluginButton, SIGNAL(clicked()), this,
            SLOT(toggleControls()));

    /*
    m_managePluginButton = new PixmapButton( this, "" );
    m_managePluginButton->setCheckable( false );
    m_managePluginButton->setCursor( Qt::PointingHandCursor );
    m_managePluginButton->move( 216, 101 );
    m_managePluginButton->setActiveGraphic
    (PLUGIN_NAME::getIconPixmap("controls_active",16,16));
    m_managePluginButton->setInactiveGraphic(PLUGIN_NAME::getIconPixmap("controls",16,16));
    connect( m_managePluginButton, SIGNAL( clicked() ),
             this, SLOT( toggleControls() ) );
    ToolTip::add( m_managePluginButton, tr( "Control VST-plugin from LMMS
    host" ) ); m_managePluginButton->setWhatsThis ( tr( "Click here, if you
    want to control VST-plugin from host." ) );
    */

    setAcceptDrops(true);
    m_instrument2 = _instrument;
    m_parent2     = _parent;
}

void VestigeInstrumentView::toggleControls()
{
    /*
    if ( m_vi->m_plugin != nullptr && m_vi->m_subWindow == nullptr ) {
            m_vi->p_subWindow = new manageVestigeInstrumentView(
    m_instrument2, m_parent2, m_vi); } else if (m_vi->m_subWindow != nullptr)
    { if (m_vi->m_subWindow->widget()->isVisible() == false ) {
                    m_vi->m_scrollArea->show();
                    m_vi->m_subWindow->show();
            } else {
                    m_vi->m_scrollArea->hide();
                    m_vi->m_subWindow->hide();
            }
    }
    */
    /*
    if(m_vi->p_subWindow == nullptr )
    {
            m_vi->p_subWindow = new manageVestigeInstrumentView(
    m_instrument2, m_parent2, m_vi);
    }
    */

    if(m_vi->m_controls == nullptr)
    {
        // qWarning("VestigeInstrumentView::toggleControls m_controls is
        // null");
        // creates m_controls
        new manageVestigeInstrumentView(m_instrument2, m_parent2, m_vi);
        m_vi->m_controls->show();
        m_vi->m_controls->raise();
    }
    else if(m_vi->m_controls->isHidden())
    {
        // qWarning("VestigeInstrumentView::toggleControls show");
        m_vi->m_controls->show();
        m_vi->m_controls->raise();
    }
    else
    {
        // qWarning("VestigeInstrumentView::toggleControls hide");
        m_vi->m_controls->hide();
    }
}

void VestigeInstrumentView::updateMenu(void)
{

    // get all presets -
    if(m_vi->m_plugin != nullptr)
    {
        m_vi->m_plugin->loadProgramNames();
        QWidget::update();

        QString str = m_vi->m_plugin->allProgramNames();

        QStringList list1 = str.split("|");

        QMenu* to_menu = m_selPresetButton->menu();
        to_menu->clear();

        QAction* presetActions[list1.size()];

        for(int i = 0; i < list1.size(); i++)
        {
            presetActions[i] = new QAction(this);
            connect(presetActions[i], SIGNAL(triggered()), this,
                    SLOT(selPreset()));

            presetActions[i]->setText(QString("%1. %2").arg(
                    QString::number(i + 1), list1.at(i)));
            presetActions[i]->setData(i);
            if(i == lastPosInMenu)
            {
                presetActions[i]->setIcon(
                        embed::getIconPixmap("sample_file", 16, 16));
            }
            else
                presetActions[i]->setIcon(
                        embed::getIconPixmap("edit_copy", 16, 16));
            to_menu->addAction(presetActions[i]);
        }
    }
}

VestigeInstrumentView::~VestigeInstrumentView()
{
}

void VestigeInstrumentView::modelChanged()
{
    m_vi = castModel<vestigeInstrument>();
}

void VestigeInstrumentView::openPlugin()
{
    FileDialog ofd(nullptr, tr("Open VST-plugin"));

    // set filters
    QStringList types;
    types << tr("DLL-files (*.dll)") << tr("EXE-files (*.exe)")
            /*<< tr("SO-files (*.so)")*/;
    ofd.setNameFilters(types);

    if(m_vi->m_pluginDLL != "")
    {
        QString f = SampleBuffer::tryToMakeAbsolute(m_vi->m_pluginDLL);
        ofd.setDirectory(QFileInfo(f).absolutePath());
        ofd.selectFile(QFileInfo(f).fileName());
    }
    else
    {
        ofd.setDirectory(ConfigManager::inst()->vstDir());
    }

    if(ofd.exec() == QDialog::Accepted)
    {
        if(ofd.selectedFiles().isEmpty())
        {
            return;
        }
        Engine::mixer()->requestChangeInModel();

        /*
        if (m_vi->p_subWindow != nullptr) {
                delete m_vi->p_subWindow;
                m_vi->p_subWindow = nullptr;
        }
        */

        m_vi->loadFile(ofd.selectedFiles()[0]);
        Engine::mixer()->doneChangeInModel();
        if(m_vi->m_plugin && m_vi->m_plugin->pluginWidget())
        {
            m_vi->m_plugin->pluginWidget()->setWindowIcon(
                    PLUGIN_NAME::getIconPixmap("logo"));
        }
    }
}

void VestigeInstrumentView::openPreset()
{

    if(m_vi->m_plugin != nullptr)
    {
        m_vi->m_plugin->openPreset();
        bool    converted;
        QString str = m_vi->m_plugin->currentProgramName().section("/", 0, 0);
        if(str != "")
            lastPosInMenu = str.toInt(&converted, 10) - 1;
        QWidget::update();
    }
}

void VestigeInstrumentView::savePreset()
{

    if(m_vi->m_plugin != nullptr)
    {
        m_vi->m_plugin->savePreset();
        /*    		bool converted;
                        QString str =
           m_vi->m_plugin->presetString().section("/", 0, 0); if (str != "")
                                lastPosInMenu = str.toInt(&converted, 10) - 1;
                        QWidget::update();*/
    }
}

void VestigeInstrumentView::nextProgram()
{

    if(m_vi->m_plugin != nullptr)
    {
        m_vi->m_plugin->rotateProgram(1);
        bool    converted;
        QString str = m_vi->m_plugin->currentProgramName().section("/", 0, 0);
        if(str != "")
            lastPosInMenu = str.toInt(&converted, 10) - 1;
        QWidget::update();
    }
}

void VestigeInstrumentView::previousProgram()
{

    if(m_vi->m_plugin != nullptr)
    {
        m_vi->m_plugin->rotateProgram(-1);
        bool    converted;
        QString str = m_vi->m_plugin->currentProgramName().section("/", 0, 0);
        if(str != "")
            lastPosInMenu = str.toInt(&converted, 10) - 1;
        QWidget::update();
    }
}

void VestigeInstrumentView::selPreset(void)
{

    QAction* action = qobject_cast<QAction*>(sender());
    if(action)
        if(m_vi->m_plugin != nullptr)
        {
            lastPosInMenu = action->data().toInt();
            m_vi->m_plugin->setProgram(action->data().toInt());
            QWidget::update();
        }
}

void VestigeInstrumentView::toggleGUI()
{
    if(m_vi == nullptr || m_vi->m_plugin == nullptr)
    {
        qWarning(
                "VestigeInstrumentView::toggleGUI m_vi == nullptr || "
                "m_vi->m_plugin == nullptr");
        return;
    }

    QMdiSubWindow* sw = m_vi->m_plugin->mdiSubWindow();
    // qWarning("VestigeInstrumentView::toggleGUI sw=%p",sw);

    if(sw != nullptr)
    {
        if(sw->isHidden())
        {
            sw->show();
            sw->raise();
        }
        else
            sw->hide();
    }
    else
    {
        qWarning("VestigeInstrumentView::toggleGUI sw is nullptr");
        /*
        QWidget * pw = m_vi->m_plugin->pluginWidget();
        if( pw == nullptr ) return;
        if ( pw->isHidden() ) pw->show();
        else pw->hide();
        */
    }
}

void VestigeInstrumentView::noteOffAll(void)
{
    if(m_vi->m_plugin != nullptr)
    {
        for(int key = 0; key <= MidiMaxKey; ++key)
        {
            if(!m_vi->m_pluginMutex.tryLock(
                       Engine::getSong()->isExporting() ? -1 : 0))
                return;

            m_vi->m_plugin->processMidiEvent(
                    MidiEvent(MidiNoteOff, 0, key, 0), 0);

            m_vi->m_pluginMutex.unlock();
        }
    }
}

void VestigeInstrumentView::dragEnterEvent(QDragEnterEvent* _dee)
{
    if(_dee->mimeData()->hasFormat(StringPairDrag::mimeType()))
    {
        QString txt = _dee->mimeData()->data(StringPairDrag::mimeType());
        if(txt.section(':', 0, 0) == "vstplugin")
        {
            _dee->acceptProposedAction();
        }
        else
        {
            _dee->ignore();
        }
    }
    else
    {
        _dee->ignore();
    }
}

void VestigeInstrumentView::dropEvent(QDropEvent* _de)
{
    QString type  = StringPairDrag::decodeKey(_de);
    QString value = StringPairDrag::decodeValue(_de);
    if(type == "vstplugin")
    {
        m_vi->loadFile(value);
        _de->accept();
        return;
    }
    _de->ignore();
}

void VestigeInstrumentView::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    p.drawPixmap(0, 0, *s_artwork);

    QString plugin_name
            = (m_vi->m_plugin != nullptr)
                      ? m_vi->m_plugin->name() /* + QString::number(
                                         m_plugin->version() )*/
                      : tr("No VST-plugin loaded");
    QFont f = p.font();
    f.setBold(true);
    p.setFont(pointSize<10>(f));
    p.setPen(QColor(255, 255, 255));
    p.drawText(12, 100, plugin_name);

    p.setPen(QColor(50, 50, 50));
    p.drawText(12, 215, tr("Preset"));

    //	m_pluginMutex.lock();
    if(m_vi->m_plugin != nullptr)
    {
        p.setPen(QColor(0, 0, 0));
        f.setBold(false);
        p.setFont(pointSize<8>(f));
        p.drawText(12, 114,  // tr( "by " ) +
                   m_vi->m_plugin->vendorString());
        p.setPen(QColor(255, 255, 255));
        p.drawText(12, 229, m_vi->m_plugin->currentProgramName());
    }

    /*
    if( m_vi->m_controls != nullptr )
    {
            m_vi->m_controls->setWindowTitle( m_vi->instrumentTrack()->name()
                                              + tr( " - VST plugin control" )
    );
    }
    */
    //	m_pluginMutex.unlock();
}

// Controls (knobs) Window

manageVestigeInstrumentView::manageVestigeInstrumentView(
        Instrument* _instrument, QWidget* _parent, vestigeInstrument* _vi) :
      InstrumentView(_instrument, _parent),
      m_vi(_vi)
{
    widget = new QWidget(this);
    widget->setAutoFillBackground(true);

    l = new QGridLayout(widget);
    l->setContentsMargins(6, 6, 6, 6);  //( 20, 10, 10, 10 );
    l->setVerticalSpacing(6);           // 10 );
    l->setHorizontalSpacing(6);         // 23 );

    // Sync

    m_syncButton = new QPushButton(tr("VST Sync"), this);
    m_syncButton->setSizePolicy(QSizePolicy::MinimumExpanding,
                                QSizePolicy::MinimumExpanding);
    connect(m_syncButton, SIGNAL(clicked()), this, SLOT(syncPlugin()));
    m_syncButton->setWhatsThis(
            tr("Click here if you want to synchronize all parameters with "
               "VST plugin."));
    l->addWidget(m_syncButton, 0, 0, 1, 2, Qt::AlignLeft);

    // Automated

    m_displayAutomatedOnly = new QPushButton(tr("Automated"), this);
    m_displayAutomatedOnly->setSizePolicy(QSizePolicy::MinimumExpanding,
                                          QSizePolicy::MinimumExpanding);
    connect(m_displayAutomatedOnly, SIGNAL(clicked()), this,
            SLOT(displayAutomatedOnly()));
    m_displayAutomatedOnly->setWhatsThis(tr(
            "Click here if you want to display automated parameters only."));
    l->addWidget(m_displayAutomatedOnly, 0, 2, 1, 2, Qt::AlignLeft);

    /*
    // Close

    m_closeButton = new QPushButton( tr( "Close" ), widget );
    m_closeButton->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    connect( m_closeButton, SIGNAL( clicked() ), this, SLOT( closeWindow() )
    ); m_closeButton->setWhatsThis ( tr( "Close VST plugin knob-controller
    window." ) );
    //l->addWidget( m_closeButton, 0, 2, 1, 7, Qt::AlignLeft );
    l->addWidget( m_closeButton, 0, 4, 1, 2, Qt::AlignLeft );
    */

    /*
    for( int i = 0; i < 10; i++ )
    {
            l->addItem( new QSpacerItem( 68, 45, QSizePolicy::Fixed,
    QSizePolicy::Fixed ), 0, i );
    }
    */

    const QMap<QString, QString>& dump = m_vi->m_plugin->parameterDump();
    m_vi->paramCount                   = dump.size();

    bool isVstKnobs = true;

    if(m_vi->vstKnobs == nullptr)
    {
        m_vi->vstKnobs = new Knob*[m_vi->paramCount];
        isVstKnobs     = false;
    }
    if(m_vi->knobFModel == nullptr)
    {
        m_vi->knobFModel = new FloatModel*[m_vi->paramCount];
    }

    char        paramStr[35];
    QStringList s_dumpValues;

    if(isVstKnobs == false)
    {
        for(int i = 0; i < m_vi->paramCount; i++)
        {
            sprintf(paramStr, "param%d", i);
            s_dumpValues = dump[paramStr].split(":");

            m_vi->vstKnobs[i]
                    = new Knob(knobBright_26, this, s_dumpValues.at(1));
            m_vi->vstKnobs[i]->setHintText(s_dumpValues.at(1) + ":", "");
            m_vi->vstKnobs[i]->setLabel(s_dumpValues.at(1).left(15));

            sprintf(paramStr, "%d", i);
            m_vi->knobFModel[i] = new FloatModel(
                    (s_dumpValues.at(2)).toFloat(), 0.0f, 1.0f, 0.01f,
                    castModel<vestigeInstrument>(), tr(paramStr));
            connect(m_vi->knobFModel[i], SIGNAL(dataChanged()), this,
                    SLOT(setParameter()));
            m_vi->vstKnobs[i]->setModel(m_vi->knobFModel[i]);
        }
    }

    int i = 0;
    int lrow, lcolumn;
    for(lrow = 1; lrow < (int(m_vi->paramCount / 10) + 1) + 1; lrow++)
    {
        for(lcolumn = 0; lcolumn < 10; lcolumn++)
        {
            if(i < m_vi->paramCount)
            {
                l->addWidget(m_vi->vstKnobs[i], lrow, lcolumn,
                             Qt::AlignCenter);
            }
            i++;
        }
    }

    // l->setRowStretch( ( int( m_vi->paramCount / 10) + 1), 1 );
    // l->setColumnStretch( 10, 1 );
    // l->setSizeConstraint(QLayout::SetFixedSize);
    // widget->setLayout(l);

    QScrollArea* sa = new QScrollArea(this);
    sa->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    sa->setPalette(QApplication::palette(sa));
    // sa->setMinimumHeight( 84 );
    // sa->setMinimumWidth( 250 );
    sa->setGeometry(0, 0, 12 + 42 * lrow + 16, 6 + 32 * lcolumn);
    sa->setWidget(widget);

    /*
    m_vi->m_subWindow = new VstSubWindow(
    );//gui->mainWindow()->workspace()->viewport() );
    m_vi->m_subWindow->setWindowFlags( Qt::SubWindow | Qt::CustomizeWindowHint
    | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                                       Qt::WindowCloseButtonHint );
    m_vi->m_subWindow->setSizePolicy( QSizePolicy::Fixed,
    QSizePolicy::MinimumExpanding ); m_vi->m_subWindow->setMinimumWidth( 250
    ); m_vi->m_subWindow->setMinimumHeight( 84 );
    m_vi->m_subWindow->setWidget(sa);
    m_vi->m_subWindow->setWindowTitle( m_vi->instrumentTrack()->name()
                                                            + tr( " - VST
    plugin control" ) ); m_vi->m_subWindow->setWindowIcon(
    PLUGIN_NAME::getIconPixmap( "logo" ) ); m_vi->m_subWindow->setAttribute(
    Qt::WA_DeleteOnClose, false );
    m_vi->m_subWindow->resize(m_vi->m_subWindow->sizeHint());
    */

    sa->setWindowTitle(m_vi->instrumentTrack()->name()
                       + tr(" - VST plugin control"));
    sa->setWindowIcon(PLUGIN_NAME::getIconPixmap("logo"));
    m_vi->m_controls
            = SubWindow::putWidgetOnWorkspace(sa, false, true, false, false);
}

void manageVestigeInstrumentView::closeWindow()
{
    qWarning("manageVestigeInstrumentView::closeWindow()");
    m_vi->m_controls->hide();
}

void manageVestigeInstrumentView::syncPlugin(void)
{
    char                          paramStr[35];
    QStringList                   s_dumpValues;
    const QMap<QString, QString>& dump = m_vi->m_plugin->parameterDump();
    float                         f_value;

    for(int i = 0; i < m_vi->paramCount; i++)
    {
        // only not automated knobs are synced from VST
        // those auto-setted values are not jurnaled, tracked for undo / redo
        if(!(m_vi->knobFModel[i]->isAutomated()
             || m_vi->knobFModel[i]->controllerConnection()))
        {
            sprintf(paramStr, "param%d", i);
            s_dumpValues = dump[paramStr].split(":");
            f_value      = (s_dumpValues.at(2)).toFloat();
            m_vi->knobFModel[i]->setAutomatedValue(f_value);
            m_vi->knobFModel[i]->setInitValue(f_value);
        }
    }
}

void manageVestigeInstrumentView::displayAutomatedOnly(void)
{
    bool isAuto = QString::compare(m_displayAutomatedOnly->text(),
                                   tr("Automated"))
                  == 0;

    for(int i = 0; i < m_vi->paramCount; i++)
    {

        if(!(m_vi->knobFModel[i]->isAutomated()
             || m_vi->knobFModel[i]->controllerConnection()))
        {
            if(m_vi->vstKnobs[i]->isVisible() == true && isAuto)
            {
                m_vi->vstKnobs[i]->hide();
                m_displayAutomatedOnly->setText("All");
            }
            else
            {
                m_vi->vstKnobs[i]->show();
                m_displayAutomatedOnly->setText("Automated");
            }
        }
    }
}

manageVestigeInstrumentView::~manageVestigeInstrumentView()
{
    if(m_vi->knobFModel != nullptr)
    {
        for(int i = 0; i < m_vi->paramCount; i++)
        {
            delete m_vi->knobFModel[i];
            delete m_vi->vstKnobs[i];
        }
    }

    if(m_vi->vstKnobs != nullptr)
    {
        delete[] m_vi->vstKnobs;
        m_vi->vstKnobs = nullptr;
    }

    if(m_vi->knobFModel != nullptr)
    {
        delete[] m_vi->knobFModel;
        m_vi->knobFModel = nullptr;
    }

    if(m_vi->m_controls != nullptr)
    {
        m_vi->m_controls->setAttribute(Qt::WA_DeleteOnClose);
        m_vi->m_controls->close();

        if(m_vi->m_controls != nullptr)
            delete m_vi->m_controls;
        m_vi->m_controls = nullptr;
    }

    // m_vi->p_subWindow = nullptr;
}

void manageVestigeInstrumentView::setParameter(void)
{

    Model* action   = qobject_cast<Model*>(sender());
    int    knobUNID = action->displayName().toInt();

    if(m_vi->m_plugin != nullptr)
    {
        m_vi->m_plugin->setParam(knobUNID,
                                 m_vi->knobFModel[knobUNID]->value());
    }
}

void manageVestigeInstrumentView::dragEnterEvent(QDragEnterEvent* _dee)
{
    if(_dee->mimeData()->hasFormat(StringPairDrag::mimeType()))
    {
        QString txt = _dee->mimeData()->data(StringPairDrag::mimeType());
        if(txt.section(':', 0, 0) == "vstplugin")
        {
            _dee->acceptProposedAction();
        }
        else
        {
            _dee->ignore();
        }
    }
    else
    {
        _dee->ignore();
    }
}

void manageVestigeInstrumentView::dropEvent(QDropEvent* _de)
{
    QString type  = StringPairDrag::decodeKey(_de);
    QString value = StringPairDrag::decodeValue(_de);
    if(type == "vstplugin")
    {
        m_vi->loadFile(value);
        _de->accept();
        return;
    }
    _de->ignore();
}

void manageVestigeInstrumentView::paintEvent(QPaintEvent*)
{
    m_vi->m_controls->setWindowTitle(m_vi->instrumentTrack()->name()
                                     + tr(" - VST plugin control"));
}

extern "C"
{

    // necessary for getting instance out of shared lib
    Plugin* PLUGIN_EXPORT lmms_plugin_main(Model*, void* _data)
    {
        return new vestigeInstrument(static_cast<InstrumentTrack*>(_data));
    }
}
