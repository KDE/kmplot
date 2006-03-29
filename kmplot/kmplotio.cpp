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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*
*/

// Qt includes
#include <qdom.h>
#include <qfile.h>
#include <QList>
#include <QTextStream>

// KDE includes
#include <kdebug.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>

// ANSI-C includes
#include <stdlib.h>

// local includes
#include "kmplotio.h"
#include "MainDlg.h"
#include "settings.h"

class XParser;

KmPlotIO::KmPlotIO( XParser *parser)
		: m_parser(parser)
{}


KmPlotIO::~KmPlotIO()
{}


QDomDocument KmPlotIO::currentState()
{
	// saving as xml by a QDomDocument
	QDomDocument doc( "kmpdoc" );
	// the root tag
	QDomElement root = doc.createElement( "kmpdoc" );
	root.setAttribute( "version", "2" );
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

	QString temp;
	temp.setNum(Settings::xScaling());
	addTag( doc, tag, "tic-x", temp );
	temp.setNum(Settings::yScaling());
	addTag( doc, tag, "tic-y", temp );
	temp.setNum(Settings::xPrinting());
	addTag( doc, tag, "print-tic-x", temp );
	temp.setNum(Settings::yPrinting());
	addTag( doc, tag, "print-tic-y", temp);

	root.appendChild( tag );
	
	foreach ( Ufkt * it, m_parser->m_ufkt )
	{
		if ( !it->fstr.isEmpty() )
			addFunction( doc, root, it );
	}

	tag = doc.createElement( "fonts" );
	addTag( doc, tag, "axes-font", Settings::axesFont() );
	addTag( doc, tag, "header-table-font", Settings::headerTableFont() );
	root.appendChild( tag );
	
	return doc;
}


bool KmPlotIO::save( const KUrl &url )
{
	QDomDocument doc = currentState();

	QFile xmlfile;
	if (!url.isLocalFile() )
	{
		KTempFile tmpfile;
		xmlfile.setFileName( KUrl( tmpfile.name() ).path() );
		if (!xmlfile.open( QIODevice::WriteOnly ) )
		{
			tmpfile.unlink();
			kWarning() << k_funcinfo << "Could not open " << KUrl( tmpfile.name() ).path() << " for writing.\n";
			return false;
		}
		QTextStream ts( &xmlfile );
		doc.save( ts, 4 );
		xmlfile.close();

		if ( !KIO::NetAccess::upload(tmpfile.name(), url,0))
		{
			tmpfile.unlink();
			kWarning() << k_funcinfo << "Could not open " << url.prettyURL() << " for writing ("<<KIO::NetAccess::lastErrorString()<<").\n";
			return false;
		}
		tmpfile.unlink();
	}
	else
	{
		xmlfile.setFileName(url.path()  );
		if (!xmlfile.open( QIODevice::WriteOnly ) )
		{
			kWarning() << k_funcinfo << "Could not open " << url.path() << " for writing.\n";
			return false;
		}
		QTextStream ts( &xmlfile );
		doc.save( ts, 4 );
		xmlfile.close();
		return true;
	}
	return true;
}

// static
void KmPlotIO::addFunction( QDomDocument & doc, QDomElement & root, Ufkt * function )
{
	QDomElement tag = doc.createElement( "function" );

	//tag.setAttribute( "number", ix );
	tag.setAttribute( "visible", function->f_mode );
	tag.setAttribute( "color", QColor( function->color ).name() );
	tag.setAttribute( "width", function->linewidth );
	tag.setAttribute( "use-slider", function->use_slider );
	
	tag.setAttribute( "visible-deriv", function->f1_mode );
	tag.setAttribute( "deriv-color", QColor( function->f1_color ).name() );
	tag.setAttribute( "deriv-width", function->f1_linewidth );
	
	tag.setAttribute( "visible-2nd-deriv", function->f2_mode );
	tag.setAttribute( "deriv2nd-color", QColor( function->f2_color ).name() );
	tag.setAttribute( "deriv2nd-width", function->f2_linewidth );
	
	tag.setAttribute( "visible-integral", function->integral_mode );
	tag.setAttribute( "integral-color", QColor( function->integral_color ).name() );
	tag.setAttribute( "integral-width", function->integral_linewidth );
	tag.setAttribute( "integral-use-precision", function->integral_use_precision );
	tag.setAttribute( "integral-precision", function->integral_precision );
	tag.setAttribute( "integral-startx", function->str_startx );
	tag.setAttribute( "integral-starty", function->str_starty );

	addTag( doc, tag, "equation", function->fstr );

	QStringList str_parameters;
	for ( QList<ParameterValueItem>::Iterator k = function->parameters.begin(); k != function->parameters.end(); ++k )
		str_parameters.append( (*k).expression);
			
	if( !str_parameters.isEmpty() )
		addTag( doc, tag, "parameterlist", str_parameters.join( ";" ) );

	/// \todo save these to file and whether uses custom min/max
	if (function->usecustomxmin)
		addTag( doc, tag, "arg-min", function->str_dmin );
	if (function->usecustomxmax)
		addTag( doc, tag, "arg-max", function->str_dmax );

	root.appendChild( tag );
}



