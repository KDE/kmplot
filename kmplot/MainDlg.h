/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
*               2000, 2002 kd.moeller@t-online.de
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
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/** @file MainDlg.h
 * @brief Contains the main window class MainDlg. */

#ifndef MainDlg_included
#define MainDlg_included

// Qt includes
#include <qpicture.h>

// KDE includes
#include <kaction.h>
#include <kcmdlineargs.h>
#include <kmainwindow.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

#undef  GrayScale

// local includes
#include "FktDlg.h"
#include "View.h"


class KConfigDialog;
class KLineEdit;
class KRecentFilesAction;
class SettingsPageCoords;
class SettingsPageColor;
class SettingsPageFonts;
class SettingsPagePrecision;
class SettingsPageScaling;
class KConstantEditor;
class KToggleFullScreenAction;



/** @short This is the main window of KmPlot.
 *
 * Its central widget view contains the parser, accessable via its parser() function.
 * @see View, View::m_parser, View::parser
 */
class MainDlg : public KMainWindow
{
	Q_OBJECT

	public:
		/** @param sessionId used for the name of a temporary file.
		 * @param args containing a filename to be plot on startup.
		 * @param name
		 */
		MainDlg( const QString sessionId, KCmdLineArgs* args, const char* name = NULL );
		/// Cleaning up a bit.
		virtual ~MainDlg();
		/// This class needs access to private members, too.
		friend class FktDlg;
		/// This class needs access to private members, too.
		friend class BezWnd;
	
	public slots:
		/// Implement the color edit dialog
		void editColors();
		/// Implement the coordinate system edit dialog
		void editAxes();
		/// Implement the scaling edit dialog
		void editScaling();
		/// Implement the fonts edit dialog
		void editFonts();
		/// Implement the constants edit dialog
		void editConstants();
		/// Implement the dialog to enter a function plot and its options
		void newFunction();
		/// Implement the dialog to enter a parametric plot and its options
		void newParametric();
		/// Implement the dialog to enter a polar plot and its options
		void newPolar();
	
	private:
		/// Settings the standard and non standard actions of the application.
		void setupActions();
		/// Predefines some space for coordinate information of the plot
		void setupStatusBar();
		/// Asks the user and returns true if modified data shall be dicarded.
		bool checkModified();
		int tbid,
		stbid;
		/// The Statusbar instance
		KStatusBar *stbar;
		/// Cached dialog to edit all functions
		FktDlg *fdlg;
		/// Central widget of the KMainWindow instance. tralala
		View *view;
		/// unique string for tmp file
		QString m_sessionId;
		///The Recent Files action
		KRecentFilesAction * m_recentFiles;
		/** Current filename of the current plot
		 *  isEmpty() == not yet saved */
		QString m_filename;      
		/// true == modifications not saved
		bool m_modified;
		///An instance of the application config file
		KConfig* m_config;
		///The KLineEdit which is in the toolbar
		KLineEdit* m_quickEdit;
		///A Configure KmPlot dialog instance
		KConfigDialog* m_settingsDialog;
		///The Precision page for the Configure KmPlot dialog
		SettingsPagePrecision* m_generalSettings;
		///The Constants page for the Configure KmPlot constants
		KConstantEditor* m_constantsSettings;
		/// The fullscreen action to be plugged/unplegged to the toolbar
		KToggleFullScreenAction* m_fullScreen;
		/// Loading the constants by start
		void loadConstants();
		/// Loading the constants when closing the program
		void saveConstants();
		
	protected slots:
		/// Implement the File -> Open action
		void slotOpen();
		///Implement the File -> New action by cleaning the plot area
		void slotOpenNew();
		/**
		* When you click on a File->Open Recent file, it'll open 
		* @param url name of the url to open
		*/
		void slotOpenRecent( const KURL &url );
		///Save a plot i.e. save the function name and all the settings for the plot
		void slotSave();
		///Save a plot and choose a name for it
		void slotSaveas();
		///Call the dialog (an instance of FktDlg) to edit the functions and make changes on them
		void slotEditPlots();
		///Print the current plot
		void slotPrint();
		///Export the current plot as a png, svg or bmp picture
		void slotExport();
		///Implement the Configure KmPlot dialog
		void slotSettings();
		///
		void newToolbarConfig();
		///
		void optionsConfigureToolbars();
		///Update settings when there is a change in the Configure KmPlot dialog
		void updateSettings();
		/// Calls the common function names dialog.
		/// @see BezWnd::hideEvent
		void slotNames();
		/// Change the coordinate systems, shows negative x-values and negative y-values.
		void slotCoord1();
		/// Change the coordinate systems, shows positive x-values and negative y-values.
		void slotCoord2();
		/// Change the coordinate systems, shows positive x-values and positive y-values.
		void slotCoord3();
		/**
		* Manages the LineEdit content after returnPressed() is emitted.
		* @param f_str the content of the KLineEdit
		*/
		void slotQuickEdit( const QString& tmp_f_str );
		/// Enable or disable fullscreen
		void slotFullScreen();
		/// Tools menu
		void getYValue();
		void findMinimumValue();
		void findMaximumValue();
		
	protected:
		/// Quits KmPlot after checking if modifications shall be saved.
		virtual bool queryClose();

};

#endif // MainDlg_included
