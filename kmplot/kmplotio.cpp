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

// KDE includes
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

bool KmPlotIO::save( const KURL &url )
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


	for( QValueVector<Ufkt>::iterator it = m_parser->ufkt.begin(); it != m_parser->ufkt.end(); ++it)
	{
		if ( !it->fstr.isEmpty() )
		{
			tag = doc.createElement( "function" );

			//tag.setAttribute( "number", ix );
			tag.setAttribute( "visible", it->f_mode );
			tag.setAttribute( "color", QColor( it->color ).name() );
			tag.setAttribute( "width", it->linewidth );
			tag.setAttribute( "use-slider", it->use_slider );

			if ( it->f1_mode)
			{
				tag.setAttribute( "visible-deriv", it->f1_mode );
				tag.setAttribute( "deriv-color", QColor( it->f1_color ).name() );
				tag.setAttribute( "deriv-width", it->f1_linewidth );
			}

			if ( it->f2_mode)
			{
				tag.setAttribute( "visible-2nd-deriv", it->f2_mode );
				tag.setAttribute( "deriv2nd-color", QColor( it->f2_color ).name() );
				tag.setAttribute( "deriv2nd-width", it->f2_linewidth );
			}

			if ( it->integral_mode)
			{
				tag.setAttribute( "visible-integral", "1" );
				tag.setAttribute( "integral-color", QColor( it->integral_color ).name() );
				tag.setAttribute( "integral-width", it->integral_linewidth );
				tag.setAttribute( "integral-use-precision", it->integral_use_precision );
				tag.setAttribute( "integral-precision", it->integral_precision );
				tag.setAttribute( "integral-startx", it->str_startx );
				tag.setAttribute( "integral-starty", it->str_starty );
			}

			addTag( doc, tag, "equation", it->fstr );

			QStringList str_parameters;
			for ( QValueList<ParameterValueItem>::Iterator k = it->parameters.begin(); k != it->parameters.end(); ++k )
				str_parameters.append( (*k).expression);
			
			if( !str_parameters.isEmpty() )
				addTag( doc, tag, "parameterlist", str_parameters.join( ";" ) );

			if (it->usecustomxmin)
				addTag( doc, tag, "arg-min", it->str_dmin );
			if (it->usecustomxmax)
				addTag( doc, tag, "arg-max", it->str_dmax );

			root.appendChild( tag );

		}
	}

	tag = doc.createElement( "fonts" );
	addTag( doc, tag, "axes-font", Settings::axesFont() );
	addTag( doc, tag, "header-table-font", Settings::headerTableFont() );
	root.appendChild( tag );

	QFile xmlfile;
	if (!url.isLocalFile() )
	{
		KTempFile tmpfile;
		xmlfile.setName(tmpfile.name() );
		if (!xmlfile.open( IO_WriteOnly ) )
		{
			tmpfile.unlink();
			return false;
		}
		QTextStream ts( &xmlfile );
		doc.save( ts, 4 );
		xmlfile.close();

		if ( !KIO::NetAccess::upload(tmpfile.name(), url,0))
		{
			tmpfile.unlink();
			return false;
		}
		tmpfile.unlink();
	}
	else
	{
		xmlfile.setName(url.prettyURL(0,KURL::StripFileProtocol)  );
		if (!xmlfile.open( IO_WriteOnly ) )
			return false;
		QTextStream ts( &xmlfile );
		doc.save( ts, 4 );
		xmlfile.close();
		return true;
	}
	return true;

}

void KmPlotIO::addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue )
{
	QDomElement tag = doc.createElement( tagName );
	QDomText value = doc.createTextNode( tagValue );
	tag.appendChild( value );
	parentTag.appendChild( tag );
}

bool KmPlotIO::load( const KURL &url )
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
		f.setName(tmpfile);
	}
	else
		f.setName( url.prettyURL(0,KURL::StripFileProtocol) );

	if ( !f.open( IO_ReadOnly ) )
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
		KMessageBox::error(0,i18n("The file had an unknown version number"));

	if ( !url.isLocalFile() )
		KIO::NetAccess::removeTempFile( f.name() );
	return true;
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

