/*
    KmPlot - a math. function plotter for the KDE-Desktop

    SPDX-FileCopyrightText: 1998, 1999, 2000, 2002 Klaus-Dieter Möller <kd.moeller@t-online.de>
    SPDX-FileCopyrightText: 2006  David Saxton <david@bluehaze.org>

    This file is part of the KDE Project.
    KmPlot is part of the KDE-EDU Project.

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#include "maindlg.h"

// Qt includes
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QIcon>
#include <QImageWriter>
#include <QMainWindow>
#include <QMimeDatabase>
#include <QMimeType>
#include <QPixmap>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrintPreviewWidget>
#include <QSvgGenerator>
#include <QTemporaryFile>
#include <QTimer>

// KDE includes
#include <KActionCollection>
#include <KConfigDialog>
#include <KConfigDialogManager>
#include <KHelpClient>
#include <KIO/StatJob>
#include <KIO/StoredTransferJob>
#include <KLineEdit>
#include <KMessageBox>
#include <KStandardAction>
#include <KToolBar>
#include <KToolInvocation>

// local includes
#include "calculator.h"
#include "functiontools.h"
#include "functioneditor.h"
#include "kconstanteditor.h"
#include "xparser.h"

#include "settings.h"
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
			layout()->setContentsMargins(0, 0, 0, 0);
		}
};

class SettingsPageFonts : public QWidget, public Ui::SettingsPageFonts
{
    public:
        SettingsPageFonts( QWidget * parent = 0 )
	: QWidget( parent )
		{
			setupUi(this);
			layout()->setContentsMargins(0, 0, 0, 0);
		}
};

class SettingsPageGeneral : public QWidget, public Ui::SettingsPageGeneral
{
    public:
        SettingsPageGeneral( QWidget * parent = 0 )
	: QWidget( parent )
		{
			setupUi(this);
                        layout()->setContentsMargins(0, 0, 0, 0);
		}
};

class SettingsPageDiagram : public QWidget, public Ui::SettingsPageDiagram
{
	public:
		SettingsPageDiagram( QWidget * parent = 0 )
	: QWidget( parent )
		{
			setupUi(this);
			layout()->setContentsMargins(0, 0, 0, 0);
		}
};

bool MainDlg::oldfileversion;
MainDlg * MainDlg::m_self = 0;

K_PLUGIN_CLASS_WITH_JSON(MainDlg, "kmplot_part.json")

//BEGIN class MainDlg
MainDlg::MainDlg(QWidget *parentWidget, QObject *parent, const QVariantList& ) :
		KParts::ReadWritePart( parent ),
		m_recentFiles( 0 ),
		m_modified(false),
		m_parent(parentWidget),
		m_rootValue( 0 )
{
	assert( !m_self ); // this class should only be constructed once
	m_self = this;

	qDebug() << "parentWidget->objectName():" << parentWidget->objectName();
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
	m_popupmenu = new QMenu( parentWidget );
	m_newPlotMenu = new QMenu( parentWidget );
	(void) new View( m_readonly, m_popupmenu, parentWidget );
	connect( View::self(), &View::setStatusBarText, this, &MainDlg::setReadOnlyStatusBarText );

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
	m_config = KSharedConfig::openConfig();
	m_recentFiles->loadEntries( m_config->group( QString() ) );


	//BEGIN undo/redo stuff
	m_currentState = kmplotio->currentState();
	m_saveCurrentStateTimer = new QTimer( this );
	m_saveCurrentStateTimer->setSingleShot( true );
	connect( m_saveCurrentStateTimer, &QTimer::timeout, this, &MainDlg::saveCurrentState );
	//END undo/redo stuff

	// Let's create a Configure Dialog
	m_settingsDialog = new KConfigDialog( parentWidget, "settings", Settings::self() );

	QMetaObject::invokeMethod(m_settingsDialog, "setHelp", Qt::DirectConnection, Q_ARG(QString, QString::fromLatin1("configuration")), Q_ARG(QString, QString::fromLatin1("kmplot")));

	// create and add the page(s)
	m_generalSettings = new SettingsPageGeneral( View::self() );
	m_colorSettings = new SettingsPageColor( View::self() );
	m_fontsSettings = new SettingsPageFonts( View::self() );
	m_diagramSettings = new SettingsPageDiagram( View::self() );

	// Make sure the dialog is at a good default size (hmm QDialog should do this automatically?)
	QSize minSize = m_generalSettings->layout()->minimumSize()
			.expandedTo( m_colorSettings->layout()->minimumSize() )
			.expandedTo( m_fontsSettings->layout()->minimumSize() )
			.expandedTo( m_diagramSettings->layout()->minimumSize() );
	m_generalSettings->setMinimumSize( minSize );

	m_settingsDialog->addPage( m_generalSettings, i18n("General"), "kmplot", i18n("General Settings") );
	m_settingsDialog->addPage( m_diagramSettings, i18n("Diagram"), "coords", i18n("Diagram Appearance") );
	m_settingsDialog->addPage( m_colorSettings, i18n("Colors"), "preferences-desktop-color", i18n("Colors") );
	m_settingsDialog->addPage( m_fontsSettings, i18n("Fonts"), "preferences-desktop-font", i18n("Fonts") );
	// User edited the configuration - update your local copies of the
	// configuration data
	connect( m_settingsDialog, &KConfigDialog::settingsChanged, View::self(), QOverload<>::of(&View::drawPlot) );


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
        m_recentFiles = KStandardAction::openRecent( this, SLOT(slotOpenRecent(QUrl)), this );
        actionCollection()->addAction( "file_open_recent", m_recentFiles );
	actionCollection()->addAction( KStandardAction::Print, "file_print", this, SLOT(slotPrint()) );
	actionCollection()->addAction( KStandardAction::PrintPreview, "file_print_preview", this, SLOT(slotPrintPreview()) );
	KStandardAction::save( this, SLOT(slotSave()), actionCollection() );
	KStandardAction::saveAs( this, SLOT(slotSaveas()), actionCollection() );

	QAction *prefs  = KStandardAction::preferences( this, SLOT(slotSettings()), actionCollection());
	prefs->setText( i18n( "Configure KmPlot..." ) );

	// KmPlot specific actions

	//BEGIN file menu
	QAction * exportAction = actionCollection()->addAction( "export" );
        exportAction->setText( i18n( "E&xport..." ) );
        exportAction->setIcon( QIcon::fromTheme( "document-export" ) );
	connect( exportAction, &QAction::triggered, this, &MainDlg::slotExport );
	//END file menu


	//BEGIN edit menu
	m_undoAction = KStandardAction::undo( this, SLOT(undo()), actionCollection() );
	m_undoAction->setEnabled( false );

	m_redoAction = KStandardAction::redo( this, SLOT(redo()), actionCollection() );
	m_redoAction->setEnabled( false );

	QAction * editAxes = actionCollection()->addAction( "editaxes" );
        editAxes->setText( i18n( "&Coordinate System..." ) );
	editAxes->setIcon( QIcon::fromTheme("coords.png") );
	connect( editAxes, &QAction::triggered, this, &MainDlg::editAxes );

	QAction * editConstants = actionCollection()->addAction( "editconstants" );
        editConstants->setText( i18n( "&Constants..." ) );
	editConstants->setIcon( QIcon::fromTheme("editconstants.png") );
	connect( editConstants, &QAction::triggered, this, &MainDlg::editConstants );
	//END edit menu


	//BEGIN view menu
	/// \todo check that new shortcuts work

	QAction * zoomIn = actionCollection()->addAction( "zoom_in" );
        zoomIn->setText( i18n("Zoom &In") );
	actionCollection()->setDefaultShortcut( zoomIn, QKeySequence(Qt::ControlModifier | Qt::Key_1) );
	zoomIn->setIcon( QIcon::fromTheme("zoom-in") );
	connect( zoomIn, &QAction::triggered, View::self(), QOverload<>::of(&View::zoomIn) );

	QAction * zoomOut = actionCollection()->addAction( "zoom_out" );
        zoomOut->setText(i18n("Zoom &Out"));
	actionCollection()->setDefaultShortcut( zoomOut, QKeySequence(Qt::ControlModifier | Qt::Key_2) );
	zoomOut->setIcon( QIcon::fromTheme("zoom-out") );
	connect( zoomOut, &QAction::triggered, View::self(), QOverload<>::of(&View::zoomOut) );

	QAction * zoomTrig = actionCollection()->addAction( "zoom_trig" );
        zoomTrig->setText( i18n("&Fit Widget to Trigonometric Functions") );
	connect( zoomTrig, &QAction::triggered, View::self(), &View::zoomToTrigonometric );

	QAction * resetView = actionCollection()->addAction( "reset_view" );
	resetView->setText( i18n( "Reset View" ) );
	resetView->setIcon( QIcon::fromTheme("resetview") );
	connect( resetView, &QAction::triggered, this, &MainDlg::slotResetView );

	View::self()->m_menuSliderAction = actionCollection()->add<KToggleAction>( "options_configure_show_sliders" );
	View::self()->m_menuSliderAction->setText( i18n( "Show Sliders" ) );
	connect( View::self()->m_menuSliderAction, &QAction::toggled, this, &MainDlg::toggleShowSliders );
	//END view menu


	//BEGIN tools menu
	QAction *mnuCalculator = actionCollection()->addAction( "calculator" );
	mnuCalculator->setText( i18n( "Calculator") );
	mnuCalculator->setIcon( QIcon::fromTheme("system-run") );
	connect( mnuCalculator, &QAction::triggered, this, &MainDlg::calculator );

	QAction *mnuArea = actionCollection()->addAction( "grapharea" );
        mnuArea->setText( i18n( "Plot &Area..." ) );
	connect( mnuArea, &QAction::triggered, this, &MainDlg::graphArea );

	QAction *mnuMaxValue = actionCollection()->addAction( "maximumvalue" );
        mnuMaxValue->setText( i18n( "Find Ma&ximum..." ) );
	mnuMaxValue->setIcon( QIcon::fromTheme("maximum") );
	connect( mnuMaxValue, &QAction::triggered, this, &MainDlg::findMaximumValue );

	QAction *mnuMinValue = actionCollection()->addAction( "minimumvalue" );
        mnuMinValue->setText( i18n( "Find Mi&nimum..." ) );
	mnuMinValue->setIcon( QIcon::fromTheme("minimum") );
	connect( mnuMinValue, &QAction::triggered, this, &MainDlg::findMinimumValue );
	//END tools menu


	//BEGIN help menu
	QAction * namesAction = actionCollection()->addAction( "names" );
        namesAction->setText( i18nc("@action:inmenu Help", "Predefined &Math Functions") );
	namesAction->setIcon( QIcon::fromTheme("functionhelp") );
	connect( namesAction, &QAction::triggered, this, &MainDlg::slotNames );
	//END help menu


	//BEGIN new plots menu
	QAction * newFunction = actionCollection()->addAction( "newcartesian" );
        newFunction->setText( i18n( "Cartesian Plot" ) );
	newFunction->setIcon( QIcon::fromTheme("newfunction") );
	connect( newFunction, &QAction::triggered, m_functionEditor, &FunctionEditor::createCartesian );
	m_newPlotMenu->addAction( newFunction );

	QAction * newParametric = actionCollection()->addAction( "newparametric" );
        newParametric->setText( i18n( "Parametric Plot" ) );
	newParametric->setIcon( QIcon::fromTheme("newparametric") );
	connect( newParametric, &QAction::triggered, m_functionEditor, &FunctionEditor::createParametric );
	m_newPlotMenu->addAction( newParametric );

	QAction * newPolar = actionCollection()->addAction( "newpolar" );
        newPolar->setText( i18n( "Polar Plot" ) );
	newPolar->setIcon( QIcon::fromTheme("newpolar") );
	connect( newPolar, &QAction::triggered, m_functionEditor, &FunctionEditor::createPolar );
	m_newPlotMenu->addAction( newPolar );

	QAction * newImplicit = actionCollection()->addAction( "newimplicit" );
        newImplicit->setText( i18n( "Implicit Plot" ) );
	newImplicit->setIcon( QIcon::fromTheme("newimplicit") );
	connect( newImplicit, &QAction::triggered, m_functionEditor, &FunctionEditor::createImplicit );
	m_newPlotMenu->addAction( newImplicit );

	QAction * newDifferential = actionCollection()->addAction( "newdifferential" );
        newDifferential->setText( i18n( "Differential Plot" ) );
	newDifferential->setIcon( QIcon::fromTheme("newdifferential") );
	connect( newDifferential, &QAction::triggered, m_functionEditor, &FunctionEditor::createDifferential );
	m_newPlotMenu->addAction( newDifferential );
	//END new plots menu


	//BEGIN function popup menu
	QAction *mnuEdit = actionCollection()->addAction( "mnuedit"  );
        mnuEdit->setText(i18n("&Edit"));
	m_firstFunctionAction = mnuEdit;
	mnuEdit->setIcon( QIcon::fromTheme("editplots") );
	connect(mnuEdit, &QAction::triggered, View::self(), &View::editCurrentPlot );
	m_popupmenu->addAction( mnuEdit );

	QAction *mnuHide = actionCollection()->addAction( "mnuhide" );
        mnuHide->setText( i18n("&Hide") );
	connect( mnuHide, &QAction::triggered, View::self(), &View::hideCurrentFunction );
	m_popupmenu->addAction( mnuHide );

	QAction *mnuRemove = actionCollection()->addAction( "mnuremove"  );
        mnuRemove->setText(i18n("&Remove"));
	mnuRemove->setIcon( QIcon::fromTheme("edit-delete") );
	connect( mnuRemove, &QAction::triggered, View::self(), &View::removeCurrentPlot );
	m_popupmenu->addAction( mnuRemove );

	m_popupmenu->addSeparator();

	QAction * animateFunction = actionCollection()->addAction( "animateFunction" );
        animateFunction->setText(i18n("Animate Plot..."));
	connect( animateFunction, &QAction::triggered, View::self(), &View::animateFunction );
	m_popupmenu->addAction( animateFunction );
	m_popupmenu->addSeparator();

	m_popupmenu->addAction( mnuCalculator );
	m_popupmenu->addAction( mnuMinValue );
	m_popupmenu->addAction( mnuMaxValue );
	m_popupmenu->addAction( mnuArea );

	QAction * copyXY = actionCollection()->addAction( "copyXY" );
	copyXY->setText(i18n("Copy (x, y)"));
	connect( copyXY, &QAction::triggered, []{
		QClipboard * cb = QApplication::clipboard();
		QPointF currentXY = View::self()->getCrosshairPosition();
		cb->setText( i18nc("Copied pair of coordinates (x, y)", "(%1, %2)", QLocale().toString( currentXY.x(), 'f', 5 ), QLocale().toString( currentXY.y(), 'f', 5 )), QClipboard::Clipboard );
	} );
	m_popupmenu->addAction( copyXY );

	QAction * copyRootValue = actionCollection()->addAction( "copyRootValue" );
	copyRootValue->setText(i18n("Copy Root Value"));
	connect( View::self(), &View::updateRootValue, [this, copyRootValue]( bool haveRoot, double rootValue ){
		copyRootValue->setVisible(haveRoot);
		m_rootValue = rootValue;
	} );
	connect( copyRootValue, &QAction::triggered, [this]{
		QClipboard * cb = QApplication::clipboard();
		cb->setText( QLocale().toString( m_rootValue, 'f', 5 ), QClipboard::Clipboard );
	} );
	m_popupmenu->addAction( copyRootValue );

	//END function popup menu
}


void MainDlg::undo()
{
	qDebug() ;

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
	qDebug() ;

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
		qDebug() << "saved";
		m_modified = false;
	}

}

void MainDlg::slotSaveas()
{
	if (m_readonly)
		return;
	const QUrl url = QFileDialog::getSaveFileUrl(m_parent, i18n( "Save As" ), QUrl::fromLocalFile(QDir::currentPath()),
			    i18n( "KmPlot Files (*.fkt);;All Files (*)" )
			      );

	if ( url.isEmpty() )
		return;

	if ( !kmplotio->save( url ) )
		KMessageBox::error(m_parent, i18n("The file could not be saved") );
	else
	{
		setUrl(url);
		m_recentFiles->addUrl( url );
		setWindowCaption( QUrl(this->url()).toString() );
		m_modified = false;
	}
}

void MainDlg::slotExport()
{
	QString filters;
	QMimeDatabase mimeDatabase;
	for (const QByteArray &mimeType : QImageWriter::supportedMimeTypes()) {
		const QString filter = mimeDatabase.mimeTypeForName(QLatin1String(mimeType)).filterString();
		if (!filter.isEmpty()) {
			if (mimeType == QByteArrayLiteral("image/png")) {
				if (!filters.isEmpty()) {
					filters.prepend(QStringLiteral(";;"));
				}
				filters.prepend(filter);
			} else {
				if (!filters.isEmpty()) {
					filters.append(QStringLiteral(";;"));
                                }
				filters.append(filter);
			}
		}
	}
	if (!filters.isEmpty()) {
		filters.append(QStringLiteral(";;"));
	}
	filters.append(i18n("Scalable Vector Graphics (*.svg)"));

	QUrl url = QFileDialog::getSaveFileUrl(m_parent, i18nc("@title:window", "Export as Image"), QUrl::fromLocalFile(QDir::currentPath()), filters);

	if ( !url.isValid() )
		return;

	QMimeType mimeType = mimeDatabase.mimeTypeForUrl( url );
	qDebug() << "mimetype: " << mimeType.name();

	bool isSvg = mimeType.name() == "image/svg+xml";

	bool saveOk = true;

	if ( isSvg )
	{
		QSvgGenerator img;
		img.setViewBox( QRect( QPoint(0, 0), View::self()->size() ) );

		QFile file;
		QTemporaryFile tmp;

		if ( url.isLocalFile() )
		{
			file.setFileName( url.toLocalFile() );
			img.setOutputDevice( &file );
		}
		else
		{
 			tmp.setFileTemplate(QDir::tempPath() + QLatin1String("/kmplot_XXXXXX") + QLatin1String(".svg")); 
			img.setOutputDevice( &tmp );
		}

		View::self()->draw( &img, View::SVG );

		if ( !url.isLocalFile() )
		{
		    Q_CONSTEXPR int permission = -1;
		    QFile file(tmp.fileName());
		    file.open(QIODevice::ReadOnly);
		    KIO::StoredTransferJob *putjob = KIO::storedPut(file.readAll(), url, permission, KIO::JobFlag::Overwrite);
		    saveOk &= putjob->exec();
		    file.close();
		}
	}
	else
	{
		QPixmap img( View::self()->size() );
		View::self()->draw( & img, View::Pixmap );

		QStringList types = mimeType.suffixes();
		if ( types.isEmpty() )
			return; // TODO error dialog?

		if ( url.isLocalFile() )
			saveOk = img.save( url.toLocalFile(), types.at(0).toLatin1() );
		else
		{
			QTemporaryFile tmp;
			tmp.open();
			img.save( tmp.fileName(), types.at(0).toLatin1() );
			Q_CONSTEXPR int permission = -1;
			QFile file(tmp.fileName());
			file.open(QIODevice::ReadOnly);
			KIO::StoredTransferJob *putjob = KIO::storedPut(file.readAll(), url, permission, KIO::JobFlag::Overwrite);
			saveOk = putjob->exec();
			file.close();
		}
	}

	if ( !saveOk )
	    KMessageBox::error(m_parent,
			       i18n("Sorry, something went wrong while saving to image \"%1\"",
				    url.toString()));
}


bool MainDlg::openFile()
{
	if (url()==m_currentfile || !kmplotio->load( url() ) )
	{
		m_recentFiles->removeUrl( url() ); //remove the file from the recent-opened-file-list
		setUrl(QUrl());
		return false;
	}

	m_currentfile = url();
	m_recentFiles->addUrl( url() );
	setWindowCaption( url().toDisplayString() );
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

void MainDlg::slotOpenRecent( const QUrl &url )
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
    setWindowCaption( QUrl(this->url()).toString() );
	resetUndoRedo();
    View::self()->updateSliders();
    View::self()->drawPlot();
}

void MainDlg::slotPrint()
{
	QPrinter prt( QPrinter::PrinterResolution );
	prt.setResolution( 72 );
	KPrinterDlg* printdlg = new KPrinterDlg( m_parent );
	printdlg->setObjectName( "KmPlot page" );
	QPointer<QPrintDialog> printDialog = new QPrintDialog( &prt, m_parent );
	printDialog->setOptionTabs( QList<QWidget*>() << printdlg );
	printDialog->setWindowTitle( i18nc("@title:window", "Print Plot") );

	if (printDialog->exec())
	{
		setupPrinter(printdlg, &prt);
	}
        delete printDialog;
}

void MainDlg::slotPrintPreview()
{
	QPrinter prt( QPrinter::PrinterResolution );
	QPointer<QPrintPreviewDialog> preview = new QPrintPreviewDialog( &prt );
	QPointer<KPrinterDlg> printdlg = new KPrinterDlg( m_parent );
	QList<QToolBar *> toolbarlist = preview->findChildren<QToolBar *>();
	if(!toolbarlist.isEmpty())
	{
		QAction *printSettings = toolbarlist.first()->addAction( QIcon::fromTheme( "configure" ), i18n("Print Settings") );
		QList<QPrintPreviewWidget*> previewWidgetsList = preview->findChildren<QPrintPreviewWidget*>();
		QPrintPreviewWidget *previewWidget = previewWidgetsList.first();
		connect( printSettings, &QAction::triggered, [preview, previewWidget, printdlg]{
			QDialog *printSettingsDialog = new QDialog( preview, Qt::WindowFlags() );
			printSettingsDialog->setWindowTitle( i18nc("@title:window", "Print Settings") );
			QVBoxLayout *mainLayout = new QVBoxLayout;
			printSettingsDialog->setLayout(mainLayout);
			mainLayout->addWidget(printdlg);
			QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok|QDialogButtonBox::Cancel );
			connect(buttonBox, &QDialogButtonBox::accepted, [previewWidget, printSettingsDialog]{
				previewWidget->updatePreview();
				printSettingsDialog->close();
			} );
			connect(buttonBox, &QDialogButtonBox::rejected, printSettingsDialog, &QDialog::reject);
			mainLayout->addWidget(buttonBox);
			printSettingsDialog->show();
		});
	}
	connect(preview, &QPrintPreviewDialog::paintRequested, [this, &printdlg, &prt]{
		setupPrinter(printdlg, &prt);
	} );
	preview->exec();
	delete printdlg;
	delete preview;
}

void MainDlg::setupPrinter(KPrinterDlg *printDialog, QPrinter *printer)
{
		View::self()->setPrintHeaderTable( printDialog->printHeaderTable() );
		View::self()->setPrintBackground( printDialog->printBackground() );
		View::self()->setPrintWidth( printDialog->printWidth() );
		View::self()->setPrintHeight( printDialog->printHeight() );
		View::self()->draw(printer, View::Printer);
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


void MainDlg::editConstantsModal(QWidget *parent)
{
	if (m_constantEditor) {
		m_constantEditor->hide();
	}
	else {
		m_constantEditor = new KConstantEditor(parent);
	}

	m_constantEditor->setModal(true);
	m_constantEditor->show();
}

bool MainDlg::fileExists(const QUrl &url)
{
	bool fileExists = false;
	if (url.isValid()) {
		short int detailLevel = 0; // Lowest level: file/dir/symlink/none
		KIO::StatJob* statjob = KIO::stat(url, KIO::StatJob::SourceSide, detailLevel);
		bool noerror = statjob->exec();
		if (noerror) {
			// We want a file
			fileExists = !statjob->statResult().isDir();
		}
	}
	return fileExists;
}

void MainDlg::slotNames()
{
	KHelpClient::invokeHelp( "func-predefined", "kmplot" );
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
	View::self()->updateSliders();
}

void MainDlg::setReadOnlyStatusBarText(const QString &text)
{
	setStatusBarText(text);
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
		connect( m_coordsDialog, &CoordsConfigDialog::settingsChanged, View::self(), QOverload<>::of(&View::drawPlot) );
	}

	return m_coordsDialog;
}
//END class MainDlg


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
