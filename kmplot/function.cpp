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


//BEGIN class Value
Value::Value( const QString & expression )
{
	m_value = 0.0;
	if ( ! expression.isEmpty() )
		updateExpression( expression );
}


bool Value::updateExpression( const QString & expression )
{
	double newValue = XParser::self()->eval( expression );
	if ( XParser::self()->parserError( false ) )
		return false;
	
	m_value = newValue;
	m_expression = expression;
	return true;
}


bool Value::operator == ( const Value & other )
{
	return m_expression == other.expression();
}
//END class Value



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



//BEGIN class Equation
Equation::Equation( Type type, Function * parent )
	: m_type( type ),
	  m_parent( parent )
{
	mem = new unsigned char [MEMSIZE];
	mptr = 0;
	oldyprim = 0.0;
	oldx = 0.0;
	oldy = 0;
}


Equation::~ Equation()
{
	delete [] mem;
	mem = 0;
}


QString Equation::fname( ) const
{
	int pos = m_fstr.indexOf( '(' );
	if ( pos == -1 )
	{
		kWarning() << k_funcinfo << "No bracket!\n";
		return QString();
	}
	
	return m_fstr.left( pos );
}


QString Equation::fvar( ) const
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


QString Equation::fpar( ) const
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


bool Equation::setFstr( const QString & fstr, bool force  )
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
	XParser::self()->initEquation( this );
	if ( XParser::self()->parserError( true ) != Parser::ParseSuccess )
	{
		m_fstr = prevFstr;
		XParser::self()->initEquation( this );
// 		kDebug() << "BAD\n";
		return false;
	}
	else
	{
// 		kDebug() << "GoOd :)\n";
		return true;
	}
}
//END class Equation


//BEGIN class Function
Function::Function( Type type )
	: m_type( type )
{
	eq = new Equation(
			(type == Cartesian) ? Equation::Cartesian :
			(type == ParametricX) ? Equation::ParametricX :
			(type == ParametricY) ? Equation::ParametricY : Equation::Polar,
					 this
					 );
	
	id = 0;
	f0.visible = true;
	integral_use_precision = false;
	
	k = 0;
	integral_precision = Settings::stepWidth();
	use_slider = -1;
	
	// min/max stuff
	dmin.updateExpression( QString("-")+QChar(960) );
	dmax.updateExpression( QChar(960) );
	usecustomxmin = false;
	usecustomxmax = false;
}


Function::~Function()
{
	delete eq;
}


bool Function::copyFrom( const Function & function )
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
	COPY_AND_CHECK( dmin.expression() );					// 5
	COPY_AND_CHECK( dmax.expression() );					// 6
	COPY_AND_CHECK( dmin );						// 7
	COPY_AND_CHECK( dmax );						// 8
	COPY_AND_CHECK( startx.expression() );				// 9
	COPY_AND_CHECK( starty.expression() );				// 10
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
		foreach ( Value p, parameters )
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
//END class Function
