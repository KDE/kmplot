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

KmPlotIO::KmPlotIO()
{
}


KmPlotIO::~KmPlotIO()
{
}

bool KmPlotIO::save(  XParser *parser, const KURL &url )
{
	// saving as xml by a QDomDocument
	QDomDocument doc( "kmpdoc" );
	// the root tag
	QDomElement root = doc.createElement( "kmpdoc" );
	root.setAttribute( "version", "1" );
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
        
        
        for( QValueVector<XParser::FktExt>::iterator it = parser->fktext.begin(); it != parser->fktext.end(); ++it)
	{
		if ( !it->extstr.isEmpty() )
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
			
			addTag( doc, tag, "equation", it->extstr );
			
			if( !it->str_parameter.isEmpty() )
				addTag( doc, tag, "parameterlist", it->str_parameter.join( "," ) );
			
			addTag( doc, tag, "arg-min", it->str_dmin );
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

bool KmPlotIO::load( XParser *parser, const KURL &url )
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
	if ( version == QString::null) //an old kmplot-file
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
				oldParseFunction( parser, n.toElement() );
		}
	}
	else if (version == "1")
	{
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


void KmPlotIO::parseScale( const QDomElement & n )
{
	Settings::setXScaling( atoi( n.namedItem( "tic-x" ).toElement().text().latin1() ) );
	Settings::setYScaling( atoi( n.namedItem( "tic-y" ).toElement().text().latin1() ) );
	Settings::setXPrinting( atoi( n.namedItem( "print-tic-x" ).toElement().text().latin1() ) );
	Settings::setYPrinting( atoi( n.namedItem( "print-tic-y" ).toElement().text().latin1() ) );
}


void KmPlotIO::parseFunction(  XParser *parser, const QDomElement & n )
{
	QString temp;
        XParser::FktExt fktext;
        int const next_index=parser->getNextIndex()+1;
        
	fktext.f_mode = n.attribute( "visible" ).toInt();
	fktext.color = QColor( n.attribute( "color" ) ).rgb();
	fktext.linewidth = n.attribute( "width" ).toInt();
	fktext.use_slider = n.attribute( "use-slider" ).toInt();
	
	temp = n.attribute( "visible-deriv" );
	if (temp != QString::null)
	{
		fktext.f1_mode = temp.toInt();
		fktext.f1_color = QColor(n.attribute( "deriv-color" )).rgb();
		fktext.f1_linewidth = n.attribute( "deriv-width" ).toInt();
	}
	else
	{
		fktext.f1_mode = 0;
		fktext.f1_color = parser->defaultColor(next_index);
		fktext.f1_linewidth = parser->linewidth0;
	}
		
	temp = n.attribute( "visible-2nd-deriv" );
	if (temp != QString::null)
	{
		fktext.f2_mode = temp.toInt();
		fktext.f2_color = QColor(n.attribute( "deriv2nd-color" )).rgb();
		fktext.f2_linewidth = n.attribute( "deriv2nd-width" ).toInt();
	}
	else
	{
		fktext.f2_mode = 0;
		fktext.f2_color = parser->defaultColor(next_index);
		fktext.f2_linewidth = parser->linewidth0;
	}
	
	temp = n.attribute( "visible-integral" );
	if (temp != QString::null)
	{
		fktext.integral_mode = temp.toInt();
		fktext.integral_color = QColor(n.attribute( "integral-color" )).rgb();
		fktext.integral_linewidth = n.attribute( "integral-width" ).toInt();
		fktext.integral_use_precision = n.attribute( "integral-use-precision" ).toInt();
		fktext.integral_precision = n.attribute( "integral-precision" ).toInt();
		fktext.str_startx = n.attribute( "integral-startx" );
		fktext.startx = parser->eval( fktext.str_startx );
		fktext.str_starty = n.attribute( "integral-starty" );
		fktext.starty = parser->eval( fktext.str_starty );
		
	}
	else
	{
		fktext.integral_mode = 0;
		fktext.integral_color = parser->defaultColor(next_index);
		fktext.integral_linewidth = parser->linewidth0;
		fktext.integral_use_precision = 0;
		fktext.integral_precision = fktext.linewidth;
	}
	
        
        fktext.str_dmin = n.namedItem( "arg-min" ).toElement().text();
        if( fktext.str_dmin.isEmpty() )
        {
                fktext.str_dmin = "0.0";
                fktext.dmin = 0;
                }
        else fktext.dmin = parser->eval( fktext.str_dmin );
        fktext.str_dmax = n.namedItem( "arg-max" ).toElement().text();
        if( fktext.str_dmax.isEmpty() )
        {
                fktext.str_dmax = "0.0";
                fktext.dmax = 0;
        }
        else fktext.dmax = parser->eval( fktext.str_dmax );
        
	fktext.extstr = n.namedItem( "equation" ).toElement().text();
        parseParameters( parser, n, fktext );
        parser->fktext.append(fktext );
        
	QCString fstr = fktext.extstr.utf8();
	if ( !fstr.isEmpty() )
	{
		int const i = fstr.find( ';' );
		QString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		int const id = parser->addfkt( str );
                parser->fktext.last().id = id;
	}  
}

void KmPlotIO::parseParameters( XParser *parser, const QDomElement &n, XParser::FktExt &fktext  )
{
	fktext.str_parameter = QStringList::split( ",", n.namedItem( "parameterlist" ).toElement().text() );
	
	for( QStringList::Iterator it = fktext.str_parameter.begin(); it != fktext.str_parameter.end(); ++it )
	{
		fktext.k_liste.append( parser->eval( *it ) );
	}
	
}
void KmPlotIO::oldParseFunction(  XParser *parser, const QDomElement & n )
{
	kdDebug() << "parsing old function" << endl;

        XParser::FktExt fktext;
	//int ix = n.attribute( "number" ).toInt();
	fktext.f_mode = n.attribute( "visible" ).toInt();
	fktext.f1_mode = n.attribute( "visible-deriv" ).toInt();
	fktext.f2_mode = n.attribute( "visible-2nd-deriv" ).toInt();
        fktext.f2_mode = 0;
	fktext.linewidth = n.attribute( "width" ).toInt();
        fktext.use_slider = -1;
	fktext.color = fktext.f1_color = fktext.f2_color = fktext.integral_color = QColor( n.attribute( "color" ) ).rgb();

        fktext.str_dmin = n.namedItem( "arg-min" ).toElement().text();
        if( fktext.str_dmin.isEmpty() )
        {
                fktext.str_dmin = "0.0";
                fktext.dmin = 0;
        }
        else fktext.dmin = parser->eval( fktext.str_dmin );
        fktext.str_dmax = n.namedItem( "arg-max" ).toElement().text();
        if( fktext.str_dmax.isEmpty() )
        {
                fktext.str_dmax = "0.0";
                fktext.dmax = 0;
        }
        else fktext.dmax = parser->eval( fktext.str_dmax );
        
	fktext.extstr = n.namedItem( "equation" ).toElement().text();
        parser->fktext.append(fktext );
        
	QCString fstr = fktext.extstr.utf8();
	if ( !fstr.isEmpty() )
	{
		int i = fstr.find( ';' );
		QCString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		int const id = parser->addfkt( str );
                parser->getext(  parser->fktext.end() );
                parser->fktext.end()->id = id;
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

