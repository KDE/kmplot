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
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

// local includes
#include "xparser.h"



XParser::XParser()
{
	int ix;

	fktext = new FktExt[ UFANZ ];
	for ( ix = 0; ix < UFANZ; ++ix )
	{
		fktext[ ix ].color = 0;
		// setup slider support
		fktext[ ix ].slider_min = fktext[ ix ].slider_max = 0.0;
		fktext[ ix ].use_slider = -1;
	}
	setDecimalSymbol( KGlobal::locale()->decimalSymbol() );
}

XParser::XParser( int anz, int m_size, int s_size ) : Parser( anz, m_size, s_size )
{
	int ix;

	fktext = new FktExt[ ufanz ];
	for ( ix = 0; ix < ufanz; ++ix )
	{
		fktext[ ix ].color = 0;
		// setup slider support
		fktext[ ix ].slider_min = fktext[ ix ].slider_max = 0.0;
		fktext[ ix ].use_slider = -1;
	}
	setDecimalSymbol( KGlobal::locale()->decimalSymbol() );
}

XParser::~XParser()
{
	delete [] fktext;
}

int XParser::getext( int ix )
{
	int errflg = 0, p1, p2, p3, pe;
	QCString str, tstr;

	if ( fktext[ ix ].extstr.find( ';' ) == -1 )
		return 0;

	pe = fktext[ ix ].extstr.length();
	if ( fktext[ ix ].extstr.find( 'N' ) != -1 )
		fktext[ ix ].f_mode = 0;
	else
	{
		if ( fktext[ ix ].extstr.find( "A1" ) != -1 )
			fktext[ ix ].f1_mode = 1;
		if ( fktext[ ix ].extstr.find( "A2" ) != -1 )
			fktext[ ix ].f2_mode = 1;
	}
	switch ( fktext[ ix ].extstr[ 0 ].latin1() )
	{
	case 'x':
	case 'y':
	case 'r':
		fktext[ ix ].f1_mode = fktext[ ix ].f2_mode = 0;
	}

	p1 = fktext[ ix ].extstr.find( "D[" );
	if ( p1 != -1 )
	{
		p1 += 2;
		str = ( fktext[ ix ].extstr.mid( p1, pe - p1 ) ).latin1();
		p2 = str.find( ',' );
		p3 = str.find( ']' );
		if ( p2 > 0 && p2 < p3 )
		{
			tstr = str.left( p2 );
			fktext[ ix ].dmin = eval( tstr );
			if ( err )
				errflg = 1;
			tstr = str.mid( p2 + 1, p3 - p2 - 1 );
			fktext[ ix ].dmax = eval( tstr );
			if ( err )
				errflg = 1;
			if ( fktext[ ix ].dmin > fktext[ ix ].dmax )
				errflg = 1;
		}
		else
			errflg = 1;
	}

	p1 = fktext[ ix ].extstr.find( "P[" );
	if ( p1 != -1 )
	{
		int i = 0;

		p1 += 2;
		str = ( fktext[ ix ].extstr.mid( p1, 1000 ) ).latin1();
		p3 = str.find( ']' );
		do
		{
			p2 = str.find( ',' );
			if ( p2 == -1 || p2 > p3 )
				p2 = p3;

			tstr = str.left( p2++ );
			str = str.mid( p2, 1000 );
			fktext[ ix ].k_liste[ i++ ] = eval( tstr );
			if ( err )
			{
				errflg = 1;
				break;
			}
			p3 -= p2;
		}
		while ( p3 > 0 && i < 10 );
		fktext[ ix ].k_anz = i;
	}

	if ( errflg )
	{
		KMessageBox::error( 0, i18n( "Error in extension." ) );
		delfkt( ix );
		return -1;
	}
	else
		return 0;
}

int XParser::delfkt( int ix )
{
	if ( Parser::delfkt( ix ) == -1 )
		return -1;

	fktext[ ix ].f_mode = fktext[ ix ].f1_mode = fktext[ ix ].f2_mode = fktext[ ix ].anti_mode = 0;
	fktext[ ix ].linewidth = dicke0;
	fktext[ ix ].k_anz = 0;
	fktext[ ix ].dmin = fktext[ ix ].dmax = 0.;
	fktext[ ix ].extstr = ""; //.resize(1);
//	fktext[ ix ].color = fktext[ ix ].color0;
	return ix;
}

double XParser::a1fkt( int ix, double x, double h )
{
	return ( ufkt[ ix ].fkt( x + h ) - ufkt[ ix ].fkt( x ) ) / h;
}

double XParser::a2fkt( int ix, double x, double h )
{
	return ( ufkt[ ix ].fkt( x + h + h ) - 2 * ufkt[ ix ].fkt( x + h ) + ufkt[ ix ].fkt( x ) ) / h / h;
}

char XParser::findFunctionName()
{
	char function_name ='f';
	for (bool ok=true; function_name< 'z'+1 ;function_name++)
	{
		for ( int i = 0; i < ufanz; i++ )
		{
			if (fktext[ i ].extstr.at(0) == function_name )
				if (fktext[ i ].extstr.at(1) == '(' )
					ok = false;
		}
		if ( ok)
		{
			return function_name;
		}
		ok = true;
	}
}
void XParser::fixFunctionName( QString &str)
{
	int p1=str.find('(');
	int p2=str.find(')');
	
	if ( p1==-1 || !str.at(p1+1).isLetter() ||  p2==-1 || str.at(p2+1) != '=')
	{
		char function_name = findFunctionName();
		str.prepend("(x)=");
		str.prepend(function_name);
	}
}


void XParser::euler_method(double &x, double &y, const int &index)
{
	if (x == fktext[index].startx ) //the first point we should draw
	{
		ufkt[index].oldy = fktext[index].starty;
		fktext[index].oldyprim = fktext[index].anti_precision;
		
		
		/*kdDebug() << "*******************" << endl;
		kdDebug() << "   start-x: " << x << endl;
		kdDebug() << "   start-y: " << m_parser->fktext[index].starty << endl;
		kdDebug() << "*******************" << endl;*/
		
		fktext[index].oldx = x;
		y=fktext[index].starty;
		return;
	}
	else
	{
		double yprim = y;
		double h = x-fktext[index].oldx;
		y = ufkt[index].oldy + (h *  fktext[index].oldyprim);
		
		ufkt[index].oldy = y;
		fktext[index].oldx = x;
		fktext[index].oldyprim = yprim;
		return;
	}
}
