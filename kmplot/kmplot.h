/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar
*                     f_edemar@linux.se
*               
* This file is part of the KDE Project.
* KmPlot is part of the KDE-EDU Project.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

#ifndef KMPLOT_H_
#define KMPLOT_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kparts/mainwindow.h>

#include "kmplotIface.h"
#include "kmplotprogress.h"

class KToggleAction;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Fredrik Edemar <f_edemar@linux.se>
 */
class KmPlot : public KParts::MainWindow, virtual public KmPlotIface
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 */
	KmPlot( KCmdLineArgs* args);

	/**
	 * Default Destructor
	 */
	virtual ~KmPlot();

	/**
	 * Use this method to load whatever file/URL you have
	 */
	bool load(const KURL& url);

protected:
	/**
	 * This method is called when it is time for the app to save its
	 * properties for session management purposes.
	 */
	void saveProperties(KConfig *);

	/**
	 * This method is called when this app is restored.  The KConfig
	 * object points to the session management config file that was saved
	 * with @ref saveProperties
	 */
	void readProperties(KConfig *);

	/// Quits KmPlot after checking if modifications shall be saved.
	virtual bool queryClose();

private slots:
	void fileNew();
	void fileOpen();
	void fileOpen(const KURL &url);
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	void applyNewToolbarConfig();

	/// Called when the user want to cancel the drawing
	void progressbar_clicked();

public slots:
	/// Called when fullscren is enabled/disabled
	void slotUpdateFullScreen(bool);
	void setStatusBarText(const QString &, int id);

private:
	void setupAccel();
	void setupActions();
	void setupStatusBar();
	bool checkModified();
	bool isModified();
	void openFileInNewWindow(const KURL url);

	/// Functions for the progressbar
	bool stopProgressBar();
	void startProgressBar(int);
	void increaseProgressBar();


private:
	KParts::ReadOnlyPart *m_part;
	/// The fullscreen action to be plugged/unplegged to the toolbar
	KToggleFullScreenAction* m_fullScreen;
	KmPlotProgress *m_progressbar;
};

#endif // KMPLOT_H_
