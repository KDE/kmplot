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

// Qt includes
#include <qdom.h>
#include <qfile.h>
#include <qstring.h>
#include <qstringlist.h>

// local includes
#include "kmplotio.h"
#include "misc.h"
#include "settings.h"

KmPlotIO::KmPlotIO()
{
}


KmPlotIO::~KmPlotIO()
{
}

void KmPlotIO::save( const QString filename )
{
	// saving as xml by a QDomDocument
	QDomDocument doc( "kmpdoc" );
	// the root tag
	QDomElement root = doc.createElement( "kmpdoc" );
	doc.appendChild( root );

	// the axes tag
	QDomElement tag = doc.createElement( "axes" );

	tag.setAttribute( "color", Settings::axesColor().name() );
	tag.setAttribute( "width", Settings::axesLineWidth() );
	tag.setAttribute( "tic-width", Settings::ticWidth() );
	tag.setAttribute( "tic-legth", Settings::ticLength() );

	addTag( doc, tag, "mode", QString::number( mode ) );
	addTag( doc, tag, "xmin", xminstr );
	addTag( doc, tag, "xmax", xmaxstr );
	addTag( doc, tag, "ymin", yminstr );
	addTag( doc, tag, "ymax", ymaxstr );
	addTag( doc, tag, "xcoord", QString::number( Settings::xRange() ) );
	addTag( doc, tag, "ycoord", QString::number( Settings::yRange() ) );

	root.appendChild( tag );

	tag = doc.createElement( "grid" );

	tag.setAttribute( "color", Settings::gridColor().name() );
	tag.setAttribute( "width", Settings::gridLineWidth() );

	addTag( doc, tag, "mode", QString::number( Settings::gridStyle() ) );

	root.appendChild( tag );

	tag = doc.createElement( "scale" );

	const char* units[ 8 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4" };
	addTag( doc, tag, "tic-x", units[ Settings::xScaling() ] );
	addTag( doc, tag, "tic-y", units[ Settings::yScaling() ] );
	addTag( doc, tag, "print-tic-x", units[ Settings::xPrinting() ] );
	addTag( doc, tag, "print-tic-y", units[ Settings::yPrinting() ] );
	
	root.appendChild( tag );

	addTag( doc, root, "step", QString::number( rsw ) );

	for ( int ix = 0; ix < ps.ufanz; ix++ )
	{
		if ( !ps.fktext[ ix ].extstr.isEmpty() )
		{
			tag = doc.createElement( "function" );

			tag.setAttribute( "number", ix );
			tag.setAttribute( "visible", ps.fktext[ ix ].f_mode );
			tag.setAttribute( "visible-deriv", ps.fktext[ ix ].f1_mode );
			tag.setAttribute( "visible-2nd-deriv", ps.fktext[ ix ].f2_mode );
			tag.setAttribute( "width", ps.fktext[ ix ].dicke );
			tag.setAttribute( "color", QColor( ps.fktext[ ix ].color ).name() );
			
			addTag( doc, tag, "equation", ps.fktext[ ix ].extstr );
			
			if( ps.fktext[ ix ].k_anz > 0 )
			{
				QStringList listOfParameters;
				for( int k_index = 0; k_index < ps.fktext[ ix ].k_anz; k_index++ )
				{
					listOfParameters += 
						QString::number( ps.fktext[ ix ].k_liste[ k_index ] );
				}
				addTag( doc, tag, "parameterlist", listOfParameters.join( "," ) );
			}
			
			root.appendChild( tag );
			
		}
	}
	
	tag = doc.createElement( "fonts" );
	addTag( doc, tag, "axes-font", Settings::axesFont().family() );
	addTag( doc, tag, "header-table-font", Settings::headerTableFont().family() );
	root.appendChild( tag );


	QFile xmlfile( filename );
	xmlfile.open( IO_WriteOnly );
	QTextStream ts( &xmlfile );
	doc.save( ts, 4 );
	xmlfile.close();
}

void KmPlotIO::addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue )
{
	QDomElement tag = doc.createElement( tagName );
	QDomText value = doc.createTextNode( tagValue );
	tag.appendChild( value );
	parentTag.appendChild( tag );
}