// static
void KmPlotIO::addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue )
{
	QDomElement tag = doc.createElement( tagName );
	QDomText value = doc.createTextNode( tagValue );
	tag.appendChild( value );
	parentTag.appendChild( tag );
}


bool KmPlotIO::restore( const QDomDocument & doc )
{
	kDebug() << k_funcinfo << endl;
	
	// temporary measure: for now, delete all previous functions
	QList<int> prevFunctionIDs = m_parser->m_ufkt.keys();
	foreach ( int id, prevFunctionIDs )
		m_parser->delfkt( id );
	
	QDomElement element = doc.documentElement();
	QString version = element.attribute( "version" );
	if ( version.isNull()) //an old kmplot-file
	{
		MainDlg::oldfileversion = true;
		for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
		{
			if ( n.nodeName() == "axes" )
				oldParseAxes( n.toElement() );
			if ( n.nodeName() == "grid" )
				parseGrid( n.toElement() );
			if ( n.nodeName() == "scale" )
				oldParseScale( n.toElement() );
			if ( n.nodeName() == "function" )
				oldParseFunction( m_parser, n.toElement() );
		}
	}
	else if (version == "1" || version == "2")
	{
		MainDlg::oldfileversion = false;
		for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
		{
			if ( n.nodeName() == "axes" )
				parseAxes( n.toElement() );
			if ( n.nodeName() == "grid" )
				parseGrid( n.toElement() );
			if ( n.nodeName() == "scale" )
				parseScale( n.toElement() );
			if ( n.nodeName() == "function")
				parseFunction( m_parser, n.toElement() );
		}
	}
	else
	{
		KMessageBox::error(0,i18n("The file had an unknown version number"));
		return false;
	}
	
	return true;
}


bool KmPlotIO::load( const KUrl &url )
{
	QDomDocument doc( "kmpdoc" );
	QFile f;
	if ( !url.isLocalFile() )
	{
		if( !KIO::NetAccess::exists( url, true, 0 ) )
		{
			KMessageBox::error(0,i18n("The file does not exist."));
			return false;
		}
		QString tmpfile;
		if( !KIO::NetAccess::download( url, tmpfile, 0 ) )
		{
			KMessageBox::error(0,i18n("An error appeared when opening this file"));
			return false;
		}
		f.setFileName(tmpfile);
	}
	else
		f.setFileName( url.path() );

	if ( !f.open( QIODevice::ReadOnly ) )
	{
		KMessageBox::error(0,i18n("An error appeared when opening this file"));
		return false;
	}
	if ( !doc.setContent( &f ) )
	{
		KMessageBox::error(0,i18n("The file could not be loaded"));
		f.close();
		return false;
	}
	f.close();

	if ( !restore( doc ) )
		return false;

	if ( !url.isLocalFile() )
		KIO::NetAccess::removeTempFile( f.fileName() );
	return true;
}

void KmPlotIO::parseAxes( const QDomElement &n )
{
	Settings::setAxesLineWidth( n.attribute( "width", "0.1" ).toDouble() );
	Settings::setAxesColor( QColor( n.attribute( "color", "#000000" ) ) );
	Settings::setTicWidth( n.attribute( "tic-width", "0.3" ).toDouble() );
	Settings::setTicLength( n.attribute( "tic-length", "1.0" ).toDouble() );

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
	Settings::setGridLineWidth( n.attribute( "width", "0.1" ).toDouble() );

	Settings::setGridStyle( n.namedItem( "mode" ).toElement().text().toInt() );
}

int unit2index( const QString unit )
{
	QString units[ 9 ] = { "10", "5", "2", "1", "0.5", "pi/2", "pi/3", "pi/4",i18n("automatic") };
	int index = 0;
	while( ( index < 9 ) && ( unit!= units[ index ] ) ) index ++;
	if( index == 9 ) index = -1;
	return index;
}


void KmPlotIO::parseScale(const QDomElement & n )
{
	Settings::setXScaling(  n.namedItem( "tic-x" ).toElement().text().toInt()  );
	Settings::setYScaling(  n.namedItem( "tic-y" ).toElement().text().toInt() );
	Settings::setXPrinting(  n.namedItem( "print-tic-x" ).toElement().text().toInt()  );
	Settings::setYPrinting(  n.namedItem( "print-tic-y" ).toElement().text().toInt() );
}

