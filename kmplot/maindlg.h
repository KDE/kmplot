/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
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
#include <QStack>
#include <QMenu>

// KDE includes
#include <ksharedconfig.h>
#include <kstandarddirs.h>
#include <kparts/browserextension.h>
#include <kparts/part.h>
#include <kparts/readwritepart.h>
#include <kparts/factory.h>

#undef  GrayScale

// local includes
#include "coordsconfigdialog.h"
#include "view.h"
#include "kmplotio.h"

class BrowserExtension;
class Calculator;
class FunctionEditor;
class FunctionTools;
class KConfigDialog;
class KConstantEditor;
class QAction;
class KRecentFilesAction;
class QTimer;


class SettingsPageColor;
class SettingsPageFonts;
class SettingsPageGeneral;
class SettingsPageDiagram;

/**
 * @short This is the main window of KmPlot.
 */
class MainDlg : public KParts::ReadWritePart
{
	Q_OBJECT

	public:
		/** Constructor.
		 * @param parentWidget parent widget for this part
		 * @param parent parent object
		 */
		MainDlg(QWidget *parentWidget, QObject *parent, const QVariantList& = QVariantList() );

		/// Initialized as a pointer to this MainDlg object on creation
		static MainDlg * self() { return m_self; }

	/// Cleaning up a bit.
	virtual ~MainDlg();

	/// Is set to true if a file from an old file format was loaded
	static bool oldfileversion;

	/// The function editor
	FunctionEditor * functionEditor() const { return m_functionEditor; }

	/// The coords config dialog
	CoordsConfigDialog* coordsDialog();

	/// Returns true if any changes are done
	bool isModified(){return m_modified;}

	bool queryClose() Q_DECL_OVERRIDE;

	/// For inserting the title in the function popup menu
	QAction * m_firstFunctionAction;

	/// Show the constants editor modal to a parent dialog
	void editConstantsModal(QWidget *parent);

	/// Check whether the url links to an existing file
	static bool fileExists(const QUrl &url);


public Q_SLOTS:
    // DBus interface
    /// Asks the user and returns true if modified data shall be discarded.
    Q_SCRIPTABLE bool checkModified();


public slots:
	/// Implement the coordinate system edit dialog
	void editAxes();
	/// Show the constants editor
	void editConstants();
	/// Toggle whether the sliders window is shown
	void toggleShowSliders();
	/// Revert to the previous document state (in m_undoStack).
	void undo();
	/// Revert to the next document state (in m_redoStack).
	void redo();
	/// Pushes the previous document state to the undo stack and records the current one
	void requestSaveCurrentState();

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
	void slotNames();
	/// Resets the view
	void slotResetView();
	/// Tools menu
	void calculator();
	void findMinimumValue();
	void findMaximumValue();
	void graphArea();

private:
	/// Settings the standard and non standard actions of the application.
	void setupActions();
	/// Called when a file is opened. The filename is is m_url
	bool openFile() Q_DECL_OVERRIDE;


    bool saveFile() Q_DECL_OVERRIDE;

	///The Recent Files action
	KRecentFilesAction * m_recentFiles;
	/// true == modifications not saved
	bool m_modified;
	///An instance of the application config file
	KSharedConfigPtr m_config;
	///A Configure KmPlot dialog instance
	KConfigDialog* m_settingsDialog;
	///The General page for the Configure KmPlot dialog
	SettingsPageGeneral * m_generalSettings;
	///The Colors page for the Configure KmPlot constants
	SettingsPageColor * m_colorSettings;
	///The Fonts page for the Configure KmPlot constants
	SettingsPageFonts * m_fontsSettings;
	/// The diagram config page
	SettingsPageDiagram* m_diagramSettings;

	/// A dialog used by many tools-menu-items
	FunctionTools *m_functionTools;
	/// The calculator dialog
	Calculator *m_calculator;
	/// the popup menu shown when cling with the right mouse button on a graph in the graph widget
	QMenu *m_popupmenu;
	/// the popup that shows when clicking on the new plot button in the function editor
	QMenu * m_newPlotMenu;
	/// Loads and saves the user's file.
	KmPlotIO *kmplotio;
	/// Set to true if the application is readonly
	bool m_readonly;
	/// MainDlg's parent widget
	QWidget *m_parent;
	/// Current file
	QUrl m_currentfile;
	/// The axes config dialogs
	CoordsConfigDialog* m_coordsDialog;
	/// The constants editor
	QPointer<KConstantEditor> m_constantEditor;
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
	QAction * m_undoAction;
	/// The redo action
	QAction * m_redoAction;

	/// A pointer to ourself
	static MainDlg * m_self;

protected slots:
	/**
	* When you click on a File->Open Recent file, it'll open
	* @param url name of the url to open
	*/
	void slotOpenRecent( const QUrl &url );
	/// @see requestSaveCurrentState
	void saveCurrentState();
	/// Used when opening a new file
	void resetUndoRedo();

	void setReadOnlyStatusBarText(const QString &);
};

class BrowserExtension : public KParts::BrowserExtension
{
	Q_OBJECT
public:
	explicit BrowserExtension(MainDlg*);

public slots:
	// Automatically detected by the host.
	void print();
};

#endif // MainDlg_included
