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
#include "kconfig.h"
#include <klocale.h>
#include <kmainwindow.h>
#include <kstandarddirs.h>
#include "kfiledialog.h"
#include <kmenubar.h>
#include <ktoolbar.h>
#include <kstatusbar.h>
#include "kmessagebox.h"
#include "kurl.h"
#undef  GrayScale
#include <kprinter.h>

// local includes
#include "FktDlg.h"
#include "KoordDlg.h"
#include "RstDlg.h"
#include "SkalDlg.h"
#include "SwDlg.h"
#include "BezWnd.h"
#include "misc.h"
#include "View.h"


class MainDlg : public KMainWindow
{
	Q_OBJECT

public:
	MainDlg( const QString sessionId, KCmdLineArgs* args, const char* name = NULL );
	virtual ~MainDlg();
	friend class FktDlg;
	friend class BezWnd;

public slots:
	void neu();
	void save();
	void saveas();
	void load();
	void print();
	void bezeichnungen();
	void funktionen();
	void skalierung();
	void schrittw();
	void raster();
	void achsen();
	void onachsen1();
	void onachsen2();
	void onachsen3();
	void hilfe();
	void slotSettings();

private:
	void setupActions();
	void doSave( QString filename );
	void setupStatusBar();
	void addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue );
	void openFile( QString d );

	void parseAxes( const QDomElement &n );
	void parseGrid( const QDomElement &n );
	void parseScale( const QDomElement &n );
	void parseStep( const QDomElement &n );
	void parseFunction( const QDomElement &n );
	int tbid,
	stbid;
	KStatusBar *stbar;
	FktDlg *fdlg;
	BezWnd *bez;
	KToggleAction *view_bezeichnungen;
	View *view;
	QString m_sessionId;

private slots:
  void newToolbarConfig();
  void optionsConfigureKeys();
  void optionsConfigureToolbars();

};
#endif // MainDlg_included
