/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999  Klaus-Dieter M�ler
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

/**
 * Version string of KmPlot.
 */
#define	KP_VERSION	"1.0"


// KDE includes
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

// local includes
#include "MainDlg.h"

static const char description[] =
    I18N_NOOP( "KmPlot is a mathematical function plotter for the KDE Desktop." );

static KCmdLineOptions options[] =
    {
        { "+[URL]", I18N_NOOP( "File to open" ), 0 },
        KCmdLineLastOption
        // INSERT YOUR COMMANDLINE OPTIONS HERE
    };


int main( int argc, char **argv )
{
	KAboutData aboutData(
	    "kmplot",
	    I18N_NOOP( "KmPlot" ),
	    KP_VERSION, description, KAboutData::License_GPL,
	    "(c) 2000-2002, Klaus-Dieter Möller",
	    0, 0,
	    "submit@bugs.kde.org" );
	aboutData.addAuthor(
	    "Klaus-Dieter Möller", 0 ,
	    "kd.moeller@t-online.de" );
	aboutData.addCredit("David Vignoni", I18N_NOOP("svg icon"), "david80v@tin.it");
	aboutData.addCredit("Fredrik Edemar", I18N_NOOP("fullscreen mode"), "08.7683331@telia.com");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	KApplication ka;
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	MainDlg *w = new MainDlg( ka.sessionId(), args );
	w->resize( 450, 400 );
	w->show();
	ka.setMainWidget( w );
	return ka.exec();
}
