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

// Qt includes
#include <qtooltip.h>
#include <qtabwidget.h>

// KDE includes
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <klineedit.h>

// local includes
#include "keditfunction.h"
#include "keditparametric.h"
#include "keditpolar.h"
#include "kmplotio.h"
#include "kprinterdlg.h"
#include "MainDlg.h"
#include "MainDlg.moc"
#include "settings.h"
#include "settingspagecolor.h"
#include "settingspagecoords.h"
#include "settingspagefonts.h"
#include "settingspageprecision.h"
#include "settingspagescaling.h"

MainDlg::MainDlg( const QString sessionId, KCmdLineArgs* args, const char* name ) : KMainWindow( 0, name ), m_recentFiles( 0 )
{
	init();
	fdlg = 0;
	bez = 0;
	view = new View( this );
	setCentralWidget( view );
	m_quickEdit = new KLineEdit( this );
	QToolTip::add( m_quickEdit, i18n( "Enter a function equation, for example: f(x)=x^2" ) );
	setupActions();
	setupStatusBar();
	m_sessionId = sessionId;
	if (args -> count() > 0) 
	{
		m_filename = args -> url( 0 ).url();
		KmPlotIO::load( m_filename );
		view->update();
	}
	m_config = kapp->config();
	m_recentFiles->loadEntries( m_config );
	
	// Let's create a Configure Diloag
	m_settingsDialog = new KConfigDialog( this, "settings", Settings::self() ); 
	// create and add the page(s)
	m_precisionSettings = new SettingsPagePrecision( 0, "precisionSettings" );
	m_settingsDialog->addPage( m_precisionSettings, i18n( "Precision" ), "" ); 
	// User edited the configuration - update your local copies of the 
	// configuration data 
	connect( m_settingsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) ); 
	m_modified = false;
}

MainDlg::~MainDlg()
{
	m_recentFiles->saveEntries( m_config );
}

void MainDlg::setupActions()
{
	// standard actions
	KStdAction::openNew( this, SLOT( slotOpenNew() ), actionCollection() );
	KStdAction::open( this, SLOT( slotOpen() ), actionCollection() );
	m_recentFiles = KStdAction::openRecent( this, SLOT( slotOpenRecent( const KURL& ) ), actionCollection());
	KStdAction::print( this, SLOT( slotPrint() ), actionCollection() );
	KStdAction::save( this, SLOT( slotSave() ), actionCollection() );
	KStdAction::saveAs( this, SLOT( slotSaveas() ), actionCollection() );
	KStdAction::quit( kapp, SLOT( closeAllWindows() ), actionCollection() );
	connect( kapp, SIGNAL( lastWindowClosed() ), kapp, SLOT( quit() ) );
	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
	KStdAction::preferences( this, SLOT( slotSettings() ), actionCollection());

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);
	
	// KmPLot specific actions
	// file menu
	( void ) new KAction( i18n( "E&xport..." ), 0, this, SLOT( slotExport() ), actionCollection(), "export");
	
	// edit menu
	( void ) new KAction( i18n( "&Colors..." ), "colorize.png", 0, this, SLOT( editColors() ), actionCollection(), "editcolors" );
	( void ) new KAction( i18n( "&Coordinate System..." ), "coords.png", 0, this, SLOT( editAxes() ), actionCollection(), "editaxes" );
// 	( void ) new KAction( i18n( "&Grid..." ), "coords.png", 0, this, SLOT( editGrid() ), actionCollection(), "editgrid" );
	( void ) new KAction( i18n( "&Scaling..." ), "scaling", 0, this, SLOT( editScaling() ), actionCollection(), "editscaling" );
	( void ) new KAction( i18n( "&Fonts..." ), "fonts", 0, this, SLOT( editFonts() ), actionCollection(), "editfonts" );
//	( void ) new KAction( i18n( "&Precision..." ), 0, this, SLOT( editPrecision() ), actionCollection(), "editprecision" );
	
	( void ) new KAction( i18n( "Coordinate System I" ), "ksys1.png", 0, this, SLOT( slotCoord1() ), actionCollection(), "coord_i" );
	( void ) new KAction( i18n( "Coordinate System II" ), "ksys2.png", 0, this, SLOT( slotCoord2() ), actionCollection(), "coord_ii" );
	( void ) new KAction( i18n( "Coordinate System III" ), "ksys3.png", 0, this, SLOT( slotCoord3() ), actionCollection(), "coord_iii" );

	// functions menu	
	( void ) new KAction( i18n( "&New Function Plot..." ), "kfkt.png", 0, this, SLOT( newFunction() ), actionCollection(), "newfunction" );
	( void ) new KAction( i18n( "New Parametric Plot..." ), 0, this, SLOT( newParametric() ), actionCollection(), "newparametric" );
	( void ) new KAction( i18n( "New Polar Plot..." ), 0, this, SLOT( newPolar() ), actionCollection(), "newpolar" );
	( void ) new KAction( i18n( "Edit Plots..." ), 0, this, SLOT( slotEditPlots() ), actionCollection(), "editplots" );

	// help menu
	view_names = new KToggleAction( i18n( "&Names" ), 0, this, SLOT( slotNames() ), actionCollection(), "names" );

	
	connect( m_quickEdit, SIGNAL( returnPressed( const QString& ) ), this, SLOT( slotQuickEdit( const QString& ) ) );
	KWidgetAction* quickEditAction =  new KWidgetAction( m_quickEdit, i18n( "Quick Edit" ), 0, this, 0, actionCollection(), "quickedit" );
	quickEditAction->setWhatsThis( i18n( "Enter a simple function equation here.\n"
		"For instance: f(x)=x^2\nFor more options use Functions->Edit Plots... menu." ) );
	
	createGUI( locate( "data", "kmplot/kmplotui.rc" ) );
}

