/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter Möller
*               2000, 2002 kdmoeller@foni.net
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
#include <qslider.h>
#include <qtooltip.h>

// KDE includes
#include <kapplication.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kio/netaccess.h> 
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <ktoolbar.h>

// local includes

#include "editfunction.h"
#include "keditparametric.h"
#include "keditpolar.h"
#include "kprinterdlg.h"
#include "kconstanteditor.h"
#include "MainDlg.h"
#include "MainDlg.moc"
#include "settings.h"
#include "settingspagecolor.h"
#include "settingspagecoords.h"
#include "settingspagefonts.h"
#include "settingspageprecision.h"
#include "settingspagescaling.h"
#include "sliderwindow.h"
class XParser;
class KmPlotIO;

bool MainDlg::oldfileversion;

MainDlg::MainDlg( KCmdLineArgs* args, const char* name ) : KMainWindow( 0, name ),  m_recentFiles( 0 ), m_modified(false)
{
	fdlg = 0;
	m_popupmenu = new KPopupMenu(this);
	view = new View( m_modified, m_popupmenu, this );
	setCentralWidget( view );
	view->setFocusPolicy(QWidget::ClickFocus);
	minmaxdlg = new KMinMax(view);
	view->setMinMaxDlg(minmaxdlg);
	m_quickEdit = new KLineEdit( this );
	QToolTip::add( m_quickEdit, i18n( "Enter a function equation, for example: f(x)=x^2" ) );

	setupStatusBar();
	setupActions();
	loadConstants();
	kmplotio = new KmPlotIO(view->parser());
	if (args -> count() > 0)
	{
		m_filename = args -> url( 0 ).url(-1);
		if (kmplotio->load( KURL(m_filename) ) )
		{
			setCaption( m_filename );
			view->updateSliders();
			view->drawPlot();
		}
	}
	m_config = kapp->config();
	m_recentFiles->loadEntries( m_config );

	// Let's create a Configure Diloag
	m_settingsDialog = new KConfigDialog( this, "settings", Settings::self() );
	m_settingsDialog->setHelp("general-config");
	
	// create and add the page(s)
	m_generalSettings = new SettingsPagePrecision( 0, "precisionSettings", "precision" );
	m_constantsSettings = new KConstantEditor( view, 0, "constantsSettings" );
	m_settingsDialog->addPage( m_generalSettings, i18n("General"), "package_settings", i18n("General Settings") );
	m_settingsDialog->addPage( m_constantsSettings, i18n("Constants"), "editconstants", i18n("Constants") );
	// User edited the configuration - update your local copies of the
	// configuration data
	connect( m_settingsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) );
}

