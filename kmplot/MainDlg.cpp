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
#include <qdom.h>
#include <qfile.h>
#include <qtooltip.h>
#include <qtabwidget.h>

// KDE includes
#include <kactionclasses.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <klineedit.h>
#include <kurl.h>

// local includes
#include "MainDlg.h"
#include "MainDlg.moc"
#include "misc.h"
#include "kprinterdlg.h"
#include "settings.h"
#include "settingspagecolor.h"
#include "settingspagecoords.h"
#include "settingspagescaling.h"
#include "settingspagefonts.h"
#include "keditfunction.h"

MainDlg::MainDlg( const QString sessionId, KCmdLineArgs* args, const char* name ) : KMainWindow( 0, name ), m_recentFiles( 0 )
{
	init();
	fdlg = 0;
	bez = 0;
	view = new View( this );
	setCentralWidget( view );
	m_quickEdit = new KLineEdit( this );
	QToolTip::add( m_quickEdit, i18n( "enter a function equation, for instance: f(x)=x^2" ) );
	setupActions();
	setupStatusBar();
	m_sessionId = sessionId;
	if (args -> count() > 0) 
	{
		m_filename = args -> url( 0 ).url();
		openFile( m_filename );
	}
	m_config = kapp->config();
	m_recentFiles->loadEntries( m_config );
	
	// Let's create a Configure Diloag
	m_settingsDialog = new KConfigDialog( this, "settings", Settings::self() ); 
	color_settings = new SettingsPageColor( 0, "colorSettings" ); 
	coords_settings = new SettingsPageCoords( 0, "coordsSettings" ); 
	scaling_settings = new SettingsPageScaling( 0, "scalingSettings" ); 
	fonts_settings = new SettingsPageFonts( 0, "fontsSettings" ); 
 
	m_settingsDialog->addPage( color_settings, i18n( "Colors" ), "colorize" ); 
	m_settingsDialog->addPage( coords_settings, i18n( "Coords" ), "coords" ); 
	m_settingsDialog->addPage( scaling_settings, i18n( "Scaling" ), "scaling" ); 
	m_settingsDialog->addPage( fonts_settings, i18n( "Fonts" ), "fonts" ); 
 
	// User edited the configuration - update your local copies of the 
	// configuration data 
	connect( m_settingsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) ); 
}

MainDlg::~MainDlg()
{
	m_recentFiles->saveEntries( m_config );
}

