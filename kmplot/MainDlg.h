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
#include <qwidget.h>
#include <qpaintdevicemetrics.h>
#include <qpopupmenu.h>
#include "qcolor.h"
#include <qkeycode.h>
#include <qdatastream.h>
#include <qfile.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qpicture.h>

// KDE includes
#include <kapplication.h>
#include <kaction.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>
#include <kmenubar.h>
#include <kprinter.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kurl.h>

#undef  GrayScale

// local includes
#include "FktDlg.h"
#include "KoordDlg.h"
#include "RstDlg.h"
#include "SkalDlg.h"
#include "SwDlg.h"
#include "BezWnd.h"
#include "misc.h"
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
	
	void doexport();
	void print();
	void editColors();
	void editAxes();
	void editGrid();
	void editScaling();
	void editFonts();
	void editPrecision();
	void bezeichnungen();
	void onNewFunction();
	void onNewParametric();
	void onNewPolar();
	void funktionen();
	void onQuickEdit( const QString& );
	void onachsen1();
	void onachsen2();
	void onachsen3();
	
	

private:
	void setupActions();
	void doSave( QString filename = "" );
	void setupStatusBar();
	void addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue );

	void parseAxes( const QDomElement &n );
	void parseGrid( const QDomElement &n );
	void parseScale( const QDomElement &n );
	void parseStep( const QDomElement &n );
	void parseFunction( const QDomElement &n );
	bool checkModified();
	int tbid,
	stbid;
	KStatusBar *stbar;
	FktDlg *fdlg;
	BezWnd *bez;
	KToggleAction *view_bezeichnungen;
	View *view;
	QString m_sessionId;
	KRecentFilesAction * m_recentFiles;
	QString m_filename;      // current filename
	bool m_modified;
	KConfig* m_config;
	KLineEdit* m_quickEdit;
	
	KConfigDialog* m_settingsDialog;
	SettingsPageColor* color_settings;
	SettingsPageCoords* coords_settings;
	SettingsPageScaling* scaling_settings;
	SettingsPageFonts* fonts_settings;
	SettingsPagePrecision* precision_settings;
	
protected slots:

	void slotOpen();
	///Implement the File -> New action by cleaning the plot area
	void slotOpenNew();
	
	void slotOpenRecent( const KURL &url );
	///Save a plot i.e. save the function name and all the settings for the plot
	void slotSave();
	void slotSaveas();
	///Implement the Configure KmPlot dialog
	void slotSettings();
	void newToolbarConfig();
	void optionsConfigureKeys();
	void optionsConfigureToolbars();
	void updateSettings();

protected:
	void openFile( const QString filename );
	
};
#endif // MainDlg_included