MainDlg::~MainDlg()
{
	m_recentFiles->saveEntries( m_config );
	saveConstants();
	delete kmplotio;
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

	KStdAction::preferences( this, SLOT( slotSettings() ), actionCollection());
	m_fullScreen = KStdAction::fullScreen( NULL, NULL, actionCollection(), this, "fullscreen");
	connect( m_fullScreen, SIGNAL( toggled( bool )), this, SLOT( slotUpdateFullScreen( bool )));


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

	// plot menu
	( void ) new KAction( i18n( "&New Function Plot..." ), "newfunction", 0, this, SLOT( newFunction() ), actionCollection(), "newfunction" );
	( void ) new KAction( i18n( "New Parametric Plot..." ), "newparametric", 0, this, SLOT( newParametric() ), actionCollection(), "newparametric" );
	( void ) new KAction( i18n( "New Polar Plot..." ), "newpolar", 0, this, SLOT( newPolar() ), actionCollection(), "newpolar" );
	( void ) new KAction( i18n( "Edit Plots..." ), "editplots", 0, this, SLOT( slotEditPlots() ), actionCollection(), "editplots" );

	//zoom menu
	KRadioAction * mnuNoZoom = new KRadioAction(i18n("&No Zoom") ,"CTRL+0",view, SLOT( mnuNoZoom_clicked() ),actionCollection(),"no_zoom" );
	KRadioAction * mnuRectangular = new KRadioAction(i18n("Zoom &Rectangular"), "viewmagfit", "CTRL+1",view, SLOT( mnuRectangular_clicked() ),actionCollection(),"zoom_rectangular" );
	KRadioAction * mnuZoomIn = new KRadioAction(i18n("Zoom &In"), "viewmag+", "CTRL+2",view, SLOT( mnuZoomIn_clicked() ),actionCollection(),"zoom_in" );
	KRadioAction * mnuZoomOut = new KRadioAction(i18n("Zoom &Out"), "viewmag-", "CTRL+3",view, SLOT( mnuZoomOut_clicked() ),actionCollection(),"zoom_out" );
	KRadioAction * mnuZoomCenter = new KRadioAction(i18n("&Center Point") ,"CTRL+4",view, SLOT( mnuCenter_clicked() ),actionCollection(),"zoom_center" );
	(void ) new KAction(i18n("&Fit Widget to Trigonometric Functions") ,0,view, SLOT( mnuTrig_clicked() ),actionCollection(),"zoom_trig" );
	mnuNoZoom->setExclusiveGroup("zoom_modes");
	mnuNoZoom->setChecked(true);
	mnuRectangular->setExclusiveGroup("zoom_modes");
	mnuZoomIn->setExclusiveGroup("zoom_modes");
	mnuZoomOut->setExclusiveGroup("zoom_modes");
	mnuZoomCenter->setExclusiveGroup("zoom_modes");


	// tools menu
	KAction *mnuHide = new KAction(i18n("&Hide") ,0,view, SLOT( mnuHide_clicked() ),actionCollection(),"mnuhide" );
	mnuHide->plug(m_popupmenu);
	KAction *mnuRemove = new KAction(i18n("&Remove"),0,view, SLOT( mnuRemove_clicked() ),actionCollection(),"mnuremove"  );
	mnuRemove->plug(m_popupmenu);
	KAction *mnuEdit = new KAction(i18n("&Edit"), 0,view, SLOT( mnuEdit_clicked() ),actionCollection(),"mnuedit"  );
	mnuEdit->plug(m_popupmenu);

	KAction *mnuYValue =  new KAction( i18n( "&Get y-Value..." ), 0, this, SLOT( getYValue() ), actionCollection(), "yvalue" );
	KAction *mnuMinValue = new KAction( i18n( "&Search for Minimum Value..." ), "minimum", 0, this, SLOT( findMinimumValue() ), actionCollection(), "minimumvalue" );
	KAction *mnuMaxValue = new KAction( i18n( "&Search for Maximum Value..." ), "maximum", 0, this, SLOT( findMaximumValue() ), actionCollection(), "maximumvalue" );
	KAction *mnuArea = new KAction( i18n( "&Area Under Graph..." ), 0, this, SLOT( graphArea() ), actionCollection(), "grapharea" );

	// help menu
	( void ) new KAction( i18n( "Predefined &Math Functions" ), "functionhelp", 0, this, SLOT( slotNames() ), actionCollection(), "names" );


	connect( m_quickEdit, SIGNAL( returnPressed( const QString& ) ), this, SLOT( slotQuickEdit( const QString& ) ) );
	KWidgetAction* quickEditAction =  new KWidgetAction( m_quickEdit, i18n( "Quick Edit" ), 0, this, 0, actionCollection(), "quickedit" );
	quickEditAction->setWhatsThis( i18n( "Enter a simple function equation here.\n"
		"For instance: f(x)=x^2\nFor more options use Functions->Edit Plots... menu." ) );

	( void ) new KToggleAction( i18n( "Show Slider 1" ), 0, this, SLOT( toggleShowSlider0() ), actionCollection(), QString( "options_configure_show_slider_0" ).latin1() );
	( void ) new KToggleAction( i18n( "Show Slider 2" ), 0, this, SLOT( toggleShowSlider1() ), actionCollection(), QString( "options_configure_show_slider_1" ).latin1() );
	( void ) new KToggleAction( i18n( "Show Slider 3" ), 0, this, SLOT( toggleShowSlider2() ), actionCollection(), QString( "options_configure_show_slider_2" ).latin1() );
	( void ) new KToggleAction( i18n( "Show Slider 4" ), 0, this, SLOT( toggleShowSlider3() ), actionCollection(), QString( "options_configure_show_slider_3" ).latin1() );

	m_popupmenu->insertSeparator();
	mnuYValue->plug(m_popupmenu);
	mnuMinValue->plug(m_popupmenu);
	mnuMaxValue->plug(m_popupmenu);
	mnuArea->plug(m_popupmenu);

	setupGUI();
}

