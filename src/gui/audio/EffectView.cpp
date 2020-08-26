/*
 * EffectView.cpp - view-component for an effect
 *
 * Copyright (c) 2018-2019 gi0e5b06 (on github.com)
 * Copyright (c) 2006-2007 Danny McRae <khjklujn/at/users.sourceforge.net>
 * Copyright (c) 2007-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "EffectView.h"

#include "CaptionMenu.h"
#include "Clipboard.h"
#include "DummyEffect.h"
#include "GuiApplication.h"
#include "Knob.h"
#include "LedCheckBox.h"
#include "MainWindow.h"
#include "TempoSyncKnob.h"
#include "ToolTip.h"
#include "embed.h"
#include "gui_templates.h"
#include "lmms_qt_gui.h"

//#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>
//#include <QMdiArea>
//#include <QMdiSubWindow>
#include <QPainter>
#include <QWhatsThis>

// const int EFFECT_WIDTH  = 228;
// const int EFFECT_HEIGHT = 60;

/*! The width of the resize grip in pixels
 */
const int RESIZE_GRIP_WIDTH = 4;

EffectView::EffectView(Effect* _model, QWidget* _parent) :
      PluginView(_model, _parent),
      m_bg(embed::getIconPixmap("effect_plugin")), m_subWindow(nullptr),
      m_controlView(nullptr)
{
    setFixedSize(m_bg.width(), m_bg.height());

    // Disable effects that are of type "DummyEffect"
    bool isEnabled = !dynamic_cast<DummyEffect*>(model());
    m_enabledLCB   = new LedCheckBox(this, "Enabled", LedCheckBox::Green);
    m_enabledLCB->move(19 + 3, 3);
    // m_enabledLCB->setGeometry(0,0,19,19);
    // m_enabledLCB->setText(tr("ON"));
    // m_enabledLCB->setTextAnchorPoint(Qt::AnchorBottom);
    m_enabledLCB->setEnabled(isEnabled);
    m_enabledLCB->setBlinking(!isEnabled);
    m_enabledLCB->setWhatsThis(tr("Toggles the effect on or off."));
    ToolTip::add(m_enabledLCB, tr("On/Off"));

    m_clippingLCB = new LedCheckBox(this, "Clipping", LedCheckBox::Red);
    m_clippingLCB->move(19 + 3, 17);
    // m_clippingLCB->setGeometry(0,30,19,19);
    m_clippingLCB->setBlinking(true);
    // m_clippingLCB->setText(tr("CLIP"));
    // m_clippingLCB->setTextAnchorPoint(Qt::AnchorBottom);
    // m_clippingLCB->setWhatsThis( tr( "Toggles the effect on or off." ) );

    m_runningLCB = new LedCheckBox(this, "Running", LedCheckBox::Yellow);
    m_runningLCB->move(19 + 3, 31);
    // m_runningLCB->setGeometry(0,0,19,19);
    // m_runningLCB->setText(tr("ON"));
    // m_runningLCB->setTextAnchorPoint(Qt::AnchorBottom);
    // m_runningLCB->setEnabled( isRunning );
    // m_runningLCB->setWhatsThis( tr( "Toggles the effect on or off." ) );
    // ToolTip::add( m_runningLCB, tr( "On/Off" ) );

    m_wetDryKNB = new Knob(knobBright_26, this);
    m_wetDryKNB->setText(tr("D-WET"));
    m_wetDryKNB->setGeometry(19 + 19, 5, 36, 36);
    m_wetDryKNB->setEnabled(isEnabled);
    m_wetDryKNB->setHintText(tr("Wet Level:"), "");
    m_wetDryKNB->setWhatsThis(
            tr("The Wet/Dry knob sets the ratio between "
               "the input signal and the effect signal that "
               "forms the output."));

    m_autoQuitKNB = new TempoSyncKnob(knobBright_26, this);
    m_autoQuitKNB->setText(tr("DECAY"));
    m_autoQuitKNB->setGeometry(19 + 55, 5, 36, 36);
    m_autoQuitKNB->setEnabled(isEnabled);
    m_autoQuitKNB->setHintText(tr("Time:"), "ms");
    m_autoQuitKNB->setWhatsThis(
            tr("The Decay knob controls how many buffers of silence must "
               "pass before the "
               "plugin stops processing.  Smaller values will reduce the CPU "
               "overhead but "
               "run the risk of clipping the tail on delay and reverb "
               "effects."));

    m_gateInKNB = new Knob(knobBright_26, this);
    m_gateInKNB->setText(tr("GATE"));
    m_gateInKNB->setGeometry(19 + 91, 5, 36, 36);
    m_gateInKNB->setEnabled(isEnabled);
    m_gateInKNB->setHintText(tr("Gate:"), "");
    m_gateInKNB->setWhatsThis(
            tr("The Gate knob controls the signal level that is considered "
               "to be 'silence' "
               "while deciding when to stop processing signals."));

    m_balanceKNB = new Knob(knobBright_26, this);
    m_balanceKNB->setText(tr("BAL"));
    m_balanceKNB->setGeometry(19 + 127, 5, 36, 36);
    m_balanceKNB->setEnabled(isEnabled);
    m_balanceKNB->setHintText(tr("Balance:"), "");
    m_balanceKNB->setWhatsThis(tr("The Balance knob controls how ..."));

    m_balanceKNB->setVisible(_model->isBalanceable());
    setModel(_model);

    QPushButton* menuBTN = new QPushButton(this);
    menuBTN->setIcon(embed::getIcon("menu"));
    menuBTN->move(223 - 25, 34);
    menuBTN->setFixedSize(20, 20);
    connect(menuBTN, SIGNAL(clicked()), this, SLOT(showContextMenu()));

    Effect* m=model();
    if(m==nullptr)
        qWarning("EffectView: effect null");
    else if(m->controls()==nullptr)
        qWarning("EffectView: effect controls null");
    else if(m->controls()->controlCount()==0)
        qWarning("EffectView: effect control count 0");
    else
    // if( effect()->controls()->controlCount() > 0 )
    {
        // QPushButton * ctrlBTN = new QPushButton( tr( "Controls" ),this );
        QPushButton* ctrlBTN = new QPushButton(
                embed::getIcon("configure"), "", this);
        ctrlBTN->setGeometry(223 - 25, 5 + 3, 20, 20);
        // QFont f = ctrlBTN->font();
        // ctrlBTN->setFont( pointSize<8>( f ) );
        // ctrlBTN->setGeometry( 163+24, 5, 36, 36 );//41
        connect(ctrlBTN, SIGNAL(clicked()), this, SLOT(openControls()));
    }

    setWhatsThis(tr(
            "Effect plugins function as a chained series of effects where "
            "the signal will "
            "be processed from top to bottom.\n\n"

            "The On/Off switch allows you to bypass a given plugin at any "
            "point in "
            "time.\n\n"

            "The Wet/Dry knob controls the balance between the input signal "
            "and the "
            "effected signal that is the resulting output from the effect.  "
            "The input "
            "for the stage is the output from the previous stage. So, the "
            "'dry' signal "
            "for effects lower in the chain contains all of the previous "
            "effects.\n\n"

            "The Decay knob controls how long the signal will continue to be "
            "processed "
            "after the notes have been released.  The effect will stop "
            "processing signals "
            "when the volume has dropped below a given threshold for a given "
            "length of "
            "time.  This knob sets the 'given length of time'.  Longer times "
            "will require "
            "more CPU, so this number should be set low for most effects.  "
            "It needs to be "
            "bumped up for effects that produce lengthy periods of silence, "
            "e.g. "
            "delays.\n\n"

            "The Gate knob controls the 'given threshold' for the effect's "
            "auto shutdown.  "
            "The clock for the 'given length of time' will begin as soon as "
            "the processed "
            "signal level drops below the level specified with this knob.\n\n"

            "The Controls button opens a dialog for editing the effect's "
            "parameters.\n\n"

            "Right clicking will bring up a context menu where you can "
            "change the order "
            "in which the effects are processed or delete an effect "
            "altogether."));

    // move above vst effect view creation
    // setModel( _model );
    modelChanged();
}

