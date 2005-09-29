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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

/**
 * Version string of KmPlot.
 */
#define KP_VERSION "1.2.0"

// local includes
#include "kmplot.h"


// KDE includes
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>



static const char description[] =
    I18N_NOOP( "Mathematical function plotter for KDE" );

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
	    0,
	    "http://edu.kde.org/kmplot/" );
	aboutData.addAuthor(
	    "Klaus-Dieter Möller", I18N_NOOP( "Original Author" ) ,
	    "kdmoeller@foni.net" );
	aboutData.addAuthor(
	    "Matthias Meßmer", I18N_NOOP( "GUI" ) ,
	    "bmlmessmer@web.de" );
	aboutData.addAuthor( "Fredrik Edemar", I18N_NOOP( "Various improvements" ), "f_edemar@linux.se" );
	aboutData.addCredit( "David Vignoni", I18N_NOOP( "svg icon" ), "david80v@tin.it" );
	aboutData.addCredit( "Albert Astals Cid", I18N_NOOP( "command line options, MIME type" ), "tsdgeos@terra.es" );

	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	KApplication ka;
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
	KmPlot *w = new KmPlot( args );
	w->show();
	ka.setMainWidget( w );
	
	return ka.exec();
}

