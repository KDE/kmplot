/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar <f_edemar@linux.se>
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

#include "kmplot.h"

#include <kaction.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kshortcutsdialog.h>
#include <kfiledialog.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmplotprogress.h>
#include <kstatusbar.h>
#include <kstandardaction.h>
#include <kurl.h>
#include <kactioncollection.h>
#include "maindlg.h"
#include <ktoolinvocation.h>
#include <ktogglefullscreenaction.h>
#include <kapplication.h>

#include "kmplotadaptor.h"

KmPlot::KmPlot( KCmdLineArgs* args)
		: KParts::MainWindow()
{
	setObjectName( "KmPlot" );

	// set the shell's ui resource file
	setXMLFile("kmplot_shell.rc");
	// then, setup our actions
	setupActions();

	// setup the status bar
	setupStatusBar();

	// this routine will find and load our Part.  it finds the Part by
	// name which is a bad idea usually.. but it's alright in this
	// case since our Part is made for this Shell
    KPluginFactory *factory = KPluginLoader("libkmplotpart").factory();
	if (factory)
	{
		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
        m_part = factory->create<KParts::ReadWritePart>(this);
		if (m_part)
		{
			// tell the KParts::MainWindow that this is indeed the main widget
			setCentralWidget(m_part->widget());
			//m_part->widget()->setFocus();
			// and integrate the part's GUI with the shell's
			createGUI(m_part);
		}
	}
	else
	{
		// if we couldn't find our Part, we exit since the Shell by
		// itself can't do anything useful
		KMessageBox::error(this, i18n("Could not find KmPlot's part."));
		qApp->quit();
		// we return here, cause qApp->quit() only means "exit the
		// next time we enter the event loop...
		return;
	}

	if (!initialGeometrySet())
		resize( QSize(800, 520).expandedTo(minimumSizeHint()));

	// apply the saved mainwindow settings, if any, and ask the mainwindow
	// to automatically save settings if changed: window size, toolbar
	// position, icon size, etc.
	setAutoSaveSettings();
	if (args)
	{
		bool exit = false;
		for (int i=0; i < args->count(); i++ )
		{
			if (i==0)
			{
				if (!load(args->url(0) ) )
					exit = true;
			}
			else
				openFileInNewWindow( args->url(i) );
		}
		if (exit)
			deleteLater(); // couln't open the file, and therefore exit
	}

	show();

    new KmPlotAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/kmplot", this);

    if ( args && args->isSet("function") )
    {
        QString f = args->getOption("function");
        QDBusReply<bool> reply = QDBusInterface( QDBusConnection::sessionBus().baseService(), "/parser", "org.kde.kmplot.Parser").call( QDBus::BlockWithGui, "addFunction", f, "" );
    }
    if( args )
	args->clear();
}

KmPlot::~KmPlot()
{}

void KmPlot::slotUpdateFullScreen( bool checked)
{
	if (checked)
	{
		KToggleFullScreenAction::setFullScreen( this, true );
		//m_fullScreen->plug( toolBar( "mainToolBar" ) ); deprecated annma 2006-03-01
	}
	else
	{
		KToggleFullScreenAction::setFullScreen( this, false );
		//m_fullScreen->unplug( toolBar( "mainToolBar" ) ); deprecated annma 2006-03-01
	}
}

bool KmPlot::load(const KUrl& url)
{
	m_part->openUrl( url );
  if (m_part->url().isEmpty())
    return false;
  setCaption(url.prettyUrl(KUrl::LeaveTrailingSlash));
  return true;
}

void KmPlot::setupActions()
{
	KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
	KStandardAction::open(this, SLOT(fileOpen()), actionCollection());
	KStandardAction::quit(kapp, SLOT(quit()), actionCollection());

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);

	KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());

	m_fullScreen = KStandardAction::fullScreen( NULL, NULL, this, actionCollection());
	actionCollection()->addAction("fullscreen", m_fullScreen);
	connect( m_fullScreen, SIGNAL( toggled( bool )), this, SLOT( slotUpdateFullScreen( bool )));
}