EffectView::~EffectView()
{

#ifdef LMMS_BUILD_LINUX

    delete m_subWindow;
#else
    if(m_subWindow)
    {
        // otherwise on win32 build VST GUI can get lost
        m_subWindow->hide();
    }
#endif
}

void EffectView::cut()
{
    model()->copy();
    removeEffect();
}

void EffectView::copy()
{
    model()->copy();
}

void EffectView::paste()
{
    // model()->paste();
}

void EffectView::changeName()
{
    /*
QString      s = m_tco->name();
RenameDialog rename_dlg(s);
rename_dlg.exec();
m_tco->setName(s);
    */
}

void EffectView::resetName()
{
    // m_tco->setName(m_tco->track()->name());
}

void EffectView::changeColor()
{
    /*
QColor new_color = QColorDialog::getColor(color());

if(!new_color.isValid())
{
    return;
}

if(isSelected())
{
    QVector<SelectableObject*> selected
            = gui->songEditor()->m_editor->selectedObjects();
    for(QVector<SelectableObject*>::iterator it = selected.begin();
        it != selected.end(); ++it)
    {
        EffectView* tcov
                = dynamic_cast<EffectView*>(*it);
        if(tcov)
            tcov->setColor(new_color);
    }
}
else
    setColor(new_color);
    */
}

