/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 2004  Fredrik Edemar
*                     f_edemar@linux.se
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
#include <kkeydialog.h>
#include <kfiledialog.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kurl.h>

#include "MainDlg.h"
#include "kmplotprogress.h"

KmPlot::KmPlot( KCmdLineArgs* args)
		: DCOPObject( "KmPlotShell" ), KParts::MainWindow( 0L, "KmPlot" )
{
	// set the shell's ui resource file
	setXMLFile("kmplot_shell.rc");
	// then, setup our actions
	setupActions();
	
	// setup the status bar
	setupStatusBar();
	
	// this routine will find and load our Part.  it finds the Part by
	// name which is a bad idea usually.. but it's alright in this
	// case since our Part is made for this Shell
	KLibFactory *factory = KLibLoader::self()->factory("libkmplotpart");
	if (factory)
	{
		// now that the Part is loaded, we cast it to a Part to get
		// our hands on it
		m_part = static_cast<KParts::ReadOnlyPart *>(factory->create(this,
		         "kmplot_part", "KParts::ReadOnlyPart" ));
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
		kapp->quit();
		// we return here, cause kapp->quit() only means "exit the
		// next time we enter the event loop...
		return;
	}

	if (!initialGeometrySet())
		resize( QSize(450, 520).expandedTo(minimumSizeHint()));

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
		args->clear();
		if (exit)
			deleteLater(); // couln't open the file, and therefore exit
	}
}

KmPlot::~KmPlot()
{}

void KmPlot::slotUpdateFullScreen( bool checked)
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

bool KmPlot::load(const KURL& url)
{
	m_part->openURL( url );
  if (m_part->url().isEmpty())
    return false;
  setCaption(url.prettyURL(0, KURL::StripFileProtocol));
  return true;
}

void KmPlot::setupActions()
{
	KStdAction::openNew(this, SLOT(fileNew()), actionCollection());
	KStdAction::open(this, SLOT(fileOpen()), actionCollection());
	KStdAction::quit(kapp, SLOT(quit()), actionCollection());

	createStandardStatusBarAction();
	setStandardToolBarMenuEnabled(true);
	
	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());

	m_fullScreen = KStdAction::fullScreen( NULL, NULL, actionCollection(), this, "fullscreen");
	connect( m_fullScreen, SIGNAL( toggled( bool )), this, SLOT( slotUpdateFullScreen( bool )));
}

void KmPlot::saveProperties(KConfig* /*config*/)
{
	// the 'config' object points to the session managed
	// config file.  anything you write here will be available
	// later when this app is restored
}

void KmPlot::readProperties(KConfig* /*config*/)
{
	// the 'config' object points to the session managed
	// config file.  this function is automatically called whenever
	// the app is being restored.  read in here whatever you wrote
	// in 'saveProperties'
}

void KmPlot::fileNew()
{
	// About this function, the style guide (
	// http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
	// says that it should open a new window if the document is _not_
	// in its initial state.  This is what we do here..
	if ( !m_part->url().isEmpty() || isModified() )
		//KApplication::startServiceByDesktopName("kmplot");
		KApplication::kdeinitExec("kmplot");
}

bool KmPlot::stopProgressBar()
{
	if (m_progressbar && m_progressbar->isShown())
	{
		m_progressbar->hide();
		return true;
	}
	return false;
}

void KmPlot::startProgressBar(int steps)
{
	if (m_progressbar)
	{
		m_progressbar->progress->setTotalSteps(steps);
		m_progressbar->show();
	}
}

void KmPlot::increaseProgressBar()
{
	if (m_progressbar)
		m_progressbar->increase();
}

void KmPlot::optionsConfigureKeys()
{
	KKeyDialog::configure(actionCollection(), "kmplot_shell.rc");
}

void KmPlot::optionsConfigureToolbars()
{
	saveMainWindowSettings(KGlobal::config() );
	// use the standard toolbar editor
	KEditToolbar dlg(factory());
	connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(applyNewToolbarConfig()));
	dlg.exec();
}

void KmPlot::applyNewToolbarConfig()
{
	applyMainWindowSettings(KGlobal::config());
}

void KmPlot::fileOpen()
{
	// this slot is called whenever the File->Open menu is selected,
	// the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
	// button is clicked
	KURL const url = KFileDialog::getOpenURL( QDir::currentDirPath(),
	                 i18n( "*.fkt|KmPlot Files (*.fkt)\n*.*|All Files" ), this, i18n( "Open" ) );

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

void KmPlot::fileOpen(const KURL &url)
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


void KmPlot::openFileInNewWindow(const KURL url)
{
 KApplication::startServiceByDesktopName("kmplot",url.url());
}

bool KmPlot::checkModified()
{
	QCString replyType;
	QByteArray replyData;
	kapp->dcopClient()->call(kapp->dcopClient()->appId(), "MainDlg","checkModified()", QByteArray(), replyType, replyData, false);
	bool result;
	QDataStream stream(replyData, IO_ReadOnly);
	stream >> result;
	return result;
}

bool KmPlot::isModified()
{
	QCString replyType;
	QByteArray replyData;
	kapp->dcopClient()->call(kapp->dcopClient()->appId(), "MainDlg","isModified()", QByteArray(), replyType, replyData, false);
	bool result;
	QDataStream stream(replyData, IO_ReadOnly);
	stream >> result;
	return result;
}

bool KmPlot::queryClose()
{
	return checkModified();
}

void KmPlot::setStatusBarText(const QString &text, int id)
{
	statusBar()->changeItem(text,id);
}


void KmPlot::setupStatusBar()
{
	statusBar()->insertFixedItem( "1234567890", 1 );
	statusBar()->insertFixedItem( "1234567890", 2 );
	statusBar()->insertItem( "", 3, 3 );
	statusBar()->insertItem( "", 4 );
	statusBar()->changeItem( "", 1 );
	statusBar()->changeItem( "", 2 );
	statusBar()->setItemAlignment( 3, AlignLeft );

	m_progressbar = new KmPlotProgress( statusBar() );
	m_progressbar->setMaximumHeight( statusBar()->height()-10 );
	connect( m_progressbar->button, SIGNAL (clicked() ), this, SLOT( progressbar_clicked() ) );
	statusBar()->addWidget(m_progressbar);
}

void KmPlot::progressbar_clicked()
{
	kapp->dcopClient()->send(kapp->dcopClient()->appId(), "View","stopDrawing()", QByteArray());
}

#include "kmplot.moc"