void MainDlg::setupStatusBar()
{   
	stbar=statusBar();
	stbar->insertFixedItem("1234567890", 1);
	stbar->insertFixedItem("1234567890", 2);
	stbar->insertItem("", 3, 1);
	stbar->changeItem("", 1);
	stbar->changeItem("", 2);
	stbar->setItemAlignment(3, AlignLeft);
	view->stbar=stbar;
}


bool MainDlg::checkModified()
{
	if( m_modified )
	{
		int saveit = KMessageBox::warningYesNoCancel( this, i18n( "The plot has been modified.\n"
			"Do you want to save it?" ) );
		switch( saveit )
		{
			case KMessageBox::Yes:
				slotSave();
				break;
			case KMessageBox::Cancel:
				return false;
		}
	}
	return true;
}

// Slots

void MainDlg::slotOpenNew()
{
	if( !checkModified() ) return;
	init(); // set globals to default
	m_filename = ""; // empty filename == new file
	setCaption( m_filename );
	view->update();
	m_modified = false;
}

void MainDlg::slotSave()
{
	if ( m_filename.isEmpty() )            // if there is no file name set yet
		slotSaveas();
	else
	{
		KmPlotIO::save( m_filename );
		m_modified = false;
	}

}

void MainDlg::slotSaveas()
{
	QString filename = KFileDialog::getSaveFileName( QDir::currentDirPath(), i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), this, i18n( "Save As" ) );
	if ( !filename.isEmpty() )
	{
		if ( filename.find( "." ) == -1 )            // no file extension
			filename += ".fkt"; // use fkt-type as default
		KmPlotIO::save( filename );
		m_filename = filename;
		m_recentFiles->addURL( KURL(m_filename) );
		setCaption( m_filename );
		m_modified = false;
	}
}

void MainDlg::slotExport()
{	QString filename = KFileDialog::getSaveFileName(QDir::currentDirPath(), 
		i18n("*.svg|Scalable Vector Graphics (*.svg)\n*.bmp|Bitmap 180dpi(*.bmp)\n*.png|Bitmap 180dpi (*.png)"),
		this, i18n("export") );
	if(!filename.isEmpty())
	{	
		if( filename.right(4).lower()==".svg")
		{	
			QPicture pic;
			view->draw(&pic, 2);
	        	pic.save( filename, "SVG");
		}
		
		else if( filename.right(4).lower()==".bmp")
		{	
			QPixmap pix(100, 100);
			view->draw(&pix, 3);
			pix.save( filename, "BMP");
		}
		
		else if( filename.right(4).lower()==".png")
		{	
			QPixmap pix(100, 100);
			view->draw(&pix, 3);
			pix.save( filename, "PNG");
		}
	}
}

void MainDlg::slotOpen()
{
	if( !checkModified() ) return;
	QString filename = KFileDialog::getOpenFileName( QDir::currentDirPath(), 
		i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), this, i18n( "Open" ) );
	if ( filename.isEmpty() ) return ;
	KmPlotIO::load( filename );
	view->update();
	m_filename = filename;
	m_recentFiles->addURL( KURL( m_filename ) );
	setCaption( m_filename );
	m_modified = false;
}

void MainDlg::slotOpenRecent( const KURL &url )
{
	if( !checkModified() ) return;
	KmPlotIO::load( url.path() );
	view->update();
	m_filename = url.path();
	setCaption( m_filename );
	m_modified = false;
}

void MainDlg::slotPrint()
{	
	KPrinter prt( QPrinter::PrinterResolution );
	prt.setResolution(72);
	prt.addDialogPage( new KPrinterDlg( this, "KmPlot page" ) );
	if ( prt.setup( this, i18n("Print Function") ) )
	{	
		prt.setFullPage(true);
		printtable = prt.option( "app-kmplot-printtable" ) != "-1";
		view->draw(&prt, 1);
	}
}

