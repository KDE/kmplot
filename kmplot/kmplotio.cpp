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

#include "kmplotio.h"
#include "misc.h"

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

	tag.setAttribute( "color", QColor( axesColor ).name() );
	tag.setAttribute( "width", axesThickness );
	tag.setAttribute( "tic-width", gradThickness );
	tag.setAttribute( "tic-legth", gradLength );

	addTag( doc, tag, "mode", QString::number( mode ) );
	addTag( doc, tag, "xmin", xminstr );
	addTag( doc, tag, "xmax", xmaxstr );
	addTag( doc, tag, "ymin", yminstr );
	addTag( doc, tag, "ymax", ymaxstr );
	addTag( doc, tag, "xcoord", QString::number( koordx ) );
	addTag( doc, tag, "ycoord", QString::number( koordy ) );

	root.appendChild( tag );

	tag = doc.createElement( "grid" );

	tag.setAttribute( "color", QColor( gridColor ).name() );
	tag.setAttribute( "width", gridThickness );

	addTag( doc, tag, "mode", QString::number( g_mode ) );

	root.appendChild( tag );

	tag = doc.createElement( "scale" );

	addTag( doc, tag, "tic-x", tlgxstr );
	addTag( doc, tag, "tic-y", tlgystr );
	addTag( doc, tag, "print-tic-x", drskalxstr );
	addTag( doc, tag, "print-tic-y", drskalystr );

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
			tag.setAttribute( "color", QColor( ps.fktext[ ix ].farbe ).name() );

			addTag( doc, tag, "equation", ps.fktext[ ix ].extstr );

			root.appendChild( tag );
		}
	}

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

	///////////
	// postprocessing loading
	switch ( koordx )
	{
	case 0:
		xmin = -8.0;
		xmax = 8.0;
		break;
	case 1:
		xmin = -5.0;
		xmax = 5.5;
		break;
	case 2:
		xmin = 0.0;
		xmax = 16.0;
		break;
	case 3:
		xmin = 0.0;
		xmax = 10.0;
		break;
	case 4:
		xmin = ps.eval( xminstr );
		xmax = ps.eval( xmaxstr );
	}
	switch ( koordy )
	{
	case 0:
		ymin = -8.0;
		ymax = 8.0;
		break;
	case 1:
		ymin = -5.0;
		ymax = 5.5;
		break;
	case 2:
		ymin = 0.0;
		ymax = 16.0;
		break;
	case 3:
		ymin = 0.0;
		ymax = 10.0;
		break;
	case 4:
		ymin = ps.eval( yminstr );
		ymax = ps.eval( ymaxstr );
	}
}

void KmPlotIO::parseAxes( const QDomElement &n )
{
	axesThickness = n.attribute( "width", "1" ).toInt();
	axesColor = QColor( n.attribute( "color", "#000000" ) ).rgb();
	gradThickness = n.attribute( "tic-width", "3" ).toInt();
	gradLength = n.attribute( "tic-length", "10" ).toInt();

	mode = n.namedItem( "mode" ).toElement().text().toInt();
	xminstr = n.namedItem( "xmin" ).toElement().text();
	xmaxstr = n.namedItem( "xmax" ).toElement().text();
	yminstr = n.namedItem( "ymin" ).toElement().text();
	ymaxstr = n.namedItem( "ymax" ).toElement().text();
	koordx = n.namedItem( "xcoord" ).toElement().text().toInt();
	koordy = n.namedItem( "ycoord" ).toElement().text().toInt();
}

void KmPlotIO::parseGrid( const QDomElement & n )
{
	gridColor = QColor( n.attribute( "color", "#c0c0c0" ) ).rgb();
	gridThickness = n.attribute( "width", "1" ).toInt();

	g_mode = n.namedItem( "mode" ).toElement().text().toInt();
}

void KmPlotIO::parseScale( const QDomElement & n )
{
	tlgxstr = n.namedItem( "tic-x" ).toElement().text();
	tlgystr = n.namedItem( "tic-y" ).toElement().text();
	drskalxstr = n.namedItem( "print-tic-x" ).toElement().text();
	drskalystr = n.namedItem( "print-tic-y" ).toElement().text();

	tlgx = ps.eval( tlgxstr );
	tlgy = ps.eval( tlgystr );
	drskalx = ps.eval( drskalxstr );
	drskaly = ps.eval( drskalystr );
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
	ps.fktext[ ix ].farbe = QColor( n.attribute( "color" ) ).rgb();

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
	}
}
