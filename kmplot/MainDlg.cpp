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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

// Qt includes
#include <qslider.h>
#include <qtooltip.h>

// KDE includes
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kio/netaccess.h>
#include <kinstance.h>
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
#include "settingspagefonts.h"
#include "settingspageprecision.h"
#include "settingspagescaling.h"
#include "ksliderwindow.h"

class XParser;
class KmPlotIO;

bool MainDlg::oldfileversion;

MainDlg::MainDlg(QWidget *parentWidget, const char *, QObject *parent, const char *name) :  DCOPObject( "MainDlg" ), KParts::ReadOnlyPart( parent, name ), m_recentFiles( 0 ), m_modified(false), m_parent(parentWidget)
{
	// we need an instance
	setInstance( KmPlotPartFactory::instance() );

	kdDebug() << "parentWidget->name():" << parentWidget->name() << endl;
	if ( QString(parentWidget->name()).startsWith("KmPlot") )
	{
		setXMLFile("kmplot_part.rc");
		m_readonly = false;
	}
	else
	{
		setXMLFile("kmplot_part_readonly.rc");
		m_readonly = true;
		new BrowserExtension(this); // better integration with Konqueror
	}
	fdlg = 0;
	coordsDialog = 0;
	m_popupmenu = new KPopupMenu(parentWidget);
	view = new View( m_readonly, m_modified, m_popupmenu, parentWidget );
	connect( view, SIGNAL( setStatusBarText(const QString &)), this, SLOT( setReadOnlyStatusBarText(const QString &) ) );
	setWidget( view );
	view->setFocusPolicy(QWidget::ClickFocus);
	minmaxdlg = new KMinMax(view, m_parent);
	view->setMinMaxDlg(minmaxdlg);
	m_quickEdit = new KLineEdit( parentWidget );
	m_quickEdit->setFocus();
	QToolTip::add( m_quickEdit, i18n( "Enter a function equation, for example: f(x)=x^2" ) );
	setupActions();
	loadConstants();
	kmplotio = new KmPlotIO(view->parser());
	m_config = kapp->config();
	m_recentFiles->loadEntries( m_config );

	// Let's create a Configure Diloag
	m_settingsDialog = new KConfigDialog( parentWidget, "settings", Settings::self() );
	m_settingsDialog->setHelp("general-config");

	// create and add the page(s)
	m_generalSettings = new SettingsPagePrecision( 0, "precisionSettings", "precision" );
	m_constantsSettings = new KConstantEditor( view, 0, "constantsSettings" );
	m_settingsDialog->addPage( m_generalSettings, i18n("General"), "package_settings", i18n("General Settings") );
	m_settingsDialog->addPage( m_constantsSettings, i18n("Constants"), "editconstants", i18n("Constants") );
	// User edited the configuration - update your local copies of the
	// configuration data
	connect( m_settingsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) );
    connect( view, SIGNAL( resetZoom() ), this, SLOT(resetZoom() ) );
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
	m_recentFiles = KStdAction::openRecent( this, SLOT( slotOpenRecent( const KURL& ) ), actionCollection(),"file_openrecent");
	KStdAction::print( this, SLOT( slotPrint() ), actionCollection(),"file_print" );
	KStdAction::save( this, SLOT( slotSave() ), actionCollection() );
	KStdAction::saveAs( this, SLOT( slotSaveas() ), actionCollection() );
	connect( kapp, SIGNAL( lastWindowClosed() ), kapp, SLOT( quit() ) );

	KAction *prefs  = KStdAction::preferences( this, SLOT( slotSettings() ), actionCollection());
	prefs->setText( i18n( "Configure KmPlot..." ) );
	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());


	// KmPlot specific actions
	// file menu
	( void ) new KAction( i18n( "E&xport..." ), 0, this, SLOT( slotExport() ), actionCollection(), "export");

	//zoom menu
	m_mnuNoZoom = new KRadioAction(i18n("&No Zoom") ,"CTRL+0",view, SLOT( mnuNoZoom_clicked() ),actionCollection(),"no_zoom" );
	KRadioAction * mnuRectangular = new KRadioAction(i18n("Zoom &Rectangular"), "viewmagfit", "CTRL+1",view, SLOT( mnuRectangular_clicked() ),actionCollection(),"zoom_rectangular" );
	KRadioAction * mnuZoomIn = new KRadioAction(i18n("Zoom &In"), "viewmag+", "CTRL+2",view, SLOT( mnuZoomIn_clicked() ),actionCollection(),"zoom_in" );
	KRadioAction * mnuZoomOut = new KRadioAction(i18n("Zoom &Out"), "viewmag-", "CTRL+3",view, SLOT( mnuZoomOut_clicked() ),actionCollection(),"zoom_out" );
	KRadioAction * mnuZoomCenter = new KRadioAction(i18n("&Center Point") ,"CTRL+4",view, SLOT( mnuCenter_clicked() ),actionCollection(),"zoom_center" );
	(void ) new KAction(i18n("&Fit Widget to Trigonometric Functions") ,0,view, SLOT( mnuTrig_clicked() ),actionCollection(),"zoom_trig" );
	m_mnuNoZoom->setExclusiveGroup("zoom_modes");
    m_mnuNoZoom->setChecked(true);
	mnuRectangular->setExclusiveGroup("zoom_modes");
	mnuZoomIn->setExclusiveGroup("zoom_modes");
	mnuZoomOut->setExclusiveGroup("zoom_modes");
	mnuZoomCenter->setExclusiveGroup("zoom_modes");

	// help menu
	( void ) new KAction( i18n( "Predefined &Math Functions" ), "functionhelp", 0, this, SLOT( slotNames() ), actionCollection(), "names" );

	// edit menu
	( void ) new KAction( i18n( "&Colors..." ), "colorize.png", 0, this, SLOT( editColors() ), actionCollection(), "editcolors" );
	( void ) new KAction( i18n( "&Coordinate System..." ), "coords.png", 0, this, SLOT( editAxes() ), actionCollection(), "editaxes" );
	//  ( void ) new KAction( i18n( "&Grid..." ), "coords.png", 0, this, SLOT( editGrid() ), actionCollection(), "editgrid" );
	( void ) new KAction( i18n( "&Scaling..." ), "scaling", 0, this, SLOT( editScaling() ), actionCollection(), "editscaling" );
	( void ) new KAction( i18n( "&Fonts..." ), "fonts", 0, this, SLOT( editFonts() ), actionCollection(), "editfonts" );

	( void ) new KAction( i18n( "Coordinate System I" ), "ksys1.png", 0, this, SLOT( slotCoord1() ), actionCollection(), "coord_i" );
	( void ) new KAction( i18n( "Coordinate System II" ), "ksys2.png", 0, this, SLOT( slotCoord2() ), actionCollection(), "coord_ii" );
	( void ) new KAction( i18n( "Coordinate System III" ), "ksys3.png", 0, this, SLOT( slotCoord3() ), actionCollection(), "coord_iii" );

	// plot menu
	( void ) new KAction( i18n( "&New Function Plot..." ), "newfunction", 0, this, SLOT( newFunction() ), actionCollection(), "newfunction" );
	( void ) new KAction( i18n( "New Parametric Plot..." ), "newparametric", 0, this, SLOT( newParametric() ), actionCollection(), "newparametric" );
	( void ) new KAction( i18n( "New Polar Plot..." ), "newpolar", 0, this, SLOT( newPolar() ), actionCollection(), "newpolar" );
	( void ) new KAction( i18n( "Edit Plots..." ), "editplots", 0, this, SLOT( slotEditPlots() ), actionCollection(), "editplots" );

		// tools menu
	KAction *mnuYValue =  new KAction( i18n( "&Get y-Value..." ), 0, this, SLOT( getYValue() ), actionCollection(), "yvalue" );
	KAction *mnuMinValue = new KAction( i18n( "&Search for Minimum Value..." ), "minimum", 0, this, SLOT( findMinimumValue() ), actionCollection(), "minimumvalue" );
	KAction *mnuMaxValue = new KAction( i18n( "&Search for Maximum Value..." ), "maximum", 0, this, SLOT( findMaximumValue() ), actionCollection(), "maximumvalue" );
	KAction *mnuArea = new KAction( i18n( "&Calculate Integral" ), 0, this, SLOT( graphArea() ), actionCollection(), "grapharea" );

	connect( m_quickEdit, SIGNAL( returnPressed( const QString& ) ), this, SLOT( slotQuickEdit( const QString& ) ) );
	KWidgetAction* quickEditAction =  new KWidgetAction( m_quickEdit, i18n( "Quick Edit" ), 0, this, 0, actionCollection(), "quickedit" );
	quickEditAction->setWhatsThis( i18n( "Enter a simple function equation here.\n"
	                                     "For instance: f(x)=x^2\nFor more options use Functions->Edit Plots... menu." ) );

	view->mnuSliders[0] = new KToggleAction( i18n( "Show Slider 1" ), 0, this, SLOT( toggleShowSlider0() ), actionCollection(), QString( "options_configure_show_slider_0" ).latin1() );
	view->mnuSliders[1] = new KToggleAction( i18n( "Show Slider 2" ), 0, this, SLOT( toggleShowSlider1() ), actionCollection(), QString( "options_configure_show_slider_1" ).latin1() );
	view->mnuSliders[2] = new KToggleAction( i18n( "Show Slider 3" ), 0, this, SLOT( toggleShowSlider2() ), actionCollection(), QString( "options_configure_show_slider_2" ).latin1() );
	view->mnuSliders[3] = new KToggleAction( i18n( "Show Slider 4" ), 0, this, SLOT( toggleShowSlider3() ), actionCollection(), QString( "options_configure_show_slider_3" ).latin1() );

	// Popup menu
	KAction *mnuHide = new KAction(i18n("&Hide") ,0,view, SLOT( mnuHide_clicked() ),actionCollection(),"mnuhide" );
	mnuHide->plug(m_popupmenu);
	KAction *mnuRemove = new KAction(i18n("&Remove"),"editdelete", 0,view, SLOT( mnuRemove_clicked() ),actionCollection(),"mnuremove"  );
	mnuRemove->plug(m_popupmenu);
	KAction *mnuEdit = new KAction(i18n("&Edit"),"editplots", 0,view, SLOT( mnuEdit_clicked() ),actionCollection(),"mnuedit"  );
	mnuEdit->plug(m_popupmenu);
	m_popupmenu->insertSeparator();
	KAction *mnuCopy = new KAction(i18n("&Copy"), 0,view, SLOT( mnuCopy_clicked() ),actionCollection(),"mnucopy"  );
	mnuCopy->plug(m_popupmenu);
	KAction *mnuMove = new KAction(i18n("&Move"), 0,view, SLOT( mnuMove_clicked() ),actionCollection(),"mnumove"  );
	mnuMove->plug(m_popupmenu);
	m_popupmenu->insertSeparator();
	mnuYValue->plug(m_popupmenu);
	mnuMinValue->plug(m_popupmenu);
	mnuMaxValue->plug(m_popupmenu);
	mnuArea->plug(m_popupmenu);
}
bool MainDlg::checkModified()
{
	if( m_modified )
	{
		int saveit = KMessageBox::warningYesNoCancel( m_parent, i18n( "The plot has been modified.\n"
		             "Do you want to save it?" ), QString::null, KStdGuiItem::save(), KStdGuiItem::discard() );
		switch( saveit )
		{
			case KMessageBox::Yes:
				slotSave();
				if ( m_modified) // the user didn't saved the file
					return false;
				break;
			case KMessageBox::Cancel:
				return false;
		}
	}
	return true;
}
/*
void MainDlg::slotCleanWindow()
{
	if (m_readonly)
		return;
	view->init(); // set globals to default
	view->updateSliders();
	view->drawPlot();
}
*/
void MainDlg::slotSave()
{
	if ( !m_modified || m_readonly) //don't save if no changes are made or readonly is enabled
		return;
	if ( m_url.isEmpty() )            // if there is no file name set yet
		slotSaveas();
	else
	{
		if ( !m_modified) //don't save if no changes are made
			return;

		if ( oldfileversion)
		{
			if ( KMessageBox::warningContinueCancel( m_parent, i18n( "This file is saved with an old file format; if you save it, you cannot open the file with older versions of Kmplot. Are you sure you want to continue?" ), QString::null, i18n("Save New Format") ) == KMessageBox::Cancel)
				return;
		}
		kmplotio->save( m_url.url() );
		kdDebug() << "saved" << endl;
		m_modified = false;
	}

}