void MainDlg::setupActions()
{
	// standard actions
	KStdAction::openNew( this, SLOT( neu() ), actionCollection() );
	KStdAction::open( this, SLOT( load() ), actionCollection() );
	m_recentFiles = KStdAction::openRecent( this, SLOT( openRecent( const KURL& ) ), actionCollection());
	KStdAction::print( this, SLOT( print() ), actionCollection() );
	KStdAction::save( this, SLOT( save() ), actionCollection() );
	KStdAction::saveAs( this, SLOT( saveas() ), actionCollection() );
	KStdAction::quit( kapp, SLOT( closeAllWindows() ), actionCollection() );
	connect( kapp, SIGNAL( lastWindowClosed() ), kapp, SLOT( quit() ) );
	KStdAction::helpContents( this, SLOT( hilfe() ), actionCollection(), "helpcontents" );
	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
	KStdAction::preferences( this, SLOT( slotSettings() ), actionCollection());

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);
	
	// KmPLot specific actions
	// file menu
	( void ) new KAction( i18n( "E&xport..." ), 0, this, SLOT( doexport() ), actionCollection(), "export");
	
	// edit menu
	( void ) new KAction( i18n( "&Colors..." ), "colorize.png", 0, this, SLOT( editColors() ), actionCollection(), "editcolors" );
	( void ) new KAction( i18n( "&Axes..." ), "coords.png", 0, this, SLOT( editAxes() ), actionCollection(), "editaxes" );
	( void ) new KAction( i18n( "&Grid..." ), "coords.png", 0, this, SLOT( editGrid() ), actionCollection(), "editgrid" );
	( void ) new KAction( i18n( "&Scaling..." ), "scaling", 0, this, SLOT( editScaling() ), actionCollection(), "editscaling" );
	( void ) new KAction( i18n( "&Fonts..." ), "fonts", 0, this, SLOT( editFonts() ), actionCollection(), "editfonts" );
	
	( void ) new KAction( i18n( "Coordinate System I" ), "ksys1.png", 0, this, SLOT( onachsen1() ), actionCollection(), "coord_i" );
	( void ) new KAction( i18n( "Coordinate System II" ), "ksys2.png", 0, this, SLOT( onachsen2() ), actionCollection(), "coord_ii" );
	( void ) new KAction( i18n( "Coordinate System III" ), "ksys3.png", 0, this, SLOT( onachsen3() ), actionCollection(), "coord_iii" );
	
	( void ) new KAction( i18n( "&Step..." ), 0, this, SLOT( schrittw() ), actionCollection(), "step" );

	// functions menu	
	( void ) new KAction( i18n( "&New Function Plot..." ), 0, this, SLOT( onNewFunction() ), actionCollection(), "newfunction" );
	( void ) new KAction( i18n( "New Parametric Plot..." ), 0, this, SLOT( onNewParametric() ), actionCollection(), "newparametric" );
	( void ) new KAction( i18n( "New Polar Plot..." ), 0, this, SLOT( onNewPolar() ), actionCollection(), "newpolar" );
	( void ) new KAction( i18n( "Edit Functions..." ), "kfkt.png", 0, this, SLOT( funktionen() ), actionCollection(), "functions" );

	// help menu
	view_bezeichnungen = new KToggleAction( i18n( "&Names" ), 0, this, SLOT( bezeichnungen() ), actionCollection(), "names" );

	
	connect( m_quickEdit, SIGNAL( returnPressed( const QString& ) ), this, SLOT( onQuickEdit( const QString& ) ) );
	KWidgetAction* quickEditAction =  new KWidgetAction( m_quickEdit, i18n( "Quick Edit" ), 0, this, 0, actionCollection(), "quickedit" );
	quickEditAction->setWhatsThis( i18n( "Enter a simple function equation here.\n"
		"For instance: f(x)=x^2\nFor more options use Functions->Edit Functions... menu." ) );
	
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


// Slots

void MainDlg::neu()
{
	init(); // set globals to default
	m_filename = ""; // empty filename == new file
	setCaption( m_filename );
	view->update();
}

void MainDlg::save()
{
	if ( m_filename.isEmpty() )            // if there is no file name set yet
		saveas();
	else
		doSave( m_filename );

}

void MainDlg::saveas()
{
	QString filename = KFileDialog::getSaveFileName( QDir::currentDirPath(), i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), this, i18n( "Save As" ) );
	if ( !filename.isEmpty() )
	{
		if ( filename.find( "." ) == -1 )            // no file extension
			filename += ".fkt"; // use fkt-type as default
		doSave( filename );
		m_filename = filename;
		m_recentFiles->addURL( KURL(m_filename) );
		setCaption( m_filename );
	}
}

void MainDlg::doexport()
{	QString filename = KFileDialog::getSaveFileName(QDir::currentDirPath(), 
		i18n("*.svg|Scalable Vector Graphics (*.svg)\n*.bmp|Bitmap 180dpi(*.bmp)\n*.png|Bitmap 180dpi (*.png)"),
		this, i18n("export") );
	if(!filename.isEmpty())
	{	if( filename.right(4).lower()==".svg")
		{	QPicture pic;
			view->draw(&pic, 2);
	        	pic.save( filename, "SVG");
		}
		
		else if( filename.right(4).lower()==".bmp")
		{	QPixmap pix(100, 100);
			view->draw(&pix, 3);
			pix.save( filename, "BMP");
		}
		
		else if( filename.right(4).lower()==".png")
		{	QPixmap pix(100, 100);
			view->draw(&pix, 3);
			pix.save( filename, "PNG");
		}
	}
}


