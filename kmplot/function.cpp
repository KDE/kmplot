/*
* KmPlot - a math. function plotter for the KDE-Desktop
*
* Copyright (C) 1998, 1999, 2000, 2002  Klaus-Dieter M�ler <kd.moeller@t-online.de>
*               2006 David Saxton <david@bluehaze.org>
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "function.h"
#include "ksliderwindow.h"
#include "settings.h"
#include "view.h"
#include "xparser.h"

#include <kdebug.h>
#include <QImage>
#include <QLinearGradient>
#include <QMap>
#include <QPainter>

#include <assert.h>
#include <cmath>

int MAX_PM = 4;


//BEGIN class Value
Value::Value( const QString & expression )
{
	m_value = 0.0;
	if ( expression.isEmpty() )
		m_expression = "0";
	else
		updateExpression( expression );
}


Value::Value( double value )
{
	updateExpression( value );
}


bool Value::updateExpression( const QString & expression )
{
	double newValue = XParser::self()->eval( expression );
	if ( XParser::self()->parserError( false ) )
		return false;
	
	m_value = newValue;
	m_expression = expression;
	return true;
}


void Value::updateExpression( double value )
{
	m_value = value;
	m_expression = Parser::number( value );
}


bool Value::operator == ( const Value & other ) const
{
	return m_expression == other.expression();
}
//END class Value



//BEGIN class PlotAppearance
PlotAppearance::PlotAppearance( )
{
	lineWidth = 0.2;
	color = Qt::black;
	useGradient = false;
	visible = false;
	style = Qt::SolidLine;
	showExtrema = false;
	showTangentField = false;
}


bool PlotAppearance::operator !=( const PlotAppearance & other ) const
{
	return
			(lineWidth != other.lineWidth) ||
			(color != other.color) ||
			(useGradient != other.useGradient) ||
			(gradient.stops() != other.gradient.stops()) ||
			(visible != other.visible) ||
			(style != other.style) ||
			(showExtrema != other.showExtrema) ||
			(showTangentField != other.showTangentField);
}


QString PlotAppearance::penStyleToString( Qt::PenStyle style )
{
	switch ( style )
	{
		case Qt::NoPen:
			return "NoPen";
			
		case Qt::SolidLine:
			return "SolidLine";
			
		case Qt::DashLine:
			return "DashLine";
			
		case Qt::DotLine:
			return "DotLine";
			
		case Qt::DashDotLine:
			return "DashDotLine";
		
		case Qt::DashDotDotLine:
			return "DashDotDotLine";
			
		case Qt::MPenStyle:
		case Qt::CustomDashLine:
			kWarning() << "Unsupported pen style\n";
			break;
	}
	
	kWarning() << k_funcinfo << "Unknown style " << style << endl;
	return "SolidLine";
}


Qt::PenStyle PlotAppearance::stringToPenStyle( const QString & style )
{
	if ( style == "NoPen" )
		return Qt::NoPen;
			
	if ( style == "SolidLine" )
		return Qt::SolidLine;
			
	if ( style == "DashLine" )
		return Qt::DashLine;
			
	if ( style == "DotLine" )
		return Qt::DotLine;
			
	if ( style == "DashDotLine" )
		return Qt::DashDotLine;
		
	if ( style == "DashDotDotLine" )
		return Qt::DashDotDotLine;
	
	kWarning() << k_funcinfo << "Unknown style " << style << endl;
	return Qt::SolidLine;
}
//END class PlotAppearance



//BEGIN class DifferentialState
DifferentialState::DifferentialState()
{
	x = 0;
}


DifferentialState::DifferentialState( int order )
{
	x = 0;
	setOrder( order );
}


void DifferentialState::setOrder( int order )
{
	bool orderWasZero = (y0.size() == 0);
	
	y.resize( order );
	y0.resize( order );
	
	if ( orderWasZero && order >= 1 )
		y0[0].updateExpression( "1" );
	
	resetToInitial();
}


void DifferentialState::resetToInitial()
{
	x = x0.value();
	y = y0;
}


bool DifferentialState::operator == ( const DifferentialState & other ) const
{
	return (x0 == other.x0) && (x == other.x) && (y0 == other.y0) && (y == other.y);
}
//END class DifferentialState



//BEGIN class DifferentialStates
DifferentialStates::DifferentialStates()
{
	m_order = 0;
	m_step.updateExpression( 0.05 );
}


void DifferentialStates::setOrder( int order )
{
	m_order = order;
	for ( int i = 0; i < m_data.size(); ++i )
		m_data[i].setOrder( order );
}


DifferentialState * DifferentialStates::add()
{
	m_data << DifferentialState( order() );
	return & m_data[ size() - 1 ];
}


void DifferentialStates::resetToInitial( )
{
	for ( int i = 0; i < m_data.size(); ++i )
		m_data[i].resetToInitial();
}
//END class DifferentialStates



//BEGIN class Equation
Equation::Equation( Type type, Function * parent )
	: m_type( type ),
	  m_parent( parent )
{
	mem = new unsigned char [MEMSIZE];
	mptr = 0;
	
	if ( type == Differential || type == Cartesian )
		differentialStates.add();
}


Equation::~ Equation()
{
	delete [] mem;
	mem = 0;
}


int Equation::order( ) const
{
	if ( type() == Cartesian )
	{
		// For drawing integrals
		return 1;
	}
	else
		return name(false).count( '\'' );
}


int Equation::pmCount() const
{
	return m_fstr.count( QChar( 0xb1 ) );
}


QString Equation::name( bool removePrimes ) const
{
	if ( m_fstr.isEmpty() )
		return QString();
	
	int pos = m_fstr.indexOf( '(' );
	
	if ( pos == -1 )
		return QString();
	
	QString n = m_fstr.left( pos ).trimmed();
	
	if ( removePrimes )
		n.remove( '\'' );
	
	return n;
}


void Equation::updateVariables()
{
	m_variables.clear();
	
	int p1 = m_fstr.indexOf( '(' );
	int p2 = m_fstr.indexOf( ')' );
	if ( (p1 == -1) || (p2 == -1) )
		return;
	
	m_variables = m_fstr.mid( p1+1, p2-p1-1 ).split( ',' );
	
	// If we are a differential equation, then add on y, y', etc
	if ( type() == Differential )
	{
		QString n = name();
		
		int order = this->order();
		for ( int i = 0; i < order; ++i )
		{
			m_variables << n;
			n += '\'';
		}
	}
}


bool Equation::setFstr( const QString & fstr )
{
	QString prevFstr = m_fstr;
	m_fstr = fstr;
	updateVariables();
	
	// require order to be greater than 0 for differential equations
	if ( (type() == Differential) && (order() < 1) )
	{
		m_fstr = prevFstr;
		updateVariables();
		
		XParser::self()->setParserError( Parser::ZeroOrder );
		return false;
	}
	
	int maxArg = order() + (( type() == Implicit ) ? 3 : 2);
	if ( variables().size() > maxArg )
	{
		m_fstr = prevFstr;
		updateVariables();
		
		/// \todo indicate the position of the invalid argument?
		XParser::self()->setParserError( Parser::TooManyArguments );
		return false;
	}
	
	XParser::self()->initEquation( this );
	if ( XParser::self()->parserError( false ) != Parser::ParseSuccess )
	{
		kDebug() << k_funcinfo << "BAD XParser::self()->errorPosition()="<< XParser::self()->errorPosition()<< " error="<<XParser::self()->errorString()<< endl;
		
		m_fstr = prevFstr;
		XParser::self()->initEquation( this );
		return false;
	}
	
	differentialStates.setOrder( order() );
	return true;
}


void Equation::setPMSignature( QVector< bool > pmSignature )
{
	differentialStates.resetToInitial();
	m_pmSignature = pmSignature;
}


bool Equation::operator !=( const Equation & other )
{
	return (fstr() != other.fstr()) ||
			(differentialStates != other.differentialStates);
}


Equation & Equation::operator =( const Equation & other )
{
	setFstr( other.fstr() );
	differentialStates = other.differentialStates;
	
	return * this;
}
//END class Equation



//BEGIN class Function
Function::Function( Type type )
	: m_type( type )
{
	x = y = 0;
	m_implicitMode = UnfixedXY;
	
	usecustomxmin = false;
	usecustomxmax = false;
	
	switch ( m_type )
	{
		case Cartesian:
			eq << new Equation( Equation::Cartesian, this );
			dmin.updateExpression( QString("-")+QChar(960) );
			dmax.updateExpression( QChar(960) );
			break;
			
		case Polar:
			eq << new Equation( Equation::Polar, this );
			dmin.updateExpression( QChar('0') );
			dmax.updateExpression( QString(QChar('2')) + QChar(960) );
			usecustomxmin = true;
			usecustomxmax = true;
			break;
			
		case Parametric:
			eq << new Equation( Equation::ParametricX, this );
			eq << new Equation( Equation::ParametricY, this );
			dmin.updateExpression( QString("-")+QChar(960) );
			dmax.updateExpression( QChar(960) );
			usecustomxmin = true;
			usecustomxmax = true;
			break;
			
		case Implicit:
			eq << new Equation( Equation::Implicit, this );
			break;
			
		case Differential:
			eq << new Equation( Equation::Differential, this );
			break;
	}
	
	id = 0;
	f0.visible = true;
	
	k = 0;
}


Function::~Function()
{
	foreach ( Equation * e, eq )
		delete e;
}


bool Function::copyFrom( const Function & function )
{
	bool changed = false;
	int i = 0;
#define COPY_AND_CHECK(s) \
	{ \
		if ( s != function.s ) \
		{ \
			s = function.s; \
			changed = true; \
		} \
	} \
	i++;
	
	COPY_AND_CHECK( f0 );				// 0
	COPY_AND_CHECK( f1 );				// 1
	COPY_AND_CHECK( f2 );				// 2
	COPY_AND_CHECK( integral );			// 3
	COPY_AND_CHECK( dmin );				// 4
	COPY_AND_CHECK( dmax );				// 5
	COPY_AND_CHECK( usecustomxmin );	// 6
	COPY_AND_CHECK( usecustomxmax );	// 7
	COPY_AND_CHECK( m_parameters );		// 8
	
	// handle equations separately
	for ( int i = 0; i < eq.size(); ++i )
	{
		if ( *eq[i] != *function.eq[i] )
		{
			changed = true;
			*eq[i] = *function.eq[i];
		}
	}
	
	return changed;
}


QString Function::prettyName( Function::PMode mode ) const
{
	if ( type() == Parametric )
		return eq[0]->fstr() + " ; " + eq[1]->fstr();
	
	switch ( mode )
	{
		case Function::Derivative0:
			return eq[0]->fstr();
			
		case Function::Derivative1:
			return eq[0]->name() + '\'';
			
		case Function::Derivative2:
			return eq[0]->name() + "\'\'";
			
		case Function::Integral:
			return eq[0]->name().toUpper();
	}
	
	kWarning() << k_funcinfo << "Unknown mode!\n";
	return "???";
}


PlotAppearance & Function::plotAppearance( PMode plot )
{
	// NOTE: This function is identical to the const one, so changes to this should be applied to both
	
	switch ( plot )
	{
		case Function::Derivative0:
			return f0;
		case Function::Derivative1:
			return f1;
			
		case Function::Derivative2:
			return f2;
			
		case Function::Integral:
			return integral;
	}
	
	kError() << k_funcinfo << "Unknown plot " << plot << endl;
	return f0;
}
PlotAppearance Function::plotAppearance( PMode plot ) const
{
	// NOTE: This function is identical to the none-const one, so changes to this should be applied to both
	
	switch ( plot )
	{
		case Function::Derivative0:
			return f0;
		case Function::Derivative1:
			return f1;
		case Function::Derivative2:
			return f2;
		case Function::Integral:
			return integral;
	}
	
	kError() << k_funcinfo << "Unknown plot " << plot << endl;
	return f0;
}


bool Function::allPlotsAreHidden( ) const
{
	return !f0.visible && !f1.visible && !f2.visible && !integral.visible;
}


QString Function::typeToString( Type type )
{
	switch ( type )
	{
		case Cartesian:
			return "cartesian";
			
		case Parametric:
			return "parametric";
			
		case Polar:
			return "polar";
			
		case Implicit:
			return "implicit";
			
		case Differential:
			return "differential";
	}
	
	kWarning() << "Unknown type " << type << endl;
	return "unknown";
}


Function::Type Function::stringToType( const QString & type )
{
	if ( type == "cartesian" )
		return Cartesian;
	
	if ( type == "parametric" )
		return Parametric;
	
	if ( type == "polar" )
		return Polar;
	
	if ( type == "implicit" )
		return Implicit;
	
	if ( type == "differential" )
		return Differential;
	
	kWarning() << "Unknown type " << type << endl;
	return Cartesian;
}


QList< Plot > Function::plots( PlotCombinations combinations ) const
{
	QList< Plot > list;
	
	Plot plot;
	plot.setFunctionID( id );
	plot.plotNumberCount = m_parameters.useList ? m_parameters.list.size() + (m_parameters.useSlider?1:0) : 1;
	
	bool singlePlot = (!m_parameters.useList && !m_parameters.useSlider) ||
			m_parameters.animating ||
			(~combinations & DifferentParameters);
	
	if ( singlePlot )
	{
		if ( m_parameters.animating )
			plot.parameter = Parameter( Parameter::Animated );
		
		list << plot;
	}
	else
	{
		int i = 0;
		
		if ( m_parameters.useSlider )
		{
			Parameter param( Parameter::Slider );
			param.setSliderID( m_parameters.sliderID );
			plot.parameter = param;
			plot.plotNumber = i++;
			list << plot;
		}
		
		if ( m_parameters.useList )
		{
			int pos = 0;
			foreach ( Value v, m_parameters.list )
			{
				Parameter param( Parameter::List );
				param.setListPos( pos++ );
				plot.parameter = param;
				plot.plotNumber = i++;
				list << plot;
			}
		}
	}
	
	
	// Copy each plot in the list for other variations
	if ( (type() == Cartesian) && (combinations & DifferentDerivatives) )
	{
		QList< Plot > duplicated;
		
		for ( PMode p = Derivative0; p <= Integral; p = PMode(p+1) )
		{
			foreach ( Plot plot, list )
			{
				if ( !plotAppearance(p).visible )
					continue;
				plot.plotMode = p;
				duplicated << plot;
			}
		}
		
		list = duplicated;
	}
	
	if ( (type() == Differential) && (combinations & DifferentInitialStates) )
	{
		QList< Plot > duplicated;
		
		for ( int i = 0; i < eq[0]->differentialStates.size(); ++i )
		{
			foreach ( Plot plot, list )
			{
				plot.state = i;
				duplicated << plot;
			}
		}
		
		list = duplicated;
	}
	
	if ( combinations & DifferentPMSignatures )
	{
		int size = 0;
		foreach ( Equation * equation, eq )
			size += equation->pmCount();
	
		unsigned max = unsigned( pow( 2, size ) );
		QVector< QVector<bool> > signatures( max );
	
		for ( unsigned i = 0; i < max; ++i )
		{
			QVector<bool> sig( size );
			
			for ( int j = 0; j < size; ++j )
				sig[ j ] = i & (1<<j);
			
			signatures[i] = sig;
		}
	
		// Generate a plot for each signature in signatures
		QList< Plot > duplicated;
		foreach ( QVector<bool> signature, signatures )
		{
			int at = 0;
			QList< QVector<bool> > pmSignature;
		
			foreach ( Equation * equation, eq )
			{
				int pmCount = equation->pmCount();
				QVector<bool> sig( pmCount );
				for ( int i = 0; i < pmCount; ++i )
					sig[i] = signature[ i + at];
				at += pmCount;
			
				pmSignature << sig;
			}
		
			foreach ( Plot plot, list )
			{
				plot.pmSignature = pmSignature;
				duplicated << plot;
			}
		}
		list = duplicated;
	}
	
	return list;
}
//END class Function



//BEGIN class ParameterSettings
ParameterSettings::ParameterSettings()
{
	animating = false;
	useSlider = false;
	sliderID = 0;
	useList = false;
}


bool ParameterSettings::operator == ( const ParameterSettings & other ) const
{
	return ( useSlider == other.useSlider ) &&
			( sliderID == other.sliderID ) &&
			( useList == other.useList ) &&
			( list == other.list );
}
//END class ParameterSettings



//BEGIN class Parameter
Parameter::Parameter( Type type )
	: m_type( type )
{
	m_sliderID = -1;
	m_listPos = -1;
}


bool Parameter::operator == ( const Parameter & other ) const
{
	return ( type() == other.type() ) &&
			( listPos() == other.listPos() ) &&
			( sliderID() == other.sliderID() );
}
//END class Parameter



//BEGIN class Plot
Plot::Plot( )
{
	state = -1;
	plotNumberCount = 1;
	plotNumber = 0;
	m_function = 0;
	m_functionID = -1;
	plotMode = Function::Derivative0;
}


bool Plot::operator ==( const Plot & other ) const
{
	return ( m_functionID == other.functionID() ) &&
			( plotMode == other.plotMode ) &&
			( parameter == other.parameter ) &&
			( state == other.state );
}


void Plot::setFunctionID( int id )
{
	m_functionID = id;
	updateCached();
}


void Plot::updateCached()
{
	m_function = XParser::self()->functionWithID( m_functionID );
}


void Plot::updateFunction() const
{
	if ( !m_function )
		return;
	
	// Update the plus-minus signature
	assert( pmSignature.size() <= m_function->eq.size() );
	for ( int i = 0; i < pmSignature.size(); ++i )
		m_function->eq[i]->setPMSignature( pmSignature[i] );
	
	
	// Update the parameter
	
	double k = 0.0;
	
	switch ( parameter.type() )
	{
		case Parameter::Unknown:
			break;
			
		case Parameter::Slider:
		{
			KSliderWindow * sw = View::self()->m_sliderWindow;
			
			if ( !sw )
			{
				// Slider window isn't open. Ask View to open it
				View::self()->updateSliders();
				
				// It should now be open
				sw = View::self()->m_sliderWindow;
				assert( sw );
			}
			
			k = sw->value( parameter.sliderID() );
			break;
		}
			
		case Parameter::List:
		{
			if ( (parameter.listPos() >= 0) && (parameter.listPos() < m_function->m_parameters.list.size()) )
				k = m_function->m_parameters.list[ parameter.listPos() ].value();
			break;
		}
		
		case Parameter::Animated:
		{
			// Don't adjust the current function parameter
			return;
		}
	}
	
	m_function->setParameter( k );
}


void Plot::differentiate()
{
	switch ( plotMode )
	{
		case Function::Integral:
			plotMode = Function::Derivative0;
			break;

		case Function::Derivative0:
			plotMode = Function::Derivative1;
			break;

		case Function::Derivative1:
			plotMode = Function::Derivative2;
			break;

		case Function::Derivative2:
			kWarning() << k_funcinfo << "Can't handle this yet!\n";
			break;
	}
}


void Plot::integrate()
{
	switch ( plotMode )
	{
		case Function::Integral:
			kWarning() << k_funcinfo << "Can't handle this yet!\n";
			break;

		case Function::Derivative0:
			plotMode = Function::Integral;
			break;

		case Function::Derivative1:
			plotMode = Function::Derivative0;
			break;

		case Function::Derivative2:
			plotMode = Function::Derivative1;
			break;
	}
}


QColor Plot::color( ) const
{
	Function * f = function();
	assert(f); // Shouldn't call color without a function
	PlotAppearance appearance = f->plotAppearance( plotMode );
	
	if ( (plotNumberCount <= 1) || !appearance.useGradient )
		return appearance.color;
	
	// Is a gradient
	
	int x = plotNumber;
	int l = plotNumberCount;
	
	QLinearGradient lg( 0, 0, l-1, 0 );
	lg.setStops( appearance.gradient.stops() );
	QImage im( l, 1, QImage::Format_RGB32 );
	QPainter p(&im);
	p.setPen( QPen( lg, 1 ) );
	p.drawLine( 0, 0, l, 0 );
	return im.pixel( x, 0 );
}


int Plot::derivativeNumber( ) const
{
	switch ( plotMode )
	{
		case Function::Integral:
			return -1;
		case Function::Derivative0:
			return 0;
		case Function::Derivative1:
			return 1;
		case Function::Derivative2:
			return 2;
	}
	
	kWarning() << k_funcinfo << "Unknown derivative number.\n";
	return 0;
}
//END class Plot
