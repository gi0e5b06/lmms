/*
 * GuiApplication.h
 *
 * Copyright (c) 2014 Lukas W <lukaswhl/at/gmail.com>
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

#ifndef GUIAPPLICATION_H
#define GUIAPPLICATION_H

#include <QObject>
#include <QThread>

#include "export.h"

class QLabel;

class MainWindow;

class AutomationWindow;
class BBWindow;
class PianoRollWindow;
class SongWindow;

class ControllerRackView;
class FxMixerView;
class ProjectNotes;

class EXPORT GuiApplication : public QObject
{
	Q_OBJECT;
public:
	explicit GuiApplication(bool showSplashScreen = true);
	virtual ~GuiApplication();

	static GuiApplication* instance();

	MainWindow* mainWindow() { return m_mainWindow; }
	AutomationWindow* automationWindow() { return m_automationWindow; }
	BBWindow* bbWindow() { return m_bbWindow; }
	PianoRollWindow* pianoRollWindow() { return m_pianoRollWindow; }
	SongWindow* songWindow() { return m_songWindow; }

	ControllerRackView* getControllerRackView() { return m_controllerRackView; }
	FxMixerView* fxMixerView() { return m_fxMixerView; }
	ProjectNotes* getProjectNotes() { return m_projectNotes; }

public slots:
	void displayInitProgress(const QString &msg);
	void hasSongFinished();

private slots:
	void childDestroyed(QObject *obj);

private:
	static GuiApplication* s_instance;

	MainWindow* m_mainWindow;

	AutomationWindow* m_automationWindow;
	BBWindow* m_bbWindow;
	PianoRollWindow* m_pianoRollWindow;
	SongWindow* m_songWindow;

	ControllerRackView* m_controllerRackView;
	FxMixerView* m_fxMixerView;
	ProjectNotes* m_projectNotes;

	QLabel* m_loadingProgressLabel;
};

#define gui GuiApplication::instance()

#define PAINT_THREAD_CHECK						\
	if(QThread::currentThread()!=GuiApplication::instance()->thread()) \
		qWarning("PAINT THREAD CHECK: CT=%s OT=%s %s#%d",	\
			 qPrintable(QThread::currentThread()->objectName()), \
			 qPrintable(thread()->objectName()),__FILE__,__LINE__);

#endif // GUIAPPLICATION_H
