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
#include "xparser.h"
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
Ufkt::Ufkt( Type type )
	: m_type( type )
{
	id = 0;
	mem = new unsigned char [MEMSIZE];
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
	
	// handle parameters separately
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
	
// 	kDebug() << k_funcinfo << "changed="<<changed<<endl;
	return changed;
}


QString Ufkt::fname( ) const
{
	int pos = m_fstr.indexOf( '(' );
	if ( pos == -1 )
	{
		kWarning() << k_funcinfo << "No bracket!\n";
		return QString();
	}
	
	return m_fstr.left( pos );
}


QString Ufkt::fvar( ) const
{
	int p1 = m_fstr.indexOf( '(' );
	if ( p1 == -1 )
	{
		kWarning() << k_funcinfo << "No bracket!\n";
		return QString();
	}
	
	int p2 = m_fstr.indexOf( ',' );
	if ( p2 == -1 )
		p2 = m_fstr.indexOf( ')' );
	
	if ( p2 == -1 )
	{
		kWarning() << k_funcinfo << "No closing!\n";
		return QString();
	}
	
	return m_fstr.mid( p1+1, p2-p1-1 );
}


QString Ufkt::fpar( ) const
{
	int p1 = m_fstr.indexOf( ',' );
	if ( p1 == -1 )
	{
		// no parameter
		return QString();
	}
	
	int p2 = m_fstr.indexOf( ')' );
	if ( p2 == -1 )
	{
		kWarning() << k_funcinfo << "No closing bracket!\n";
		return QString();
	}
	
	return m_fstr.mid( p1+1, p2-p1-1 );
}


bool Ufkt::setFstr( const QString & fstr, bool force  )
{
// 	kDebug() << "fstr: "<<fstr<<endl;
	
	if ( force )
	{
		m_fstr = fstr;
		return true;
	}
	
	if ( !XParser::self()->isFstrValid( fstr ) )
	{
		XParser::self()->parserError( true );
// 		kDebug() << "invalid fstr\n";
		return false;
	}
	
	QString prevFstr = m_fstr;
	m_fstr = fstr;
	XParser::self()->initFunction( this );
	if ( XParser::self()->parserError( true ) != Parser::ParseSuccess )
	{
		m_fstr = prevFstr;
		XParser::self()->initFunction( this );
// 		kDebug() << "BAD\n";
		return false;
	}
	else
	{
// 		kDebug() << "GoOd :)\n";
		return true;
	}
}
//END class Ufkt
