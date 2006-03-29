/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
*               2000, 2002 kd.moeller@t-online.de
*                     2006 David Saxton <david@bluehaze.org>
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
*/

/** @file MainDlg.h
 * @brief Contains the main window class MainDlg. */

#ifndef MainDlg_included
#define MainDlg_included

// Qt includes
#include <QDomDocument>
#include <QPicture>
#include <QStack>

// KDE includes
#include <kaction.h>
#include <kfiledialog.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>
#include <kparts/factory.h>

#undef  GrayScale

// local includes
#include "coordsconfigdialog.h"
#include "View.h"
#include "kminmax.h"
#include "kmplotio.h"
#include "MainDlgIface.h"

#include "editscaling.h"
#include "settingspagecolor.h"
#include "settingspagefonts.h"
#include "settingspagegeneral.h"

class BrowserExtension;
class EditScaling;
class FunctionEditor;
class KConfigDialog;
class KConstantEditor;
class KToggleFullScreenAction;
class KAboutData;
class KAction;
class KLineEdit;
class KRecentFilesAction;
class QTimer;
class SettingsPageColor;
class SettingsPageConstants;
class SettingsPageFonts;
class SettingsPageGeneral;

/** @short This is the main window of KmPlot.
 *
 * Its central widget view contains the parser, accessable via its parser() function.
 * @see View, View::m_parser, View::parser
 */
class MainDlg : public KParts::ReadOnlyPart, virtual public MainDlgIface
{
	Q_OBJECT

public:
	/** Constuctor.
	 * @param parentWidget parent widget for this part
	 * @param parent parent object
	 * @param name name of this dialog
	 */
	MainDlg(QWidget *parentWidget, const char *, QObject *parent );
	/// Cleaning up a bit.
	virtual ~MainDlg();
	/// This class needs access to private members, too.
	friend class FktDlg;
	/// This class needs access to private members, too.
	friend class BezWnd;

	/// Asks the user and returns true if modified data shall be dicarded.
	bool checkModified();
	/// Is set to true if a file from an old file format was loaded
	static bool oldfileversion;

public slots:
	/// Implement the coordinate system edit dialog
	void editAxes();
	/// Implement the scaling edit dialog
	void editScaling();
	/// Toggle whether the sliders window is shown
	void toggleShowSliders();
	/// Revert to the previous document state (in m_undoStack).
	void undo();
	/// Revert to the next document state (in m_redoStack).
	void redo();
	/// Pushes the previous document state to the undo stack and records the current one
	void requestSaveCurrentState();
	
	// ///I'm not sure it a delete-all-functions command is necessary
	// void slotCleanWindow();
	///Save a plot i.e. save the function name and all the settings for the plot
	void slotSave();
	///Save a plot and choose a name for it
	void slotSaveas();
	///Print the current plot
	void slotPrint();
	///Export the current plot as a png, svg or bmp picture
	void slotExport();
	///Implement the Configure KmPlot dialog
	void slotSettings();
	/// Calls the common function names dialog.
	/// @see BezWnd::hideEvent
	void slotNames();
	/// Change the coordinate systems, shows negative x-values and negative y-values.
	void slotCoord1();
	/// Change the coordinate systems, shows positive x-values and negative y-values.
	void slotCoord2();
	/// Change the coordinate systems, shows positive x-values and positive y-values.
	void slotCoord3();
	/// Tools menu
	void getYValue();
	void findMinimumValue();
	void findMaximumValue();
	void graphArea();

private:
	/// Settings the standard and non standard actions of the application.
	void setupActions();
	/// Called when a file is opened. The filename is is m_url
	virtual bool openFile();
	/// Returns true if any changes are done
	bool isModified(){return m_modified;}
	
	/// Central widget of the KMainWindow instance. tralala
	View *view;
	///The Recent Files action
	KRecentFilesAction * m_recentFiles;
	/// true == modifications not saved
	bool m_modified;
	///An instance of the application config file
	KConfig* m_config;
	///A Configure KmPlot dialog instance
	KConfigDialog* m_settingsDialog;
	///The General page for the Configure KmPlot dialog
	SettingsPageGeneral * m_generalSettings;
	///The Colors page for the Configure KmPlot constants
	SettingsPageColor * m_colorSettings;
	///The Fonts page for the Configure KmPlot constants
	SettingsPageFonts * m_fontsSettings;
	///The Constants page for the Configure KmPlot constants
	KConstantEditor* m_constantsSettings;

	/// A dialog used by many tools-menu-items
	KMinMax *minmaxdlg;
	/// the popup menu shown when cling with the right mouse button on a graph in the graph widget
	KMenu *m_popupmenu;
	/// Loads and saves the user's file.
	KmPlotIO *kmplotio;
	/// Set to true if the application is readonly
	bool m_readonly;
	/// MainDlg's parent widget
	QWidget *m_parent;
	/// Current file
	KUrl m_currentfile;
	/// The axes config dialogs
	CoordsConfigDialog* coordsDialog;
	/// The function editor
	FunctionEditor * m_functionEditor;
	/// The undo stack
	QStack<QDomDocument> m_undoStack;
	/// The reod stack
	QStack<QDomDocument> m_redoStack;
	/**
	 * The current document state - this is pushed to the undo stack when a new
	 * document state is created.
	 */
	QDomDocument m_currentState;
	/// Timer to ensure saveCurrentState() is called only once for a set of simultaneous changes
	QTimer * m_saveCurrentStateTimer;
	/// The undo action
	KAction * m_undoAction;
	/// The redo action
	KAction * m_redoAction;

protected slots:
	/**
	* When you click on a File->Open Recent file, it'll open 
	* @param url name of the url to open
	*/
	void slotOpenRecent( const KUrl &url );
	///Update settings when there is a change in the Configure KmPlot dialog
	void updateSettings();
	/// @see requestSaveCurrentState
	void saveCurrentState();

	void setReadOnlyStatusBarText(const QString &);

	/// slots for the settings-menu
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
};

class KmPlotPartFactory : public KParts::Factory
{
	Q_OBJECT
public:
	KmPlotPartFactory();
	virtual ~KmPlotPartFactory();
	virtual KParts::Part* createPartObject( QWidget *parentWidget, const char *widgetName,
	                                        QObject *parent, const char *name,
	                                        const char *classname, const QStringList &args );
	static KInstance* instance();

private:
	static KInstance* s_instance;
	static KAboutData* s_about;
};

class BrowserExtension : public KParts::BrowserExtension
{
	Q_OBJECT
public:
	BrowserExtension(MainDlg*);

public slots:
	// Automatically detected by the host.
	void print();
};


class EditScaling : public QWidget, public Ui::EditScaling
{
	public:
		EditScaling( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};

class SettingsPageColor : public QWidget, public Ui::SettingsPageColor
{
	public:
		SettingsPageColor( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};

class SettingsPageFonts : public QWidget, public Ui::SettingsPageFonts
{
	public:
		SettingsPageFonts( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};

class SettingsPageGeneral : public QWidget, public Ui::SettingsPageGeneral
{
	public:
		SettingsPageGeneral( QWidget * parent = 0 )
	: QWidget( parent )
		{ setupUi(this); }
};

#endif // MainDlg_included