void MainDlg::slotSaveas()
{
	if (m_readonly)
		return;
	const KURL url = KFileDialog::getSaveURL( QDir::currentDirPath(), i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), m_parent, i18n( "Save As" ) );

	if ( !url.isEmpty() )
	{
		// check if file exists and overwriting is ok.
		if( !KIO::NetAccess::exists( url,false,m_parent ) || KMessageBox::warningContinueCancel( m_parent, i18n( "A file named \"%1\" already exists. Are you sure you want to continue and overwrite this file?" ).arg( url.url()), i18n( "Overwrite File?" ), KGuiItem( i18n( "&Overwrite" ) ) ) == KMessageBox::Continue )
		{
			if ( !kmplotio->save( url ) )
				KMessageBox::error(m_parent, i18n("The file could not be saved") );
			else
			{
				m_url = url;
				m_recentFiles->addURL( url );
        setWindowCaption( m_url.prettyURL(0, KURL::StripFileProtocol) );
				m_modified = false;
			}
			return;
		}
	}
}

void MainDlg::slotExport()
{
	KURL const url = KFileDialog::getSaveURL(QDir::currentDirPath(),
	                 i18n("*.svg|Scalable Vector Graphics (*.svg)\n"
	                      "*.bmp|Bitmap 180dpi (*.bmp)\n"
	                      "*.png|Bitmap 180dpi (*.png)"), m_parent, i18n("Export") );
	if(!url.isEmpty())
	{
		// check if file exists and overwriting is ok.
		if( KIO::NetAccess::exists(url,false,m_parent ) && KMessageBox::warningContinueCancel( m_parent, i18n( "A file named \"%1\" already exists. Are you sure you want to continue and overwrite this file?" ).arg(url.url() ), i18n( "Overwrite File?" ), KGuiItem( i18n( "&Overwrite" ) ) ) != KMessageBox::Continue ) return;

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
					KMessageBox::error(m_parent, i18n("The URL could not be saved.") );
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
					KMessageBox::error(m_parent, i18n("The URL could not be saved.") );
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
					KMessageBox::error(m_parent, i18n("The URL could not be saved.") );
				tmp.unlink();
			}
		}
	}
}
bool MainDlg::openFile()
{
	view->init();
	if (m_url==m_currentfile || !kmplotio->load( m_url ) )
	{
		m_recentFiles->removeURL(m_url ); //remove the file from the recent-opened-file-list
		m_url = "";
		return false;
	}
	m_currentfile = m_url;
  m_recentFiles->addURL( m_url.prettyURL(0, KURL::StripFileProtocol)  );
  setWindowCaption( m_url.prettyURL(0, KURL::StripFileProtocol) );
	m_modified = false;
	view->updateSliders();
	view->drawPlot();
	return true;
}

