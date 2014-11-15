/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar <f_edemar@linux.se>
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

#include <kcmdlineargs.h>
#include <kparts/mainwindow.h>
#include <kparts/readwritepart.h>

class KmPlotProgress;
class KToggleFullScreenAction;

/**
 * This is the application "Shell".  It has a menubar, toolbar, and
 * statusbar but relies on the "Part" to do all the real work.
 *
 * @short Application Shell
 * @author Fredrik Edemar <f_edemar@linux.se>
 */
class KmPlot : public KParts::MainWindow
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
	bool load(const QUrl& url);

protected:
	/// Quits KmPlot after checking if modifications shall be saved.
 	virtual bool queryClose();

private slots:
	void fileNew();
	void applyNewToolbarConfig();
	/**
	 * Called when the cancel button is clicked in the progress bar.
	 */
	void cancelDraw();

public Q_SLOTS:
    // DBus interface
    Q_SCRIPTABLE void fileOpen();
    Q_SCRIPTABLE void setStatusBarText(const QString &, int id);
	Q_SCRIPTABLE void openFileInNewWindow(const QUrl &url);
	/**
	 * Set the progress of drawing the plots, with \p progress ranging from 0
	 * to 1. After initially calling this function with \p progress less than
	 * 1, the progress bar will only be shown after a small delay (to avoid it
	 * flashing quickly when taking only a short while to draw). If \p progress
	 * is 1, then the progress bar will be hidden.
	 */
	Q_SCRIPTABLE void setDrawProgress( double progress );

public slots:
	/// Called when fullscren is enabled/disabled
	void slotUpdateFullScreen(bool);
	void fileOpen(const QUrl &url);

private:
	void setupAccel();
	void setupActions();
	void setupStatusBar();
	bool isModified();


private:
	KParts::ReadWritePart *m_part;
	/// The fullscreen action to be plugged/unplegged to the toolbar
	KToggleFullScreenAction* m_fullScreen;
	/// The progress bar for drawing functions
	KmPlotProgress * m_progressBar;
};

#endif // KMPLOT_H_
