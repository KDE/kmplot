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

#include "xparser.h"

#include "kmessagebox.h"



XParser::XParser()
{
	int ix;

	fktext = new FktExt[ UFANZ ];
	for ( ix = 0; ix < UFANZ; ++ix )
		fktext[ ix ].farbe0 = 0;
}

XParser::XParser( int anz, int m_size, int s_size ) : Parser( anz, m_size, s_size )
{
	int ix;

	fktext = new FktExt[ ufanz ];
	for ( ix = 0; ix < ufanz; ++ix )
		fktext[ ix ].farbe0 = 0;
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

	fktext[ ix ].f_mode = 1;
	fktext[ ix ].f1_mode = fktext[ ix ].f2_mode = 0;
	fktext[ ix ].dicke = dicke0;
	fktext[ ix ].k_anz = 0;
	fktext[ ix ].dmin = fktext[ ix ].dmax = 0.;
	fktext[ ix ].extstr = ""; //.resize(1);
	fktext[ ix ].farbe = fktext[ ix ].farbe0;
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