void MainDlg::setupStatusBar()
{
	stbar=statusBar();
	stbar->insertFixedItem( "1234567890", 1 );
	stbar->insertFixedItem( "1234567890", 2 );
	stbar->insertItem( "", 3, 3 );
	stbar->insertItem( "", 4 );
	stbar->changeItem( "", 1 );
	stbar->changeItem( "", 2 );
	stbar->setItemAlignment( 3, AlignLeft );
	view->progressbar = new KmPlotProgress( stbar );
	view->progressbar->setMaximumHeight( stbar->height()-10 );
	connect( view->progressbar->button, SIGNAL (clicked() ), view, SLOT( progressbar_clicked() ) );
	stbar->addWidget(view->progressbar);
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
				if ( m_modified) // the user didn't save the file
					return false;
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
	view->init(); // set globals to default
	m_filename = ""; // empty filename == new file
	setCaption( m_filename );
	view->updateSliders();
	view->drawPlot();
	m_modified = false;
	m_recentFiles->setCurrentItem( -1 );
}

void MainDlg::slotSave()
{
        if ( !m_modified) //don't save if no changes are made
                return;
	if ( m_filename.isEmpty() )            // if there is no file name set yet
		slotSaveas();
	else
	{
		if ( !m_modified) //don't save if no changes are made
			return;

		if ( oldfileversion)
		{
			if ( KMessageBox::warningYesNo( this, i18n( "This file is saved with an old file format; if you save it, you cannot open the file with older versions of Kmplot. Are you sure you want to continue?" ) ) == KMessageBox::No)
				return;
		}
		kmplotio->save( m_filename );
		kdDebug() << "saved" << endl;
		m_modified = false;
	}

}

void MainDlg::slotSaveas()
{
	KURL url = KFileDialog::getSaveURL( QDir::currentDirPath(), i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), this, i18n( "Save As" ) );
        
	if ( !url.isEmpty() )
	{
		// check if file exists and overwriting is ok.
		if( !KIO::NetAccess::exists( url,false,this ) || KMessageBox::warningContinueCancel( this, i18n( "A file named \"%1\" already exists. Are you sure you want to continue and overwrite this file?" ).arg( url.url()), i18n( "Overwrite File?" ), KGuiItem( i18n( "&Overwrite" ) ) ) == KMessageBox::Continue )
		{
			if ( !kmplotio->save( url ) )
                                KMessageBox::error(this, i18n("The file could not be saved") );
                        else
                        {
			     m_filename = url.url();
			     m_recentFiles->addURL( url );
			     setCaption( m_filename );
			     m_modified = false;
                        }
                        return;
		}
	}
}