void MainDlg::slotOpenRecent( const KURL &url )
{
	if( isModified() || !m_url.isEmpty() ) // open the file in a new window
	{
		QByteArray data;
		QDataStream stream(data, IO_WriteOnly);
		stream << url;
		KApplication::kApplication()->dcopClient()->send(KApplication::kApplication()->dcopClient()->appId(), "KmPlotShell","openFileInNewWindow(KURL)", data);
		return;
	}

	view->init();
	if ( !kmplotio->load( url ) ) //if the loading fails
	{
		m_recentFiles->removeURL(url ); //remove the file from the recent-opened-file-list
		return;
	}
  m_url = m_currentfile = url;
	m_recentFiles->setCurrentItem(-1); //don't select the item in the open-recent menu
  setWindowCaption( m_url.prettyURL(0, KURL::StripFileProtocol) );
  m_modified = false;
	view->updateSliders();
	view->drawPlot();
}

void MainDlg::slotPrint()
{
	KPrinter prt( QPrinter::PrinterResolution );
	prt.setResolution( 72 );
	prt.addDialogPage( new KPrinterDlg( m_parent, "KmPlot page" ) );
	if ( prt.setup( m_parent, i18n( "Print Plot" ) ) )
	{
		prt.setFullPage( true );
		view->draw(&prt, 1);
	}
}

