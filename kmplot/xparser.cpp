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

XParser::XParser( int m_size, int s_size ) : Parser( m_size, s_size )
{
        // setup slider support
	setDecimalSymbol( KGlobal::locale()->decimalSymbol() );
}

XParser::~XParser()
{
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
			fktext[ ix ].k_liste.append( eval( tstr ) );
			if ( err )
			{
				errflg = 1;
				break;
			}
			p3 -= p2;
		}
		while ( p3 > 0 && i < 10 );
	}

	if ( errflg )
	{
		KMessageBox::error( 0, i18n( "Error in extension." ) );
		return -1;
	}
	else
		return 0;
}

void XParser::delfkt( Ufkt *u_item, FktExt *f_item)
{
        Parser::delfkt(u_item);
        fktext.erase( f_item);
}


bool XParser::delfkt( int ix )
{
        if( ix<0 && ix>=int(fktext.count()) )
                return false;
        Parser::delfkt( ix );
        fktext.erase( &fktext[ix]);
        return true;
}

double XParser::a1fkt( int ix, double x, double h )
{
	return ( ufkt[ ix ].fkt( x + h ) - ufkt[ ix ].fkt( x ) ) / h;
}

double XParser::a2fkt( int ix, double x, double h )
{
	return ( ufkt[ ix ].fkt( x + h + h ) - 2 * ufkt[ ix ].fkt( x + h ) + ufkt[ ix ].fkt( x ) ) / h / h;
}

QString XParser::findFunctionName(int const index)
{
	QString function_name("f");
        char last_character;
        for (int pos=0; ; ++pos)
        {
                last_character = 'f';
                for (bool ok=true; last_character<'x'; ++last_character)
                {
                        if ( pos==0 && last_character == 'r') continue;
                        int i = 0;
                        function_name.at(pos)=last_character;
                        for( QValueVector<FktExt>::iterator it = fktext.begin(); it != fktext.end(); ++it)
                        {
                                if ( it->extstr.startsWith(function_name+'(') && i!=index) //check if 
                                                ok = false;
                                ++i;
                        }
                        if ( ok) //free name
                        {
                                //kdDebug() << "function_name:" << function_name << endl;
                                return function_name;
                        }
                        ok = true;
	       }
               function_name.at(pos)='f';
               function_name.append('f');
        }
        function_name = "e";
        return function_name; //this should never happen
}
void XParser::fixFunctionName( QString &str, int const type, int const index)
{
	int const p1=str.find('(');
	int const p2=str.find(')');
	if (str.at(0) == 'r' && str.at(1) == '(')
	{
		str.remove(0,1);
		str.prepend( findFunctionName(index) );
		str.prepend('r');
	}
	else if ( p1==-1 || !str.at(p1+1).isLetter() ||  p2==-1 || str.at(p2+1) != '=')
	{
		str.prepend("(x)=");
		str.prepend( findFunctionName(index) );
	}
}


void XParser::euler_method(double &x, double &y,const int &index)
{
	// y == the old yprim-value
	
	if (x == fktext[index].startx ) //the first point we should draw
	{
		ufkt[index].oldy = fktext[index].starty;
		fktext[index].oldyprim = fktext[index].integral_precision;
		
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
		double const yprim = y;
		double const h = x-fktext[index].oldx;
		y = ufkt[index].oldy + (h *  fktext[index].oldyprim);
		
		ufkt[index].oldy = y;
		fktext[index].oldx = x;
		fktext[index].oldyprim = yprim;
		return;
	}
}

QRgb XParser::defaultColor(int function)
{
        switch ( function )
        {
                case 1:
                        return Settings::color0().rgb();
                        break;
                case 2:
                        return Settings::color1().rgb();
                        break;
                case 3:
                        return Settings::color2().rgb();
                        break;
                case 4:
                        return Settings::color3().rgb();
                        break;
                case 5:
                        return Settings::color4().rgb();
                        break;
                case 6:
                        return Settings::color5().rgb();
                        break;
                case 7:
                        return Settings::color6().rgb();
                        break;
                case 8:
                        return Settings::color7().rgb();
                        break;
                case 9:
                        return Settings::color8().rgb();
                        break;
                case 10:
                        return Settings::color9().rgb();
                        break;
                default:
                        return Settings::color0().rgb();
                        break;
        }
}

void XParser::prepareAddingFktExtFunction(FktExt &temp)
{
        temp.color = temp.f1_color = temp.f2_color = temp.integral_color = defaultColor(fktext.count()+1 );
        temp.linewidth = temp.f1_linewidth = temp.f2_linewidth = temp.integral_linewidth = linewidth0;
        temp.f_mode = true;
        temp.f1_mode = false;
        temp.f2_mode = false;
        temp.integral_mode = false;
        temp.integral_precision = Settings::relativeStepWidth();
        temp.dmin = 0;
        temp.dmax = 0;
        temp.str_dmin = "";
        temp.str_dmax = "";
        temp.use_slider = -1;
        //TODO temp.slider_min = 0; temp.slider_max = 50;

}
int XParser::getNextIndex()
{
        return fktext.count();
}