void KmPlotIO::parseFunction( XParser *m_parser, const QDomElement & n )
{
	QString temp;
	Ufkt ufkt;
	m_parser->prepareAddingFunction(&ufkt);
	int const next_index=m_parser->getNextIndex()+1;

	ufkt.f_mode = n.attribute( "visible" ).toInt();
	ufkt.color = QColor( n.attribute( "color" ) ).rgb();
	ufkt.linewidth = n.attribute( "width" ).toInt();
	ufkt.use_slider = n.attribute( "use-slider" ).toInt();

	temp = n.attribute( "visible-deriv" );
	if (!temp.isNull())
	{
		ufkt.f1_mode = temp.toInt();
		ufkt.f1_color = QColor(n.attribute( "deriv-color" )).rgb();
		ufkt.f1_linewidth = n.attribute( "deriv-width" ).toInt();
	}
	else
	{
		ufkt.f1_mode = 0;
		ufkt.f1_color = m_parser->defaultColor(next_index);
		ufkt.f1_linewidth = m_parser->linewidth0;
	}

	temp = n.attribute( "visible-2nd-deriv" );
	if (!temp.isNull())
	{
		ufkt.f2_mode = temp.toInt();
		ufkt.f2_color = QColor(n.attribute( "deriv2nd-color" )).rgb();
		ufkt.f2_linewidth = n.attribute( "deriv2nd-width" ).toInt();
	}
	else
	{
		ufkt.f2_mode = 0;
		ufkt.f2_color = m_parser->defaultColor(next_index);
		ufkt.f2_linewidth = m_parser->linewidth0;
	}

	temp = n.attribute( "visible-integral" );
	if (!temp.isNull())
	{
		ufkt.integral_mode = temp.toInt();
		ufkt.integral_color = QColor(n.attribute( "integral-color" )).rgb();
		ufkt.integral_linewidth = n.attribute( "integral-width" ).toInt();
		ufkt.integral_use_precision = n.attribute( "integral-use-precision" ).toInt();
		ufkt.integral_precision = n.attribute( "integral-precision" ).toInt();
		ufkt.str_startx = n.attribute( "integral-startx" );
		ufkt.startx = m_parser->eval( ufkt.str_startx );
		ufkt.str_starty = n.attribute( "integral-starty" );
		ufkt.starty = m_parser->eval( ufkt.str_starty );

	}
	else
	{
		ufkt.integral_mode = 0;
		ufkt.integral_color = m_parser->defaultColor(next_index);
		ufkt.integral_linewidth = m_parser->linewidth0;
		ufkt.integral_use_precision = 0;
		ufkt.integral_precision = ufkt.linewidth;
	}

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
	if (MainDlg::oldfileversion)
		parseThreeDotThreeParameters( m_parser, n, ufkt );
	else
		parseParameters( m_parser, n, ufkt );

	QString fstr = ufkt.fstr;
	if ( !fstr.isEmpty() )
	{
		int const i = fstr.find( ';' );
		QString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		m_parser->addfkt( str );
		Ufkt *added_function = &m_parser->ufkt.last();
		added_function->f_mode = ufkt.f_mode;
		added_function->f1_mode = ufkt.f1_mode;
		added_function->f2_mode = ufkt.f2_mode;
		added_function->integral_mode = ufkt.integral_mode;
		added_function->integral_use_precision = ufkt.integral_use_precision;
		added_function->linewidth = ufkt.linewidth;
		added_function->f1_linewidth = ufkt.f1_linewidth;
		added_function->f2_linewidth = ufkt.f2_linewidth;
		added_function->integral_linewidth = ufkt.integral_linewidth;
		added_function->str_dmin = ufkt.str_dmin;
		added_function->str_dmax = ufkt.str_dmax;
		added_function->dmin = ufkt.dmin;
		added_function->dmax = ufkt.dmax;
		added_function->str_startx = ufkt.str_startx;
		added_function->str_starty = ufkt.str_starty;
		added_function->oldx = ufkt.oldx;
		added_function->starty = ufkt.starty;
		added_function->startx = ufkt.startx;
		added_function->integral_precision = ufkt.integral_precision;
		added_function->color = ufkt.color;
		added_function->f1_color = ufkt.f1_color;
		added_function->f2_color = ufkt.f2_color;
		added_function->integral_color = ufkt.integral_color;
		added_function->parameters = ufkt.parameters;
		added_function->use_slider = ufkt.use_slider;
		added_function->usecustomxmin = ufkt.usecustomxmin;
		added_function->usecustomxmax = ufkt.usecustomxmax;
	}
}

