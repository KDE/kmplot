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

// KDE includes
#include <kmessagebox.h>
#include <kurl.h>

// local includes
#include "misc.h"
#include "settings.h"

KApplication *ka;

XParser ps( 10, 200, 20 );

double sw,       // Schrittweite
rsw,       // rel. Schrittweite
tlgx,       // x-Achsenteilung
tlgy,       // y-Achsenteilung
drskalx,       // x-Ausdruckskalierung
drskaly;     // y-Ausdruckskalierung

QString xminstr,      // String fr xmind
xmaxstr,      // String fr xmaxd
yminstr,      // String fr ymind
ymaxstr,      // String fr ymaxd
tlgxstr,                  // String fr tlgx
tlgystr,                  // String fr tlgy
drskalxstr,               // String fr drskalx
drskalystr;             // String fr drskaly

QString font_header; // Font family names

void getSettings()
{
	rsw = 1.;

	// axes settings
	
	xminstr = Settings::xMin();
	xmaxstr = Settings::xMax();
	yminstr = Settings::yMin();
	ymaxstr = Settings::yMax();
	
	if( xminstr.isEmpty() ) xminstr = "-2*pi";
	if( xmaxstr.isEmpty() ) xmaxstr = "2*pi";
	if( yminstr.isEmpty() ) yminstr = "-2*pi";
	if( ymaxstr.isEmpty() ) ymaxstr = "2*pi";

	// graph settings

	ps.dicke0 = Settings::gridLineWidth();
	ps.fktext[ 0 ].color = Settings::color0().rgb();
	ps.fktext[ 1 ].color = Settings::color1().rgb();
	ps.fktext[ 2 ].color = Settings::color2().rgb();
	ps.fktext[ 3 ].color = Settings::color3().rgb();
	ps.fktext[ 4 ].color = Settings::color4().rgb();
	ps.fktext[ 5 ].color = Settings::color5().rgb();
	ps.fktext[ 6 ].color = Settings::color6().rgb();
	ps.fktext[ 7 ].color = Settings::color7().rgb();
	ps.fktext[ 8 ].color = Settings::color8().rgb();
	ps.fktext[ 9 ].color = Settings::color9().rgb();
	
	// precision settings
	rsw = Settings::stepWidth();
}

void init()
{
	getSettings();

	for ( int ix = 0; ix < ps.ufanz; ++ix )
		ps.delfkt( ix );
}
