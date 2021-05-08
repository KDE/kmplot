/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 2004 Fredrik Edemar <f_edemar@linux.se>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-FileCopyrightText: Fredrik Edemar <f_edemar@linux.se>
    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef KMPLOT_H_
#define KMPLOT_H_

#include <KParts/MainWindow>
#include <KParts/ReadWritePart>

class QCommandLineParser;
class QLabel;
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
	explicit KmPlot( const QCommandLineParser& parser );

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
 	bool queryClose() Q_DECL_OVERRIDE;

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
	Q_SCRIPTABLE void fileOpen(const QString &str) { fileOpen(QUrl(str)); }
	Q_SCRIPTABLE void setStatusBarText(const QString &, int id);
	Q_SCRIPTABLE void openFileInNewWindow(const QString &str) { openFileInNewWindow(QUrl(str)); }
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
	/// Called when fullscreen is enabled/disabled
	void slotUpdateFullScreen(bool);
	void fileOpen(const QUrl &url);

private:
	void setupAccel();
	void setupActions();
	void setupStatusBar();
	bool isModified();


private:
	KParts::ReadWritePart *m_part;
	/// The fullscreen action to be plugged/unplugged to the toolbar
	KToggleFullScreenAction* m_fullScreen;
	/// The progress bar for drawing functions
	KmPlotProgress * m_progressBar;
	/// The labels set to show information on the statusbar
	QList<QLabel *> statusBarLabels;
};

#endif // KMPLOT_H_
