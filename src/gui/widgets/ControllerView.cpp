/*
 * ControllerView.cpp - view-component for an controller
 *
 * Copyright (c) 2008-2009 Paul Giblock <drfaygo/at/gmail.com>
 * Copyright (c) 2011-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include <QLabel>
#include <QMdiArea>
#include <QPushButton>
//#include <QMdiSubWindow>
#include <QPainter>
#include <QWhatsThis>
//#include <QLayout>
#include "Backtrace.h"
#include "CaptionMenu.h"
#include "ControllerDialog.h"
#include "ControllerView.h"
#include "GuiApplication.h"
#include "LedCheckBox.h"
#include "MainWindow.h"
#include "ToolTip.h"
#include "embed.h"
#include "gui_templates.h"

#include <QInputDialog>

// const int CONTROLLER_WIDTH  = 228;
// const int CONTROLLER_HEIGHT = 60;

ControllerView::ControllerView(Controller* _model, QWidget* _parent) :
      QWidget(_parent),  // QFrame( _parent ),
      ModelView(_model, this),
      m_bg(embed::getPixmap("controller_plugin")), m_subWindow(nullptr),
      m_controllerDlg(nullptr)
//,m_show( true )
{
    /*
      this->setFrameStyle( QFrame::StyledPanel );
      this->setFrameShadow( QFrame::Raised );

      QVBoxLayout *vBoxLayout = new QVBoxLayout(this);

      QHBoxLayout *hBox = new QHBoxLayout();
      vBoxLayout->addLayout(hBox);

      QLabel *label = new QLabel( "<b>" + _model->displayName() + "</b>",
      this); QSizePolicy sizePolicy = label->sizePolicy();
      sizePolicy.setHorizontalStretch(1);
      label->setSizePolicy(sizePolicy);

      hBox->addWidget(label);

      QPushButton * controlsButton = new QPushButton( tr( "Controls" ), this
      ); connect( controlsButton, SIGNAL( clicked() ), SLOT( editControls() )
      );

      hBox->addWidget(controlsButton);

      m_nameLabel = new QLabel(_model->name(), this);
      vBoxLayout->addWidget(m_nameLabel);
    */

    setFixedSize(m_bg.width(), m_bg.height());
    // setFixedSize( 230, 60 );

    // Disable controllers that are of type "DummyController"
    bool isEnabled
            = true;  //! dynamic_cast<DummyController *>( controller() );
    m_bypass = new LedCheckBox(
            this, "[bypass]", isEnabled ? LedCheckBox::Green : LedCheckBox::Red);
    m_bypass->move(3, 3);
    m_bypass->setEnabled(isEnabled);
    m_bypass->setWhatsThis(tr("Toggles the controller on or off."));

    ToolTip::add(m_bypass, tr("On/Off"));

    // qWarning("!!! Create a controller view model_is_null=%d",_model ==
    // nullptr); BACKTRACE
    setModel(_model);

    // QPushButton * ctls_btn = new QPushButton( tr( "Controls" ), this );
    if(_model != nullptr && _model->isConfigurable())
    {
        QPushButton* ctls_btn
                = new QPushButton(embed::getIcon("trackop"), "", this);
        QFont f = ctls_btn->font();
        ctls_btn->setFont(pointSize<8>(f));
        ctls_btn->setGeometry(163 + 24, 5, 36, 36);  // 41
        connect(ctls_btn, SIGNAL(clicked()), this, SLOT(editControls()));
    }

    setWhatsThis(
            tr("Controllers are able to automate the value of a knob, "
               "slider, and other controls."));
}

ControllerView::~ControllerView()
{
    if(m_subWindow)
    {
        delete m_subWindow;
    }
}

void ControllerView::setModel(Model* model)//, bool isOldModelValid)
{
    if(model == nullptr)
        qWarning("!!! ControllerView::setModel() model is nullptr");
    ModelView::setModel(model);//, isOldModelValid);
}

void ControllerView::editControls()
{
    if(!m_subWindow)
    {
        m_controllerDlg
                = controller()->createDialog(gui->mainWindow()->workspace());

        /*
        m_subWindow = gui->mainWindow()->addWindowedWidget( m_controllerDlg );
        Qt::WindowFlags flags = m_subWindow->windowFlags();
        flags &= ~Qt::WindowMaximizeButtonHint;
        m_subWindow->setWindowFlags( flags );
        m_subWindow->setFixedSize( m_subWindow->size() );
        m_subWindow->setWindowIcon( m_controllerDlg->windowIcon() );
        */
        m_subWindow = SubWindow::putWidgetOnWorkspace(m_controllerDlg, false,
                                                      false, false);

        connect(m_controllerDlg, SIGNAL(closed()), this,
                SLOT(closeControls()));
        m_subWindow->hide();
    }

    if(m_subWindow)
    {
        if(!m_subWindow->isVisible())
        {
            m_subWindow->show();
            m_subWindow->raise();
            // m_show = false;
        }
        else
        {
            m_subWindow->hide();
            // m_show = true;
        }
    }
}

void ControllerView::closeControls()
{
    if(m_subWindow)
        m_subWindow->hide();

    // m_show = true;
}

void ControllerView::deleteController()
{
    emit deleteController(controller());
}

void ControllerView::renameController()
{
    bool        ok;
    Controller* c        = controller();  // castModel<Controller>();
    QString     new_name = QInputDialog::getText(
            this, tr("Rename controller"),
            tr("Enter the new name for this controller"), QLineEdit::Normal,
            c->name(), &ok);
    if(ok && !new_name.isEmpty())
    {
        c->setName(new_name);
        if(controller()->type() == Controller::LfoController)
        {
            m_controllerDlg->setWindowTitle(tr("LFO") + " (" + new_name
                                            + ")");
        }
        // m_nameLabel->setText( new_name );
    }
}

void ControllerView::mouseDoubleClickEvent(QMouseEvent* event)
{
    renameController();
}

void ControllerView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, m_bg);

    QFont f = pointSizeF(font(), 7.5f);
    f.setBold(true);
    p.setFont(f);

    // p.setPen( palette().shadow().color() );
    // p.drawText( 7, 54, model()->displayName() );
    p.setPen(palette().text().color());
    p.drawText(6, 53, model()->displayName());

    /*QFont*/ f = pointSizeF(font(), 7.5f);  // 9.5
    // f.setBold( true );
    p.setFont(f);

    Controller* c = controller();  // castModel<Controller>();
    // p.setPen( palette().shadow().color() );
    // p.drawText( 87, 54, c->name() );
    p.setPen(palette().text().color());
    p.drawText(86, 53, c->name());
}

void ControllerView::modelChanged()
{
    m_bypass->setModel(&controller()->m_enabledModel);
}

void ControllerView::contextMenuEvent(QContextMenuEvent*)
{
    Controller* c = controller();
    if(c == nullptr)
        return;

    QPointer<CaptionMenu> contextMenu
            = new CaptionMenu(model()->displayName(), this);
    if(c->isRemovable())
    {
        contextMenu->addAction(embed::getIcon("cancel"),
                               tr("&Remove this controller"), this,
                               SLOT(deleteController()));
    }
    contextMenu->addAction(tr("Re&name this controller"), this,
                           SLOT(renameController()));
    contextMenu->addSeparator();
    contextMenu->addHelpAction();
    contextMenu->exec(QCursor::pos());
    delete contextMenu;
}

void ControllerView::displayHelp()
{
    QWhatsThis::showText(mapToGlobal(rect().center()), whatsThis());
}
