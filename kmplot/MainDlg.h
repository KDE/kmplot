/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
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
*
*/

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
#include "BezWnd.h"
#include "View.h"


class KConfigDialog;
class KLineEdit;
class KRecentFilesAction;
class SettingsPageCoords;
class SettingsPageColor;
class SettingsPageFonts;
class SettingsPagePrecision;
class SettingsPageScaling;

class MainDlg : public KMainWindow
{
	Q_OBJECT

public:
	MainDlg( const QString sessionId, KCmdLineArgs* args, const char* name = NULL );
	virtual ~MainDlg();
	friend class FktDlg;
	friend class BezWnd;

public slots:
	
	void editColors();
	void editAxes();
	void editGrid();
	void editScaling();
	void editFonts();
	void editPrecision();
	void onNewFunction();
	void onNewParametric();
	void onNewPolar();

private:
	void setupActions();
	
	void setupStatusBar();
	bool checkModified();
	int tbid,
	stbid;
	///The Statusbar instance
	KStatusBar *stbar;
	FktDlg *fdlg;
	BezWnd *bez;
	///In the Help menu, this action will display a dialog with the names of the common mathematical functions
	KToggleAction *view_names;
	View *view;
	QString m_sessionId;
	///The Recent Files action
	KRecentFilesAction * m_recentFiles;
	///Current filename of the current plot
	QString m_filename;      
	bool m_modified;
	///An instance of the application config file
	KConfig* m_config;
	///The KLineEdit which is in the toolbar
	KLineEdit* m_quickEdit;
	///A Configure KmPlot dialog instance
	KConfigDialog* m_settingsDialog;
	///The Colors tab for the Configure KmPlot dialog
	SettingsPageColor* color_settings;
	///The Coordinates tab for the Configure KmPlot dialog
	SettingsPageCoords* coords_settings;
	///The Scaling tab  for the Configure KmPlot dialog
	SettingsPageScaling* scaling_settings;
	///The Fonts tab for the Configure KmPlot dialog
	SettingsPageFonts* fonts_settings;
	///The Precision page for the Configure KmPlot dialog
	SettingsPagePrecision* precision_settings;
	
protected slots:

	void slotOpen();
	///Implement the File -> New action by cleaning the plot area
	void slotOpenNew();
	/**
	When you click on a File->Open Recent file, it'll open 
	
	@param url name of the url to open
	*/
	void slotOpenRecent( const KURL &url );
	///Save a plot i.e. save the function name and all the settings for the plot
	void slotSave();
	///Save a plot and choose a name for it
	void slotSaveas();
	///Call the dialog (an instance of FktDlg) to edit the functions and make changes on them
	void slotEditFunctions();
	///Print the current plot
	void slotPrint();
	///Export the current plot as a png, svg or bmp picture
	void slotExport();
	///Implement the Configure KmPlot dialog
	void slotSettings();
	///
	void newToolbarConfig();
	///
	void optionsConfigureKeys();
	///
	void optionsConfigureToolbars();
	///Update settings when there is a change in the Configure KmPlot dialog
	void updateSettings();
	///Calls the common function names dialog
	void slotNames();
	///SLot to change the coordinate systems, shows negative x-values and negative y-values
	void slotCoord1();
	///SLot to change the coordinate systems, shows positive x-values and negative y-values
	void slotCoord2();
	///SLot to change the coordinate systems, shows positive x-values and positive y-values
	void slotCoord3();
	/**
	Manages the LineEdit content after returnPressed() is emitted
	@param the content of the KLineEdit
	*/
	void slotQuickEdit( const QString& );
};

#endif // MainDlg_included