// static
void KmPlotIO::parseFunction( XParser *m_parser, const QDomElement & n, bool allowRename )
{
	kDebug() << k_funcinfo << endl;
	
	Ufkt ufkt;
	m_parser->prepareAddingFunction(&ufkt);

	ufkt.f_mode = n.attribute( "visible" ).toInt();
	ufkt.color = QColor( n.attribute( "color" ) ).rgb();
	ufkt.linewidth = n.attribute( "width" ).toDouble();
	ufkt.use_slider = n.attribute( "use-slider" ).toInt();

	ufkt.f1_mode = n.attribute( "visible-deriv", "0" ).toInt();
	ufkt.f1_color = QColor(n.attribute( "deriv-color" )).rgb();
	ufkt.f1_linewidth = n.attribute( "deriv-width" ).toDouble();
	
	ufkt.f2_mode = n.attribute( "visible-2nd-deriv", "0" ).toInt();
	ufkt.f2_color = QColor(n.attribute( "deriv2nd-color" )).rgb();
	ufkt.f2_linewidth = n.attribute( "deriv2nd-width" ).toDouble();
	
	ufkt.integral_mode = n.attribute( "visible-integral", "0" ).toInt();
	ufkt.integral_color = QColor(n.attribute( "integral-color" )).rgb();
	ufkt.integral_linewidth = n.attribute( "integral-width" ).toDouble();
	ufkt.integral_use_precision = n.attribute( "integral-use-precision" ).toInt();
	ufkt.integral_precision = n.attribute( "integral-precision" ).toInt();
	ufkt.str_startx = n.attribute( "integral-startx" );
	ufkt.startx = m_parser->eval( ufkt.str_startx );
	ufkt.str_starty = n.attribute( "integral-starty" );
	ufkt.starty = m_parser->eval( ufkt.str_starty );

	ufkt.str_dmin = n.namedItem( "arg-min" ).toElement().text();
	if( ufkt.str_dmin.isEmpty() )
	  ufkt.usecustomxmin = false;
	else
	{
	  ufkt.usecustomxmin = true;
	  ufkt.dmin = m_parser->eval( ufkt.str_dmin );
	    
	}
	ufkt.str_dmax = n.namedItem( "arg-max" ).toElement().text();
	if( ufkt.str_dmax.isEmpty() )
	  ufkt.usecustomxmax = false;
	else
	{
	  ufkt.usecustomxmax = true;
	  ufkt.dmax = m_parser->eval( ufkt.str_dmax );
	}
	  
	if (ufkt.usecustomxmin && ufkt.usecustomxmax && ufkt.str_dmin==ufkt.str_dmax)
	{
	  ufkt.usecustomxmin = false;
	  ufkt.usecustomxmax = false;
	}
	
	ufkt.fstr = n.namedItem( "equation" ).toElement().text();
	if ( allowRename && !ufkt.fstr.isEmpty() )
	{
		QChar prefix = ufkt.fstr[0];
		if ( prefix == 'r' )
			m_parser->fixFunctionName( ufkt.fstr, XParser::Polar, -1 );
		else if ( prefix == 'x' )
			m_parser->fixFunctionName( ufkt.fstr, XParser::ParametricX, -1 );
		else if ( prefix == 'y' )
			m_parser->fixFunctionName( ufkt.fstr, XParser::ParametricY, -1 );
		else
			m_parser->fixFunctionName( ufkt.fstr, XParser::Function, -1 );
	}
	
	if (MainDlg::oldfileversion)
		parseThreeDotThreeParameters( m_parser, n, ufkt );
	else
		parseParameters( m_parser, n, ufkt );

	QString fstr = ufkt.fstr;
	if ( !fstr.isEmpty() )
	{
		int const i = fstr.indexOf( ';' );
		QString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		int id = m_parser->addfkt( str );
		Ufkt * added_function = m_parser->m_ufkt[id];
		added_function->copyFrom( ufkt );
	}
}

// static
void KmPlotIO::parseParameters( XParser *m_parser, const QDomElement &n, Ufkt &ufkt  )
{
	QStringList str_parameters;
	for ( QList<ParameterValueItem>::Iterator it = ufkt.parameters.begin(); it != ufkt.parameters.end(); ++it )
		str_parameters.append( (*it).expression);
	str_parameters = n.namedItem( "parameterlist" ).toElement().text().split( ";", QString::SkipEmptyParts );
	for( QStringList::Iterator it = str_parameters.begin(); it != str_parameters.end(); ++it )
		ufkt.parameters.append( ParameterValueItem( *it, m_parser->eval( *it ) ));
}