void KmPlot::fileNew()
{
	// About this function, the style guide (
	// http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
	// says that it should open a new window if the document is _not_
	// in its initial state.  This is what we do here..
	if ( !m_part->url().isEmpty() || isModified() )
		//KToolInvocation::startServiceByDesktopName("kmplot");
		KToolInvocation::kdeinitExec("kmplot");
}


void KmPlot::optionsConfigureKeys()
{
	/// \todo check that configuring keys works
// 	KShortcutsDialog::configure(actionCollection(), "kmplot_shell.rc");
	KShortcutsDialog::configure( actionCollection() );
}

void KmPlot::optionsConfigureToolbars()
{
	saveMainWindowSettings( KGlobal::config()->group( QString() ));
	// use the standard toolbar editor
	KEditToolBar dlg(factory());
	connect(&dlg, SIGNAL(newToolBarConfig()), this, SLOT(applyNewToolbarConfig()));
	dlg.exec();
}

void KmPlot::applyNewToolbarConfig()
{
	applyMainWindowSettings(KGlobal::config()->group( QString() ));
}

void KmPlot::fileOpen()
{
	// this slot is called whenever the File->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	KUrl const url = KFileDialog::getOpenUrl( QDir::currentPath(),
	                 i18n( "*.fkt|KmPlot Files (*.fkt)\n*|All Files" ), this, i18n( "Open" ) );

	if ( !url.isEmpty())
	{
		// About this function, the style guide (
		// http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
		// says that it should open a new window if the document is _not_
		// in its initial state.  This is what we do here..
		if ( m_part->url().isEmpty() && !isModified() )
			load( url ); // we open the file in this window...
		else
			openFileInNewWindow(url); // we open the file in a new window...
	}
}

void KmPlot::fileOpen(const KUrl &url)
{
	if ( !url.isEmpty())
	{
		// About this function, the style guide (
		// http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
		// says that it should open a new window if the document is _not_
		// in its initial state.  This is what we do here..
		if ( m_part->url().isEmpty() && !isModified() )
         load( KStandardDirs::realFilePath(url.url())); // we open the file in this window...
		else
         openFileInNewWindow(url); // we open the file in a new window...
	}
}


void KmPlot::openFileInNewWindow(const KUrl &url)
{
 KToolInvocation::startServiceByDesktopName("kmplot",url.url());
}

bool KmPlot::isModified()
{
	QDBusReply<bool> reply = QDBusInterface( QDBusConnection::sessionBus().baseService(), "/maindlg", "org.kde.kmplot.MainDlg").call( QDBus::BlockWithGui, "isModified" );
    return reply.value();
}

bool KmPlot::queryClose()
{
	return m_part->queryClose();
}

void KmPlot::setStatusBarText(const QString &text, int id)
{
 	statusBar()->changeItem(text,id);
}


void KmPlot::setupStatusBar()
{
	statusBar()->insertFixedItem( "1234567890123456", 1 );
	statusBar()->insertFixedItem( "1234567890123456", 2 );
	statusBar()->insertItem( "", 3, 3 );
	statusBar()->insertItem( "", 4 );
	statusBar()->changeItem( "", 1 );
	statusBar()->changeItem( "", 2 );
	statusBar()->setItemAlignment( 3, Qt::AlignLeft );

	m_progressBar = new KmPlotProgress( statusBar() );
	m_progressBar->setMaximumHeight( statusBar()->height()-10 );
	connect( m_progressBar, SIGNAL (cancelDraw() ), this, SLOT( cancelDraw() ) );
	statusBar()->addWidget(m_progressBar);
}


void KmPlot::setDrawProgress( double progress )
{
	m_progressBar->setProgress( progress );
}


void KmPlot::cancelDraw()
{
	QDBusInterface( QDBusConnection::sessionBus().baseService(), "/kmplot", "org.kde.kmplot.KmPlot" ).call( QDBus::NoBlock, "stopDrawing" );
}


#include "kmplot.moc"
