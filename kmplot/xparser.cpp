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

int XParser::getext( Ufkt *item )
{
	int errflg = 0, p1, p2, p3, pe;
	QCString str, tstr;

	if ( item->extstr.find( ';' ) == -1 )
		return 0;

	pe = item->extstr.length();
	if ( item->extstr.find( 'N' ) != -1 )
		item->f_mode = 0;
	else
	{
		if ( item->extstr.find( "A1" ) != -1 )
			item->f1_mode = 1;
		if ( item->extstr.find( "A2" ) != -1 )
			item->f2_mode = 1;
	}
	switch ( item->extstr[ 0 ].latin1() )
	{
	case 'x':
	case 'y':
	case 'r':
		item->f1_mode = item->f2_mode = 0;
	}

	p1 = item->extstr.find( "D[" );
	if ( p1 != -1 )
	{
		p1 += 2;
		str = ( item->extstr.mid( p1, pe - p1 ) ).latin1();
		p2 = str.find( ',' );
		p3 = str.find( ']' );
		if ( p2 > 0 && p2 < p3 )
		{
			tstr = str.left( p2 );
			item->dmin = eval( tstr );
			if ( err )
				errflg = 1;
			tstr = str.mid( p2 + 1, p3 - p2 - 1 );
			item->dmax = eval( tstr );
			if ( err )
				errflg = 1;
			if ( item->dmin > item->dmax )
				errflg = 1;
		}
		else
			errflg = 1;
	}

	p1 = item->extstr.find( "P[" );
	if ( p1 != -1 )
	{
		int i = 0;

		p1 += 2;
		str = ( item->extstr.mid( p1, 1000 ) ).latin1();
		p3 = str.find( ']' );
		do
		{
			p2 = str.find( ',' );
			if ( p2 == -1 || p2 > p3 )
				p2 = p3;

			tstr = str.left( p2++ );
			str = str.mid( p2, 1000 );
			item->k_liste.append( eval( tstr ) );
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

double XParser::a1fkt( Ufkt *u_item, double x, double h )
{
	return ( fkt(u_item, x + h ) - fkt( u_item, x ) ) / h;
}

double XParser::a2fkt( Ufkt *u_item, double x, double h )
{
	return ( fkt( u_item, x + h + h ) - 2 * fkt( u_item, x + h ) + fkt( u_item, x ) ) / h / h;
}

void XParser::findFunctionName(QString &function_name, int const id, int const type)
{
        char last_character;
        int pos;
        if ( type == XParser::Polar)
                pos=1;
        else
                pos=0;
        for ( ; ; ++pos)
        {
                last_character = 'f';
                for (bool ok=true; last_character<'x'; ++last_character)
                {
                        if ( pos==0 && last_character == 'r') continue;
                        function_name.at(pos)=last_character;
                        for( QValueVector<Ufkt>::iterator it = ufkt.begin(); it != ufkt.end(); ++it)
                        {
                                if (it == ufkt.begin() && it->fname.isEmpty() ) continue;
                                if ( it->extstr.startsWith(function_name+'(') && (int)it->id!=id) //check if the name is free
                                                ok = false;
                        }
                        if ( ok) //a free name was found
                        {
                                //kdDebug() << "function_name:" << function_name << endl;
                                return;
                        }
                        ok = true;
	       }
               function_name.at(pos)='f';
               function_name.append('f');
        }
        function_name = "e"; //this should never happen
}

void XParser::fixFunctionName( QString &str, int const type, int const id)
{
	int const p1=str.find('(');
	int const p2=str.find(')');
	if ( p1==-1 || !str.at(p1+1).isLetter() ||  p2==-1 || str.at(p2+1 ) != '=')
	{
                QString function_name;
                if ( type == XParser::Polar )
                        function_name = "rf";
                else
                        function_name = "f";
                str.prepend("(x)=");
                findFunctionName(function_name, id, type);
		str.prepend( function_name );
	}
}

double XParser::euler_method(const double x, const QValueVector<Ufkt>::iterator it)
{
	double const y = it->oldy + ((x-it->oldx) * it->oldyprim);
	it->oldy = y;
	it->oldx = x;
	it->oldyprim = fkt( it, x ); //yprim;
	return y;
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

void XParser::prepareAddingFunction(Ufkt *temp)
{
        temp->color = temp->f1_color = temp->f2_color = temp->integral_color = defaultColor(getNextIndex() );
        temp->linewidth = temp->f1_linewidth = temp->f2_linewidth = temp->integral_linewidth = linewidth0;
        temp->f_mode = true;
        temp->f1_mode = false;
        temp->f2_mode = false;
        temp->integral_mode = false;
        temp->integral_precision = Settings::relativeStepWidth();
        temp->dmin = 0;
        temp->dmax = 0;
        temp->str_dmin = "";
        temp->str_dmax = "";
        temp->use_slider = -1;
        //TODO temp->slider_min = 0; temp->slider_max = 50;

}
int XParser::getNextIndex()
{
        //return ufkt.count();
        return getNewId();
}
