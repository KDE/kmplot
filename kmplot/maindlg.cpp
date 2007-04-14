/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter M�ler <kd.moeller@t-online.de>
*                                 2006  David Saxton <david@bluehaze.org>
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
#include <QMainWindow>
#include <QPainter>
#include <QPixmap>
#include <QSlider>
#include <QSvgGenerator>
#include <QTimer>

// KDE includes
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfigdialog.h>
#include <kconfigdialogmanager.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kimageio.h>
#include <kio/netaccess.h>
#include <kcomponentdata.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kstandardaction.h>
#include <ktemporaryfile.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>
#include <krecentfilesaction.h>
#include <kactioncollection.h>
#include <kicon.h>
#include <kiconloader.h>

// local includes
#include "calculator.h"
#include "functiontools.h"
#include "functioneditor.h"
#include "kprinterdlg.h"
#include "kconstanteditor.h"
#include "xparser.h"

#include "settings.h"
#include "maindlg.h"
#include "ui_settingspagecolor.h"
#include "ui_settingspagefonts.h"
#include "ui_settingspagegeneral.h"
#include "ui_settingspagediagram.h"
#include "ksliderwindow.h"
#include "maindlgadaptor.h"

#include <assert.h>

class XParser;
class KmPlotIO;


class SettingsPageColor : public QWidget, public Ui::SettingsPageColor
{
    public:
        SettingsPageColor( QWidget * parent = 0 )
    : QWidget( parent )
        {
			setupUi(this);
			layout()->setMargin(0);
		}
};

class SettingsPageFonts : public QWidget, public Ui::SettingsPageFonts
{
    public:
        SettingsPageFonts( QWidget * parent = 0 )
	: QWidget( parent )
		{
			setupUi(this);
			layout()->setMargin(0);
		}
};

class SettingsPageGeneral : public QWidget, public Ui::SettingsPageGeneral
{
    public:
        SettingsPageGeneral( QWidget * parent = 0 )
	: QWidget( parent )
		{
			setupUi(this);
			layout()->setMargin(0);
		}
};

class SettingsPageDiagram : public QWidget, public Ui::SettingsPageDiagram
{
	public:
		SettingsPageDiagram( QWidget * parent = 0 )
	: QWidget( parent )
		{
			setupUi(this);
			layout()->setMargin(0);
		}
};

bool MainDlg::oldfileversion;
MainDlg * MainDlg::m_self = 0;