void EffectView::resetColor()
{
    /*
if(isSelected())
{
    QVector<SelectableObject*> selected
            = gui->songEditor()->m_editor->selectedObjects();
    for(QVector<SelectableObject*>::iterator it = selected.begin();
        it != selected.end(); ++it)
    {
        EffectView* tcov
                = dynamic_cast<EffectView*>(*it);
        if(tcov)
            tcov->setUseStyleColor(true);
    }
}
else
    setUseStyleColor(true);
    */
}

void EffectView::openControls()
{
    if(m_controlView == nullptr)
    {
        m_controlView = model()->controls()->createView();
        if(m_controlView)
        {
            /*
              m_subWindow = gui->mainWindow()->addWindowedWidget(
            m_controlView ); Qt::WindowFlags flags =
            m_subWindow->windowFlags(); flags &=
            ~Qt::WindowMaximizeButtonHint; m_subWindow->setWindowFlags( flags
            ); m_subWindow->resize(m_subWindow->sizeHint());
              m_subWindow->setSizePolicy( QSizePolicy::Fixed,
            QSizePolicy::Fixed );
              //m_subWindow->setFixedSize( m_subWindow->sizeHint() );
            m_subWindow->setWindowIcon( m_controlView->windowIcon() );
            */
            m_subWindow = SubWindow::putWidgetOnWorkspace(
                    m_controlView, false, false, false, false);

            connect(m_controlView, SIGNAL(closed()), this,
                    SLOT(closeControls()));
            m_subWindow->hide();
        }
    }

    if(m_subWindow)
    {
        if(!m_subWindow->isVisible())
        {
            m_subWindow->show();
            m_subWindow->raise();
            // effect()->controls()->setViewVisible( true );
        }
        else
        {
            m_subWindow->hide();
            // effect()->controls()->setViewVisible( false );
        }
    }
}

void EffectView::closeControls()
{
    if(m_subWindow)
    {
        m_subWindow->hide();
    }
    // effect()->controls()->setViewVisible( false );
}

void EffectView::cloneEffect()
{
    // emit removeEffect(this);
}

void EffectView::toggleEffect()
{
    model()->toggleMute();
}

void EffectView::clearEffect()
{
    // emit removeEffect(this);
}

void EffectView::removeEffect()
{
    emit removeEffect(this);
}

void EffectView::moveUp()
{
    emit moveUp(this);
}

void EffectView::moveDown()
{
    emit moveDown(this);
}

void EffectView::moveTop()
{
    emit moveTop(this);
}

void EffectView::moveBottom()
{
    emit moveBottom(this);
}

void EffectView::displayHelp()
{
    QWhatsThis::showText(mapToGlobal(rect().bottomRight()), whatsThis());
}

void EffectView::mousePressEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::LeftButton)
    {
        _me->accept();
    }
}

void EffectView::mouseReleaseEvent(QMouseEvent* _me)
{
    if(_me->button() == Qt::LeftButton)
    {
        _me->accept();
        if(rect().contains(_me->pos()))
            Selection::select(model());
    }
}

void EffectView::contextMenuEvent(QContextMenuEvent* _cme)
{
    if(_cme->modifiers())
    {
        _cme->ignore();
        return;
    }

    QMenu* cm = buildContextMenu();
    cm->exec(QCursor::pos());
    delete cm;
}

void EffectView::showContextMenu()
{

    QMenu* cm = buildContextMenu();
    cm->exec(mapToGlobal(QPoint(0, 55)));
    delete cm;
}

QMenu* EffectView::buildContextMenu()
{
    QPointer<CaptionMenu> cm = new CaptionMenu(model()->displayName(), this);
    // QMenu* cm = new QMenu(this);

    QAction* a;
    a = cm->addAction(embed::getIconPixmap("piano"), tr("Open controls"),
                      this, SLOT(openControls()));
    a->setEnabled(true);
    a = cm->addAction(embed::getIconPixmap("edit_copy"),
                      tr("Clone this effect"), this, SLOT(cloneEffect()));
    a->setEnabled(false);

    cm->addSeparator();
    addRemoveMuteClearMenu(cm, true, true, false);

    cm->addSeparator();
    addCutCopyPasteMenu(cm, true, true, false);

    cm->addSeparator();
    cm->addAction(embed::getIconPixmap("arp_up"), tr("Move &up"), this,
                  SLOT(moveUp()));
    cm->addAction(embed::getIconPixmap("arp_down"), tr("Move &down"), this,
                  SLOT(moveDown()));
    cm->addAction(embed::getIconPixmap("arp_up"), tr("Move to &top"), this,
                  SLOT(moveTop()));
    cm->addAction(embed::getIconPixmap("arp_down"), tr("Move to &bottom"),
                  this, SLOT(moveBottom()));

    cm->addSeparator();
    addNameMenu(cm, false);

    cm->addSeparator();
    addColorMenu(cm, false);

    cm->addSeparator();
    cm->addHelpAction();

    return cm;
    // contextMenu->exec(_xy);
    // delete contextMenu;
}