void MainDlg::editColors()
{
	// create a config dialog and add a colors page
	KConfigDialog* colorsDialog = new KConfigDialog( this, "colors", Settings::self() ); 
	colorsDialog->addPage( new SettingsPageColor( 0, "colorSettings" ), i18n( "Colors" ), "colorize", i18n( "Edit Colors" ) ); 
	
	// User edited the configuration - update your local copies of the 
	// configuration data 
	connect( colorsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) ); 
	colorsDialog->show();
}

void MainDlg::editAxes()
{
	// create a config dialog and add a colors page
	KConfigDialog* coordsDialog = new KConfigDialog( this, "coords", Settings::self() ); 
	coordsDialog->addPage( new SettingsPageCoords( 0, "coordsSettings" ), i18n( "Coords" ), "coords", i18n( "Edit Coordinate System" ) ); 
	// User edited the configuration - update your local copies of the 
	// configuration data 
	connect( coordsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) ); 
	coordsDialog->show();
}

void MainDlg::editScaling()
{
	// create a config dialog and add a colors page
	KConfigDialog* scalingDialog = new KConfigDialog( this, "scaling", Settings::self() ); 
	scalingDialog->addPage( new SettingsPageScaling( 0, "scalingSettings" ), i18n( "Scale" ), "scaling", i18n( "Edit Scaling" ) ); 
	// User edited the configuration - update your local copies of the 
	// configuration data 
	connect( scalingDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) ); 
	scalingDialog->show();
}

void MainDlg::editFonts()
{
	// create a config dialog and add a colors page
	KConfigDialog* fontsDialog = new KConfigDialog( this, "fonts", Settings::self() ); 
	fontsDialog->addPage( new SettingsPageFonts( 0, "fontsSettings" ), i18n( "Fonts" ), "fonts", i18n( "Edit Fonts" ) ); 
	// User edited the configuration - update your local copies of the 
	// configuration data 
	connect( fontsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) ); 
	fontsDialog->show();
}

void MainDlg::slotNames()
{
	if ( !bez )
		bez = new BezWnd( this, "bez" );
	if ( view_names->isChecked() )
	{
		bez->show();
	}
	else
		bez->hide();
}

void MainDlg::newFunction()
{
	KEditFunction* editFunction = new KEditFunction( &ps, this );
	editFunction->initDialog();
	m_modified = editFunction->exec() == QDialog::Accepted;
	view->update();
}

void MainDlg::newParametric()
{
	KEditParametric* editParametric = new KEditParametric( &ps, this );
	editParametric->initDialog();
	m_modified = editParametric->exec() == QDialog::Accepted;
	view->update();
}

void MainDlg::newPolar()
{
	KEditPolar* editPolar = new KEditPolar( &ps, this );
	editPolar->initDialog();
	m_modified = editPolar->exec() == QDialog::Accepted;
	view->update();
}

void MainDlg::slotEditPlots()
{
	if ( !fdlg ) fdlg = new FktDlg( this, &ps ); // make the dialog only if not allready done
	fdlg->getPlots(); // 
	QString tmpName = locate ( "tmp", "" ) + "kmplot-" + m_sessionId;
	KmPlotIO::save( tmpName );
	if( fdlg->exec() == QDialog::Rejected ) KmPlotIO::load( tmpName );
	else m_modified = true;
	QFile::remove( tmpName );
	view->update();
}

void MainDlg::slotQuickEdit( const QString& f_str )
{
	int index = ps.addfkt( f_str );
	if( index == -1 ) 
	{
		ps.errmsg();
		m_quickEdit->setFocus();
		m_quickEdit->selectAll();
		return;
	}
	ps.fktext[ index ].extstr = f_str;
	ps.getext( index );
	m_quickEdit->clear();
	m_modified = true;
	view->update();
}


void MainDlg::slotCoord1()
{
	Settings::setXRange( 0 );
	Settings::setYRange( 0 );
	getSettings();
	m_modified = true;
	view->update();
}

void MainDlg::slotCoord2()
{
	Settings::setXRange( 2 );
	Settings::setYRange( 0 );
	getSettings();
	m_modified = true;
	view->update();
}

void MainDlg::slotCoord3()
{
	Settings::setXRange( 2 );
	Settings::setYRange( 2 );
	getSettings();
	m_modified = true;
	view->update();
}

void MainDlg::slotSettings()
{
	// An instance of your dialog has already been created and has been cached, 
	// so we want to display the cached dialog instead of creating 
	// another one 
	KConfigDialog::showDialog( "settings" );
}

void MainDlg::updateSettings()
{
	getSettings();
	m_modified = true;
	view->update();
}

void MainDlg::newToolbarConfig()
{
	createGUI();
	applyMainWindowSettings( KGlobal::config(), autoSaveGroup() );
}

void MainDlg::optionsConfigureKeys()
{
  	KKeyDialog::configure(actionCollection());
}

void MainDlg::optionsConfigureToolbars()
{
	saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
	KEditToolbar dlg(actionCollection());
	connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(newToolbarConfig()));
	dlg.exec();
}

bool MainDlg::queryClose()
{
	return checkModified() && KMainWindow::queryClose();
}