//BEGIN class MainDlg
MainDlg::MainDlg(QWidget *parentWidget, QObject *parent, const QStringList& ) :
		KParts::ReadWritePart( parent ),
		m_recentFiles( 0 ),
		m_modified(false),
		m_parent(parentWidget)
{
	assert( !m_self ); // this class should only be constructed once
	m_self = this;

	// we need an instance
	setComponentData( KmPlotPartFactory::componentData() );

	kDebug() << "parentWidget->objectName():" << parentWidget->objectName() << endl;
	if ( QString(parentWidget->objectName()).startsWith("KmPlot") )
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

	m_coordsDialog = 0;
	m_constantEditor = 0;
	m_popupmenu = new KMenu( parentWidget );
	m_newPlotMenu = new KMenu( parentWidget );
	(void) new View( m_readonly, m_popupmenu, parentWidget );
	connect( View::self(), SIGNAL( setStatusBarText(const QString &)), this, SLOT( setReadOnlyStatusBarText(const QString &) ) );

	m_functionEditor = 0;
	if ( !m_readonly )
	{
		m_functionEditor = new FunctionEditor( m_newPlotMenu, parentWidget );
		static_cast<QMainWindow*>(parentWidget)->addDockWidget( Qt::LeftDockWidgetArea, m_functionEditor );
	}

	setWidget( View::self() );
	View::self()->setFocusPolicy(Qt::ClickFocus);
	m_functionTools = new FunctionTools(m_parent);
	m_calculator = new Calculator( m_parent );
	setupActions();
	XParser::self()->constants()->load();
	kmplotio = new KmPlotIO();
	m_config = KGlobal::config();
	m_recentFiles->loadEntries( m_config->group( QString() ) );


	//BEGIN undo/redo stuff
	m_currentState = kmplotio->currentState();
	m_saveCurrentStateTimer = new QTimer( this );
	m_saveCurrentStateTimer->setSingleShot( true );
	connect( m_saveCurrentStateTimer, SIGNAL(timeout()), this, SLOT(saveCurrentState()) );
	//END undo/redo stuff



	// Allow config manager to read from equation edits
	KConfigDialogManager::changedMap()->insert( "EquationEdit", SIGNAL(textEdited(const QString &)) );
	// Let's create a Configure Diloag
	m_settingsDialog = new KConfigDialog( parentWidget, "settings", Settings::self() );
	m_settingsDialog->setHelp("general-config");

	// create and add the page(s)
	m_generalSettings = new SettingsPageGeneral( View::self() );
	m_colorSettings = new SettingsPageColor( View::self() );
	m_fontsSettings = new SettingsPageFonts( View::self() );
	m_diagramSettings = new SettingsPageDiagram( View::self() );

	// Make sure the dialog is at a good default size (hmm kdialog should do this automatically?)
	QSize minSize = m_generalSettings->layout()->minimumSize()
			.expandedTo( m_colorSettings->layout()->minimumSize() )
			.expandedTo( m_fontsSettings->layout()->minimumSize() )
			.expandedTo( m_diagramSettings->layout()->minimumSize() );
	m_generalSettings->setMinimumSize( minSize );

	m_settingsDialog->addPage( m_generalSettings, i18n("General"), "tool", i18n("General Settings") );
	m_settingsDialog->addPage( m_diagramSettings, i18n("Diagram"), "coords", i18n("Diagram Appearance") );
	m_settingsDialog->addPage( m_colorSettings, i18n("Colors"), "colorscm", i18n("Colors") );
	m_settingsDialog->addPage( m_fontsSettings, i18n("Fonts"), "text", i18n("Fonts") );
	// User edited the configuration - update your local copies of the
	// configuration data
	connect( m_settingsDialog, SIGNAL( settingsChanged( const QString &) ), View::self(), SLOT(drawPlot() ) );


    new MainDlgAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/maindlg", this);

}

MainDlg::~MainDlg()
{
        m_recentFiles->saveEntries( m_config->group( QString() ) );
	XParser::self()->constants()->save();
	delete kmplotio;
}

