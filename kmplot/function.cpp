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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "function.h"
#include "settings.h"

#include <kdebug.h>

#include <cmath>


//BEGIN class Plot
Plot::Plot( )
{
	lineWidth = Settings::gridLineWidth();
	color = Qt::black;
	visible = false;
}


bool Plot::operator !=( const Plot & other ) const
{
	return (visible != other.visible) ||
			(color != other.color) ||
			(lineWidth != other.lineWidth);
}
//END class Plot


//BEGIN class Ufkt
Ufkt::Ufkt()
{
	id = 0;
	mem = 0;
	mptr = 0;
	k = 0;
	oldy = 0;
	f0.visible = true;
	integral_use_precision = false;
	
	oldyprim = 0.0;
	oldx = 0.0;
	starty = 0.0;
	startx = 0.0;
	integral_precision = Settings::stepWidth();
	use_slider = -1;
	
	// min/max stuff
	dmin = -M_PI;
	dmax = M_PI;
	str_dmin = QString("-")+QChar(960);
	str_dmax = QChar(960);
	usecustomxmin = false;
	usecustomxmax = false;
}


Ufkt::~Ufkt()
{
	delete [] mem;
	mem = 0;
}


bool Ufkt::copyFrom( const Ufkt & function )
{
	bool changed = false;
	int i = 0;
#define COPY_AND_CHECK(s) \
		if ( s != function.s ) \
{ \
			kDebug() << "i="<<i/*<<" this="<<s<<" that="<<function.s*/<<endl; \
			s = function.s; \
			changed = true; \
} \
		i++;
	
	COPY_AND_CHECK( f0 );						// 0
	COPY_AND_CHECK( f1 );						// 1
	COPY_AND_CHECK( f2 );						// 2
	COPY_AND_CHECK( integral );					// 3
	COPY_AND_CHECK( integral_use_precision );	// 4
	COPY_AND_CHECK( str_dmin );					// 5
	COPY_AND_CHECK( str_dmax );					// 6
	COPY_AND_CHECK( dmin );						// 7
	COPY_AND_CHECK( dmax );						// 8
	COPY_AND_CHECK( str_startx );				// 9
	COPY_AND_CHECK( str_starty );				// 10
	COPY_AND_CHECK( starty );					// 11
	COPY_AND_CHECK( startx );					// 12
	COPY_AND_CHECK( integral_precision );		// 13
	COPY_AND_CHECK( use_slider );				// 14
	COPY_AND_CHECK( usecustomxmin );			// 15
	COPY_AND_CHECK( usecustomxmax );			// 16
	
#if 0
	COPY_AND_CHECK( parameters );
#endif
	// handle this separately
	if ( parameters.count() != function.parameters.count() )
{
	changed = true;
	parameters = function.parameters;
}
	else
{
	foreach ( ParameterValueItem p, parameters )
	{
		if ( !function.parameters.contains( p ) )
		{
			changed = true;
			parameters = function.parameters;
			break;
		}
	}
}
	
	kDebug() << k_funcinfo << "changed="<<changed<<endl;
	return changed;
}
//END class Ufkt
