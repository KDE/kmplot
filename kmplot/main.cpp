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

// local includes
#include "main.h"

// KDE includes
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

static const char description[] =
    I18N_NOOP( "KmPlot is a mathematical function plotter for the KDE Desktop." );

static KCmdLineOptions options[] =
    {
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
	aboutData.addCredit("David Vignoni",
                 I18N_NOOP("SVG icon" ),
                 "david80v@tin.it");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	KApplication ka;
	kc = KGlobal::config();
	MainDlg *w = new MainDlg;
	w->resize( 450, 400 );
	//w->setPalette( QPalette( QColor( 255, 255, 255 ) ) );
	w->show();
	ka.setMainWidget( w );
	return ka.exec();
}