void MainDlg::setupActions()
{
	// standard actions
        m_recentFiles = KStandardAction::openRecent( this, SLOT( slotOpenRecent( const KUrl& ) ), this );
        actionCollection()->addAction( "file_openrecent", m_recentFiles );
	actionCollection()->addAction( KStandardAction::Print, "file_print", this, SLOT( slotPrint() ) );
	KStandardAction::save( this, SLOT( slotSave() ), actionCollection() );
	KStandardAction::saveAs( this, SLOT( slotSaveas() ), actionCollection() );
	connect( kapp, SIGNAL( lastWindowClosed() ), kapp, SLOT( quit() ) );

	QAction *prefs  = KStandardAction::preferences( this, SLOT( slotSettings() ), actionCollection());
	prefs->setText( i18n( "Configure KmPlot..." ) );
	KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());


	// KmPlot specific actions

	//BEGIN file menu
	QAction * exportAction = actionCollection()->addAction( "export" );
        exportAction->setText( i18n( "E&xport..." ) );
	connect( exportAction, SIGNAL(triggered(bool)), this, SLOT( slotExport() ) );
	//END file menu


	//BEGIN edit menu
	m_undoAction = KStandardAction::undo( this, SLOT(undo()), actionCollection() );
	m_undoAction->setEnabled( false );

	m_redoAction = KStandardAction::redo( this, SLOT(redo()), actionCollection() );
	m_redoAction->setEnabled( false );

	QAction * editAxes = actionCollection()->addAction( "editaxes" );
        editAxes->setText( i18n( "&Coordinate System..." ) );
	editAxes->setIcon( KIcon("coords.png") );
	connect( editAxes, SIGNAL(triggered(bool)), this, SLOT( editAxes() ) );

	QAction * editConstants = actionCollection()->addAction( "editconstants" );
        editConstants->setText( i18n( "&Constants..." ) );
	editConstants->setIcon( KIcon("editconstants.png") );
	connect( editConstants, SIGNAL(triggered(bool)), this, SLOT( editConstants() ) );
	//END edit menu


	//BEGIN view menu
	/// \todo check that new shortcuts work

	QAction * zoomIn = actionCollection()->addAction( "zoom_in" );
        zoomIn->setText( i18n("Zoom &In") );
	zoomIn->setShortcut( QKeySequence(Qt::ControlModifier | Qt::Key_1) );
	zoomIn->setIcon( KIcon("zoom-in") );
	connect( zoomIn, SIGNAL(triggered(bool)), View::self(), SLOT(zoomIn()) );

	QAction * zoomOut = actionCollection()->addAction( "zoom_out" );
        zoomOut->setText(i18n("Zoom &Out"));
	zoomOut->setShortcut( QKeySequence(Qt::ControlModifier | Qt::Key_2) );
	zoomOut->setIcon( KIcon("zoom-out") );
	connect( zoomOut, SIGNAL(triggered(bool)), View::self(), SLOT( zoomOut() ) );

	QAction * zoomTrig = actionCollection()->addAction( "zoom_trig" );
        zoomTrig->setText( i18n("&Fit Widget to Trigonometric Functions") );
	connect( zoomTrig, SIGNAL(triggered(bool)), View::self(), SLOT( zoomToTrigonometric() ) );

	QAction * resetView = actionCollection()->addAction( "reset_view" );
	resetView->setText( i18n( "Reset View" ) );
	resetView->setIcon( KIcon("resetview") );
	connect( resetView, SIGNAL(triggered(bool)), this, SLOT( slotResetView() ) );
	//END view menu


	//BEGIN tools menu
	QAction *mnuCalculator = actionCollection()->addAction( "calculator" );
	mnuCalculator->setText( i18n( "Calculator") );
	mnuCalculator->setIcon( KIcon("exec") );
	connect( mnuCalculator, SIGNAL(triggered(bool)), this, SLOT( calculator() ) );

	QAction *mnuArea = actionCollection()->addAction( "grapharea" );
        mnuArea->setText( i18n( "Plot &Area..." ) );
	connect( mnuArea, SIGNAL(triggered(bool)),this, SLOT( graphArea() )  );

	QAction *mnuMaxValue = actionCollection()->addAction( "maximumvalue" );
        mnuMaxValue->setText( i18n( "Find Ma&ximum..." ) );
	mnuMaxValue->setIcon( KIcon("maximum") );
	connect( mnuMaxValue, SIGNAL(triggered(bool)), this, SLOT( findMaximumValue() ) );

	QAction *mnuMinValue = actionCollection()->addAction( "minimumvalue" );
        mnuMinValue->setText( i18n( "Find Mi&nimum..." ) );
	mnuMinValue->setIcon( KIcon("minimum") );
	connect( mnuMinValue, SIGNAL(triggered(bool)), this, SLOT( findMinimumValue() ) );
	//END tools menu


	//BEGIN help menu
	QAction * namesAction = actionCollection()->addAction( "names" );
        namesAction->setText( i18n( "Predefined &Math Functions" ) );
	namesAction->setIcon( KIcon("functionhelp") );
	connect( namesAction, SIGNAL(triggered(bool)), this, SLOT( slotNames() ) );
	//END help menu


	//BEGIN new plots menu
	QAction * newFunction = actionCollection()->addAction( "newcartesian" );
        newFunction->setText( i18n( "Cartesian Plot" ) );
	newFunction->setIcon( KIcon("newfunction") );
	connect( newFunction, SIGNAL(triggered(bool)), m_functionEditor, SLOT( createCartesian() ) );
	m_newPlotMenu->addAction( newFunction );

	QAction * newParametric = actionCollection()->addAction( "newparametric" );
        newParametric->setText( i18n( "Parametric Plot" ) );
	newParametric->setIcon( KIcon("newparametric") );
	connect( newParametric, SIGNAL(triggered(bool)), m_functionEditor, SLOT( createParametric() ) );
	m_newPlotMenu->addAction( newParametric );

	QAction * newPolar = actionCollection()->addAction( "newpolar" );
        newPolar->setText( i18n( "Polar Plot" ) );
	newPolar->setIcon( KIcon("newpolar") );
	connect( newPolar, SIGNAL(triggered(bool)), m_functionEditor, SLOT( createPolar() ) );
	m_newPlotMenu->addAction( newPolar );

	QAction * newImplicit = actionCollection()->addAction( "newimplicit" );
        newImplicit->setText( i18n( "Implicit Plot" ) );
	newImplicit->setIcon( KIcon("newimplicit") );
	connect( newImplicit, SIGNAL(triggered(bool)), m_functionEditor, SLOT( createImplicit() ) );
	m_newPlotMenu->addAction( newImplicit );

	QAction * newDifferential = actionCollection()->addAction( "newdifferential" );
        newDifferential->setText( i18n( "Differential Plot" ) );
	newDifferential->setIcon( KIcon("newdifferential") );
	connect( newDifferential, SIGNAL(triggered(bool)), m_functionEditor, SLOT( createDifferential() ) );
	m_newPlotMenu->addAction( newDifferential );
	//END new plots menu


	kDebug() << "KStandardDirs::resourceDirs( icon )="<<KGlobal::dirs()->resourceDirs( "icon" )<<endl;

	View::self()->m_menuSliderAction = actionCollection()->add<KToggleAction>( "options_configure_show_sliders" );
        View::self()->m_menuSliderAction->setText( i18n( "Show Sliders" ) );
	connect( View::self()->m_menuSliderAction, SIGNAL(triggered(bool)), this, SLOT( toggleShowSliders() ) );


	//BEGIN function popup menu
	QAction *mnuEdit = actionCollection()->addAction( "mnuedit"  );
        mnuEdit->setText(i18n("&Edit"));
	m_firstFunctionAction = mnuEdit;
	mnuEdit->setIcon( KIcon("editplots") );
	connect(mnuEdit , SIGNAL(triggered(bool)), View::self(), SLOT( editCurrentPlot() ) );
	m_popupmenu->addAction( mnuEdit );

	QAction *mnuHide = actionCollection()->addAction( "mnuhide" );
        mnuHide->setText( i18n("&Hide") );
	connect( mnuHide, SIGNAL(triggered(bool)), View::self(), SLOT( hideCurrentFunction() ) );
	m_popupmenu->addAction( mnuHide );

	QAction *mnuRemove = actionCollection()->addAction( "mnuremove"  );
        mnuRemove->setText(i18n("&Remove"));
	mnuRemove->setIcon( KIcon("edit-delete") );
	connect( mnuRemove, SIGNAL(triggered(bool)), View::self(), SLOT( removeCurrentPlot() ) );
	m_popupmenu->addAction( mnuRemove );

	m_popupmenu->addSeparator();

	QAction * animateFunction = actionCollection()->addAction( "animateFunction" );
        animateFunction->setText(i18n("Animate Plot..."));
	connect( animateFunction, SIGNAL(triggered(bool)), View::self(), SLOT( animateFunction() ) );
	m_popupmenu->addAction( animateFunction );
	m_popupmenu->addSeparator();

	m_popupmenu->addAction( mnuCalculator );
	m_popupmenu->addAction( mnuMinValue );
	m_popupmenu->addAction( mnuMaxValue );
	m_popupmenu->addAction( mnuArea );
	//END function popup menu
}