void MainDlg::editColors()
{
	// create a config dialog and add a colors page
	KConfigDialog* colorsDialog = new KConfigDialog( m_parent, "colors", Settings::self() );
	colorsDialog->setHelp("color-config");
	colorsDialog->addPage( new SettingsPageColor( 0, "colorSettings" ), i18n( "Colors" ), "colorize", i18n( "Edit Colors" ) );

	// User edited the configuration - update your local copies of the
	// configuration data
	connect( colorsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) );
	colorsDialog->show();
}

void MainDlg::editAxes()
{
	// create a config dialog and add a axes page
	if ( !coordsDialog)
	{
		coordsDialog = new CoordsConfigDialog( view->parser(), m_parent);
		// User edited the configuration - update your local copies of the
		// configuration data
		connect( coordsDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) );
	}
	coordsDialog->show();
}

void MainDlg::editScaling()
{
	// create a config dialog and add a scaling page
	KConfigDialog *scalingDialog = new KConfigDialog( m_parent, "scaling", Settings::self() );
	scalingDialog->setHelp("scaling-config");
	scalingDialog->addPage( new SettingsPageScaling( 0, "scalingSettings" ), i18n( "Scale" ), "scaling", i18n( "Edit Scaling" ) );
	// User edited the configuration - update your local copies of the
	// configuration data
	connect( scalingDialog, SIGNAL( settingsChanged() ), this, SLOT(updateSettings() ) );
	scalingDialog->show();
}