void KmPlotIO::parseParameters( XParser *m_parser, const QDomElement &n, Ufkt &ufkt  )
{
	QStringList str_parameters;
	for ( QValueList<ParameterValueItem>::Iterator it = ufkt.parameters.begin(); it != ufkt.parameters.end(); ++it )
		str_parameters.append( (*it).expression);
	str_parameters = QStringList::split( ";", n.namedItem( "parameterlist" ).toElement().text() );
	for( QStringList::Iterator it = str_parameters.begin(); it != str_parameters.end(); ++it )
		ufkt.parameters.append( ParameterValueItem( *it, m_parser->eval( *it ) ));
}

void KmPlotIO::parseThreeDotThreeParameters( XParser *m_parser, const QDomElement &n, Ufkt &ufkt  )
{
	QStringList str_parameters;
	for ( QValueList<ParameterValueItem>::Iterator it = ufkt.parameters.begin(); it != ufkt.parameters.end(); ++it )
		str_parameters.append( (*it).expression);
	str_parameters = QStringList::split( ",", n.namedItem( "parameterlist" ).toElement().text() );
	for( QStringList::Iterator it = str_parameters.begin(); it != str_parameters.end(); ++it )
		ufkt.parameters.append( ParameterValueItem( *it, m_parser->eval( *it ) ));
}

void KmPlotIO::oldParseFunction(  XParser *m_parser, const QDomElement & n )
{
	kdDebug() << "parsing old function" << endl;
	Ufkt ufkt;
	m_parser->prepareAddingFunction(&ufkt);
	
	ufkt.f_mode = n.attribute( "visible" ).toInt();
	ufkt.f1_mode = n.attribute( "visible-deriv" ).toInt();
	ufkt.f2_mode = n.attribute( "visible-2nd-deriv" ).toInt();
	ufkt.f2_mode = 0;
	ufkt.linewidth = n.attribute( "width" ).toInt();
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
	const int pos = tmp_fstr.find(';');
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
		int const i = fstr.find( ';' );
		QString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		m_parser->addfkt( str );
		Ufkt *added_function = &m_parser->ufkt.last();
		added_function->f_mode = ufkt.f_mode;
		added_function->f1_mode = ufkt.f1_mode;
		added_function->f2_mode = ufkt.f2_mode;
		added_function->integral_mode = ufkt.integral_mode;
		added_function->integral_use_precision = ufkt.integral_use_precision;
		added_function->linewidth = ufkt.linewidth;
		added_function->f1_linewidth = ufkt.f1_linewidth;
		added_function->f2_linewidth = ufkt.f2_linewidth;
		added_function->integral_linewidth = ufkt.integral_linewidth;
		added_function->str_dmin = ufkt.str_dmin;
		added_function->str_dmax = ufkt.str_dmax;
		added_function->dmin = ufkt.dmin;
		added_function->dmax = ufkt.dmax;
		added_function->str_startx = ufkt.str_startx;
		added_function->str_starty = ufkt.str_starty;
		added_function->oldx = ufkt.oldx;
		added_function->starty = ufkt.starty;
		added_function->startx = ufkt.startx;
		added_function->integral_precision = ufkt.integral_precision;
		added_function->color = ufkt.color;
		added_function->f1_color = ufkt.f1_color;
		added_function->f2_color = ufkt.f2_color;
		added_function->integral_color = ufkt.integral_color;
		added_function->parameters = ufkt.parameters;
		added_function->use_slider = ufkt.use_slider;
		added_function->usecustomxmin = ufkt.usecustomxmin;
		added_function->usecustomxmax = ufkt.usecustomxmax;
	}
}

void KmPlotIO::oldParseAxes( const QDomElement &n )
{
	Settings::setAxesLineWidth( n.attribute( "width", "1" ).toInt() );
	Settings::setAxesColor( QColor( n.attribute( "color", "#000000" ) ) );
	Settings::setTicWidth( n.attribute( "tic-width", "3" ).toInt() );
	Settings::setTicLength( n.attribute( "tic-length", "10" ).toInt() );

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