void MainDlg::undo()
{
	kDebug() << k_funcinfo << endl;

	if ( m_undoStack.isEmpty() )
		return;

	m_redoStack.push( m_currentState );
	m_currentState = m_undoStack.pop();

	kmplotio->restore( m_currentState );
	View::self()->drawPlot();

	m_undoAction->setEnabled( !m_undoStack.isEmpty() );
	m_redoAction->setEnabled( true );
}


void MainDlg::redo()
{
	kDebug() << k_funcinfo << endl;

	if ( m_redoStack.isEmpty() )
		return;

	m_undoStack.push( m_currentState );
	m_currentState = m_redoStack.pop();

	kmplotio->restore( m_currentState );
	View::self()->drawPlot();

	m_undoAction->setEnabled( true );
	m_redoAction->setEnabled( !m_redoStack.isEmpty() );
}


void MainDlg::requestSaveCurrentState()
{
	m_saveCurrentStateTimer->start( 0 );
}
void MainDlg::saveCurrentState( )
{
	m_redoStack.clear();
	m_undoStack.push( m_currentState );
	m_currentState = kmplotio->currentState();

	// limit stack size to 100 items
	while ( m_undoStack.count() > 100 )
		m_undoStack.pop_front();

	m_undoAction->setEnabled( true );
	m_redoAction->setEnabled( false );

	m_modified = true;
}