void EffectView::addRemoveMuteClearMenu(QMenu* _cm,
                                        bool   _remove,
                                        bool   _mute,
                                        bool   _clear)
{
    QAction* a;
    a = _cm->addAction(
            embed::getIconPixmap("cancel"),
            tr("Remove this effect"),  // Delete
            // (<%1>+middle click)").arg(UI_SHIFT_KEY),
            this, SLOT(removeEffect()));  // remove
    a->setEnabled(_remove);
    a = _cm->addAction(
            embed::getIconPixmap("muted"),
            tr("Toggle"),  // (<%1>+middle click)").arg(UI_CTRL_KEY),
            this, SLOT(toggleEffect()));
    a->setEnabled(_mute);
    a = _cm->addAction(embed::getIconPixmap("edit_erase"), tr("Clear"), this,
                       SLOT(clearEffect()));
    a->setEnabled(_clear);
}

void EffectView::addCutCopyPasteMenu(QMenu* _cm,
                                     bool   _cut,
                                     bool   _copy,
                                     bool   _paste)
{
    QAction* a;
    a = _cm->addAction(embed::getIconPixmap("edit_cut"), tr("Cut"), this,
                       SLOT(cut()));
    a->setEnabled(_cut);
    a = _cm->addAction(embed::getIconPixmap("edit_copy"), tr("Copy"), this,
                       SLOT(copy()));
    a->setEnabled(_copy);
    a = _cm->addAction(embed::getIconPixmap("edit_paste"), tr("Paste"), this,
                       SLOT(paste()));
    a->setEnabled(_paste);
}

void EffectView::addNameMenu(QMenu* _cm, bool _enabled)
{
    QAction* a;
    a = _cm->addAction(embed::getIconPixmap("edit_rename"), tr("Change name"),
                       this, SLOT(changeName()));
    a->setEnabled(_enabled);
    a = _cm->addAction(embed::getIconPixmap("reload"), tr("Reset name"), this,
                       SLOT(resetName()));
    a->setEnabled(_enabled);
}

void EffectView::addColorMenu(QMenu* _cm, bool _enabled)
{
    QAction* a;
    a = _cm->addAction(embed::getIconPixmap("colorize"), tr("Change color"),
                       this, SLOT(changeColor()));
    a->setEnabled(_enabled);
    a = _cm->addAction(embed::getIconPixmap("colorize"), tr("Reset color"),
                       this, SLOT(resetColor()));
    a->setEnabled(_enabled);
}

void EffectView::paintEvent(QPaintEvent*)
{
    static QPixmap s_grip(embed::getPixmap("track_op_grip"));

    QPainter p(this);
    p.drawPixmap(0, 0, m_bg);

    p.setBrush(model()->color());
    lmms::fillRoundedRect(p, QRect(8, 2, width() - 11 + 1, height() - 4 + 1),
                          2, 2);

    p.setBrush(QColor(59, 66, 74));
    lmms::fillRoundedRect(p, QRect(1, 2, 8, height() - 4 + 1), 2, 2);
    int y = 2;
    while(y < height())
    {
        p.drawPixmap(2, y, s_grip);
        y += s_grip.height();
    }

    Effect* m = model();

    QFont ft = pointSizeF(font(), 7.5f);
    ft.setBold(true);
    p.setFont(ft);

    QString s = m->displayName();
    //p.setPen(QColor(0, 0, 0, 64));
    //p.drawText(19 + 23, 52, s);
    //p.drawText(19 + 25, 54, s);
    p.setPen(palette().text().color());
    p.drawText(19 + 24, 53, s);
}

void EffectView::modelChanged()
{
    Effect* m = model();
    m_runningLCB->setModel(&m->m_runningModel);
    m_enabledLCB->setModel(&m->m_enabledModel);
    m_clippingLCB->setModel(&m->m_clippingModel);
    m_wetDryKNB->setModel(&m->m_wetDryModel);
    m_autoQuitKNB->setModel(&m->m_autoQuitModel);
    m_gateInKNB->setModel(&m->m_gateModel);
    m_balanceKNB->setModel(&m->m_balanceModel);
    m_balanceKNB->setVisible(m->isBalanceable());

    m_enabledLCB->setVisible(m->isSwitchable());
    m_wetDryKNB->setVisible(m->isWetDryable());
    m_autoQuitKNB->setVisible(m->isGateable());
    m_gateInKNB->setVisible(m->isGateable());
}