// here the real storing is done...
void MainDlg::doSave( QString filename )
{
	// saving as xml by a QDomDocument
	QDomDocument doc( "kmpdoc" );
	// the root tag
	QDomElement root = doc.createElement( "kmpdoc" );
	doc.appendChild( root );

	// the axes tag
	QDomElement tag = doc.createElement( "axes" );

	tag.setAttribute( "color", QColor( AchsenFarbe ).name() );
	tag.setAttribute( "width", AchsenDicke );
	tag.setAttribute( "tic-width", TeilstrichDicke );
	tag.setAttribute( "tic-legth", TeilstrichLaenge );

	addTag( doc, tag, "mode", QString::number( mode ) );
	addTag( doc, tag, "xmin", xminstr );
	addTag( doc, tag, "xmax", xmaxstr );
	addTag( doc, tag, "ymin", yminstr );
	addTag( doc, tag, "ymax", ymaxstr );
	addTag( doc, tag, "xcoord", QString::number( koordx ) );
	addTag( doc, tag, "ycoord", QString::number( koordy ) );

	root.appendChild( tag );

	tag = doc.createElement( "grid" );

	tag.setAttribute( "color", QColor( GitterFarbe ).name() );
	tag.setAttribute( "width", GitterDicke );

	addTag( doc, tag, "mode", QString::number( g_mode ) );

	root.appendChild( tag );

	tag = doc.createElement( "scale" );

	addTag( doc, tag, "tic-x", tlgxstr );
	addTag( doc, tag, "tic-y", tlgystr );
	addTag( doc, tag, "print-tic-x", drskalxstr );
	addTag( doc, tag, "print-tic-y", drskalystr );

	root.appendChild( tag );

	addTag( doc, root, "step", QString::number( rsw ) );

	for ( int ix = 0; ix < ps.ufanz; ix++ )
	{
		if ( !ps.fktext[ ix ].extstr.isEmpty() )
		{
			tag = doc.createElement( "function" );

			tag.setAttribute( "number", ix );
			tag.setAttribute( "visible", ps.fktext[ ix ].f_mode );
			tag.setAttribute( "visible-deriv", ps.fktext[ ix ].f1_mode );
			tag.setAttribute( "visible-2nd-deriv", ps.fktext[ ix ].f2_mode );
			tag.setAttribute( "width", ps.fktext[ ix ].dicke );
			tag.setAttribute( "color", QColor( ps.fktext[ ix ].farbe ).name() );

			addTag( doc, tag, "equation", ps.fktext[ ix ].extstr );

			root.appendChild( tag );
		}
	}

	QFile xmlfile( filename );
	xmlfile.open( IO_WriteOnly );
	QTextStream ts( &xmlfile );
	doc.save( ts, 4 );
	xmlfile.close();
}

void MainDlg::addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue )
{
	QDomElement tag = doc.createElement( tagName );
	QDomText value = doc.createTextNode( tagValue );
	tag.appendChild( value );
	parentTag.appendChild( tag );
}

void MainDlg::load()
{
	QString filename = KFileDialog::getOpenFileName( QDir::currentDirPath(), 
		i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), this, i18n( "Open" ) );
	if ( filename.isEmpty() ) return ;
	openFile( filename );
	m_filename = filename;
	m_recentFiles->addURL( KURL(m_filename) );
	setCaption( m_filename );
}

void MainDlg::openRecent( const KURL &url )
{
	openFile( url.path() );
	m_filename = url.path();
	setCaption( m_filename );
}