void MainDlg::slotExport()
{
        KURL url = KFileDialog::getSaveURL(QDir::currentDirPath(),
		i18n("*.svg|Scalable Vector Graphics (*.svg)\n*.bmp|Bitmap 180dpi (*.bmp)\n*.png|Bitmap 180dpi (*.png)"),
		this, i18n("Export") );
	if(!url.isEmpty())
	{
               // check if file exists and overwriting is ok.
               if( KIO::NetAccess::exists(url,false,this ) && KMessageBox::warningContinueCancel( this, i18n( "A file named \"%1\" already exists. Are you sure you want to continue and overwrite this file?" ).arg(url.url() ), i18n( "Overwrite File?" ), KGuiItem( i18n( "&Overwrite" ) ) ) != KMessageBox::Continue ) return;

		if( url.fileName().right(4).lower()==".svg")
		{
			QPicture pic;
			view->draw(&pic, 2);
                        if (url.isLocalFile() )
	        	      pic.save( url.prettyURL(0,KURL::StripFileProtocol), "SVG");
                        else
                        {
                                KTempFile tmp;
                                pic.save( tmp.name(), "SVG");
                                if ( !KIO::NetAccess::upload(tmp.name(), url, 0) )
                                        KMessageBox::error(this, i18n("The url could not be saved") );
                                tmp.unlink();
                        }
		}

		else if( url.fileName().right(4).lower()==".bmp")
		{
			QPixmap pic(100, 100);
			view->draw(&pic, 3);
			if (url.isLocalFile() )
                              pic.save(  url.prettyURL(0,KURL::StripFileProtocol), "BMP");
                        else
                        {
                                KTempFile tmp;
                                pic.save( tmp.name(), "BMP");
                                if ( !KIO::NetAccess::upload(tmp.name(), url, 0) )
                                        KMessageBox::error(this, i18n("The url could not be saved") );
                                tmp.unlink();
                        }
		}

		else if( url.fileName().right(4).lower()==".png")
		{
			QPixmap pic(100, 100);
			view->draw(&pic, 3);
			if (url.isLocalFile() )
                              pic.save( url.prettyURL(0,KURL::StripFileProtocol), "PNG");
                        else
                        {
                                KTempFile tmp;
                                pic.save( tmp.name(), "PNG");
                                if ( !KIO::NetAccess::upload(tmp.name(), url, 0) )
                                        KMessageBox::error(this, i18n("The url could not be saved") );
                                tmp.unlink();
                        }
		}
	}
}

void MainDlg::slotOpen()
{
	if( !checkModified() ) return;
	KURL file = KFileDialog::getOpenURL( QDir::currentDirPath(),
		i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), this, i18n( "Open" ) );
	if ( file.isEmpty() )
                return;
	view->init();
	if ( !kmplotio->load( file ) )
		return;
	m_filename = file.url();
	m_recentFiles->addURL( file  );
	setCaption( m_filename );
	m_modified = false;
	view->updateSliders();
	view->drawPlot();
}

void MainDlg::slotOpenRecent( const KURL &url )
{
	if( !checkModified() ) return;
	view->init();
	if ( !kmplotio->load( url ) ) //if the loading fails
	{
		m_recentFiles->removeURL(url ); //remove the file from the recent-opened-file-list
		return;
	}
	view->updateSliders();
	view->drawPlot();
	m_filename = url.path();
	setCaption( m_filename );
	m_modified = false;
}

void MainDlg::slotPrint()
{
	KPrinter prt( QPrinter::PrinterResolution );
	prt.setResolution( 72 );
	prt.addDialogPage( new KPrinterDlg( this, "KmPlot page" ) );
	if ( prt.setup( this, i18n( "Print Plot" ) ) )
	{
		prt.setFullPage( true );
		view->draw(&prt, 1);
	}
}

void MainDlg::editColors()
{
	// create a config dialog and add a colors page
	KConfigDialog* colorsDialog = new KConfigDialog( this, "colors", Settings::self() );
	colorsDialog->setHelp("color-config");
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
	coordsDialog->setHelp("axes-config");
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
	scalingDialog->setHelp("scaling-config");
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
	fontsDialog->setHelp("font-config");
	fontsDialog->addPage( new SettingsPageFonts( 0, "fontsSettings" ), i18n( "Fonts" ), "fonts", i18n( "Edit Fonts" ) );
	// User edited the configuration - update your local copies of the
	// configuration data
	connect( fontsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) );
	fontsDialog->show();
}

