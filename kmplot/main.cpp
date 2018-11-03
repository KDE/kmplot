/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
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
static const char* version =  "1.2.1";

// local includes
#include "kmplot.h"

// Qt
#include <QApplication>
#include <QCommandLineParser>

// KDE includes
#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>

int main( int argc, char **argv )
{
	QApplication qapp(argc, argv);
    KCrash::initialize();

	KLocalizedString::setApplicationDomain("kmplot");

	KAboutData aboutData(
	    "kmplot",
	    i18n( "KmPlot" ),
	    version, i18n("Mathematical function plotter by KDE"), KAboutLicense::GPL,
	    i18n("(c) 2000-2002, Klaus-Dieter Möller"),
	    QString(),
	    "http://edu.kde.org/kmplot/");
	
	aboutData.addAuthor(
	    i18n("Klaus-Dieter Möller"), i18n( "Original Author" ) ,
	    "kdmoeller@foni.net" );
	aboutData.addAuthor(
	    i18n("Matthias Meßmer"), i18n( "GUI" ) ,
	    "bmlmessmer@web.de" );
	aboutData.addAuthor( i18n("Fredrik Edemar"), i18n( "Various improvements" ), "f_edemar@linux.se" );
	aboutData.addAuthor( i18n("David Saxton"), i18n( "Porting to Qt 4, UI improvements, features" ), "david@bluehaze.org" );
	
	aboutData.addCredit( i18n("David Vignoni"), i18n( "svg icon" ), "david80v@tin.it" );
	aboutData.addCredit( i18n("Albert Astals Cid"), i18n( "command line options, MIME type" ), "aacid@kde.org" );
	KAboutData::setApplicationData(aboutData);

	QCommandLineParser parser;
	parser.addOption(QCommandLineOption(QStringList{"function", "f"}, i18n( "Initial functions to plot" ), i18n("argument")));
	parser.addPositionalArgument("URL", i18n("URLs to open"), "[url...]");
	aboutData.setupCommandLine(&parser);
	parser.process(qapp);
	aboutData.processCommandLine(&parser);
	KDBusService service;
	new KmPlot( parser );
	return qapp.exec();
}