void MainDlg::resetUndoRedo()
{
	m_redoStack.clear();
	m_undoStack.clear();
	m_currentState = kmplotio->currentState();
	m_undoAction->setEnabled( false );
	m_redoAction->setEnabled( false );
}


bool MainDlg::checkModified()
{
	if( m_modified )
	{
		int saveit = KMessageBox::warningYesNoCancel( m_parent, i18n( "The plot has been modified.\n"
		             "Do you want to save it?" ), QString(), KStandardGuiItem::save(), KStandardGuiItem::discard() );
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


void MainDlg::slotSave()
{
	if ( !m_modified || m_readonly) //don't save if no changes are made or readonly is enabled
		return;
	if ( url().isEmpty() )            // if there is no file name set yet
		slotSaveas();
	else
	{
		if ( !m_modified) //don't save if no changes are made
			return;

		if ( oldfileversion)
		{
			if ( KMessageBox::warningContinueCancel( m_parent, i18n( "This file is saved with an old file format; if you save it, you cannot open the file with older versions of KmPlot. Are you sure you want to continue?" ), QString(), KGuiItem(i18n("Save New Format")) ) == KMessageBox::Cancel)
				return;
		}
		kmplotio->save( this->url() );
		kDebug() << "saved" << endl;
		m_modified = false;
	}

}

void MainDlg::slotSaveas()
{
	if (m_readonly)
		return;
	const KUrl url = KFileDialog::getSaveUrl( QDir::currentPath(), i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), m_parent, i18n( "Save As" ) );

	if ( url.isEmpty() )
		return;

	bool exists = KIO::NetAccess::exists( url,false,m_parent );
	if ( exists )
	{
		// check if file exists and overwriting is ok.

		int answer = KMessageBox::warningContinueCancel( m_parent, i18n( "A file named \"%1\" already exists. Are you sure you want to continue and overwrite this file?", url.url()), i18n( "Overwrite File?" ), KGuiItem( i18n( "&Overwrite" ) ) );

		if ( answer != KMessageBox::Continue )
			return;
	}

	if ( !kmplotio->save( url ) )
		KMessageBox::error(m_parent, i18n("The file could not be saved") );
	else
	{
		setUrl(url);
		m_recentFiles->addUrl( url );
		setWindowCaption( this->url().prettyUrl(KUrl::LeaveTrailingSlash) );
		m_modified = false;
	}
}

void MainDlg::slotExport()
{
	QString filter = KImageIO::pattern( KImageIO::Writing );
	filter += i18n("\n*.svg|Scalable Vector Graphics");

	KFileDialog * kfd = new KFileDialog( QDir::currentPath(), filter, m_parent );
	kfd->setCaption( i18n( "Export as Image" ) );

	if ( !kfd->exec() )
		return;

	KUrl url = kfd->selectedUrl();
	delete kfd;
	
	bool exists = KIO::NetAccess::exists( url,false,m_parent );
	if ( exists )
	{
		// check if file exists and overwriting is ok.

		int answer = KMessageBox::warningContinueCancel( m_parent, i18n( "A file named \"%1\" already exists. Are you sure you want to continue and overwrite this file?", url.url()), i18n( "Overwrite File?" ), KGuiItem( i18n( "&Overwrite" ) ) );

		if ( answer != KMessageBox::Continue )
			return;
	}

	KMimeType::Ptr mimeType = KMimeType::findByUrl( url );
	kDebug() << k_funcinfo << "mimetype: " << mimeType->name() << endl;

	bool isSvg = mimeType->name() == "image/svg+xml";

	if ( !KImageIO::isSupported( mimeType->name(), KImageIO::Writing ) && !isSvg )
	{
		KMessageBox::sorry( m_parent, i18n( "Sorry, this file format is not supported." ) );
		return;
	};

	bool saveOk = true;

	if ( isSvg )
	{
		QSvgGenerator img;
		img.setSize( View::self()->size() );
		
		QFile file;
		KTemporaryFile tmp;
		
		if ( url.isLocalFile() )
		{
			file.setFileName( url.path() );
			img.setOutputDevice( &file );
		}
		else
		{
			tmp.setSuffix( ".svg" );
			img.setOutputDevice( &tmp );
		}
		
		View::self()->draw( &img, View::SVG );
		
		if ( !url.isLocalFile() )
			saveOk &= KIO::NetAccess::upload(tmp.fileName(), url, 0);
	}
	else
	{
		QPixmap img( View::self()->size() );
		View::self()->draw( & img, View::Pixmap );

		QStringList types = KImageIO::typeForMime( mimeType->name() );
		if ( types.isEmpty() )
			return; // TODO error dialog?

		if ( url.isLocalFile() )
			saveOk = img.save( url.path(), types.at(0).toLatin1() );
		else
		{
			KTemporaryFile tmp;
			tmp.open();
			img.save( tmp.fileName(), types.at(0).toLatin1() );
			saveOk = KIO::NetAccess::upload(tmp.fileName(), url, 0);
		}
	}

	if ( !saveOk )
		KMessageBox::error( m_parent, i18n( "Sorry, something went wrong while saving to image \"%1\"", url.prettyUrl() ) );
}


bool MainDlg::openFile()
{
	if (url()==m_currentfile || !kmplotio->load( url() ) )
	{
		m_recentFiles->removeUrl( url() ); //remove the file from the recent-opened-file-list
		setUrl(KUrl());
		return false;
	}
	
	m_currentfile = url();
	m_recentFiles->addUrl( url().prettyUrl(KUrl::LeaveTrailingSlash)  );
	setWindowCaption( url().prettyUrl(KUrl::LeaveTrailingSlash) );
	resetUndoRedo();
	View::self()->updateSliders();
	View::self()->drawPlot();
	return true;
}

bool MainDlg::saveFile()
{
    slotSave();
    return !isModified();
}

void MainDlg::slotOpenRecent( const KUrl &url )
{
 	if( isModified() || !this->url().isEmpty() ) // open the file in a new window
 	{
		QDBusReply<void> reply = QDBusInterface( QDBusConnection::sessionBus().baseService(), "/kmplot", "org.kde.kmplot.KmPlot" ).call( QDBus::Block, "openFileInNewWindow", url.url() );
		return;
	}

	if ( !kmplotio->load( url ) ) //if the loading fails
	{
		m_recentFiles->removeUrl(url ); //remove the file from the recent-opened-file-list
		return;
	}
    m_currentfile = url;
    setUrl(url);
    m_recentFiles->setCurrentItem(-1); //don't select the item in the open-recent menu
    setWindowCaption( this->url().prettyUrl(KUrl::LeaveTrailingSlash) );
	resetUndoRedo();
    View::self()->updateSliders();
    View::self()->drawPlot();
}

void MainDlg::slotPrint()
{
	KPrinter prt( QPrinter::PrinterResolution );
	prt.setResolution( 72 );
	KPrinterDlg* printdlg = new KPrinterDlg( m_parent );
	printdlg->setObjectName( "KmPlot page" );
	prt.addDialogPage( printdlg );
	if ( prt.setup( m_parent, i18n( "Print Plot" ) ) )
		View::self()->draw(&prt, View::Printer);
}


void MainDlg::editAxes()
{
	coordsDialog()->show();
}


void MainDlg::editConstants()
{
	if ( !m_constantEditor)
		m_constantEditor = new KConstantEditor(m_parent);

	m_constantEditor->show();
}


void MainDlg::slotNames()
{
	KToolInvocation::invokeHelp( "func-predefined", "kmplot" );
}


void MainDlg::slotResetView()
{
	View::self()->animateZoom( QRectF( -8, -8, 16, 16 ) );
}

void MainDlg::slotSettings()
{
	// An instance of your dialog has already been created and has been cached,
	// so we want to display the cached dialog instead of creating
	// another one
	KConfigDialog::showDialog( "settings" );
}


void MainDlg::calculator()
{
	m_calculator->show();
}

void MainDlg::findMinimumValue()
{
	m_functionTools->init( FunctionTools::FindMinimum );
	m_functionTools->show();
}

void MainDlg::findMaximumValue()
{
	m_functionTools->init( FunctionTools::FindMaximum );
	m_functionTools->show();
}

void MainDlg::graphArea()
{
	m_functionTools->init( FunctionTools::CalculateArea );
	m_functionTools->show();
}

void MainDlg::toggleShowSliders()
{
	// create the slider if it not exists already
	if ( !View::self()->m_sliderWindow )
	{
		View::self()->m_sliderWindow = new KSliderWindow( View::self() );
		connect( View::self()->m_sliderWindow, SIGNAL( valueChanged() ), View::self(), SLOT( drawPlot() ) );
		connect( View::self()->m_sliderWindow, SIGNAL( windowClosed() ), View::self(), SLOT( slidersWindowClosed() ) );
	}
	if ( !View::self()->m_sliderWindow->isVisible() )
		View::self()->m_sliderWindow->show();
	else
		View::self()->m_sliderWindow->hide();
}

void MainDlg::setReadOnlyStatusBarText(const QString &text)
{
	setStatusBarText(text);
}

void MainDlg::optionsConfigureKeys()
{
// 	KApplication::kApplication()->dcopClient()->send(KApplication::kApplication()->dcopClient()->appId(), "KmPlotShell","optionsConfigureKeys()", QByteArray());
}

void MainDlg::optionsConfigureToolbars()
{
// 	KApplication::kApplication()->dcopClient()->send(KApplication::kApplication()->dcopClient()->appId(), "KmPlotShell","optionsConfigureToolbars()", QByteArray());
}

bool MainDlg::queryClose()
{
    return checkModified();
}

CoordsConfigDialog * MainDlg::coordsDialog( )
{
	if ( !m_coordsDialog)
	{
		m_coordsDialog = new CoordsConfigDialog(m_parent);
		connect( m_coordsDialog, SIGNAL( settingsChanged(const QString &) ), View::self(), SLOT(drawPlot() ) );
	}

	return m_coordsDialog;
}
//END class MainDlg


// It's usually safe to leave the factory code alone.. with the
// notable exception of the KAboutData data
#include <kaboutdata.h>
#include <klocale.h>
#include <ktoolinvocation.h>
#include <kglobal.h>

KComponentData *KmPlotPartFactory::s_instance = 0L;
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

KParts::Part* KmPlotPartFactory::createPartObject( QWidget *parentWidget,
        QObject *parent, const char *, const QStringList &args )
{
	// Create an instance of our Part
	MainDlg* obj = new MainDlg( parentWidget, parent, args );
	emit objectCreated( obj );
	return obj;
}

const KComponentData &KmPlotPartFactory::componentData()
{
	if( !s_instance )
	{
		s_about = new KAboutData("kmplot",I18N_NOOP( "KmPlotPart" ), "1");
		s_instance = new KComponentData(s_about);
	}
	return *s_instance;
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
		: KParts::BrowserExtension( parent )
{
	emit enableAction("print", true);
	setURLDropHandlingEnabled(true);
}

void BrowserExtension::print()
{
	static_cast<MainDlg*>(parent())->slotPrint();
}

#include "maindlg.moc"