void MainDlg::editConstants()
{
	QConstantEditor* contsDialog = new QConstantEditor();
	contsDialog->show();
}

void MainDlg::slotNames()
{
	kapp->invokeHelp( "func-predefined", "kmplot" );
}

void MainDlg::newFunction()
{
	EditFunction* editFunction = new EditFunction( view->parser(), this );
	editFunction->setCaption(i18n( "New Function Plot" ) );
	editFunction->initDialog();
	if ( editFunction->exec() == QDialog::Accepted )
	{
		m_modified = true;
		view->updateSliders();
		view->drawPlot();
	}
}

void MainDlg::newParametric()
{
	KEditParametric* editParametric = new KEditParametric( view->parser(), this );
	editParametric->setCaption(i18n( "New Parametric Plot"));
	editParametric->initDialog();
	if ( editParametric->exec() == QDialog::Accepted )
	{
		m_modified = true;
		view->drawPlot();
	}

}

void MainDlg::newPolar()
{
	KEditPolar* editPolar = new KEditPolar( view->parser(), this );
	editPolar->setCaption(i18n( "New Polar Plot"));
	editPolar->initDialog();
	if (  editPolar->exec() == QDialog::Accepted )
	{
		m_modified = true;
		view->drawPlot();
	}

}

void MainDlg::slotEditPlots()
{
	if ( !fdlg ) fdlg = new FktDlg( this, view->parser() ); // make the dialog only if not allready done
	fdlg->getPlots();
        KTempFile tmpfile;
	kmplotio->save( tmpfile.name() );
	if( fdlg->exec() == QDialog::Rejected )
	{
		if ( fdlg->isChanged() )
		{
			view->init();
			kmplotio->load( tmpfile.name() );
			view->drawPlot();
		}
	}
        else if ( fdlg->isChanged() )
        {
                view->updateSliders();
                m_modified = true;
        }
        tmpfile.unlink();
        
}

void MainDlg::slotQuickEdit(const QString& tmp_f_str )
{
	//creates a valid name for the function if the user has forgotten that
	QString f_str( tmp_f_str );
	view->parser()->fixFunctionName(f_str);
	if ( f_str.at(0)== 'x' || f_str.at(0)== 'y')
	{
		KMessageBox::error( this, i18n("Parametric functions must be definied in the \"New Parametric Plot\"-dialog which you can find in the menubar"));
		return;
	}
        if  ( f_str.contains('y') != 0)
        {
                KMessageBox::error( this, i18n( "Recursive function is not allowed"));
                m_quickEdit->setFocus();
                m_quickEdit->selectAll();
                return;
        }
        
	int const id = view->parser()->addfkt( f_str );
	if (id==-1)
	{
		view->parser()->parserError();
		m_quickEdit->setFocus();
		m_quickEdit->selectAll();
		return;
	}
        Ufkt *ufkt = &view->parser()->ufkt.last();
        view->parser()->prepareAddingFunction(ufkt);
         
        if ( view->parser()->getext( ufkt ) == -1)
        {
                m_quickEdit->setFocus();
                m_quickEdit->selectAll();
                view->parser()->Parser::delfkt( ufkt );
                return;
        }
	m_quickEdit->clear();
        m_modified = true;
	view->drawPlot();
}


void MainDlg::slotCoord1()
{
	Settings::setXRange( 0 );
	Settings::setYRange( 0 );
	m_modified = true;
	view->drawPlot();
}

void MainDlg::slotCoord2()
{
	Settings::setXRange( 2 );
	Settings::setYRange( 0 );
	m_modified = true;
	view->drawPlot();
}