void MainDlg::editFonts()
{
	// create a config dialog and add a font page
	KConfigDialog* fontsDialog = new KConfigDialog( m_parent, "fonts", Settings::self() );
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
	EditFunction* editFunction = new EditFunction( view->parser(), m_parent );
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
	KEditParametric* editParametric = new KEditParametric( view->parser(), m_parent );
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
	KEditPolar* editPolar = new KEditPolar( view->parser(), m_parent );
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
	if ( !fdlg ) fdlg = new FktDlg( m_parent, view ); // make the dialog only if not allready done
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

void MainDlg::slotQuickEdit(const QString& f_str_const )
{
	//creates a valid name for the function if the user has forgotten that
  	QString f_str( f_str_const );
	int const pos = f_str_const.find(';');
	if (pos!=-1)
	  f_str = f_str.left(pos);
	if (f_str.at(0)=='r')
	  	view->parser()->fixFunctionName(f_str, XParser::Polar);
	else
	  	view->parser()->fixFunctionName(f_str);
	if ( f_str.at(0)== 'x' || f_str.at(0)== 'y')
	{
		KMessageBox::error( m_parent, i18n("Parametric functions must be definied in the \"New Parametric Plot\"-dialog which you can find in the menubar"));
		return;
	}
	if  ( f_str.contains('y') != 0)
	{
		KMessageBox::error( m_parent, i18n( "Recursive function is not allowed"));
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

	if ( pos!=-1 && !view->parser()->getext(ufkt, QString(f_str_const)))
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

void MainDlg::loadConstants()
{
	KSimpleConfig conf ("kcalcrc");
	conf.setGroup("UserConstants");
	QString tmp;
	QString tmp_constant;
	QString tmp_value;
	char constant;
	double value;
	for( int i=0; ;i++)
	{
		tmp.setNum(i);
		tmp_constant = conf.readEntry("nameConstant"+tmp," ");
		tmp_value = conf.readEntry("valueConstant"+tmp," ");
// 		kdDebug() << "konstant: " << tmp_constant.latin1() << endl;
// 		kdDebug() << "value: " << value << endl;
// 		kdDebug() << "**************" << endl;
		
		if ( tmp_constant == " " || tmp_constant == " ")
		  return;
		
 		constant = tmp_constant.at(0).upper().latin1();

		if ( constant<'A' || constant>'Z')
			constant = 'A';
		value = view->parser()->eval(tmp_value);
		if ( view->parser()->parserError(false) ) //couln't parse the value
		  continue;
		
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
	conf.setGroup("UserConstants");
	QString tmp;
	for( int i = 0; i< (int)view->parser()->constant.size();i++)
	{
		tmp.setNum(i);
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
	toggleShowSlider(0);
}

void MainDlg::toggleShowSlider1()
{
	toggleShowSlider(1);
}

void MainDlg::toggleShowSlider2()
{
	toggleShowSlider(2);
}

void MainDlg::toggleShowSlider3()
{
	toggleShowSlider(3);
}

void MainDlg::toggleShowSlider(int const num)
{
	// create the slider if it not exists already
	if ( view->sliders[ num ] == 0 )
	{
		view->sliders[ num ] = new KSliderWindow( view, num);
		connect( view->sliders[num]->slider, SIGNAL( valueChanged( int ) ), view, SLOT( drawPlot() ) );
		connect( view->sliders[num], SIGNAL( windowClosed( int ) ), view, SLOT( sliderWindowClosed(int) ) );
	}
	if ( !view->sliders[ num ]->isShown() )
		view->sliders[ num ]->show();
	else
		view->sliders[ num ]->hide();
}

void MainDlg::setReadOnlyStatusBarText(const QString &text)
{
	setStatusBarText(text);
}

void MainDlg::optionsConfigureKeys()
{
	KApplication::kApplication()->dcopClient()->send(KApplication::kApplication()->dcopClient()->appId(), "KmPlotShell","optionsConfigureKeys()", QByteArray());
}

void MainDlg::optionsConfigureToolbars()
{
	KApplication::kApplication()->dcopClient()->send(KApplication::kApplication()->dcopClient()->appId(), "KmPlotShell","optionsConfigureToolbars()", QByteArray());
}

void MainDlg::resetZoom()
{
  m_mnuNoZoom->activate();
}

// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>

KInstance*  KmPlotPartFactory::s_instance = 0L;
KAboutData* KmPlotPartFactory::s_about = 0L;

KmPlotPartFactory::KmPlotPartFactory()
		: KParts::Factory()
{}

KmPlotPartFactory::~KmPlotPartFactory()
{
	delete s_instance;
	delete s_about;

	s_instance = 0L;
}

KParts::Part* KmPlotPartFactory::createPartObject( QWidget *parentWidget, const char *widgetName,
        QObject *parent, const char *name,
        const char *, const QStringList & )
{
	// Create an instance of our Part
	MainDlg* obj = new MainDlg( parentWidget, widgetName, parent, name );
	emit objectCreated( obj );
	return obj;
}

KInstance* KmPlotPartFactory::instance()
{
	if( !s_instance )
	{
		s_about = new KAboutData("kmplot",I18N_NOOP( "KmPlotPart" ), "1");
		s_instance = new KInstance(s_about);
	}
	return s_instance;
}

extern "C"
{
	KDE_EXPORT void* init_libkmplotpart()
	{
		return new KmPlotPartFactory;
	}
}


/// BrowserExtension class
BrowserExtension::BrowserExtension(MainDlg* parent)
		: KParts::BrowserExtension( parent, "KmPlot::BrowserExtension" )
{
	emit enableAction("print", true);
	setURLDropHandlingEnabled(true);
}

void BrowserExtension::print()
{
	static_cast<MainDlg*>(parent())->slotPrint();
}