void MainDlg::openFile( const QString filename )
{
	init();

	QDomDocument doc( "kmpdoc" );

	QFile f( filename );
	if ( !f.open( IO_ReadOnly ) )
		return ;
	if ( !doc.setContent( &f ) )
	{
		f.close();
		return ;
	}
	f.close();

	QDomElement element = doc.documentElement();
	for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
	{
		if ( n.nodeName() == "axes" )
			parseAxes( n.toElement() );
		if ( n.nodeName() == "grid" )
			parseGrid( n.toElement() );
		if ( n.nodeName() == "scale" )
			parseScale( n.toElement() );
		if ( n.nodeName() == "step" )
			parseStep( n.toElement() );
		if ( n.nodeName() == "function" )
			parseFunction( n.toElement() );
	}

	///////////
	// postprocessing loading
	switch ( koordx )
	{
	case 0:
		xmin = -8.0;
		xmax = 8.0;
		break;
	case 1:
		xmin = -5.0;
		xmax = 5.5;
		break;
	case 2:
		xmin = 0.0;
		xmax = 16.0;
		break;
	case 3:
		xmin = 0.0;
		xmax = 10.0;
		break;
	case 4:
		xmin = ps.eval( xminstr );
		xmax = ps.eval( xmaxstr );
	}
	switch ( koordy )
	{
	case 0:
		ymin = -8.0;
		ymax = 8.0;
		break;
	case 1:
		ymin = -5.0;
		ymax = 5.5;
		break;
	case 2:
		ymin = 0.0;
		ymax = 16.0;
		break;
	case 3:
		ymin = 0.0;
		ymax = 10.0;
		break;
	case 4:
		ymin = ps.eval( yminstr );
		ymax = ps.eval( ymaxstr );
	}
	
	view->update();
}

void MainDlg::parseAxes( const QDomElement & n )
{
	kdDebug() << "parsing axes" << endl;

	AchsenDicke = n.attribute( "width", "1" ).toInt();
	AchsenFarbe = QColor( n.attribute( "color", "#000000" ) ).rgb();
	TeilstrichDicke = n.attribute( "tic-width", "3" ).toInt();
	TeilstrichLaenge = n.attribute( "tic-length", "10" ).toInt();

	mode = n.namedItem( "mode" ).toElement().text().toInt();
	xminstr = n.namedItem( "xmin" ).toElement().text();
	xmaxstr = n.namedItem( "xmax" ).toElement().text();
	yminstr = n.namedItem( "ymin" ).toElement().text();
	ymaxstr = n.namedItem( "ymax" ).toElement().text();
	koordx = n.namedItem( "xcoord" ).toElement().text().toInt();
	koordy = n.namedItem( "ycoord" ).toElement().text().toInt();
}

void MainDlg::parseGrid( const QDomElement & n )
{
	kdDebug() << "parsing grid" << endl;

	GitterFarbe = QColor( n.attribute( "color", "#c0c0c0" ) ).rgb();
	GitterDicke = n.attribute( "width", "1" ).toInt();

	g_mode = n.namedItem( "mode" ).toElement().text().toInt();
}

void MainDlg::parseScale( const QDomElement & n )
{
	kdDebug() << "parsing scale" << endl;

	tlgxstr = n.namedItem( "tic-x" ).toElement().text();
	tlgystr = n.namedItem( "tic-y" ).toElement().text();
	drskalxstr = n.namedItem( "print-tic-x" ).toElement().text();
	drskalystr = n.namedItem( "print-tic-y" ).toElement().text();

	tlgx = ps.eval( tlgxstr );
	tlgy = ps.eval( tlgystr );
	drskalx = ps.eval( drskalxstr );
	drskaly = ps.eval( drskalystr );
}

void MainDlg::parseStep( const QDomElement & n )
{
	kdDebug() << "parsing step" << endl;
	rsw = n.text().toDouble();
}

void MainDlg::parseFunction( const QDomElement & n )
{
	kdDebug() << "parsing function" << endl;

	int ix = n.attribute( "number" ).toInt();
	ps.fktext[ ix ].f_mode = n.attribute( "visible" ).toInt();
	ps.fktext[ ix ].f1_mode = n.attribute( "visible-deriv" ).toInt();
	ps.fktext[ ix ].f2_mode = n.attribute( "visible-2nd-deriv" ).toInt();
	ps.fktext[ ix ].dicke = n.attribute( "width" ).toInt();
	ps.fktext[ ix ].farbe = QColor( n.attribute( "color" ) ).rgb();

	ps.fktext[ ix ].extstr = n.namedItem( "equation" ).toElement().text();
	QCString fstr = ps.fktext[ ix ].extstr.utf8();
	if ( !fstr.isEmpty() )
	{
		int i = fstr.find( ';' );
		QCString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		ix = ps.addfkt( str );
		ps.getext( ix );
	}
}