void MainDlg::slotCoord3()
{
	Settings::setXRange( 2 );
	Settings::setYRange( 2 );
	m_modified = true;
	view->drawPlot();
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
	view->getSettings();
	m_modified = true;
	view->drawPlot();
}

bool MainDlg::queryClose()
{
	return checkModified() && KMainWindow::queryClose();
}

void MainDlg::slotUpdateFullScreen( bool checked)
{
	if (checked)
	{
		showFullScreen();
		m_fullScreen->plug( toolBar( "mainToolBar" ) );
	}
	else
	{
		showNormal();
		m_fullScreen->unplug( toolBar( "mainToolBar" ) );
	}

}

void MainDlg::loadConstants()
{

	KSimpleConfig conf ("kcalcrc");
	conf.setGroup("Constants");
	QString tmp;
	QString tmp_constant;
	char constant;
	double value;
	for( int i=0; ;i++)
	{
		tmp.setNum(i+1);
		tmp_constant = conf.readEntry("nameConstant"+tmp," ");
		value = conf.readDoubleNumEntry("valueConstant"+tmp,1.23456789);
		constant = tmp_constant.at(0).upper().latin1();

		if ( constant<'A' || constant>'Z')
			constant = 'A';
		if ( tmp_constant == " " || value == 1.23456789)
			return;
		
		if ( !view->parser()->constant.empty() )
		{
			bool copy_found=false;
			while (!copy_found)
			{
				// go through the constant list
				QValueVector<Constant>::iterator it =  view->parser()->constant.begin();
				while (it!= view->parser()->constant.end() && !copy_found)
				{
					if (constant == it->constant )
						copy_found = true;
					else
						++it;
				}
				if ( !copy_found)
					copy_found = true;
				else
				{
					copy_found = false;
					if (constant == 'Z')
						constant = 'A';
					else
						constant++;
				}
			}
		}
		/*kdDebug() << "**************" << endl;
		kdDebug() << "C:" << constant << endl;
		kdDebug() << "V:" << value << endl;*/

		view->parser()->constant.append(Constant(constant, value) );
	}
}

void MainDlg::saveConstants()
{
	KSimpleConfig conf ("kcalcrc");
	conf.deleteGroup("Constants");
	conf.setGroup("Constants");
	QString tmp;
	for( int i = 0; i< (int)view->parser()->constant.size();i++)
	{
		tmp.setNum(i+1);
		conf.writeEntry("nameConstant"+tmp, QString( QChar(view->parser()->constant[i].constant) ) ) ;
		conf.writeEntry("valueConstant"+tmp, view->parser()->constant[i].value);
	}
}

void MainDlg::getYValue()
{
	minmaxdlg->init(2);
	minmaxdlg->show();
}

void MainDlg::findMinimumValue()
{
	minmaxdlg->init(0);
	minmaxdlg->show();
}

void MainDlg::findMaximumValue()
{
	minmaxdlg->init(1);
	minmaxdlg->show();
}

void MainDlg::graphArea()
{
	minmaxdlg->init(3);
	minmaxdlg->show();
}

void MainDlg::toggleShowSlider0()
{
	if( !view->sliders[ 0 ]->isShown() ) view->sliders[ 0 ]->show();
	else view->sliders[ 0 ]->hide();
}

void MainDlg::toggleShowSlider1()
{
	if( !view->sliders[ 1 ]->isShown() ) view->sliders[ 1 ]->show();
	else view->sliders[ 1 ]->hide();
}

void MainDlg::toggleShowSlider2()
{
	if( !view->sliders[ 2 ]->isShown() ) view->sliders[ 2 ]->show();
	else view->sliders[ 2 ]->hide();
}

void MainDlg::toggleShowSlider3( )
{
	if( !view->sliders[ 3 ]->isShown() ) view->sliders[ 3 ]->show();
	else view->sliders[ 3 ]->hide();
}

