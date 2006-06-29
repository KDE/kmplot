/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter Möller <kd.moeller@t-online.de>
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
#include "maindlg.h"
#include "settings.h"

static QString CurrentVersionString( "5" );

class XParser;

KmPlotIO::KmPlotIO()
{
	KmPlotIO::version = CurrentVersionString.toInt();
	lengthScaler = 1.0;
}


KmPlotIO::~KmPlotIO()
{
}


QDomDocument KmPlotIO::currentState()
{
	// saving as xml by a QDomDocument
	QDomDocument doc( "kmpdoc" );
	// the root tag
	QDomElement root = doc.createElement( "kmpdoc" );
	root.setAttribute( "version", CurrentVersionString );
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
	
	foreach ( Function * it, XParser::self()->m_ufkt )
		addFunction( doc, root, it );

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
			kWarning() << k_funcinfo << "Could not open " << url.prettyUrl() << " for writing ("<<KIO::NetAccess::lastErrorString()<<").\n";
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


void KmPlotIO::addFunction( QDomDocument & doc, QDomElement & root, Function * function )
{
	QDomElement tag = doc.createElement( "function" );
	
	QString names[] = { "f0", "f1", "f2", "integral" };
	PlotAppearance * plots[] = { & function->plotAppearance( Function::Derivative0 ),
		& function->plotAppearance( Function::Derivative1 ),
		& function->plotAppearance( Function::Derivative2 ),
		& function->plotAppearance( Function::Integral ) };
		
	for ( int i = 0; i < 4; ++i )
	{
		tag.setAttribute( QString("%1-width").arg( names[i] ), plots[i]->lineWidth );
		tag.setAttribute( QString("%1-color").arg( names[i] ), QColor( plots[i]->color ).name() );
		tag.setAttribute( QString("%1-use-gradient").arg( names[i] ), plots[i]->useGradient );
		tag.setAttribute( QString("%1-color1").arg( names[i] ), QColor( plots[i]->color1 ).name() );
		tag.setAttribute( QString("%1-color2").arg( names[i] ), QColor( plots[i]->color2 ).name() );
		tag.setAttribute( QString("%1-visible").arg( names[i] ), plots[i]->visible );
		tag.setAttribute( QString("%1-style").arg( names[i] ), PlotAppearance::penStyleToString( plots[i]->style ) );
		tag.setAttribute( QString("%1-show-extrema").arg( names[i] ), plots[i]->showExtrema );
	}
	

	//BEGIN parameters
	tag.setAttribute( "use-parameter-slider", function->m_parameters.useSlider );
	tag.setAttribute( "parameter-slider", function->m_parameters.sliderID );
	
	tag.setAttribute( "use-parameter-list", function->m_parameters.useList );
	QStringList str_parameters;
	foreach ( Value k, function->m_parameters.list )
		str_parameters << k.expression();
			
	if( !str_parameters.isEmpty() )
		addTag( doc, tag, "parameter-list", str_parameters.join( ";" ) );
	//END parameters
	
	
	tag.setAttribute( "integral-use-precision", function->integral_use_precision );
	tag.setAttribute( "integral-precision", function->integral_precision );
	tag.setAttribute( "integral-startx", function->eq[0]->integralInitialX().expression() );
	tag.setAttribute( "integral-starty", function->eq[0]->integralInitialY().expression() );
	
	tag.setAttribute( "type", Function::typeToString( function->type() ) );
	for ( unsigned i=0; i<2; ++i )
	{
		if ( !function->eq[i] )
			continue;
		QString fstr = function->eq[i]->fstr();
		if ( fstr.isEmpty() )
			continue;
		addTag( doc, tag, QString("equation-%1").arg(i), fstr );
	}


	addTag( doc, tag, "arg-min", function->dmin.expression() ).setAttribute( "use", function->usecustomxmin );
	addTag( doc, tag, "arg-max", function->dmax.expression() ).setAttribute( "use", function->usecustomxmax );

	root.appendChild( tag );
}


QDomElement KmPlotIO::addTag( QDomDocument &doc, QDomElement &parentTag, const QString tagName, const QString tagValue )
{
	QDomElement tag = doc.createElement( tagName );
	QDomText value = doc.createTextNode( tagValue );
	tag.appendChild( value );
	parentTag.appendChild( tag );
	return tag;
}


bool KmPlotIO::restore( const QDomDocument & doc )
{
	kDebug() << k_funcinfo << endl;
	
	// temporary measure: for now, delete all previous functions
	QList<int> prevFunctionIDs = XParser::self()->m_ufkt.keys();
	foreach ( int id, prevFunctionIDs )
		XParser::self()->removeFunction( id );
	
	QDomElement element = doc.documentElement();
	QString versionString = element.attribute( "version" );
	if ( versionString.isNull()) //an old kmplot-file
	{
		MainDlg::oldfileversion = true;
		for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
		{
			version = 0;
			lengthScaler = 0.1;
			
			if ( n.nodeName() == "axes" )
				parseAxes( n.toElement() );
			if ( n.nodeName() == "grid" )
				parseGrid( n.toElement() );
			if ( n.nodeName() == "scale" )
				parseScale( n.toElement() );
			if ( n.nodeName() == "function" )
				oldParseFunction( n.toElement() );
		}
	}
	else if ( versionString == "1" ||
				 versionString == "2" ||
				 versionString == "3" ||
				 versionString == "4" ||
				 versionString == "5" )
	{
		MainDlg::oldfileversion = false;
		version = versionString.toInt();
		lengthScaler = (version < 3) ? 0.1 : 1.0;
		
		for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
		{
			if ( n.nodeName() == "axes" )
				parseAxes( n.toElement() );
			if ( n.nodeName() == "grid" )
				parseGrid( n.toElement() );
			if ( n.nodeName() == "scale" )
				parseScale( n.toElement() );
			if ( n.nodeName() == "function")
				parseFunction( n.toElement() );
		}
	}
	else
	{
		KMessageBox::sorry(0,i18n("The file had an unknown version number"));
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
			KMessageBox::sorry(0,i18n("The file does not exist."));
			return false;
		}
		QString tmpfile;
		if( !KIO::NetAccess::download( url, tmpfile, 0 ) )
		{
			KMessageBox::sorry(0,i18n("An error appeared when opening this file (%1)", KIO::NetAccess::lastErrorString() ));
			return false;
		}
		f.setFileName(tmpfile);
	}
	else
		f.setFileName( url.path() );

	if ( !f.open( QIODevice::ReadOnly ) )
	{
		KMessageBox::sorry(0,i18n("%1 could not be opened", f.fileName() ) );
		return false;
	}
	QString errorMessage;
	int errorLine, errorColumn;
	if ( !doc.setContent( &f, & errorMessage, & errorLine, & errorColumn ) )
	{
		KMessageBox::sorry(0,i18n("%1 could not be loaded (%2 at line %3, column %4)", f.fileName(), errorMessage, errorLine, errorColumn ) );
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
	Settings::setAxesLineWidth( n.attribute( "width", (version<3) ? "1" : "0.1" ).toDouble() * lengthScaler );
	Settings::setAxesColor( QColor( n.attribute( "color", "#000000" ) ) );
	Settings::setTicWidth( n.attribute( "tic-width", (version<3) ? "3" : "0.3" ).toDouble() * lengthScaler );
	Settings::setTicLength( n.attribute( "tic-length", (version<3) ? "10" : "1.0" ).toDouble() * lengthScaler );
	
	if ( version < 1 )
	{
		Settings::setShowAxes( true );
		Settings::setShowArrows( true );
		Settings::setShowLabel( true );
		Settings::setShowFrame( true );
		Settings::setShowExtraFrame( true );
	}
	else
	{
		Settings::setShowAxes( n.namedItem( "show-axes" ).toElement().text().toInt() == 1 );
		Settings::setShowArrows( n.namedItem( "show-arrows" ).toElement().text().toInt() == 1 );
		Settings::setShowLabel( n.namedItem( "show-label" ).toElement().text().toInt() == 1 );
		Settings::setShowFrame( n.namedItem( "show-frame" ).toElement().text().toInt() == 1 );
		Settings::setShowExtraFrame( n.namedItem( "show-extra-frame" ).toElement().text().toInt() == 1 );
	}
	
	Settings::setXRange( n.namedItem( "xcoord" ).toElement().text().toInt() );
	Settings::setXMin( n.namedItem( "xmin" ).toElement().text() );
	Settings::setXMax( n.namedItem( "xmax" ).toElement().text() );
	Settings::setYRange( n.namedItem( "ycoord" ).toElement().text().toInt() );
	Settings::setYMin( n.namedItem( "ymin" ).toElement().text() );
	Settings::setYMax( n.namedItem( "ymax" ).toElement().text() );
	
	View::self()->getSettings();
}


void KmPlotIO::parseGrid( const QDomElement & n )
{
	Settings::setGridColor( QColor( n.attribute( "color", "#c0c0c0" ) ) );
	Settings::setGridLineWidth( n.attribute( "width", (version<3) ? "1" : "0.1" ).toDouble() * lengthScaler );

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
	if ( version < 1 )
	{
		Settings::setXScaling( unit2index( n.namedItem( "tic-x" ).toElement().text() ) );
		Settings::setYScaling( unit2index( n.namedItem( "tic-y" ).toElement().text() ) );
		Settings::setXPrinting( unit2index( n.namedItem( "print-tic-x" ).toElement().text() ) );
		Settings::setYPrinting( unit2index( n.namedItem( "print-tic-y" ).toElement().text() ) );
	}
	else
	{
		Settings::setXScaling(  n.namedItem( "tic-x" ).toElement().text().toInt()  );
		Settings::setYScaling(  n.namedItem( "tic-y" ).toElement().text().toInt() );
		Settings::setXPrinting(  n.namedItem( "print-tic-x" ).toElement().text().toInt()  );
		Settings::setYPrinting(  n.namedItem( "print-tic-y" ).toElement().text().toInt() );
	}
	
	View::self()->getSettings();
}


void KmPlotIO::parseFunction( const QDomElement & n, bool allowRename )
{
	kDebug() << k_funcinfo << "version="<<version<<endl;
	
	Function::Type type;
	QString eq0, eq1;
	
	if ( version < 3 )
	{
		eq0 = n.namedItem( "equation" ).toElement().text();
	
		if ( eq0.isEmpty() )
		{
			kWarning() << k_funcinfo << "eq0 is empty!\n";
			return;
		}
		
		switch ( eq0[0].unicode() )
		{
			case 'r':
				type = Function::Polar;
				break;
			
			case 'x':
				parametricXEquation = eq0;
				return;
			
			case 'y':
				type = Function::Parametric;
				eq1 = eq0;
				eq0 = parametricXEquation;
				kDebug() << k_funcinfo << "Parametric: eq0=\""<<eq0<<"\" eq1=\""<<eq1<<"\"\n";
				break;
			
			default:
				type = Function::Cartesian;
				break;
		}
	}
	else
	{
		eq0 = n.namedItem( "equation-0" ).toElement().text();
		eq1 = n.namedItem( "equation-1" ).toElement().text();
		
		kDebug() << "eq0: "<<eq0<<endl;
		kDebug() << "eq1: "<<eq1<<endl;
	
		if ( eq0.isEmpty() )
		{
			kWarning() << k_funcinfo << "eq0 is empty!\n";
			return;
		}
		
		type = Function::stringToType( n.attribute( "type" ) );
	}
	
	if ( allowRename )
	{
		switch ( type )
		{
			case Function::Polar:
				XParser::self()->fixFunctionName( eq0, Equation::Polar, -1 );
				break;
				
			case Function::Parametric:
				XParser::self()->fixFunctionName( eq0, Equation::ParametricX, -1 );
				if ( !eq1.isEmpty() )
					XParser::self()->fixFunctionName( eq1, Equation::ParametricY, -1 );
				break;
				
			case Function::Cartesian:
				XParser::self()->fixFunctionName( eq0, Equation::Cartesian, -1 );
				break;
				
			case Function::Implicit:
				XParser::self()->fixFunctionName( eq0, Equation::Implicit, -1 );
				break;
		}
	}
	
	Function ufkt( type );
	ufkt.eq[0]->setFstr( eq0 );
	if ( !eq1.isEmpty() )
		ufkt.eq[1]->setFstr( eq1 );
	
	PlotAppearance * plots[] = { & ufkt.plotAppearance( Function::Derivative0 ),
		& ufkt.plotAppearance( Function::Derivative1 ),
		& ufkt.plotAppearance( Function::Derivative2 ),
		& ufkt.plotAppearance( Function::Integral ) };
	
	if ( version < 4 )
	{
		plots[ 0 ]->visible = n.attribute( "visible" ).toInt();
		plots[ 0 ]->color = QColor( n.attribute( "color" ) );
		plots[ 0 ]->lineWidth = n.attribute( "width" ).toDouble() * lengthScaler;

		plots[ 1 ]->visible = n.attribute( "visible-deriv", "0" ).toInt();
		plots[ 1 ]->color = QColor(n.attribute( "deriv-color" ));
		plots[ 1 ]->lineWidth = n.attribute( "deriv-width" ).toDouble() * lengthScaler;
	
		plots[ 2 ]->visible = n.attribute( "visible-2nd-deriv", "0" ).toInt();
		plots[ 2 ]->color = QColor(n.attribute( "deriv2nd-color" ));
		plots[ 2 ]->lineWidth = n.attribute( "deriv2nd-width" ).toDouble() * lengthScaler;
	
		plots[ 3 ]->visible = n.attribute( "visible-integral", "0" ).toInt();
		plots[ 3 ]->color = QColor(n.attribute( "integral-color" ));
		plots[ 3 ]->lineWidth = n.attribute( "integral-width" ).toDouble() * lengthScaler;
	}
	else
	{
		QString names[] = { "f0", "f1", "f2", "integral" };
		
		for ( int i = 0; i < 4; ++i )
		{
			plots[i]->lineWidth = n.attribute( QString("%1-width").arg( names[i] ) ).toDouble() * lengthScaler;
			plots[i]->color = n.attribute( QString("%1-color").arg( names[i] ) );
			plots[i]->useGradient = n.attribute( QString("%1-use-gradient").arg( names[i] ) ).toInt();
			plots[i]->color1 = n.attribute( QString("%1-color1").arg( names[i] ) );
			plots[i]->color2 = n.attribute( QString("%1-color2").arg( names[i] ) );
			plots[i]->visible = n.attribute( QString("%1-visible").arg( names[i] ) ).toInt();
			plots[i]->style = PlotAppearance::stringToPenStyle( n.attribute( QString("%1-style").arg( names[i] ) ) );
			plots[i]->showExtrema = n.attribute( QString("%1-show-extrema").arg( names[i] ) ).toInt();
		}
	}
	
	
	//BEGIN parameters
	parseParameters( n, ufkt );
	
	if ( version < 5 )
	{
		int use_slider = n.attribute( "use-slider" ).toInt();
		ufkt.m_parameters.useSlider = (use_slider >= 0);
		ufkt.m_parameters.sliderID = use_slider;
		
		ufkt.m_parameters.useList = !ufkt.m_parameters.list.isEmpty();
	}
	else
	{
		ufkt.m_parameters.useSlider = n.attribute( "use-parameter-slider" ).toInt();
		ufkt.m_parameters.sliderID = n.attribute( "parameter-slider" ).toInt();
		
		ufkt.m_parameters.useList = n.attribute( "use-parameter-list" ).toInt();
	}
	//END parameters
	
	
	ufkt.integral_use_precision = n.attribute( "integral-use-precision" ).toInt();
	ufkt.integral_precision = n.attribute( "integral-precision" ).toInt();
	ufkt.eq[0]->setIntegralStart( n.attribute( "integral-startx" ), n.attribute( "integral-starty" ) );

	QDomElement minElement = n.namedItem( "arg-min" ).toElement();
	QString expression = minElement.text();
	if ( expression.isEmpty() )
		ufkt.usecustomxmin = false;
	else
	{
		ufkt.dmin.updateExpression( expression );
		ufkt.usecustomxmin = minElement.attribute( "use", "1" ).toInt();
	}

	QDomElement maxElement = n.namedItem( "arg-max" ).toElement();
	expression = maxElement.text();
	if ( expression.isEmpty() )
		ufkt.usecustomxmax = false;
	else
	{
		ufkt.dmax.updateExpression( expression );
		ufkt.usecustomxmax = maxElement.attribute( "use", "1" ).toInt();
	}
	

	QString fstr = ufkt.eq[0]->fstr();
	if ( !fstr.isEmpty() )
	{
		int const i = fstr.indexOf( ';' );
		QString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		
		int id = XParser::self()->Parser::addFunction( str, eq1, type );
		
		Function * added_function = XParser::self()->m_ufkt[id];
		added_function->copyFrom( ufkt );
	}
}


void KmPlotIO::parseParameters( const QDomElement &n, Function &ufkt  )
{
	QChar separator = (version < 1) ? ',' : ';';
	QString tagName = (version < 5) ? "parameterlist" : "parameter-list";
	
	QStringList str_parameters = n.namedItem( tagName ).toElement().text().split( separator, QString::SkipEmptyParts );
	for( QStringList::Iterator it = str_parameters.begin(); it != str_parameters.end(); ++it )
		ufkt.m_parameters.list.append( Value( *it ));
}

void KmPlotIO::oldParseFunction( const QDomElement & n )
{
	kDebug() << "parsing old function" << endl;
	
	QString tmp_fstr = n.namedItem( "equation" ).toElement().text();
	if ( tmp_fstr.isEmpty() )
	{
		kWarning() << k_funcinfo << "tmp_fstr is empty!\n";
		return;
	}
	
	Function::Type type;
	switch ( tmp_fstr[0].unicode() )
	{
		case 'r':
			type = Function::Polar;
			break;
			
		case 'x':
// 			type = Function::ParametricX;
			parametricXEquation = tmp_fstr;
			return;
			
		case 'y':
			type = Function::Parametric;
			break;
			
		default:
			type = Function::Cartesian;
			break;
	}
	
	Function ufkt( type );
	
	ufkt.plotAppearance( Function::Derivative0 ).visible = n.attribute( "visible" ).toInt();
	ufkt.plotAppearance( Function::Derivative1 ).visible = n.attribute( "visible-deriv" ).toInt();
	ufkt.plotAppearance( Function::Derivative2 ).visible = n.attribute( "visible-2nd-deriv" ).toInt();
	ufkt.plotAppearance( Function::Derivative0 ).lineWidth = n.attribute( "width" ).toDouble() * lengthScaler;
	ufkt.plotAppearance( Function::Derivative0 ).color =
			ufkt.plotAppearance( Function::Derivative1 ).color =
			ufkt.plotAppearance( Function::Derivative2 ).color =
			ufkt.plotAppearance( Function::Integral ).color = QColor( n.attribute( "color" ) );

	QString expression = n.namedItem( "arg-min" ).toElement().text();
	ufkt.dmin.updateExpression( expression );
	ufkt.usecustomxmin = !expression.isEmpty();

	expression = n.namedItem( "arg-max" ).toElement().text();
	ufkt.dmax.updateExpression( expression );
	ufkt.usecustomxmax = !expression.isEmpty();
	
	if (ufkt.usecustomxmin && ufkt.usecustomxmax && ufkt.dmin.expression()==ufkt.dmax.expression())
	{
	  ufkt.usecustomxmin = false;
	  ufkt.usecustomxmax = false;
	}
	
	const int pos = tmp_fstr.indexOf(';');
	if ( pos == -1 )
		ufkt.eq[0]->setFstr( tmp_fstr );
	else
	{
		ufkt.eq[0]->setFstr( tmp_fstr.left(pos) );
		if ( !XParser::self()->getext( &ufkt, tmp_fstr) )
		{
			KMessageBox::sorry(0,i18n("The function %1 could not be loaded", ufkt.eq[0]->fstr()));
			return;
		}
	}

	QString fstr = ufkt.eq[0]->fstr();
	if ( !fstr.isEmpty() )
	{
		int const i = fstr.indexOf( ';' );
		QString str;
		if ( i == -1 )
			str = fstr;
		else
			str = fstr.left( i );
		
		int id;
		if ( type == Function::Parametric )
			id = XParser::self()->Parser::addFunction( str, parametricXEquation, type );
		else
			id = XParser::self()->Parser::addFunction( str, 0, type );
		
		Function *added_function = XParser::self()->m_ufkt[id];
		added_function->copyFrom( ufkt );
	}
}
