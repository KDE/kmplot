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

// KDE includes
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kkeydialog.h>
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

MainDlg::MainDlg( const QString sessionId, KCmdLineArgs* args, const char* name ) : KMainWindow( 0, name )
{
	init();
	fdlg = 0;
	bez = 0;
	view = new View( this );
	setCentralWidget( view );
	setupActions();
	setupStatusBar();
	m_sessionId = sessionId;
	if (args -> count() > 0) openFile( args -> url(0).fileName() );
}

MainDlg::~MainDlg()
{
}

void MainDlg::setupActions()
{
	// standard actions
	KStdAction::openNew( this, SLOT( neu() ), actionCollection() );
	KStdAction::open( this, SLOT( load() ), actionCollection() );
	KStdAction::print( this, SLOT( print() ), actionCollection() );
	KStdAction::save( this, SLOT( save() ), actionCollection() );
	KStdAction::saveAs( this, SLOT( saveas() ), actionCollection() );
	KStdAction::quit( kapp, SLOT( closeAllWindows() ), actionCollection() );
	connect( kapp, SIGNAL( lastWindowClosed() ), kapp, SLOT( quit() ) );
	KStdAction::helpContents( this, SLOT( hilfe() ), actionCollection(), "helpcontents" );

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);

	// KmPLot specific actions
	( void ) new KAction( i18n( "&Axes..." ), 0, this, SLOT( achsen() ), actionCollection(), "axes" );
	( void ) new KAction( i18n( "&Scale..." ), 0, this, SLOT( skalierung() ), actionCollection(), "scale" );
	( void ) new KAction( i18n( "&Grid..." ), 0, this, SLOT( raster() ), actionCollection(), "grid" );
	( void ) new KAction( i18n( "&Step..." ), 0, this, SLOT( schrittw() ), actionCollection(), "step" );
	view_bezeichnungen = new KToggleAction( i18n( "&Names" ), 0, this, SLOT( bezeichnungen() ), actionCollection(), "names" );

	( void ) new KAction( i18n( "Functions..." ), "kfkt.png", 0, this, SLOT( funktionen() ), actionCollection(), "functions" );
	( void ) new KAction( i18n( "Coordinate System I" ), "ksys1.png", 0, this, SLOT( onachsen1() ), actionCollection(), "coord_i" );
	( void ) new KAction( i18n( "Coordinate System II" ), "ksys2.png", 0, this, SLOT( onachsen2() ), actionCollection(), "coord_ii" );
	( void ) new KAction( i18n( "Coordinate System III" ), "ksys3.png", 0, this, SLOT( onachsen3() ), actionCollection(), "coord_iii" );

	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());
	KStdAction::preferences( this, SLOT( slotSettings() ), actionCollection());

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
	datei = ""; // empty filename == new file
	setCaption( datei );
	view->update();
}

void MainDlg::save()
{
	if ( datei.isEmpty() )            // if there is no file name set yet
		saveas();
	else
		doSave( datei );
}

void MainDlg::saveas()
{
	datei = KFileDialog::getSaveFileName( QDir::currentDirPath(), i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), this, i18n( "Save As" ) );
	if ( !datei.isEmpty() )
	{
		if ( datei.find( "." ) == -1 )            // no file extension
			datei = datei + ".fkt"; // use fkt-type as default
		doSave( datei );
		setCaption( datei );
	}
}

// here the real storing is done...
void MainDlg::doSave( QString filename )
{
	////////////
	// save as svg by drawing into a QPicture and saving it as svg
	/*    if ( datei.right( 4 ).lower() == ".svg" )
	    {
	        QPicture pic;
	        view->draw( &pic );
	        pic.save( datei, "svg" );
	        return ;
	    }*/

	///////////
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
	QString d = KFileDialog::getOpenFileName( QDir::currentDirPath(), i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), this, i18n( "Open" ) );
	if ( d.isEmpty() )
		return ;
	openFile(d);
	datei = d;
	setCaption( datei );
}

void MainDlg::openFile( QString filename )
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
{
	KPrinter prt( QPrinter::PrinterResolution );

	prt.addDialogPage( new KPrinterDlg( this, "KmPlot page" ) );

	if ( prt.setup( this, i18n("Print Function") ) )
	{
		prt.setFullPage( true );
		printtable = prt.option( "app-kmplot-printtable" ) != "-1";
		view->draw( &prt );
	}
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

void MainDlg::funktionen()
{
	if ( !fdlg ) fdlg = new FktDlg( this ); // make the dialog only if not allready done
	fdlg->fillList(); // 
	QString tmpName = locate ( "tmp", "" ) + "kmplot-" + m_sessionId;
	doSave( tmpName );
	if( fdlg->exec() == QDialog::Rejected ) openFile( tmpName );
	else QFile::remove( tmpName );
	view->update();
}

void MainDlg::skalierung()
{
	SkalDlg skdlg;

	skdlg.exec();
	view->update();
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
	// An instance of your dialog could be already created and could be cached, 
	// in which case you want to display the cached dialog instead of creating 
	// another one 
	if ( KConfigDialog::showDialog( "settings" ) ) return; 
 
	// KConfigDialog didn't find an instance of this dialog, so lets create it : 
	KConfigDialog* dialog = new KConfigDialog( this, "settings", Settings::self() ); 
	SettingsPageColor* color_settings = new SettingsPageColor( 0, "colorSettings" ); 
	SettingsPageCoords* coords_settings = new SettingsPageCoords( 0, "coordsSettings" ); 
	SettingsPageScaling* scaling_settings = new SettingsPageScaling( 0, "scalingSettings" ); 
	SettingsPageFonts* fonts_settings = new SettingsPageFonts( 0, "fontsSettings" ); 
 
	dialog->addPage( color_settings, i18n( "Colors" ), "colorize" ); 
	dialog->addPage( coords_settings, i18n( "Coords" ), "coords" ); 
	dialog->addPage( scaling_settings, i18n( "Scaling" ), "scaling" ); 
	dialog->addPage( fonts_settings, i18n( "Fonts" ), "fonts" ); 
 
	// User edited the configuration - update your local copies of the 
	// configuration data 
	// connect( dialog, SIGNAL(settingsChanged()), this, SLOT(updateConfiguration()) ); 
	// These Settings are not meant for the current plot but as default for next new plot.
	// So we dont need to update local member variables...
 
	dialog->show();
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