// static
void KmPlotIO::parseThreeDotThreeParameters( XParser *m_parser, const QDomElement &n, Ufkt &ufkt  )
{
	QStringList str_parameters;
	for ( QList<ParameterValueItem>::Iterator it = ufkt.parameters.begin(); it != ufkt.parameters.end(); ++it )
		str_parameters.append( (*it).expression);
	str_parameters = n.namedItem( "parameterlist" ).toElement().text().split( ",", QString::SkipEmptyParts );
	for( QStringList::Iterator it = str_parameters.begin(); it != str_parameters.end(); ++it )
		ufkt.parameters.append( ParameterValueItem( *it, m_parser->eval( *it ) ));
}

void KmPlotIO::oldParseFunction(  XParser *m_parser, const QDomElement & n )
{
	kDebug() << "parsing old function" << endl;
	Ufkt ufkt;
	m_parser->prepareAddingFunction(&ufkt);
	
	ufkt.f_mode = n.attribute( "visible" ).toInt();
	ufkt.f1_mode = n.attribute( "visible-deriv" ).toInt();
	ufkt.f2_mode = n.attribute( "visible-2nd-deriv" ).toInt();
	ufkt.f2_mode = 0;
	ufkt.linewidth = n.attribute( "width" ).toDouble();
	ufkt.use_slider = -1;
	ufkt.color = ufkt.f1_color = ufkt.f2_color = ufkt.integral_color = QColor( n.attribute( "color" ) ).rgb();

	ufkt.str_dmin = n.namedItem( "arg-min" ).toElement().text();
	if( ufkt.str_dmin.isEmpty() )
	  ufkt.usecustomxmin = false;
	else
	{
	  ufkt.dmin = m_parser->eval( ufkt.str_dmin );
	  ufkt.usecustomxmin = true;
	}
	ufkt.str_dmax = n.namedItem( "arg-max" ).toElement().text();
	if( ufkt.str_dmax.isEmpty() )
	  ufkt.usecustomxmax = false;
	else
	{
	  ufkt.dmax = m_parser->eval( ufkt.str_dmax );
	  ufkt.usecustomxmax = true;
	}
	if (ufkt.usecustomxmin && ufkt.usecustomxmax && ufkt.str_dmin==ufkt.str_dmax)
	{
	  ufkt.usecustomxmin = false;
	  ufkt.usecustomxmax = false;
	}
	
	const QString tmp_fstr = n.namedItem( "equation" ).toElement().text();
	const int pos = tmp_fstr.indexOf(';');
	if ( pos == -1 )
	  ufkt.fstr = tmp_fstr;
	else
	{
	  ufkt.fstr = tmp_fstr.left(pos);
	  if ( !m_parser->getext( &ufkt, tmp_fstr) )
	  {
	    KMessageBox::error(0,i18n("The function %1 could not be loaded").arg(ufkt.fstr));
	    return;
	  }
	}

	QString fstr = ufkt.fstr;
	if ( !fstr.isEmpty() )
	{
		int const i = fstr.indexOf( ';' );
		QString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		int id = m_parser->addfkt( str );
		Ufkt *added_function = m_parser->m_ufkt[id];
		added_function->copyFrom( ufkt );
	}
}

void KmPlotIO::oldParseAxes( const QDomElement &n )
{
	Settings::setAxesLineWidth( n.attribute( "width", "0.1" ).toDouble() );
	Settings::setAxesColor( QColor( n.attribute( "color", "#000000" ) ) );
	Settings::setTicWidth( n.attribute( "tic-width", "0.3" ).toDouble() );
	Settings::setTicLength( n.attribute( "tic-length", "1.0" ).toDouble() );

	Settings::setShowAxes( true );
	Settings::setShowArrows( true );
	Settings::setShowLabel( true );
	Settings::setShowFrame( true );
	Settings::setShowExtraFrame( true );
	Settings::setXRange( n.namedItem( "xcoord" ).toElement().text().toInt() );
	Settings::setXMin( n.namedItem( "xmin" ).toElement().text() );
	Settings::setXMax( n.namedItem( "xmax" ).toElement().text() );
	Settings::setYRange( n.namedItem( "ycoord" ).toElement().text().toInt() );
	Settings::setYMin( n.namedItem( "ymin" ).toElement().text() );
	Settings::setYMax( n.namedItem( "ymax" ).toElement().text() );
}

void KmPlotIO::oldParseScale( const QDomElement & n )
{
	Settings::setXScaling( unit2index( n.namedItem( "tic-x" ).toElement().text() ) );
	Settings::setYScaling( unit2index( n.namedItem( "tic-y" ).toElement().text() ) );
	Settings::setXPrinting( unit2index( n.namedItem( "print-tic-x" ).toElement().text() ) );
	Settings::setYPrinting( unit2index( n.namedItem( "print-tic-y" ).toElement().text() ) );
}