void MainDlg::print()
{	KPrinter prt( QPrinter::PrinterResolution );
	prt.setResolution(72);
	prt.addDialogPage( new KPrinterDlg( this, "KmPlot page" ) );
	if ( prt.setup( this, i18n("Print Function") ) )
	{	prt.setFullPage(true);
		printtable = prt.option( "app-kmplot-printtable" ) != "-1";
		view->draw(&prt, 1);
	}
}

void MainDlg::editColors()
{
	m_settingsDialog->showPage( 0 );
	m_settingsDialog->show();
}

void MainDlg::editAxes()
{
	m_settingsDialog->showPage( 1 );
	coords_settings->tabs->setCurrentPage( 0 );
	m_settingsDialog->show();
}

void MainDlg::editGrid()
{
	m_settingsDialog->showPage( 1 );
	coords_settings->tabs->setCurrentPage( 1 );
	m_settingsDialog->show();
}

void MainDlg::editScaling()
{
	m_settingsDialog->showPage( 2 );
	m_settingsDialog->show();
}

void MainDlg::editFonts()
{
	m_settingsDialog->showPage( 3 );
	m_settingsDialog->show();
}

void MainDlg::bezeichnungen()
{
	if ( !bez )
		bez = new BezWnd( this, "bez" );
	if ( view_bezeichnungen->isChecked() )
	{
		bez->show();
	}
	else
		bez->hide();
}

void MainDlg::onNewFunction()
{
	KEditFunction* editFunction = new KEditFunction( &ps, this );
	editFunction->initDialog( KEditFunction::Function );
	editFunction->exec();
	view->update();
}

void MainDlg::onNewParametric()
{
	KEditFunction* editFunction = new KEditFunction( &ps, this );
	editFunction->initDialog( KEditFunction::Parametric );
	editFunction->exec();
	view->update();
}

void MainDlg::onNewPolar()
{
	KEditFunction* editFunction = new KEditFunction( &ps, this );
	editFunction->initDialog( KEditFunction::Polar );
	editFunction->exec();
	view->update();
}

void MainDlg::funktionen()
{
	if ( !fdlg ) fdlg = new FktDlg( this ); // make the dialog only if not allready done
	fdlg->fillList(); // 
	QString tmpName = locate ( "tmp", "" ) + "kmplot-" + m_sessionId;
	doSave( tmpName );
	if( fdlg->exec() == QDialog::Rejected ) openFile( tmpName );
	QFile::remove( tmpName );
	view->update();
}

void MainDlg::onQuickEdit( const QString& f_str )
{
	int index = ps.addfkt( f_str );
	if( index == -1 ) 
	{
		ps.errmsg();
		m_quickEdit->setFocus();
		m_quickEdit->selectAll();
		return;
	}
	ps.fktext[ index ].extstr = f_str;;
	ps.getext( index );
	m_quickEdit->clear();
	view->update();
}

void MainDlg::skalierung()
{
/*	SkalDlg skdlg;

	skdlg.exec();
	view->update();*/
	m_settingsDialog->showPage( 1 );
	coords_settings->tabs->setCurrentPage( 1 );
	KConfigDialog::showDialog( "settings" );
}

void MainDlg::schrittw()
{
	SwDlg sdlg;

	sdlg.exec();
	view->update();
}

void MainDlg::raster()
{
	RstDlg rdlg( this, "rdlg" );

	rdlg.exec();
	view->update();
}

void MainDlg::achsen()
{
	KoordDlg kdlg;

	kdlg.exec();
	view->update();
}

void MainDlg::onachsen1()
{
	koordx = koordy = 0;
	xmin = ymin = -8.;
	xmax = ymax = 8.;
	view->update();
}

void MainDlg::onachsen2()
{
	koordx = 2;
	koordy = 0;
	xmin = 0.;
	ymin = -8.;
	xmax = 16.;
	ymax = 8.;
	view->update();
}

void MainDlg::onachsen3()
{
	koordx = koordy = 2;
	xmin = ymin = 0.;
	xmax = ymax = 16.;
	view->update();
}

void MainDlg::hilfe()
{
	kapp->invokeHelp( "", "kmplot" );
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
