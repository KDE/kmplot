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
#include "xparser.h"

KmPlotIO::KmPlotIO()
{
}


KmPlotIO::~KmPlotIO()
{
}

void KmPlotIO::save(  XParser *parser, const QString filename )
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

	addTag( doc, tag, "show-axes", Settings::showAxes() ? "1" : "-1" );
	addTag( doc, tag, "show-arrows", Settings::showArrows() ? "1" : "-1" );
	addTag( doc, tag, "show-label", Settings::showLabel() ? "1" : "-1" );
	addTag( doc, tag, "show-frame", Settings::showExtraFrame() ? "1" : "-1" );
	addTag( doc, tag, "show-extra-frame", Settings::showExtraFrame() ? "1" : "-1" );
	
	addTag( doc, tag, "xcoord", QString::number( Settings::xRange() ) );
	if( Settings::xRange() == 4 ) // custom plot range
	{
		addTag( doc, tag, "xmin", Settings::xMin() );
		addTag( doc, tag, "xmax", Settings::xMax() );
	}
	
	addTag( doc, tag, "ycoord", QString::number( Settings::yRange() ) );
	if( Settings::yRange() == 4 ) // custom plot range
	{
		addTag( doc, tag, "ymin", Settings::yMin() );
		addTag( doc, tag, "ymax", Settings::yMax() );
	}

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

	for ( int ix = 0; ix < parser->ufanz; ix++ )
	{
		if ( !parser->fktext[ ix ].extstr.isEmpty() )
		{
			tag = doc.createElement( "function" );

			tag.setAttribute( "number", ix );
			tag.setAttribute( "visible", parser->fktext[ ix ].f_mode );
			tag.setAttribute( "visible-deriv", parser->fktext[ ix ].f1_mode );
			tag.setAttribute( "visible-2nd-deriv", parser->fktext[ ix ].f2_mode );
			tag.setAttribute( "width", parser->fktext[ ix ].dicke );
			tag.setAttribute( "color", QColor( parser->fktext[ ix ].color ).name() );
			
			addTag( doc, tag, "equation", parser->fktext[ ix ].extstr );
			
			if( parser->fktext[ ix ].k_anz > 0 )
			{
				QStringList listOfParameters;
				for( int k_index = 0; k_index < parser->fktext[ ix ].k_anz; k_index++ )
				{
					listOfParameters += 
						QString::number( parser->fktext[ ix ].k_liste[ k_index ] );
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

void KmPlotIO::load( XParser *parser, const QString filename )
{
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
		if ( n.nodeName() == "function" )
			parseFunction( parser, n.toElement() );
	}
}

void KmPlotIO::parseAxes( const QDomElement &n )
{
	Settings::setAxesLineWidth( n.attribute( "width", "1" ).toInt() );
	Settings::setAxesColor( QColor( n.attribute( "color", "#000000" ) ) );
	Settings::setTicWidth( n.attribute( "tic-width", "3" ).toInt() );
	Settings::setTicLength( n.attribute( "tic-length", "10" ).toInt() );

	Settings::setShowAxes( n.namedItem( "show-axes" ).toElement().text().toInt() == 1 );
	Settings::setShowArrows( n.namedItem( "show-arrows" ).toElement().text().toInt() == 1 );
	Settings::setShowLabel( n.namedItem( "show-label" ).toElement().text().toInt() == 1 );
	Settings::setShowFrame( n.namedItem( "show-frame" ).toElement().text().toInt() == 1 );
	Settings::setShowExtraFrame( n.namedItem( "show-extra-frame" ).toElement().text().toInt() == 1 );
	Settings::setXRange( n.namedItem( "xcoord" ).toElement().text().toInt() );
	Settings::setXMin( n.namedItem( "xmin" ).toElement().text() );
	Settings::setXMax( n.namedItem( "xmax" ).toElement().text() );
	Settings::setYRange( n.namedItem( "ycoord" ).toElement().text().toInt() );
	Settings::setYMin( n.namedItem( "ymin" ).toElement().text() );
	Settings::setYMax( n.namedItem( "ymax" ).toElement().text() );
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

void KmPlotIO::parseFunction(  XParser *parser, const QDomElement & n )
{
	int ix = n.attribute( "number" ).toInt();
	parser->fktext[ ix ].f_mode = n.attribute( "visible" ).toInt();
	parser->fktext[ ix ].f1_mode = n.attribute( "visible-deriv" ).toInt();
	parser->fktext[ ix ].f2_mode = n.attribute( "visible-2nd-deriv" ).toInt();
	parser->fktext[ ix ].dicke = n.attribute( "width" ).toInt();
	parser->fktext[ ix ].color = QColor( n.attribute( "color" ) ).rgb();

	parser->fktext[ ix ].extstr = n.namedItem( "equation" ).toElement().text();
	QCString fstr = parser->fktext[ ix ].extstr.utf8();
	if ( !fstr.isEmpty() )
	{
		int i = fstr.find( ';' );
		QCString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		ix = parser->addfkt( str );
		parseParameters( parser, n, ix );
		parser->getext( ix );
	}
}

void KmPlotIO::parseParameters( XParser *parser, const QDomElement &n, int ix )
{
	QStringList listOfParameters = QStringList::split( ",", n.namedItem( "parameterlist" ).toElement().text() );
	parser->fktext[ ix ].k_anz = 0;
	for( QStringList::Iterator it = listOfParameters.begin(); it != listOfParameters.end(); ++it )
	{
		parser->fktext[ ix ].k_liste[ parser->fktext[ ix ].k_anz ] = 
			( *it ).toDouble();
		parser->fktext[ ix ].k_anz++;
	}
	
}