void KmPlotIO::load( const QString filename )
{
	init();

	QDomDocument doc( "kmpdoc" );

	QFile f( filename );
	if ( !f.open( IO_ReadOnly ) )
		return ;
	if ( !doc.setContent( &f ) )
	{
		f.close();
		return ;
	}
	f.close();

	QDomElement element = doc.documentElement();
	for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
	{
		if ( n.nodeName() == "axes" )
			parseAxes( n.toElement() );
		if ( n.nodeName() == "grid" )
			parseGrid( n.toElement() );
		if ( n.nodeName() == "scale" )
			parseScale( n.toElement() );
		if ( n.nodeName() == "step" )
			parseStep( n.toElement() );
		if ( n.nodeName() == "function" )
			parseFunction( n.toElement() );
	}
}

void KmPlotIO::parseAxes( const QDomElement &n )
{
	Settings::setAxesLineWidth( n.attribute( "width", "1" ).toInt() );
	Settings::setAxesColor( QColor( n.attribute( "color", "#000000" ) ) );
	Settings::setTicWidth( n.attribute( "tic-width", "3" ).toInt() );
	Settings::setTicLength( n.attribute( "tic-length", "10" ).toInt() );

	mode = n.namedItem( "mode" ).toElement().text().toInt();
	xminstr = n.namedItem( "xmin" ).toElement().text();
	xmaxstr = n.namedItem( "xmax" ).toElement().text();
	yminstr = n.namedItem( "ymin" ).toElement().text();
	ymaxstr = n.namedItem( "ymax" ).toElement().text();
	Settings::setXRange( n.namedItem( "xcoord" ).toElement().text().toInt() );
	Settings::setYRange( n.namedItem( "ycoord" ).toElement().text().toInt() );
}

void KmPlotIO::parseGrid( const QDomElement & n )
{
	Settings::setGridColor( QColor( n.attribute( "color", "#c0c0c0" ) ) );
	Settings::setGridLineWidth( n.attribute( "width", "1" ).toInt() );

	Settings::setGridStyle( n.namedItem( "mode" ).toElement().text().toInt() );
}

int unit2index( const QString unit )
{
	const char* units[ 8 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4" };
	int index = 0;
	while( ( index < 8 ) && ( unit!= units[ index ] ) ) index ++;
	if( index == 8 ) index = -1;
	return index;
}


void KmPlotIO::parseScale( const QDomElement & n )
{
	Settings::setXScaling( unit2index( n.namedItem( "tic-x" ).toElement().text() ) );
	Settings::setYScaling( unit2index( n.namedItem( "tic-y" ).toElement().text() ) );
	Settings::setXPrinting( unit2index( n.namedItem( "print-tic-x" ).toElement().text() ) );
	Settings::setYPrinting( unit2index( n.namedItem( "print-tic-y" ).toElement().text() ) );
}

void KmPlotIO::parseStep( const QDomElement & n )
{
	rsw = n.text().toDouble();
}

void KmPlotIO::parseFunction( const QDomElement & n )
{
	int ix = n.attribute( "number" ).toInt();
	ps.fktext[ ix ].f_mode = n.attribute( "visible" ).toInt();
	ps.fktext[ ix ].f1_mode = n.attribute( "visible-deriv" ).toInt();
	ps.fktext[ ix ].f2_mode = n.attribute( "visible-2nd-deriv" ).toInt();
	ps.fktext[ ix ].dicke = n.attribute( "width" ).toInt();
	ps.fktext[ ix ].color = QColor( n.attribute( "color" ) ).rgb();

	ps.fktext[ ix ].extstr = n.namedItem( "equation" ).toElement().text();
	QCString fstr = ps.fktext[ ix ].extstr.utf8();
	if ( !fstr.isEmpty() )
	{
		int i = fstr.find( ';' );
		QCString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		ix = ps.addfkt( str );
		ps.getext( ix );
		parseParameters( n, ix );
	}
}

void KmPlotIO::parseParameters( const QDomElement &n, int ix )
{
	QStringList listOfParameters = QStringList::split( ",", n.namedItem( "parameterlist" ).toElement().text() );
	ps.fktext[ ix ].k_anz = 0;
	for( QStringList::Iterator it = listOfParameters.begin(); it != listOfParameters.end(); ++it )
	{
		ps.fktext[ ix ].k_liste[ ps.fktext[ ix ].k_anz ] = 
			( *it ).toDouble();
		ps.fktext[ ix ].k_anz++;
	}
	
}
